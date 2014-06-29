#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>
#include "../g_local.hpp"

class Session
{
public:
    Session();
    struct Client
    {
        Client();
        std::string guid;
        std::string hwid;
    };

    void Init(int clientNum);
    void WriteSessionData(int clientNum);
    void ReadSessionData(int clientNum);
    bool GuidReceived(gentity_t *ent);
    void PrintGuid(gentity_t* ent);
    void PrintSessionData();
    std::string Guid(gentity_t *ent) const;
private:
    Client clients_[MAX_CLIENTS];


    
};

#endif