#include "etj_entity_events_handler.h"
#include "etj_maxspeed.h"
#include "cg_local.h"
#include <string>

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
}

ETJump::DisplayMaxSpeed::~DisplayMaxSpeed()
{
	_entityEventsHandler->unsubcribe(EV_LOAD_TELEPORT);
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
	Vector4Set(color, cg.speedColor[0], cg.speedColor[1], cg.speedColor[2], fade);

	auto sizex = 0.1f, sizey = 0.1f;
	sizex *= cg_speedSizeX.integer;
	sizey *= cg_speedSizeY.integer;

	auto str = va("%0.f", _displayMaxSpeed);
	auto w = CG_Text_Width_Ext(str, sizex, 0, &cgs.media.limboFont2) / 2;
	
	auto x = etj_maxSpeedX.value;
	auto y = etj_maxSpeedY.value;

	CG_AdjustPosX(&x);

	auto style = ITEM_TEXTSTYLE_NORMAL;

	if (etj_speedShadow.integer) {
		style = ITEM_TEXTSTYLE_SHADOWED;
	}

	CG_Text_Paint_Ext(x - w, y, sizex, sizey, color, str, 0, 0, style, &cgs.media.limboFont1);
}