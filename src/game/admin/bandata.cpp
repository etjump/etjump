#include "bandata.h"
#include "../g_local.hpp"
#include "../g_utilities.hpp"
// TODO: replaced g_local.hpp for printf
// with something that includes less shit

BanData::BanData(): db_(NULL), selectAllBans_(NULL), insertBan_(NULL), deleteBan_(NULL)
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
        if(!PrepareStatements())
        {
            return;
        }
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
    FinalizeStatements();
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
        "name VARCHAR(36),"
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

bool BanData::PrepareStatements()
{
    int rc = sqlite3_prepare_v2(db_, 
        "SELECT expires, ban_date, guid, ip, hwid, reason, banner FROM bans;",
        -1, &selectAllBans_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't prepare selectAllBans statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_prepare_v2(db_,
        "INSERT INTO bans (expires, ban_date, name, guid, ip, hwid, reason, banner) VALUES (?, ?, ?, ?, ?, ?, ?, ?);",
        -1, &insertBan_, 0);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't prepare insertBan statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_prepare_v2(db_, "DELETE FROM bans WHERE guid=? OR hwid=? OR ip=?;", -1, &deleteBan_, 0);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't prepare delete ban statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    return true;
}

bool BanData::LoadBans()
{
    // Should be empty but do it anyway
    bans_.clear();

    int rc = sqlite3_step(selectAllBans_);
    while(rc == SQLITE_ROW)
    {
        int index = 0;
        int expires = sqlite3_column_int(selectAllBans_, index++);
        int ban_date = sqlite3_column_int(selectAllBans_, index++);
        const char *guidPtr = (const char*)sqlite3_column_text(selectAllBans_, index++);
        const char *ipPtr = (const char*)sqlite3_column_text(selectAllBans_, index++);
        const char *hwidPtr = (const char*)sqlite3_column_text(selectAllBans_, index++);
        const char *reasonPtr = (const char*)sqlite3_column_text(selectAllBans_, index++);
        const char *bannerPtr = (const char*)sqlite3_column_text(selectAllBans_, index++);

        BanPtr temp(new Ban);
        temp->expires = expires;
        temp->ban_date = ban_date;
        CharPtrToString(guidPtr, temp->guid);
        CharPtrToString(ipPtr, temp->ip);
        CharPtrToString(hwidPtr, temp->hwid);
        CharPtrToString(reasonPtr, temp->reason);
        CharPtrToString(bannerPtr, temp->banner);

        bans_.push_back(temp);

        rc = sqlite3_step(selectAllBans_);
    }

    return true;
}

bool BanData::CloseDatabase()
{
    sqlite3_close(db_);
    db_ = NULL;
    return true;
}

bool BanData::FinalizeStatements()
{
    sqlite3_finalize(selectAllBans_);
    selectAllBans_ = NULL;
    sqlite3_finalize(insertBan_);
    insertBan_ = NULL;
    sqlite3_finalize(deleteBan_);
    deleteBan_ = NULL;
    return true;
}

bool BanData::AddBan(std::string const& guid, 
                     std::string const& ip, 
                     std::string const& hwid, 
                     int expires, int date_banned, 
                     std::string const& name, 
                     std::string const& banner, 
                     std::string const& reason, 
                     std::string& errorMsg)
{
    BanPtr temp(new Ban);
    temp->name = name;
    temp->guid = guid;
    temp->ip = ip;
    temp->hwid = hwid;
    temp->expires = expires;
    temp->ban_date = date_banned;
    temp->banner = banner;
    temp->reason = reason;
    bans_.push_back(temp);

    int index = 1;
    sqlite3_reset(insertBan_);
    int rc = sqlite3_bind_int(insertBan_, index++, expires);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind expires to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    rc = sqlite3_bind_int(insertBan_, index++, date_banned);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind ban date to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_bind_text(insertBan_, index++, name.c_str(), name.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind name to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_bind_text(insertBan_, index++, guid.c_str(), guid.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind guid to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    rc = sqlite3_bind_text(insertBan_, index++, ip.c_str(), ip.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind ip to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    rc = sqlite3_bind_text(insertBan_, index++, hwid.c_str(), hwid.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind hwid to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    rc = sqlite3_bind_text(insertBan_, index++, reason.c_str(), reason.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind reason to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    rc = sqlite3_bind_text(insertBan_, index++, banner.c_str(), banner.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind banner to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_step(insertBan_);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Couldn't execute insert ban statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

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

    DeleteFromDatabase(id);

    bans_.erase(bans_.begin() + id);

    return true;
}

void BanData::DeleteFromDatabase(int id)
{
    std::vector<BanPtr>::iterator it = bans_.begin() + id;

    if(it == bans_.end())
    {
        return;
    }

    int index = 1;
    sqlite3_reset(deleteBan_);
    int rc = sqlite3_bind_text(deleteBan_, index++, (it->get()->guid.length() > 0 ? it->get()->guid.c_str() : "NOGUID"), it->get()->guid.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK) 
    {
        G_LogPrintf("Couldn't bind guid to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    rc = sqlite3_bind_text(deleteBan_, index++, (it->get()->hwid.length() > 0 ? it->get()->hwid.c_str() : "NOHWID"), it->get()->hwid.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK) 
    {
        G_LogPrintf("Couldn't bind hwid to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    rc = sqlite3_bind_text(deleteBan_, index++, (it->get()->ip.length() > 0 ? it->get()->ip.c_str() : "NOIP"), it->get()->ip.length(), SQLITE_STATIC);
    if(rc != SQLITE_OK) 
    {
        G_LogPrintf("Couldn't bind ip to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    rc = sqlite3_step(deleteBan_);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Couldn't execute deleteBan statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }    
}

void BanData::ListBans(int clientNum)
{
    std::vector<BanPtr>::const_iterator it =
        bans_.begin();

    gentity_t *ent = NULL;

    if(clientNum >= 0)
    {
        ent = g_entities + clientNum;
    } 
    
    ChatPrintTo(ent, "^3listbans: ^7check console for more information.");
    BeginBufferPrint();
    int id = 1;
    BufferPrint(ent, va("^5Listing %d bans.\n", bans_.size()));
    BufferPrint(ent, "^5id | name | ban date | banner | expires | reason \n");
    while(it != bans_.end())
    {
        BufferPrint(ent, va("^5%d ^7%s^7 %s %s %s %s\n",
            id, it->get()->name.c_str(), TimeStampToString(it->get()->ban_date).c_str(),
            it->get()->banner.c_str(), TimeStampToString(it->get()->expires).c_str(), it->get()->reason.c_str()));
        it++;
    }
    FinishBufferPrint(ent, false);
}

BanData::Ban::Ban(): expires(0), ban_date(0),
    guid(""), ip(""), hwid(""), reason(""), banner("")
{
}