extern "C" {
#include "g_local.h"
};

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "sqlite3.h"

using std::string;
using std::vector;

/* 
 * Client -class to keep track of active client information
 */

class ClientData : public boost::noncopyable {
public:

    static const unsigned GUID_LEN = 40;
    static const unsigned HWID_LEN = 40;
        
    ClientData();
    ~ClientData();
    // Returns ent's etjump-guid
    string Guid(gentity_t *ent) const;

    // Returns ent's HWID
    string HWID(gentity_t *ent) const;

    // Returns ent's IP
    string IP(gentity_t *ent) const;

    // Sets ent's guid to guid
    bool SetGuid(gentity_t *ent, string guid);

    // Sets ent's HWID to hwid
    bool SetHWID(gentity_t *ent, string hwid);

    // Sets ent's IP to ip
    bool SetIP(gentity_t *ent, string ip);

    // Prints client data to server console
    void DebugPrint(gentity_t *ent) const;

    // Called whenever "etguid" command is received from client
    void GuidReceived(gentity_t *ent);

    // Requests Guid from client
    void RequestGuid(gentity_t *ent);

    // Called whenever "hwid" command is received from client
    void HWIDReceived(gentity_t *ent);

    // Requests HWID from client
    void RequestHWID(gentity_t *ent);

    // Called on client connect
    void OnClientConnect(gentity_t *ent, bool firstTime);

    // Resets all client data
    void ResetClientData( gentity_t * ent );

    // Called on clientbegin()
    void OnClientBegin( gentity_t *ent );

    // Called on client disconnect
    void OnClientDisconnect(gentity_t *ent);

private:
    struct Client
    {
        // Unique guid for each client
        string guid;
        // Client's hardware id
        string hardwareId;
        // Client's ip
        string ip;
    };

    Client clients_[MAX_CLIENTS];
};

/*
 * Everything ban related goes here
 */

class BanDatabase {
public:
    BanDatabase();
    ~BanDatabase();
    bool BanCheck(const string& hardwareID, const string& ip) const;
    bool AddBan(int expires, const string& banner,
        const string& date, const string& hardwareID, const string& ip,
        const string& name, const string& reason);
    // TODO: add ban id ban removal
    bool RemoveBan(const string& hardwareID, const string& ip);

private:
    struct Ban {
        Ban(int expires_, const string& banner_, 
            const string& date_, 
            const string& hardwareID_,
            const string& ip_, 
            const string& name_, 
            const string& reason_);
        // linux timestamp
        int expires;
        // Who banned
        string banner;
        // When ban was given
        string date;
        // Banned HWID
        string hardwareID;
        // Banned IP
        string ip;
        string name;
        string reason;
    };

    vector<Ban*> bans_;
};