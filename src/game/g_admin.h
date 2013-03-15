#ifndef G_ADMIN_H
#define G_ADMIN_H

#include <string>
#include <bitset>

using std::string;

namespace admin {

    using std::string;

    const unsigned MAX_COMMANDS = 256;
    const int UNKNOWN_ID = -1;

    struct Client {
        Client();

        // Unique sqlite ID
        int database_id;
        // Admin level
        int level;
        // Users 40-char guid
        string guid;
        // Users username, no use for now
        string username;
        // Users password, no use for now
        string password;
        // Users personal greeting, overwrites the level greeting
        string greeting;
        // Nick of the user when he was first seen/last time level was set
        string name;
        // Users ip the first time he was seen/last time level was set
        string ip;
        // If true prints greeting
        bool need_greeting;
        // Bitset of commands, used to check whether user can use a certain command or not
        std::bitset<MAX_COMMANDS> commands;
        // Better store personal command string here for easier updates
        string personal_commands;

    };

    struct AdminCommand {
        string keyword;
        qboolean (*handler)(gentity_t *ent, unsigned skipargs);
        char flag;
        string function;
        string syntax;
    };

    string Guid(gentity_t *ent);
}

void UpdatePermissions(int clientNum);

#endif
