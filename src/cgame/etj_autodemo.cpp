#include "etj_autodemo.h"
#include "etj_entity_events_handler.h"
#include "etj_client_commands_handler.h"
#include "../game/etj_string_utilities.h"
#include "../game/etj_numeric_utilities.h"

#define AUTODEMO_MAX_TEMP_DEMOS 20
#define AUTODEMO_START_DELAY 500
#define AUTODEMO_STOP_DELAY 500

ETJump::AutoDemoRecorder::AutoDemoRecorder()
{
	if (cg.demoPlayback)
	{
		return;
	}

	entityEventsHandler->subscribe(EV_LOAD_TELEPORT, [&](centity_t *cent)
	{
		if (etj_autoDemo.integer <= 0)
		{
			return;
		}

		checkStartRecord();
	});

	serverCommandsHandler->subscribe("completion", [&](const std::vector<std::string>& args)
	{
		if (etj_autoDemo.integer <= 0)
		{
			return;
		}

		keepDemo = etj_ad_savePBsOnly.integer > 0 ? false : true;
		if (keepDemo)
		{
			isTimerunStop = true;
			runName = args[1];
			runTime = args[2];
			prepareStop();
		}		
	});

	serverCommandsHandler->subscribe("record", [&](const std::vector<std::string>& args)
	{
		if (etj_autoDemo.integer <= 0)
		{
			return;
		}

		keepDemo = true;
		isTimerunStop = true;
		runName = args[1];
		runTime = args[2];
		prepareStop();
	});

	consoleCommandsHandler->subscribe("ad_save", [&](const std::vector<std::string>& args)
	{
		if (etj_autoDemo.integer <= 0)
		{
			return;
		}

		if (args.size() != 1)
		{
			CG_Printf("^3Usage: ^7ad_save <name>\nSaves currently ongoing autodemo temp file manually with given name.\n");
			return;
		}

		manualDemoName = args[0];
		keepDemo = true;
		manualStop = true;
		prepareStop();
	});
}

void ETJump::AutoDemoRecorder::checkRespawn()
{
	switch (cgs.clientinfo[cg.clientNum].team)
	{
	case TEAM_ALLIES:
	case TEAM_AXIS:
		checkStartRecord();
		break;
	}
}

void ETJump::AutoDemoRecorder::checkStartRecord()
{
	if (cgs.clientinfo[cg.clientNum].timerunActive || stopIsIssued)
	{
		return;
	}

	if (demoStartTime + AUTODEMO_START_DELAY > cg.time)
	{
		return;
	}

	demoStartTime = cg.time;
	isTimerunStop = false;

	if (cl_demorecording.integer)
	{
		stopRecord();
	}
	startRecord();
}

void ETJump::AutoDemoRecorder::prepareStop()
{
	if (stopIsIssued)
	{
		return;
	}

	CG_AddPMItem(PM_MESSAGE, "^7Stopping demo...\n", cgs.media.voiceChatShader);
	stopIsIssued = true;

	demoStopDelay = Numeric::clamp(etj_ad_stopDelay.integer, 0, 10000);
	demoStopTime = manualStop ? cg.time : cg.time + demoStopDelay;
	demoSaveDelay();
}

void ETJump::AutoDemoRecorder::demoSaveDelay()
{
	if (manualStop)
	{
		if (!recordingRestarted)
		{
			stopRecord();
			startRecord();
			recordingRestarted = true;
		}

		if (cg.time > demoStopTime + AUTODEMO_STOP_DELAY)
		{
			saveDemo();
			manualStop = false;
			keepDemo = false;
			isTimerunStop = false;
			stopIsIssued = false;
			recordingRestarted = false;
			return;
		}
	}

	if (!isTimerunStop || !keepDemo)
	{
		return;
	}

	if (cg.time > demoStopTime)
	{
		if (!recordingRestarted)
		{
			stopRecord();
			startRecord();
			recordingRestarted = true;
		}

		if (cg.time > demoStopTime + AUTODEMO_STOP_DELAY)
		{
			saveDemo();
			keepDemo = false;
			isTimerunStop = false;
			stopIsIssued = false;
			recordingRestarted = false;
		}
	}
}

void ETJump::AutoDemoRecorder::startRecord()
{
	recordCommand = ETJump::stringFormat("record temp/temp_%02d\n", currentDemoNum);
	currentDemoNum++;

	trap_SendConsoleCommand("set cl_noprint 1\n");
	trap_SendConsoleCommand(recordCommand.c_str());
	trap_SendConsoleCommand("set cl_noprint 0\n");

	if (currentDemoNum > AUTODEMO_MAX_TEMP_DEMOS)
	{
		currentDemoNum = 1;
	}
}

void ETJump::AutoDemoRecorder::stopRecord()
{
	trap_SendConsoleCommand("set cl_noprint 1\n");
	trap_SendConsoleCommand("stoprecord\n");
	trap_SendConsoleCommand("set cl_noprint 0\n");
}

void ETJump::AutoDemoRecorder::saveDemo()
{
	fileHandle_t temp, demo;
	auto len = 0;
	std::string currentDemo;
		
	getRealTime();

	currentDemo = ETJump::stringFormat("demos/temp/temp_%02d.dm_84", currentDemoNum - 2);
	len = trap_FS_FOpenFile(currentDemo.c_str(), &temp, FS_READ);

	replaceIllegalChars();
	generateFileName();

	if (trap_FS_FOpenFile(name.c_str(), &demo, FS_WRITE) < 0)
	{
		CG_Printf("^3Error: ^7Unable to save demo %s.\n", name.c_str());
		trap_FS_FCloseFile(temp);
		return;
	}

	for (auto i = 0; i < len; ++i)
	{
		byte b = 0;

		trap_FS_Read(&b, 1, temp);
		trap_FS_Write(&b, 1, demo);
	}

	trap_FS_FCloseFile(temp);
	trap_FS_FCloseFile(demo);

	CG_Printf("^7Demo saved to %s\n", name.c_str());
	CG_AddPMItem(PM_MESSAGE, "^7Demo saved!\n", cgs.media.voiceChatShader);
}

void ETJump::AutoDemoRecorder::generateFileName()
{
	if (manualStop)
	{
		name = ETJump::stringFormat("demos/%s/%s_%s_%s[%s].dm_84",
									targetPath,
									playerName,
									cgs.rawmapname,
									manualDemoName,
									currentRealTime);
	}
	else
	{
		formatRunTime();
		name = ETJump::stringFormat("demos/%s/%s_%s_%s_%s[%s].dm_84",
									targetPath,
									playerName,
									cgs.rawmapname,
									runName,
									runTimeFormatted,
									currentRealTime);
	}
}

void ETJump::AutoDemoRecorder::replaceIllegalChars()
{
	std::string illegalChars = "\\/:*?\"<>|";
	targetPath = etj_ad_targetPath.string;
	playerName = cgs.clientinfo[cg.clientNum].cleanname;

	// Feel free to replace this shit with regex
	for (int i = 0; i < illegalChars.length(); ++i)
	{
		std::replace(targetPath.begin(), targetPath.end(), illegalChars[i], '_');
		std::replace(playerName.begin(), playerName.end(), illegalChars[i], '_');
		std::replace(runName.begin(), runName.end(), illegalChars[i], '_');
		std::replace(manualDemoName.begin(), manualDemoName.end(), illegalChars[i], '_');
	}
}

void ETJump::AutoDemoRecorder::getRealTime()
{
	qtime_t ct;
	trap_RealTime(&ct);

	currentRealTime = ETJump::stringFormat("%02d-%02d-%d-%02d%02d%02d",
											ct.tm_mday,
											ct.tm_mon + 1,
											1900 + ct.tm_year,
											ct.tm_hour,
											ct.tm_min,
											ct.tm_sec);
}

void ETJump::AutoDemoRecorder::formatRunTime()
{
	millis = atoi(runTime.c_str());
	minutes = millis / 60000;
	millis -= minutes * 60000;
	seconds = millis / 1000;
	millis -= seconds * 1000;

	runTimeFormatted = ETJump::stringFormat("%02d.%02d.%03d", minutes, seconds, millis);
}
