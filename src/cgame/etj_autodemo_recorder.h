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