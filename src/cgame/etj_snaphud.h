#pragma once

#include "cg_local.h"

namespace ETJump
{
	constexpr int SNAPHUD_MAXZONES{ 128 };
	float snapSpeed;
	float snapZones[SNAPHUD_MAXZONES];
	int snapCount;

	void DrawSnapHUD();
	void UpdateSnapHUDSettings(float speed);
	void CG_FillAngleYaw(float start, float end, float viewangle, float y, float height, const float* color);
	static float PM_CalcScale(playerState_t* ps);
	static int QDECL sortSnapZones(const void* a, const void* b);
}
