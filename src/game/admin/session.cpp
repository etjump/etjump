#include "session.hpp"
#include "../g_utilities.hpp"

Session::Session()
{
    
}

void Session::Init(int clientNum)
{
    G_DPrintf("Session::Init called for %d\n", clientNum);
    clients_[clientNum].guid = "";
    clients_[clientNum].hwid = "";

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
}

bool Session::GuidReceived(gentity_t *ent)
{
    int argc = trap_Argc();
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

    clients_[ClientNum(ent)].guid = G_SHA1(guidBuf);
    clients_[ClientNum(ent)].hwid = G_SHA1(hwidBuf);

    G_DPrintf("GuidReceived: %d GUID: %s HWID: %s\n",
        ClientNum(ent), clients_[ClientNum(ent)].guid.c_str(),
        clients_[ClientNum(ent)].hwid.c_str());

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