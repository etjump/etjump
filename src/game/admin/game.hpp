#ifndef GAME_HPP
#define GAME_HPP

#include <memory>
class Levels;
class Session;
class Commands;
class SaveSystem;
class Database;
class Races;
class CustomMapVotes;
class OperationQueue;
class Motd;
class Timerun;
class RandomMapMode;
class MapStatistics;

struct Game
{
    Game();

	std::shared_ptr<Levels> levels;
	std::shared_ptr<Database> database;
	std::shared_ptr<Session> session;
	std::shared_ptr<Commands> commands;
	std::shared_ptr<SaveSystem> saves;
	std::shared_ptr<OperationQueue> operationQueue;
	std::shared_ptr<Races> races;
	std::shared_ptr<CustomMapVotes> customMapVotes;
	std::shared_ptr<Motd> motd;
	std::shared_ptr<Timerun> timerun;
	std::shared_ptr<RandomMapMode> randomMapMode;
	std::shared_ptr<MapStatistics> mapStatistics;
};

#endif 
