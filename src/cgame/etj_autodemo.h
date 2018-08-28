#pragma once
#include "cg_local.h"

namespace ETJump
{
	class AutoDemoRecorder 
	{
	private:
		bool isTimerunStop{ false };
		bool stopIsIssued{ false };
		bool keepDemo{ false };
		bool recordingRestarted{ false };
		bool manualStop{ false };
		int currentDemoNum{ 1 };
		int demoStartTime, demoStopTime, demoStopDelay;
		int millis, seconds, minutes;
		std::string runTime, runTimeFormatted;
		std::string currentRealTime;
		std::string recordCommand;
		std::string targetPath, playerName, runName, manualDemoName, name;

		void getRealTime();
		void formatRunTime();
		void checkStartRecord();
		void prepareStop();
		void startRecord();
		void stopRecord();
		void saveDemo();
		void generateFileName();
		void replaceIllegalChars();

	public:
		void checkRespawn();
		void demoSaveDelay();
		AutoDemoRecorder();
		~AutoDemoRecorder() { };
	};
}
