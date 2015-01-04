#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>
#include <vector>
#include <bitset>
#include "../g_local.hpp"
#include "levels.hpp"
#include "database.hpp"
#include "iauthentication.hpp"


class Session
{
public:
    static const unsigned MAX_COMMANDS = 256;
    Session(IAuthentication *database);
    void ResetClient(int clientNum);

    struct Client
    {
        Client();
        std::string guid;
        std::string hwid;
        std::bitset<MAX_COMMANDS> permissions;
        std::string ip;
        const User_s *user;
        const Levels::Level *level;
    };

    void Init(int clientNum);
    void ReadSessionData(int clientNum);
    void WriteSessionData(int clientNum);
    void GetUserAndLevelData(int clientNum);
    bool GuidReceived(gentity_t *ent);
    void PrintGuid(gentity_t* ent);
    void PrintSessionData();
    void PrintGreeting(gentity_t* ent);
    void OnClientDisconnect(int clientNum);
    std::string Guid(gentity_t *ent) const;
    std::bitset<256> Permissions(gentity_t *ent) const;
    int GetLevel(gentity_t *ent) const;
    int GetId(gentity_t *ent) const;
    bool SetLevel(gentity_t* target, int level);
    bool SetLevel(unsigned id, int level);
    int GetLevelById(unsigned id) const;
    bool UserExists(unsigned id);
    std::string GetMessage() const;
    void PrintAdmintest(gentity_t* ent);
    void PrintFinger(gentity_t* ent, gentity_t* target);
    bool Ban(gentity_t *ent, gentity_t *player, unsigned expires, std::string reason);
    bool IsIpBanned(int clientNum);
    void ParsePermissions(int clientNum);
    bool HasPermission(gentity_t *ent, char flag);
    void NewName(gentity_t *ent);
    // Returns the amount of users with that level
    int LevelDeleted(int level);
    std::vector<Session::Client*> FindUsersByLevel(int level);
private:
    IAuthentication *database_;

    void UpdateLastSeen(int clientNum);
    Client clients_[MAX_CLIENTS];
    std::string message_;


    
};

#endif


