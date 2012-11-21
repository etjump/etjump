#ifndef G_COMMANDDATABASE_H
#define G_COMMANDDATABASE_H

extern "C" {
#include "g_local.h"
}

#include <string>

using std::string;

struct AdminCommand {
        string keyword;
        qboolean (*handler)(gentity_t *ent, int skipargs);
        char flag;
        string function;
        string syntax;
};

class CommandDatabase {
public:
    CommandDatabase();
    ~CommandDatabase();
    AdminCommand *Command(const string& keyword);
    
private:
};

#endif