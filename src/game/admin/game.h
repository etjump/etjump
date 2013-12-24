#ifndef game_h__
#define game_h__

class UserData;
class SessionData;
class SaveSystem;
class CommandInterpreter;

#include <boost/shared_ptr.hpp>

struct Game
{
    Game();
    boost::shared_ptr< UserData > userData;
    boost::shared_ptr< SessionData > session;
    boost::shared_ptr< SaveSystem > saveData;
    boost::shared_ptr< CommandInterpreter > command;
};

#endif // game_h__
