#ifndef USER_HH
#define USER_HH

#include <string>
#include <vector>

struct User_s
{
    User_s();
    User_s(unsigned id, const std::string& guid, const std::string& name, const std::string& hwid);
    User_s(unsigned id, std::string const& guid, int level, unsigned lastSeen,
        std::string const& name, std::string const& hwid, std::string const& title,
        std::string const& commands, std::string const& greeting);
    
    // These are needed for multi_index_container operations
    unsigned GetId();
    std::string GetGuid();

    const char *ToChar() const;
    std::string GetLastSeenString() const;
    std::string GetLastVisitString() const;

    int id;
    std::string guid;
    int level;
    int lastSeen;
    std::string name;
    std::string title;
    std::string commands;
    std::string greeting;
    std::vector<std::string> hwids;
    unsigned updated;
};

#endif 
