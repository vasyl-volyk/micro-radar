#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

#include "LGFX.h"
#include "WiFiManagerHelpers.h"
#include "ConfigurationWebServer.h"
#include "HttpRequestManager.h"
#include "OpenSkyAuthTokenHandler.h"
#include "AircraftManager.h"
#include "DrawHelpers.h"
#include "models/Aircraft.h"
#include "models/TrackedAircraft.h"

// Optional hard-coded Wi-Fi credentials. Leave both blank to skip pre-baking them and use the setup hotspot instead.
const char* preconfiguredWifiSsid = "";
const char* preconfiguredWifiPassword = "";

constexpr int SCREEN_SIZE = 240;
constexpr int SCREEN_SIZE_DIV_2 = (SCREEN_SIZE / 2);

LGFX tft;
LGFX_Sprite backbuffer(&tft);

WiFiManager wm;
ConfigurationWebServer configServer;
HttpRequestManager http;
OpenSkyAuthTokenHandler authHandler(http);

AircraftManager aircraftManager(configServer, authHandler, http, tft);

void setup()
{
  Serial.begin(115200);
  // delay(1000); // avoids immediate serial output being cut off - uncomment if needed

  // initialise LGFX + screen
  tft.init();
  tft.invertDisplay(true);
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);

  backbuffer.setColorDepth(8);
  backbuffer.createSprite(SCREEN_SIZE, SCREEN_SIZE);

  // establish WiFi connection
  tft.fillScreen(lgfx::color888(0, 0, 0));
  tft.setTextColor(lgfx::color888(0, 255, 0));
  tft.drawCentreString("Connecting to WiFi...", SCREEN_SIZE / 2, SCREEN_SIZE / 2);

  WiFiManagerHelpers::ConfigureWiFiManager(wm, tft);

  if (strlen(preconfiguredWifiSsid) > 0) {
    WiFi.begin(preconfiguredWifiSsid, preconfiguredWifiPassword);
    WiFi.waitForConnectResult();
  }

  wm.autoConnect(WiFiManagerHelpers::WiFiManagerName);

  // begin background server for configuration
  configServer.Initialise();

  // initialise aircraft manager
  aircraftManager.Initialise();
}

void loop()
{
  aircraftManager.Update();

  // draw cycle
  backbuffer.fillScreen(lgfx::color888(0, 0, 0));

  String renderScanlines = configServer.GetStoredString("scanline");
  const bool sweepEnabled = renderScanlines.isEmpty() || renderScanlines == "true";
  const float sweepAngle = millis() / 3000.0f; // radians, monotonically increasing - one full sweep every ~18.8s

  if (sweepEnabled) {
    DrawScanLine(backbuffer,
      SCREEN_SIZE_DIV_2 - 1,
      SCREEN_SIZE_DIV_2 - 1,
      SCREEN_SIZE_DIV_2 - 1 + (std::cos(sweepAngle) * SCREEN_SIZE_DIV_2),
      SCREEN_SIZE_DIV_2 - 1 + (std::sin(sweepAngle) * SCREEN_SIZE_DIV_2)
    );
  }

  // when the sweep is on, targets glow brightest as the beam passes them and fade until it comes back around
  aircraftManager.Draw(backbuffer, sweepEnabled, sweepAngle);
  backbuffer.pushSprite(0, 0);
}

