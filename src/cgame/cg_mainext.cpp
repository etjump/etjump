#include "etj_trickjump_lines.hpp"
#include "cg_mainext.h"
#include "cg_timerun.h"
#include "etj_timerun_view.h"

#include <string>
#include <memory>


static std::unique_ptr<Timerun> timerun;
static std::unique_ptr<ETJump::TimerunView> timerunView;

static std::unique_ptr<TrickjumpLines> trickjumpLines;
static int nextNearest = 0;

/**
 * Initializes the CPP side of client
 */
void InitGame()
{
	ETJump_ClearDrawables();
	timerun = std::unique_ptr<Timerun>(new Timerun(cg.clientNum));
	timerunView = std::unique_ptr<ETJump::TimerunView>(new ETJump::TimerunView());
	// restores timerun after vid_restart (if required)
	trap_SendClientCommand("timerun_status");

	trickjumpLines = std::unique_ptr<TrickjumpLines>(new TrickjumpLines);
	
	// Check if load TJL on connection is enable
	if (etj_tjlAlwaysLoadTJL.integer == 1)
	{		
		CG_Printf("All mapper Trickjump lines will be loaded due to your cvar : etj_tjlAlwaysLoadTJL. \n");
		trickjumpLines->loadRoutes(nullptr);
	}

	if (etj_tjlEnableLine.integer == 1)
	{
		trickjumpLines->toggleRoutes(true);
	}
	else
	{
		trickjumpLines->toggleRoutes(false);
	}

	if (etj_tjlEnableMarker.integer == 1)
	{
		trickjumpLines->toggleMarker(true);
	}
	else
	{
		trickjumpLines->toggleMarker(false);
	}
}

/**
 * Extended CG_ServerCommand function. Checks whether server
 * sent command matches to any defined here. If no match is found
 * returns false
 * @return qboolean Whether a match was found or not
 */
qboolean CG_ServerCommandExt(const char *cmd)
{
	const std::string command = cmd != nullptr ? cmd : "";

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

	if (command == "tjl_displaybyname")
	{
		return CG_displaybyname();
	}

	if (command == "tjl_displaybynumber")
	{
		return CG_displaybynumber();
	}

	return qfalse;
}


/**
* Checks if the command exists and calls the handler
* @param cmd The command to be matched
* @returns qboolean Whether a match was found
*/
qboolean CG_ConsoleCommandExt(const char *cmd)
{
	const std::string command = cmd ? cmd : "";

	if (command == "tjl_displaybyname")
	{
		return CG_displaybyname();
	}

	if (command == "tjl_displaybynumber")
	{
		return CG_displaybynumber();
	}

	if (command == "tjl_clearrender")
	{
		trickjumpLines->setCurrentRouteToRender(-1);
		return qtrue;
	}

	// TODO: could just make an array out of this and go thru it
	if (command == "tjl_record")
	{
		const auto argc = trap_Argc();
		if (argc == 1)
		{
			trickjumpLines->record(nullptr);
		}
		else
		{
			auto name = CG_Argv(1);
			trickjumpLines->record(name);
		}
		return qtrue;
	}

	if (command == "tjl_stoprecord")
	{
		trickjumpLines->stopRecord();
		return qtrue;
	}

	if (command == "tjl_listroute")
	{
		trickjumpLines->listRoutes();
		return qtrue;
	}


	if (command == "tjl_displaynearestroute")
	{
		trickjumpLines->displayNearestRoutes();
		return qtrue;
	}

	if (command == "tjl_renameroute")
	{
		const auto argc = trap_Argc();

		if (argc > 2)
		{
			const std::string name = CG_Argv(1);
			const std::string name2 = CG_Argv(2);
			trickjumpLines->renameRoute(name.c_str(), name2.c_str());
		}
		else
		{
			trickjumpLines->renameRoute(nullptr, nullptr);
		}
		return qtrue;
	}

	if (command == "tjl_saveroute")
	{
		const auto argc = trap_Argc();
		if (argc > 1)
		{
			const auto name = CG_Argv(1);
			trickjumpLines->saveRoutes(name);
			return qtrue;
		}
		else
		{
			CG_Printf("Please provide a name to save your TJL. (without .tjl extension). \n");
			return qfalse;
		}
	}

	if (command == "tjl_loadroute")
	{
		const auto argc = trap_Argc();
		if (argc > 1)
		{
			const auto name = CG_Argv(1);
			trickjumpLines->loadRoutes(name);
		}
		else
		{
			trickjumpLines->loadRoutes(nullptr);
		}
		return qtrue;
	}

	if (command == "tjl_deleteroute")
	{
		const auto argc = trap_Argc();
		if (argc > 1)
		{
			const auto name = CG_Argv(1);
			trickjumpLines->deleteRoute(name);
		}
		else
		{
			trickjumpLines->deleteRoute(nullptr);
		}
		return qtrue;
	}

	if (command == "tjl_overwriterecording")
	{
		const auto argc = trap_Argc();
		if (argc == 1)
		{
			trickjumpLines->overwriteRecording(nullptr);
		}
		else
		{
			const auto name = CG_Argv(1);
			trickjumpLines->overwriteRecording(name);
		}
		return qtrue;
	}

	if (command == "tjl_enableline")
	{
		const auto argc = trap_Argc();
		if (argc == 1)
		{
			CG_Printf("Please add 0 or 1 as argument to enable or disable line.\n");
			return qfalse;
		}
		else
		{
			const std::string state = CG_Argv(1);
			if (state == "0")
			{
				trickjumpLines->toggleRoutes(false);
			}
			else
			{
				trickjumpLines->toggleRoutes(true);
			}
			return qtrue;
		}
	}

	if (command == "tjl_enablejumpmarker")
	{
		const auto argc = trap_Argc();
		if (argc == 1)
		{
			CG_Printf("Please add 0 or 1 as argument to enable or disable marker.\n");
			return qfalse;
		}
		else
		{
			std::string state = CG_Argv(1);
			CG_Printf("Enable marker arg : %s \n", state);
			if (state == "0")
			{
				trickjumpLines->toggleMarker(false);
			}
			else
			{
				trickjumpLines->toggleMarker(true);
			}
			return qtrue;
		}
	}
	return qfalse;
}

// TODO : (Zero) And this prolly should be elsewhere (e.g. cg_view_ext.cpp) but I'll just go with this one
// for now.. :P
void CG_DrawActiveFrameExt()
{
	// Check if recording
	if (trickjumpLines->isRecording())
	{
		// TODO : (xis) player origin doesn't change if crouch or prone, stay on feet. //cg.refdef.vieworg , //cg.predictedPlayerState.origin
		trickjumpLines->addPosition(cg.predictedPlayerState.origin);
	}
	else
	{
		// Check if nearest mode is activate
		if (etj_tjlNearestInterval.integer > 0)
		{
			// Check if nearest mode timer is due to check for nearest.
			if (nextNearest < cg.time)
			{
				if (trickjumpLines->isDebug())
				{
					CG_Printf("Check for nearest line!. \n");
				}
				trickjumpLines->displayNearestRoutes();
				nextNearest = cg.time + 1000 * etj_tjlNearestInterval.integer;
			}
		}

		// Check if line or jumper marker are enable.
		if (trickjumpLines->isEnableLine() || trickjumpLines->isEnableMarker())
		{
			// Check if record or if there line in the list
			if (trickjumpLines->countRoute() > 0 && !trickjumpLines->isRecording())
			{
				// Check if display has not been cleared.
				if (trickjumpLines->getCurrentRouteToRender() != -1)
				{
					// Display current route with the #
					trickjumpLines->displayCurrentRoute(trickjumpLines->getCurrentRouteToRender());
				}
			}
		}
	}
}

qboolean CG_displaybyname()
{
	const auto argc = trap_Argc();
	if (argc > 1)
	{
		const auto name = CG_Argv(1);
		trickjumpLines->displayByName(name);
	}
	else
	{
		trickjumpLines->displayByName(nullptr);
	}
	return qtrue;
}

qboolean CG_displaybynumber()
{
	const auto argc = trap_Argc();
	if (argc > 1)
	{
		const auto number = atoi(CG_Argv(1));
		const auto total = trickjumpLines->countRoute();
		if (number > -1 && number < total)
		{
			trickjumpLines->setCurrentRouteToRender(number);
			return qtrue;
		}
		return qfalse;
	}
	else
	{
		CG_Printf("You need to pass the route number by argument. Use command /tjl_listroute to get number. \n");
		return qfalse;
	}
}