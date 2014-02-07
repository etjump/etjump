#ifndef sessiondata_h__
#define sessiondata_h__

#include <string>
#include <bitset>
#include <boost/array.hpp>

#include "../g_local.hpp"
#include "iuserdata.h"
#include "iguid.h"
#include "ileveldata.h"

class SessionData : public IGuid
{
public:
    struct Client
    {
        Client();
        static const int MAX_COMMANDS = 256;
        int id;
        int level;
        std::string guid;
        std::string name;
        std::string hwid;
        std::string title;
        std::string commands;
        std::string greeting;
        std::bitset< MAX_COMMANDS > permissions;
    };

    SessionData( IUserData *userData, ILevelData *levelData );
    ~SessionData();

    // Handles everything needed when guid is received
    bool GuidReceived( gentity_t *ent );

    void PrintAdmintest( gentity_t *ent );
    void PrintFinger( gentity_t *ent, gentity_t *target );
    
    virtual std::string GetGuid( gentity_t *ent );

    int GetLevel( gentity_t *ent );
private:
    // Checks whether guid is valid or not
    bool ValidGuid( const std::string& guid );

    void GetUserData( gentity_t *ent, const std::string& guid );

    boost::array<Client, 64> clients_;

    IUserData *userData_;
    ILevelData *levelData_;
};

#endif // sessiondata_h__
