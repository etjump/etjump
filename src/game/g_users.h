#ifndef g_users_h__
#define g_users_h__

#include <string>
#include <vector>
#include <bitset>
#include <boost/noncopyable.hpp>
#ifdef min
#undef min
#endif
#include "../sqlite/sqlite3pp.h"
#include "g_local.hpp"

class UserDatabase : public boost::noncopyable {
public:

    static const unsigned GUID_LEN = 40;
    static const unsigned HARDWARE_ID_LEN = 40;
    static const size_t MAX_CMDS = 256;

    UserDatabase();
    ~UserDatabase();

    // Called on game init
    void Init();
    // Called on game shutdown
    void Shutdown();

    bool SetGuid(gentity_t *ent, const std::string& guid);
    bool SetHardwareID(gentity_t *ent, const std::string& hardwareID);
    bool SetIP(gentity_t *ent, const std::string& ip);
    bool SetPassword(gentity_t *ent, const std::string& password);
    bool SetUsername(gentity_t *ent, const std::string& username);
    bool SetLevel(gentity_t *ent, int level);

	void UpdatePermissions(gentity_t *ent);
	void UpdatePermissions();

    std::string Guid(gentity_t *ent) const;
    std::string HardwareID(gentity_t *ent) const;
    std::string IP(gentity_t *ent) const;
    std::string Password(gentity_t *ent) const;
    std::string Username(gentity_t *ent) const;
    std::bitset<MAX_CMDS> Permissions(gentity_t *ent) const;
    int Level(gentity_t *ent) const;

    void ResetGuid(gentity_t *ent);
    void ResetHardwareID(gentity_t *ent);
    void ResetIP(gentity_t *ent);
    void ResetPassword(gentity_t *ent);
    void ResetUsername(gentity_t *ent);
    void ResetDBUserID(gentity_t *ent);
    void ResetTitle(gentity_t *ent);
    void ResetCommands(gentity_t *ent);
    void ResetGreeting(gentity_t *ent);
    void ResetLevel(gentity_t *ent);
    void ResetPermissions(gentity_t *ent);

    void ResetData(gentity_t *ent);

    void Print(gentity_t *caller, int targetClientNum) const;

    // DB Functions
    void AddNameToDatabase(gentity_t *ent);
    
    const std::vector<std::string> *GetAliases( gentity_t *ent );
    
private:

    struct Client {
        Client();
        std::string guid;
        std::string hardwareID;
        std::string ip;
        std::string password;
        std::string username;
        std::string title;
        std::string commands;
        std::string greeting;

        // User ID from DB, used for some queries. Cached on 
        // ClientGuidReceived
        int dbUserID;
        int level;
        std::bitset<MAX_CMDS> permissions;
    };

    Client clients_[MAX_CLIENTS];
    sqlite3pp::database db_;
};

#endif // g_users_h__
