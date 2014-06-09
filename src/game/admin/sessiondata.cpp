#include "sessiondata.h"
#include "common.h"
#include "../g_utilities.hpp"
#include <boost/algorithm/string.hpp>

SessionData::Client::Client()
    : id(0), level(0), lastSeen(0), guid(""), name(""), hwid(""), title(""), commands(""), 
    greeting("")
{
    permissions.reset();
}

SessionData::SessionData( IUserData *userData, ILevelData *levelData, IBanData *banData )
    : userData_( userData ), levelData_( levelData ), banData_( banData )
{

}

SessionData::~SessionData()
{

}

void SessionData::Reset(gentity_t* ent)
{
    clients_[ent->client->ps.clientNum].commands.clear();
    clients_[ent->client->ps.clientNum].greeting.clear();
    clients_[ent->client->ps.clientNum].guid.clear();
    clients_[ent->client->ps.clientNum].hwid.clear();
    clients_[ent->client->ps.clientNum].id = 0;
    clients_[ent->client->ps.clientNum].lastSeen = 0;
    clients_[ent->client->ps.clientNum].level = 0;
    clients_[ent->client->ps.clientNum].name.clear();
    clients_[ent->client->ps.clientNum].permissions.reset();
}

bool SessionData::EditUser(gentity_t *toPrint, std::string const& guid, std::string const& title, std::string const& commands, std::string const& greeting, int updated)
{
    if(!userData_->UpdateUser(guid, commands, greeting, title, updated))
    {
        ChatPrintTo(toPrint, "^3edituser: ^7couldn't find user with guid \"" + guid + "\".");
        return false;
    }

    // Now we know there was only 1 match
    for(int i = 0; i < level.numConnectedClients; i++)
    {
        int clientNum = level.sortedClients[i];
        if(clients_[clientNum].guid.compare(0, guid.length(), guid) == 0)
        {
            if(updated & CMDS_OPEN)
            {
                const ILevelData::LevelInformation *levelInfo = 
                levelData_->GetLevelInformation(clients_[clientNum].level);

                clients_[clientNum].commands = commands;
                clients_[clientNum].permissions.reset();

                ParsePermissions(clients_[clientNum].permissions,
                    levelInfo->commands + clients_[clientNum].commands);                
            }

            if(updated & GREETING_OPEN)
            {
                clients_[clientNum].greeting = greeting;
            }

            if(updated & TITLE_OPEN)
            {
                clients_[clientNum].title = title;
            }
            break;
        }
    }

    return true;
}

bool SessionData::BanPlayer(gentity_t* ent, 
                            gentity_t *target, 
                            int seconds, const std::string& 
                            reason)
{
    std::string guid = clients_[target->client->ps.clientNum].guid;
    User const * user = userData_->GetUserData(guid);
    if(!user)
    {
        ChatPrintTo(ent, "^3ban: ^7couldn't find user with guid " + guid + ".");
        return false;
    }

    time_t t;
    if(!time(&t))
    {
        ChatPrintTo(ent, "^3ban: ^7couldn't get current time.");
        return false;
    }

    std::string ip = ValueForKey(target, "ip");
    std::string::size_type index = ip.find(":");
    if(index != std::string::npos)
    {
        ip = ip.substr(0, index);
    }

    std::string banner = "console";
    if(ent)
    {
        banner = ent->client->pers.netname;
    }
    std::string err;
    if(seconds)
    {
        if(!banData_->AddBan(guid, ip, user->hwid, static_cast<int>(t) + seconds,
        static_cast<int>(t), target->client->pers.netname, banner.c_str(), reason, err))
        {
            ChatPrintTo(ent, "^3ban: ^7" + err);
            return false;
        }
    } else
    {
        if(!banData_->AddBan(guid, ip, user->hwid, 0,
        static_cast<int>(t), target->client->pers.netname, banner.c_str(), reason, err))
        {
            ChatPrintTo(ent, "^3ban: ^7" + err);
            return false;
        }
    }
    

    if(seconds == 0)
    {
        ChatPrintTo(ent, va("^3ban: ^7banned player %s permanently for %s", target->client->pers.netname, 
            reason.c_str()));
    } else
    {
        ChatPrintTo(ent, va("^3ban: ^7banned player %s until %s for %s", target->client->pers.netname, 
            TimeStampToString(seconds).c_str(), reason.c_str()));
    }

    return true;
}

bool SessionData::BanPlayer(gentity_t* ent, 
                            std::string const& guid, 
                            int seconds, std::string 
                            const& reason)
{
    User const * user = userData_->GetUserData(guid);
    if(!user)
    {
        ChatPrintTo(ent, "^3ban: ^7couldn't find user with guid " + guid + ".");
        return false;
    }

    if(ent)
    {
        User const *banner = userData_->GetUserData(clients_[ent->client->ps.clientNum].guid);
        if(banner)
        {
            if(banner->level <= user->level)
            {
                ChatPrintTo(ent, "^3ban: ^7cannot ban a fellow admin.");
                return false;
            }
        }
    }
    

    time_t t;
    if(!time(&t))
    {
        ChatPrintTo(ent, "^3ban: ^7couldn't get current time.");
        return false;
    }

    std::string ip = "NOTCONNECTED";

    std::string err;
    if(!banData_->AddBan(guid, ip, user->hwid, static_cast<int>(t) + seconds,
        static_cast<int>(t), user->name.c_str(), ent->client->pers.netname, reason, err))
    {
        ChatPrintTo(ent, "^3ban: ^7" + err);
        return false;
    }
    return true;
}

void SessionData::UpdatePermissions()
{
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        const ILevelData::LevelInformation *levelInfo = 
            levelData_->GetLevelInformation(clients_[i].level);

        ParsePermissions(clients_[i].permissions, levelInfo->commands + clients_[i].commands);
    }
}

bool SessionData::ValidGuid( const std::string& guid )
{
    const unsigned GUID_SIZE = 40;
    if(guid.size() != GUID_SIZE)
    {
        return false;
    }

    for( size_t i = 0; i < guid.size(); i++)
    {
        if(guid[i] < '0' || guid[i] > 'F')
        {
            return false;
        }
    }
    return true;
}

bool SessionData::GuidReceived( gentity_t *ent )
{
    int argc = trap_Argc();
    char guidBuffer[MAX_TOKEN_CHARS] = "\0";
    char hwidBuffer[MAX_TOKEN_CHARS] = "\0";

    // Client sends "etguid <guid> <hwid>"
    if(argc != 3)
    {
        G_LogPrintf("Possible guid/hwid spoof attempt by: %s (%s).\n", ent->client->pers.netname, ClientIPAddr( ent ));
        return false;
    }

    trap_Argv(1, guidBuffer, sizeof(guidBuffer));
    trap_Argv(2, hwidBuffer, sizeof(hwidBuffer));

    if(!ValidGuid(guidBuffer) || !ValidGuid(hwidBuffer))
    {
        G_LogPrintf("Possible guid/hwid spoof attempt by: %s (%s).\n", ent->client->pers.netname, ClientIPAddr( ent ));
        return false;
    }

    clients_[ent->client->ps.clientNum].guid = G_SHA1(guidBuffer);
    clients_[ent->client->ps.clientNum].hwid = G_SHA1(hwidBuffer);

    GetUserData( ent, clients_[ent->client->ps.clientNum].guid );

    std::string ip = ValueForKey(ent, "ip");
    std::string::size_type i = ip.find(":");
    if(i != std::string::npos)
    {
        ip = ip.substr(0, i);
    }

    std::string err;
    if(banData_->Banned(clients_[ent->client->ps.clientNum].guid,
        ip,
        clients_[ent->client->ps.clientNum].hwid, err))
    {
        trap_DropClient(ent->client->ps.clientNum, "You are banned.", 0);
    }

    return true;
}



void SessionData::GetUserData( gentity_t *ent, const std::string& guid )
{
    const User *currentUser = userData_->GetUserData( guid );

    if(!currentUser)
    {
        // user does not exist. Create a new one
        clients_[ent->client->ps.clientNum].id = 
            userData_->CreateNewUser( guid, ent->client->pers.netname, clients_[ent->client->ps.clientNum].hwid );
    } else
    {
        const ILevelData::LevelInformation *levelInfo = 
            levelData_->GetLevelInformation(currentUser->level);

        clients_[ent->client->ps.clientNum].id = currentUser->id;
        clients_[ent->client->ps.clientNum].level = currentUser->level;
        clients_[ent->client->ps.clientNum].name = currentUser->name;
        if(currentUser->title.length() > 0)
        {
            clients_[ent->client->ps.clientNum].title = currentUser->title;
        } else
        {
            clients_[ent->client->ps.clientNum].title = levelInfo->name;
        }
        
        clients_[ent->client->ps.clientNum].commands = currentUser->commands;
        
        if(currentUser->greeting.length() > 0)
        {
            clients_[ent->client->ps.clientNum].greeting = currentUser->greeting;
        } else
        {
            clients_[ent->client->ps.clientNum].greeting = levelInfo->greeting;
        }
        
        ParsePermissions(clients_[ent->client->ps.clientNum].permissions, levelInfo->commands + currentUser->commands);

        if(ent->client->sess.needGreeting)
        {
            std::string welcomeMsg;
            CharPtrToString( g_lastVisitedMessage.string, welcomeMsg );
            boost::replace_all( welcomeMsg, "[t]", TimeStampToString(currentUser->lastSeen));
            CPMTo(ent, welcomeMsg);
            ent->client->sess.needGreeting = qfalse;
            
            if(clients_[ent->client->ps.clientNum].greeting.length() > 0)
            {
                std::string greeting = clients_[ent->client->ps.clientNum].greeting;
                boost::replace_all( greeting, "[n]", ent->client->pers.netname );
                ChatPrintAll( greeting );
            }
        }
        
    }

    userData_->UserIsOnline(guid);
}

std::string SessionData::GetGuid( gentity_t *ent )
{
    return clients_[ent->client->ps.clientNum].guid;
}

void SessionData::PrintAdmintest( gentity_t *ent )
{
    const ILevelData::LevelInformation *level = 
        levelData_->GetLevelInformation(clients_[ent->client->ps.clientNum].level);
    ChatPrintAll(va("^3admintest: ^7%s^7 is a level %d user (%s^7)",
        ent->client->pers.netname, clients_[ent->client->ps.clientNum].level, 
        level->name.c_str()));
}

int SessionData::GetLevel( gentity_t *ent )
{
    return clients_[ent->client->ps.clientNum].level;
}

void SessionData::PrintFinger( gentity_t *toPrint, gentity_t *target )
{
    const ILevelData::LevelInformation *level = 
        levelData_->GetLevelInformation(clients_[target->client->ps.clientNum].level);
    if(clients_[target->client->ps.clientNum].name.length() > 0)
    {
        ChatPrintTo(toPrint, va("^3finger: ^7%s^7 (%s^7) is a level %d user (%s^7)",
            target->client->pers.netname, 
            clients_[target->client->ps.clientNum].name.c_str(),
            clients_[target->client->ps.clientNum].level,
            level->name.c_str()));
    } else
    {
        ChatPrintTo(toPrint, va("^3finger: ^7%s^7 is a level %d user (%s^7)",
            target->client->pers.netname, 
            clients_[target->client->ps.clientNum].level,
            level->name.c_str()));
    }
    ConsolePrintTo(toPrint, "^3GUID: ^7" + clients_[target->client->ps.clientNum].guid.substr(0, 8));
    ConsolePrintTo(toPrint, "^3HWID: ^7" + clients_[target->client->ps.clientNum].hwid);
    ConsolePrintTo(toPrint, "^3Title: ^7" + clients_[target->client->ps.clientNum].title);
    ConsolePrintTo(toPrint, "^3Commands: ^7" + clients_[target->client->ps.clientNum].commands);
    ConsolePrintTo(toPrint, "^3Greeting: ^7" + clients_[target->client->ps.clientNum].greeting);
}

std::bitset<SessionData::Client::MAX_COMMANDS> 
    SessionData::GetPermissions(gentity_t* ent)
{
    if(!ent)
    {
         std::bitset<SessionData::Client::MAX_COMMANDS> temp;
         for(int i = 0; i < SessionData::Client::MAX_COMMANDS; i++)
         {
             temp[i] = true;
         }
         return temp;
    }
    return clients_[ent->client->ps.clientNum].permissions;
}

bool SessionData::SetLevel( gentity_t *target, int level )
{
    const ILevelData::LevelInformation *levelInfo = 
        levelData_->GetLevelInformation(level);

    if(!levelInfo->isValid)
    {
        return false;
    }

    clients_[target->client->ps.clientNum].level = level;
    UpdateUserSessionData(target);
    userData_->UpdateLevel(clients_[target->client->ps.clientNum].guid, level);
    return true;
}

void SessionData::UpdateUserSessionData( gentity_t *ent )
{
    const ILevelData::LevelInformation *levelInfo = 
        levelData_->GetLevelInformation(clients_[ent->client->ps.clientNum].level);

    const User* user = userData_->GetUserData(clients_[ent->client->ps.clientNum].guid);

    if(!user)
    {
        G_LogPrintf("ERROR: couldn't find user with guid: %s\n", clients_[ent->client->ps.clientNum].guid.c_str());
        return;
    }

    if(user->title.length() > 0)
    {
        clients_[ent->client->ps.clientNum].title = user->title;
    } else
    {
        clients_[ent->client->ps.clientNum].title = levelInfo->name;
    }

    if(user->greeting.length() > 0)
    {
        clients_[ent->client->ps.clientNum].greeting = user->greeting;
    } else
    {
        clients_[ent->client->ps.clientNum].greeting = levelInfo->greeting;
    }

    ParsePermissions(clients_[ent->client->ps.clientNum].permissions, 
        levelInfo->commands + user->commands);
}
enum {
    ALLOW,
    DENY
};
void SessionData::ParsePermissions(std::bitset<SessionData::Client::MAX_COMMANDS>& temp, std::string const& permissions)
{
    

	int state = ALLOW;

	for(size_t i = 0; i < permissions.size(); i++)
	{
		char c = permissions.at(i);
		if(state == ALLOW)
		{
			if(c == '*')
			{
				// Allow all commands
				for(size_t i = 0; i < Client::MAX_COMMANDS; i++)
				{
					temp.set(i, true);
				}
			}
			else if (c == '+')
			{
				// ignore +
				continue;
			}
			else if (c == '-')
			{
				state = DENY;
			}
			else
			{
				temp.set(c, true);
			}
		}
		else
		{
			if(c == '*')
			{
				// Ignore * while in deny-mode
				continue;
			}
			else if(c == '+')
			{
				state = ALLOW;
			}
			else
			{
				temp.set(c, false);
			}
		}
	}
}

void SessionData::PrintUserList(gentity_t* playerToPrintTo, int page)
{
    // ^5User list Page 1/n
    // len 8    15    10        15       36
    // ^5GUID | IP | Level | Last Seen | Name 
    const std::string HEADER =
        "^5__GUID__|__Level___|___Last Seen__|_____Name____\n";

    std::vector<std::string> linesToPrint;
    if(!userData_->UserListData(page, linesToPrint))
    {
        // Out of range
        ConsolePrintTo(playerToPrintTo, "^3userlist: ^7page doesn't exist.");
        return;
    }

    BeginBufferPrint();
    BufferPrint(playerToPrintTo, HEADER);
    for(unsigned i = 0; i < linesToPrint.size(); i++)
    {
        BufferPrint(playerToPrintTo, linesToPrint.at(i));
    }
    
    FinishBufferPrint(playerToPrintTo, false);
}

void SessionData::UpdateLastSeen(gentity_t* ent)
{
    time_t t = 0;
    if(!time(&t))
    {
        G_LogPrintf("ERROR: Couldn't get time for GuidReceived.\n");
    }
    clients_[ent->client->ps.clientNum].lastSeen = static_cast<int>(t);
    userData_->UpdateLastSeen( clients_[ent->client->ps.clientNum].id, static_cast<int>(t) );
}

void SessionData::PrintUserinfo( gentity_t *ent, gentity_t *target )
{
    ChatPrintTo(ent, "^3userinfo: ^7check console for more information.");
    BeginBufferPrint();
    BufferPrint(ent, va("name: %s\n", clients_[target->client->ps.clientNum].name.c_str()));
    BufferPrint(ent, va("id: %d\n", clients_[target->client->ps.clientNum].id));
    BufferPrint(ent, va("guid: %s\n", clients_[target->client->ps.clientNum].guid.c_str()));
    BufferPrint(ent, va("hwid: %s\n", clients_[target->client->ps.clientNum].hwid.c_str()));
    BufferPrint(ent, va("level: %d\n", clients_[target->client->ps.clientNum].level));
    BufferPrint(ent, va("title: %s\n", clients_[target->client->ps.clientNum].title.c_str()));
    BufferPrint(ent, va("commands: %s\n", clients_[target->client->ps.clientNum].commands.c_str()));
    BufferPrint(ent, va("greeting: %s\n", clients_[target->client->ps.clientNum].greeting.c_str()));    
    FinishBufferPrint(ent, false);
}
