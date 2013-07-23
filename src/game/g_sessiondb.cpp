#include "g_sessiondb.hpp"

void Reset(Client& toReset)
{
    toReset.guid.clear();
    toReset.greeting.clear();
    toReset.id = -1;
    toReset.level = 0;
    toReset.permissions.reset();
    toReset.title.clear();
}


void SessionDB::Reset()
{
    for(size_t i = 0; i < MAX_CLIENTS; i++)
    {
        ::Reset(clients_[i]);
    }
}

void SessionDB::ResetClient( gentity_t *ent )
{
    if(ent->client) {
        ::Reset(clients_[ent->client->ps.clientNum]);
    }
}

std::string SessionDB::Guid( gentity_t *ent )
{
    return clients_[ent->client->ps.clientNum].guid;
}

void SessionDB::Set( gentity_t *ent, int id, const std::string& guid, int level, 
                    const std::string& name, 
                    const std::string& levelPermissions, 
                    const std::string& personalPermissions, 
                    const std::string& greeting, 
                    const std::string& personalTitle )
{
    clients_[ent->client->ps.clientNum].id = id;
    clients_[ent->client->ps.clientNum].guid = guid;
    clients_[ent->client->ps.clientNum].level = level;
    clients_[ent->client->ps.clientNum].title = personalTitle;
    clients_[ent->client->ps.clientNum].greeting = greeting;
    
    SetPermissions(clients_[ent->client->ps.clientNum], 
        personalPermissions, levelPermissions);
}

void ParsePermissions(std::bitset<MAX_COMMANDS>& temp, 
                      const std::string& permissions)
{
    const enum {
        ALLOW,
        DENY
    };

    int state = ALLOW;

    for(size_t i = 0; i < permissions.size(); i++)
    {
        char c = permissions.at(i);
        if(state == ALLOW)
        {
            if(c == '*')
            {
                // Allow all commands
                for(size_t i = 0; i < MAX_COMMANDS; i++)
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

void SessionDB::SetPermissions( Client& client, 
                               const std::string& personalCommands, 
                               const std::string& levelCommands )
{
    // Make sure it's empty
    client.permissions.reset();

    // Set permissions based on level commands
    ParsePermissions(client.permissions, levelCommands);

    // Override level perms with personal permissions
    ParsePermissions(client.permissions, personalCommands);
}

std::string SessionDB::Greeting( gentity_t *ent )
{
    return clients_[ent->client->ps.clientNum].greeting;
}

int SessionDB::Level( gentity_t *ent )
{
    if(!ent)
    {
        return std::numeric_limits<int>::max();
    }
    return clients_[ent->client->ps.clientNum].level;
}

bool SessionDB::HasPermission( gentity_t *ent, char flag ) const
{
    if(!ent)
    {
        return true;
    }

    return clients_[ent->client->ps.clientNum].permissions.at(flag);        
}

int SessionDB::Id( gentity_t *ent )
{
    if(!ent)
    {
        return -1;
    }

    return clients_[ent->client->ps.clientNum].id;
}

void SessionDB::SetLevel( gentity_t *ent, int level, 
              const std::string& levelPermissions, 
              const std::string& personalPermissions )
{
    clients_[ent->client->ps.clientNum].level = level;

    SetPermissions(clients_[ent->client->ps.clientNum], personalPermissions, levelPermissions);
}

Client::Client()
{
    this->id = -1;
    this->guid.clear();
    this->level = 0;
    this->permissions.reset();
    this->greeting.clear();
    this->title.clear();
}