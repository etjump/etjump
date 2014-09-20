#include "database.hpp"
#include "../g_utilities.hpp"

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
    int rc = sqlite3_bind_text(stmt, index, val.c_str(), val.length(), SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

bool Database::AddBanToSQLite(Ban ban)
{
    int rc = 0;
    sqlite3_stmt *stmt = NULL;
    // TODO: do this once and finalize in game shutdown
    if (!PrepareStatement("INSERT INTO bans (name, guid, hwid, ip, banned_by, ban_date, expires, reason) VALUES (?, ?, ?, ?, ?, ?, ?, ?);", &stmt))
    {
        return false;
    }


    if (!BindString(stmt, 1, ban->name) ||
        !BindString(stmt, 2, ban->guid) ||
        !BindString(stmt, 3, ban->hwid) ||
        !BindString(stmt, 4, ban->ip) ||
        !BindString(stmt, 5, ban->bannedBy) ||
        !BindString(stmt, 6, ban->banDate) ||
        !BindInt(stmt, 7, ban->expires) ||
        !BindString(stmt, 8, ban->reason))
    {
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

bool Database::AddUserToSQLite(User user)
{
    int rc = 0;
    sqlite3_stmt *stmt = NULL;
    // TODO: do this once and finalize in game shutdown
    if (!PrepareStatement("INSERT INTO users (id, guid, level, lastSeen, name, hwid, title, commands, greeting) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);", &stmt))
    {
        return false;
    }


    G_DPrintf("HWIDS: %s\n", boost::algorithm::join(user->hwids, ",").c_str());
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
    if (user != GuidIterEnd())
    {
        return true;
    }
    return false;
}

bool Database::UserInfo(gentity_t* ent, int id)
{
    ConstIdIterator user = GetUserConst(id);

    if (user == users_.get<0>().end())
    {
        ChatPrintTo(ent, "^3userinfo: ^7no user found with id " + ToString(id));
        return false;
    }

    ChatPrintTo(ent, "^3userinfo: ^7check console for more information.");
    BeginBufferPrint();
    //unsigned id;
    //std::string guid;

    //// non indexed properties
    //int level;
    //unsigned lastSeen;
    //std::string name;
    //std::string title;
    //std::string commands;
    //std::string greeting;
    //std::vector<std::string> hwids;
    //unsigned updated;
    BufferPrint(ent, va("^5ID: ^7%d\n^5GUID: ^7%s\n^5Level: ^7%d\n^5Last seen:^7 %s\n^5Name: ^7%s\n^5Title: ^7%s\n^5Commands: ^7%s\n^5Greeting: ^7%s\n",
        user->get()->id, user->get()->guid.c_str(), user->get()->level, TimeStampToString(user->get()->lastSeen).c_str(), user->get()->name.c_str(), user->get()->title.c_str(), user->get()->commands.c_str(), user->get()->greeting.c_str()));

    FinishBufferPrint(ent, false);
    return true;
}

bool Database::ListUsers(gentity_t* ent, int page)
{
    const int USERS_PER_PAGE = 20;
    int size = users_.size();
    int pages = (size / USERS_PER_PAGE) + 1;
    int i = (page - 1) * USERS_PER_PAGE;
    
    if (page > pages)
    {
        ChatPrintTo(ent, "^3listusers: ^7no page #" + ToString(page));
        return false;
    }

    ConstIdIterator it = users_.get<0>().begin();
    ConstIdIterator end = users_.get<0>().end();

    ChatPrintTo(ent, "^3listusers: ^7check console for more information.");
    BeginBufferPrint();

    BufferPrint(ent, va("Listing page %d/%d\n", page, pages));
    int curr = 0;
    time_t t;
    time(&t);
    BufferPrint(ent, va("%-5s %-10s %-15s %-36s\n", "ID", "Level", "Last seen", "Name"));
    while (it != end)
    {
        if (curr >= i && curr < i + USERS_PER_PAGE)
        {
            BufferPrint(ent, va("%-5d %-10d %-15s %-36s\n", it->get()->id, it->get()->level, (TimeStampDifferenceToString(static_cast<unsigned>(t) - it->get()->lastSeen) + " ago").c_str(), it->get()->name.c_str()));
        }

        curr++;
        it++;
    }
    FinishBufferPrint(ent, false);

    return true;
}

bool Database::Unban(gentity_t* ent, int id)
{
    for (unsigned i = 0, len = bans_.size(); i < len; i++)
    {
        if (bans_[i]->id == (unsigned)id)
        {
            sqlite3_stmt *stmt = NULL;
            if (!PrepareStatement("DELETE FROM bans WHERE id=?;", &stmt))
            {
                return false;
            }

            if (!BindInt(stmt, 1, id))
            {
                return false;
            }

            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                message_ = sqlite3_errmsg(db_);
                return false;
            }

            bans_.erase(bans_.begin() + i);

            return true;
        }
    }
    message_ = "no ban with id " + ToString(id);
    return false;
}

bool Database::ListBans(gentity_t* ent, int page)
{
    const int BANS_PER_PAGE = 10;
    // 0-19, 20-39
    int i = (page - 1) * BANS_PER_PAGE;
    int printed = 0;
    int size = bans_.size();
    int pages = (size / BANS_PER_PAGE) + 1;

    if (page > pages)
    {
        ChatPrintTo(ent, "^3listbans: ^7no page #" + ToString(page));
    }

    ChatPrintTo(ent, "^3listbans: ^7check console for more information.");
    BeginBufferPrint();
    BufferPrint(ent, va("^7Showing page %d/%d\n", page, pages));
    for (; i < size; i++, printed++)
    {
        if (printed == BANS_PER_PAGE)
        {
            break;
        }
        BufferPrint(ent, va("%d %s ^7%s %s %s %s %s\n",
            bans_[i]->id, bans_[i]->name.c_str(),
            bans_[i]->banDate.c_str(),
            bans_[i]->bannedBy.c_str(),
            bans_[i]->expires != 0 ? TimeStampToString(bans_[i]->expires).c_str() : "PERMANENTLY",
            bans_[i]->reason.c_str()));
    }
    FinishBufferPrint(ent, false);
    return true;
}

bool Database::IsBanned(std::string const& guid, std::string const& hwid)
{
    for (unsigned i = 0; i < bans_.size(); i++)
    {
        if ((bans_[i]->guid.length() > 0 && bans_[i]->guid == guid) ||
            (bans_[i]->hwid.length() > 0 && bans_[i]->hwid == hwid))
        {
            return true;
        }
    }
    return false;
}

bool Database::IsIpBanned(std::string const& ip)
{
    for (unsigned i = 0; i < bans_.size(); i++)
    {
        if (bans_[i]->ip.length() > 0 && bans_[i]->ip == ip)
        {
            return true;
        }
    }
    return false;
}

bool Database::BanUser(std::string const& name, std::string const& guid, std::string const& hwid, std::string const& ip, 
    std::string const& bannedBy, std::string const& banDate, unsigned expires, std::string const& reason)
{
    Ban newBan(new Ban_s);

    newBan->name = name;
    newBan->guid = guid;
    newBan->hwid = hwid;
    newBan->ip = ip;
    newBan->bannedBy = bannedBy;
    newBan->banDate = banDate;
    newBan->expires = expires;
    newBan->reason = reason;

    if (!AddBanToSQLite(newBan))
    {
        return false;
    }

    bans_.push_back(newBan);

    return true;
}

bool Database::UserExists(unsigned id)
{
    ConstIdIterator user = GetUserConst(id);
    if (user != IdIterEnd())
    {
        return true;
    }
    return false;
}

std::string Database::GetMessage() const
{
    return message_;
}

bool Database::UpdateLastSeen(unsigned id, unsigned lastSeen)
{
    IdIterator user = GetUser(id);
    if (user != IdIterEnd())
    {
        (*user)->lastSeen = lastSeen;
        
        sqlite3_stmt *stmt = NULL;
        if (!PrepareStatement("UPDATE users SET lastSeen=? WHERE id=?;", &stmt) ||
            !BindInt(stmt, 1, lastSeen) ||
            !BindInt(stmt, 2, id))
        {
            return false;
        }

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            message_ = "Failed to update user's last seen property.";
            return false;
        }

        sqlite3_finalize(stmt);

        return true;
    }
    message_ = "Couldn't find user with id " + ToString(id);
    return false;
}

bool Database::SetLevel(unsigned id, int level)
{
    IdIterator user = GetUser(id);
    if (user != IdIterEnd())
    {   
        user->get()->level = level;
        return Save(user, Updated::LEVEL);
    }

    message_ = "Couldn't find user with id " + ToString(id);
    return false;
}

bool Database::Save(IdIterator user, unsigned updated)
{
    std::vector<std::string> queryOptions;
    if (updated & Updated::COMMANDS)
    {
        queryOptions.push_back("commands=:commands");
    } 

    if (updated & Updated::GREETING)
    {
        queryOptions.push_back("greeting=:greeting");
    }

    if (updated & Updated::LAST_SEEN)
    {
        queryOptions.push_back("lastSeen=:lastSeen");
    }

    if (updated & Updated::LEVEL)
    {
        queryOptions.push_back("level=:level");
    }

    if (updated & Updated::NAME)
    {
        queryOptions.push_back("name=:name");
    }

    if (updated & Updated::TITLE)
    {
        queryOptions.push_back("title=:title");
    }

    std::string query = "UPDATE users SET " + boost::join(queryOptions, ", ") + " WHERE id=:id;";

    sqlite3_stmt *stmt = NULL;
    if (!PrepareStatement(query.c_str(), &stmt))
    {
        message_ = sqlite3_errmsg(db_);
        sqlite3_finalize(stmt);
        return false;
    }

    if (updated & Updated::COMMANDS)
    {
        if (!BindString(stmt, sqlite3_bind_parameter_index(stmt, ":commands"), user->get()->commands.c_str()))
        {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    if (updated & Updated::GREETING)
    {
        if (!BindString(stmt, sqlite3_bind_parameter_index(stmt, ":greeting"), user->get()->greeting.c_str()))
        {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    if (updated & Updated::LAST_SEEN)
    {
        if (!BindInt(stmt, sqlite3_bind_parameter_index(stmt, ":lastSeen"), user->get()->lastSeen))
        {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    if (updated & Updated::LEVEL)
    {
        if (!BindInt(stmt, sqlite3_bind_parameter_index(stmt, ":level"), user->get()->level))
        {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    if (updated & Updated::NAME)
    {
        if (!BindString(stmt, sqlite3_bind_parameter_index(stmt, ":name"), user->get()->name.c_str()))
        {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    if (updated & Updated::TITLE)
    {
        if (!BindString(stmt, sqlite3_bind_parameter_index(stmt, ":title"), user->get()->title.c_str()))
        {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    if (!BindInt(stmt, sqlite3_bind_parameter_index(stmt, ":id"), user->get()->id))
    {
        sqlite3_finalize(stmt);
        return false;
    }

    int rc = 0;
    if (rc = sqlite3_step(stmt) != SQLITE_DONE)
    {
        G_LogPrintf("SQL ERROR: stepping failed when saving user (%d) %s\n", rc, sqlite3_errmsg(db_));
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::AddNewHWID(unsigned id, std::string const& hwid)
{
    IdIterator user = GetUser(id);

    if (user != IdIterEnd())
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
            sqlite3_finalize(stmt);
            return false;
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
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
    bans_.clear();
    sqlite3_close(db_);
    return true;
}

Database::User_s const* Database::GetUserData(unsigned id) const
{
    ConstIdIterator user = GetUser(id);
    if (user != IdIterEnd())
    {
        return user->get();
    }
    return NULL;
}

bool Database::LoadBans()
{
    int rc = 0;
    sqlite3_stmt *stmt = NULL;

    if (!PrepareStatement("SELECT id, name, guid, hwid, ip, banned_by, ban_date, expires, reason FROM bans;", &stmt))
    {
        return false;
    }

    const char *val = NULL;
    rc = sqlite3_step(stmt);
    while (rc != SQLITE_DONE)
    {
        Ban newBan(new Ban_s());
        switch (rc)
        {
        case SQLITE_ROW:
            newBan->id = sqlite3_column_int(stmt, 0);
            val = (const char*)(sqlite3_column_text(stmt, 1));
            newBan->name = val ? val : "";
            val = (const char*)(sqlite3_column_text(stmt, 2));
            newBan->guid = val ? val : "";
            val = (const char*)(sqlite3_column_text(stmt, 3));
            newBan->hwid = val ? val : "";
            val = (const char*)(sqlite3_column_text(stmt, 4));
            newBan->ip = val ? val : "";
            val = (const char*)(sqlite3_column_text(stmt, 5));
            newBan->bannedBy = val ? val : "";
            val = (const char*)(sqlite3_column_text(stmt, 6));
            newBan->banDate = val ? val : "";
            newBan->expires = sqlite3_column_int(stmt, 7);
            val = (const char*)(sqlite3_column_text(stmt, 8));
            newBan->reason = val ? val : "";
            bans_.push_back(newBan);
            G_DPrintf("Ban: %s\n", newBan->ToChar());
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

bool Database::LoadUsers()
{
    int rc = 0;
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
            G_DPrintf("User: %s\n", newUser->ToChar());
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

bool Database::CreateBansTable()
{
    int rc = 0;
    char *errMsg = NULL;

    rc = sqlite3_exec(db_, "CREATE TABLE IF NOT EXISTS bans (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, guid TEXT NOT NULL, hwid TEXT, ip TEXT, banned_by TEXT, ban_date TEXT, expires INT, reason TEXT);",
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

bool Database::CreateUsersTable()
{
    int rc = 0;
    char *errMsg = NULL;

    rc = sqlite3_exec(db_, "CREATE TABLE IF NOT EXISTS users (id INT PRIMARY KEY, guid TEXT UNIQUE NOT NULL, level INT, lastSeen INT, name TEXT, hwid TEXT, title TEXT, commands TEXT, greeting TEXT);",
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

Database::User_s const* Database::GetUserData(std::string const& guid) const
{
    ConstGuidIterator user = GetUser(guid);
    if (user != GuidIterEnd())
    {
        return user->get();
    }
    return NULL;
}

bool Database::InitDatabase(char const* config)
{
    int rc = sqlite3_open(GetPath(config).c_str(), &db_);

    users_.clear();
    bans_.clear();

    if (rc)
    {
        message_ = std::string("Can't open database: ") + sqlite3_errmsg(db_);
        sqlite3_close(db_);
        return false;
    }

    if (!CreateUsersTable() ||
        !CreateBansTable()) {
        return false;
    }

    if (!LoadUsers() || !LoadBans())
    {
        return false;
    }

    

    return true;
}

Database::ConstIdIterator Database::IdIterEnd() const
{
    return users_.get<0>().end();
}

Database::ConstGuidIterator Database::GuidIterEnd() const
{
    return users_.get<1>().end();
}

Database::ConstGuidIterator Database::GetUserConst(std::string const& guid) const
{
    return users_.get<1>().find(guid);
}

std::string Database::User_s::GetLastSeenString() const
{
    return TimeStampToString(lastSeen);
}

std::string Database::User_s::GetLastVisitString() const
{
    time_t t;
    time(&t);
    unsigned now = static_cast<unsigned>(t);

    return TimeStampDifferenceToString(now - lastSeen);
}