#pragma once

#include "LGFX.h"

// Draws a single radar sweep line from centre (x0,y0) to the sweep edge (x1,y1).
// No trailing fan/afterglow here - persistence is handled per-target instead (see AircraftManager::Draw).
void DrawScanLine(LGFX_Sprite& buf, const int x0, const int y0, const int x1, const int y1)
{
    buf.drawLine(x0, y0, x1, y1, lgfx::color888(0, 200, 0));
}