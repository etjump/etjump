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

#include "etj_jump_speeds.h"
#include "etj_utilities.h"

namespace ETJump
{
	void DrawJumpSpeeds()
	{
		playerState_t* ps = getValidPlayerState();
		float offX = 0.0f;
		float offY = 0.0f;
		float x1 = 6 + etj_jumpSpeedsX.value;
		float x2 = 6 + 30 + etj_jumpSpeedsX.value;
		float y1 = 240 + etj_jumpSpeedsY.value;
		float y2 = 240 + 12 +etj_jumpSpeedsY.value;
		auto textStyle = etj_jumpSpeedsShadow.integer ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;
		vec4_t color;
		bool vertical = !etj_jumpSpeedsStyle.integer;

		if (!etj_drawJumpSpeeds.integer)
		{
			return;
		}

		if (ps->persistant[PERS_TEAM] == TEAM_SPECTATOR)
		{
			return;
		}

		if (cg.zoomedBinoc || cg.zoomedScope)
		{
			return;
		}

		// draw the label text first
		x1 = ETJump_AdjustPosition(x1);
		x2 = ETJump_AdjustPosition(x2);
		parseColorString(etj_jumpSpeedsColor.string, color);
		DrawString(x1, y1, 0.2f, 0.2f, color, qfalse, "Jump Speeds:", 0, textStyle);

		// adjust x or y depending on style chosen
		vertical ? y1 += 12 : x1 = DrawStringWidth("Jump Speeds: ", 0.2f) + 5;

		for (auto i = 0; i < jumpSpeedHistory.size(); i++)
		{
			auto jumpSpeed = std::to_string(jumpSpeedHistory.at(i));
			if (etj_jumpSpeedsShowDiff.integer)
			{
				AdjustColors(i, &color);
			}
			if (vertical)
			{
				// first column
				if (i < 5)
				{
					DrawString(x1, y1, 0.2f, 0.2f, color, qfalse, jumpSpeed.c_str(), 0, textStyle);
					y1 += 12;
				}
				// second column
				else
				{
					DrawString(x2, y2, 0.2f, 0.2f, color, qfalse, jumpSpeed.c_str(), 0, textStyle);
					y2 += 12;
				}
			}
			// horizontal list
			else
			{
				DrawString(x1, y1, 0.2f, 0.2f, color, qfalse, jumpSpeed.c_str(), 0, textStyle);
				x1 += 30;
			}
		}
	}

	void UpdateJumpSpeeds()
	{
		playerState_t* ps = getValidPlayerState();
		// queue reset if last update was on different team
		if (team != ps->persistant[PERS_TEAM])
		{
			QueueJumpSpeedsReset();
		}
		// if reset is queued, do that before we start storing new jump speeds
		if (resetQueued)
		{
			ResetJumpSpeeds();
			resetQueued = false;
		}

		team = ps->persistant[PERS_TEAM];
		jumpSpeedHistory.push_back(ps->persistant[PERS_JUMP_SPEED]);
		// we only want to keep last 10 jumps, so remove first value if we go over that
		if (jumpSpeedHistory.size() > MAX_JUMPS)
		{
			// store the deleted speed so we can do diff comparisons later
			lastDeletedSpeed = jumpSpeedHistory.front();
			jumpSpeedHistory.erase(jumpSpeedHistory.begin());
			jumpSpeedDeleted = true;
		}
	}

	void QueueJumpSpeedsReset()
	{
		resetQueued = true;
	}

	void ResetJumpSpeeds()
	{
		jumpSpeedHistory.clear();
		lastDeletedSpeed = 0;
	}

	void AdjustColors(int jumpNum, vec4_t* color)
	{
		// equal/first jump color comes from etj_jumpSpeedsColor
		vec4_t fasterColor;
		vec4_t slowerColor;
		parseColorString(etj_jumpSpeedsFasterColor.string, fasterColor);
		parseColorString(etj_jumpSpeedsSlowerColor.string, slowerColor);
		auto currentJumpSpeed = jumpSpeedHistory[jumpNum];

		// when checking for the first jump speed, we need to make sure
		// we don't do comparison of (first jump - 1). Instead, compare against
		// previously deleted jump speed, but only if the history is full
		// and we start deleting jump speeds (11th jump)
		if (jumpNum == 0)
		{
			if (jumpSpeedHistory.size() == MAX_JUMPS && jumpSpeedDeleted)
			{
				// faster than previous jump
				if (currentJumpSpeed > lastDeletedSpeed)
				{
					Vector4Copy(fasterColor, *color);
				}
				// slower than previous jump
				else if (currentJumpSpeed < lastDeletedSpeed)
				{
					Vector4Copy(slowerColor, *color);
				}
			}
			// history not full
			else
			{
				jumpSpeedDeleted = false;
			}
		}
		else
		{
			auto previousJumpSpeed = jumpSpeedHistory[jumpNum - 1];
			// faster than previous jump
			if (currentJumpSpeed > previousJumpSpeed)
			{
				Vector4Copy(fasterColor, *color);
			}
			// slower than previous jump
			else if (previousJumpSpeed > currentJumpSpeed)
			{
				Vector4Copy(slowerColor, *color);
			}
		}
	}

	static playerState_t* getValidPlayerState()
	{
		return (cg.snap->ps.clientNum != cg.clientNum)
			// spectating
			? &cg.snap->ps
			// playing
			: &cg.predictedPlayerState;
	}
}
