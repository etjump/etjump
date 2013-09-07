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
        char flag);
    boost::function<bool(gentity_t *ent, Arguments argv)> handler;
    char flag;
};

namespace Flag
{
    const char BAN = 'b';
    // For everyone
    const char BASIC = 'a';
    const char CANCELVOTE = 'C';
    const char EBALL = '8';
    const char EDIT = 'A';
    const char FINGER = 'f';
    const char HELP = 'h';
    const char KICK = 'k';
    const char LISTBANS = 'L';
    const char LISTPLAYERS = 'l';
    const char MAP = 'M';
    const char MUTE = 'm';
    const char NOCLIP = 'N';
    const char PASSVOTE = 'P';
    const char PUTTEAM = 'p';
    const char READCONFIG = 'G';
    const char RENAME = 'R';
    const char RESTART = 'r';
    const char SAVESYSTEM = 'T';
    const char SETLEVEL = 's';
}

namespace AdminCommand
{
    bool AddLevel(gentity_t *ent, Arguments argv);
    bool Admintest(gentity_t *ent, Arguments argv);
    bool Ban(gentity_t *ent, Arguments argv);
    bool Cancelvote(gentity_t *ent, Arguments argv);
    bool DeleteLevel(gentity_t *ent, Arguments argv);
    bool DeleteUser(gentity_t *ent, Arguments argv);
    bool EditCommands(gentity_t *ent, Arguments argv);
    bool EditLevel(gentity_t *ent, Arguments argv);
    bool EditUser(gentity_t *ent, Arguments argv);
    bool Finger(gentity_t *ent, Arguments argv);
    bool Help(gentity_t *ent, Arguments argv);
    bool Kick(gentity_t *ent, Arguments argv);
    bool LevelInfo(gentity_t *ent, Arguments argv);
    bool ListBans(gentity_t *ent, Arguments argv);
    bool ListFlags(gentity_t *ent, Arguments argv);
    bool ListMaps(gentity_t *ent, Arguments argv);
    bool ListPlayers(gentity_t *ent, Arguments argv);
    bool ListUsers(gentity_t *ent, Arguments argv);
    bool Magical8Ball(gentity_t *ent, Arguments argv);
    bool Map( gentity_t *ent, Arguments argv );
    bool Mute( gentity_t *ent, Arguments argv );
    bool Noclip(gentity_t *ent, Arguments argv);
    bool Nogoto(gentity_t *ent, Arguments argv);
    bool Nosave(gentity_t *ent, Arguments argv);
    bool Passvote(gentity_t *ent, Arguments argv);
    bool Putteam(gentity_t *ent, Arguments argv);
    bool ReadConfig(gentity_t *ent, Arguments argv);
    bool RemoveSaves(gentity_t *ent, Arguments argv);
    bool Rename(gentity_t *ent, Arguments argv);
    bool Restart(gentity_t *ent, Arguments argv);
    bool Setlevel(gentity_t *ent, Arguments argv);
    bool Unban(gentity_t *ent, Arguments argv);
    bool Unmute(gentity_t *ent, Arguments argv);
    bool UserInfo(gentity_t *ent, Arguments argv);
}

typedef std::map<std::string, AdminCommand_s> CommandList;

#endif // g_admin_h__
