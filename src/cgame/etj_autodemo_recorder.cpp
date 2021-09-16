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

#include "etj_autodemo_recorder.h"
#include "etj_utilities.h"
#include "etj_demo_recorder.h"
#include "../game/etj_filesystem.h"
#include "../game/etj_string_utilities.h"
#include "../game/etj_numeric_utilities.h"
#include "../game/etj_time_utilities.h"
#include "etj_client_commands_handler.h"
#include "etj_player_events_handler.h"
#include "cg_local.h"

// constants
static const int DEMO_SAVE_DELAY = 500;
static const int DEMO_MAX_SAVE_DELAY = 10000;
static const int DEMO_START_TIMEOUT = 500;
static const int MAX_TEMP = 20;
static const char *TEMP_PATH = "temp";

std::string ETJump::AutoDemoRecorder::TempNameGenerator::pop()
{
	if (!names.size()) return "temp";
	auto name = names.front();
	names.pop();
	return name;
}

std::string ETJump::AutoDemoRecorder::TempNameGenerator::next()
{
	// Clock clock = getCurrentClock();
	// auto name = stringFormat("%s/temp_%d-%d-%d-%03d", TEMP_PATH, clock.hours, clock.min, clock.sec, clock.ms);
	auto name = stringFormat("%s/temp_%02d", TEMP_PATH, (nameCounter++ % MAX_TEMP) + 1);
	names.push(name);
	return name;
}

std::string ETJump::AutoDemoRecorder::TempNameGenerator::current()
{
	return names.size() > 0 ? names.back() : "temp";
}

int ETJump::AutoDemoRecorder::TempNameGenerator::size()
{
	return names.size();
}

ETJump::AutoDemoRecorder::AutoDemoRecorder()
{
	if (cg.demoPlayback) return;

	playerEventsHandler->subscribe("load", [&](const std::vector<std::string>& args)
	{
		if (etj_autoDemo.integer > 0) tryRestart();
	});

	playerEventsHandler->subscribe("respawn", [&](const std::vector<std::string>& args)
	{
		auto revive = atoi(args[0].c_str());
		if (revive) return;
		if (etj_autoDemo.integer > 0) tryRestart();		
	});

	playerEventsHandler->subscribe("timerun:completion", [&](const std::vector<std::string>& args)
	{
		if (etj_autoDemo.integer > 0 && etj_ad_savePBOnly.integer <= 0)
		{
			trySaveTimerunDemo(args[0], args[1]);
		}
	});

	playerEventsHandler->subscribe("timerun:record", [&](const std::vector<std::string>& args)
	{
		if (etj_autoDemo.integer > 0) trySaveTimerunDemo(args[0], args[1]);
	});

	consoleCommandsHandler->subscribe("ad_save", [&](const std::vector<std::string>& args)
	{
		if (etj_autoDemo.integer <= 0) return;
		if (!cl_demorecording.integer)
		{
			CG_AddPMItem(PM_MESSAGE, "^7Not recording a demo.\n", cgs.media.voiceChatShader);
			return;
		}
		auto src = createDemoTempPath(_demoNames.current());
		auto dst = createDemoPath(args.size() ? args[0] : "demo");
		saveDemoWithRestart(src, dst);
	});
}

ETJump::AutoDemoRecorder::~AutoDemoRecorder() {}

void ETJump::AutoDemoRecorder::tryRestart()
{
	// no autodemo for specs
	if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR) return;
	// start autodemo for timerun maps only
	if (etj_autoDemo.integer == 1 && !cg.hasTimerun) return;
	// don't start autodemo if timeruns are disabled unless autodemo is enabled for all maps
	if (etj_autoDemo.integer < 2 && !etj_enableTimeruns.integer) return;
	// timeout
	if (_demo.getStartTime() + DEMO_START_TIMEOUT >= cg.time) return;
	// dont attempt to restart if in timerun mode
	if (cgs.clientinfo[cg.clientNum].timerunActive || _delayedTimerId) return;
	restart();
}

void ETJump::AutoDemoRecorder::restart()
{
	_demo.restart(_demoNames.next());
	if (_demoNames.size() > MAX_TEMP)
	{
		// FileSystem::remove(createDemoTempPath(_demoNames.pop()));
		_demoNames.pop();
	}
}

void ETJump::AutoDemoRecorder::trySaveTimerunDemo(const std::string &runName, const std::string &runTime)
{
	if (_delayedTimerId) return;
	if (!_demo.isRecording()) return;
	auto src = createDemoTempPath(_demoNames.current());
	auto dst = createTimerunDemoPath(runName, runTime);
	CG_AddPMItem(PM_MESSAGE, "^7Stopping demo...\n", cgs.media.voiceChatShader);
	saveTimerunDemo(src, dst);
}

void ETJump::AutoDemoRecorder::saveTimerunDemo(const std::string &src, const std::string &dst)
{
	auto delay = Numeric::clamp(etj_ad_stopDelay.integer, 0, DEMO_MAX_SAVE_DELAY);
	_delayedTimerId = setTimeout([&, src, dst]{ saveDemoWithRestart(src, dst); }, delay);
}

void ETJump::AutoDemoRecorder::saveDemo(const std::string &src, const std::string &dst)
{
	maybeCancelDelayedSave();
	setTimeout([src, dst]{ FileSystem::safeCopy(src, dst); }, DEMO_SAVE_DELAY);
}

void ETJump::AutoDemoRecorder::saveDemoWithRestart(const std::string &src, const std::string &dst)
{
	saveDemo(src, dst);
	CG_AddPMItem(PM_MESSAGE, "^7Demo saved!\n", cgs.media.voiceChatShader);
	CG_Printf("^7Demo saved to %s\n", dst.c_str());
	restart();
}

std::string ETJump::AutoDemoRecorder::createDemoPath(std::string name)
{
	return stringFormat("demos/%s/%s_%s_%s[%s].dm_84",
		FileSystem::Path::sanitizeFolder(etj_ad_targetPath.string),
		FileSystem::Path::sanitize(cgs.clientinfo[cg.clientNum].cleanname),
		cgs.rawmapname,
		FileSystem::Path::sanitize(name),
		createTimeString());
}

std::string ETJump::AutoDemoRecorder::createTimerunDemoPath(const std::string& runName, const std::string& runTime)
{
	return stringFormat("demos/%s/%s_%s_%s_%s[%s].dm_84",
		FileSystem::Path::sanitizeFolder(etj_ad_targetPath.string),
		FileSystem::Path::sanitize(cgs.clientinfo[cg.clientNum].cleanname),
		cgs.rawmapname,
		FileSystem::Path::sanitize(runName),
		formatRunTime(atoi(runTime.c_str())),
		createTimeString());
}

std::string ETJump::AutoDemoRecorder::createDemoTempPath(const std::string& name)
{
	return stringFormat("demos/%s.dm_84", name);
}

std::string ETJump::AutoDemoRecorder::createTimeString()
{
	Time time = getCurrentTime();
	return stringFormat("%02d-%02d-%d-%02d%02d%02d",
			time.date.day, time.date.mon, time.date.year, time.clock.hours, time.clock.min, time.clock.sec);
}

std::string ETJump::AutoDemoRecorder::formatRunTime(int millis)
{
	Clock clock = toClock(millis);
	return stringFormat("%02d.%02d.%03d", clock.min, clock.sec, clock.ms);
}

void ETJump::AutoDemoRecorder::maybeCancelDelayedSave()
{
	if (_delayedTimerId)
	{
		clearTimeout(_delayedTimerId);
		_delayedTimerId = 0;
	}
}
