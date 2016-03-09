#include <string>
#include <boost/format.hpp>

extern "C" {
#include "cg_local.h"
}

#include "etj_timerun_view.hpp"

void ETJump::TimerunView::start()
{
	start(_ownTimerunInformation);
}

void ETJump::TimerunView::stop()
{
	stop(_ownTimerunInformation);
}

void ETJump::TimerunView::start(PlayerTimerunInformation& playerTimerunInformation)
{
	playerTimerunInformation.startTime = atoi(CG_Argv(1));;
	playerTimerunInformation.runName = CG_Argv(2);
	playerTimerunInformation.previousRecord = atoi(CG_Argv(3));
	playerTimerunInformation.running = true;
}

void ETJump::TimerunView::interrupt(PlayerTimerunInformation& playerTimerunInformation)
{
	playerTimerunInformation.running = false;
	playerTimerunInformation.runName = "";
	playerTimerunInformation.completionTime = -1;
	playerTimerunInformation.previousRecord = 0;
	playerTimerunInformation.startTime = 0;
}

void ETJump::TimerunView::stop(PlayerTimerunInformation& playerTimerunInformation)
{
	playerTimerunInformation.completionTime = atoi(CG_Argv(1));
	playerTimerunInformation.running = false;
}

void ETJump::TimerunView::interrupt()
{
	interrupt(_ownTimerunInformation);
}

void ETJump::TimerunView::otherInterrupt()
{
	auto clientNum = atoi(CG_Argv(1));
	interrupt(_playersTimerunInformation[clientNum]);
}

void ETJump::TimerunView::otherStart()
{
	auto clientNum = atoi(CG_Argv(1));
	start(_playersTimerunInformation[clientNum]);
}

void ETJump::TimerunView::otherStop()
{
	auto clientNum = atoi(CG_Argv(1));
	stop(_playersTimerunInformation[clientNum]);
}

const ETJump::PlayerTimerunInformation* ETJump::TimerunView::currentRun() const
{
	auto clientNum = cg.snap->ps.clientNum;
	return clientNum == cg.clientNum ? &_ownTimerunInformation : &_playersTimerunInformation[clientNum];
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
	}

	vec4_t *color = &colorWhite;

	if (run->previousRecord > 0)
	{
		if (millis > run->previousRecord)
		{
			color = &colorRed;
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

	CG_Text_Paint_Ext(x - textWidth, y, 0.3, 0.3, *color, text.c_str(), 0, 0, 0, &cgs.media.limboFont1);
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
	} else if (!Q_stricmp(cmd, "other_start"))
	{
		otherStart();
	} else if (!Q_stricmp(cmd, "other_stop"))
	{
		otherStop();
	} else if (!Q_stricmp(cmd, "interrupt"))
	{
		interrupt();
	}
	else if (!Q_stricmp(cmd, "other_interrupt")) {
		otherInterrupt();
	} else
	{
		return false;
	}

	return true;
}
