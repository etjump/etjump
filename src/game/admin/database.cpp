#include "database.hpp"

Database::Database()
{
}

Database::~Database()
{
}

Database::IdIterator Database::GetUser(unsigned id) const
{
    return users_.get<0>().find(id);
}

Database::ConstIdIterator Database::GetUserConst(unsigned id) const
{
    return users_.get<0>().find(id);
}

Database::GuidIterator Database::GetUser(std::string const& guid) const
{
    return users_.get<1>().find(guid);
}

bool Database::BindInt(sqlite3_stmt* stmt, int index, int val)
{
    int rc = sqlite3_bind_int(stmt, index, val);
    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

bool Database::BindString(sqlite3_stmt* stmt, int index, std::string val)
{
    int rc = sqlite3_bind_text(stmt, index, val.c_str(), 0, 0);
    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

bool Database::AddUserToSQLite(User user)
{
    sqlite3_stmt *stmt = NULL;
    unsigned rc = sqlite3_prepare_v2(db_, "INSERT INTO users (id, guid, level, lastSeen, name, hwid, title, commands, greeting) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);", NULL, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }

    if (!BindInt(stmt, 1, user->id) ||
        !BindString(stmt, 2, user->guid) ||
        !BindInt(stmt, 3, user->level) ||
        !BindInt(stmt, 4, user->lastSeen) ||
        !BindString(stmt, 5, user->name) ||
        !BindString(stmt, 6, user->hwid) ||
        !BindString(stmt, 7, user->title) ||
        !BindString(stmt, 8, user->commands) ||
        !BindString(stmt, 9, user->greeting)
        )
    {
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }

    return false;
}

unsigned Database::GetHighestFreeId() const
{
    // if it's empty, let's start from 1
    if (users_.get<0>().empty())
    {
        return 1;
    }
    return users_.get<0>().rbegin()->get()->id + 1;
}

std::string Database::GetMessage() const
{
    return message_;
}

bool Database::AddUser(std::string const& guid, std::string const& hwid, std::string const& name)
{
    unsigned id = GetHighestFreeId();

    User newUser(new User_s(id, guid, 0, 0, name, hwid, "", "", ""));

    // Automatically generated type.. :D
    std::pair<detail::bidir_node_iterator<detail::ordered_index_node<detail::ordered_index_node<detail::index_node_base<boost::shared_ptr<User_s>, std::allocator<boost::shared_ptr<User_s> > > > > >, bool> ret = users_.insert(newUser);
    if (!ret.second)
    {
        return false;
    }

    if (!AddUserToSQLite(newUser))
    {
        return false;
    }

    return true;
}

bool Database::CloseDatabase()
{
    sqlite3_close(db_);
    return true;
}

bool Database::InitDatabase()
{
    int rc = sqlite3_open("etjump.db", &db_);
    char *errMsg = NULL;

    if (rc)
    {
        message_ = std::string("Can't open database: ") + sqlite3_errmsg(db_);
        sqlite3_close(db_);
        return false;
    }

    rc = sqlite3_exec(db_, "CREATE TABLE IF NOT EXISTS users (id INT PRIMARY KEY, guid VARCHAR(40) UNIQUE NOT NULL, level INT, lastSeen INT, name VARCHAR(36), hwid VARCHAR(40), title VARCHAR(256), commands VARCHAR(256), greeting VARCHAR(256));",
        NULL, NULL, &errMsg);

    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + errMsg;
        sqlite3_free(errMsg);
        sqlite3_close(db_);
        return false;
    }
    return true;

}

Database::ConstGuidIterator Database::GetUserConst(std::string const& guid) const
{
    return users_.get<1>().find(guid);
}