#ifndef g_sessiondb_h__
#define g_sessiondb_h__

#include "g_local.hpp"
#include <string>
#include <bitset>
#include <boost/array.hpp>

class Session
{
public:
    static const unsigned MAX_COMMANDS = 256;

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

    Session();
    ~Session();

    // Called on G_GameInit() & G_GameShutdown()
    static void InitGame();
    static void ShutdownGame();

    // Reset all clients
    static void ResetAll();

    // Reset client
    static void ResetClient(Client& toReset);
    static void ResetClient(gentity_t *ent);

    // Sets all necessary values when client connects
    static void ClientConnect(gentity_t *ent, int id, 
        const std::string& guid,
        int level, const std::string& name, 
        const std::string& levelCmds,
        const std::string& personalCmds, 
        const std::string& personalGreeting,
        const std::string& personalTitle);

    static void SetLevel( gentity_t *ent, int level, 
        const std::string& levelCmds, 
        const std::string& personalCmds );

    // Getters
    static int ID(gentity_t *ent);
    static int Level(gentity_t *ent);
    static std::string Guid(gentity_t *ent);
    static std::string Greeting(gentity_t *ent);
    static std::string Title(gentity_t *ent);
    static bool HasPermission(gentity_t *ent, char flag);
private:
    static void SetPermissions(Client& client, 
        const std::string& personalCmds, const std::string& levelCmds);

    static void ParsePermissions(std::bitset<MAX_COMMANDS>& temp, 
        const std::string& permissions);

    static boost::array<Client, 64> clients_;
};

#endif // g_sessiondb_h__
