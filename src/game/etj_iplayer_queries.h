#pragma once
#include <string>

namespace ETJump
{
	enum class Team
	{
		Free,
		Axis,
		Allies,
		Spectator
	};

	class IPlayerQueries
	{
	public:
		virtual ~IPlayerQueries() {}
		/**
		* Gets connected player count
		*/
		virtual int connectedPlayerCount() const = 0;

		/**
		 * Gets number of players playing (not spectating)
		 */
		virtual int playingPlayerCount() const = 0;

		/**
		* Gets the player name
		*/
		virtual std::string name(int clientNum) const = 0;

		/**
		 * Gets the player's team
		 */
		virtual Team team(int clientNum) const = 0;
	};
}
