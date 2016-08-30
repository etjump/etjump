#pragma once
#include <string>

namespace ETJump
{
	struct Map
	{
		Map(): id(0), secondsPlayed(0), callvoted(0), 
			votesPassed(0), timesPlayed(0), lastPlayed(0),
			isOnServer(false), changed(false)
		{
		}

		// database ID for the map
		long id;
		std::string name;
		int secondsPlayed;
		int callvoted;
		int votesPassed;
		int timesPlayed;
		int lastPlayed;
		bool isOnServer;
		bool changed;
	};
}
