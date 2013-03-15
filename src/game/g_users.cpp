#include "g_users.h"
#include "g_utilities.h"

ClientData::~ClientData()
{

}

ClientData::ClientData()
{

}

string ClientData::Guid( gentity_t *ent ) const
{
    return clients_[ent->client->ps.clientNum].guid;
}

string ClientData::HWID( gentity_t *ent ) const
{
    return clients_[ent->client->ps.clientNum].hardwareId;
}

string ClientData::IP( gentity_t *ent ) const
{
    return clients_[ent->client->ps.clientNum].ip;
}

bool ClientData::SetGuid( gentity_t *ent, string guid )
{
    if( guid.length() < GUID_LEN ) {
        G_LogPrintf("Error while setting %s guid.\n", 
            ent->client->pers.netname);
        G_LogPrintf("Client::SetGuid: Guid is less than 40 chars.");
        return false;
    } else if( guid.length() > GUID_LEN ) {
        G_LogPrintf("Error while setting %s guid.\n", 
            ent->client->pers.netname);
        G_LogPrintf("Client::SetGuid: Guid is more than 40 chars.");
        return false;
    } 

    clients_[ent->client->ps.clientNum].guid = guid;
    return true;
}

bool ClientData::SetHWID( gentity_t *ent, string hwid )
{
    if( hwid.length() < HWID_LEN ) {
        G_LogPrintf("Error while setting %s hardware ID.\n", 
            ent->client->pers.netname);
        G_LogPrintf("Client::SetHWID: HWID is less than 40 chars.\n");
        return false;
    } else if( hwid.length() < HWID_LEN ) {
        G_LogPrintf("Error while setting %s hardware ID.\n", 
            ent->client->pers.netname);
        G_LogPrintf("Client::SetHWID: HWID is more than 40 chars.\n");
        return false;
    }

    clients_[ent->client->ps.clientNum].hardwareId = hwid;
    return true;
}

bool ClientData::SetIP( gentity_t *ent, string ip )
{
    if( ip.length() > MAX_IP_LEN ) {
        G_LogPrintf("Error while setting %s's IP.\nClient::SetIP: \
                    IP is more than 15 chars.");
        return false;
    }

    clients_[ent->client->ps.clientNum].ip = ip;
    return true;
}

void ClientData::DebugPrint( gentity_t *ent ) const
{
    G_LogPrintf("Client: %s\nGuid: %s\nHWID: %s\nIP: %s\n", 
        ent->client->pers.netname,
        clients_[ent->client->ps.clientNum].guid.c_str(),
        clients_[ent->client->ps.clientNum].hardwareId.c_str(),
        clients_[ent->client->ps.clientNum].ip.c_str());
}

/*
 * FIXME: this belongs to g_utilities.cpp
 */
string GetIP( gentity_t *ent ) {
    char userinfo[MAX_INFO_STRING];
    trap_GetUserinfo( ent->client->ps.clientNum, 
        userinfo, sizeof(userinfo));

    char *ipPtr = Info_ValueForKey(userinfo, "ip");
    if(!ipPtr) {
        G_LogPrintf("GetIP: Error while getting %s's ip",
            ent->client->pers.netname);
        return "";
    }

    unsigned charactersCopied = 0;
    char ip[MAX_IP_LEN + 1];

    // Copy the IP without port separator
    while(ipPtr[charactersCopied] != ':') {
        if(charactersCopied >= MAX_IP_LEN) {
            break;
        }
        ip[charactersCopied] = ipPtr[charactersCopied];
        charactersCopied++;
    }

    ip[charactersCopied] = NULL;

    G_LogPrintf("GetIp called: %s\n", ip);

    return string(ip);
}

/*
 * This function is called every time we receive
 * "etguid"-command from client
 */
void ClientData::GuidReceived( gentity_t *ent )
{
    Arguments argv = GetArgs();

    // Client sends "etguid <hash>"
    if(argv->size() != 2) {
        // Request client to send guid again
        RequestGuid(ent);
        return;
    }

    if(argv->at(1).length() != 40) {
        // We received an invalid guid hash, request guid again
        RequestGuid(ent);
        return;
    }

    // Hash the guid again and update the client data
    string guid = SHA1(argv->at(1));
    string ip   = GetIP(ent);

    if(ip.length() > 0) {
        // If for some reason it fails to get the IP, don't add it to db
        // Should never happen
        SetIP(ent, ip);
    }

    SetGuid(ent, guid);
}

/*
 * Called when hardware id is received from ClientCommand()
 */
void ClientData::HWIDReceived( gentity_t *ent )
{
    // HWID <hwid hash>
    Arguments argv = GetArgs();

    if(argv->size() != 2) {
        // Possible fake HWID command. Log it and request HWID again
        G_LogPrintf("Invalid HWID info from client on slot %d\n\
                    name: %s\n", ent->client->ps.clientNum,
                    ent->client->pers.netname);
        RequestHWID(ent);
        return;
    }

    if(argv->at(1).length() != HWID_LEN) {
        // Possible fake HWID. Log it and request HWID again
        G_LogPrintf("Invalid HWID info from client on slot %d\n\
                    name: %s\n", ent->client->ps.clientNum,
                    ent->client->pers.netname);
        RequestHWID(ent);
        return;
    }

    /*
     * Check if either guid is banned & add to client db
     */

    if(HardwareBanCheck(argv->at(1).c_str())) {
        // TODO: Handle kicking.
    }

    char userinfo[MAX_INFO_STRING];
    trap_GetUserinfo( ent->client->ps.clientNum, 
        userinfo, sizeof(userinfo));

    char *userinfoHWID = Info_ValueForKey(userinfo, "hwinfo");
    if(!userinfoHWID) {
        // Shouldn't happen
        RequestHWID(ent);
        return;
    }

    if(!Q_stricmp(userinfoHWID, "NOHWID")) {
        RequestHWID(ent);
    } else {
        if(HardwareBanCheck(userinfoHWID)) {
            // TODO: Handle kicking.
        }
    }

    SetHWID(ent, argv->at(1));
}

void ClientData::OnClientConnect( gentity_t *ent, bool firstTime )
{
    if(firstTime) {
        ResetClientData(ent);
    }
}


void ClientData::OnClientBegin( gentity_t *ent )
{
    if( clients_[ent->client->ps.clientNum].guid.length() == 0 ) {
        RequestGuid(ent);
    }
    if( clients_[ent->client->ps.clientNum].hardwareId.length() == 0 ) {
        RequestHWID(ent);
    }
}

void ClientData::OnClientDisconnect( gentity_t *ent )
{
    ResetClientData(ent);
}

void ClientData::ResetClientData( gentity_t * ent )
{
    clients_[ent->client->ps.clientNum].guid.clear();
    clients_[ent->client->ps.clientNum].hardwareId.clear();
    clients_[ent->client->ps.clientNum].ip.clear();
}

void ClientData::RequestGuid( gentity_t *ent )
{
    trap_SendServerCommand(ent->client->ps.clientNum,
        GUID_REQUEST);
}

void ClientData::RequestHWID( gentity_t *ent )
{
    trap_SendServerCommand(ent->client->ps.clientNum,
        HWID_REQUEST);
}


bool BanDatabase::BanCheck( const string& hardwareID, 
                           const string& ip ) const
{
    vector<Ban*>::const_iterator it = bans_.begin();

    while(it != bans_.end()) {
        if((*it)->ip == ip || (*it)->hardwareID == hardwareID) {
            return true;
        }
    }

    return false;
}

bool BanDatabase::AddBan( int expires, const string& banner, 
                         const string& date, const string& hardwareID, 
                         const string& ip, const string& name, 
                         const string& reason )
{
    if(expires < 0) {
        G_LogPrintf("Invalid expiration timestamp. Ignoring ban.\n");
        return false;
    }

    Ban *newBan = new Ban(expires, banner, date, 
        hardwareID, ip, name, reason);

    bans_.push_back(newBan);

    // TODO: sqlite database updates

    return false;
}

bool BanDatabase::RemoveBan( const string& hardwareID, const string& ip )
{
    vector<Ban*>::iterator it = bans_.begin();

    while(it != bans_.end()) {
        if((*it)->ip == ip || (*it)->hardwareID == hardwareID) {
            // Considering there won't be too many bans (~hundreds max)
            // I'd say it's ok to have a rather crappy algorithm to
            // delete the bans
            delete *it;
            bans_.erase(it);

            // TODO: update sqlite 
        }
        it++;
    }
    return false;
}

const string fs_game_path = "etjump/";

BanDatabase::BanDatabase()
{
    
}

BanDatabase::~BanDatabase()
{
    
}

BanDatabase::Ban::Ban( int expires_, const string& banner_, 
                      const string& date_, 
                      const string& hardwareID_, const string& ip_, 
                      const string& name_, const string& reason_ )
{
    expires = expires_;
    banner = banner_;
    date = date_;
    hardwareID = hardwareID_;
    ip = ip_;
    name = name_;
    reason = reason_;
}

/*
 * C Interface for Client class method calls.
 */

static ClientData clients;

void Client_OnClientConnect( gentity_t *ent, qboolean firstTime )
{
    clients.OnClientConnect(ent, firstTime);
}

void Client_OnClientBegin( gentity_t *ent ) {
    clients.OnClientBegin(ent);
}

void Client_OnClientDisconnect( gentity_t *ent ) {
    clients.OnClientDisconnect(ent);
}

void Client_GuidReceived( gentity_t *ent ) {
    clients.GuidReceived(ent);
}

void Client_HWIDReceived( gentity_t *ent ) {
    clients.HWIDReceived(ent);
}

void Client_DebugPrint(gentity_t *ent) {
    if(!ent) {
        return;
    }
    clients.DebugPrint(ent);
}

qboolean HardwareBanCheck(const char* hardwareId) {
    return qfalse;
}