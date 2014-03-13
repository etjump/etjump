#ifndef BANDATA_H
#define BANDATA_H
#include <string>
#include <boost\shared_ptr.hpp>
#include <vector>
#include "ibandata.h"
#include <sqlite3.h>

class BanData : public IBanData
{
public:

    BanData();
    ~BanData();

    void Initialize(const std::string& banDatabaseFileName);
    void Shutdown();

    bool OpenDatabase(std::string banDatabaseFileName);
    bool CreateBansTable();
    bool LoadBans();
    bool CloseDatabase();

    virtual bool AddBan(const std::string& guid, 
        const std::string& ip,
        const std::string& hwid, 
        int expires,
        int date_banned,
        const std::string& banner,
        const std::string& reason,
        std::string& errorMsg);

    virtual bool Banned(const std::string& guid,
        const std::string& ip,
        const std::string& hwid,
        std::string& errorMsg);

    virtual bool RemoveBan(int id,
        std::string& errorMsg);

    void DeleteFromDatabase();

    struct Ban
    {
        Ban();
        int expires;
        int ban_date;
        std::string guid;
        std::string ip;
        std::string hwid;
        std::string reason;
        std::string banner;
    };

    typedef boost::shared_ptr<Ban> BanPtr;

private:
    std::vector<BanPtr> bans_;

    sqlite3 *db_;
};

#endif // BANDATA_H