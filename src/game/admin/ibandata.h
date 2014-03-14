#ifndef IBANDATA_H
#define IBANDATA_H

class IBanData
{
public:
    virtual bool AddBan(const std::string& guid, 
        const std::string& ip,
        const std::string& hwid, 
        int expires,
        int date_banned,
        const std::string& name,
        const std::string& banner,
        const std::string& reason,
        std::string& errorMsg) = 0;

    virtual bool Banned(const std::string& guid,
        const std::string& ip,
        const std::string& hwid,
        std::string& errorMsg) = 0;

    virtual bool RemoveBan(int id,
        std::string& errorMsg) = 0;
private:

};

#endif // IBANDATA_H