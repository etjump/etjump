#include "commandinterpreter.h"
#include "commands.h"
#include "../g_local.hpp"
#include "../g_utilities.hpp"

#include <string>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

CommandInterpreter::CommandInterpreter()
{
    commands_["admintest"] = AdminCommand(Admintest, CommandFlags::BASIC);
    commands_["map"] = AdminCommand(Map, CommandFlags::MAP);
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

    std::vector<std::map<std::string, AdminCommand>::iterator> foundCommands;

    while(it != commands_.end() && 
        it->first.compare(0, command.length(), command) == 0)
    {
        if(it->first == command)
        {
            foundCommands.push_back(it);
            break;
        } else
        {
            foundCommands.push_back(it);
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


