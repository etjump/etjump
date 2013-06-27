#include "g_database.hpp"
#include <algorithm>

bool Database::NewLevel( Level level )
{
	G_LogPrintf("Database::NewLevel is not yet implemented.\n");
	return true;
}

bool Database::NewUser( User user )
{
	std::pair<std::string, UserDataPtr> newUser;
    newUser.second = UserDataPtr(new UserData);
	newUser.first = user.guid;
	newUser.second->level = user.level;
	newUser.second->name = user.name;
	newUser.second->personalCommands = user.personalCommands;
	newUser.second->personalGreeting = user.personalGreeting;
	newUser.second->personalTitle = user.personalTitle;
	
	if(users_.insert(newUser).second == false) 
	{
		// User already exists
		return false;
	}

	WriteUserConfig();

	// user is now on db
	return true;
}

bool Database::NewBan( Ban ban )
{
	G_LogPrintf("Database::NewBan is not yet implemented.\n");
	return true;
}

bool Database::SaveLevel( const Level& level )
{
	G_LogPrintf("Database::SaveLevel is not yet implemented.\n");
	return true;
}

bool Database::SaveUser( const User& user )
{
	G_LogPrintf("Database::SaveUser is not yet implemented.\n");
	return true;
}

bool Database::SaveBan( const Ban& ban )
{
	G_LogPrintf("Database::SaveBan is not yet implemented.\n");
	return true;
}

LevelData Database::GetLevel( int level ) const
{
	static Level dummyLevel;
	dummyLevel.name = "Dummy level";
	dummyLevel.greeting = "Hello [n]^7!";
	dummyLevel.level = 0;
	dummyLevel.commands = "abc";
	return &dummyLevel;
}


BanData Database::GetBan() const
{
	return 0;
}

Database::~Database()
{

}

Database::Database()
{
	usersSorted_ = false;
}

bool Database::GetUser( const std::string& guid, ConstUserIterator& it ) const
{
	it = users_.find(guid);
	if(it != users_.end())
	{
		return true;
	}
	return false;
}

void WriteString(const char* toWrite, fileHandle_t& f)
{
	trap_FS_Write(toWrite, strlen(toWrite), f);
	trap_FS_Write("\n", 1, f);
}

void WriteInt(int toWrite, fileHandle_t& f)
{
#define BUFSIZE 32
	char buf[BUFSIZE];
	Com_sprintf(buf, sizeof(buf), "%d", toWrite);
	trap_FS_Write(buf, strlen(buf), f);
	trap_FS_Write("\n", 1, f);
}

void Database::WriteUserConfig()
{
	if(g_userConfig.string[0] == 0)
	{
		return;
	}

	fileHandle_t f = -1;
	
	if(trap_FS_FOpenFile(g_userConfig.string, &f, FS_WRITE) < 0)
	{
		G_LogPrintf("WriteConfig: failed to open user config file.\n");
		return;
	}

	ConstUserIterator it = users_.begin();

	while(it != users_.end())
	{
		trap_FS_Write("[user]\n", 7, f);
		trap_FS_Write("level = ", 8, f);
		WriteInt(it->second->level, f);

		trap_FS_Write("name = ", 7, f);
		WriteString(it->second->name.c_str(), f);

		trap_FS_Write("guid = ", 7, f);
		WriteString(it->first.c_str(), f);

		trap_FS_Write("cmds = ", 7, f);
		WriteString(it->second->personalCommands.c_str(), f);

		trap_FS_Write("greeting = ", 11, f);
		WriteString(it->second->personalGreeting.c_str(), f);

		trap_FS_Write("title = ", 8, f);
		WriteString(it->second->personalTitle.c_str(), f);
	
		trap_FS_Write("\n", 1, f);
		it++;
	}

	trap_FS_FCloseFile(f);
}

void ResetUserDataPair( std::pair<std::string, UserData>& tempUser ) 
{
	tempUser.first.clear();
	tempUser.second.level = 0;
	tempUser.second.name.clear();
	tempUser.second.personalCommands.clear();
	tempUser.second.personalGreeting.clear();
	tempUser.second.personalTitle.clear();
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

bool Database::ReadUserConfig(gentity_t *ent)
{
	if(!g_userConfig.string[0])
	{
		return false;
	}
	fileHandle_t f = -1;
	int len = trap_FS_FOpenFile(g_userConfig.string, &f, FS_READ);
	if(len < 0)
	{
		ChatPrintTo(ent, va("readconfig: failed to open %s.\n", g_userConfig.string));
		return false;
	}

	char *configFile = (char*)malloc(len+1);
	char *configFile2 = configFile;

	trap_FS_Read(configFile, len, f);
	configFile[len] = 0;	
	trap_FS_FCloseFile(f);

	char *token = NULL;
	bool userOpen = false;
    // UserDataPtr is initialized when a new user block is opened
    // while parsing .cfg so no need to do it now.
	std::pair<std::string, UserDataPtr> tempUser;

	users_.clear();

	token = COM_Parse(&configFile);
	while(*token)
	{
		if(!Q_stricmp(token, "[user]"))
		{
			if(userOpen)
			{
				users_.insert(tempUser);
			}
			userOpen = false;
		}

		if(userOpen)
		{
			if(!Q_stricmp(token, "level"))
			{
				ReadInt(&configFile, tempUser.second->level);
			}
			else if(!Q_stricmp(token, "name"))
			{
				ReadString(&configFile, tempUser.second->name);
			}
			else if (!Q_stricmp(token, "guid"))
			{
				ReadString(&configFile, tempUser.first);
			}
			else if (!Q_stricmp(token, "cmds"))
			{
				ReadString(&configFile, tempUser.second->personalCommands);
			}
			else if (!Q_stricmp(token, "greeting"))
			{
				ReadString(&configFile, tempUser.second->personalGreeting);
			}
			else if (!Q_stricmp(token, "title"))
			{
				ReadString(&configFile, tempUser.second->personalTitle);
			}
			else
			{
				ChatPrintTo(ent, va("^3readconfig: ^7parse error near %s on line %d",
					token, COM_GetCurrentParseLine()));
			}
		}
		if(!Q_stricmp(token, "[user]"))
		{
            tempUser.second = UserDataPtr(new UserData);
			userOpen = true;
		}

		token = COM_Parse(&configFile);
	}

	if(userOpen)
	{
		users_.insert(tempUser);
	}
	free(configFile2);
	ChatPrintTo(ent, va("^3readconfig: ^7loaded %d users.", users_.size()));

	return true;
}

void Database::DefaultLevels()
{
	levels_.clear();

	boost::shared_ptr<Level> levelToAdd(new Level(0, "Visitor", "", "Welcome [n]^7!"));
	levels_.push_back(levelToAdd);

	levelToAdd = boost::shared_ptr<Level>(new Level(1, "Regular", "", "Welcome Regular [n]^7!"));
	levels_.push_back(levelToAdd);

	levelToAdd = boost::shared_ptr<Level>(new Level(2, "Moderator", "", "Welcome Moderator [n]^7!"));
	levels_.push_back(levelToAdd);

	levelToAdd = boost::shared_ptr<Level>(new Level(3, "Admin", "", "Welcome Admin [n]^7!"));
	levels_.push_back(levelToAdd);

	WriteLevelConfig();
}

void Database::WriteLevelConfig()
{
	if(g_levelConfig.string[0] == 0)
	{
		return;
	}

	fileHandle_t f = -1;

	if(trap_FS_FOpenFile(g_levelConfig.string, &f, FS_WRITE) < 0)
	{
		G_LogPrintf("WriteConfig: failed to open level config file.\n");
		return;
	}

	ConstLevelIterator it = levels_.begin();
	
	while(it != levels_.end())
	{
		trap_FS_Write("[level]", 7, f);
		trap_FS_Write("level = ", 8, f);
		WriteInt(it->get()->level, f);

		trap_FS_Write("name = ", 7, f);
		WriteString(it->get()->name.c_str(), f);

		trap_FS_Write("cmds = ", 7, f);
		WriteString(it->get()->commands.c_str(), f);

		trap_FS_Write("greeting = ", 11, f);
		WriteString(it->get()->greeting.c_str(), f);
		it++;
	}
	trap_FS_FCloseFile(f);
}

bool Database::ReadLevelConfig( gentity_t *ent )
{
    if(g_levelConfig.string[0] == 0)
    {
        return false;
    }

    fileHandle_t f = -1;
    int len = trap_FS_FOpenFile(g_levelConfig.string, &f, FS_READ);
    if(len < 0)
    {
        ChatPrintTo(ent, va("readconfig: failed to open %s.\n", 
            g_levelConfig.string));
        return false;
    }
    else if(len == 0)
    {
        DefaultLevels();
    }
    else {
        char* file = (char*)malloc(len+1);
        char* file2 = file;

        trap_FS_Read(file, len, f);
        file[len] = 0;
        trap_FS_FCloseFile(f);

        char *token = NULL;
        bool levelOpen = false;
        LevelPtr tempLevel;

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
            }
            else if(!Q_stricmp(token, "cmds"))
            {

            }
            else if(!Q_stricmp(token, "level"))
            {

            }
            else if(!Q_stricmp(token, "greeting"))
            {

            }
            else if(!Q_stricmp(token, "name"))
            {

            }
            else {
                ChatPrintTo(ent, va("^3readconfig: ^7parse error near %s "
                    "on line %d", token, COM_GetCurrentParseLine()));

            }
            token = COM_Parse(&file);
        }
        free(file2);


    }
    return true;
}

void Database::PrintUsers()
{
    ConstUserIterator it = users_.begin();
    G_LogPrintf("User database: \n");
    while(it != users_.end())
    {
        char buf[MAX_TOKEN_CHARS] = "\0";

        Com_sprintf(buf, sizeof(buf), 
            "[user]\nname = %s\n"
            "level = %d\n"
            "guid = %s\n"
            "commands = %s\n"
            "greeting = %s\n"
            "title = %s\n",
            it->second->name.c_str(), 
            it->second->level, 
            it->first.c_str(),
            it->second->personalCommands.c_str(), 
            it->second->personalGreeting.c_str(), 
            it->second->personalTitle.c_str());

        G_LogPrintf("%s\n", buf);

        it++;
    }
}

Level::Level( int level, const std::string& name, const std::string& greeting, const std::string& commands )
{
	this->level = level;
	this->name = name;
	this->greeting = greeting;
	this->commands = commands;
}

Level::Level()
{
	this->level = 0;
	this->name.clear();
	this->greeting.clear();
	this->commands.clear();
}
