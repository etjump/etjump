#ifndef g_admin_h__
#define g_admin_h__

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <map>

namespace Admin
{
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
}

struct AdminCommand_s
{
    AdminCommand_s(boost::function<bool(gentity_t *ent, Arguments argv)> handler,
        char flag, const std::string& function, const std::string& syntax);
    boost::function<bool(gentity_t *ent, Arguments argv)> handler;
    char flag;
    std::string function;
    std::string syntax;
};

namespace AdminCommand
{
    bool Magical8Ball(gentity_t *ent, Arguments argv);
    bool Admintest(gentity_t *ent, Arguments argv);
    bool EditUser(gentity_t *ent, Arguments argv);
    bool Finger(gentity_t *ent, Arguments argv);
    bool Help(gentity_t *ent, Arguments argv);
    bool Kick(gentity_t *ent, Arguments argv);
    bool Cancelvote(gentity_t *ent, Arguments argv);
    bool Passvote(gentity_t *ent, Arguments argv);
    bool RemoveSaves(gentity_t *ent, Arguments argv);
    bool Setlevel(gentity_t *ent, Arguments argv);
    bool Map( gentity_t *ent, Arguments argv );
    bool Mute( gentity_t *ent, Arguments argv );
}

typedef std::map<std::string, AdminCommand_s> CommandList;

#endif // g_admin_h__
