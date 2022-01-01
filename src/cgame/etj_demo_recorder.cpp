/*
 * MIT License
 * 
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
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
