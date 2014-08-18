#include <bitset>
#include <boost/algorithm/string.hpp>
#include "commands.hpp"
#include "../g_local.hpp"
#include "../g_save.hpp"
#include "session.hpp"

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

    bool PrintIdent(gentity_t *ent, Arguments argv)
    {
        ChatPrintTo(ent, va("Map progression: %d", ent->client->sess.clientMapProgression));
        ChatPrintTo(ent, va("Upcoming progression: %d", ent->client->sess.upcomingClientMapProgression));
        ChatPrintTo(ent, va("Previous progression: %d", ent->client->sess.previousClientMapProgression));
        return qtrue;
    }
}

//namespace AdminCommands
//{
//    struct AdminCommand 
//    {
//    public:
//        AdminCommand(boost::function < bool(gentity_t *ent, Arguments argv)> command, char flag)
//            :func_(command), flag_(flag)
//        {
//            
//        }
//        boost::function<bool(gentity_t *ent, Arguments argv)> func_;
//        char flag_;
//    };
//
//    namespace Flags
//    {
//        const char BASIC = 'a';
//        const char CANCELVOTE = 'C';
//        const char EBALL = '8';
//        const char EDIT = 'A';
//        const char FINGER = 'f';
//        const char HELP = 'h';
//        const char KICK = 'k';
//        const char LISTBANS = 'L';
//        const char LISTPLAYERS = 'l';
//        const char MAP = 'M';
//        const char MUTE = 'm';
//        const char NOCLIP = 'N';
//        const char NOGOTO = 'K';
//        const char PASSVOTE = 'P';
//        const char PUTTEAM = 'p';
//        const char READCONFIG = 'G';
//        const char RENAME = 'R';
//        const char RESTART = 'r';
//        const char SAVESYSTEM = 'T';
//        const char SETLEVEL = 's';
//    }
//
//    bool AddLevel(gentity_t *ent, Arguments argv)
//    {
//        int level = 0;
//        std::string name;
//        std::string commands;
//        std::string greeting;
//
//        if (argv->size() == 2)
//        {
//
//        }
//        return true;
//    }
//}

Commands::Commands()
{
    //using AdminCommands::AdminCommand;
    //adminCommands_["addlevel"] = AdminCommand(AdminCommands::AddLevel, 'a');

    commands_["backup"] = ClientCommands::BackupLoad;
    commands_["save"] = ClientCommands::Save;
    commands_["load"] = ClientCommands::Load;
    commands_["progression"] = ClientCommands::PrintIdent;
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

//bool Commands::AdminCommand(gentity_t* ent)
//{
//    std::string command = "",
//                arg = SayArgv(0);
//    int skip = 0;
//
//    if (arg == "say" || arg == "enc_say")
//    {
//        arg = SayArgv(1);
//        skip = 1;
//    }
//
//    Arguments argv = GetSayArgs(skip);
//
//    if (arg.length() == 0)
//    {
//        return false;
//    }
//
//    if (arg[0] == '!')
//    {
//        command = &arg[1];
//    }
//    else if (ent == NULL)
//    {
//        command = arg;
//    }
//    else
//    {
//        return false;
//    }
//
//    boost::to_lower(command);
//
//    ConstCommandIterator it = adminCommands_.lower_bound(command);
//
//    if (it == adminCommands_.end())
//    {
//        return false;
//    }
//
//    std::bitset<256> permissions =
//        game.session->Permissions(ent);
//
//    std::vector<ConstCommandIterator> foundCommands;
//
//    while (it != commands_.end() &&
//        it->first.compare(0, command.length(), command) == 0) {
//        if (permissions[it->second.flag])
//        {
//            if (it->first == command)
//            {
//                foundCommands.push_back(it);
//                break;
//            }
//        }
//        else
//        {
//            
//        }
//    }
//
//    return false;
////}