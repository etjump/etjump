#include "etj_snaphud.h"
#include "etj_utilities.h"
#include "../game/etj_numeric_utilities.h"
#include "etj_cgaz.h"

// Snaphud implementation based on iodfe

namespace ETJump
{
	constexpr int SNAPHUD_MAXZONES{ 128 };
	float         snapSpeed;
	float         snapZones[SNAPHUD_MAXZONES];
	int           snapCount;

	static int QDECL sortSnapZones(const void *a, const void *b)
	{
		return *(float *)a - *(float *)b;
	}

	static void UpdateSnapHUDSettings(float speed)
	{
		float step;

		snapSpeed = speed;
		speed    /= 125;
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
		float         scale;
		float         speed;
		float         y, h;
		float         yaw;
		float         fov;
		vec4_t        color[2];
		int           colorID = 0;
		playerState_t *ps;

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
		scale = PM_CalcScale(*ps);

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
		usercmd_t    cmd       = getUsercmd(*ps, ucmdScale);

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

	bool inMainAccelZone(const playerState_t& ps)
	{
		// get player yaw
		float yaw = ps.viewangles[YAW];

		// get usercmd
		constexpr int8_t ucmdScale = 127;
		const usercmd_t  cmd       = getUsercmd(ps, ucmdScale);

		// determine whether strafestyle is "forwards"
		const bool forwards = strafingForwards(ps);

		// determine whether strafing to the right ("moving mouse rightwards")
		const bool rightStrafe = (forwards && cmd.rightmove > 0) || (!forwards && (cmd.rightmove < 0 || (cmd.forwardmove != 0 && cmd.rightmove == 0)));

		// get opt angle
		float opt = getOptAngle(ps);

		// update snapzones even if snaphud not drawn
		const float speed = cg.snap->ps.speed * PM_CalcScale(ps);
		if (speed != snapSpeed)
		{
			UpdateSnapHUDSettings(speed);
		}

		// necessary 45 degrees shift to match snapzones
		if ((cmd.forwardmove != 0 && cmd.rightmove != 0) || (cmd.forwardmove == 0 && cmd.rightmove == 0))
		{
			yaw += 45;
			opt += 45;
		}

		// snapzones only cover (a bit more than) one fourth of all viewangles, therefore crop yaw and opt to [0,90)
		yaw = std::fmod(AngleNormalize360(yaw), 90);
		opt = std::fmod(AngleNormalize360(opt), 90);

		// get snapzone index which corresponds to the *next* snapzone
		int i = 0;
		// linear search is good enough here as snapCount is always relatively small
		while (i < snapCount && opt >= snapZones[i])
		{
			++i;
		}

		// adjust snapzone index for rightStrafe
		if (rightStrafe)
		{
			i = (i == 0 ? snapCount : i - 1);
		}

		// get the snapzone
		const float& snap = snapZones[i];
		// snap now contains the yaw value corresponding to the start of the next snapzone,
		// or equivalently the end of the current snapzone

		// return true if yaw is between opt angle and end of the current snapzone
		// also account for jumps at the boundary (e.g. 100 and 10 both have to be valid)
		if (rightStrafe)
		{
			if (yaw < opt && yaw > snap)
			{
				return true;
			}
			// this is awkward because can not check for yaw >= 0 since that is always true
			if (snap > 90 && (yaw > 90 - std::fmod(snap, 90) || yaw < opt))
			{
				return true;
			}
		}
		else
		{
			if (yaw > opt && yaw < snap)
			{
				return true;
			}
			if (snap > 90 && yaw < std::fmod(snap, 90))
			{
				return true;
			}
		}

		return false;
	}
}
