#pragma once

#include "cg_local.h"

namespace ETJump
{
	constexpr int CGAZ3_ANG{ 20 };

	void DrawCGazHUD(void);

	static void PutPixel(float x, float y);
	static void DrawLine(float x1, float y1, float x2, float y2, vec4_t color);
	static void PM_CalcFriction(playerState_t* ps, vec3_t& vel, float& accel);
	static float PM_CalcScale(playerState_t* ps);
}
