#include "session.hpp"
#include "../g_utilities.hpp"
#include "database.hpp"
#include "levels.hpp"

Session::Session()
{
    for (unsigned i = 0; i < MAX_CLIENTS; i++)
    {
        ResetClient(i);
    }
}

void Session::ResetClient(int clientNum)
{
    clients_[clientNum].guid = "";
    clients_[clientNum].hwid = "";
    clients_[clientNum].user = NULL;
    clients_[clientNum].level = NULL;
    clients_[clientNum].permissions.reset();
}

void Session::Init(int clientNum)
{
    G_DPrintf("Session::Init called for %d\n", clientNum);
    clients_[clientNum].guid = "";
    clients_[clientNum].hwid = "";
    clients_[clientNum].user = NULL;
    clients_[clientNum].level = NULL;
    clients_[clientNum].permissions.reset();

    std::string ip = ValueForKey(clientNum, "ip");
    std::string::size_type pos = ip.find(":");
    clients_[clientNum].ip = ip.substr(0, pos);

    WriteSessionData(clientNum);
}

void Session::UpdateLastSeen(int clientNum)
{
    unsigned lastSeen = 0;

    // DEBUG: crash on client dc?
    G_LogPrintf("DEBUG: updating client %d last seen.\n", clientNum);

    if (clients_[clientNum].user)
    {   
        time_t t;
        if (!time(&t))
        {
            G_LogPrintf("ERROR: couldn't get current time.");
            return;
        }

        lastSeen = static_cast<unsigned>(t);

        G_DPrintf("Updating client's last seen to: %s\n", TimeStampToString(lastSeen).c_str());

        if (!game.database->UpdateLastSeen(clients_[clientNum].user->id, lastSeen))
        {
            G_LogPrintf("ERROR: %s\n", game.database->GetMessage().c_str());
        }
    }
}

void Session::WriteSessionData(int clientNum)
{
    G_LogPrintf("DEBUG: Writing client %d etjump session data\n", clientNum);

    const char *sessionData = va("%s %s",
        clients_[clientNum].guid.c_str(),
        clients_[clientNum].hwid.c_str());

    trap_Cvar_Set(va("etjumpsession%i", clientNum), sessionData);
}

std::string Session::Guid(gentity_t* ent) const
{
    return clients_[ClientNum(ent)].guid;
}

void Session::ReadSessionData(int clientNum)
{
    G_DPrintf("Session::ReadSessionData called for %d\n", clientNum);

    char sessionData[MAX_STRING_CHARS] = "\0";

    trap_Cvar_VariableStringBuffer(va("etjumpsession%i", clientNum), sessionData, sizeof(sessionData));

    char guidBuf[MAX_TOKEN_CHARS] = "\0";
    char hwidBuf[MAX_TOKEN_CHARS] = "\0";

    sscanf(sessionData, "%s %s", &guidBuf, &hwidBuf);

    CharPtrToString(guidBuf, clients_[clientNum].guid);
    CharPtrToString(hwidBuf, clients_[clientNum].hwid);

    GetUserAndLevelData(clientNum);

    if (!clients_[clientNum].user)
    {
        G_Error("client %d has no user\n", clientNum);
    }
    else if (!clients_[clientNum].level)
    {
        G_Error("client %d has no level.\n", clientNum);
    }
}

bool Session::GuidReceived(gentity_t *ent)
{
    int argc = trap_Argc();
    int clientNum = ClientNum(ent);
    char guidBuf[MAX_TOKEN_CHARS];
    char hwidBuf[MAX_TOKEN_CHARS];

    // Client sends "etguid guid hwid
    const unsigned ARGC = 3;
    if (argc != ARGC)
    {
        G_LogPrintf("Possible guid/hwid spoof attempt by %s (%s).\n",
            ent->client->pers.netname, ClientIPAddr(ent));
        return false;
    }

    trap_Argv(1, guidBuf, sizeof(guidBuf));
    trap_Argv(2, hwidBuf, sizeof(hwidBuf));

    if (!ValidGuid(guidBuf) || !ValidGuid(hwidBuf))
    {
        G_LogPrintf("Possible guid/hwid spoof attempt by %s (%s).\n",
            ent->client->pers.netname, ClientIPAddr(ent));
        return false;
    }    

    clients_[clientNum].guid = G_SHA1(guidBuf);
    clients_[clientNum].hwid = G_SHA1(hwidBuf);

    G_DPrintf("GuidReceived: %d GUID: %s HWID: %s\n",
        clientNum, clients_[clientNum].guid.c_str(),
        clients_[clientNum].hwid.c_str());

    GetUserAndLevelData(clientNum);

    if (game.database->IsBanned(clients_[clientNum].guid, clients_[clientNum].hwid))
    {
        G_LogPrintf("Banned player %s tried to connect with guid %s and hardware id %s\n",
            clients_[clientNum].guid.c_str(), clients_[clientNum].hwid.c_str());
        CPMAll(va("Banned player %s ^7tried to connect.", ent->client->pers.netname));
        trap_DropClient(clientNum, "You are banned.", 0);
    }

    return true;
}

void Session::GetUserAndLevelData(int clientNum)
{
    gentity_t *ent = g_entities + clientNum;
    if (!game.database->UserExists(clients_[clientNum].guid))
    {
        if (!game.database->AddUser(clients_[clientNum].guid, clients_[clientNum].hwid, std::string(ent->client->pers.netname)))
        {
            G_LogPrintf("ERROR: failed to add user to database: %s\n", game.database->GetMessage().c_str());
        }
        else
        {
            G_DPrintf("New user connected. Added user to the user database\n");
            clients_[clientNum].user = game.database->GetUserData(clients_[clientNum].guid);
        }
    }
    else
    {
        G_DPrintf("Old user connected. Getting user data from the database.\n");

        clients_[clientNum].user = game.database->GetUserData(clients_[clientNum].guid);
        if (clients_[clientNum].user)
        {
            G_DPrintf("User data found: %s\n", clients_[clientNum].user->ToChar());

            if (find(clients_[clientNum].user->hwids.begin(), clients_[clientNum].user->hwids.end(), clients_[clientNum].hwid)
                == clients_[clientNum].user->hwids.end())
            {
                G_DPrintf("New HWID detected. Adding HWID %s to list.\n", clients_[clientNum].hwid.c_str());

                if (!game.database->AddNewHWID(clients_[clientNum].user->id, clients_[clientNum].hwid))
                {
                    G_LogPrintf("Failed to add a new hardware ID to user %s\n", ent->client->pers.netname);
                }
            }
        }
        else
        {
            G_LogPrintf("ERROR: couldn't get user's data (%s)\n", ent->client->pers.netname);
        }
    }

    clients_[clientNum].level = game.levels->GetLevel(clients_[clientNum].user->level);

    if (ent->client->sess.firstTime)
    {
        PrintGreeting(ent);
        CPMTo(ent, std::string("^5Your last visit was on ") + clients_[clientNum].user->GetLastSeenString() + ".");
    }

    if (!clients_[clientNum].user)
    {
        // Debugging purposes, should be never executed
        G_Error("Client doesn't have db::user.\n");
    }

    ParsePermissions(clientNum);
}

void Session::ParsePermissions(int clientNum)
{
    clients_[clientNum].permissions.reset();
    // First parse level commands then user commands (as user commands override level ones)
    std::string commands = clients_[clientNum].level->commands + clients_[clientNum].user->commands;

    const int STATE_ALLOW = 1;
    const int STATE_DENY = 2;
    int state = STATE_ALLOW;
    for (unsigned i = 0; i < commands.length(); i++)
    {
        char c = commands.at(i);
        if (state == STATE_ALLOW)
        {
            if (c == '*')
            {
                // Allow all commands
                for (size_t i = 0; i < MAX_COMMANDS; i++)
                {
                    clients_[clientNum].permissions.set(i, true);
                }
            }
            else if (c == '+')
            {
                // ignore +
                continue;
            }
            else if (c == '-')
            {
                state = STATE_DENY;
            }
            else
            {
                clients_[clientNum].permissions.set(c, true);
            }
        }
        else
        {
            if (c == '*')
            {
                // Ignore * while in deny-mode
                continue;
            }
            
            if (c == '+')
            {
                state = STATE_ALLOW;
            }
            else
            {
                clients_[clientNum].permissions.set(c, false);
            }
        }
    }
}

void Session::OnClientDisconnect(int clientNum)
{
    WriteSessionData(clientNum);
    UpdateLastSeen(clientNum);

    G_LogPrintf("DEBUG: nullifying user %d\n", clientNum);
    clients_[clientNum].user = NULL;
    G_LogPrintf("DEBUG: nullifying level %d\n", clientNum);
    clients_[clientNum].level = NULL;
    G_LogPrintf("DEBUG: nullifying permissions %d\n", clientNum);
    clients_[clientNum].permissions.reset();
}

void Session::PrintGreeting(gentity_t* ent)
{
    int clientNum = ClientNum(ent);
    Client *cl = &clients_[clientNum];

    // If user has own greeting, print it
    if (cl->user->greeting.length() > 0)
    {
        std::string greeting = cl->user->greeting;
        boost::replace_all(greeting, "[n]", ent->client->pers.netname);
        boost::replace_all(greeting, "[t]", cl->user->GetLastSeenString());
        boost::replace_all(greeting, "[d]", cl->user->GetLastVisitString());
        G_DPrintf("Printing greeting %s\n", greeting.c_str());
        ChatPrintAll(greeting);
    }
    else
    {
        if (!cl->level)
        {
            return;
        }
        // if user has a level greeting, print it
        if (cl->level->greeting.length() > 0)
        {
            std::string greeting = cl->level->greeting;
            boost::replace_all(greeting, "[n]", ent->client->pers.netname);
            boost::replace_all(greeting, "[t]", cl->user->GetLastSeenString());
            boost::replace_all(greeting, "[d]", cl->user->GetLastVisitString());
            G_DPrintf("Printing greeting %s\n", greeting.c_str());
            ChatPrintAll(greeting);
        }
    }
}

void Session::PrintSessionData()
{
    BeginBufferPrint();
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        BufferPrint(NULL,
            va("%d %s %s\n", i,
            clients_[i].guid.c_str(),
            clients_[i].hwid.c_str()));
    }
    FinishBufferPrint(NULL, false);
}

bool Session::SetLevel(gentity_t* target, int level)
{
    if (!clients_[ClientNum(target)].user)
    {
        message_ = "you must wait until user has connected.";
        return false;
    }

    if (!game.database->SetLevel(clients_[ClientNum(target)].user->id, level))
    {
        message_ = game.database->GetMessage();
        return false;
    }

    clients_[ClientNum(target)].level = game.levels->GetLevel(level);

    ParsePermissions(ClientNum(target));
    return true;
}

bool Session::SetLevel(unsigned id, int level)
{
    if (!game.database->SetLevel(id, level))
    {
        message_ = game.database->GetMessage();
        return false;
    }

    for (unsigned i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients_[i].user && clients_[i].user->id == id)
        {
            ParsePermissions(i);
            ChatPrintTo(g_entities + i, va("^3setlevel: ^7you are now a level %d user.", level));
        }        
    }
    return true;
}


bool Session::UserExists(unsigned id)
{
    return game.database->UserExists(id);
}

int Session::GetLevelById(unsigned id) const
{
    return game.database->GetUserData(id)->level;
}

int Session::GetLevel(gentity_t* ent) const
{
    int num = ClientNum(ent);
    if (ent && clients_[num].user)
    {
        return clients_[num].user->level;
    }

    return 0;
}

bool Session::IsIpBanned(int clientNum)
{
    return game.database->IsIpBanned(clients_[clientNum].ip);
}

bool Session::Ban(gentity_t* ent, gentity_t *player, unsigned expires, std::string reason)
{
    int clientNum = ClientNum(player);
    time_t t;
    time(&t);

    if (clients_[clientNum].guid.length() == 0)
    {
        message_ = "User doesn't have a guid. Are you sure he's connected?";
        return false;
    }

    std::string ipport = ValueForKey(player, "ip");
    std::string::size_type pos = ipport.find(":");
    std::string ip = ipport.substr(0, pos);

    return game.database->BanUser(std::string(player->client->pers.netname), clients_[clientNum].guid,
        clients_[clientNum].hwid, ip, std::string(ent ? ent->client->pers.netname : "Console"),
        TimeStampToString(static_cast<unsigned>(t)), expires, reason);
}

void Session::PrintFinger(gentity_t* ent, gentity_t* target)
{
    int num = ClientNum(target);

    if (!clients_[num].user)
    {
        message_ = "you must wait until user has connected.";
        return;
    }

    ChatPrintTo(ent, va("^3finger: ^7%s^7 is a level %d user (%s)", target->client->pers.netname,
        clients_[num].user->level, 
        clients_[num].user->title.length() > 0 ? clients_[num].user->title.c_str() : clients_[num].level->name.c_str()));
}

void Session::PrintAdmintest(gentity_t* ent)
{
    int clientNum = ClientNum(ent);

    if (!clients_[ClientNum(ent)].user)
    {
        message_ = "you must wait until user has connected.";
        return;
    }

    if (ent && clients_[clientNum].user && clients_[clientNum].level)
    {
        std::string message = va("^3admintest: ^7%s^7 is a level %d user (%s^7).",
            ent->client->pers.netname,
            clients_[clientNum].user->level,
            clients_[clientNum].user->title.length() > 0 ? clients_[clientNum].user->title.c_str() : clients_[clientNum].level->name.c_str());

        ChatPrintAll(message);
    }
}

std::string Session::GetMessage() const
{
    return message_;
}

std::bitset<256> Session::Permissions(gentity_t* ent) const
{
    if (!ent)
    {
        std::bitset<MAX_COMMANDS> all;
        all.set();
        return all;
    } 
    return clients_[ClientNum(ent)].permissions;
}

void Session::PrintGuid(gentity_t* ent)
{
    G_DPrintf("Printing GUID to %s\n", ent->client->pers.netname);
    ChatPrintTo(ent, va("Your guid is: %s", clients_[ClientNum(ent)].guid.c_str()));
}

Session::Client::Client():
guid(""), hwid("")
{

}

bool Session::HasPermission(gentity_t *ent, char flag) 
{
    return clients_[ClientNum(ent)].permissions[flag];
}