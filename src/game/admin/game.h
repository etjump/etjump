#ifndef game_h__
#define game_h__

class UserData;
class SessionData;
class SaveSystem;
class CommandInterpreter;
class LevelData;
class MapData;

#include <boost/shared_ptr.hpp>

struct Game
{
    Game();
    boost::shared_ptr< UserData > userData;
    boost::shared_ptr< LevelData > levelData;
    boost::shared_ptr< SessionData > session;
    boost::shared_ptr< SaveSystem > saveData;
    boost::shared_ptr< CommandInterpreter > command;
    boost::shared_ptr< MapData > mapData;
};

#endif // game_h__
