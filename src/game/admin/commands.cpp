#include "commands.h"
#include "game.h"
#include "sessiondata.h"

extern Game game;

bool Admintest( gentity_t *ent, Arguments argv )
{
    if(!ent)
    {
        ChatPrintAll("^3admintest: ^7console is a level ? user.");
        return true;
    }

    game.session->PrintAdmintest( ent );
    return true;
} 

bool Map( gentity_t *ent, Arguments argv )
{
    if(argv->size() != 2)
    {
        // PrintManual(ent, "map");
        return false;
    }

    if(!MapExists(argv->at(1)))
    {
        ChatPrintTo(ent, "^3map: ^7map " + argv->at(1) + " does not exist.");
        return false;
    }

    trap_SendConsoleCommand(EXEC_APPEND, va("map %s", argv->at(1).c_str()));
    return true;
}
