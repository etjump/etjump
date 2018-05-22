#pragma once

#include <array>
#include <string>
#include "etj_drawable.h"

namespace ETJump
{
	struct PlayerTimerunInformation
	{
		int startTime;
		int completionTime;
		std::string runName;
		int previousRecord;
		bool running;
		// used for fading
		int lastRunTimer;
	};

	class TimerunView : public Drawable
	{
	public:
		TimerunView();
		~TimerunView();
		static const int MaxClients = 64;
		// whenever server sends any command that starts with 
		// `timerun` this will be called
		bool parseServerCommand();

		// whenever the player starts a timerun
		void start();

		// whenever the player stops a timerun
		void stop();

		// whenever the player's timerun is interrupted (not finished)
		void interrupt();
		static void interrupt(PlayerTimerunInformation& playerTimerunInformation);

		// draws the timer
		void draw();

		int getTransitionRange(int previousRunTime);

		void pastRecordAnimation(vec4_t *color, const char* text, int timerTime, int record);

		// returns the currently active run if there's any
		// e.g. if player is running => return player's run,
		// else if player is running and we're speccing the player
		// => return that player's run
		const PlayerTimerunInformation *currentRun() const;
	private:
		std::array<PlayerTimerunInformation, MaxClients> _playersTimerunInformation;
		PlayerTimerunInformation _ownTimerunInformation;
		vec4_t inactiveTimerColor;

		bool canSkipDraw() const;
	};
}
