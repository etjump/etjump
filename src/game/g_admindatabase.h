#ifndef ADMINDATABASE
#define ADMINDATABASE

extern "C" {
#include "g_local.h"
}

#include <string>
#include <vector>

using std::string;
using std::vector;


// Where client session data is stored at
// Data such as client guid, client level, password, username
// commands, backup save positions etc.
class ClientDatabase {
public:
    ClientDatabase();
    ~ClientDatabase();
    string guid(int clientNum);
    void setGuid(int clientNum, string guid);
    void setCommands(int clientNum, string commands);
    void setUsername(int clientNum, string username);
    void setPassword(int clientNum, string password);
    void setActive(int clientNum);
    void setInactive(int clientNum);
    void resetClientData(int clientNum);
private:

    struct Client {
        Client();
        int level;
        string guid;
        string username;
        string password;
        // Store personal commands here
        string commands;
        bool active;
    };
    Client clients_[MAX_CLIENTS];
};

struct admin_user_t {
    int level;
    string guid;
    string commands;
    string username;
    string password;
    string ingame_name;
};

struct admin_level_t {
    int level;
    string name;
    string commands;
    string greeting;
};

struct admin_ban_t {
    string name;
    string ip;
    string hardware_id;
    string etguid;
    string reason;
    string date;
    int expires;
    string banned_by;
};

class AdminDatabase
{
public:
    AdminDatabase();
    ~AdminDatabase();

    bool readConfig();
    bool writeConfig();
    bool writeDefaultConfig();
    void clearDatabase();

    /////////////////////
    // Users
    /////////////////////
    // Adds user to database
    bool addUser(admin_user_t user);

    // Deletes user from database
    bool deleteUser(const string& key, int keytype);

    // Updates user in database
    bool updateUser(const string& username, const string& guid,
               int level, const string& commands, const string& password,
               const string& ingame_name);
    // Finds user in database
    admin_user_t *findUser(const string& key, int keytype);

    // Checks if user exists in database
    bool userExists(const string& username, int keytype);

    int userCount() const;

    /////////////////////
    // Levels
    /////////////////////

    bool addLevel(int level, const string& name, const string& commands,
                  const string& greeting);
    bool addLevel(admin_level_t level);

    bool deleteLevel(int level);

    bool updateLevel(int level, const string& name, const string& commands,
                     const string& greeting);

    admin_level_t *findLevel(int level);

    bool levelExists(int level);

    int levelCount() const;

    /////////////////////
    // Bans
    /////////////////////

    bool addBan(const string& name, const string& ip, const string& hardware_id,
                const string& etguid, const string& reason, const string& date,
                int expires, const string& banned_by);
    bool addBan(admin_ban_t ban);

    bool deleteBan(int index);
    bool deleteBan(const string& key, int keytype);

    // If any function returns false, this function returns
    // error message.
    string error() const;

    int banCount() const;

private:

    enum Keytypes {
        USERNAME,
        GUID
    };

    bool readConfig_string(std::stringstream& current_line,
                       string& data);

    bool readConfig_int(std::stringstream& current_line,
                    int& data) ;

    bool readConfig_parseCommands(std::stringstream& current_line,
                              string& commands);

    void sort();
    // Users
    vector<admin_user_t*> admin_users_;
    // Levels
    vector<admin_level_t*> admin_levels_;
    // Bans
    vector<admin_ban_t*> admin_bans_;
    string error_msg_;

    // Copy constructor is forbidden
    AdminDatabase(const AdminDatabase&);
    // Assignment operator is forbidden
    AdminDatabase& operator=(const AdminDatabase&);
};

#endif // ADMINDATABASE