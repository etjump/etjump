#include "sessiondata.h"
#include "../g_utilities.hpp"

SessionData::Client::Client()
    : id(0), level(0), guid(""), name(""), hwid(""), title(""), commands(""),
    greeting("")
{
    permissions.reset();
}

SessionData::SessionData( IUserData *userData, ILevelData *levelData )
    : userData_( userData ), levelData_( levelData )
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
        
    }

    
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

void SessionData::PrintFinger( gentity_t *ent, gentity_t *target )
{
    const ILevelData::LevelInformation *level = 
        levelData_->GetLevelInformation(clients_[ent->client->ps.clientNum].level);
    if(clients_[ent->client->ps.clientNum].name.length() > 0)
    {
        ChatPrintTo(ent, va("^3finger: ^7%s^7 (%s^7) is a level %d user (%s^7)",
            ent->client->pers.netname, 
            clients_[ent->client->ps.clientNum].name.c_str(),
            clients_[ent->client->ps.clientNum].level,
            level->name.c_str()));
    } else
    {
        ChatPrintTo(ent, va("^3finger: ^7%s^7 is a level %d user (%s^7)",
            ent->client->pers.netname, 
            clients_[ent->client->ps.clientNum].level,
            level->name.c_str()));
    }
    
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
    return true;
}

void SessionData::UpdateUserSessionData( gentity_t *ent )
{
    const ILevelData::LevelInformation *levelInfo = 
        levelData_->GetLevelInformation(clients_[ent->client->ps.clientNum].level);

    const User* user = userData_->GetUserData(clients_[ent->client->ps.clientNum].guid);

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

    // TODO: update permissions
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
