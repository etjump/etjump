#include "g_local.hpp"
#include "g_utilities.hpp"
#include "g_admin.hpp"
#include "g_save.hpp"
#include "g_sessiondb.hpp"

#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>

static std::map<std::string, AdminCommand_s> adminCommands 
    = boost::assign::map_list_of
    ("8ball", AdminCommand_s(AdminCommand::Magical8Ball, '8', "Magical 8 ball of pure awesomeness gives an answer to any question that you might have!", "!8ball <question>"))
    ("admintest", AdminCommand_s(AdminCommand::Admintest, 'a', "Displays your current admin level.", "!admintest"))
    ("cancelvote", AdminCommand_s(AdminCommand::Cancelvote, 'C',"Cancels current vote in progress.", "!cancelvote"))
    ("edituser", AdminCommand_s(AdminCommand::EditUser, 'A', "Edits user's attributes.", "!edituser meh"))
    ("finger", AdminCommand_s(AdminCommand::Finger, 'f', "Displays target's admin level.", "!finger <target>"))
    ("help", AdminCommand_s(AdminCommand::Help, 'f', "Prints useful information about commands.", "!help <command>"))
    ("kick", AdminCommand_s(AdminCommand::Kick, 'k', "Kicks target player.", "!kick <target>"))
    ("map", AdminCommand_s(AdminCommand::Map, 'M', "Changes map.", "!map <mapname>"))
    ("mute", AdminCommand_s(AdminCommand::Mute, 'm', "Mutes target player.", "!mute <target>"))
    ("passvote", AdminCommand_s(AdminCommand::Passvote, 'P', "Passes the current vote in progress.", "!passvote}"))
    ("rmsaves", AdminCommand_s(AdminCommand::RemoveSaves, 'T',    "Clears target's saved positions.", "!rmsaves <target>"))
    ("setlevel", AdminCommand_s(AdminCommand::Setlevel, 's', "Sets the target's admin level.", "!setlevel <target> <level>\n!setlevel -id <id> <level>"))
    ;

bool TargetIsHigherLevel(gentity_t *ent, gentity_t *target, bool equalIsHigher = false)
{
    if(!ent)
    {
        return false;
    }

    if(equalIsHigher)
    {
        return Session::Level(target) >= Session::Level(ent);
    } else
    {
        return Session::Level(target) > Session::Level(ent);
    }
}

void MutePlayer(gentity_t *target)
{
    target->client->sess.muted = qtrue;

    char userinfo[MAX_INFO_STRING] = "\0";
    char *ip = NULL;

    trap_GetUserinfo(target->client->ps.clientNum, userinfo, sizeof(userinfo));
    ip = Info_ValueForKey(userinfo, "ip");

    G_AddIpMute(ip);
}

void PrintManual(gentity_t *ent, const std::string& command)
{
    if(ent)
    {
        ChatPrintTo(ent, va("^3%s: ^7check console for more information.", command.c_str()));
        trap_SendServerCommand(ent->client->ps.clientNum, va("manual %s", command.c_str()));
    } else
    {
        for(int i = 0; i < sizeof(commandManuals)/sizeof(commandManuals[0]); i++)
        {
            if(!Q_stricmp(commandManuals[i].cmd, command.c_str()))
            {
                G_Printf("%s\n\nUsage:\n%s\n\nDescription:\n%s\n",
                    commandManuals[i].cmd, commandManuals[i].usage, 
                    commandManuals[i].description);
                return;
            }
        }
    }
}

namespace AdminCommand
{
    static const std::string Magical8BallResponses[] = {
        "It is certain",
        "It is decidedly so",
        "Without a doubt",
        "Yes - definitely",
        "You may rely on it",

        "As I see it, yes",
        "Most likely",
        "Outlook good",
        "Signs point to yes",
        "Yes",

        "Reply hazy, try again",
        "Ask again later",
        "Better not tell you now",
        "Cannot predict now",
        "Concentrate and ask again",

        "Don't count on it",
        "My reply is no",
        "My sources say no",
        "Outlook not so good",
        "Very doubtful"
    };

    bool Magical8Ball(gentity_t *ent, Arguments argv)
    {
        const int DELAY_8BALL = 3000; // in milliseconds

        if(ent && ent->client->last8BallTime + DELAY_8BALL > level.time)
        {
            ChatPrintTo(ent, va("^3!8ball: ^7you must wait %i seconds before using !8ball again.", (ent->client->last8BallTime + DELAY_8BALL - level.time)/1000));
            return false;
        }

        if(argv->size() != 2)
        {
            PrintManual(ent, "8ball");
            return false;
        }

        int random = rand() % 20;
        const int POSITIVE = 10;
        const int NO_IDEA = 15;
        if(random < POSITIVE)
        {
            ChatPrintAll("^3Magical 8 Ball: ^2" + Magical8BallResponses[random]);
        } else if(random < NO_IDEA)
        {
            ChatPrintAll("^3Magical 8 Ball: ^3" + Magical8BallResponses[random]);
        } else
        {
            ChatPrintAll("^3Magical 8 Ball: ^1" + Magical8BallResponses[random]);
        }

        if(ent)
        {
            ent->client->last8BallTime = level.time;
        }
        return true;
    }

    bool Admintest(gentity_t *ent, Arguments argv)
    {
        if(!ent)
        {
            ChatPrintAll("^3admintest: ^7console is a level 2147483647 user "
                "(Why?)");
            return true;
        }

        adminDB.PrintAdminTest(ent);
        return true;
    }

    bool Finger(gentity_t *ent, Arguments argv)
    {
        if(argv->size() != 2)
        {
            PrintManual(ent, "finger");
            return false;
        }

        std::string err;
        gentity_t *target = PlayerGentityFromString(argv->at(1), err);
        if(!target)
        {
            ChatPrintTo(ent, "^3finger: ^7" + err);
            return false;
        }

        
        Database::PrintFinger(ent, target);
        return true;
    }

    bool Help(gentity_t *ent, Arguments argv)
    {
        if(argv->size() == 1)
        {
            if(ent)
            {
                ChatPrintTo(ent, "^3help: ^7check console for more information.");
            }
            ConsolePrintTo(ent, "list of commands:");
            
            BeginBufferPrint();
            unsigned commandCount = 0;
            for(CommandList::const_iterator it = adminCommands.begin();
                it != adminCommands.end(); it++)
            {
                if(Session::HasPermission(ent, it->second.flag))
                {
                    if(commandCount != 0 && commandCount % 3 == 0)
                    {
                        BufferPrint(ent, "\n");
                    }
                    BufferPrint(ent, (boost::format("%-21s ") % it->first).str());
                    ++commandCount;
                }
            }
            FinishBufferPrint(ent, true);
        } else
        {
            CommandList::const_iterator command = 
                adminCommands.find(argv->at(1));
            if(command == adminCommands.end())
            {
                ChatPrintTo(ent, "^3help: ^7unknown command.");
                return false;
            }

            if(!Session::HasPermission(ent, command->second.flag))
            {
                ChatPrintTo(ent, "^3help: ^7permission denied.");
                return false;
            }

            if(ent)
            {
                trap_SendServerCommand(ent->client->ps.clientNum, va("manual %s", command->first.c_str()));
            } else
            {
                for(int i = 0; i < sizeof(commandManuals)/sizeof(commandManuals[0]); i++)
                {
                    if(!Q_stricmp(command->first.c_str(), commandManuals[i].cmd))
                    {
                        G_Printf("%s\n\nUsage:\n%s\n\nDescription:\n%s\n\n",
                            command->first.c_str(), commandManuals[i].usage, commandManuals[i].description);
                        break;
                    }
                }
            }
        }
        return true;
    }

    bool Kick(gentity_t *ent, Arguments argv)
    {
        const unsigned MIN_ARGS = 2;
        if(argv->size() < MIN_ARGS)
        {
            PrintManual(ent, "kick");
            return false;
        }

        std::string error = "";
        gentity_t *target = PlayerGentityFromString(argv->at(1), error);
        if(!target)
        {
            ChatPrintTo(ent, "^3kick: " + error);
            return false;
        }

        if(ent)
        {
            if(ent == target)
            {
                ChatPrintTo(ent, "^3kick: ^7you can't kick yourself.");
                return false;
            }

            if(TargetIsHigherLevel(ent, target, true))
            {
                ChatPrintTo(ent, "^3kick: ^7you can't kick a fellow admin.");
                return false;
            }
        }

        int timeout = 0;
        if(argv->size() >= 3)
        {
            if(!StringToInt(argv->at(2), timeout))
            {
                ChatPrintTo(ent, "^3kick: ^7invalid timeout \"" + argv->at(2) + "\" specified.");
                return false;
            }
        }

        std::string reason;
        if(argv->size() >= 4)
        {
            reason = argv->at(3);
        }

        trap_DropClient(target->client->ps.clientNum, reason.c_str(), timeout);
        return true;
    }

    bool Cancelvote(gentity_t *ent, Arguments argv)
    {
        if(level.voteInfo.voteTime)
        {
            level.voteInfo.voteYes = 0;
            level.voteInfo.voteNo = level.numConnectedClients;
            ChatPrintAll("^3cancelvote: ^7vote has been canceled");
        } else
        {
            ChatPrintTo(ent, "^3cancelvote: ^7no vote in progress.");
        }
        return true;
    }

    bool Passvote(gentity_t *ent, Arguments argv) 
    {
        if(level.voteInfo.voteTime) {
            level.voteInfo.voteNo = 0;
            level.voteInfo.voteYes = level.numConnectedClients;
            ChatPrintAll("^3passvote:^7 vote has been passed.");
        }
        else {
            ChatPrintAll("^3passvote:^7 no vote in progress.");
        }
        return qtrue;
    }

    bool RemoveSaves(gentity_t *ent, Arguments argv)
    {
        if(argv->size() != 2)
        {
            PrintManual(ent, "rmsaves");
            return false;
        }

        std::string error;
        gentity_t *target = PlayerGentityFromString(argv->at(1), error);
        if(!target)
        {
            ChatPrintTo(ent, "^3rmsaves: ^7" + error);
            return false;
        }

        if(!TargetIsHigherLevel(ent, target, true))
        {
            ChatPrintTo(ent, "^3rmsaves: ^7can't remove fellow admin's saves.");
            return false;
        }

        SaveSystem::ResetSavedPositions(target);
        ChatPrintTo(ent, (boost::format("^3system: ^7%s^7's saves were removed.") % target->client->pers.netname).str());
        ChatPrintTo(target, "^3system: ^7your saves were removed");
        return true;
    }

    bool Setlevel(gentity_t *ent, Arguments argv)
    {
        // !setlevel player level
        // !setlevel -id id level
        if(argv->size() < 3)
        {
            PrintManual(ent, "setlevel");
            return false;
        } else if(argv->size() == 3)
        {
            // Normal setlevel
            std::string err;
            gentity_t *target = PlayerGentityFromString(argv->at(1), 
                err);

            if(!target)
            {
                ChatPrintTo(ent, err);
                return false;
            }

            int level = 0;
            if(!StringToInt(argv->at(2), level))
            {
                ChatPrintTo(ent, "^3setlevel: ^7invalid level " + argv->at(2));
                return false;
            }

            if(ent)
            {
                if(TargetIsHigherLevel(ent, target)) 
                {
                    ChatPrintTo(ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
                    return false;
                }

                if(level > Session::Level(ent))
                {
                    ChatPrintTo(ent, "^3setlevel: ^7you're not allowed to setlevel higher than your own level.");
                    return false;
                }
            }            

            // AdminDB does the level exists check
            return adminDB.SetLevel(ent, target, level);
        } else if(argv->size() == 4)
        {
            // Set by ID
            // !setlevel -id 0 1
            if(argv->at(1) != "-id")
            {
                ChatPrintTo(ent, "^3usage: ^7!setlevel -id <id> <level>");
                return false;
            }

            int id = 0;
            if(!StringToInt(argv->at(2), id))
            {
                ChatPrintTo(ent, "^3setlevel: ^7invalid id " + argv->at(2));
                return false;
            }

            int level = 0;
            if(!StringToInt(argv->at(3), level))
            {
                ChatPrintTo(ent, "^3setlevel: ^7invalid level " + argv->at(3));
                return false;
            }

            if(level > Session::Level(ent))
            {
                ChatPrintTo(ent, "^3setlevel: ^7you're not allowed to setlevel higher than your own level.");
                return false;
            }

            return adminDB.IDSetLevel(ent, id, level);

        } else {
            ChatPrintTo(ent, "^3usage: ^7!setlevel <player> <level>");
            ChatPrintTo(ent, "^3usage: ^7!setlevel -id <id> <level>");
        }

        return true;
    }

    bool EditUser(gentity_t *ent, Arguments argv)
    {
        // !edituser -id <id> -cmds <personal commands> -title <personal title> -greeting <personal greeting>
        // !edituser -guid <guid> ...rest is the same...
        
        if(argv->size() < 5)
        {
            PrintManual(ent, "edituser");
            return false;
        }

        enum Mode {
            NONE,
            ID,
            GUID
        };

        int mode = NONE;
        int id = 0;
        std::string guid;
        if((*argv)[1] == "-id")
        {
            if(!StringToInt((*argv)[2], id))
            {
                ChatPrintTo(ent, "^3edituser: ^7invalid id \"" + (*argv)[2] + "\" specified.");
                return false;
            }
            mode = ID;
        } else if((*argv)[1] == "-guid")
        {
            mode = GUID;
            guid = (*argv)[2];
        } else
        {
            PrintManual(ent, "edituser");
            return false;
        }

        ConstArgIter it = argv->begin() + 3;

        const int STATE_NONE = 0;
        const int STATE_COMMANDS = 1;
        const int STATE_TITLE = 2;
        const int STATE_GREETING = 4;
        const int STATE_LEVEL = 8;

        int parsing = STATE_NONE;
        int updated = STATE_NONE;
        int level = 0;
        std::string commands;
        std::string title;
        std::string greeting;

        while(it != argv->end())
        {
            if(*it == "-cmds" && it+1 != argv->end())
            {
                parsing = STATE_COMMANDS;
                it++;
                continue;
            } else if(*it == "-title" && it+1 != argv->end())
            {
                parsing = STATE_TITLE;
                it++;
                continue;
            } else if(*it == "-greeting" && it+1 != argv->end())
            {
                parsing = STATE_GREETING;
                it++;
                continue;
            } else if(*it == "-level" && it+1 != argv->end())
            {
                parsing = STATE_LEVEL;
                it++;
                continue;
            }
            
            // For commands spaces are useless
            if(parsing == STATE_COMMANDS)
            {
                updated |= STATE_COMMANDS;
                commands += *it;
            } 
            // For title & greeting spaces are there for a reason
            else if(parsing == STATE_TITLE)
            {
                updated |= STATE_TITLE;
                title += *it + " ";
            } else if(parsing == STATE_GREETING)
            {
                updated |= STATE_GREETING;
                greeting += *it + " ";
            } else if(parsing == STATE_LEVEL)
            {
                updated |= STATE_LEVEL;
                if(!StringToInt(*it, level))
                {
                    ChatPrintTo(ent, "^3edituser: ^7invalid level \"" + *it + "\" specified.");
                    return false;
                }
            }
         
            it++;
        }

        boost::trim_right(greeting);
        boost::trim_right(title);

        if(mode == ID)
        {
            adminDB.UpdateUserByID(ent, id, updated, level, commands, greeting, title);
        } else 
        {
            adminDB.UpdateUserByGUID(ent, guid, updated, level, commands, greeting, title);
        }
        
        return true;
    }

    bool Map( gentity_t *ent, Arguments argv )
    {
        if(argv->size() != 2)
        {
            PrintManual(ent, "map");
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

    bool Mute( gentity_t *ent, Arguments argv )
    {
        if(argv->size() != 2)
        {
            PrintManual(ent, "mute");
            return false;
        }

        std::string errorMsg;

        gentity_t *target = PlayerGentityFromString(argv->at(1), errorMsg);
        if(!target)
        {
            ChatPrintTo(ent, "^3!mute: ^7" + errorMsg);
            return false;
        }

        if(ent)
        {
            if(ent == target)
            {
                ChatPrintTo(ent, "^3mute: ^7you cannot mute yourself.");
                return false;
            }

            if(TargetIsHigherLevel(ent, target, true))
            {
                ChatPrintTo(ent, "^3mute: ^7you cannot mute a fellow admin.");
                return false;
            }
        }

        if(target->client->sess.muted == qtrue)
        {
            ChatPrintTo(ent, "^3mute: " + std::string(target->client->pers.netname) + " ^7is already muted.");
            return false;
        }

        MutePlayer(target);
        CPTo(target, "^5You've been muted");
        ChatPrintTo(ent, std::string(target->client->pers.netname) + " ^7has been muted.");
        return true;
    }
}

qboolean CheckCommand( gentity_t *ent )
{
    // say "!command additional args"
    using boost::format;
    if(g_admin.integer == 0)
    {
        return qfalse;
    }

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
        return qfalse;
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
        return qfalse;
    }

    boost::to_lower(command);

    std::map<std::string, AdminCommand_s>::iterator it
        = adminCommands.lower_bound(command);

    if(it == adminCommands.end())
    {
        return qfalse;
    }

    std::vector<std::map<std::string, AdminCommand_s>::iterator> foundCommands;

    while(it != adminCommands.end() && 
        it->first.compare(0, command.length(), command) == 0)
    {
        if(it->first == command)
        {
            if(Session::HasPermission(ent, it->second.flag))
            {
                foundCommands.push_back(it);
                break;
            } else
            {
                ChatPrintTo(ent, (format("^3%s: ^7permission denied.") % it->first ).str());
                return qfalse;
            }
        }
        if(Session::HasPermission(ent, it->second.flag)) 
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
            BufferPrint(ent, (format("* %s\n") % foundCommands.at(i)->first).str());
        }
        FinishBufferPrint(ent);
    } else if(foundCommands.size() == 1)
    {
        // Already checked for permission
        foundCommands[0]->second.handler(ent, argv);
    } 
    return qfalse;
}

Admin::UserData_s::UserData_s()
{
    this->id = -1;
    this->level = 0;
}

AdminCommand_s::AdminCommand_s( boost::function<bool(gentity_t *ent, Arguments argv)> handler, char flag, const std::string& function, const std::string& syntax )
{
    this->handler = handler;
    this->flag = flag;
    this->function = function;
    this->syntax = syntax;
}
