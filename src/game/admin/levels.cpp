#include <boost/algorithm/string.hpp>
#include <boost/shared_array.hpp>
#include "levels.hpp"
#include "../g_local.hpp"
#include "../g_utilities.hpp"

Levels::Level::Level(int level, std::string const& name, std::string const& commands, std::string const& greeting)
{
    this->level = level;
    this->name = name;
    this->greeting = greeting;
    this->commands = commands;
}

Levels::Levels()
{
    dummyLevel_ = boost::shared_ptr<Level>(new Level(0, "", "", ""));
}

Levels::~Levels()
{
}

bool Levels::Add(int level, std::string const name, std::string const commands, std::string const greeting)
{
    ConstIter it = Find(level);
    if (it != levels_.end())
    {
        errorMessage = "level exists";
        return false;
    }

    boost::shared_ptr<Level> levelPtr(new Level(level, name, greeting, commands));
    levels_.push_back(levelPtr);

    if (WriteToConfig())
    {
        return false;
    }
    return true;
}

bool Levels::Edit(int level, std::string const& name, std::string const& commands, std::string const& greeting, int updated)
{
    ConstIter it = Find(level);
    if (it == levels_.end())
    {
        errorMessage = "level does not exist";
        return false;
    }

    const int CMDS_UPDATED = 1;
    const int GREETING_UPDATED = 2;
    const int NAME_UPDATED = 4;

    if (updated & CMDS_UPDATED)
    {
        it->get()->commands = commands;
    }

    if (updated & GREETING_UPDATED)
    {
        it->get()->greeting = greeting;

    }

    if (updated & NAME_UPDATED)
    {
        it->get()->name = name;

    }

    if (!WriteToConfig())
    {
        return false;
    }
    return true;
}

bool Levels::CreateDefaultLevels()
{
    levels_.clear();

    boost::shared_ptr< Level > tempLevel = boost::shared_ptr< Level >
        (new Level(0, "Visitor",
        "Welcome Visitor [n]^7! Your last visit was on [t]!", "8aS"));
    levels_.push_back(tempLevel);

    tempLevel = boost::shared_ptr< Level >
        (new Level(1, "Friend",
        "Welcome Friend [n]^7! Your last visit was [d] ago!", "128Sa"));
    levels_.push_back(tempLevel);

    tempLevel = boost::shared_ptr< Level >
        (new Level(2, "Moderator",
        "Welcome Moderator [n]^7!§", "*-AGs"));
    levels_.push_back(tempLevel);

    tempLevel = boost::shared_ptr< Level >
        (new Level(3, "Administrator",
        "Welcome Administrator [n]^7!", "*"));
    levels_.push_back(tempLevel);

    if (!WriteToConfig())
    {
        return false;
    }
    return true;
}

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

bool Levels::SortByLevel(const boost::shared_ptr<Level> lhs, const boost::shared_ptr<Level> rhs)
{
    return lhs->level < rhs->level;
}

bool Levels::WriteToConfig()
{
    fileHandle_t f = 0;
    if (trap_FS_FOpenFile(g_levelConfig.string, &f, FS_WRITE) < 0)
    {
        errorMessage = std::string("Couldn't open file \"") + g_levelConfig.string + "\" to save admin levels.";
        return false;
    }

    std::sort(levels_.begin(), levels_.end(), SortByLevel);

    for (ConstIter it = levels_.begin(); it != levels_.end(); it++)
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
    return true;
}

Levels::Level const* Levels::GetLevel(int level)
{
    std::vector<boost::shared_ptr<Level>>::iterator it = levels_.begin();
    std::vector<boost::shared_ptr<Level>>::iterator end = levels_.end();
    for (; it != end; it++)
    {
        if (it->get()->level == level)
        {
            return it->get();
        }
    }
    

    return dummyLevel_.get();
}

bool Levels::LevelExists(int level) const
{
    ConstIter it = levels_.begin();

    for (; it != levels_.end(); it++)
    {
        if (it->get()->level == level)
        {
            return true;
        }
    }
    return false;
}

void Levels::PrintLevels()
{
    ConstIter it = levels_.begin();

    std::string level;

    for (; it != levels_.end(); it++)
    {
        level += it->get()->level + "\n" +
            it->get()->name + "\n" +
            it->get()->commands + "\n" +
            it->get()->greeting + "\n";

        G_LogPrintf(level.c_str());

        level = "";
    }
}

std::string Levels::ErrorMessage()
{
    return errorMessage;
}

Levels::ConstIter Levels::Find(int level)
{
    ConstIter it = levels_.begin();
    for (; it != levels_.end(); it++)
    {
        if (it->get()->level == level)
        {
            return it;
        }
    }
    return it;
}

void ReadInt(char ** configFile, int& level)
{
    char *token = COM_ParseExt(configFile, qfalse);

    if (!Q_stricmp(token, "="))
    {
        token = COM_ParseExt(configFile, qfalse);
    }
    else
    {
        G_LogPrintf("readconfig: missing = before \"%s\" on line %d.",
            token, COM_GetCurrentParseLine());
    }
    level = atoi(token);
}

void ReadString(char** configFile, std::string& str)
{
    char *token = COM_ParseExt(configFile, qfalse);

    if (!Q_stricmp(token, "="))
    {
        token = COM_ParseExt(configFile, qfalse);
    }
    else
    {
        G_LogPrintf("readconfig: missing = before \"%s\" on line %d.",
            token, COM_GetCurrentParseLine());
    }
    str.clear();
    while (token[0])
    {
        str += token;
        str.push_back(' ');
        token = COM_ParseExt(configFile, qfalse);
    }

    boost::trim_right(str);
}

bool Levels::ReadFromConfig()
{
    fileHandle_t f = 0;
    int len = trap_FS_FOpenFile(g_levelConfig.string, &f, FS_READ);
    if (len < 0)
    {
        CreateDefaultLevels();
        return true;
    }
    else
    {
        boost::shared_array<char> file;
        try
        {
            file = boost::shared_array<char>(new char[len + 1]);
        }
        catch (std::bad_alloc& e)
        {
            G_Error("Failed to allocate memory to parse level config.");
            trap_FS_FCloseFile(f);
            return false;
        }

        trap_FS_Read(file.get(), len, f);
        file[len] = 0;
        trap_FS_FCloseFile(f);

        char *token = NULL;
        bool levelOpen = false;
        boost::shared_ptr<Level> tempLevel;

        levels_.clear();

        char *file2 = file.get();

        token = COM_Parse(&file2);

        while (*token)
        {
            if (!Q_stricmp(token, "[level]"))
            {
                if (levelOpen)
                {
                    levels_.push_back(tempLevel);
                }
                levelOpen = false;
            }
            else if (!Q_stricmp(token, "cmds"))
            {
                ReadString(&file2, tempLevel->commands);
            }
            else if (!Q_stricmp(token, "level"))
            {
                ReadInt(&file2, tempLevel->level);
            }
            else if (!Q_stricmp(token, "greeting"))
            {
                ReadString(&file2, tempLevel->greeting);
            }
            else if (!Q_stricmp(token, "name"))
            {
                ReadString(&file2, tempLevel->name);
            }
            else {
                G_LogPrintf("readconfig: parse error near %s on line %d",
                    token, COM_GetCurrentParseLine());
            }

            if (!Q_stricmp(token, "[level]"))
            {
                try
                {
                    tempLevel = boost::shared_ptr<Level>(new Level(0, "", "", ""));
                }
                catch (std::bad_alloc& e)
                {
                    G_Error("Failed to allocate memory for a level.");
                    return false;
                }

                levelOpen = true;
            }

            token = COM_Parse(&file2);
        }

        if (levelOpen)
        {
            levels_.push_back(tempLevel);
        }
    }
    return true;
}

//bool Levels::Delete(int level)
//{
//    ConstIter it = Find(level);
//    bool deleted = false;
//    if (it != levels_.end())
//    {
//        levels_.erase(it);
//        deleted = true;
//    }
//    if (!deleted)
//    {
//        errorMessage = "couldn't find level" + ToString(level);
//        return false;
//    }
//    return true;
//}