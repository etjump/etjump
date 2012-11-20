#include "g_clientdatabase.h"

ClientDatabase::Client::Client() {
    level = 0;
}

void ClientDatabase::ResetData(int clientNum) {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return;
    }

    setInactive(clientNum);
    clients_[clientNum].guid.clear();
    clients_[clientNum].commands.clear();
    clients_[clientNum].hardware_id.clear();
    clients_[clientNum].level = 0;
    clients_[clientNum].password.clear();
    clients_[clientNum].username.clear();
}

void ClientDatabase::setLevel(int clientNum, int level) {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return;
    }

    setActive(clientNum);
    clients_[clientNum].level = level;
}

void ClientDatabase::setGuid(int clientNum, string guid) {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return;
    }

    // If guid exists theres no point setting it again
    if(clients_[clientNum].guid.length()) {
        return;
    }

    setActive(clientNum);
    clients_[clientNum].guid = guid;
}

void ClientDatabase::setUsername(int clientNum, string username) {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return;
    }
    
    setActive(clientNum);
    clients_[clientNum].username = username;
}

void ClientDatabase::setPassword(int clientNum, string password) {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return;
    }

    setActive(clientNum);
    clients_[clientNum].password = password;
}

void ClientDatabase::setCommands(int clientNum, string commands) {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return;
    }

    setActive(clientNum);
    clients_[clientNum].commands = commands;
}

void ClientDatabase::setHardwareID(int clientNum, string hardware_id) {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return;
    }

    setActive(clientNum);
    clients_[clientNum].hardware_id = hardware_id;
}

void ClientDatabase::setActive(int clientNum) {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return;
    }

    clients_[clientNum].active_client = true;
}

void ClientDatabase::setInactive(int clientNum) {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return;
    }
    clients_[clientNum].active_client = false;

}



int ClientDatabase::level(int clientNum) const {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return -1;
    }

    return clients_[clientNum].level;
}

string ClientDatabase::guid(int clientNum) const {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return "";
    }

    return clients_[clientNum].guid;
}

string ClientDatabase::username(int clientNum) const {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return "";
    }

    return clients_[clientNum].username;
}

string ClientDatabase::password(int clientNum) const {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return "";
    }

    return clients_[clientNum].password;
}

string ClientDatabase::commands(int clientNum) const {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return "";
    }

    return clients_[clientNum].commands;
}

string ClientDatabase::hardwareID(int clientNum) const {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return "";
    }

    return clients_[clientNum].hardware_id;
}

string ClientDatabase::getAll(int clientNum) const {
    if(clientNum < 0 || clientNum > MAX_CLIENTS) {
        return "ClientDatabase: invalid clientNum";
    }

    return string("---------------------------------------------------\n") + 
           string("- Client \n") +
           string("---------------------------------------------------\n") +
           string("- GUID: ") + clients_[clientNum].guid + 
           string("\n- USER: ") + clients_[clientNum].username + 
           string("\n- PASS: ") + clients_[clientNum].password +
           string("\n- CMDS: ") + clients_[clientNum].commands +
           string("\n- HWID: ") + clients_[clientNum].hardware_id +
           string("\n- ACTV: ") + (clients_[clientNum].active_client ? string("YES") : string("NO")) +
           string("\n---------------------------------------------------\n");
}