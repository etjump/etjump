#include "etj_cgaz.h"
#include "etj_utilities.h"
#include "../game/etj_numeric_utilities.h"

namespace ETJump
{
	constexpr int CGAZ3_ANG{ 20 };

	static float update_d_min(state_t const& state)
	{
		float const num_squared =
			state.wishspeed * state.wishspeed - state.v_squared + state.vf_squared;
		assert(num_squared >= 0);
		float const num = sqrtf(num_squared);
		return num >= state.vf ? 0 : acosf(num / state.vf);
	}

	static float update_d_opt(state_t const& state)
	{
		float const num = state.wishspeed - state.a;
		return num >= state.vf ? 0 : acosf(num / state.vf);
	}

	static float update_d_max_cos(state_t const& state, float d_opt)
	{
		float const num = state.v - state.vf;
		float       d_max_cos = num >= state.a ? 0 : acosf(num / state.a);
		if (d_max_cos < d_opt)
		{
			d_max_cos = d_opt;
		}
		return d_max_cos;
	}

	static float update_d_max(state_t const& state, float d_max_cos)
	{
		float const num = state.v_squared - state.vf_squared - state.a_squared;
		float const den = 2 * state.a * state.vf;
		if (num >= den)
		{
			return 0;
		}
		else if (-num >= den)
		{
			return (float)M_PI;
		}
		float d_max = acosf(num / den);
		if (d_max < d_max_cos)
		{
			assert(state.a == 0);
			d_max = d_max_cos;
		}
		return d_max;
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

	static void PM_CalcFriction(playerState_t* ps, vec3_t& vel, float& accel)
	{
		VectorCopy(ps->velocity, vel);
		// for a simplicity water, ladder etc. calculations are omitted
		// only air, ground and ice movement is important
		if (cg_pmove.walking && !(cg_pmove.groundTrace.surfaceFlags & SURF_SLICK))
		{
			// apply friction
			float speed, newspeed, control;
			float drop;
			speed = VectorLength(vel);
			if (speed > 0)
			{
				drop = 0;

				// if getting knocked back, no friction
				if (!(ps->pm_flags & PMF_TIME_KNOCKBACK))
				{
					control = speed < pm_stopspeed ? pm_stopspeed : speed;
					drop += control * pm_friction * pmove_msec.integer / 1000;
				}
				newspeed = speed - drop;
				if (newspeed < 0)
				{
					newspeed = 0;
				}
				newspeed /= speed;
				VectorScale(vel, newspeed, vel);
			}
			// on ground
			accel = pm_accelerate;
		}
		else
		{
			// in air or on ice, no friction
			accel = pm_airaccelerate;
		}
	}

	static float PM_CalcScale(playerState_t* ps)
	{
		// based on PM_CmdScale from bg_pmove.c
		float scale = ps->stats[STAT_USERCMD_BUTTONS] & (BUTTON_SPRINT << 8) && cg.pmext.sprintTime > 50 ? ps->sprintSpeedScale : ps->runSpeedScale;
		return scale;
	}

	static float PM_CalcScaleAlt(playerState_t const& pm_ps, usercmd_t const& cmd)
	{
		int32_t max = abs(cmd.forwardmove);
		if (abs(cmd.rightmove) > max)
		{
			max = abs(cmd.rightmove);
		}
		if (!max)
		{
			return 0;
		}

		float const total = sqrtf(cmd.forwardmove * cmd.forwardmove + cmd.rightmove * cmd.rightmove);
		float const scale = (float)pm_ps.speed * max / (127.f * total);

		if (pm_ps.stats[STAT_USERCMD_BUTTONS] & (BUTTON_SPRINT << 8) && cg.pmext.sprintTime > 50)
		{
			return scale * pm_ps.sprintSpeedScale;
		}
		else
		{
			return scale * pm_ps.runSpeedScale;
		}
	}

	void DrawCGazHUD(void)
	{
		float         vel_angle; // absolute velocity angle
		float         vel_relang; // relative velocity angle to viewangles[1]
		vec_t         vel_size;
		vec4_t        color;
		float         per_angle;
		float         ang; // only for drawing
		float         a;
		float         y, h, w;
		playerState_t* ps;
		int           right = 0, forward = 0;
		int           scx = SCREEN_CENTER_X - 1, scy = SCREEN_CENTER_Y - 1;
		vec4_t        color1, color2;

		ps = &cg.predictedPlayerState;

		if (!etj_drawCGaz.integer)
		{
			return;
		}

		if (ps->persistant[PERS_TEAM] == TEAM_SPECTATOR)
		{
			return;
		}

		a = Numeric::clamp(etj_CGazAlpha.value, 0.0f, 1.0f);
		color[3] = a;
		y = etj_CGazY.integer > 0 ? etj_CGazY.integer % 480 : 0;
		h = etj_CGazHeight.integer > 0 ? etj_CGazHeight.integer : 0;
		w = etj_CGazWidth.integer > 0 ? etj_CGazWidth.integer : 0;

		vec3_t vel;
		float  accel;
		PM_CalcFriction(ps, vel, accel);

		vel_size = sqrt(vel[0] * vel[0] + vel[1] * vel[1]);
		accel = accel * ps->speed * pmove_msec.integer / 1000;

		float scale = PM_CalcScale(ps);

		per_angle = (ps->speed - accel) / vel_size * scale;
		if (per_angle < 1)
		{
			per_angle = RAD2DEG(acos(per_angle));
		}
		else
		{
			per_angle = ps->viewangles[YAW];
		}
		vel_angle = AngleNormalize180(RAD2DEG(atan2(vel[1], vel[0])));
		vel_relang = AngleNormalize180(ps->viewangles[YAW] - vel_angle);

		// parse usercmd
		if (ps->stats[STAT_USERCMD_MOVE] & UMOVE_FORWARD)
		{
			forward = 127;
		}
		else if (ps->stats[STAT_USERCMD_MOVE] & UMOVE_BACKWARD)
		{
			forward = -128;
		}
		if (ps->stats[STAT_USERCMD_MOVE] & UMOVE_RIGHT)
		{
			right = 127;
		}
		else if (ps->stats[STAT_USERCMD_MOVE] & UMOVE_LEFT)
		{
			right = -128;
		}

		// setup's original CGaz HUD
		if (etj_drawCGaz.integer == 1)
		{
			VectorCopy(colorBlue, color);
			CG_FillRect(scx - w / 2, y, w, h, color);
			CG_FillRect(scx - w / 2, y, 1, h, colorWhite);
			CG_FillRect(scx - w / 4, y, 1, h, colorWhite);
			CG_FillRect(scx, y, 1, h, colorWhite);
			CG_FillRect(scx + w / 4, y, 1, h, colorWhite);
			CG_FillRect(scx + w / 2, y, 1, h, colorWhite);

			if (vel_size < ps->speed * scale)
			{
				return;
			}

			// velocity
			if (vel_relang > -90 && vel_relang < 90)
			{
				CG_FillRect(scx + w * vel_relang / 180, y,
					1, h, colorOrange);
			}

			// left/right perfect strafe
			if (vel_relang > 0)
			{
				ang = AngleNormalize180(vel_relang - per_angle);
				if (ang > -90 && ang < 90)
				{
					CG_FillRect(scx + w * ang / 180, y,
						1, h, colorGreen);
				}
			}
			else
			{
				ang = AngleNormalize180(vel_relang + per_angle);
				if (ang > -90 && ang < 90)
				{
					CG_FillRect(scx + w * ang / 180, y,
						1, h, colorGreen);
				}
			}
			return;
		}

		// Dzikie Weze's 2D-CGaz
		if (etj_drawCGaz.integer == 2)
		{

			parseColorString(etj_CGazColor1.string, color1);
			parseColorString(etj_CGazColor2.string, color2);

			if (etj_stretchCgaz.integer)
			{
				ETJump_EnableWidthScale(false);
				scx -= SCREEN_OFFSET_X;
			}

			vel_relang = DEG2RAD(vel_relang);
			per_angle = DEG2RAD(per_angle);

			DrawLine(scx, scy, scx + right, scy - forward, color2);

			// When under speed*scale velocity, most accel happens when you move straight
			// towards your current velocity, so skip drawing the "wings" on the sides
			auto drawSides = vel_size > (ps->speed * scale);

			vel_size /= 5;
			if (vel_size > SCREEN_HEIGHT / 2)
			{
				vel_size = SCREEN_HEIGHT / 2;
			}

			DrawLine(scx, scy,
				scx + vel_size * sin(vel_relang),
				scy - vel_size * cos(vel_relang), color1);

			if (drawSides)
			{
				vel_size /= 2;
				DrawLine(scx, scy,
					scx + vel_size * sin(vel_relang + per_angle),
					scy - vel_size * cos(vel_relang + per_angle), color1);
				DrawLine(scx, scy,
					scx + vel_size * sin(vel_relang - per_angle),
					scy - vel_size * cos(vel_relang - per_angle), color1);
			}

			if (etj_stretchCgaz.integer)
			{
				ETJump_EnableWidthScale(true);
			}
			return;
		}

		//Feen: CGaz stuff pulled from TJMod
		// another setup's one
		if (etj_drawCGaz.integer == 3)
		{
			float accel_angle;

			accel_angle = atan2(-right, forward);
			accel_angle = AngleNormalize180(ps->viewangles[YAW] + RAD2DEG(accel_angle));

			CG_FillRect(scx - w / 2, y, 1, h, colorWhite);
			CG_FillRect(scx, y, 1, h, colorWhite);
			CG_FillRect(scx + w / 2, y, 1, h, colorWhite);

			if (vel_size < ps->speed * scale)
			{
				return;
			}

			// first case (consider left strafe)
			ang = AngleNormalize180(vel_angle + per_angle - accel_angle);
			if (ang > -CGAZ3_ANG && ang < CGAZ3_ANG)
			{
				// acceleration "should be on the left side" from velocity
				if (ang < 0)
				{
					VectorCopy(colorGreen, color);
					CG_FillRect(scx + w * ang / (2 * CGAZ3_ANG), y,
						-w * ang / (2 * CGAZ3_ANG), h, color);
				}
				else
				{
					VectorCopy(colorRed, color);
					CG_FillRect(scx, y,
						w * ang / (2 * CGAZ3_ANG), h, color);
				}
				return;
			}
			// second case (consider right strafe)
			ang = AngleNormalize180(vel_angle - per_angle - accel_angle);
			if (ang > -CGAZ3_ANG && ang < CGAZ3_ANG)
			{
				// acceleration "should be on the right side" from velocity
				if (ang > 0)
				{
					VectorCopy(colorGreen, color);
					CG_FillRect(scx, y,
						w * ang / (2 * CGAZ3_ANG), h, color);
				}
				else
				{
					VectorCopy(colorRed, color);
					CG_FillRect(scx + w * ang / (2 * CGAZ3_ANG), y,
						-w * ang / (2 * CGAZ3_ANG), h, color);
				}
				return;
			}
			return;
		}

		// corpus of original defrag one
		if (etj_drawCGaz.integer == 4)
		{
			float accel_angle;

			accel_angle = atan2(-right, forward);
			accel_angle = AngleNormalize180(ps->viewangles[YAW] + RAD2DEG(accel_angle));

			if (vel_size < ps->speed * scale)
			{
				return;
			}

			VectorCopy(colorGreen, color);

			// Speed direction
			// FIXME: Shows @side of screen when going backward
			//CG_FillRect(SCREEN_CENTER_X + w * vel_relang / 180, y+20, 1, h/2, colorCyan);
			CG_DrawPic(scx + w * vel_relang / 180, y + h, 16, 16, cgs.media.CGazArrow);

			// FIXME show proper outside border where you stop accelerating
			// first case (consider left strafe)
			ang = AngleNormalize180(vel_angle + per_angle - accel_angle);
			if (ang < 90 && ang > -90)
			{
				// acceleration "should be on the left side" from velocity
				CG_FillRect(scx + w * ang / 180, y, w / 2, h, color);
				CG_FillRect(scx + w * ang / 180, y, 1, h, colorGreen);
				return;

			}
			// second case (consider right strafe)
			ang = AngleNormalize180(vel_angle - per_angle - accel_angle);
			if (ang < 90 && ang > -90)
			{
				// acceleration "should be on the right side" from velocity
				CG_FillRect(scx + w * ang / 180 - w / 2, y, w / 2, h, color);
				CG_FillRect(scx + w * ang / 180, y, 1, h, colorGreen);
				return;
			}
			return;
		}

		// DeFRaG CGaz
		// FIXME: doesn't work on 5:4 resolution
		if (etj_drawCGaz.integer == 5)
		{
			if (VectorLengthSquared2(ps->velocity) == 0)
			{
				return;
			}

			int8_t const ucmdScale = 127;
			usercmd_t cmd = getUsercmd(*ps, ucmdScale);

			// Use default key combination when no user input
			if (!cmd.forwardmove && !cmd.rightmove)
			{
				cmd.forwardmove = ucmdScale;
			}

			// save old velocity for crashlanding
			vec3_t previous_velocity;
			VectorCopy(ps->velocity, previous_velocity);

			vec3_t forward;
			vec3_t right;
			vec3_t up;
			AngleVectors(ps->viewangles, forward, right, up);

			//
			PM_CalcFriction(ps, vel, accel);

			float const scale = PM_CalcScaleAlt(*ps, cmd);

			// project moves down to flat plane
			forward[2] = 0;
			right[2] = 0;
			VectorNormalize(forward);
			VectorNormalize(right);

			vec2_t wishvel;
			for (uint8_t i = 0; i < 2; ++i)
			{
				wishvel[i] = cmd.forwardmove * forward[i] + cmd.rightmove * right[i];
			}

			float const wishspeed = scale * VectorLength2(wishvel);
			// printf("wishspeed: %.6f\n", wishspeed);

			state_t state;
			state.v_squared = VectorLengthSquared2(previous_velocity);
			state.vf_squared = VectorLengthSquared2(ps->velocity);
			state.v_squared = state.vf_squared;

			state.wishspeed = wishspeed;
			state.a = accel * state.wishspeed * pmove_msec.integer / 1000.f;
			state.a_squared = state.a * state.a;
			state.v = sqrtf(state.v_squared);
			state.vf = sqrtf(state.vf_squared);

			float const d_min = update_d_min(state);
			float const d_opt = update_d_opt(state);
			float const d_max_cos = update_d_max_cos(state, d_opt);
			float const d_max = update_d_max(state, d_max_cos);
			// if (d_min > d_opt || d_opt > d_max_cos || d_max_cos > d_max)
			// {
			// 	printf("%.6f <= %.6f <= %.6f <= %.6f\n", d_min, d_opt, d_max_cos, d_max);
			// 	printf("vf: %.6f, a: %.6f, wishspeed: %.6f, frametime: %.6f", state.vf, state.a, state.wishspeed, pmove_msec.integer / 1000.f);
			// }

			float const d_vel = atan2f(ps->velocity[1], ps->velocity[0]);

			float const yaw = atan2f(wishvel[1], wishvel[0]) - d_vel;

			float fov; 
			if (!etj_CGaz5Fov.value)
			{
				fov = cg.refdef.fov_x;
			}
			else
			{
				fov = Numeric::clamp(etj_CGaz5Fov.value, 1, 180);
			}

			// No accel zone
			{
				vec4_t color;
				parseColorString(etj_CGaz5Color1.string, color);
				color[3] = a;
				CG_FillAngleYaw_Ext(-d_min, +d_min, yaw, y, h, fov, color);
			}

			// Min angle
			{
				vec4_t color;
				parseColorString(etj_CGaz5Color2.string, color);
				color[3] = a;
				CG_FillAngleYaw_Ext(+d_min, +d_opt, yaw, y, h, fov, color);
				CG_FillAngleYaw_Ext(-d_opt, -d_min, yaw, y, h, fov, color);
			}

			// Accel zone
			{
				vec4_t color;
				parseColorString(etj_CGaz5Color3.string, color);
				color[3] = a;
				CG_FillAngleYaw_Ext(+d_opt, +d_max_cos, yaw, y, h, fov, color);
				CG_FillAngleYaw_Ext(-d_max_cos, -d_opt, yaw, y, h, fov, color);
			}

			// Max angle
			{
				vec4_t color;
				parseColorString(etj_CGaz5Color4.string, color);
				color[3] = a;
				CG_FillAngleYaw_Ext(+d_max_cos, +d_max, yaw, y, h, fov, color);
				CG_FillAngleYaw_Ext(-d_max, -d_max_cos, yaw, y, h, fov, color);
			}
		}
	}
}
