#ifndef CLIENTDATABASE_H
#define CLIENTDATABASE_H

// Datastructure for storing client data
// Doesn't do any sort of checks whether data is correct
// Just saves it so it must be done elsewhere.

extern "C" {
#include "g_local.h"
}

#include <string>

using std::string;

class ClientDatabase {
public:

    void ResetData(int clientNum);

    void setLevel(int clientNum, int level);
    void setGuid(int clientNum, string guid);
    void setUsername(int clientNum, string username);
    void setPassword(int clientNum, string password);
    void setCommands(int clientNum, string commands);
    void setHardwareID(int clientNum, string hardware_id);
    void setGreeting(int clientNum, string greeting);
    void setActive(int clientNum);
    void setInactive(int clientNum);

    int level(int clientNum) const;
    string guid(int clientNum) const;
    string username(int clientNum) const;
    string password(int clientNum) const;
    string commands(int clientNum) const;
    string hardwareID(int clientNum) const;
    string greeting(int clientNum) const;

    string getAll(int clientNum) const;

private:
    
    struct Client {
        Client();
        int level;
        string guid;
        string username;
        string password;
        string commands;
        string hardware_id;
        string greeting;
        bool active_client;
    };

    Client clients_[MAX_CLIENTS];
};

#endif