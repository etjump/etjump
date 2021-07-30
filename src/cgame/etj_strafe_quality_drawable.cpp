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

#include "etj_strafe_quality_drawable.h"
#include "etj_cvar_update_handler.h"
#include "etj_client_commands_handler.h"
#include "etj_utilities.h"
#include "etj_snaphud.h"
#include "etj_cgaz.h"

namespace ETJump {
	StrafeQuality::StrafeQuality()
	{
		parseColor();
		startListeners();
	}

	void StrafeQuality::startListeners()
	{
		// only subscribe to cvars whose parsing would be inefficient each frame
		cvarUpdateHandler->subscribe(&etj_strafeQualityColor,
			[&](const vmCvar_t *cvar) {
				parseColor();
			});

		consoleCommandsHandler->subscribe(
			"resetStrafeQuality",
			[&](const std::vector<std::string> &args) {
				resetStrafeQuality();
			});
	}

	void StrafeQuality::parseColor()
	{
		parseColorString(etj_strafeQualityColor.string, _color);
	}

	void StrafeQuality::resetStrafeQuality()
	{
		// reset underlying vars, but not what is rendered until actually needed
		_totalFrames = _goodFrames = _oldSpeed = 0;
	}

	void StrafeQuality::beforeRender()
	{
		// get player state
		const playerState_t &ps = *getValidPlayerState();

		// get usercmd
		constexpr int8_t ucmdScale = 127;
		const usercmd_t  cmd       = getUsercmd(ps, ucmdScale);

		// update team
		if (_team != ps.persistant[PERS_TEAM])
		{
			_team = ps.persistant[PERS_TEAM];
			// reset strafe quality upon team change
			// note: not handled by consoleCommandsHandler because _team is needed in
			// render() either ways
			resetStrafeQuality();
		}

		// don't count frames if not strafing
		if (cmd.forwardmove == 0 && cmd.rightmove == 0)
		{
			return;
		}

		// don't count frames if not in air and not on ice
		if (ps.groundEntityNum != ENTITYNUM_NONE &&
				!(cg_pmove.groundTrace.surfaceFlags & SURF_SLICK))
		{
			return;
		}

		// count this frame towards strafe quality
		++_totalFrames;

		// check whether user input is good
		const float speed = std::sqrt(ps.velocity[0] * ps.velocity[0] +
			ps.velocity[1] * ps.velocity[1]);
		const float scale = PM_CalcScale(ps);
		if (speed < ps.speed * scale)
		{
			// possibly good frame under ground speed if speed increased
			// note that without speed increased you could go forward in a
			// "ps.speed - 1" angle endlessly because of velocity snapping
			if (speed > _oldSpeed)
			{
				// good frame if no upmove and either only forwardmove or only rightmove
				if (cmd.upmove == 0 && ((cmd.forwardmove != 0 && cmd.rightmove == 0) ||
					(cmd.forwardmove == 0 && cmd.rightmove != 0)))
				{
					++_goodFrames;
				}
				// otherwise only half as good because not optimal
				else
				{
					_goodFrames += 0.5;
				}
			}
		}
		else
		{
			// good frame above ground speed if no upmove and in main accel zone
			if (cmd.upmove == 0 && inMainAccelZone(ps))
			{
				++_goodFrames;
			}
			// or if speed increased, half as good because not optimal
			else if (speed > _oldSpeed)
			{
				_goodFrames += 0.5;
			}
		}

		// update strafe quality
		_strafeQuality = 100 * _goodFrames / _totalFrames;

		// update vars for next frame
		_oldSpeed = speed;
	}

	void StrafeQuality::render() const
	{
		// check whether to skip render
		if (!etj_drawStrafeQuality.integer || cg.showScores ||
				cg.scoreFadeTime + FADE_TIME > cg.time || _team == TEAM_SPECTATOR)
		{
			return;
		}

		// get coordinates and size
		float       x    = etj_strafeQualityX.value;
		float       y    = etj_strafeQualityY.value;
		const float size = 0.1f * etj_strafeQualitySize.value;
		ETJump_AdjustPosition(&x);

		// get hud text
		std::string str;
		std::string qualityStr = std::to_string(_strafeQuality);
		qualityStr.resize(_digits + 1);
		switch (etj_strafeQualityStyle.integer)
		{
		case 1:
			// percent
			str = qualityStr + "%";
			break;
		case 2:
			// number
			str = qualityStr;
			break;
		default:
			// full
			str = "Strafe Quality: " + qualityStr + "%";
			break;
		}

		// get hud shadow
		const auto textStyle =
			(etj_strafeQualityShadow.integer != 0 ? ITEM_TEXTSTYLE_SHADOWED
			: ITEM_TEXTSTYLE_NORMAL);

		// draw quality on screen
		CG_Text_Paint_Ext(x, y, size, size, _color, str, 0, 0, textStyle,
			&cgs.media.limboFont1);
	}
} // namespace ETJump
