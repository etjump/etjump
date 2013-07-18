#include "g_levels.hpp"
#include "g_local.hpp"
#include "g_utilities.hpp"

LevelData_s::LevelData_s( int level, 
                         const std::string& name, 
                         const std::string& greeting, 
                         const std::string& commands )
{
    this->level = level;
    this->name = name;
    this->greeting = greeting;
    this->commands = commands;
}

LevelData_s::LevelData_s()
{
    this->level = 0;
}

LevelDatabase::LevelDatabase()
{

}

LevelDatabase::~LevelDatabase()
{

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
    if(str.length()) 
    {
        str.pop_back();
    }
}

void LevelDatabase::ReadLevels()
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
        char *file = new char[len + 1];
        char *file2 = file;

        trap_FS_Read(file, len, f);
        file[len] = 0;
        trap_FS_FCloseFile(f);

        char *token = NULL;
        bool levelOpen = false;
        LevelData tempLevel;

        levels_.clear();

        token = COM_Parse(&file);

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
                ReadString(&file, tempLevel->commands);
            }
            else if(!Q_stricmp(token, "level"))
            {
                ReadInt(&file, tempLevel->level);
            }
            else if(!Q_stricmp(token, "greeting"))
            {
                ReadString(&file, tempLevel->greeting);
            }
            else if(!Q_stricmp(token, "name"))
            {
                ReadString(&file, tempLevel->name);
            }
            else {
                G_LogPrintf("readconfig: parse error near %s on line %d",
                    token, COM_GetCurrentParseLine());
            }

            if(!Q_stricmp(token, "[level]"))
            {
                tempLevel = LevelData(new LevelData_s);
                levelOpen = true;
            }

            token = COM_Parse(&file);
        }

        if(levelOpen)
        {
            levels_.push_back(tempLevel);
        }


        delete[] file2;
    }


}

void LevelDatabase::WriteLevels()
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

    for(ConstLevelIter it = levels_.begin();
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

void LevelDatabase::CreateDefaultLevels()
{
    if(!g_admin.integer || g_levelConfig.string[0] == 0)
    {
        return;
    }

    LevelData tempLevel = LevelData(new LevelData_s(0, "Visitor", 
        "Welcome Visitor [n]^7!", ""));
    levels_.push_back(tempLevel);

    tempLevel = LevelData(new LevelData_s(1, "Friend", 
        "Welcome Friend [n]^7!", ""));
    levels_.push_back(tempLevel);

    tempLevel = LevelData(new LevelData_s(2, "Moderator",
        "Welcome Moderator [n]^7!", ""));
    levels_.push_back(tempLevel);

    tempLevel = LevelData(new LevelData_s(3, "Administrator",
        "Welcome Administrator [n]^7!", ""));
    levels_.push_back(tempLevel);

    WriteLevels();
}

std::string LevelDatabase::Permissions( int level )
{
    ConstLevelIter it = levels_.begin();

    while(it != levels_.end())
    {
        if(it->get()->level == level)
        {
            return it->get()->commands;
        }

        it++;
    }
    return "";
}

std::string LevelDatabase::Greeting( int level )
{
    ConstLevelIter it = levels_.begin();

    while(it != levels_.end())
    {
        if(it->get()->level == level)
        {
            return it->get()->greeting;
        }
        it++;
    }
    return "";
}

std::string LevelDatabase::Name( int level )
{
    ConstLevelIter it = levels_.begin();

    while(it != levels_.end())
    {
        if(it->get()->level == level)
        {
            return it->get()->name;
        }
        it++;
    }
    return "";
}

bool LevelDatabase::LevelExists( int level )
{
    ConstLevelIter it = levels_.begin();
    while(it != levels_.end())
    {
        if(it->get()->level == level)
        {
            return true;
        }
        it++;
    }
    return false;
}
