#include "etj_entity_events_handler.h"
#include "etj_maxspeed.h"
#include "cg_local.h"
#include <string>
#include "etj_utilities.h"
#include "etj_cvar_update_handler.h"

ETJump::DisplayMaxSpeed::DisplayMaxSpeed(EntityEventsHandler* entityEventsHandler) :
	_entityEventsHandler{ entityEventsHandler }
{
	if (!entityEventsHandler)
	{
		CG_Error("DisplayMaxSpeed: clientCommandsHandler is null.\n");
		return;
	}

	entityEventsHandler->subscribe(EV_LOAD_TELEPORT, [&](centity_t *cent)
	{
		if (cg.snap->ps.clientNum != cg.clientNum)
		{
			_maxSpeed = 0;
			return;
		}

		_displayMaxSpeed = _maxSpeed;
		_maxSpeed = 0;
		_animationStartTime = cg.time;
	});

	parseColor(cg_speedColor.string, _color);
	cvarUpdateHandler->subscribe(&cg_speedColor, [&](const vmCvar_t *cvar)
	{
		parseColor(cg_speedColor.string, _color);
	});
	cvarUpdateHandler->subscribe(&cg_speedAlpha, [&](const vmCvar_t *cvar)
	{
		parseColor(cg_speedColor.string, _color);
	});
}

ETJump::DisplayMaxSpeed::~DisplayMaxSpeed()
{
	_entityEventsHandler->unsubcribe(EV_LOAD_TELEPORT);
}

void ETJump::DisplayMaxSpeed::parseColor(const std::string& color, vec4_t& out)
{
	parseColorString(color, out);
	out[3] *= cg_speedAlpha.value;
}

void ETJump::DisplayMaxSpeed::beforeRender()
{
	auto speed = sqrt(cg.snap->ps.velocity[0] * cg.snap->ps.velocity[0] + cg.snap->ps.velocity[1] * cg.snap->ps.velocity[1]);

	if (speed >= _maxSpeed)
	{
		_maxSpeed = speed;
	}
}

void ETJump::DisplayMaxSpeed::render() const
{
	if (!etj_drawMaxSpeed.integer)
	{
		return;
	}

	vec4_t color;
	auto fade = CG_FadeAlpha(_animationStartTime, etj_maxSpeedDuration.integer);
	Vector4Copy(_color, color);
	color[3] *= fade;

	auto sizex = 0.1f, sizey = 0.1f;
	sizex *= cg_speedSizeX.integer;
	sizey *= cg_speedSizeY.integer;

	auto str = va("%0.f", _displayMaxSpeed);
	auto w = CG_Text_Width_Ext(str, sizex, 0, &cgs.media.limboFont2) / 2;
	
	auto x = etj_maxSpeedX.value;
	auto y = etj_maxSpeedY.value;

	ETJump_AdjustPosition(&x);

	auto style = ITEM_TEXTSTYLE_NORMAL;

	if (etj_speedShadow.integer > 0) {
		style = ITEM_TEXTSTYLE_SHADOWED;
	}

	CG_Text_Paint_Ext(x - w, y, sizex, sizey, color, str, 0, 0, style, &cgs.media.limboFont1);
}