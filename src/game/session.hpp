#ifndef session_h__
#define session_h__

#include <string>
#include <bitset>

namespace Session
{
    const unsigned MAX_COMMANDS = 256;

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
}

#endif // session_h__
