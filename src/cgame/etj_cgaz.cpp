/*
 * MIT License
 *
 * Copyright (c) 2021 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// HUGE thanks to Jelvan1 for CGaz 1 code
// https://github.com/Jelvan1/cgame_proxymod/

#include "etj_cgaz.h"
#include "etj_utilities.h"
#include "etj_pmove_utils.h"
#include "../game/etj_numeric_utilities.h"

namespace ETJump
{
	void CGaz::UpdateCGaz1(vec3_t wishvel, int8_t uCmdScale, usercmd_t cmd)
	{
		// set default key combination if no user input
		if (!cmd.forwardmove && !cmd.rightmove)
		{
			cmd.forwardmove = uCmdScale;

			// recalculate wishvel with defaulted forwardmove
			PmoveUtils::PM_UpdateWishvel(wishvel, cmd, pm->pmext->forward, pm->pmext->right, pm->pmext->up, *ps);
		}

		yaw = atan2f(wishvel[1], wishvel[0]) - drawVel;
	}

	void CGaz::UpdateCGaz2(void)
	{
		drawVel = AngleNormalize180(ps->viewangles[YAW] - AngleNormalize180(RAD2DEG(drawVel)));
		drawVel = DEG2RAD(drawVel);
	}

	void CGaz::UpdateDraw(float wishspeed, float accel)
	{
		state.gSquared = GetSlickGravity();
		state.vSquared = VectorLengthSquared2(pm->pmext->previous_velocity);
		state.vfSquared = VectorLengthSquared2(pm->pmext->velocity);
		state.wishspeed = wishspeed;
		state.a = accel * state.wishspeed * pm->pmext->frametime;
		state.aSquared = pow(state.a, 2);
		// show true ground zones?
		if (!(etj_CGazTrueness.integer & static_cast<int>(CGazTrueness::CGAZ_GROUND)) ||
			state.vSquared - state.vfSquared >= 2 * state.a * state.wishspeed - state.aSquared)
		{
			state.vSquared = state.vfSquared;
		}

		state.v = sqrtf(state.vSquared);
		state.vf = sqrtf(state.vfSquared);

		drawMin = UpdateDrawMin(&state);
		drawOpt = UpdateDrawOpt(&state);
		drawMaxCos = UpdateDrawMaxCos(&state, drawOpt);
		drawMax = UpdateDrawMax(&state, drawMaxCos);

		drawVel = atan2f(pm->pmext->velocity[1], pm->pmext->velocity[0]);
	}

	float CGaz::UpdateDrawMin(state_t const* state)
	{
		float const num_squared =
			state->wishspeed * state->wishspeed - state->vSquared + state->vfSquared + state->gSquared;
		float const num = sqrtf(num_squared);
		return num >= state->vf ? 0 : acosf(num / state->vf);
	}

	float CGaz::UpdateDrawOpt(state_t const* state)
	{
		float const num = state->wishspeed - state->a;
		return num >= state->vf ? 0 : acosf(num / state->vf);
	}

	float CGaz::UpdateDrawMaxCos(state_t const* state, float drawOpt)
	{
		float const num       = sqrtf(state->vSquared - state->gSquared) - state->vf;
		float       drawMaxCos = num >= state->a ? 0 : acosf(num / state->a);
		if (drawMaxCos < drawOpt)
		{
			drawMaxCos = drawOpt;
		}
		return drawMaxCos;
	}

	float CGaz::UpdateDrawMax(state_t const* state, float drawMaxCos)
	{
		float const num = state->vSquared - state->vfSquared - state->aSquared - state->gSquared;
		float const den = 2 * state->a * state->vf;
		if (num >= den)
		{
			return 0;
		}
		else if (-num >= den)
		{
			return (float)M_PI;
		}
		float drawMax = acosf(num / den);
		if (drawMax < drawMaxCos)
		{
			drawMax = drawMaxCos;
			return drawMax;
		}
		return drawMax;
	}

	float CGaz::GetSlickGravity(void)
	{
		if ((pm->pmext->groundTrace.surfaceFlags & SURF_SLICK) || (ps->pm_flags & PMF_TIME_KNOCKBACK))
		{
			return pow(ps->gravity * pm->pmext->frametime, 2);
		}

		return 0;
	}

	void CGaz::beforeRender()
	{
		const int8_t uCmdScale = ps->stats[STAT_USERCMD_BUTTONS] & (BUTTON_WALKING << 8) ? CMDSCALE_WALK : CMDSCALE_DEFAULT;
		const usercmd_t cmd = PmoveUtils::getUserCmd(*ps, uCmdScale);

		// get correct pmove state
		pm = PmoveUtils::getPmove(cmd);

		// show upmove influence?
		float scale = etj_CGazTrueness.integer & static_cast<int>(CGazTrueness::CGAZ_JUMPCROUCH)
			? pm->pmext->scale
			: pm->pmext->scaleAlt;

		vec3_t wishvel;
		float wishspeed = PmoveUtils::PM_GetWishspeed(wishvel, scale, cmd, pm->pmext->forward, pm->pmext->right, pm->pmext->up, *ps, pm);

		// set default wishspeed for drawing if no user input
		if (!cmd.forwardmove && !cmd.rightmove)
		{
			wishspeed = ps->speed * ps->sprintSpeedScale;
		}

		switch (etj_drawCGaz.integer)
		{
		case 1:
			UpdateDraw(wishspeed, pm->pmext->accel);
			UpdateCGaz1(wishvel, uCmdScale, cmd);
			break;
		case 2:
			UpdateDraw(wishspeed, pm->pmext->accel);
			UpdateCGaz2();
			break;
		default:
			break;
		}
	}

	void CGaz::render() const
	{
		if (canSkipDraw())
		{
			return;
		}

		// DeFRaG proxymod CGaz by Jelvan1
		if (etj_drawCGaz.integer == 1)
		{
			float y = etj_CGazY.integer > 0 ? etj_CGazY.integer % 480 : 0;
			float h = etj_CGazHeight.integer > 0 ? etj_CGazHeight.integer : 0;

			float fov;
			if (!etj_CGazFov.value)
			{
				fov = cg.refdef.fov_x;
			}
			else
			{
				fov = Numeric::clamp(etj_CGazFov.value, 1, 179);
			}

			vec4_t color;

			// No accel zone
			parseColorString(etj_CGaz1Color1.string, color);
			CG_FillAngleYaw(-drawMin, +drawMin, yaw, y, h, fov, color);

			// Min angle
			parseColorString(etj_CGaz1Color2.string, color);
			CG_FillAngleYaw(+drawMin, +drawOpt, yaw, y, h, fov, color);
			CG_FillAngleYaw(-drawOpt, -drawMin, yaw, y, h, fov, color);

			// Accel zone
			parseColorString(etj_CGaz1Color3.string, color);
			CG_FillAngleYaw(+drawOpt, +drawMaxCos, yaw, y, h, fov, color);
			CG_FillAngleYaw(-drawMaxCos, -drawOpt, yaw, y, h, fov, color);

			// Max angle
			parseColorString(etj_CGaz1Color4.string, color);
			CG_FillAngleYaw(+drawMaxCos, +drawMax, yaw, y, h, fov, color);
			CG_FillAngleYaw(-drawMax, -drawMaxCos, yaw, y, h, fov, color);

			return;
		}

		// Dzikie Weze's 2D-CGaz
		if (etj_drawCGaz.integer == 2)
		{
			const usercmd_t cmd = pm->cmd;
			int scx = SCREEN_CENTER_X - 1;
			int scy = SCREEN_CENTER_Y - 1;
			vec4_t color1, color2;

			parseColorString(etj_CGaz2Color1.string, color1);
			parseColorString(etj_CGaz2Color2.string, color2);

			if (etj_stretchCgaz.integer)
			{
				ETJump_EnableWidthScale(false);
				scx -= SCREEN_OFFSET_X;
			}
			DrawLine(scx, scy, scx + cmd.rightmove, scy - cmd.forwardmove, color2);

			// When under wishspeed velocity, most accel happens when you move straight
			// towards your current velocity, so skip drawing the "wings" on the sides
			auto drawSides = state.vf > state.wishspeed;

			auto velSize = state.vf;
			velSize /= 5;
			if (velSize > SCREEN_HEIGHT / 2)
			{
				velSize = SCREEN_HEIGHT / 2;
			}

			DrawLine(scx, scy,
				scx + velSize * sin(drawVel),
				scy - velSize * cos(drawVel), color1);

			if (drawSides)
			{
				velSize /= 2;
				DrawLine(scx, scy,
					scx + velSize * sin(drawVel + drawOpt),
					scy - velSize * cos(drawVel + drawOpt), color1);
				DrawLine(scx, scy,
					scx + velSize * sin(drawVel - drawOpt),
					scy - velSize * cos(drawVel - drawOpt), color1);
			}

			if (etj_stretchCgaz.integer)
			{
				ETJump_EnableWidthScale(true);
			}
			return;
		}
	}

	bool CGaz::strafingForwards(const playerState_t& ps, pmove_t *pm)
	{
		// "forwards" means player viewangles naturally match keys pressed, i.e. looking
		// ahead with +forward and looking backwards with +back, except for nobeat:
		// looking to the left with +forward only and looking to the right with +back only

		// get player speed
		const float speed = VectorLength2(ps.velocity);

		// get sprint scale
		const float scale = PmoveUtils::PM_SprintScale(&ps);

		// get usercmd
		const int8_t ucmdScale     = ps.stats[STAT_USERCMD_BUTTONS] & (BUTTON_WALKING << 8) ? CMDSCALE_WALK : CMDSCALE_DEFAULT;
		const usercmd_t  cmd       = PmoveUtils::getUserCmd(ps, ucmdScale);

		// not strafing if speed lower than ground speed or no user input
		if (speed < ps.speed * scale || (cmd.forwardmove == 0 && cmd.rightmove == 0))
		{
			return false;
		}

		// get wishvel
		vec3_t wishvel;
		PmoveUtils::PM_UpdateWishvel(wishvel, cmd, pm->pmext->forward, pm->pmext->right, pm->pmext->up, ps);

		// get angle between wishvel and player velocity
		const float wishvelAngle = RAD2DEG(std::atan2(wishvel[1], wishvel[0]));
		const float velAngle     = RAD2DEG(std::atan2(ps.velocity[1], ps.velocity[0]));
		const float diffAngle    = AngleDelta(wishvelAngle, velAngle);

		// return true if diffAngle matches notion of "forwards"
		if (cmd.rightmove < 0)
		{
			// fullbeat / halfbeat / invert (holding +moveleft)
			if (diffAngle >= 0)
			{
				return true;
			}
		}
		else if (cmd.rightmove > 0)
		{
			// fullbeat / halfbeat / invert (holding +moveright)
			if (diffAngle < 0)
			{
				return true;
			}
		}
		else if (cmd.forwardmove != 0)
		{
			// nobeat
			if (diffAngle >= 0)
			{
				return true;
			}
		}

		return false;
	}

	float CGaz::getOptAngle(const playerState_t& ps, pmove_t* pm)
	{
		// get player speed
		const float speed = VectorLength2(ps.velocity);

		// get sprint scale
		const float scale = PmoveUtils::PM_SprintScale(&ps);

		// get usercmd
		const int8_t ucmdScale     = ps.stats[STAT_USERCMD_BUTTONS] & (BUTTON_WALKING << 8) ? CMDSCALE_WALK : CMDSCALE_DEFAULT;
		const usercmd_t  cmd       = PmoveUtils::getUserCmd(ps, ucmdScale);

		// no meaningful value if speed lower than ground speed or no user input
		if (speed < ps.speed * scale || (cmd.forwardmove == 0 && cmd.rightmove == 0))
		{
			return 0;
		}

		// get player yaw
		const float& yaw = ps.viewangles[YAW];

		// determine whether strafing "forwards"
		const bool forwards = strafingForwards(ps, pm);

		// get accel defined by physics
		const float accel = ps.speed * pm->pmext->frametime;

		// get variables associated with optimal angle
		const float velAngle   = RAD2DEG(std::atan2(ps.velocity[1], ps.velocity[0]));
		const float accelAngle = RAD2DEG(std::atan2(-cmd.rightmove, cmd.forwardmove));
		float       perAngle   = RAD2DEG(std::acos((ps.speed - accel) / speed * scale));
		if (!forwards)
		{
			perAngle *= -1;
		}

		// shift yaw to optimal angle for all strafe styles
		float opt = yaw;
		if (cmd.rightmove < 0)
		{
			// fullbeat / halfbeat / invert (holding +moveleft)
			opt -= AngleDelta(yaw + accelAngle, velAngle + perAngle);
		}
		else if (cmd.rightmove > 0)
		{
			// fullbeat / halfbeat / invert (holding +moveright)
			opt -= AngleDelta(yaw + accelAngle, velAngle - perAngle);
		}
		else if (cmd.forwardmove != 0)
		{
			// nobeat
			opt = velAngle + perAngle;
		}

		// return minimum angle for which you still gain the highest accel
		return AngleNormalize180(opt);
	}

	bool CGaz::canSkipDraw() const
	{
		if (!etj_drawCGaz.integer)
		{
			return true;
		}

		if (VectorLengthSquared2(ps->velocity) == 0)
		{
			return true;
		}

		if (ps->persistant[PERS_TEAM] == TEAM_SPECTATOR || ps->pm_type == PM_NOCLIP || ps->pm_type == PM_DEAD)
		{
			return true;
		}

		if (cg.showScores || cg.scoreFadeTime + FADE_TIME > cg.time)
		{
			return true;
		}

		if ((cg.zoomedBinoc || cg.zoomedScope) && !cg.renderingThirdPerson)
		{
			return true;
		}

		if (BG_PlayerMounted(ps->eFlags) || ps->weapon == WP_MOBILE_MG42_SET || ps->weapon == WP_MORTAR_SET)
		{
			return true;
		}

		// water and ladder movement are not important
		// since speed is capped anyway
		if (pm->pmext->waterlevel > 1 || pm->pmext->ladder)
		{
			return true;
		}

		return false;
	}
}
