#pragma once

#include "cg_local.h"

namespace ETJump
{
	void DrawCGazHUD(void);
	usercmd_t getUsercmd(const playerState_t& pm_ps, int8_t ucmdScale);
	void PM_CalcFriction(const playerState_t& ps, vec3_t& vel, float& accel);
	float PM_CalcScale(const playerState_t& ps);
	float PM_CalcScaleAlt(const playerState_t& pm_ps, usercmd_t const& cmd);
	bool strafingForwards(const playerState_t& ps);
	float getOptAngle(const playerState_t& ps);
}
