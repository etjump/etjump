#include "database.hpp"
#include "../g_utilities.hpp"

Database::Database()
{
    ID_IT_END = users_.get<0>().end();
    GUID_IT_END = users_.get<1>().end();
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

bool Database::PrepareStatement(const char* query, sqlite3_stmt **stmt)
{
    unsigned rc = sqlite3_prepare_v2(db_, query, -1, stmt, 0);

    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }

    return true;
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

bool Database::BindString(sqlite3_stmt* stmt, int index, const std::string& val)
{
    G_LogPrintf("Binding %s to index %d\n", val.c_str(), index);
    int rc = sqlite3_bind_text(stmt, index, val.c_str(), val.length(), SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

bool Database::AddUserToSQLite(User user)
{
    int rc = 0;
    sqlite3_stmt *stmt = NULL;
    // TODO: do this once and finalize in game shutdown
    if (!PrepareStatement("INSERT INTO users (id, guid, level, lastSeen, name, hwid, title, commands, greeting) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);", &stmt))
    {
        message_ = "Preparing statement failed.";
        return false;
    }


    G_LogPrintf("HWIDS: %s\n", boost::algorithm::join(user->hwids, ",").c_str());
    std::string hwids = boost::algorithm::join(user->hwids, ",");

    if (!BindInt(stmt, 1, user->id) ||
        !BindString(stmt, 2, user->guid) ||
        !BindInt(stmt, 3, user->level) ||
        !BindInt(stmt, 4, user->lastSeen) ||
        !BindString(stmt, 5, user->name) ||
        !BindString(stmt, 6, hwids) ||
        !BindString(stmt, 7, user->title) ||
        !BindString(stmt, 8, user->commands) ||
        !BindString(stmt, 9, user->greeting)
        )
    {
        message_ = "Binding values to statement failed.";
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }

    sqlite3_finalize(stmt);

    return true;
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

bool Database::UserExists(std::string const& guid)
{
    ConstGuidIterator user = GetUserConst(guid);
    if (user != GUID_IT_END) 
    {
        return true;
    }
    return false;
}

std::string Database::GetMessage() const
{
    return message_;
}

bool Database::AddNewHWID(unsigned id, std::string const& hwid)
{
    IdIterator user = GetUser(id);

    if (user != ID_IT_END)
    {
        (*user)->hwids.push_back(hwid);

        sqlite3_stmt *stmt = NULL;
        int rc = 0;
        
        if (!PrepareStatement("UPDATE users SET hwid=? WHERE id=?;", &stmt))
        {
            return false;
        }

        std::string hwids = boost::algorithm::join((*user)->hwids, ",");

        if (!BindString(stmt, 1, hwids) ||
            !BindInt(stmt, 2, id)) {
            return false;
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
            return false;
        }

        return true;
    }
    message_ = "Couldn't find user with id " + ToString(id);
    return false;
}

bool Database::AddUser(std::string const& guid, std::string const& hwid, std::string const& name)
{
    unsigned id = GetHighestFreeId();

    User newUser(new User_s(id, guid, name, hwid));
    
    // Automatically generated type.. :D
    std::pair<detail::bidir_node_iterator<detail::ordered_index_node<detail::ordered_index_node<detail::index_node_base<boost::shared_ptr<User_s>, std::allocator<boost::shared_ptr<User_s> > > > > >, bool> ret = users_.insert(newUser);
    if (!ret.second)
    {
        message_ = "User guid is not unique.";
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
    users_.clear();
    sqlite3_close(db_);
    return true;
}

Database::User_s const* Database::GetUserData(unsigned id) const
{
    ConstIdIterator user = GetUser(id);
    if (user != ID_IT_END)
    {
        return user->get();
    }
    return NULL;
}

Database::User_s const* Database::GetUserData(std::string const& guid) const
{
    ConstGuidIterator user = GetUser(guid);
    if (user != GUID_IT_END)
    {
        return user->get();
    }
    return NULL;
}

bool Database::InitDatabase(char const* config)
{
    int rc = sqlite3_open(GetPath(config).c_str(), &db_);
    char *errMsg = NULL;

    if (rc)
    {
        message_ = std::string("Can't open database: ") + sqlite3_errmsg(db_);
        sqlite3_close(db_);
        return false;
    }

    rc = sqlite3_exec(db_, "CREATE TABLE IF NOT EXISTS users (id INT PRIMARY KEY, guid TEXT UNIQUE NOT NULL, level INT, lastSeen INT, name TEXT, hwid TEXT, title TEXT, commands TEXT, greeting TEXT);",
        NULL, NULL, &errMsg);

    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + errMsg;
        sqlite3_free(errMsg);
        sqlite3_close(db_);
        return false;
    }

    sqlite3_stmt *stmt = NULL;

    if (!PrepareStatement("SELECT id, guid, level, lastSeen, name, hwid, title, commands, greeting FROM users;", &stmt))
    {
        return false;
    }

    const char *val = NULL;
    rc = sqlite3_step(stmt);
    while (rc != SQLITE_DONE)
    {
        User newUser(new User_s());
        
        switch (rc)
        {
        case SQLITE_ROW:
            newUser->id = sqlite3_column_int(stmt, 0);
            val = (const char*)(sqlite3_column_text(stmt, 1));
            newUser->guid = val ? val : "";
            newUser->level = sqlite3_column_int(stmt, 2);
            newUser->lastSeen = sqlite3_column_int(stmt, 3);
            val = (const char*)(sqlite3_column_text(stmt, 4));
            newUser->name = val ? val : "";
            val = (const char*)(sqlite3_column_text(stmt, 5));
            if (val)
            {
                newUser->hwids = split(newUser->hwids, val, boost::algorithm::is_any_of(","));
            }
            val = (const char*)(sqlite3_column_text(stmt, 6));
            newUser->title = val ? val : "";
            val = (const char*)(sqlite3_column_text(stmt, 7));
            newUser->commands = val ? val : "";
            val = (const char*)(sqlite3_column_text(stmt, 8));
            newUser->greeting = val ? val : "";
            users_.insert(newUser);
            G_LogPrintf("User: %s\n", newUser->ToChar());
            break;
        case SQLITE_BUSY:
        case SQLITE_ERROR:
        case SQLITE_MISUSE:
        default:
            message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
            sqlite3_finalize(stmt);
            return false;
        }
        rc = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);

    return true;
}

Database::ConstGuidIterator Database::GetUserConst(std::string const& guid) const
{
    return users_.get<1>().find(guid);
}