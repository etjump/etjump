#ifndef sessiondata_h__
#define sessiondata_h__

#include <string>
#include <bitset>
#include <boost/array.hpp>
#include "iuserdata.h"
#include "iguid.h"
#include "ileveldata.h"
#include "ibandata.h"
#include "../g_local.hpp"

class SessionData : public IGuid
{
public:

    struct Client
    {
        Client();
        static const int MAX_COMMANDS = 256;
        int id;
        int level;
        int lastSeen;
        std::string guid;
        std::string name;
        std::string hwid;
        std::string title;
        std::string commands;
        std::string greeting;
        std::bitset< MAX_COMMANDS > permissions;
    };

    SessionData( IUserData *userData, ILevelData *levelData,
        IBanData *banData );
    ~SessionData();

    // Handles everything needed when guid is received
    bool GuidReceived( gentity_t *ent );

    void PrintAdmintest( gentity_t *ent );
    void PrintFinger( gentity_t *ent, gentity_t *target );
    
    virtual std::string GetGuid( gentity_t *ent );

    int GetLevel( gentity_t *ent );
    std::bitset<SessionData::Client::MAX_COMMANDS> 
        GetPermissions( gentity_t *ent );
    bool SetLevel( gentity_t *target, int level );
    void PrintUserinfo( gentity_t *ent, gentity_t *target );
    void PrintUserList( gentity_t *playerToPrintTo, int page );
    // Need this for the OnDisconnect()
    void UpdateLastSeen(gentity_t *ent);
    void Reset(gentity_t *ent);

    bool EditUser(gentity_t *toPrint, const std::string& guid, const std::string& title, 
        const std::string& commands, const std::string& greeting, 
        int updated);
    bool BanPlayer( gentity_t *ent, const std::string& guid,
        int seconds, const std::string& reason );
    bool BanPlayer( gentity_t *ent, gentity_t *target,
        int seconds, const std::string& reason );
    void UpdatePermissions();
private:
    // Checks whether guid is valid or not
    bool ValidGuid( const std::string& guid );

    void GetUserData( gentity_t *ent, const std::string& guid );

    void UpdateUserSessionData( gentity_t *ent );

    void ParsePermissions(std::bitset<SessionData::Client::MAX_COMMANDS>& temp, 
					  const std::string& permissions);

    boost::array<Client, 64> clients_;

    IUserData *userData_;
    ILevelData *levelData_;
    IBanData *banData_;
};

#endif // sessiondata_h__
