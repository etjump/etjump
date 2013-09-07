#include "g_sessiondb.hpp"
#include <boost/array.hpp>

Session::Session()
{

}

Session::~Session()
{

}

boost::array<Session::Client, 64> Session::clients_;

Session::Client::Client()
{
    this->id = -1;
    this->guid.clear();
    this->level = 0;
    this->permissions.reset();
    this->greeting.clear();
    this->title.clear();
    this->name.clear();
}

void Session::InitGame()
{
    ResetAll();
}

void Session::ShutdownGame()
{
    ResetAll();
}

void Session::ResetAll()
{
    G_LogPrintf("**********************************\nResetAll()\n********************************\n");
    for(size_t i = 0; i < MAX_CLIENTS; i++)
    {
        ResetClient(clients_[i]);
    }
}

void Session::ResetClient( Client& toReset )
{
    toReset.guid.clear();
    toReset.greeting.clear();
    toReset.id = -1;
    toReset.level = 0;
    toReset.permissions.reset();
    toReset.title.clear();
    toReset.name.clear();
}

void Session::ResetClient( int clientNum )
{
    if(clientNum > MAX_CLIENTS || clientNum < 0)
    {
        return;
    }

    ResetClient(clients_[clientNum]);
}

void Session::ResetClient( gentity_t *ent )
{
    if(ent)
    {
        ResetClient(clients_[ClientNum(ent)]);
    }
}

void Session::ClientConnect(gentity_t *ent, int id, 
                            const std::string& guid,
                            int level, const std::string& name, 
                            const std::string& levelCmds,
                            const std::string& personalCmds, 
                            const std::string& personalGreeting,
                            const std::string& personalTitle)
{
    clients_[ClientNum(ent)].id = id;
    clients_[ClientNum(ent)].guid = guid;
    clients_[ClientNum(ent)].title = personalTitle;
    clients_[ClientNum(ent)].greeting = personalGreeting;
    clients_[ClientNum(ent)].name = name;

    SetLevel(ent, level, levelCmds, personalCmds);

    PrintDB();
}

// FIXME: update name
void Session::SetLevel( gentity_t *ent, int level, 
                       const std::string& levelCmds, 
                       const std::string& personalCmds )
{
    clients_[ClientNum(ent)].level = level;

    SetPermissions(clients_[ClientNum(ent)], personalCmds, levelCmds);
}

void Session::SetPermissions(Client& client, 
                             const std::string& personalCmds, 
                             const std::string& levelCmds)
{
    // Make sure it's empty
    client.permissions.reset();

    // Set permissions based on level commands
    ParsePermissions(client.permissions, levelCmds);

    // Override level perms with personal permissions
    ParsePermissions(client.permissions, personalCmds);
}

void Session::ParsePermissions(std::bitset<MAX_COMMANDS>& temp, 
                      const std::string& permissions)
{
    enum {
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

int Session::ID(gentity_t *ent)
{
    return clients_[ClientNum(ent)].id;
}

int Session::Level(gentity_t *ent)
{
    return clients_[ClientNum(ent)].level;
}

std::string Session::Guid(gentity_t *ent)
{
    return clients_[ClientNum(ent)].guid;
}

std::string Session::Greeting(gentity_t *ent)
{
    return clients_[ClientNum(ent)].greeting;
}

std::string Session::Title(gentity_t *ent)
{
    return clients_[ClientNum(ent)].title;
}

bool Session::HasPermission(gentity_t *ent, char flag)
{
    if(!ent)
    {
        return true;
    }

    return clients_[ent->client->ps.clientNum].permissions.test(flag); 
}

std::string Session::Name( gentity_t *ent )
{
    if(!ent)
    {
        return "";
    }

    return clients_[ent->client->ps.clientNum].name;
}

/*
*    struct Client
{
Client();
int id;
int level;
// Name when first seen/last setlevel
std::string name;
std::string guid;
std::string greeting;
std::bitset<MAX_COMMANDS> permissions;
std::string title;
};

 */

#include <fstream>
#include <boost/format.hpp>
void Session::PrintDB()
{
    std::ofstream out("log.txt");
    out << "****************************\n";
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        
        out << (boost::format("(%d)(%d)(%s)(%s)(%s)(%s)(%s)\n")
            
            % clients_[i].id % clients_[i].level % clients_[i].name.c_str() %
            clients_[i].guid.c_str() % clients_[i].greeting.c_str() %
            clients_[i].permissions.to_string().c_str() % 
            clients_[i].title.c_str()
            
            ).str();
           
    }
    out << "****************************\n";
    out.close();
}

void Session::UpdateUser( int id, int updated, int level, const std::string& levelCommands, 
                         const std::string& personalCommands, const std::string& greeting, const std::string& title )
{
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients_[i].id == id)
        {
            if(updated & UPDATED_LEVEL)
            {
                clients_[i].level = level;
                SetPermissions(clients_[i], personalCommands, levelCommands);
            }

            if(updated & UPDATED_COMMANDS)
            {
                SetPermissions(clients_[i], personalCommands, levelCommands);
            }

            if(updated & UPDATED_GREETING)
            {
                clients_[i].greeting = greeting;
            }

            if(updated & UPDATED_TITLE)
            {
                clients_[i].title = title;
            }
            

            return;
        }
    }
}
