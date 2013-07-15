#ifndef g_database_h__
#define g_database_h__

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include "g_sqlite.hpp"
#include "g_local.hpp"

// Users are stored in a map where GUID is the key
// The key is a regular string
// The actual user data (dbid, hwid, level, name, pcmds, ptitle, pgreet)
// is stored in a struct
// 

struct UserData_s
{
    int id;
    int level;
    std::string hwid;
    std::string name;
    std::string personalCmds;
    std::string personalTitle;
    std::string personalGreeting;
};

typedef boost::shared_ptr<UserData_s> UserData;
typedef std::map<std::string, UserData>::const_iterator ConstUserIterator;
typedef std::map<std::string, UserData>::iterator UserIterator;

class Database
{
public:
    Database();
    ~Database();

    void ClientConnect(gentity_t *ent, const std::string& guid);
    void SetLevel(gentity_t *ent, int level);
private:
    SQLite db_;
    std::map<std::string, UserData> users_;
};

#endif // g_database_h__
