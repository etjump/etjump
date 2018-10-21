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
		if (etj_autoDemo.integer > 1) tryRestart();
	});

	playerEventsHandler->subscribe("respawn", [&](const std::vector<std::string>& args)
	{
		auto revive = atoi(args[0].c_str());
		if (revive) return;
		if (etj_autoDemo.integer > 1) tryRestart();
		// cut the recording if not in timerun
		if (etj_autoDemo.integer == 1) checkTimerunDemoStop();		
	});

	playerEventsHandler->subscribe("timerun:start", [&](const std::vector<std::string>& args)
	{
		if (etj_autoDemo.integer != 1) return;
		forceDelayedDemoSave(); // if required
		tryRestart();
	});

	playerEventsHandler->subscribe("timerun:completion", [&](const std::vector<std::string>& args)
	{
		if (etj_autoDemo.integer > 0 && etj_ad_savePBsOnly.integer <= 0)
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
		auto src = createDemoTempPath(_demoNames.current());
		auto dst = createDemoPath(args.size() ? args[0] : "demo");
		saveDemoWithRestart(src, dst);
	});
}

ETJump::AutoDemoRecorder::~AutoDemoRecorder() {}

void ETJump::AutoDemoRecorder::tryRestart()
{
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
	saveTimerunDemo(src, dst);
}

void ETJump::AutoDemoRecorder::saveTimerunDemo(const std::string &src, const std::string &dst)
{
	auto delay = Numeric::clamp(etj_ad_stopDelay.integer, 0, DEMO_MAX_SAVE_DELAY);
	_delayedTimerId = setTimeout([&, src, dst]{ saveDemoWithRestart(src, dst); }, delay);
}

void ETJump::AutoDemoRecorder::saveDemo(const std::string &src, const std::string &dst)
{
	cancelDelayedDemoSave(); // if any
	setTimeout([src, dst]{ FileSystem::safeCopy(src, dst); }, DEMO_SAVE_DELAY);
}

void ETJump::AutoDemoRecorder::saveDemoWithRestart(const std::string &src, const std::string &dst)
{
	saveDemo(src, dst);
	if (etj_autoDemo.integer > 1) restart();
	else _demo.stop(); // don't auto restart if etj_autoDemo is set to timerun mode
}

std::string ETJump::AutoDemoRecorder::createDemoPath(std::string name)
{
	return stringFormat("demos/%s/%s_%s_%s[%s].dm_84",
		FileSystem::Path::sanitizeFolder(etj_ad_targetPath.string),
		FileSystem::Path::sanitize(cgs.clientinfo[cg.clientNum].cleanname),
		cgs.rawmapname,
		name,
		createTimeString());
}

std::string ETJump::AutoDemoRecorder::createTimerunDemoPath(const std::string& runName, const std::string& runTime)
{
	return stringFormat("demos/%s/%s_%s_%s_%s[%s].dm_84",
		FileSystem::Path::sanitizeFolder(etj_ad_targetPath.string),
		FileSystem::Path::sanitize(cgs.clientinfo[cg.clientNum].cleanname),
		cgs.rawmapname,
		runName,
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

void ETJump::AutoDemoRecorder::cancelDelayedDemoSave()
{
	if (_delayedTimerId)
	{
		clearTimeout(_delayedTimerId);
		_delayedTimerId = 0;
	}
}

void ETJump::AutoDemoRecorder::checkTimerunDemoStop()
{
	if (!cgs.clientinfo[cg.clientNum].timerunActive && !_delayedTimerId) {
		_demo.stop();
	}
}

void ETJump::AutoDemoRecorder::forceDelayedDemoSave()
{
	if (!_delayedTimerId) return;
	executeTimeout(_delayedTimerId);
	cancelDelayedDemoSave();
}
