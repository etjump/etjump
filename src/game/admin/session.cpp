#include "session.hpp"
#include "../g_utilities.hpp"
#include "database.hpp"

Session::Session()
{
    
}

void Session::Init(int clientNum)
{
    G_DPrintf("Session::Init called for %d\n", clientNum);
    clients_[clientNum].guid = "";
    clients_[clientNum].hwid = "";
    clients_[clientNum].user = NULL;

    WriteSessionData(clientNum);
}

void Session::WriteSessionData(int clientNum)
{
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
    clients_[clientNum].user = NULL;
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

    if (!game.database->UserExists(clients_[clientNum].guid))
    {
        if (!game.database->AddUser(clients_[clientNum].guid, clients_[clientNum].hwid, std::string(ent->client->pers.netname)))
        {
            G_LogPrintf("ERROR: failed to add user to database: %s\n", game.database->GetMessage().c_str());
        }
        else
        {
            G_LogPrintf("New user connected. Adding user to the user database\n");
        }
    }
    else
    {
        G_LogPrintf("Old user connected. Getting user data from the database.\n");

        clients_[clientNum].user = game.database->GetUserData(clients_[clientNum].guid);
        if (clients_[clientNum].user)
        {
            G_LogPrintf("User data found: %s\n", clients_[clientNum].user->ToChar());

            if (std::find(clients_[clientNum].user->hwids.begin(), clients_[clientNum].user->hwids.end(), clients_[clientNum].hwid) 
                == clients_[clientNum].user->hwids.end())
            {
                G_LogPrintf("New HWID detected. Adding HWID %s to list.\n", clients_[clientNum].hwid.c_str());
                
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
    
    return true;
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

std::bitset<256> Session::Permissions(gentity_t* ent) const
{
    std::bitset<256> set;
    set.set();
    return set;
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