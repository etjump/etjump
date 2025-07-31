/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "q_shared.h"

namespace ETJump {
// portal trail colors
inline constexpr vec3_t portalBlueTrail = {0.0f, 0.0f, 1.0f};
inline constexpr vec3_t portalRedTrail = {1.0f, 0.0f, 0.0f};
inline constexpr vec3_t portalGreenTrail = {0.2f, 1.0f, 0.5f};
inline constexpr vec3_t portalYellowTrail = {1.0f, 1.0f, 0.0f};

// portalteam
inline constexpr int PORTAL_TEAM_NONE = 0;
inline constexpr int PORTAL_TEAM_FT = 1;
inline constexpr int PORTAL_TEAM_ALL = 2;

// cooldown between portal touch events (ms)
inline constexpr int PORTAL_TOUCH_COOLDOWN = 100;

// radius of portal bbox
inline constexpr float PORTAL_BBOX_RADIUS = 30.0f;
// depth for horizontal portals
inline constexpr float PORTAL_BBOX_HOR_DEPTH = 15.0f;
// depth for vertical portals
inline constexpr float PORTAL_BBOX_VERT_DEPTH = 5.0f;

// radius for portal shader drawing
inline constexpr float PORTAL_DRAW_RADIUS = 48.0f;
// scalar for drawing the portal shader proportional to the portal size
inline constexpr float PORTAL_DRAW_SCALAR =
    PORTAL_DRAW_RADIUS / (PORTAL_BBOX_RADIUS * 2);
} // namespace ETJump
