#include "game.hpp"
#include "session.hpp"
#include "commands.hpp"
#include "../g_save.hpp"
#include "levels.hpp"
#include "database.hpp"
#include "../mapdata.h"
#include "../races.hpp"
#include "../custommapvotes.hpp"
#include "../operationqueue.hpp"

Game::Game()
{
    levels = boost::shared_ptr<Levels>(new Levels());
    database = boost::shared_ptr<Database>(new Database());
    session = boost::shared_ptr<Session>(new Session(database.get()));
    commands = boost::shared_ptr<Commands>(new Commands());
    saves = boost::shared_ptr<SaveSystem>(new SaveSystem(session.get()));
    mapData = boost::shared_ptr<MapData>(new MapData());
    operationQueue = boost::shared_ptr<OperationQueue>(new OperationQueue());
    races = boost::shared_ptr<Races>(new Races(operationQueue.get()));
    customMapVotes = boost::shared_ptr<CustomMapVotes>(new CustomMapVotes());
}