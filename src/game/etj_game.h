#ifndef GAME_HPP
#define GAME_HPP

#include <memory>
class SaveSystem;
class CustomMapVotes;
class Motd;
class Timerun;
class MapStatistics;

struct Game
{
	Game()
	{
	}

	std::shared_ptr<SaveSystem> saves;
	std::shared_ptr<CustomMapVotes> customMapVotes;
	std::shared_ptr<Motd> motd;
	std::shared_ptr<Timerun> timerun;
	std::shared_ptr<MapStatistics> mapStatistics;
};

#endif
