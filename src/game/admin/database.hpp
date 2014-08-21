#ifndef DATABASE_HH
#define DATABASE_HH
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/shared_ptr.hpp>
#include <sqlite3.h>

using namespace boost::multi_index;

// Loads the data from SQLite database to memory as
// some servers don't support threads making it impossible
// to just query everything from database.

class Database
{
public:
    Database();
    ~Database();

    /* tags for accessing the corresponding indices of employee_set */
    struct id{};
    struct guid{};

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

        User_s(unsigned id, std::string const& guid, int level, unsigned lastSeen, 
            std::string const& name, std::string const& hwid, std::string const& title, 
            std::string const& commands, std::string const& greeting)
            : id(id), guid(guid), level(level), lastSeen(lastSeen), name(name), hwid(hwid), title(title), commands(commands), greeting(greeting)
        {
        }

        unsigned id;
        std::string guid;

        // non indexed properties
        int level;
        unsigned lastSeen;
        std::string name;
        std::string hwid;
        std::string title;
        std::string commands;
        std::string greeting;
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

    IdIterator GetUser(unsigned id) const;
    ConstIdIterator GetUserConst(unsigned id) const;
    GuidIterator GetUser(const std::string& guid) const;
    ConstGuidIterator GetUserConst(const std::string& guid) const;

    bool InitDatabase();
    bool CloseDatabase();
    // When user is added, all we have is the guid, hwid, name, lastSeen and level
    // Adds user to database
    bool AddUser(const std::string& guid, const std::string& hwid, const std::string& name);
    std::string GetMessage() const;
private:
    unsigned GetHighestFreeId() const;
    bool AddUserToSQLite(User user);
    bool BindInt(sqlite3_stmt* stmt, int index, int val);
    bool BindString(sqlite3_stmt* stmt, int index, std::string val);
    Users users_;
    sqlite3 *db_;
    std::string message_;
};

#endif // DATABASE_HH