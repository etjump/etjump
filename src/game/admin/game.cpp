#include "game.h"

#include "sessiondata.h"
#include "userdata.h"
#include "commandinterpreter.h"
#include "../g_save.hpp"
#include "leveldata.h"

Game::Game()
{
    levelData = boost::shared_ptr< LevelData >( new LevelData() );
    userData = boost::shared_ptr< UserData >( new UserData() );
    session = boost::shared_ptr< SessionData >( new SessionData( userData.get(), levelData.get() ) );
    command = boost::shared_ptr< CommandInterpreter >( new CommandInterpreter );
    saveData = boost::shared_ptr< SaveSystem >( new SaveSystem( session.get() ) ); 
}
