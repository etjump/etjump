/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <zero@etjump.com>
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

#include "etj_speed_drawable.h"
#include "etj_utilities.h"
#include "etj_cvar_update_handler.h"
#include "etj_client_commands_handler.h"
#include <string>
#include "../game/etj_string_utilities.h"

constexpr int   ACCEL_COLOR_SMOOTHING_TIME = 250;
constexpr float ACCEL_FOR_SOLID_COLOR      = 100;

ETJump::DisplaySpeed::DisplaySpeed()
{
	parseColor(cg_speedColor.string, _color);
	checkShadow();
	startListeners();
}

ETJump::DisplaySpeed::~DisplaySpeed() {}

void ETJump::DisplaySpeed::parseColor(const std::string& color, vec4_t& out)
{
	parseColorString(color, out);
	out[3] *= cg_speedAlpha.value;
}

void ETJump::DisplaySpeed::startListeners()
{
	cvarUpdateHandler->subscribe(&cg_speedColor, [&](const vmCvar_t *cvar)
	{
		parseColor(cg_speedColor.string, _color);
	});

	cvarUpdateHandler->subscribe(&cg_speedAlpha, [&](const vmCvar_t *cvar)
	{
		parseColor(cg_speedColor.string, _color);
	});

	cvarUpdateHandler->subscribe(&etj_speedShadow, [&](const vmCvar_t *cvar)
	{
		checkShadow();
	});

	consoleCommandsHandler->subscribe("resetmaxspeed", [&](const std::vector<std::string> &args)
	{
		resetMaxSpeed();
	});
}

void ETJump::DisplaySpeed::beforeRender()
{
	auto speed = sqrt(cg.predictedPlayerState.velocity[0] * cg.predictedPlayerState.velocity[0] + cg.predictedPlayerState.velocity[1] * cg.predictedPlayerState.velocity[1]);
	_maxSpeed = speed > _maxSpeed ? speed : _maxSpeed;

	if (etj_speedColorUsesAccel.integer)
	{
		_storedSpeeds.push_back({ cg.time, speed });
	
		popOldStoredSpeeds();
	}
	else if (!_storedSpeeds.empty())
	{
		_storedSpeeds.clear();
	}
}

void ETJump::DisplaySpeed::resetMaxSpeed()
{
	_maxSpeed = 0;
	cg.resetmaxspeed = qtrue; // fix me
}

void ETJump::DisplaySpeed::checkShadow()
{
	_shouldDrawShadow = etj_speedShadow.integer > 0 ? true : false;
}

void ETJump::DisplaySpeed::render() const
{
	if (canSkipDraw())
	{
		return;
	}

	float size = 0.1f * etj_speedSize.integer;
	float x = cg_speedX.integer;
	float y = cg_speedY.integer;
	ETJump_AdjustPosition(&x);

	auto status = getStatus();
	float w = CG_Text_Width_Ext(status.c_str(), size, 0, &cgs.media.limboFont2) / 2;
	if (cg_drawSpeed2.integer == 8)
	{
		w = 0;
	}

	int style = _shouldDrawShadow ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;
	
	// fix me
	vec4_t color;

	if (!etj_speedColorUsesAccel.integer)
	{
		Vector4Copy(_color, color);
	}
	else
	{
		float accel = calcAvgAccel();
		float *accelColor = colorGreen;

		if (accel < 0)
		{
			accelColor = colorRed;
			accel = -accel;
		}

		float frac = accel / ACCEL_FOR_SOLID_COLOR;
		frac = std::min(frac, 1.f);
		
		LerpColor(colorWhite, accelColor, color, frac);
	}

	CG_Text_Paint_Ext(x - w, y, size, size, color, status.c_str(), 0, 0, style, &cgs.media.limboFont1);
}

std::string ETJump::DisplaySpeed::getStatus() const
{
	float speed = sqrt(cg.predictedPlayerState.velocity[0] * cg.predictedPlayerState.velocity[0] + cg.predictedPlayerState.velocity[1] * cg.predictedPlayerState.velocity[1]);
	switch (cg_drawSpeed2.integer)
	{
	case 2: return stringFormat("%.0f %.0f", speed, _maxSpeed);
	case 3: return stringFormat("%.0f ^z%.0f", speed, _maxSpeed);
	case 4: return stringFormat("%.0f (%.0f)", speed, _maxSpeed);
	case 5: return stringFormat("%.0f ^z(%.0f)", speed, _maxSpeed);
	case 6: return stringFormat("%.0f ^z[%.0f]", speed, _maxSpeed);
	case 7: return stringFormat("%.0f | %.0f", speed, _maxSpeed);
	case 8: return stringFormat("Speed: %.0f", speed);
	// tens
	case 9: return stringFormat("%02i", static_cast<int>(speed) / 10 % 10 * 10);
	default: return stringFormat("%.0f", speed);
	}
}

bool ETJump::DisplaySpeed::canSkipDraw() const
{
	return !cg_drawSpeed2.integer || cg.showScores || cg.scoreFadeTime + FADE_TIME > cg.time;
}

void ETJump::DisplaySpeed::popOldStoredSpeeds()
{
	do
	{
		auto& front = _storedSpeeds.front();

		if (cg.time - front.time > ACCEL_COLOR_SMOOTHING_TIME)
		{	// too old
			_storedSpeeds.pop_front();
			continue;
		}
		else if (cg.time < front.time)
		{	// we went back in time!
			_storedSpeeds.pop_front();
			continue;
		}
	} while (false);
}

float ETJump::DisplaySpeed::calcAvgAccel() const
{
	if (_storedSpeeds.size() < 2) 
	{	// need 2 speed points to compute acceleration
		return 0;
	}

	float totalSpeedDelta = 0;
	auto iter = _storedSpeeds.begin();
	for (auto prevIter = iter++; iter != _storedSpeeds.end(); prevIter = iter++)
	{
		totalSpeedDelta += iter->speed - prevIter->speed;
	}

	float timeDeltaMs = _storedSpeeds.back().time - _storedSpeeds.front().time;
	float accel = totalSpeedDelta / (timeDeltaMs / 1000.f);
	
	return accel;
}
