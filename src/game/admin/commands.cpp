#include "commands.hpp"
#include "../g_local.hpp"

namespace ClientCommands
{
    bool BackupLoad(gentity_t *ent, Arguments argv)
    {
        return true;
    }

    bool Load(gentity_t *ent, Arguments argv)
    {
        return true;
    }

    bool Save(gentity_t *ent, Arguments argv)
    {
        return true;
    }
}

Commands::Commands()
{
    commands_["save"] = ClientCommands::Save;
}

bool Commands::ClientCommand(gentity_t *ent)
{
    G_DPrintf("Commands::ClientCommand called for %d\n", ClientNum(ent));
    return false;
}

bool Commands::AdminCommand(gentity_t* ent)
{
    return false;
}