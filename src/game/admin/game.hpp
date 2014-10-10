#ifndef GAME_HPP
#define GAME_HPP

#include <boost/shared_ptr.hpp>
class Levels;
class Session;
class Commands;
class SaveSystem;
class Database;
class MapData;
class Races;
class CustomMapVotes;

struct Game
{
    Game();

    boost::shared_ptr<Levels> levels;
    boost::shared_ptr<Session> session;
    boost::shared_ptr<Commands> commands;
    boost::shared_ptr<SaveSystem> saves;
    boost::shared_ptr<Database> database;
    boost::shared_ptr<MapData> mapData;
    boost::shared_ptr<Races> races;
    boost::shared_ptr<CustomMapVotes> customMapVotes;
};

#endif 
