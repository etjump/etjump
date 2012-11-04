#ifndef G_COMMANDDATABASE_H
#define G_COMMANDDATABASE_H

#include "g_admindatabase.h"

#include <map>

struct admin_cmd_t {
        admin_cmd_t(qboolean (*handler) (gentity_t *ent, unsigned skipargs), char flag, const string& function, const string& syntax);
	    qboolean (*handler) (gentity_t *ent, unsigned skipargs);
	    char flag;
	    string function;
	    string syntax;
};

class CommandDatabase {
public:
    CommandDatabase();
    ~CommandDatabase();
    bool add(const string& keyword, 
        qboolean (*handler)(gentity_t *ent, unsigned skipargs), 
        char flag,
        const string& function, 
        const string& syntax);
    admin_cmd_t *find(const string& keyword);
    admin_cmd_t *findMatching(const string& partial, string& keyword);
    string matchingFlags(vector<string>& commands);
private:
    std::map<string, admin_cmd_t> commands_;
    // Copy constructor is forbidden
    CommandDatabase(const CommandDatabase&);
    // Assignment operator is forbidden
    CommandDatabase& operator=(const CommandDatabase&);
};

#endif