#include <string>
#include <boost/format.hpp>

extern "C" {
#include "cg_local.h"
}

#include "etj_timerun_view.hpp"

void ETJump::TimerunView::start()
{
	auto clientNum = atoi(CG_Argv(2));
	_playersTimerunInformation[clientNum].startTime = atoi(CG_Argv(3));
	_playersTimerunInformation[clientNum].runName = CG_Argv(4);
	_playersTimerunInformation[clientNum].previousRecord = atoi(CG_Argv(5));
	_playersTimerunInformation[clientNum].running = true;
}

void ETJump::TimerunView::stop()
{
	auto clientNum = atoi(CG_Argv(2));
	_playersTimerunInformation[clientNum].completionTime = atoi(CG_Argv(3));
	_playersTimerunInformation[clientNum].running = false;
}

void ETJump::TimerunView::interrupt(PlayerTimerunInformation& playerTimerunInformation)
{
	playerTimerunInformation.running = false;
	playerTimerunInformation.runName = "";
	playerTimerunInformation.completionTime = -1;
	playerTimerunInformation.previousRecord = 0;
	playerTimerunInformation.startTime = 0;
}

void ETJump::TimerunView::interrupt()
{
	auto clientNum = atoi(CG_Argv(2));
	interrupt(_playersTimerunInformation[clientNum]);
}

const ETJump::PlayerTimerunInformation* ETJump::TimerunView::currentRun() const
{
	return &_playersTimerunInformation[cg.snap->ps.clientNum];
}

void ETJump::TimerunView::draw()
{
	if (player_drawRunTimer.integer == 0 || !cg.hasTimerun)
	{
		return;
	}

	auto run = currentRun();


	auto startTime = run->startTime;
	auto millis = 0;

	if (run->running)
	{
		millis = cg.time - startTime;
	}
	else
	{
		millis = run->completionTime;
		if (millis == -1)
		{
			millis = 0;
		}
	}

	vec4_t *color = &colorWhite;
	vec4_t incolor;
	int range = 10000; // 10s
	int style = ITEM_TEXTSTYLE_NORMAL;

	if (etj_runTimerShadow.integer) {
		style = ITEM_TEXTSTYLE_SHADOWED;
	}

	if (run->previousRecord > 0)
	{
		if (millis > run->previousRecord)
		{
			color = &colorRed;
		}
		else if ( millis + range >= run->previousRecord ) {
			CG_InterpolateColors(&incolor, &colorWhite, &colorRed, run->previousRecord - range, run->previousRecord, millis);
			color = &incolor;
		}
	}

	auto minutes = millis / 60000;
	millis -= minutes * 60000;
	auto seconds = millis / 1000;
	millis -= seconds * 1000;

	auto text = (boost::format("%02d:%02d:%03d")
		% minutes
		% seconds
		% millis).str();

	auto textWidth = CG_Text_Width_Ext(text.c_str(), 0.3, 0, &cgs.media.limboFont1) / 2;
	auto x = player_runTimerX.integer;
	auto y = player_runTimerY.integer;

	CG_Text_Paint_Ext(x - textWidth, y, 0.3, 0.3, *color, text.c_str(), 0, 0, style, &cgs.media.limboFont1);
}

bool ETJump::TimerunView::parseServerCommand()
{
	auto argc = trap_Argc();

	if (argc == 1)
	{
		return false;
	}

	char cmd[MAX_TOKEN_CHARS]{};
	trap_Argv(1, cmd, sizeof(cmd));

	if (!Q_stricmp(cmd, "start"))
	{
		start();
	} else if (!Q_stricmp(cmd, "stop"))
	{
		stop();
	}  else if (!Q_stricmp(cmd, "interrupt"))
	{
		interrupt();
	} else
	{
		return false;
	}

	return true;
}
