#ifndef user_h__
#define user_h__

#include <string>

struct User
{
    User()
        :level(0), lastSeen(0), name(""), hwid(""), title(""), commands(""), greeting("")
    {
        
    }
    int id;
    int level;
    int lastSeen;
    std::string name;
    std::string hwid;
    std::string title;
    std::string commands;
    std::string greeting;
};

#endif // user_h__
