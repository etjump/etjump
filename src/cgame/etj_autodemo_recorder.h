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

#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <string>
#include <queue>
#include "etj_demo_recorder.h"

namespace ETJump
{
	class AutoDemoRecorder
	{
		class TempNameGenerator
		{
			std::queue<std::string> names;
			int nameCounter{ 0 };
		public:
			std::string pop();
			std::string next();
			std::string current();
			int size();
		};

		int _delayedTimerId{ 0 };
		DemoRecorder _demo;
		TempNameGenerator _demoNames;
	public:
		AutoDemoRecorder();
		~AutoDemoRecorder();
		void tryRestart();
		void restart();
		void trySaveTimerunDemo(const std::string &runName, const std::string &runTime);
		void saveTimerunDemo(const std::string &src, const std::string &dst);
		void saveDemo(const std::string &src, const std::string &dst);
		void saveDemoWithRestart(const std::string &src, const std::string &dst);
		std::string createDemoPath(std::string name = "");
		std::string createTimerunDemoPath(const std::string &runName, const std::string &runTime);
		std::string createDemoTempPath(const std::string &name);
		std::string createTimeString();
		std::string formatRunTime(int millis);
		void maybeCancelDelayedSave();
	};
}
