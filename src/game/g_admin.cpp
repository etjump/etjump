#include "g_local.hpp"
#include "g_admin.hpp"
#include "g_sessiondb.hpp"
#include "g_utilities.hpp"
#include "g_database.hpp"
#include <boost/algorithm/string/replace.hpp>

// We have just one static clientDB in the game module. 
// It stores authentication information like GUID & HWID
// of clients.
SessionDB sessionDB;

// Same thing with User, Level & BanDB
Database adminDB;

void PrintGreeting( gentity_t * ent ) 
{
	std::string greeting = sessionDB.Greeting(ent);

    // Print level greeting instead
	if(greeting.length() == 0)
	{
        // Greeting is checked on sessionDB.Set() and will always have 
        // a) personal greeting b) level greeting c) no greeting so if
        // there's none don't do anything.
        return;
	}
    else
    {
        boost::replace_all(greeting, "[n]", ent->client->pers.netname);    

        ChatPrintAll(greeting);
    }

	
}

bool ValidGuid(const std::string& guid);
// Called when "etguid" command is executed by the client
// Parses the command, checks for valid values and saves the 
// GUID to db.
void GuidReceived( gentity_t * ent ) 
{
	Arguments argv = GetArgs();
	// Get the userinfo for IP information etc.
	char userinfo[MAX_INFO_STRING] = "\0";
	trap_GetUserinfo(ent->client->ps.clientNum, 
		userinfo, sizeof(userinfo));
	// For userinfo value
	char *value = NULL;

	value = Info_ValueForKey(userinfo, "ip");

	// Command vector contains just etguid GUID, nothing else,
	// so let's not accept anything else.
	if(argv->size() != 2)
	{
		G_LogPrintf("Possible GUID spoof attempt(no GUID) by: %s. IP: %s.\n",
			ent->client->pers.netname, value);
		return;

	}

	// Make sure the GUID is valid
	if(!ValidGuid(argv->at(1)))
	{
		G_LogPrintf("Possible GUID spoof attempt(invalid GUID) by: %s. IP %s.\n",
			ent->client->pers.netname, value);
		return;

	}

	// Let's hash the GUID again.
	char *hashedGuid = G_SHA1(argv->at(1).c_str());
	if(!hashedGuid) 
	{
		G_LogPrintf("Error while trying to hash GUID.\n");
		return;

	}
	std::string guid = hashedGuid;

	// We now know that GUID is valid so we can happily save it to
	// client db
	// When we receive a GUID we also need to get the level of client
	// and the level data from the db
	// 
	// Get admin data
	ConstUserIterator userAdminData;
	if(adminDB.GetUser(hashedGuid, userAdminData)) 
	{
		// We now have the admin data and the GUID so we can save them all
		// to the temporary clientDB
		// 
		sessionDB.Set(ent, hashedGuid, userAdminData->second->level,
			userAdminData->second->name, userAdminData->second->personalCommands,
			userAdminData->second->personalGreeting, userAdminData->second->personalTitle);
	}
	else
	{
		// First time we saw the user, give the default level settings
		User user;
		user.guid = hashedGuid;
		user.level = 0;
		CharPtrToString(ent->client->pers.netname, user.name);
		// Add the user to DB and write the data to file
		adminDB.NewUser(user);
	
		// Add it to clientDB as well.
		sessionDB.Set(ent, hashedGuid, user.level, user.name, user.personalCommands,
			user.personalGreeting, user.personalTitle);
	}

	// Print greeting if it's needed 
	if(ent->client->sess.needGreeting)
	{
		PrintGreeting(ent);
	}

	// We've now added client to permanent & session database 
	// and printed greeting. 

}

bool ValidGuid(const std::string& guid)
{
	// Make sure length is correct
	if(guid.length() != 40)
	{
		// Length was not correct
		return false;

	}

	// Check each character for illegal chars
	for(int i = 0; i < guid.length(); i++)
	{
		// It's a sha hashed guid converted to hexadecimal. If values
		// greater than F are found in it, it's a fake guid.
		if(guid[i] < '0' || guid[i] > 'F')
		{
			return false;

		}

	}

	return true;

}

void PrintUserDB()
{
    adminDB.PrintUsers();
}

bool TargetIsHigherLevel(gentity_t *ent, gentity_t *target, bool equalIsHigher = false)
{
    if(equalIsHigher)
    {
        return sessionDB.Level(target) >= sessionDB.Level(ent);
    } else
    {
        return sessionDB.Level(target) > sessionDB.Level(ent);
    }
}

namespace AdminCommands
{
    bool Admintest(gentity_t *ent, Arguments argv)
    {
        if(!ent)
        {
            ChatPrintAll("^3admintest: ^7console is a level 2147483647 user "
                "(Why?)");
            return true;
        }

        ConstLevelIterator it;
        if(!adminDB.GetLevel(sessionDB.Level(ent), it))
        {
            return false;
        }
        ChatPrintAll(va("^3admintest: ^7%s^7 is a level %d user (%s^7)",
            ent->client->pers.netname, 
            it->get()->level, it->get()->name.c_str()));
        return true;
    }

    bool Setlevel(gentity_t *ent, Arguments argv)
    {
        // !setlevel player level
        // !setlevel -id id level
        if(argv->size() < 3)
        {
            ChatPrintTo(ent, "^3usage: ^7!setlevel <player> <level>");
            ChatPrintTo(ent, "^3usage: ^7!setlevel -id <id> <level>");
            return false;
        } else if(argv->size() == 3)
        {
            // Normal setlevel
            char err[MAX_TOKEN_CHARS] = "\0";
            gentity_t *target = PlayerGentityFromString(argv->at(1), 
                err, sizeof(err));

            if(!target)
            {
                ChatPrintTo(ent, err);
                return false;
            }

            int level = 0;
            if(!StringToInt(argv->at(2), level))
            {
                ChatPrintTo(ent, "^3setlevel: ^7invalid level " + argv->at(2));
                return false;
            }

            if(TargetIsHigherLevel(ent, target)) 
            {
                ChatPrintTo(ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
                return false;
            }

            if(level > sessionDB.Level(ent))
            {
                ChatPrintTo(ent, "^3setlevel: ^7you're not allowed to setlevel higher than your own level.");
                return false;
            }

            if(!adminDB.LevelExists(level))
            {
                ChatPrintTo(ent, "^3setlevel: ^7level does not exist.");
                return false;
            }

            adminDB.UpdateUserLevel(sessionDB.Guid(target), level);
            ChatPrintAll(va("^3setlevel: ^7%s is now a level %d user (%s)", 
                target->client->pers.netname,
                level, 
                adminDB.LevelName(level).c_str()));
        } else if(argv->size() == 4)
        {
            // Set by ID
            // !setlevel -id 0 1
            if(argv->at(1) != "-id")
            {
                ChatPrintTo(ent, "^3usage: ^7!setlevel -id <id> <level>");
                return false;
            }

            int id = 0;
            if(!StringToInt(argv->at(2), id))
            {
                ChatPrintTo(ent, "^3setlevel: ^7invalid id " + argv->at(2));
                return false;
            }

            UserIterator it;
            if(!adminDB.GetUser(id, it))
            {
                ChatPrintTo(ent, "^3setlevel: ^7couldn't find user with id " + argv->at(2));
                return false;
            }

            int level = 0;
            if(!StringToInt(argv->at(3), level))
            {
                ChatPrintTo(ent, "^3setlevel: ^7invalid level " + argv->at(3));
                return false;
            }

            if(!adminDB.LevelExists(level))
            {
                ChatPrintTo(ent, "^3setlevel: ^7level does not exist.");
                return false;
            }

            if(level > sessionDB.Level(ent))
            {
                ChatPrintTo(ent, "^3setlevel: ^7you're not allowed to setlevel higher than your own level.");
                return false;
            }

            if(it->second->level > sessionDB.Level(ent))
            {
                ChatPrintTo(ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
                return false;
            }

            it->second->level = level;

            ChatPrintTo(ent, va("^3setlevel: ^7%s is now a level %d user (%s)", 
                it->second->name.c_str(),
                level, 
                adminDB.LevelName(level).c_str()));
            
        } else {
            ChatPrintTo(ent, "^3usage: ^7!setlevel <player> <level>");
            ChatPrintTo(ent, "^3usage: ^7!setlevel -id <id> <level>");
        }

        return true;
    }
}

static AdminCommand_s AdminCommandList[] =
{
    {
        "admintest", AdminCommands::Admintest, 'a', "", ""
    }
    ,
    {
        "setlevel", AdminCommands::Setlevel, 's', "", ""
    }
    ,
    {
        "", 0, 0, "", ""
    }
};



qboolean CheckCommand( gentity_t *ent )
{
    // say "!command additional args"

    if(g_admin.integer == 0)
    {
        return qfalse;
    }

    std::string command = "";
    std::string arg = SayArgv(0);
    int skip = 0;
    if( arg == "say" )
    {
        // TODO: team chat commands
        arg = SayArgv(1);
        skip = 1;
    }

    Arguments argv = GetSayArgs(skip);

    // 2nd arg doesn't exist
    if(arg.length() == 0)
    {
        return qfalse;
    }

    if(arg[0] == '!')
    {
        command = &arg[1];
    }
    else if (ent == NULL)
    {
        command = arg;
    }
    else {
        return qfalse;
    }

    for(size_t i = 0; AdminCommandList[i].flag != 0; i++)
    {
        if(command != AdminCommandList[i].keyword)
        {
            continue;
        } else
        {
            if(sessionDB.HasPermission(ent, AdminCommandList[i].flag))
            {
                AdminCommandList[i].handler(ent, argv);
                return qtrue;  
            } else {
                ChatPrintTo(ent, va("^3%s: ^7permission denied", 
                    AdminCommandList[i].keyword.c_str()));
                return qtrue;
            }
        }
    }

    return qfalse;
}