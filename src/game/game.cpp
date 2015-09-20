#include "game.hpp"
#include "session.hpp"
#include "commands.hpp"
#include "save.hpp"
#include "levels.hpp"
#include "database.hpp"
#include "custom_map_votes.hpp"
#include "operation_queue.hpp"
#include "motd.hpp"
#include "timerun.hpp"
#include "map_statistics.hpp"

Game::Game()
{
	levels         = std::make_shared<Levels>();
	database       = std::make_shared<Database>();
	session        = std::make_shared<Session>(database.get());
	commands       = std::make_shared<Commands>();
	saves          = std::make_shared<SaveSystem>(session.get());
	operationQueue = std::make_shared<OperationQueue>();
	customMapVotes = std::make_shared<CustomMapVotes>();
	motd           = std::make_shared<Motd>();
	timerun        = std::make_shared<Timerun>();
	mapStatistics  = std::make_shared<MapStatistics>();
}
