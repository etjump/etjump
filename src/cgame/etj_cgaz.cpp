#include "etj_cgaz.h"
#include "etj_utilities.h"
#include "../game/etj_numeric_utilities.h"

namespace ETJump
{
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

			ETJump::parseColorString(etj_CGazColor1.string, color1);
			ETJump::parseColorString(etj_CGazColor2.string, color2);

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

	// Dzikie
	static void PutPixel(float x, float y)
	{
		CG_DrawPic(x, y, 1, 1, cgs.media.whiteShader);
	}

	// Dzikie
	static void DrawLine(float x1, float y1, float x2, float y2, vec4_t color)
	{
		float len, stepx, stepy;
		float i;

		if (x1 == x2 && y1 == y2)
		{
			return;
		}

		trap_R_SetColor(color);

		// Use a single DrawPic for horizontal or vertical lines
		if (x1 == x2)
		{
			CG_DrawPic(x1, y1 < y2 ? y1 : y2, 1, abs(y1 - y2), cgs.media.whiteShader);
		}
		else if (y1 == y2)
		{
			CG_DrawPic(x1 < x2 ? x1 : x2, y1, abs(x1 - x2), 1, cgs.media.whiteShader);
		}
		else
		{
			len = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
			len = sqrt(len);
			stepx = (x2 - x1) / len;
			stepy = (y2 - y1) / len;
			for (i = 0; i < len; i++)
			{
				PutPixel(x1, y1);
				x1 += stepx;
				y1 += stepy;
			}
		}

		trap_R_SetColor(NULL);
	}
}
