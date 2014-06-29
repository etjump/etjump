#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>
#include "../g_local.hpp"

class Session
{
public:
    struct Client
    {
        std::string guid;
        std::string hwid;
    };

    bool GuidReceived(gentity_t *ent);
private:

};

#endif