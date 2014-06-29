#include "commands.hpp"
#include "../g_local.hpp"
#include "../g_save.hpp"

typedef std::map< std::string, boost::function<bool(gentity_t *ent, Arguments argv)> >::const_iterator ConstCommandIterator;
typedef std::map< std::string, boost::function<bool(gentity_t *ent, Arguments argv)> >::iterator CommandIterator;


namespace ClientCommands
{
    bool BackupLoad(gentity_t *ent, Arguments argv)
    {
        game.saves->LoadBackupPosition(ent);
        return true;
    }

    bool Load(gentity_t *ent, Arguments argv)
    {
        game.saves->Load(ent);
        return true;
    }

    bool Save(gentity_t *ent, Arguments argv)
    {
        game.saves->Save(ent);
        return true;
    }
}

Commands::Commands()
{
    commands_["backup"] = ClientCommands::BackupLoad;
    commands_["save"] = ClientCommands::Save;
    commands_["load"] = ClientCommands::Load;
}

bool Commands::ClientCommand(gentity_t* ent, std::string commandStr)
{
    G_DPrintf("Commands::ClientCommand called for %d\n", ClientNum(ent));

    ConstCommandIterator command = commands_.find(commandStr);
    if (command == commands_.end())
    {
        return false;
    } 
     
    command->second(ent, GetArgs());

    return true;
}

bool Commands::AdminCommand(gentity_t* ent)
{
    return false;
}