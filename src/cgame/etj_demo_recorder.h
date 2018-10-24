#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <string>

namespace ETJump
{
	class DemoRecorder
	{
		int _startTime{ 0 };
		int _stopTime{ 0 };
		bool _isLocked{ false };
	public:
		DemoRecorder();
		~DemoRecorder();
		void start(const std::string &name);
		void stop();
		void restart(const std::string &name);
		static bool isRecording();
		int elapsed();
		void lock();
		void unlock();
		int getStartTime();
		int getStopTime();
	};
}