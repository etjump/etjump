#include "etj_snaphud.h"
#include "etj_utilities.h"
#include "../game/etj_numeric_utilities.h"

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

	static usercmd_t getUsercmd(playerState_t const& pm_ps, int8_t ucmdScale)
	{
		usercmd_t cmd;
		if (!cg.demoPlayback && !(pm_ps.pm_flags & PMF_FOLLOW))
		{
			int cmdNum = trap_GetCurrentCmdNumber();
			trap_GetUserCmd(cmdNum, &cmd);
		}
		else
		{
			cmd.forwardmove = ucmdScale * (!!(pm_ps.stats[STAT_USERCMD_MOVE] & UMOVE_FORWARD) -
				!!(pm_ps.stats[STAT_USERCMD_MOVE] & UMOVE_BACKWARD));
			cmd.rightmove = ucmdScale * (!!(pm_ps.stats[STAT_USERCMD_MOVE] & UMOVE_RIGHT) -
				!!(pm_ps.stats[STAT_USERCMD_MOVE] & UMOVE_LEFT));
			cmd.upmove = ucmdScale * (!!(pm_ps.stats[STAT_USERCMD_MOVE] & UMOVE_UP) -
				!!(pm_ps.stats[STAT_USERCMD_MOVE] & UMOVE_DOWN));
		}
		// printf("%d %d %d\n", cmd.forwardmove, cmd.rightmove, cmd.upmove);
		return cmd;
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
		float fov;
		vec4_t color[2];
		int colorID = 0;
		playerState_t* ps;

		ps = &cg.predictedPlayerState;

		if (!etj_drawSnapHUD.integer)
		{
			return;
		}

		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
		{
			return;
		}

		if (cg.zoomedBinoc || cg.zoomedScope)
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

		int8_t const ucmdScale = 127;
		usercmd_t cmd = getUsercmd(*ps, ucmdScale);

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

		if (!etj_snapHUDFov.value)
		{
			fov = cg.refdef.fov_x;
		}
		else
		{
			fov = Numeric::clamp(etj_snapHUDFov.value, 1, 180);
		}

		parseColorString(etj_snapHUDColor1.string, color[0]);
		parseColorString(etj_snapHUDColor2.string, color[1]);

		for (int i = 0; i < snapCount; i++)
		{
			CG_FillAngleYaw(snapZones[i], snapZones[i + 1], yaw, y, h, fov, color[colorID]);
			CG_FillAngleYaw(snapZones[i] + 90, snapZones[i + 1] + 90, yaw, y, h, fov, color[colorID]);
			colorID ^= 1;
		}
	}
}
