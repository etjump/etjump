#include "commandinterpreter.h"
#include "commands.h"
#include "../g_local.hpp"
#include "../g_utilities.hpp"

#include <string>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include "game.h"
#include "sessiondata.h"

extern Game game;

CommandInterpreter::CommandInterpreter()
{
    commands_["addlevel"] = AdminCommand(AddLevel, CommandFlags::EDIT);
    commands_["admintest"] = AdminCommand(Admintest, CommandFlags::BASIC);
    commands_["8ball"] = AdminCommand(Ball8, CommandFlags::BASIC);
    commands_["ban"] = AdminCommand(Ban, CommandFlags::BAN);
    commands_["cancelvote"] = AdminCommand(Cancelvote, CommandFlags::CANCELVOTE);
    commands_["deletelevel"] = AdminCommand(DeleteLevel, CommandFlags::EDIT);
    commands_["deleteuser"] = AdminCommand(DeleteUser, CommandFlags::EDIT);
    commands_["editcommands"] = AdminCommand(EditCommands, CommandFlags::EDIT);
    commands_["editlevel"] = AdminCommand(EditLevel, CommandFlags::EDIT);
    commands_["edituser"] = AdminCommand(EditUser, CommandFlags::EDIT);
    commands_["finger"] = AdminCommand(Finger, CommandFlags::FINGER);
    commands_["help"] = AdminCommand(Help, CommandFlags::BASIC);
    commands_["kick"] = AdminCommand(Kick, CommandFlags::KICK);
    commands_["levelinfo"] = AdminCommand(LevelInfo, CommandFlags::EDIT);
    commands_["listbans"] = AdminCommand(ListBans, CommandFlags::LISTBANS);
    commands_["listcmds"] = AdminCommand(ListCommands, CommandFlags::BASIC);
    commands_["listflags"] = AdminCommand(ListFlags, CommandFlags::EDIT);
    commands_["listmaps"] = AdminCommand(ListMaps, CommandFlags::BASIC);
    commands_["listplayers"] = AdminCommand(ListPlayers, CommandFlags::LISTPLAYERS);
    commands_["listusers"] = AdminCommand(ListUsers, CommandFlags::EDIT);
    commands_["map"] = AdminCommand(Map, CommandFlags::MAP);
    commands_["mapinfo"] = AdminCommand(MapInfo, CommandFlags::BASIC);
    commands_["mute"] = AdminCommand(Mute, CommandFlags::MUTE);
    commands_["noclip"] = AdminCommand(Noclip, CommandFlags::NOCLIP);
    commands_["nogoto"] = AdminCommand(NoGoto, CommandFlags::NOGOTO);
    commands_["nosave"] = AdminCommand(NoSave, CommandFlags::SAVESYSTEM);
    commands_["passvote"] = AdminCommand(Passvote, CommandFlags::PASSVOTE);
    commands_["putteam"] = AdminCommand(Putteam, CommandFlags::PUTTEAM);
    commands_["readconfig"] = AdminCommand(ReadConfig, CommandFlags::READCONFIG);
    commands_["rmsaves"] = AdminCommand(RemoveSaves, CommandFlags::SAVESYSTEM);
    commands_["rename"] = AdminCommand(Rename, CommandFlags::RENAME);
    commands_["restart"] = AdminCommand(Restart, CommandFlags::RESTART);
    commands_["setlevel"] = AdminCommand(SetLevel, CommandFlags::SETLEVEL);
    commands_["spectate"] = AdminCommand(Spectate, CommandFlags::BASIC);
    commands_["unban"] = AdminCommand(Unban, CommandFlags::BAN);
    commands_["unmute"] = AdminCommand(Unmute, CommandFlags::MUTE);
    commands_["userinfo"] = AdminCommand(UserInfo, CommandFlags::EDIT);

}

CommandInterpreter::~CommandInterpreter()
{

}

CommandInterpreter::AdminCommand::AdminCommand( boost::function<bool(gentity_t *ent, Arguments argv)> handler, char flag )
{
    this->handler = handler;
    this->flag = flag;
}

CommandInterpreter::AdminCommand::AdminCommand()
{

}

void CommandInterpreter::PrintFlags(gentity_t* ent)
{
    std::map<std::string, AdminCommand>::const_iterator it = 
        commands_.begin();

    ChatPrintTo(ent, "^3system: ^7check console for more information.");

    BeginBufferPrint();
    while(it != commands_.end())
    {
        BufferPrint(ent, va("%-20s %c\n",
            it->first.c_str(), it->second.flag));

        it++;
    }
    FinishBufferPrint(ent, false);
}

void CommandInterpreter::PrintCommandList(gentity_t* ent, std::bitset<256> permissions)
{
    std::map<std::string, AdminCommand>::const_iterator it = 
        commands_.begin();

    ChatPrintTo(ent, "^3system: ^7check console for more information.");

    int count = 0;
    const int COMMANDS_PER_LINE = 3;

    BeginBufferPrint();
    while(it != commands_.end())
    {
        if(permissions.at(it->second.flag))
        {
            if(count == COMMANDS_PER_LINE)
            {
                BufferPrint(ent, va("%-30 \n", it->first.c_str()));
                count = 0;
            } else
            {
                BufferPrint(ent, va("%-30s", it->first.c_str()));
                count++;
            }
        }
        it++;
    }
    FinishBufferPrint(ent, true);
}

void PrintManual(gentity_t *ent, const std::string& command);
void CommandInterpreter::PrintHelp(gentity_t* ent, std::string const& command, std::bitset<256> permissions)
{
    std::map<std::string, AdminCommand>::const_iterator lower_bound =
        commands_.lower_bound(command);

    if(lower_bound == commands_.end())
    {
        ChatPrintTo(ent, "^3help: ^7couldn't find command " + command + ".");
        return;
    }

    std::map<std::string, AdminCommand>::const_iterator upper_bound =
        commands_.upper_bound(command);

    lower_bound++;
    if(lower_bound == upper_bound || lower_bound->first == command)
    {
        lower_bound--;

        ChatPrintTo(ent, "^3help: ^7check console for more information.");
        PrintManual(ent, lower_bound->first);
    } else
    {
        ChatPrintTo(ent, "^3help: ^7found multiple matching commands. Check console for more information.");
        BeginBufferPrint();
        while(lower_bound != commands_.end())
        {
            if(lower_bound->first.compare(0, command.length(), command) != 0)
            {
                break;
            }
            BufferPrint(ent, va("^7* %s\n", lower_bound->first.c_str()));
            lower_bound++;
        }
        FinishBufferPrint(ent, true);
    }
}

bool CommandInterpreter::ClientCommand( gentity_t *ent )
{
    std::string command = "";
    std::string arg = SayArgv(0);
    int skip = 0;
    if( arg == "say" || arg == "enc_say" )
    {
        // TODO: team chat commands
        arg = SayArgv(1);
        skip = 1;
    }

    Arguments argv = GetSayArgs(skip);

    // 2nd arg doesn't exist
    if(arg.length() == 0)
    {
        return false;
    }

    if(arg[0] == '!')
    {
        command = &arg[1];
    }
    else if (ent == NULL)
    {
        command = arg;
    }
    else {
        return false;
    }

    boost::to_lower(command);

    std::map< std::string, AdminCommand >::iterator it 
        = commands_.lower_bound(command);

    if(it == commands_.end())
    {
        return false;
    }

    std::bitset<256> permissions = 
        game.session->GetPermissions(ent);

    std::vector<std::map<std::string, AdminCommand>::iterator> foundCommands;

    while(it != commands_.end() && 
        it->first.compare(0, command.length(), command) == 0)
    {
        if(permissions.at(it->second.flag))
        {
            if(it->first == command)
            {
                foundCommands.push_back(it);
                break;
            } else
            {
                foundCommands.push_back(it);
            }
        }
        it++;
    }

    if(foundCommands.size() > 1)
    {
        // !lev
        // Multiple commands found:
        // !levinfo
        // !levadd
        // !levedit
        // etc..
        // 

        ChatPrintTo(ent, "^3server: ^7multiple matching commands found. Check console for more information");
        BeginBufferPrint();
        for(size_t i = 0; i < foundCommands.size(); i++)
        {
            BufferPrint(ent, (boost::format("* %s\n") % foundCommands.at(i)->first).str());
        }
        FinishBufferPrint(ent);
    } else if(foundCommands.size() == 1)
    {
        // Already checked for permission
        foundCommands[0]->second.handler(ent, argv);
    } 
    return qfalse;
}


