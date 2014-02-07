#include "leveldata.h"
#include "../g_local.hpp"
#include "../g_utilities.hpp"

#include <boost/shared_array.hpp>
#include <boost/algorithm/string.hpp>

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


bool LevelData::AddLevel( int level, const std::string& name, const std::string& commands, const std::string& greeting )
{
    for(size_t i = 0; i < levels_.size(); i++)
    {
        if(levels_.at(i)->level == level)
        {
            return false;
        }
    }

    boost::shared_ptr< Level > newLevel(new Level(level, name, greeting, commands));

    levels_.push_back( newLevel );

    WriteLevels();
    return true;
}

// Write to & Read from file functions
void WriteString(const char* toWrite, fileHandle_t& f)
{
    trap_FS_Write(toWrite, strlen(toWrite), f);
    trap_FS_Write("\n", 1, f);
}

void WriteInt(int toWrite, fileHandle_t& f)
{
    const int BUFSIZE = 32;
    char buf[BUFSIZE];
    Com_sprintf(buf, sizeof(buf), "%d", toWrite);
    trap_FS_Write(buf, strlen(buf), f);
    trap_FS_Write("\n", 1, f);
}

void ReadInt( char ** configFile, int& level ) 
{
    char *token = COM_ParseExt(configFile, qfalse);

    if(!Q_stricmp(token, "="))
    {
        token = COM_ParseExt(configFile, qfalse);
    } else
    {
        G_LogPrintf("readconfig: missing = before \"%s\" on line %d.\n",
            token, COM_GetCurrentParseLine());
    }
    level = atoi(token);
}

void ReadString( char** configFile, std::string& str )
{
    char *token = COM_ParseExt(configFile, qfalse);

    if(!Q_stricmp(token, "="))
    {
        token = COM_ParseExt(configFile, qfalse);
    } else
    {
        G_LogPrintf("readconfig: missing = before \"%s\" on line %d.\n",
            token, COM_GetCurrentParseLine());
    }
    str.clear();
    while(token[0])
    {
        str += token;
        str.push_back(' ');
        token = COM_ParseExt(configFile, qfalse);
    }

    // trim the trailing space
    boost::trim_right(str);
}

void LevelData::ReadLevels()
{
    if(!g_admin.integer || g_levelConfig.string[0] == 0)
    {
        return;
    }

    fileHandle_t f = 0;
    int len = trap_FS_FOpenFile(g_levelConfig.string, &f, FS_READ);
    if(len < 0)
    {
        CreateDefaultLevels();
        return;
    } else
    {
        boost::shared_array<char> file;
        try
        {
            file = boost::shared_array<char>(new char[len+1]);
        }
        catch( std::bad_alloc& e )
        {
            G_LogPrintf("Failed to allocate memory to parse level config.\n");
            trap_FS_FCloseFile(f);
            return;
        }

        trap_FS_Read(file.get(), len, f);
        file[len] = 0;
        trap_FS_FCloseFile(f);

        char *token = NULL;
        bool levelOpen = false;
        boost::shared_ptr<Level> tempLevel;

        levels_.clear();

        // Little hack to be able to use shared_array with COM_Parse*
        char *file2 = file.get();

        token = COM_Parse(&file2);

        while(*token)
        {
            if(!Q_stricmp(token, "[level]"))
            {
                if(levelOpen)
                {
                    levels_.push_back(tempLevel);
                }
                levelOpen = false;
            }
            else if(!Q_stricmp(token, "cmds"))
            {
                ReadString(&file2, tempLevel->commands);
            }
            else if(!Q_stricmp(token, "level"))
            {
                ReadInt(&file2, tempLevel->level);
            }
            else if(!Q_stricmp(token, "greeting"))
            {
                ReadString(&file2, tempLevel->greeting);
            }
            else if(!Q_stricmp(token, "name"))
            {
                ReadString(&file2, tempLevel->name);
            }
            else {
                G_LogPrintf("readconfig: parse error near %s on line %d",
                    token, COM_GetCurrentParseLine());
            }

            if(!Q_stricmp(token, "[level]"))
            {
                try
                {
                    tempLevel = boost::shared_ptr<Level>(new Level(0, "", "", ""));
                }
                catch( std::bad_alloc& e )
                {
                    G_LogPrintf("Failed to allocate memory for a level.\n");
                    return;
                }

                levelOpen = true;
            }

            token = COM_Parse(&file2);
        }

        if(levelOpen)
        {
            levels_.push_back(tempLevel);
        }
    }
}

bool LevelData::SortByLevel(const boost::shared_ptr<Level>& lhs, const boost::shared_ptr<Level>& rhs)
{
    return lhs->level < rhs->level;
}

void LevelData::WriteLevels()
{
    if(!g_admin.integer || g_levelConfig.string[0] == 0)
    {
        return;
    }

    fileHandle_t f = 0;
    if(trap_FS_FOpenFile(g_levelConfig.string, &f, FS_WRITE) < 0)
    {
        G_LogPrintf("Couldn't open file \"%s\" to save admin levels.\n",
            g_levelConfig.string);
        return;
    }

    std::sort(levels_.begin(), levels_.end(), SortByLevel);

    for(std::vector<boost::shared_ptr<Level> >::const_iterator it = levels_.begin();
        it != levels_.end(); it++)
    {
        trap_FS_Write("[level]\n", 8, f);
        trap_FS_Write("level = ", 8, f);
        WriteInt(it->get()->level, f);

        trap_FS_Write("name = ", 7, f);
        WriteString(it->get()->name.c_str(), f);

        trap_FS_Write("cmds = ", 7, f);
        WriteString(it->get()->commands.c_str(), f);

        trap_FS_Write("greeting = ", 11, f);
        WriteString(it->get()->greeting.c_str(), f);
        trap_FS_Write("\n", 1, f);
    }

    trap_FS_FCloseFile(f);
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

bool LevelData::EditLevel( int level, const std::string& newName, 
                          const std::string& newCommands, 
                          const std::string& newGreeting,
                          int updated )
{
    unsigned i = 0;
    // !editlevel [level] -name [name] -cmds [commands] -greeting [greeting]
    for( i = 0; i < levels_.size(); i++ )
    {
        if( levels_.at(i)->level == level )
        {
            break;
        }
    }

    if( i == levels_.size() )
    {
        // Couldn't find level, print error msg
        return false;
    }

    const int CMDS_OPEN = 1;
    const int GREETING_OPEN = 2;
    const int TITLE_OPEN = 4;

    if(updated & CMDS_OPEN)
    {
        levels_.at(i)->commands = newCommands;
    } 
    
    if(updated & GREETING_OPEN)
    {
        levels_.at(i)->greeting = newGreeting;
    }
    
    if(updated & TITLE_OPEN)
    {
        levels_.at(i)->name = newName;
    }

    WriteLevels();

    return true;
}

const ILevelData::LevelInformation *LevelData::GetLevelInformation( int level )
{
    static ILevelData::LevelInformation targetLevel;
    targetLevel.isValid = false;
    for(unsigned i = 0; i < levels_.size(); i++)
    {
        if(levels_.at(i)->level == level)
        {
            targetLevel.name = levels_.at(i)->name;
            targetLevel.commands = levels_.at(i)->commands;
            targetLevel.greeting = levels_.at(i)->greeting;
            targetLevel.isValid = true;
        }
    }
    return &targetLevel;
}

bool LevelData::DeleteLevel( int level )
{
    for(size_t i = 0; i < levels_.size(); i++)
    {
          if(levels_.at(i)->level == level)
        {
            levels_.erase(levels_.begin() + i);
            WriteLevels();
            return true;
        }
    }
    WriteLevels();
    return false;
}

void LevelData::PrintLevels( gentity_t *ent )
{
    BeginBufferPrint();
    BufferPrint(ent, "levels: ");
    for(size_t i = 0; i < levels_.size(); i++)
    {
        BufferPrint(ent, va("%d ", levels_.at(i)->level));
    }
    FinishBufferPrint(ent, true);
}

