#include "etj_demo_recorder.h"
#include "../game/etj_string_utilities.h"
#include "cg_local.h"

ETJump::DemoRecorder::DemoRecorder()
{
}

ETJump::DemoRecorder::~DemoRecorder()
{
}

void ETJump::DemoRecorder::start(const std::string& name)
{
	//if (cl_demorecording.integer) return;
	if (_isLocked) return;
	_startTime = cg.time;

	trap_SendConsoleCommand("set cl_noprint 1\n");
	trap_SendConsoleCommand(stringFormat("record %s\n", name.c_str()).c_str());
	trap_SendConsoleCommand("set cl_noprint 0\n");
}

void ETJump::DemoRecorder::stop()
{
	//if (!cl_demorecording.integer) return;
	if (_isLocked) return;
	_stopTime = cg.time;

	trap_SendConsoleCommand("set cl_noprint 1\n");
	trap_SendConsoleCommand("stoprecord\n");
	trap_SendConsoleCommand("set cl_noprint 0\n");
}

void ETJump::DemoRecorder::restart(const std::string& name)
{
	stop();
	start(name);
}

bool ETJump::DemoRecorder::isRecording()
{
	return cl_demorecording.integer == 1;
}

int ETJump::DemoRecorder::elapsed()
{
	return (cl_demorecording.integer ? cg.time : _stopTime) - _startTime;
}

void ETJump::DemoRecorder::lock()
{
	_isLocked = true;
}

void ETJump::DemoRecorder::unlock()
{
	_isLocked = false;
}

int ETJump::DemoRecorder::getStartTime()
{
	return _startTime;
}

int ETJump::DemoRecorder::getStopTime()
{
	return _stopTime;
}
