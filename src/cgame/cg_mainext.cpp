#include "cg_mainext.h"
#include "cg_timerun.h"
#include "etj_timerun_view.h"

#include <string>
#include <memory>

static std::unique_ptr<Timerun> timerun;
static std::unique_ptr<ETJump::TimerunView> timerunView;

/**
 * Initializes the CPP side of client
 */
void InitGame()
{
	ETJump_ClearDrawables();
	timerun = std::unique_ptr<Timerun>(new Timerun(cg.clientNum));
	timerunView = std::unique_ptr<ETJump::TimerunView>(new ETJump::TimerunView());
}

/**
 * Extended CG_ServerCommand function. Checks whether server
 * sent command matches to any defined here. If no match is found
 * returns false
 * @return qboolean Whether a match was found or not
 */
qboolean CG_ServerCommandExt(const char *cmd)
{
	std::string command = cmd != nullptr ? cmd : "";

	// timerun_start runStartTime{integer} runName{string}
	if (command == "timerun_start")
	{
		auto        startTime      = atoi(CG_Argv(1));
		std::string runName        = CG_Argv(2);
		auto        previousRecord = atoi(CG_Argv(3));
		timerun->startTimerun(runName, startTime, previousRecord);
		return qtrue;
	}
	// timerun_start_spec clientNum{integer} runStartTime{integer} runName{string}
	if (command == "timerun_start_spec")
	{
		if (cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR)
		{
			return qtrue;
		}

		auto        clientNum      = atoi(CG_Argv(1));
		auto        runStartTime   = atoi(CG_Argv(2));
		std::string runName        = CG_Argv(3);
		auto        previousRecord = atoi(CG_Argv(4));

		timerun->startSpectatorTimerun(clientNum, runName, runStartTime, previousRecord);

		return qtrue;
	}
	if (command == "timerun_interrupt")
	{
		timerun->interrupt();
		return qtrue;
	}
	// timerun_stop completionTime{integer}
	if (command == "timerun_stop")
	{
		auto completionTime = atoi(CG_Argv(1));
		timerun->stopTimerun(completionTime);
		return qtrue;
	}
	// timerun_stop_spec clientNum{integer} completionTime{integer} runName{string}
	if (command == "timerun_stop_spec")
	{
		if (cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR)
		{
			return qtrue;
		}

		auto        clientNum      = atoi(CG_Argv(1));
		auto        completionTime = atoi(CG_Argv(2));
		std::string runName        = CG_Argv(3);

		timerun->stopSpectatorTimerun(clientNum, completionTime, runName);

		return qtrue;
	}
	if (command == "record")
	{
		auto        clientNum      = atoi(CG_Argv(1));
		std::string runName        = CG_Argv(2);
		auto        completionTime = atoi(CG_Argv(3));

		timerun->record(clientNum, runName, completionTime);
		return qtrue;
	}
	if (command == "completion")
	{
		auto        clientNum      = atoi(CG_Argv(1));
		std::string runName        = CG_Argv(2);
		auto        completionTime = atoi(CG_Argv(3));

		timerun->completion(clientNum, runName, completionTime);

		return qtrue;
	}
	if (command == "timerun")
	{
		return timerunView->parseServerCommand() ? qtrue : qfalse;
	}

	return qfalse;
}