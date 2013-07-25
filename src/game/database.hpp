#ifndef database_h__
#define database_h__

/*

#include "g_local.hpp"
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

class Database_new
{
public:

    typedef std::map<std::string, UserData>::const_iterator ConstUserIter;
    typedef std::map<std::string, UserData>::iterator UserIter;

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

    // These are useless as all the initialization stuff is done on every
    // game init..
    Database();
    ~Database();

    static void Init();
    static void Shutdown();
    static void ClientGuidReceived(gentity_t *ent, const std::string& guid);
    static void SaveUser(const std::string& guid, UserData u);
    static void PrintAdminTest(gentity_t *ent);
    static bool SetLevel(gentity_t *ent, gentity_t *target, int level);
    static bool IDSetLevel(gentity_t *ent, int id, int level);
      
private:
    int highestId_;
    std::map<std::string, UserData> users_;
}

*/

#endif // database_h__
