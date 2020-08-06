#include "etj_snaphud.h"
#include "etj_utilities.h"

// Snaphud implementation based on iodfe

namespace ETJump
{
	constexpr int SNAPHUD_MAXZONES{ 128 };
	float snapSpeed;
	float snapZones[SNAPHUD_MAXZONES];
	int snapCount;

	static int QDECL sortSnapZones(const void* a, const void* b)
	{
		return *(float*)a - *(float*)b;
	}

	// FIXME: share this with cgaz
	static float PM_CalcScale(playerState_t* ps)
	{
		// based on PM_CmdScale from bg_pmove.c
		float scale = ps->stats[STAT_USERCMD_BUTTONS] & (BUTTON_SPRINT << 8) && cg.pmext.sprintTime > 50 ? ps->sprintSpeedScale : ps->runSpeedScale;
		return scale;
	}

	static void UpdateSnapHUDSettings(float speed)
	{
		float step;

		snapSpeed = speed;
		speed /= 125;
		snapCount = 0;

		for (step = floor(speed + 0.5) - 0.5; step > 0 && snapCount < SNAPHUD_MAXZONES - 2; step--)
		{
			snapZones[snapCount] = RAD2DEG(acos(step / speed));
			snapCount++;
			snapZones[snapCount] = RAD2DEG(asin(step / speed));
			snapCount++;
		}

		qsort(snapZones, snapCount, sizeof(snapZones[0]), sortSnapZones);
		snapZones[snapCount] = snapZones[0] + 90;
	}

	void DrawSnapHUD(void)
	{
		float scale;
		float speed;
		float y, h;
		float yaw;
		vec4_t color[2];
		int colorID = 0;
		playerState_t* ps;
		usercmd_t cmd;
		int cmdNum;

		ps = &cg.predictedPlayerState;

		if (!etj_drawSnapHUD.integer)
		{
			return;
		}

		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
		{
			return;
		}

		// get correct speed scaling
		scale = PM_CalcScale(ps);

		// check whether snapSpeed needs to be updated
		speed = cg.snap->ps.speed * scale;
		if (speed != snapSpeed)
		{
			UpdateSnapHUDSettings(speed);
		}

		// apply correct yaw offset for different strafe styles,
		// or if no keys are pressed
		yaw = cg.predictedPlayerState.viewangles[YAW];

		cmdNum = trap_GetCurrentCmdNumber();
		trap_GetUserCmd(cmdNum, &cmd);

		if (cmd.forwardmove != 0 && cmd.rightmove != 0)
		{
			yaw += 45;
		}
		else if (cmd.forwardmove == 0 && cmd.rightmove == 0)
		{
			yaw += 45;
		}

		// draw snaphud
		h = etj_snapHUDHeight.value;
		y = 240 + etj_snapHUDOffsetY.value;

		parseColorString(etj_snapHUDColor1.string, color[0]);
		parseColorString(etj_snapHUDColor2.string, color[1]);

		for (int i = 0; i < snapCount; i++)
		{
			CG_FillAngleYaw(snapZones[i], snapZones[i + 1], yaw, y, h, color[colorID]);
			CG_FillAngleYaw(snapZones[i] + 90, snapZones[i + 1] + 90, yaw, y, h, color[colorID]);
			colorID ^= 1;
		}
	}
}
