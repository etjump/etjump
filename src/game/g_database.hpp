#ifndef g_database_h__
#define g_database_h__

#include "g_local.hpp"
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

class LevelDatabase;
class SQLite;

struct UserData_s
{
    UserData_s();
    int id;
    int level;
    std::string hwid;
    std::string name;
    std::string personalCmds;
    std::string personalTitle;
    std::string personalGreeting;
};
typedef boost::shared_ptr<UserData_s> UserData;

class Database
{
public:
    typedef std::map<std::string, UserData>::const_iterator ConstUserIter;
    typedef std::map<std::string, UserData>::iterator UserIter;

    Database();
    ~Database();

    static void Init();
    static void Shutdown();

    static void ClientGuidReceived(gentity_t *ent, const std::string& guid);
    static void SaveUser(const std::string& guid, UserData u);
    static void PrintAdminTest(gentity_t *ent);
    static void PrintFinger(gentity_t *ent, gentity_t *target);
    static bool SetLevel(gentity_t *ent, gentity_t *target, int level);
    static bool IDSetLevel(gentity_t *ent, int id, int level);
    static void UpdateUserByID(gentity_t *ent, int id, int updated,
        int level, const std::string& commands, const std::string& greeting,
        const std::string& title);
    static void UpdateUserByGUID(gentity_t *ent, const std::string& guid,
        int updated, int level, const std::string& commands, const std::string& greeting,
        const std::string& title);
private:
    // Database connection to store users
    static SQLite udb_;
    // Database connection to store levels
    static LevelDatabase ldb_;

    static int highestId_;

    // Users stored on memory
    static std::map<std::string, UserData> users_;

};

#endif // g_database_h__
