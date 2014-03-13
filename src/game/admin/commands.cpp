#include "commands.h"
#include "common.h"
#include "game.h"
#include "sessiondata.h"
#include "leveldata.h"
#include "../g_save.hpp"
#include "mapdata.h"
#include "commandinterpreter.h"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

extern Game game;

void PrintManual(gentity_t *ent, const std::string& command)
{
    if(ent)
    {
        ChatPrintTo(ent, va("^3%s: ^7check console for more information.", command.c_str()));
        trap_SendServerCommand(ent->client->ps.clientNum, va("manual %s", command.c_str()));
    }
    else
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

bool TargetIsHigherLevel( gentity_t *ent, gentity_t *target, bool equalIsHigher )
{
    if(!ent)
    {
        return false;
    }

    if(equalIsHigher)
    {
        return game.session->GetLevel(target) >= game.session->GetLevel(ent);
    }
    else
    {
        return game.session->GetLevel(target) > game.session->GetLevel(ent);
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

bool Ball8( gentity_t *ent, Arguments argv )
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
    }
    else if(random < NO_IDEA)
    {
        ChatPrintAll("^3Magical 8 Ball: ^3" + Magical8BallResponses[random]);
    }
    else
    {
        ChatPrintAll("^3Magical 8 Ball: ^1" + Magical8BallResponses[random]);
    }

    if(ent)
    {
        ent->client->last8BallTime = level.time;
    }
    return true;
}

bool AddLevel( gentity_t *ent, Arguments argv )
{
    // !addlevel [level] -cmds [commands] -greeting [greeting] -title [title]
    if(argv->size() < 2)
    {
        PrintManual(ent, "addlevel");
        return false;
    }

    int open = 0;

    int level = 0;
    std::string commands;
    std::string greeting;
    std::string title;

    if(!StringToInt(argv->at(1), level))
    {
        ChatPrintTo(ent, va("^3system: ^7%d is not an integer.", level));
        return false;
    }

    if(argv->size() > 2)
    {
        ConstArgIter it = argv->begin() + 2;

        while(it != argv->end())
        {
            if(*it == "-cmds" && it + 1 != argv->end())
            {
                open = CMDS_OPEN;
            }
            else if(*it == "-greeting" && it + 1 != argv->end())
            {
                open = GREETING_OPEN;
            }
            else if(*it == "-title" && it + 1 != argv->end())
            {
                open = TITLE_OPEN;
            }
            else
            {
                switch(open)
                {
                case 0:
                    ChatPrintTo(ent, va("^3addlevel: ^7ignored argument \"%s^7\".", it->c_str()));
                    break;
                case CMDS_OPEN:
                    commands += *it;
                    break;
                case GREETING_OPEN:
                    greeting += *it + " ";
                    break;
                case TITLE_OPEN:
                    title += *it + " ";
                    break;
                default:
                    break;
                }
            }

            it++;
        }

        boost::trim_right(greeting);
        boost::trim_right(title);
    }

    if(!game.levelData->AddLevel(level, title, commands, greeting))
    {
        ChatPrintTo(ent, "^3addlevel: ^7level exists.");
        return false;
    }

    ChatPrintTo(ent, va("^3addlevel: ^7added level %d.", level));

    return true;
}

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

void BanHandleGuidSwitch( gentity_t* ent, Arguments argv )
{
    
}

bool Ban( gentity_t *ent, Arguments argv )
{
    // !ban <name> <time> <reason>
    gentity_t *target = NULL;
    if(argv->size() < 2)
    {
        PrintManual(ent, "ban");
        return false;
    }

    if(argv->at(1) != "-guid")
    {
        // Find the target
        std::string error = "";
        target = PlayerGentityFromString(argv->at(1), error);
        if(!target)
        {
            ChatPrintTo(ent, "^3ban: " + error);
            return false;
        }

        if(ent)
        {
            if(target == ent)
            {
                ChatPrintTo(ent, "^3ban: ^7you can't ban yourself.");
                return false;
            }

            if(TargetIsHigherLevel(ent, target, true))
            {
                ChatPrintTo(ent, "^3ban: ^7can't ban a fellow admin.");
                return false;
            }
        }
        
    } else
    {
        BanHandleGuidSwitch( ent, argv );
        return false;
    }
    
    int multiplier = 1;
    int seconds = PERMANENT_BAN_DURATION;
    std::string reason = "";

    if(argv->size() == 2)
    {
        // Permanent time, no reason
        // -> no need to do anything
    } else
    {
        if(argv->size() >= 3)
        {
            int multiplier = 1;
            int seconds = 0;

            char lastChar = argv->at(2).at(argv->at(2).size() - 1);
            if(lastChar == 'm')
            {
                multiplier = 60;
            } else if(lastChar == 'h')
            {
                multiplier = 60 * 60;
            } else if(lastChar == 'd')
            {
                multiplier = 60 * 60 * 24;
            } else if(lastChar == 'w')
            {
                multiplier = 60 * 60 * 24 * 7;
            }

            if(!StringToInt(argv->at(2).substr(0, argv->at(2).length() - 1), seconds))
            {
                ChatPrintTo(ent, "^3ban: ^7invalid time specified");
                return false;
            }
        }
    
        if(argv->size() >= 4)
        {
            for(ConstArgIter it = argv->begin() + 3;
                it != argv->end(); it++)
            {
                reason += *it + " ";
            }
        }
    }
    
    if(reason.length() == 0)
    {
        reason = "Banned by Administrator.";
    }

    game.session->BanPlayer( ent, target, seconds, reason );

    if(target)
    {
        trap_DropClient(target->client->ps.clientNum, reason.c_str(),
            0);
    } 

    return true;
}

bool Cancelvote( gentity_t *ent, Arguments argv )
{
    if(level.voteInfo.voteTime)
    {
        level.voteInfo.voteYes = 0;
        level.voteInfo.voteNo = level.numConnectedClients;
        ChatPrintAll("^3cancelvote: ^7vote has been canceled");
    }
    else
    {
        ChatPrintTo(ent, "^3cancelvote: ^7no vote in progress.");
    }
    return true;
}

bool DeleteLevel( gentity_t *ent, Arguments argv )
{
    if(argv->size() != 2)
    {
        PrintManual(ent, "deletelevel");
        return false;
    }

    int level = 0;
    if(!StringToInt(argv->at(1), level))
    {
        ChatPrintTo(ent, va("^3deletelevel: ^7%s is not an integer.", argv->at(1).c_str()));
        return false;
    }

    if(!game.levelData->DeleteLevel(level))
    {
        ChatPrintTo(ent, "^3deletelevel: ^7couldn't find level.");
        return false;
    }

    ChatPrintTo(ent, "^3deletelevel: ^7deleted level.");

    return true;
}

bool DeleteUser( gentity_t *ent, Arguments argv )
{
    // TODO: is this really needed?
    ChatPrintTo(ent, "^3deleteuser: ^7command is not implemented.");
    return true;
}

bool EditCommands( gentity_t *ent, Arguments argv )
{
    ChatPrintTo(ent, "^editcommands: ^7command is not implemented.");
    return true;
}

bool EditLevel( gentity_t *ent, Arguments argv )
{
    if(argv->size() < 4)
    {
        PrintManual(ent, "editlevel");
        return false;
    }

    int updated = 0;
    int open = 0;

    int level = 0;
    std::string commands;
    std::string greeting;
    std::string title;

    if(!StringToInt(argv->at(1), level))
    {
        ChatPrintTo(ent, va("^editlevel: ^7%d is not an integer.", level));
        return false;
    }

    if(argv->size() > 2)
    {
        ConstArgIter it = argv->begin() + 2;

        while(it != argv->end())
        {
            if(*it == "-cmds" && it + 1 != argv->end())
            {
                open = CMDS_OPEN;
                updated |= CMDS_OPEN;
            }
            else if(*it == "-greeting" && it + 1 != argv->end())
            {
                open = GREETING_OPEN;
                updated |=GREETING_OPEN;
            }
            else if(*it == "-title" && it + 1 != argv->end())
            {
                open = TITLE_OPEN;
                updated |= TITLE_OPEN;
            }
            else
            {
                switch(open)
                {
                case 0:
                    ChatPrintTo(ent, va("^editlevel: ^7ignored argument \"%s^7\".", it->c_str()));
                    break;
                case CMDS_OPEN:
                    commands += *it;
                    break;
                case GREETING_OPEN:
                    greeting += *it + " ";
                    break;
                case TITLE_OPEN:
                    title += *it + " ";
                    break;
                default:
                    break;
                }
            }

            it++;
        }

        boost::trim_right(greeting);
        boost::trim_right(title);
    }

    game.levelData->EditLevel(level, title, commands, greeting, updated);
    return true;
}

bool EditUser( gentity_t *ent, Arguments argv )
{
    // !edituser guid -cmds personal_commands -title personal_title -greeting pgreeting
    if(argv->size() < 4)
    {
        PrintManual(ent, "edituser");
        return false;
    }

    ConstArgIter it = argv->begin() + 2;

    int updated = 0;
    int open = 0;
    const int CMDS_OPEN = 1;
    const int GREETING_OPEN = 2;
    const int TITLE_OPEN = 4;

    std::string commands;
    std::string greeting;
    std::string title;

    while(it != argv->end())
    {
        if(*it == "-cmds" && it + 1 != argv->end())
        {
            open = CMDS_OPEN;
            updated |= CMDS_OPEN;
        }
        else if(*it == "-greeting" && it + 1 != argv->end())
        {
            open = GREETING_OPEN;
            updated |=GREETING_OPEN;
        }
        else if(*it == "-title" && it + 1 != argv->end())
        {
            open = TITLE_OPEN;
            updated |= TITLE_OPEN;
        }
        else
        {
            switch(open)
            {
            case 0:
                ChatPrintTo(ent, va("^3edituser: ^7ignored argument \"%s^7\".", it->c_str()));
                break;
            case CMDS_OPEN:
                commands += *it;
                break;
            case GREETING_OPEN:
                greeting += *it + " ";
                break;
            case TITLE_OPEN:
                title += *it + " ";
                break;
            default:
                break;
            }
        }

        it++;
    }

    boost::trim_right(greeting);
    boost::trim_right(title);

    game.session->EditUser( ent, argv->at(1), title, commands, greeting, updated );

    return true;
}

bool Finger( gentity_t *ent, Arguments argv )
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


    game.session->PrintFinger(ent, target);
    return true;
}

bool Help( gentity_t *ent, Arguments argv )
{
    if(argv->size() == 1)
    {
        game.command->PrintCommandList(ent, game.session->GetPermissions(ent));
    } else if(argv->size() == 2)
    {
        game.command->PrintHelp(ent, argv->at(1), game.session->GetPermissions(ent));
    } else
    {
        PrintManual(ent, "help");
    }
    return true;
}

bool Kick( gentity_t *ent, Arguments argv )
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

bool LevelInfo( gentity_t *ent, Arguments argv )
{
    if(argv->size() == 1)
    {
        game.levelData->PrintLevels( ent );
    }
    else if(argv->size() > 1)
    {
        int level = 0;
        if(!StringToInt(argv->at(1), level))
        {
            ChatPrintTo(ent, va("^3levelinfo: ^7%s^7 is not a level.", argv->at(1).c_str()));
            return false;
        }

        const ILevelData::LevelInformation *info =
            game.levelData->GetLevelInformation(level);

        if(!info->isValid)
        {
            ChatPrintTo(ent, "^3levelinfo: ^7level does not exist.");
            return false;
        }

        ChatPrintTo(ent, "^3levelinfo: ^7check console for more information.");
        BeginBufferPrint();
        BufferPrint(ent, va("level: %d\n", level));
        BufferPrint(ent, va("name: %s\n", info->name.c_str()));
        BufferPrint(ent, va("commands: %s\n", info->commands.c_str()));
        BufferPrint(ent, va("greeting: %s\n", info->greeting.c_str()));
        FinishBufferPrint(ent, false);
    }
    return true;
}

bool ListBans( gentity_t *ent, Arguments argv )
{
    return true;
}

bool ListCommands( gentity_t *ent, Arguments argv )
{
    std::bitset<SessionData::Client::MAX_COMMANDS> cmds = 
        game.session->GetPermissions(ent);
    game.command->PrintCommandList(ent, cmds);
    return true;
}

bool ListFlags( gentity_t *ent, Arguments argv )
{
    game.command->PrintFlags(ent);
    return true;
}

bool ListMaps( gentity_t *ent, Arguments argv )
{
    game.mapData->ListMaps(ent);
    return true;
}

bool ListPlayers( gentity_t *ent, Arguments argv )
{
    return true;
}

bool ListUsers( gentity_t *ent, Arguments argv )
{
    int page = 1;
    if(argv->size() != 2)
    {
        PrintManual(ent, "listusers");
        return false;
    }

    if(!StringToInt(argv->at(1), page))
    {
        PrintManual(ent, "listusers");
        return false;
    }

    game.session->PrintUserList( ent, page );

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

bool MapInfo(gentity_t* ent, Arguments argv)
{
    std::string name;
    if(argv->size() != 2)
    {
        name = level.rawmapname;
    } else
    {
        name = argv->at(1);
    }

    boost::to_lower(name);

    game.mapData->PrintMapInfo(ent, name);
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

bool Noclip( gentity_t *ent, Arguments argv )
{
    if(level.noNoclip)
    {
        ChatPrintTo(ent, "^3noclip: ^7noclip is disabled on this map.");
        return false;
    }

    if(argv->size() > 2)
    {
        PrintManual(ent, "noclip");
        return false;
    }

    gentity_t *target = NULL;

    if(argv->size() == 1)
    {
        if(!ent)
        {
            return false;
        }
        target = ent;
    }
    else
    {
        std::string err;
        target = PlayerGentityFromString(argv->at(1), err);
        if(!target)
        {
            ChatPrintTo(ent, "^3noclip: ^7" + err);
            return false;
        }
    }

    if(target->client->noclip)
    {
        target->client->noclip = qfalse;
    }
    else
    {
        target->client->noclip = qtrue;
    }

    return true;
}

bool NoCall( gentity_t *ent, Arguments argv )
{
    if(argv->size() != 2)
    {
        PrintManual(ent, "nocall");
        return false;
    }

    std::string err;
    gentity_t *target = NULL;
    target = PlayerGentityFromString(argv->at(1), err);
    if(!target)
    {
        ChatPrintTo(ent, "^nocall: ^7" + err);
        return false;
    }

    if(target->client->sess.noCall)
    {
        target->client->sess.noCall = qfalse;
        ChatPrintTo(ent, va("^3nocall: ^7%s can use /call now.", target->client->pers.netname));
        ChatPrintTo(target, "^3nocall: ^7you can use /call now.");
    } else
    {
        target->client->sess.noCall = qtrue;
        ChatPrintTo(ent, va("^3nocall: ^7%s can no longer use /call.", target->client->pers.netname));
        ChatPrintTo(target, "^3nocall: ^7you can no longer use /call.");
    }

    

    return true;
}

bool NoGoto( gentity_t *ent, Arguments argv )
{
    if(argv->size() != 2)
    {
        PrintManual(ent, "nogoto");
        return false;
    }

    std::string err;
    gentity_t *target = NULL;
    target = PlayerGentityFromString(argv->at(1), err);
    if(!target)
    {
        ChatPrintTo(ent, "^nocall: ^7" + err);
        return false;
    }

    if(target->client->sess.noGoto)
    {
        target->client->sess.noGoto = qfalse;
        ChatPrintTo(ent, va("^3nocall: ^7%s can use /goto now.", target->client->pers.netname));
        ChatPrintTo(target, "^3nocall: ^7you can use /goto now.");
    } else
    {
        target->client->sess.noGoto = qtrue;
        ChatPrintTo(ent, va("^3nocall: ^7%s can no longer use /goto.", target->client->pers.netname));
        ChatPrintTo(target, "^3nocall: ^7you can no longer use /goto.");
    }

    

    return true;
}

bool NoSave( gentity_t *ent, Arguments argv )
{
    if(argv->size() != 2)
    {
        PrintManual(ent, "nosave");
        return false;
    }

    std::string err;
    gentity_t *target = PlayerGentityFromString(argv->at(1), err);
    if(!target)
    {
        ChatPrintTo(ent, "^3nosave: ^7" + err);
        return false;
    }

    if(TargetIsHigherLevel(ent, target, true))
    {
        ChatPrintTo(ent, "^3nosave:^7 can't disable fellow admin's save and load.");
        return false;
    }

    if(target->client->sess.saveAllowed)
    {
        target->client->sess.saveAllowed = qfalse;
        ChatPrintTo(target, va("^3system:^7 %s^7 you are not allowed to save your position.", target->client->pers.netname));
        ChatPrintTo(ent, va("^3system:^7 %s^7 is not allowed to save their position.", target->client->pers.netname));
    }
    else
    {
        target->client->sess.saveAllowed = qtrue;
        ChatPrintTo(target, va("^3system:^7 %s^7 you are now allowed to save your position.", target->client->pers.netname));
        ChatPrintTo(ent, va("^3system:^7 %s^7 is now allowed to save their position.", target->client->pers.netname));
    }

    return true;
}

bool Passvote( gentity_t *ent, Arguments argv )
{
    if(level.voteInfo.voteTime)
    {
        level.voteInfo.voteNo = 0;
        level.voteInfo.voteYes = level.numConnectedClients;
        ChatPrintAll("^3passvote:^7 vote has been passed.");
    }
    else
    {
        ChatPrintAll("^3passvote:^7 no vote in progress.");
    }
    return qtrue;
}

bool Putteam( gentity_t *ent, Arguments argv )
{
    if(argv->size() != 3)
    {
        PrintManual(ent, "putteam");
        return false;
    }

    std::string err;
    gentity_t *target = PlayerGentityFromString(argv->at(1), err);
    if(!target)
    {
        ChatPrintTo(ent, "^3putteam: ^7" + err);
        return false;
    }

    if(TargetIsHigherLevel(ent, target, false))
    {
        ChatPrintTo(ent, "^3putteam: ^7you can't use putteam on a fellow admin.");
        return false;
    }

    target->client->sess.lastTeamSwitch = level.time;

    const weapon_t w = static_cast<weapon_t>(-1);
    SetTeam(target, argv->at(2).c_str(), qfalse, w, w, qtrue);

    return true;
}

bool ReadConfig( gentity_t *ent, Arguments argv )
{
    return true;
}

bool Rename( gentity_t *ent, Arguments argv )
{
    if(argv->size() != 3)
    {
        PrintManual(ent, "rename");
        return false;
    }

    std::string err;
    gentity_t *target = PlayerGentityFromString(argv->at(1), err);
    if(!target)
    {
        ChatPrintTo(ent, "^3rename: ^7" + err);
        return false;
    }

    char userinfo[MAX_INFO_STRING] = "\0";
    int cn = ClientNum(target);
    trap_GetUserinfo(cn, userinfo, sizeof(userinfo));

    const char* oldName = Info_ValueForKey(userinfo, "name");
    ChatPrintAll( (boost::format("^3rename: ^7%s^7 has been renamed to %s") % oldName % argv->at(2) ).str() );
    Info_SetValueForKey(userinfo, "name", argv->at(2).c_str());
    trap_SetUserinfo(cn, userinfo);
    ClientUserinfoChanged(cn);
    trap_SendServerCommand(cn, va("set_name %s", argv->at(2).c_str()));
    return true;
}

bool Restart( gentity_t *ent, Arguments argv )
{
    Svcmd_ResetMatch_f(qfalse, qtrue);
    return true;
}

bool RemoveSaves( gentity_t *ent, Arguments argv )
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

    game.saveData->ResetSavedPositions(target);
    ChatPrintTo(ent, (boost::format("^3system: ^7%s^7's saves were removed.") % target->client->pers.netname).str());
    ChatPrintTo(target, "^3system: ^7your saves were removed");
    return true;
}

bool SetLevel( gentity_t *ent, Arguments argv )
{
    if(argv->size() == 3)
    {
        std::string err;
        gentity_t *target = PlayerGentityFromString(argv->at(1), err);

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
            if(TargetIsHigherLevel(ent, target, false))
            {
                ChatPrintTo(ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
                return false;
            }

            if(level > game.session->GetLevel(ent))
            {
                ChatPrintTo(ent, "^3setlevel: ^7you're not allowed to setlevel higher than your own level.");
                return false;
            }
        }
        if(!game.session->SetLevel(target, level))
        {
            ChatPrintTo(ent, "^3setlevel: ^7level does not exist.");
            return false;
        }
        ChatPrintTo(ent, va("^3setlevel: ^7%s^7 is now a level %d user.", target->client->pers.netname, level));
        ChatPrintTo(target, va("^3setlevel: ^7you are now a level %d user.", level));
    }
    else if(argv->size() == 4)
    {
    }
    else
    {
        PrintManual(ent, "setlevel");
        return false;
    }

    return true;
}

bool Spectate( gentity_t *ent, Arguments argv )
{
    if(!ent)
    {
        return qfalse;
    }

    if(argv->size() != 2)
    {
        if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
        {
            SetTeam( ent, "spectator", qfalse, static_cast<weapon_t>(-1), static_cast<weapon_t>(-1), qfalse );
        }

        return qtrue;
    }

    std::string error;
    gentity_t *target = PlayerGentityFromString(argv->at(1), error);

    if(!target)
    {
        ChatPrintTo(ent, "^spectate: ^7" + error);
        return false;
    }

    if(target->client->sess.sessionTeam == TEAM_SPECTATOR)
    {
        ChatPrintTo(ent, "^3!spectate:^7 you can't spectate a spectator.");
        return qfalse;
    }

    if(!G_AllowFollow(ent, target))
    {
        ChatPrintTo(ent, va("^3!spectate: %s ^7is locked from spectators.", target->client->pers.netname));
        return qfalse;
    }

    if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
    {
        SetTeam( ent, "spectator", qfalse,
                 static_cast<weapon_t>(-1), static_cast<weapon_t>(-1), qfalse );
    }

    ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
    ent->client->sess.spectatorClient = target->client->ps.clientNum;
    return qtrue;
}

bool Unban( gentity_t *ent, Arguments argv )
{
    return true;
}

bool Unmute( gentity_t *ent, Arguments argv )
{
    if(argv->size() != 2)
    {
        PrintManual(ent, "unmute");
        return false;
    }

    std::string error;
    gentity_t *target = PlayerGentityFromString(argv->at(1), error);
    if(!target)
    {
        ChatPrintTo(ent, "^3unmute: ^7" + error);
        return false;
    }


    if(!target->client->sess.muted)
    {
        ChatPrintTo(ent, "^3unmute: ^7target is not muted.");
        return false;
    }

    target->client->sess.muted = qfalse;

    char *ip = NULL;
    char userinfo[MAX_INFO_STRING] = "\0";
    trap_GetUserinfo(target->client->ps.clientNum, userinfo, sizeof(userinfo));
    ip = Info_ValueForKey(userinfo, "ip");

    G_RemoveIPMute(ip);

    CPTo(target, "^5You've been unmuted.");
    ChatPrintAll(target->client->pers.netname + std::string(" ^7has been unmuted."));

    return true;
}

bool UserInfo( gentity_t *ent, Arguments argv )
{
    if(argv->size() != 2)
    {
        PrintManual(ent, "userinfo");
        return false;
    }

    std::string error;
    gentity_t *target = PlayerGentityFromString(argv->at(1), error);
    if(!target)
    {
        ChatPrintTo(ent, "^userinfo: ^7" + error);
        return false;
    }

    game.session->PrintUserinfo( ent, target );

    return true;
}