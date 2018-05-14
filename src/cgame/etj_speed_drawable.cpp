#include "etj_speed_drawable.h"
#include "etj_utilities.h"
#include "etj_cvar_update_handler.h"
#include "etj_client_commands_handler.h"
#include <string>
#include "../game/etj_string_utilities.h"

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

	float sizex = 0.1f * cg_speedSizeX.integer;
	float sizey = 0.1f * cg_speedSizeY.integer;
	float x = cg_speedX.integer;
	float y = cg_speedY.integer;
	ETJump_AdjustPosition(&x);

	auto status = getStatus();
	float w = CG_Text_Width_Ext(status.c_str(), sizex, 0, &cgs.media.limboFont2) / 2;
	if (cg_drawSpeed2.integer == 8)
	{
		w = 0;
	}

	int style = _shouldDrawShadow ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;
	
	// fix me
	vec4_t color;
	Vector4Copy(_color, color);

	CG_Text_Paint_Ext(x - w, y, sizex, sizey, color, status.c_str(), 0, 0, style, &cgs.media.limboFont1);
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
	return !cg_drawSpeed2.integer || cg.showScores;
}
