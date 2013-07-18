#ifndef g_database_h__
#define g_database_h__

#include "g_local.hpp"
#include "g_sqlite.hpp"
#include "g_levels.hpp"
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

typedef std::map<std::string, UserData>::const_iterator ConstUserIter;
typedef std::map<std::string, UserData>::iterator UserIter;

class Database
{
public:
    Database();
    ~Database();

    void Init();
    void Shutdown();

    void ClientGuidReceived(gentity_t *ent, const std::string& guid);
    void SaveUser(const std::string& guid, UserData u);
    void PrintAdminTest(gentity_t *ent);
    bool SetLevel(gentity_t *ent, gentity_t *target, int level);
    bool IDSetLevel(gentity_t *ent, int id, int level);
private:
    // Database connection to store users
    SQLite udb_;
    // Database connection to store levels
    LevelDatabase ldb_;

    int highestId_;

    // Users stored on memory
    std::map<std::string, UserData> users_;
};

#endif // g_database_h__
