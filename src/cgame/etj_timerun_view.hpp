#pragma once

#include <array>
#include <string>
#include "Drawable.h"

namespace ETJump
{
	struct PlayerTimerunInformation
	{
		int startTime;
		int completionTime;
		std::string runName;
		int previousRecord;
		bool running;
	};

	class TimerunView : Drawable
	{
	public:
		TimerunView() : Drawable()
		{
			interrupt(_ownTimerunInformation);
			for (auto& info : _playersTimerunInformation)
			{
				interrupt(_ownTimerunInformation);
			}
		}
		static const int MaxClients = 64;
		// whenever server sends any command that starts with 
		// `timerun` this will be called
		bool parseServerCommand();

		// whenever the player starts a timerun
		void start();

		// whenever the player stops a timerun
		void stop();

		// whenever another player starts a timerun
		void otherStart();

		// whenever the player's timerun is interrupted (not finished)
		void interrupt();

		// whenever the other player's timerun is interrupted
		void otherInterrupt();

		// saves the start info
		static void start(PlayerTimerunInformation& playerTimerunInformation);

		// saves the stop info
		static void stop(PlayerTimerunInformation& playerTimerunInformation);

		// interrups the run
		static void interrupt(PlayerTimerunInformation& playerTimerunInformation);

		// whenever another player stops a timerun
		void otherStop();

		// draws the timer
		void draw() override;

		// returns the currently active run if there's any
		// e.g. if player is running => return player's run,
		// else if player is running and we're speccing the player
		// => return that player's run
		const PlayerTimerunInformation *currentRun() const;
	private:
		std::array<PlayerTimerunInformation, MaxClients> _playersTimerunInformation;
		PlayerTimerunInformation _ownTimerunInformation;
	};
}
