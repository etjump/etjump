#include "sessiondata.h"
#include "../g_utilities.hpp"

SessionData::Client::Client()
    : id(0), level(0), guid(""), name(""), hwid(""), title(""), commands(""),
    greeting("")
{
    permissions.reset();
}

SessionData::SessionData( IUserData *userData )
    : userData_( userData )
{

}

SessionData::~SessionData()
{

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

    return true;
}



void SessionData::GetUserData( gentity_t *ent, const std::string& guid )
{
    const User *currentUser = userData_->GetUserData( guid );
    if(!currentUser)
    {
        

        // user does not exist. Create a new one
        userData_->CreateNewUser( guid, ent->client->pers.netname, clients_[ent->client->ps.clientNum].hwid );
        G_LogPrintf("Couldn't find user. Adding a new one.\n");
    } else
    {
        clients_[ent->client->ps.clientNum].id = currentUser->id;
        clients_[ent->client->ps.clientNum].level = currentUser->level;
        clients_[ent->client->ps.clientNum].name = currentUser->name;
        clients_[ent->client->ps.clientNum].title = currentUser->title;
        clients_[ent->client->ps.clientNum].commands = currentUser->commands;
        clients_[ent->client->ps.clientNum].greeting = currentUser->greeting;
        G_LogPrintf("Found a user.\n");
    }

    // TODO: levelData.GetData( userData.level )
}

std::string SessionData::GetGuid( gentity_t *ent )
{
    return clients_[ent->client->ps.clientNum].guid;
}
