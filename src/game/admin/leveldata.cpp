#include "leveldata.h"
#include "../g_local.hpp"

LevelData::LevelData()
{

}

LevelData::~LevelData()
{

}

LevelData::Level::Level( int level, const std::string& name, const std::string& greeting, const std::string& commands )
{
    this->level = level;
    this->name = name;
    this->greeting = greeting;
    this->commands = commands;
}


void LevelData::AddLevel( int level, const std::string& name, const std::string& commands, const std::string& greeting )
{
    boost::shared_ptr< Level > newLevel(new Level(level, name, greeting, commands));

    levels_.push_back( newLevel );

    WriteLevels();
}

void LevelData::ReadLevels()
{
        
}

void LevelData::WriteLevels()
{

}

void LevelData::CreateDefaultLevels()
{
    if(!g_admin.integer || g_levelConfig.string[0] == 0)
    {
        return;
    }

    try
    {
        boost::shared_ptr< Level > tempLevel = boost::shared_ptr< Level >
            ( new Level(0, "Visitor", 
            "Welcome Visitor [n]^7!", "") );
        levels_.push_back(tempLevel);

        tempLevel = boost::shared_ptr< Level >
            ( new Level(1, "Friend", 
            "Welcome Friend [n]^7!", "") ); 
        levels_.push_back(tempLevel);

        tempLevel = boost::shared_ptr< Level >
            ( new Level(2, "Moderator",
            "Welcome Moderator [n]^7!", "") );
        levels_.push_back(tempLevel);

        tempLevel = boost::shared_ptr< Level >
                (new Level(3, "Administrator",
            "Welcome Administrator [n]^7!", ""));
        levels_.push_back(tempLevel);
    }
    catch( std::bad_alloc& e )
    {
        G_LogPrintf("Failed to allocate memory for a default level.\n");
        return;
    }

    WriteLevels();
}

