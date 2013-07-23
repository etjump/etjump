#ifndef g_sessiondb_h__
#define g_sessiondb_h__

#include "g_local.hpp"
#include <string>
#include <bitset>

const unsigned MAX_COMMANDS = 256;

struct Client
{
    Client();
    int id;
    int level;
    std::string guid;
    std::string greeting;
    std::bitset<MAX_COMMANDS> permissions;
    std::string title;
};

namespace Session
{
    Client clients[MAX_CLIENTS];
}

void Reset(Client& toReset);

class SessionDB
{
public:
    // Used to reset all clients' data in the data structure
    void Reset();

    // Used to reset client's data in the data structure
    void ResetClient(gentity_t *ent);

    // Returns client's guid
    std::string Guid(gentity_t *ent);

    // Sets ent's guid to "guid", level to "level", !finger name to "name",
    // personal commands to "personalCommands, personal greeting to
    // "personalGreeting" and personal level title to "personalTitle".
    void Set( gentity_t *ent, int id, const std::string& guid, int level, 
        const std::string& name, const std::string& levelPermissions, 
        const std::string& personalPermissions, 
        const std::string& personalGreeting, 
        const std::string& personalTitle );
    
    void SetLevel( gentity_t *ent, int level, 
        const std::string& levelPermissions, 
        const std::string& personalPermissions );
    // Returns client's greeting
    std::string Greeting(gentity_t *ent);

    bool HasPermission(gentity_t *ent, char flag) const;

    int Level(gentity_t *ent);

    int Id(gentity_t *ent);
private:
    void SetPermissions( Client& client, const std::string& personalCommands, const std::string& levelCommands );
    Client clients_[MAX_CLIENTS];
};

#endif // g_sessiondb_h__
