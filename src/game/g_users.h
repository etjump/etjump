#ifndef g_users_h__
#define g_users_h__

#include <string>
#include <boost/noncopyable.hpp>
#include "sqlite3pp.h"

extern "C" {
#include "g_local.h"
};

class UserDatabase : public boost::noncopyable {
public:

    static const unsigned GUID_LEN = 40;
    static const unsigned HARDWARE_ID_LEN = 40;

    UserDatabase();
    ~UserDatabase();

    bool SetGuid(gentity_t *ent, const std::string& guid);
    bool SetHardwareID(gentity_t *ent, const std::string& hardwareID);
    bool SetIP(gentity_t *ent, const std::string& ip);
    bool SetPassword(gentity_t *ent, const std::string& password);
    bool SetUsername(gentity_t *ent, const std::string& username);

    std::string Guid(gentity_t *ent) const;
    std::string HardwareID(gentity_t *ent) const;
    std::string IP(gentity_t *ent) const;
    std::string Password(gentity_t *ent) const;
    std::string Username(gentity_t *ent) const;

    void ResetGuid(gentity_t *ent);
    void ResetHardwareID(gentity_t *ent);
    void ResetIP(gentity_t *ent);
    void ResetPassword(gentity_t *ent);
    void ResetUsername(gentity_t *ent);
    void ResetDBUserID(gentity_t *ent);

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

        // User ID from DB, used for some queries. Cached on 
        // ClientGuidReceived
        int dbUserID;
    };

    Client clients_[MAX_CLIENTS];
    sqlite3pp::database db_;
};

#endif // g_users_h__
