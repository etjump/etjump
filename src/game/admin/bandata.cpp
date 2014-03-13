#include "bandata.h"
#include "../g_local.hpp"
#include "../g_utilities.hpp"
// TODO: replaced g_local.hpp for printf
// with something that includes less shit

BanData::BanData(): db_(NULL)
{
}

BanData::~BanData()
{
}

void BanData::Initialize(const std::string& banDatabaseFileName)
{
    // TODO: Load bans from DB
    if(OpenDatabase(banDatabaseFileName) && CreateBansTable())
    {
        if(!LoadBans())
        {
            G_LogPrintf("ERROR: failed to initialize ban database.\n");
        }
    }
}

void BanData::Shutdown()
{
    // shared_ptr handles memory management
    bans_.clear();
    CloseDatabase();
}

bool BanData::OpenDatabase(std::string banDatabaseFileName)
{
    if(banDatabaseFileName.length() == 0)
    {
        banDatabaseFileName = GetPath("bans.dat");
    }
    int rc = sqlite3_open(banDatabaseFileName.c_str(),
        &db_);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't open ban database: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    return true;
}

bool BanData::CreateBansTable()
{
    int rc = sqlite3_exec(db_, 
        "CREATE TABLE IF NOT EXISTS bans("
        "id INTEGER PRIMARY KEY,"
        "expires INTEGER,"
        "ban_date INTEGER,"
        "guid VARCHAR(40),"
        "ip VARCHAR(15),"
        "hwid VARCHAR(40),"
        "reason VARCHAR(255),"
        "banner VARCHAR(36));", NULL, NULL, NULL);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't create bans table: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    return true;
}

bool BanData::LoadBans()
{
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db_, 
        "SELECT expires, ban_date, guid, ip, hwid, reason, banner FROM bans;",
        -1, &stmt, 0);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't prepare loadBans statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    // Should be empty but do it anyway
    bans_.clear();

    rc = sqlite3_step(stmt);
    while(rc == SQLITE_ROW)
    {
        int index = 0;
        int expires = sqlite3_column_int(stmt, index++);
        int ban_date = sqlite3_column_int(stmt, index++);
        const char *guidPtr = (const char*)sqlite3_column_text(stmt, index++);
        const char *ipPtr = (const char*)sqlite3_column_text(stmt, index++);
        const char *hwidPtr = (const char*)sqlite3_column_text(stmt, index++);
        const char *reasonPtr = (const char*)sqlite3_column_text(stmt, index++);
        const char *bannerPtr = (const char*)sqlite3_column_text(stmt, index++);

        BanPtr temp(new Ban);
        temp->expires = expires;
        temp->ban_date = ban_date;
        CharPtrToString(guidPtr, temp->guid);
        CharPtrToString(ipPtr, temp->ip);
        CharPtrToString(hwidPtr, temp->hwid);
        CharPtrToString(reasonPtr, temp->reason);
        CharPtrToString(bannerPtr, temp->banner);

        bans_.push_back(temp);

        rc = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);

    return true;
}

bool BanData::CloseDatabase()
{
    sqlite3_close(db_);
    db_ = NULL;
    return true;
}

bool BanData::AddBan(std::string const& guid, 
                     std::string const& ip, 
                     std::string const& hwid, 
                     int expires,
                     int date_banned,
                     const std::string& banner,
                     const std::string& reason,
                     std::string& errorMsg)
{
    BanPtr temp(new Ban);
    temp->guid = guid;
    temp->ip = ip;
    temp->hwid = hwid;
    temp->expires = expires;
    temp->ban_date = date_banned;
    temp->banner = banner;
    temp->reason = reason;
    bans_.push_back(temp);

    sqlite3_stmt *insertBan = NULL;
    int rc = sqlite3_prepare_v2(db_,
        "INSERT INTO bans (expires, ban_date, guid, ip, hwid, reason, banner) VALUES (?, ?, ?, ?, ?, ?, ?);",
        -1, &insertBan, 0);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't prepare insert ban statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    int index = 1;
    rc = sqlite3_bind_int(insertBan, index++, expires);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind expires to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    rc = sqlite3_bind_int(insertBan, index++, date_banned);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind ban date to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    rc = sqlite3_bind_text(insertBan, index++, guid.c_str(), guid.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind guid to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    rc = sqlite3_bind_text(insertBan, index++, ip.c_str(), ip.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind ip to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    rc = sqlite3_bind_text(insertBan, index++, hwid.c_str(), hwid.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind hwid to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    rc = sqlite3_bind_text(insertBan, index++, reason.c_str(), reason.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind reason to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    rc = sqlite3_bind_text(insertBan, index++, banner.c_str(), banner.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind banner to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_step(insertBan);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Couldn't execute insert ban statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    sqlite3_finalize(insertBan);

    return true;
}

bool BanData::Banned(const std::string& guid,
                     const std::string& ip,
                     const std::string& hwid,
                     std::string& errorMsg)
{
    for(std::vector<BanPtr>::const_iterator it = bans_.begin();
        it != bans_.end(); it++)
    {
        if(guid.length() > 0)
        {
            if(it->get()->guid == guid)
            {
                return true;
            }
        } 
        if(ip.length() > 0)
        {
            if(it->get()->ip == ip)
            {
                return true;
            }
        }
        if(hwid.length() > 0)
        {
            if(it->get()->hwid == hwid)
            {
                return true;
            }
        }
    }

    return false;
}

bool BanData::RemoveBan(int id, std::string& errorMsg)
{
    id -= 1;
    if(id >= bans_.size() || id < 0)
    {
        errorMsg = "invalid ban id.";
        return false;
    }

    DeleteFromDatabase();

    bans_.erase(bans_.begin() + id);

    return true;
}

void BanData::DeleteFromDatabase()
{

}

BanData::Ban::Ban(): expires(0), ban_date(0),
    guid(""), ip(""), hwid(""), reason(""), banner("")
{
}