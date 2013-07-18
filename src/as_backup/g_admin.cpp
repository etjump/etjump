#include "g_local.hpp"
#include "g_admin.hpp"
#include "database.hpp"
#include "g_sessiondb.hpp"
#include "g_utilities.hpp"
#include <boost/algorithm/string/replace.hpp>

// We have just one static clientDB in the game module. 
// It stores authentication information like GUID & HWID
// of clients.
SessionDB sessionDB;

Database persistentDB;


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

void GuidReceived(gentity_t *ent)
{
    Arguments argv = GetArgs();
    char userinfo[MAX_INFO_STRING] = "\0";
    trap_GetUserinfo(ent->client->ps.clientNum,
        userinfo, sizeof(userinfo));

    char *value = Info_ValueForKey(userinfo, "ip");

    if(argv->size() != 2)
    {
        G_LogPrintf("Possible attempt of spoofing GUID (no GUID) by %s|%s\n",
            ent->client->pers.netname, value);
        return;
    }

    if(!ValidGuid(argv->at(1)))
    {
        G_LogPrintf("Possible attempt of spoofing GUID (invalid GUID) by %s|%s\n",
            ent->client->pers.netname, value);
        return;
    }

    char *hashedGuid = G_SHA1(argv->at(1).c_str());
    if(!hashedGuid)
    {
        G_LogPrintf("Couldn't hash %s's GUID.\n",
            ent->client->pers.netname);
        return;
    }

    // This is a valid GUID
    std::string guid = hashedGuid;

    persistentDB.ClientConnect(ent, guid);

    if(ent->client->sess.needGreeting)
    {
        PrintGreeting(ent);
    }
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

static AdminCommand_s AdminCommandList[] =
{
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
