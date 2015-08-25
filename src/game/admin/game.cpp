#include "game.hpp"
#include "session.hpp"
#include "commands.hpp"
#include "../g_save.hpp"
#include "levels.hpp"
#include "database.hpp"
#include "../races.hpp"
#include "../custommapvotes.hpp"
#include "../operationqueue.hpp"
#include "../motd.hpp"
#include "../timerun.h"
#include "../map_statistics.h"

Game::Game()
{
    levels = std::make_shared<Levels>();
    database = std::make_shared<Database>();
    session = std::make_shared<Session>(database.get());
    commands = std::make_shared<Commands>();
    saves = std::make_shared<SaveSystem>(session.get());
    operationQueue = std::make_shared<OperationQueue>();
    races = std::make_shared<Races>(operationQueue.get());
    customMapVotes = std::make_shared<CustomMapVotes>();
    motd = std::make_shared<Motd>();
    timerun = std::make_shared<Timerun>();
	mapStatistics = std::make_shared<MapStatistics>();
}
