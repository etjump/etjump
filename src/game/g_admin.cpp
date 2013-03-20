#include "g_utilities.h"
#include "g_users.h"

extern "C" {
#include "g_local.h"
};

// Calculates the additional length needed for formating with color codes
int StringColorLen(const std::string& str) {
    return (str.length() - CleanStrlen(str.c_str()));
}

void Svcmd_ListAliases_f(void) {
    Arguments argv = GetArgs();

    if(argv->size() != 2) {
        G_Printf("usage: listaliases <player>");
        return;
    }

    char errmsg[MAX_TOKEN_CHARS];
    gentity_t *target = PlayerGentityFromString(argv->at(1),
        errmsg, sizeof(errmsg));

    if(!target) {
        G_Printf("listaliases: %s\n", errmsg);
        return;
    }

    const std::vector<std::string> *aliases = 
        users.GetAliases(target);

    unsigned printed = 0;
    BeginBufferPrint();
    BufferPrint(NULL, target->client->pers.netname + std::string("'s aliases:\n"));
    for(std::vector<std::string>::const_iterator it = aliases->begin();
        it != aliases->end(); it++)
    {
        if( printed != 0 && printed % 4 == 0 ) {
            BufferPrint(NULL, "\n");
        }
        BufferPrint(NULL, va("^7%-*s", 36 + StringColorLen(*it), (*it).c_str()));
        printed++;
    }
    BufferPrint(NULL, "\nFound " + IntToString(aliases->size()) + " aliases.\n");
    FinishBufferPrint(NULL, false);
}