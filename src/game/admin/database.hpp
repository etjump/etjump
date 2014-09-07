#ifndef DATABASE_HH
#define DATABASE_HH
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "../g_local.hpp"
#include <sqlite3.h>
#include <vector>
#include <boost/multi_index/ordered_index.hpp>

using namespace boost::multi_index;

// Loads the data from SQLite database to memory as
// some servers don't support threads making it impossible
// to just query everything from database.

namespace Updated
{
    const unsigned NONE = 0;
    const unsigned LEVEL = 0x00001;
    const unsigned LAST_SEEN = 0x00002;
    const unsigned NAME = 0x00004;
    const unsigned TITLE = 0x00008;
    const unsigned COMMANDS = 0x00010;
    const unsigned GREETING = 0x00020;
}


class Database
{
public:
    Database();
    ~Database();

    struct User_s
    {
        // Needed for multi index container
        unsigned GetId()
        {
            return id;
        }
        // Needed for multi index container
        std::string GetGuid()
        {
            return guid;
        }

        User_s() : level(0), lastSeen(0), updated(0)
        {
            
        }

        User_s(unsigned id, const std::string& guid, const std::string& name, const std::string& hwid)
            : id(id), guid(guid), name(name), level(0), lastSeen(0), updated(0)
        {
            split(hwids, hwid, boost::algorithm::is_any_of(", "));
        }

        User_s(unsigned id, std::string const& guid, int level, unsigned lastSeen, 
            std::string const& name, std::string const& hwid, std::string const& title, 
            std::string const& commands, std::string const& greeting)
            : id(id), guid(guid), level(level), lastSeen(lastSeen), name(name), title(title), commands(commands), greeting(greeting), updated(0)
        {
            split(hwids, hwid, boost::algorithm::is_any_of(","));
        }

        const char *ToChar() const
        {
            return va("%d %s %d %d %s %s %s %s %s", id, guid.c_str(), level, lastSeen, name.c_str(), (boost::algorithm::join(hwids, ", ")).c_str(), title.c_str(), commands.c_str(), greeting.c_str());
        }

        std::string GetLastSeenString() const;
        std::string GetLastVisitString() const;
        unsigned id;
        std::string guid;

        // non indexed properties
        int level;
        unsigned lastSeen;
        std::string name;
        std::string title;
        std::string commands;
        std::string greeting;
        std::vector<std::string> hwids;
        unsigned updated;
    };


    typedef boost::shared_ptr<User_s> User;

    typedef multi_index_container<
        User,
        indexed_by<
        ordered_unique<mem_fun<User_s, unsigned, &User_s::GetId> >,
        ordered_unique<mem_fun<User_s, std::string, &User_s::GetGuid> >
        >
    > Users;

    typedef Users::nth_index<0>::type::iterator IdIterator;
    typedef Users::nth_index<0>::type::const_iterator ConstIdIterator;
    typedef Users::nth_index<1>::type::iterator GuidIterator;
    typedef Users::nth_index<1>::type::const_iterator ConstGuidIterator;

    struct Ban_s
    {
        unsigned id;
        std::string name;
        std::string guid;
        std::string hwid;
        std::string ip;
        std::string bannedBy;
        std::string banDate;
        std::string reason;
        unsigned expires;

        Ban_s() : id(0), expires(0)
        {
            
        }

        const char *ToChar()
        {
            return va("%d %s %s %s %s %s %s %d %s", id, name.c_str(), guid.c_str(), hwid.c_str(), ip.c_str(), bannedBy.c_str(), banDate.c_str(), expires, reason.c_str());
        }
    };

    typedef boost::shared_ptr<Ban_s> Ban;    

    const User_s *GetUserData(unsigned id) const;
    const User_s *GetUserData(const std::string& guid) const;

    bool CreateUsersTable();
    bool CreateBansTable();
    bool LoadUsers();
    bool LoadBans();

    bool InitDatabase(const char *config);
    bool CloseDatabase();
    // When user is added, all we have is the guid, hwid, name, lastSeen and level
    // Adds user to database
    bool AddUser(const std::string& guid, const std::string& hwid, const std::string& name);
    bool AddNewHWID(unsigned id, const std::string& hwid);
    bool UpdateLastSeen(unsigned id, unsigned lastSeen);
    bool SetLevel(unsigned id, int level);
    bool Save(IdIterator user, unsigned updated);
    std::string GetMessage() const;
    bool UserExists(const std::string& guid);
    bool UserExists(unsigned id);
    bool BanUser(const std::string& name, const std::string& guid, 
        const std::string& hwid, const std::string& ip, const std::string& bannedBy, 
        const std::string& banDate, unsigned expires, const std::string& reason);
    bool IsIpBanned(const std::string& ip);
    bool IsBanned(const std::string& guid, const std::string& hwid);
    bool ListBans(gentity_t *ent, int page);
    bool Unban(gentity_t *ent, int id);
    bool ListUsers(gentity_t *ent, int page);
    bool UserInfo(gentity_t *ent, int id);
private:
    unsigned GetHighestFreeId() const;
    bool AddUserToSQLite(User user);
    bool AddBanToSQLite(Ban ban);
    bool BindInt(sqlite3_stmt* stmt, int index, int val);
    bool BindString(sqlite3_stmt* stmt, int index, const std::string& val);
    IdIterator GetUser(unsigned id) const;
    ConstIdIterator GetUserConst(unsigned id) const;
    GuidIterator GetUser(const std::string& guid) const;
    bool PrepareStatement(char const* query, sqlite3_stmt** stmt);
    ConstGuidIterator GetUserConst(const std::string& guid) const;
    Users users_;
    std::vector<Ban> bans_;
    sqlite3 *db_;
    std::string message_;

    ConstIdIterator IdIterEnd() const;
    ConstGuidIterator GuidIterEnd() const;
};

#endif // DATABASE_HH