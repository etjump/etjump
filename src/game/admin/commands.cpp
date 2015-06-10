#include <bitset>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include "commands.hpp"
#include "../g_local.hpp"
#include "../g_save.hpp"
#include "session.hpp"
#include "../mapdata.h"
#include "../races.hpp"
#include "../custommapvotes.hpp"
#include "../timerun.h"
#include "../g_local.h"

typedef boost::function<bool(gentity_t *ent, Arguments argv)> Command;
typedef std::pair<boost::function<bool(gentity_t *ent, Arguments argv)>, char> AdminCommandPair;
typedef std::map< std::string, boost::function<bool(gentity_t *ent, Arguments argv)> >::const_iterator ConstCommandIterator;
typedef std::map< std::string, std::pair<boost::function<bool(gentity_t *ent, Arguments argv)>, char> >::const_iterator ConstAdminCommandIterator;
typedef std::map< std::string, boost::function<bool(gentity_t *ent, Arguments argv)> >::iterator CommandIterator;
typedef std::map< std::string, std::pair<boost::function<bool(gentity_t *ent, Arguments argv)>, char> >::iterator AdminCommandIterator;

namespace CommandFlags
{
    const char BAN = 'b';
    // For everyone
    const char BASIC = 'a';
    const char CANCELVOTE = 'C';
    const char EBALL = '8';
    const char EDIT = 'A';
    const char FINGER = 'f';
    const char HELP = 'h';
    const char KICK = 'k';
    const char LISTBANS = 'L';
    const char LISTPLAYERS = 'l';
    const char MAP = 'M';
    const char MUTE = 'm';
    const char NOCLIP = 'N';
    const char NOGOTO = 'K';
    const char PASSVOTE = 'P';
    const char PUTTEAM = 'p';
    const char READCONFIG = 'G';
    const char RENAME = 'R';
    const char RESTART = 'r';
    const char ROUTE_MAKER = 'V';
    const char SAVESYSTEM = 'T';
    const char SETLEVEL = 's';
}

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

    bool Race(gentity_t *ent, Arguments argv)
    {
        if (!ent->client->pers.race.isRouteMaker)
        {
            ChatPrintTo(ent, "^3race: ^7you must be a route maker to make routes.");
            return false;
        }

        if (argv->size() == 1)
        {
            ChatPrintTo(ent, "^3usage: ^7/race");
            return false;
        }

        // Adds a start point
        if (argv->at(1) == "start")
        {
            if (argv->size() == 3 && argv->at(2) == "-keep")
            {
                if (!game.races->CreateStart(ent->r.currentOrigin, ent->client->ps.viewangles, true))
                {
                    // Shouldn't happen.
                    ChatPrintTo(ent, "^3race: ^7failed to create a start point.");
                    return false;
                }
            }
            else
            {
                if (!game.races->CreateStart(ent->r.currentOrigin, ent->client->ps.viewangles, false))
                {
                    // Shouldn't happen.
                    ChatPrintTo(ent, "^3race: ^7failed to create a start point.");
                    return false;
                }
                CPTo(ent, "^7Race ^2start^7 created.");
                ConsolePrintTo(ent, "^7Cleared all checkpoints and end point. If you wish to keep them, use the -keep switch.");
            }

            ConsolePrintTo(ent, va("Setting race start point to (%f %f %f) angle: (%f %f %f)",
                ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
                ent->client->ps.viewangles[0], ent->client->ps.viewangles[1], ent->client->ps.viewangles[2]));
        }
        // Adds a new end point
        // /race end x y z
        // /race end xyzwidth
        // /race end xywidth zwidth
        // /race end xwidth ywidth zwidth
        else if (argv->at(1) == "end")
        {
            if (argv->size() < 3)
            {
                ChatPrintTo(ent, "^3usage: ^7check console for more information.");
                BeginBufferPrint();
                BufferPrint(ent, "/race end [xyz width]\n");
                BufferPrint(ent, "/race end [xy width] [z width]\n");
                BufferPrint(ent, "/race end [x width] [y width] [z width]\n");
                FinishBufferPrint(ent, false);
                return false;
            }

            vec3_t dimensions = { 0, 0, 0 };
            if (argv->size() == 3)
            {
                float width = 0;
                if (!ToFloat(argv->at(2), width) || width < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined width is not a positive number.");
                    return false;
                }
                
                for (int i = 0; i < 3; i++)
                {
                    dimensions[i] = width;
                }
            }
            else if (argv->size() == 4)
            {
                float xyWidth = 0;
                float zWidth = 0;
                if (!ToFloat(argv->at(2), xyWidth) || xyWidth < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined xy width is not a positive number.");
                    return false;
                }

                if (!ToFloat(argv->at(3), zWidth) || zWidth < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined z width is not a positive number.");
                    return false;
                }

                for (int i = 0; i < 2; i++)
                {
                    dimensions[i] = xyWidth;
                }
                dimensions[2] = zWidth;
            }
            else if (argv->size() == 5)
            {
                float xWidth = 0;
                float yWidth = 0;
                float zWidth = 0;
                if (!ToFloat(argv->at(2), xWidth) || xWidth < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined x width is not a positive number.");
                    return false;
                }

                if (!ToFloat(argv->at(3), yWidth) || yWidth < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined y width is not a positive number.");
                    return false;
                }

                if (!ToFloat(argv->at(4), zWidth) || zWidth < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined z width is not a positive number.");
                    return false;
                }
                dimensions[0] = xWidth;
                dimensions[1] = yWidth;
                dimensions[2] = zWidth;
            }

            // Divide each by two as theyll be doubled later due to
            // mins/maxs
            for (int i = 0; i < 3; i++)
            {
                dimensions[i] /= 2;
            }

            game.races->CreateEnd(ent->r.currentOrigin, ent->client->ps.viewangles, dimensions);
            CPTo(ent, "^7Race ^1end^7 created.");
            ConsolePrintTo(ent, va("^7Setting race end point to (%f %f %f) angle: (%f %f %f) dimensions: 2x(%f %f %f)",
                ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
                ent->client->ps.viewangles[0], ent->client->ps.viewangles[1], ent->client->ps.viewangles[2],
                dimensions[0], dimensions[1], dimensions[2]));
        }
        // Adds a new checkpoint
        // /race checkpoint x y z
        // /race checkpoint xyzwidth
        // /race checkpoint xywidth zwidth
        // /race checkpoint xwidth ywidth zwidth
        else if (argv->at(1) == "checkpoint")
        {
            if (argv->size() < 3)
            {
                ChatPrintTo(ent, "^3usage: ^7check console for more information.");
                BeginBufferPrint();
                BufferPrint(ent, "/race checkpoint [xyz width]\n");
                BufferPrint(ent, "/race checkpoint [xy width] [z width]\n");
                BufferPrint(ent, "/race checkpoint [x width] [y width] [z width]\n");
                FinishBufferPrint(ent, false);
                return false;
            }

            vec3_t dimensions = { 0, 0, 0 };
            if (argv->size() == 3)
            {
                float width = 0;
                if (!ToFloat(argv->at(2), width) || width < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined width is not a positive number.");
                    return false;
                }

                for (int i = 0; i < 3; i++)
                {
                    dimensions[i] = width;
                }
            }
            else if (argv->size() == 4)
            {
                float xyWidth = 0;
                float zWidth = 0;
                if (!ToFloat(argv->at(2), xyWidth) || xyWidth < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined xy width is not a positive number.");
                    return false;
                }

                if (!ToFloat(argv->at(3), zWidth) || zWidth < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined z width is not a positive number.");
                    return false;
                }

                for (int i = 0; i < 2; i++)
                {
                    dimensions[i] = xyWidth;
                }
                dimensions[2] = zWidth;
            }
            else if (argv->size() == 5)
            {
                float xWidth = 0;
                float yWidth = 0;
                float zWidth = 0;
                if (!ToFloat(argv->at(2), xWidth) || xWidth < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined x width is not a positive number.");
                    return false;
                }

                if (!ToFloat(argv->at(3), yWidth) || yWidth < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined y width is not a positive number.");
                    return false;
                }

                if (!ToFloat(argv->at(4), zWidth) || zWidth < 0)
                {
                    ChatPrintTo(ent, "^3race: ^7defined z width is not a positive number.");
                    return false;
                }
                dimensions[0] = xWidth;
                dimensions[1] = yWidth;
                dimensions[2] = zWidth;
            }

            // Divide each by two as theyll be doubled later due to
            // mins/maxs
            for (int i = 0; i < 3; i++)
            {
                dimensions[i] /= 2;
            }

            game.races->CreateCheckpoint(ent->r.currentOrigin, ent->client->ps.viewangles, dimensions);
            CPTo(ent, "^7Race ^3checkpoint^7 created.");
            ConsolePrintTo(ent, va("^7Adding a race checkpoint to (%f %f %f) angle: (%f %f %f) dimensions: 2x(%f %f %f)",
                ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
                ent->client->ps.viewangles[0], ent->client->ps.viewangles[1], ent->client->ps.viewangles[2],
                dimensions[0], dimensions[1], dimensions[2]));
        }
        // Clears all checkpoints and start + end
        else if (argv->at(1) == "clear")
        {
            game.races->ClearRoute();
            ChatPrintTo(ent, "^3race: ^7cleared current route.");
        }
        // Undoes the last checkpoint add
        else if (argv->at(1) == "undo")
        {
            if (!game.races->UndoLastCheckpoint())
            {
                ChatPrintTo(ent, "^3race: ^7" + game.races->GetMessage());
                return false;
            }
            ChatPrintTo(ent, "^3race: ^7removed last checkpoint.");
        }
        else if (argv->at(1) == "settings")
        {
            // race settings setting value
            if (argv->size() < 4)
            {
                return false;
            }
        }
        else if (argv->at(1) == "save")
        {
            if (argv->size() < 3)
            {
                ChatPrintTo(ent, "^3race: ^7/race save name");
                return false;
            }
            game.races->Save(ConcatArgs(2), ent);
        }
        else if (argv->at(1) == "load")
        {
            if (argv->size() < 3)
            {
                ChatPrintTo(ent, "^3race: ^7/race load name");
                return false;
            }
            game.races->Load(ConcatArgs(2), ent);
        }

        return true;
    }

    bool ListInfo(gentity_t *ent, Arguments argv)
    {
        if (argv->size() != 2)
        {
            std::string types = game.customMapVotes->ListTypes();
            ConsolePrintTo(ent, "^<List of custom map types: ^7" + types);
            return true;
        }

        const std::vector<std::string> *lines = game.customMapVotes->ListInfo(argv->at(1));
        if (lines->size() == 0)
        {
            ChatPrintTo(ent, "^<<listinfo: ^7list could not be found.");
            return false;
        }
        BeginBufferPrint();
        for (unsigned i = 0; i < lines->size(); i++)
        {
            BufferPrint(ent, lines->at(i));
        }
        FinishBufferPrint(ent, false);
        return true;
    }

    bool Records(gentity_t *ent, Arguments argv)
    {
        std::string map = level.rawmapname, runName;

        if (argv->size() > 1) {
            runName = argv->at(1);
        }

        if (argv->size() > 2) {
            map = argv->at(2);
        }

        game.timerun->printRecords(ClientNum(ent), map, runName);
        return true;
    }
}

void PrintManual(gentity_t *ent, const std::string& command)
{
    if (ent)
    {
        ChatPrintTo(ent, va("^3%s: ^7check console for more information.", command.c_str()));
        trap_SendServerCommand(ent->client->ps.clientNum, va("manual %s", command.c_str()));
    }
    else
    {
        int i = 0;
        int len = sizeof(commandManuals) / sizeof(commandManuals[0]);
        for (i = 0; i < len; i++)
        {
            if (!Q_stricmp(commandManuals[i].cmd, command.c_str()))
            {
                G_Printf("%s\n\nUsage:\n%s\n\nDescription:\n%s\n",
                    commandManuals[i].cmd, commandManuals[i].usage,
                    commandManuals[i].description);
                return;
            }
        }
        if (i == len)
        {
            G_Printf("Couldn't find manual for command \"%s\"\n", command.c_str());
        }
    }
}

bool IsTargetHigherLevel(gentity_t *ent, gentity_t *target, bool equalIsHigher)
{
    if (!ent)
    {
        return false;
    }

    if (equalIsHigher)
    {
        return game.session->GetLevel(ent) <= game.session->GetLevel(target);
    }

    return game.session->GetLevel(ent) < game.session->GetLevel(target);
}

bool IsTargetHigherLevel(gentity_t *ent, unsigned id, bool equalIsHigher)
{
    if (equalIsHigher)
    {
        return game.session->GetLevel(ent) <= game.session->GetLevelById(id);
    }

    return game.session->GetLevel(ent) < game.session->GetLevelById(id);
}

namespace AdminCommands
{
    const int CMDS_OPEN = 1;
    const int GREETING_OPEN = 2;
    const int TITLE_OPEN = 4;

    namespace Updated
    {
        const unsigned NONE = 0;
        const unsigned LEVEL = 0x00001;
        const unsigned LAST_SEEN = 0x00002;
        const unsigned NAME = 0x00004;
        const unsigned TITLE = 0x00008;
        const unsigned COMMANDS = 0x00010;
        const unsigned GREETING = 0x00020;
    }

    bool Admintest(gentity_t *ent, Arguments argv)
    {
        if (!ent)
        {
            ChatPrintAll("^3admintest: ^7console is a level ? user.");
            return true;
        }

        game.session->PrintAdmintest(ent);
        return true;
    }

    bool AddLevel(gentity_t* ent, Arguments argv)
    {
        // !addlevel [level] -cmds [commands] -greeting [greeting] -title [title]
        if (argv->size() < 2)
        {
            PrintManual(ent, "addlevel");
            return false;
        }
        int open = 0;

        int level = 0;
        std::string commands;
        std::string greeting;
        std::string title;

        if (!ToInt(argv->at(1), level))
        {
            ChatPrintTo(ent, va("^3system: ^7%d is not an integer.", level));
            return false;
        }

        if (argv->size() > 2)
        {
            ConstArgIter it = argv->begin() + 2;

            while (it != argv->end())
            {
                if (*it == "-cmds" && it + 1 != argv->end())
                {
                    open = CMDS_OPEN;
                }
                else if (*it == "-greeting" && it + 1 != argv->end())
                {
                    open = GREETING_OPEN;
                }
                else if (*it == "-title" && it + 1 != argv->end())
                {
                    open = TITLE_OPEN;
                }
                else
                {
                    switch (open)
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

        if (!game.levels->Add(level, title, commands, greeting))
        {
            ChatPrintTo(ent, "^3addlevel: ^7" + game.levels->ErrorMessage());
            return false;
        }

        ChatPrintTo(ent, va("^3addlevel: ^7added level %d.", level));

        return true;
    }

    bool Ball8(gentity_t* ent, Arguments argv)
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
        const int DELAY_8BALL = 3000; // in milliseconds

        if (ent && ent->client->last8BallTime + DELAY_8BALL > level.time)
        {
            ChatPrintTo(ent, va("^3!8ball: ^7you must wait %i seconds before using !8ball again.", (ent->client->last8BallTime + DELAY_8BALL - level.time) / 1000));
            return false;
        }

        if (argv->size() == 1)
        {
            PrintManual(ent, "8ball");
            return false;
        }

        int random = rand() % 20;
        const int POSITIVE = 10;
        const int NO_IDEA = 15;
        if (random < POSITIVE)
        {
            ChatPrintAll("^3Magical 8 Ball: ^2" + Magical8BallResponses[random]);
        }
        else if (random < NO_IDEA)
        {
            ChatPrintAll("^3Magical 8 Ball: ^3" + Magical8BallResponses[random]);
        }
        else
        {
            ChatPrintAll("^3Magical 8 Ball: ^1" + Magical8BallResponses[random]);
        }

        if (ent)
        {
            ent->client->last8BallTime = level.time;
        }
        return true;
    }

    bool Ban(gentity_t* ent, Arguments argv)
    {
        if (argv->size() == 1)
        {
            PrintManual(ent, "ban");
            return false;
        }
        // ban permanently
        time_t t;
        time(&t);
        std::string err;
        gentity_t *player = PlayerGentityFromString(argv->at(1).c_str(), err);

        if (!player)
        {
            ChatPrintTo(ent, "^3ban: ^7no player with name " + argv->at(1));
            return false;
        }

        if (IsTargetHigherLevel(ent, player, true))
        {
            ChatPrintTo(ent, "^3ban: ^7you cannot ban a fellow admin.");
            return false;
        }

        unsigned expires = 0;
        std::string reason = "Banned by admin.";

        // !ban <name> <time> <reason>
        if (argv->size() > 3)
        {
            if (!ToUnsigned(argv->at(2), expires))
            {
                // TODO: 10m, 2h etc.
                ChatPrintTo(ent, "^3ban: ^7time was not a number.");
                return false;
            }

            expires = static_cast<unsigned>(t)+expires;
        }
        
        if (argv->size() >= 4)
        {
            reason = "";
            for (size_t i = 3; i < argv->size(); i++)
            {
                if (i + 1 == argv->size())
                {
                    reason += argv->at(i);
                }
                else
                {
                    reason += argv->at(i) + " ";

                }

            }
        }
        if (!game.session->Ban(ent, player, expires, reason))
        {
            ChatPrintTo(ent, "^3ban: ^7" + game.session->GetMessage());
            return false;
        }

        trap_DropClient(ClientNum(player), "You are banned", 0);
        return true;
    }

    bool Cancelvote(gentity_t* ent, Arguments argv)
    {
        if (level.voteInfo.voteTime)
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

    bool DeleteLevel(gentity_t* ent, Arguments argv)
    {
        if (argv->size() != 2)
        {
            PrintManual(ent, "deletelevel");
            return false;
        }

        int level = 0;
        if (!ToInt(argv->at(1), level))
        {
            ChatPrintTo(ent, va("^3deletelevel: ^7%s is not an integer.", argv->at(1).c_str()));
            return false;
        }

        if (!game.levels->Delete(level))
        {
            ChatPrintTo(ent, "^3deletelevel: ^7couldn't find level.");
            return false;
        }

        int usersWithLevel = game.session->LevelDeleted(level);
        
        ChatPrintTo(ent, "^3deletelevel: ^7deleted level. Set " + ToString(usersWithLevel) + " users to level 0.");

        return true;
    }

    bool EditCommands(gentity_t* ent, Arguments argv)
    {
        // !editcommands level +command|-command +command|-command etc.
        if (argv->size() < 3)
        {
            PrintManual(ent, "editcommands");
            return false;
        }

        int level = 0;
        if (!ToInt(argv->at(1), level))
        {
            ChatPrintTo(ent, "^3editcommands: ^7defined level \"" + (*argv)[1] + "\" is not an integer.");
            return false;
        }

        if (!game.levels->LevelExists(level))
        {
            ChatPrintTo(ent, "^3editcommands: ^7level " + (*argv)[1] + " does not exist.");
            return false;
        }

        ConstArgIter it = argv->begin() + 2;
        ConstArgIter end = argv->end();

        std::string currentPermissions = game.levels->GetLevel(level)->commands;

        bool add = true;
        std::string currentCommand;
        std::string addCommands = "+";
        std::string deleteCommands = "-";
        for (; it != end; it++)
        {
            if ((*it)[0] == '-')
            {
                add = false;
                currentCommand = (*it).substr(1);
            }
            else if ((*it)[0] == '+')
            {
                add = true;
                currentCommand = (*it).substr(1);
            }
            else
            {
                add = true;
                currentCommand = (*it);
            }
            char flag = game.commands->FindCommandFlag(currentCommand);
            if (flag == 0)
            {
                ChatPrintTo(ent, "^3editcommands: ^7command \"" + currentCommand + "\" doesn't match any known command.");
                continue;
            }
            if (add)
            {
                addCommands += flag;
            }
            else
            {
                deleteCommands += flag;
            }
        }

        std::string duplicateFlags = "";
        for (size_t i = 0; i < addCommands.size(); i++)
        {
            if (deleteCommands.find(addCommands[i]) != std::string::npos)
            {
                ChatPrintTo(ent, va("^3editcommands: ^7ignoring command flag \"%c\". Are you trying to add or delete it?", addCommands[i]));
                duplicateFlags.push_back(addCommands[i]);
            }
        }

        if (duplicateFlags.length() > 0)
        {
            std::string temp;
            for (size_t i = 0; i < addCommands.length(); i++)
            {
                if (duplicateFlags.find(addCommands[i]) == std::string::npos)
                {
                    temp += addCommands[i];
                }
            }
            addCommands = temp;

            temp.clear();
            for (size_t i = 0; i < deleteCommands.length(); i++)
            {
                if (duplicateFlags.find(deleteCommands[i]) == std::string::npos)
                {
                    temp += deleteCommands[i];
                }
            }
            deleteCommands = temp;
        }
        
        bool editedPermissions = false;
        // always has + in it
        if (addCommands.length() > 1)
        {
            editedPermissions = true;
            currentPermissions += addCommands;
        }
        // always has - in it
        if (deleteCommands.length() > 1)
        {
            editedPermissions = true;
            currentPermissions += deleteCommands;
        }
        game.levels->Edit(level, "", currentPermissions, "", 1);

        ChatPrintTo(ent, "^3editcommands: ^7edited level " + (*argv)[1] + " permissions. New permissions are: " + game.levels->GetLevel(level)->commands);

        return true;
    }

    bool EditLevel(gentity_t* ent, Arguments argv)
    {
        if (argv->size() < 4)
        {
            PrintManual(ent, "editlevel");
            return false;
        }

        int updated = 0;
        int open = 0;

        int adminLevel = 0;
        std::string commands;
        std::string greeting;
        std::string title;

        if (!ToInt(argv->at(1), adminLevel))
        {
            ChatPrintTo(ent, va("^3editlevel: ^7%d is not an integer.", adminLevel));
            return false;
        }

        if (argv->size() > 2)
        {
            ConstArgIter it = argv->begin() + 2;

            while (it != argv->end())
            {
                if (*it == "-cmds" && it + 1 != argv->end())
                {
                    open = CMDS_OPEN;
                    updated |= CMDS_OPEN;
                }
                else if (*it == "-greeting" && it + 1 != argv->end())
                {
                    open = GREETING_OPEN;
                    updated |= GREETING_OPEN;
                }
                else if (*it == "-title" && it + 1 != argv->end())
                {
                    open = TITLE_OPEN;
                    updated |= TITLE_OPEN;
                }
                else if (*it == "-clear" && it + 1 != argv->end())
                {
                    ConstArgIter nextIt = it + 1;
                    if (*nextIt == "cmds")
                    {
                        commands = "";
                        updated |= CMDS_OPEN;

                    }
                    else if (*nextIt == "greeting")
                    {
                        greeting = "";
                        updated |= GREETING_OPEN;
                    }
                    else if (*nextIt == "title")
                    {
                        title = "";
                        updated |= TITLE_OPEN;
                    }
                    else
                    {
                        it++;
                    }
                }
                else
                {
                    switch (open)
                    {
                    case 0:
                        if (updated == 0)
                        {
                            ChatPrintTo(ent, va("^editlevel: ^7ignored argument \"%s^7\".", it->c_str()));
                        }
                        
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

        game.levels->Edit(adminLevel, title, commands, greeting, updated);
        
        for (int i = 0; i < level.numConnectedClients; i++)
        {
            int num = level.sortedClients[i];

            game.session->ParsePermissions(num);
        }

        ChatPrintTo(ent, va("^3editlevel: ^7updated level %d.", adminLevel));

        return true;
    }

    bool EditUser(gentity_t* ent, Arguments argv)
    {
        if (argv->size() < 4)
        {
            PrintManual(ent, "edituser");
            return false;
        }

        unsigned id = 0;
        if (!ToUnsigned(argv->at(1), id))
        {
            ChatPrintTo(ent, "^3edituser: ^7invalid id " + argv->at(1));
            return false;
        }

        if (!game.database->UserExists(id))
        {
            ChatPrintTo(ent, "^3edituser: ^7user does not exist.");
            return false;
        }

        int updated = 0;
        int open = 0;

        std::string commands;
        std::string greeting;
        std::string title;

        ConstArgIter it = argv->begin() + 2;
        while (it != argv->end())
        {
            if (*it == "-cmds" && it + 1 != argv->end())
            {
                open = CMDS_OPEN;
                updated |= Updated::COMMANDS;
            }
            else if (*it == "-greeting" && it + 1 != argv->end())
            {
                open = GREETING_OPEN;
                updated |= Updated::GREETING;
            }
            else if (*it == "-title" && it + 1 != argv->end())
            {
                open = TITLE_OPEN;
                updated |= Updated::TITLE;
            }
            else if (*it == "-clear" && it + 1 != argv->end())
            {
                ConstArgIter nextIt = it + 1;
                if (*nextIt == "cmds")
                {
                    commands = "";
                    updated |= Updated::COMMANDS;
                }
                else if (*nextIt == "greeting")
                {
                    greeting = "";
                    updated |= Updated::GREETING;
                }
                else if (*nextIt == "title")
                {
                    title = "";
                    updated |= Updated::TITLE;
                }
                else
                {
                    it++;
                }
            }
            else
            {
                switch (open)
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
        
        ChatPrintTo(ent, va("^3edituser: ^7updating user %d", id));
        return game.database->UpdateUser(ent, id, commands, greeting, title, updated);
    }    

    bool FindUser(gentity_t* ent, Arguments argv)
    {
        if (argv->size() != 2)
        {
            ChatPrintTo(ent, "^3usage: ^7!finduser <name>");
            return false;
        }

        game.database->FindUser(ent, argv->at(1));

        return true;
    }

    bool ListUserNames(gentity_t *ent, Arguments argv)
    {
        if (argv->size() != 2)
        {
            ChatPrintTo(ent, "^3usage: ^7!listusernames <id>");
            return false;
        }

        int id;
        if (!ToInt(argv->at(1), id))
        {
            ChatPrintTo(ent, va("^3listusernames: ^7%s is not an id", argv->at(1).c_str()));
            return false;
        }

        game.database->ListUserNames(ent, id);
        return true;
    }

    bool AdminCommands(gentity_t* ent, Arguments argv)
    {
        ChatPrintTo(ent, "AdminCommands is not implemented.");
        return true;
    }

    bool Finger(gentity_t* ent, Arguments argv)
    {
        if (argv->size() != 2)
        {
            PrintManual(ent, "finger");
            return false;
        }

        std::string err;
        gentity_t *target = PlayerGentityFromString(argv->at(1), err);
        if (!target)
        {
            ChatPrintTo(ent, "^3finger: ^7" + err);
            return false;
        }


        game.session->PrintFinger(ent, target);
        return true;
    }

    bool Help(gentity_t* ent, Arguments argv)
    {
        if (argv->size() == 1)
        {
            game.commands->List(ent);
        }
        else
        {
            PrintManual(ent, argv->at(1));
        }
        
        return true;
    }

    bool Kick(gentity_t* ent, Arguments argv)
    {
        const unsigned MIN_ARGS = 2;
        if (argv->size() < MIN_ARGS)
        {
            PrintManual(ent, "kick");
            return false;
        }

        std::string error = "";
        gentity_t *target = PlayerGentityFromString(argv->at(1), error);
        if (!target)
        {
            ChatPrintTo(ent, "^3kick: " + error);
            return false;
        }

        if (ent)
        {
            if (ent == target)
            {
                ChatPrintTo(ent, "^3kick: ^7you can't kick yourself.");
                return false;
            }

            if (IsTargetHigherLevel(ent, target, true))
            {
                ChatPrintTo(ent, "^3kick: ^7you can't kick a fellow admin.");
                return false;
            }
        }

        int timeout = 0;
        if (argv->size() >= 3)
        {
            if (!ToInt(argv->at(2), timeout))
            {
                ChatPrintTo(ent, "^3kick: ^7invalid timeout \"" + argv->at(2) + "\" specified.");
                return false;
            }
        }

        std::string reason;
        if (argv->size() >= 4)
        {
            reason = argv->at(3);
        }

        trap_DropClient(target - g_entities, reason.c_str(), timeout);
        return true;
    }

    bool LeastPlayed(gentity_t *ent, Arguments argv)
    {
        game.mapData->ListLeastPlayed(ent);
        return true;
    }

    bool LevelInfo(gentity_t* ent, Arguments argv)
    {
        if (argv->size() == 1)
        {
            game.levels->PrintLevelInfo(ent);
        }
        else
        {
            int level = 0;
            if (!ToInt(argv->at(1), level))
            {
                ChatPrintTo(ent, "^3levelinfo: ^7 invalid level.");
                return false;
            }

            game.levels->PrintLevelInfo(ent, level);
        }
        return true;
    }

    bool ListBans(gentity_t* ent, Arguments argv)
    {
        int page = 1;

        if (argv->size() > 1)
        {
            if (!ToInt(argv->at(1), page))
            {
                ChatPrintTo(ent, "^3listbans: ^7page is not a number.");
                return false;
            }
        }

        game.database->ListBans(ent, page);

        return true;
    }

    bool ListFlags(gentity_t* ent, Arguments argv)
    {
        game.commands->ListCommandFlags(ent);
        return true;
    }

    bool ListMaps(gentity_t* ent, Arguments argv)
    {
        game.mapData->ListMaps(ent);
        return true;
    }

    bool ListUsers(gentity_t* ent, Arguments argv)
    {
        int page = 1;

        if (argv->size() > 1)
        {
            if (!ToInt(argv->at(1), page))
            {
                ChatPrintTo(ent, "^3listusers: ^7page was not a number.");
                return false;
            }
        }

        game.database->ListUsers(ent, page);
        return true;
    }

    bool ListPlayers(gentity_t *ent, Arguments argv) {
        
        if (argv->size() == 1)
        {
            BeginBufferPrint();

            BufferPrint(ent, "# ETJumpID Level Player\n");

            for (auto i = 0; i < level.numConnectedClients; i++)
            {
                auto clientNum = level.sortedClients[i];
                auto id = game.session->GetId(g_entities + clientNum);

                BufferPrint(ent, (boost::format("^7%-2d%-9d%-6d%-s\n")
                    % clientNum 
                    % (id == -1 ? "-" : boost::lexical_cast<std::string>(id))
                    % game.session->GetLevel(g_entities + clientNum)
                    % (g_entities + clientNum)->client->pers.netname).str());
            }

            FinishBufferPrint(ent);            
        }

        return true;
    }

    bool Map(gentity_t* ent, Arguments argv)
    {
        if (argv->size() != 2)
        {
            // PrintManual(ent, "map");
            return false;
        }

        if (!MapExists(argv->at(1)))
        {
            ChatPrintTo(ent, "^3map: ^7map " + argv->at(1) + " does not exist.");
            return false;
        }

        trap_SendConsoleCommand(EXEC_APPEND, va("map %s", argv->at(1).c_str()));
        return true;
    }

    bool MapInfo(gentity_t* ent, Arguments argv)
    {
        game.mapData->PrintMapInfo(ent, argv->size() > 1 ? argv->at(1) : level.rawmapname);
        return true;
    }

    bool MostPlayed(gentity_t *ent, Arguments argv)
    {
        game.mapData->ListMostPlayed(ent);
        return true;
    }

    void MutePlayer(gentity_t *target)
    {
        target->client->sess.muted = qtrue;

        char userinfo[MAX_INFO_STRING] = "\0";
        char *ip = NULL;

        trap_GetUserinfo(target - g_entities, userinfo, sizeof(userinfo));
        ip = Info_ValueForKey(userinfo, "ip");

        G_AddIpMute(ip);
    }

    bool Mute(gentity_t* ent, Arguments argv)
    {
        if (argv->size() != 2)
        {
            PrintManual(ent, "mute");
            return false;
        }

        std::string errorMsg;

        gentity_t *target = PlayerGentityFromString(argv->at(1), errorMsg);
        if (!target)
        {
            ChatPrintTo(ent, "^3!mute: ^7" + errorMsg);
            return false;
        }

        if (ent)
        {
            if (ent == target)
            {
                ChatPrintTo(ent, "^3mute: ^7you cannot mute yourself.");
                return false;
            }

            if (IsTargetHigherLevel(ent, target, true))
            {
                ChatPrintTo(ent, "^3mute: ^7you cannot mute a fellow admin.");
                return false;
            }
        }

        if (target->client->sess.muted == qtrue)
        {
            ChatPrintTo(ent, "^3mute: " + std::string(target->client->pers.netname) + " ^7is already muted.");
            return false;
        }

        MutePlayer(target);
        CPTo(target, "^5You've been muted");
        ChatPrintTo(ent, std::string(target->client->pers.netname) + " ^7has been muted.");
        return true;
    }

    bool Noclip(gentity_t* ent, Arguments argv)
    {
        if (level.noNoclip) {
            ChatPrintTo(ent, "^3noclip: ^7noclip is disabled on this map.");
            return false;
        }

        if (argv->size() == 1) {
            if (!ent) {
                return false;
            }

            if (ent->client->sess.timerunActive) {
                ChatPrintTo(ent, "^3noclip: ^7cheats are disabled while timerun is active.");
                return false;
            }

            ent->client->noclip = ent->client->noclip ? qfalse : qtrue;
        }
        else {
            int count = 1;
            if (argv->size() == 3) {
                if (!ToInt(argv->at(2), count)) {
                    if (count < 0) {
                        count = 1;
                    }
                    count = 1;
                }
            }
            std::string err;
            gentity_t *other = PlayerGentityFromString(argv->at(1), err);
            if (!other) {
                ChatPrintTo(ent, "^3noclip: ^7" + err);
                return false;
            }

            if (other->client->sess.timerunActive) {
                ChatPrintTo(other, "^3noclip: ^7cheats are disabled while timerun is active.");
                return false;
            }

            if (count > 1) {
                ChatPrintTo(other, va("^3noclip: ^7you can use /noclip %d times.", count));
                ChatPrintTo(ent, va("^3noclip: ^7%s^7 can use /noclip %d times.", other->client->pers.netname, count));
            }
            else {
                ChatPrintTo(other, "^3noclip: ^7you can use /noclip once.");
                ChatPrintTo(ent, va("^3noclip: ^7%s^7 can use /noclip once.", other->client->pers.netname));
            }

            other->client->pers.noclipCount = count;
        }

        return true;
    }

    bool NoGoto(gentity_t* ent, Arguments argv)
    {
        if (argv->size() != 2)
        {
            PrintManual(ent, "nogoto");
            return false;
        }

        std::string err;
        gentity_t *target = NULL;
        target = PlayerGentityFromString(argv->at(1), err);
        if (!target)
        {
            ChatPrintTo(ent, "^3nocall: ^7" + err);
            return false;
        }

        if (target->client->sess.noGoto)
        {
            target->client->sess.noGoto = qfalse;
            ChatPrintTo(ent, va("^nogoto: ^7%s can use /goto now.", target->client->pers.netname));
            ChatPrintTo(target, "^3nogoto: ^7you can use /goto now.");
        }
        else
        {
            target->client->sess.noGoto = qtrue;
            ChatPrintTo(ent, va("^3nogoto: ^7%s can no longer use /goto.", target->client->pers.netname));
            ChatPrintTo(target, "^3nogoto: ^7you can no longer use /goto.");
        }

        return true;
    }

    bool NoSave(gentity_t* ent, Arguments argv)
    {
        if (argv->size() != 2)
        {
            PrintManual(ent, "nosave");
            return false;
        }

        std::string err;
        gentity_t *target = PlayerGentityFromString(argv->at(1), err);
        if (!target)
        {
            ChatPrintTo(ent, "^3nosave: ^7" + err);
            return false;
        }

        if (IsTargetHigherLevel(ent, target, true))
        {
            ChatPrintTo(ent, "^3nosave:^7 can't disable fellow admin's save and load.");
            return false;
        }

        if (target->client->sess.saveAllowed)
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

    bool Passvote(gentity_t* ent, Arguments argv)
    {
        if (level.voteInfo.voteTime)
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

    bool Putteam(gentity_t* ent, Arguments argv)
    {
        if (argv->size() != 3)
        {
            PrintManual(ent, "putteam");
            return false;
        }

        std::string err;
        gentity_t *target = PlayerGentityFromString(argv->at(1), err);
        if (!target)
        {
            ChatPrintTo(ent, "^3putteam: ^7" + err);
            return false;
        }

        if (IsTargetHigherLevel(ent, target, false))
        {
            ChatPrintTo(ent, "^3putteam: ^7you can't use putteam on a fellow admin.");
            return false;
        }

        target->client->sess.lastTeamSwitch = level.time;

        const weapon_t w = static_cast<weapon_t>(-1);
        SetTeam(target, argv->at(2).c_str(), qfalse, w, w, qtrue);

        return true;
    }

    bool ReadConfig(gentity_t* ent, Arguments argv)
    {
        ChatPrintTo(ent, "ReadConfig is not implemented.");
        return true;
    }

    bool RemoveSaves(gentity_t* ent, Arguments argv)
    {
        if (argv->size() != 2)
        {
            PrintManual(ent, "rmsaves");
            return false;
        }

        std::string error;
        gentity_t *target = PlayerGentityFromString(argv->at(1), error);
        if (!target)
        {
            ChatPrintTo(ent, "^3rmsaves: ^7" + error);
            return false;
        }

        if (IsTargetHigherLevel(ent, target, true))
        {
            ChatPrintTo(ent, "^3rmsaves: ^7can't remove fellow admin's saves.");
            return false;
        }

        game.saves->ResetSavedPositions(target);
        ChatPrintTo(ent, va("^3system: ^7%s^7's saves were removed.", target->client->pers.netname));
        ChatPrintTo(target, "^3system: ^7your saves were removed");
        return true;
    }

    bool Rename(gentity_t* ent, Arguments argv)
    {
        if (argv->size() != 3)
        {
            PrintManual(ent, "rename");
            return false;
        }

        std::string err;
        gentity_t *target = PlayerGentityFromString(argv->at(1), err);
        if (!target)
        {
            ChatPrintTo(ent, "^3rename: ^7" + err);
            return false;
        }

        char userinfo[MAX_INFO_STRING] = "\0";
        int cn = ClientNum(target);
        trap_GetUserinfo(cn, userinfo, sizeof(userinfo));

        const char* oldName = Info_ValueForKey(userinfo, "name");
        ChatPrintAll(va("^3rename: ^7%s^7 has been renamed to %s", oldName, argv->at(2).c_str()));
        Info_SetValueForKey(userinfo, "name", argv->at(2).c_str());
        trap_SetUserinfo(cn, userinfo);
        ClientUserinfoChanged(cn);
        trap_SendServerCommand(cn, va("set_name %s", argv->at(2).c_str()));
        return true;
    }

    bool Restart(gentity_t* ent, Arguments argv)
    {
        Svcmd_ResetMatch_f(qfalse, qtrue);
        return true;
    }

    bool RouteMaker(gentity_t *ent, Arguments argv)
    {
        if (argv->size() == 1)
        {
            PrintManual(ent, "routemaker");
            return false;
        }
        std::string err;
        gentity_t *target = PlayerGentityFromString(argv->at(1), err);
        if (!target)
        {
            ChatPrintTo(ent, "^3routemaker: ^7" + err);
            return false;
        }

        if (!target->client->pers.race.isRouteMaker)
        {
            ChatPrintTo(ent, va("^3routemaker: ^7%s ^7is the route maker.", target->client->pers.netname));
            ChatPrintTo(target, "^3routemaker: ^7you are the route maker.");
            target->client->pers.race.isRouteMaker = qtrue;
            game.races->StopRace();
            game.races->DesignMode(true);
            trap_SendServerCommand(target - g_entities, "route_designer 1");
        }
        else
        {
            ChatPrintTo(ent, va("^3routemaker: ^7%s ^7is no longer the route maker.", target->client->pers.netname));
            ChatPrintTo(target, "^3routemaker: ^7you are no longer the route maker.");
            game.races->DesignMode(false);
            target->client->pers.race.isRouteMaker = qfalse;
            trap_SendServerCommand(target - g_entities, "route_designer 0");
        }

        for (int i = 0; i < level.numConnectedClients; i++)
        {
            int cnum = level.sortedClients[i];
            gentity_t *p = g_entities + cnum;

            if (p != target && p->client->pers.race.isRouteMaker)
            {
                p->client->pers.race.isRouteMaker = qfalse;
                CPMTo(p, "^<ETJump: ^7new routemaker was selected.");
                trap_SendServerCommand(ent - g_entities, "route_designer 0");
            }
        }

        return true;
    }

    bool SetLevel(gentity_t* ent, Arguments argv)
    {
        // !setlevel <player> <level>
        // !setlevel -id id level

        if (argv->size() == 3)
        {
            std::string err;
            gentity_t *target = PlayerGentityFromString(argv->at(1), err);

            if (!target)
            {
                ChatPrintTo(ent, err);
                return false;
            }

            int level = 0;
            if (!ToInt(argv->at(2), level))
            {
                ChatPrintTo(ent, "^3setlevel: ^7invalid level " + argv->at(2));
                return false;
            }

            if (ent)
            {
                if (IsTargetHigherLevel(ent, target, false))
                {
                    ChatPrintTo(ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
                    return false;
                }

                if (level > game.session->GetLevel(ent))
                {
                    ChatPrintTo(ent, "^3setlevel: ^7you're not allowed to setlevel higher than your own level.");
                    return false;
                }
            }

            if (!game.levels->LevelExists(level))
            {
                ChatPrintTo(ent, "^3setlevel: ^7level does not exist.");
                return false;
            }

            if (!game.session->SetLevel(target, level))
            {
                ChatPrintTo(ent, va("^3setlevel: ^7%s", game.session->GetMessage().c_str()));
                return false;
            }

            ChatPrintTo(ent, va("^3setlevel: ^7%s^7 is now a level %d user.", target->client->pers.netname, level));
            ChatPrintTo(target, va("^3setlevel: ^7you are now a level %d user.", level));

            return true;
        }
        if (argv->size() == 4)
        {
            unsigned id = 0;
            if (!ToUnsigned(argv->at(2), id))
            {
                ChatPrintTo(ent, "^3setlevel: ^7invalid id " + argv->at(2));
                return false;
            }

            if (!game.session->UserExists(id))
            {
                ChatPrintTo(ent, "^3setlevel: ^7user with id " + argv->at(2) + " doesn't exist.");
                return false;
            }

            int level = 0;
            if (!ToInt(argv->at(3), level))
            {
                ChatPrintTo(ent, "^3setlevel: ^7invalid level " + argv->at(2));
                return false;
            }

            if (ent)
            {
                if (IsTargetHigherLevel(ent, id, false))
                {
                    ChatPrintTo(ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
                    return false;
                }

                if (level > game.session->GetLevel(ent))
                {
                    ChatPrintTo(ent, "^3setlevel: ^7you're not allowed to setlevel higher than your own level.");
                    return false;
                }
            }

            if (!game.levels->LevelExists(level))
            {
                ChatPrintTo(ent, "^3setlevel: ^7level does not exist.");
                return false;
            }

            if (!game.session->SetLevel(id, level))
            {
                ChatPrintTo(ent, va("^3setlevel: ^7%s", game.session->GetMessage().c_str()));
                return false;
            }

            ChatPrintTo(ent, va("^3setlevel: ^7user with id %d is now a level %d user.", id, level));
        }
        else
        {
            
        }

        return true;
    }

    bool Spectate(gentity_t* ent, Arguments argv)
    {
        if (!ent)
        {
            return qfalse;
        }

        if (argv->size() != 2)
        {
            if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
            {
                SetTeam(ent, "spectator", qfalse, static_cast<weapon_t>(-1), static_cast<weapon_t>(-1), qfalse);
            }

            return qtrue;
        }

        std::string error;
        gentity_t *target = PlayerGentityFromString(argv->at(1), error);

        if (!target)
        {
            ChatPrintTo(ent, "^spectate: ^7" + error);
            return false;
        }

        if (target->client->sess.sessionTeam == TEAM_SPECTATOR)
        {
            ChatPrintTo(ent, "^3!spectate:^7 you can't spectate a spectator.");
            return qfalse;
        }

        if (!G_AllowFollow(ent, target))
        {
            ChatPrintTo(ent, va("^3!spectate: %s ^7is locked from spectators.", target->client->pers.netname));
            return qfalse;
        }

        if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
        {
            SetTeam(ent, "spectator", qfalse,
                static_cast<weapon_t>(-1), static_cast<weapon_t>(-1), qfalse);
        }

        ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
        ent->client->sess.spectatorClient = target->client->ps.clientNum;
        return qtrue;
    }

    bool Unban(gentity_t* ent, Arguments argv)
    {

        if (argv->size() == 1)
        {
            PrintManual(ent, "unban");
            return false;
        }

        int id;
        if (!ToInt(argv->at(1), id))
        {
            ChatPrintTo(ent, "^3unban: ^7id is not a number.");
            return false;
        }

        if (!game.database->Unban(ent, id))
        {
            ChatPrintTo(ent, "^3unban: ^7" + game.database->GetMessage());
            return false;
        }

        ChatPrintTo(ent, "^3unban: ^7removed ban with id " + argv->at(1));
        return true;
    }

    bool Unmute(gentity_t* ent, Arguments argv)
    {
        if (argv->size() != 2)
        {
            PrintManual(ent, "unmute");
            return false;
        }

        std::string error;
        gentity_t *target = PlayerGentityFromString(argv->at(1), error);
        if (!target)
        {
            ChatPrintTo(ent, "^3unmute: ^7" + error);
            return false;
        }


        if (!target->client->sess.muted)
        {
            ChatPrintTo(ent, "^3unmute: ^7target is not muted.");
            return false;
        }

        target->client->sess.muted = qfalse;

        char *ip = NULL;
        char userinfo[MAX_INFO_STRING] = "\0";
        trap_GetUserinfo(ClientNum(target), userinfo, sizeof(userinfo));
        ip = Info_ValueForKey(userinfo, "ip");

        G_RemoveIPMute(ip);

        CPTo(target, "^5You've been unmuted.");
        ChatPrintAll(target->client->pers.netname + std::string(" ^7has been unmuted."));

        return true;
    }

    bool UserInfo(gentity_t* ent, Arguments argv)
    {
        if (argv->size() == 1)
        {
            PrintManual(ent, "userinfo");
            return false;
        }

        int id;
        if (!ToInt(argv->at(1), id))
        {
            ChatPrintTo(ent, "^3userinfo: ^7id was not a number.");
            return false;
        }

        game.database->UserInfo(ent, id);

        return true;
    }

    
    
}

Commands::Commands()
{
    //using AdminCommands::AdminCommand;
    //adminCommands_["addlevel"] = AdminCommand(AdminCommands::AddLevel, 'a');
    
    
    adminCommands_["addlevel"] = AdminCommandPair(AdminCommands::AddLevel, CommandFlags::EDIT);
    adminCommands_["admintest"] = AdminCommandPair(AdminCommands::Admintest, CommandFlags::BASIC);
    adminCommands_["8ball"] = AdminCommandPair(AdminCommands::Ball8, CommandFlags::BASIC);
    adminCommands_["ban"] = AdminCommandPair(AdminCommands::Ban, CommandFlags::BAN);
    adminCommands_["cancelvote"] = AdminCommandPair(AdminCommands::Cancelvote, CommandFlags::CANCELVOTE);
    adminCommands_["deletelevel"] = AdminCommandPair(AdminCommands::DeleteLevel, CommandFlags::EDIT);
    //adminCommands_["deleteuser"] = AdminCommandPair(AdminCommands::DeleteUser, CommandFlags::EDIT);
    adminCommands_["editcommands"] = AdminCommandPair(AdminCommands::EditCommands, CommandFlags::EDIT);
    adminCommands_["editlevel"] = AdminCommandPair(AdminCommands::EditLevel, CommandFlags::EDIT);
    adminCommands_["edituser"] = AdminCommandPair(AdminCommands::EditUser, CommandFlags::EDIT);
    adminCommands_["finduser"] = AdminCommandPair(AdminCommands::FindUser, CommandFlags::EDIT);
    adminCommands_["listusernames"] = AdminCommandPair(AdminCommands::ListUserNames, CommandFlags::EDIT);
    adminCommands_["finger"] = AdminCommandPair(AdminCommands::Finger, CommandFlags::FINGER);
    adminCommands_["help"] = AdminCommandPair(AdminCommands::Help, CommandFlags::BASIC);
    adminCommands_["kick"] = AdminCommandPair(AdminCommands::Kick, CommandFlags::KICK);
    adminCommands_["leastplayed"] = AdminCommandPair(AdminCommands::LeastPlayed, CommandFlags::BASIC);
    adminCommands_["levelinfo"] = AdminCommandPair(AdminCommands::LevelInfo, CommandFlags::EDIT);
    adminCommands_["listbans"] = AdminCommandPair(AdminCommands::ListBans, CommandFlags::LISTBANS);
    //adminCommands_["listcmds"] = AdminCommandPair(AdminCommands::ListCommands, CommandFlags::BASIC);
    adminCommands_["listflags"] = AdminCommandPair(AdminCommands::ListFlags, CommandFlags::EDIT);
    adminCommands_["listmaps"] = AdminCommandPair(AdminCommands::ListMaps, CommandFlags::BASIC);
    adminCommands_["listplayers"] = AdminCommandPair(AdminCommands::ListPlayers, CommandFlags::LISTPLAYERS);
    adminCommands_["listusers"] = AdminCommandPair(AdminCommands::ListUsers, CommandFlags::EDIT);
    adminCommands_["map"] = AdminCommandPair(AdminCommands::Map, CommandFlags::MAP);
    adminCommands_["mapinfo"] = AdminCommandPair(AdminCommands::MapInfo, CommandFlags::BASIC);
    adminCommands_["mostplayed"] = AdminCommandPair(AdminCommands::MostPlayed, CommandFlags::BASIC);
    adminCommands_["mute"] = AdminCommandPair(AdminCommands::Mute, CommandFlags::MUTE);
    adminCommands_["noclip"] = AdminCommandPair(AdminCommands::Noclip, CommandFlags::NOCLIP);
    adminCommands_["nogoto"] = AdminCommandPair(AdminCommands::NoGoto, CommandFlags::NOGOTO);
    adminCommands_["nosave"] = AdminCommandPair(AdminCommands::NoSave, CommandFlags::SAVESYSTEM);
    adminCommands_["passvote"] = AdminCommandPair(AdminCommands::Passvote, CommandFlags::PASSVOTE);
    adminCommands_["putteam"] = AdminCommandPair(AdminCommands::Putteam, CommandFlags::PUTTEAM);
//    adminCommands_["readconfig"] = AdminCommandPair(AdminCommands::ReadConfig, CommandFlags::READCONFIG);
    adminCommands_["rmsaves"] = AdminCommandPair(AdminCommands::RemoveSaves, CommandFlags::SAVESYSTEM);
    adminCommands_["rename"] = AdminCommandPair(AdminCommands::Rename, CommandFlags::RENAME);
    adminCommands_["restart"] = AdminCommandPair(AdminCommands::Restart, CommandFlags::RESTART);
    adminCommands_["routemaker"] = AdminCommandPair(AdminCommands::RouteMaker, CommandFlags::ROUTE_MAKER);
    adminCommands_["setlevel"] = AdminCommandPair(AdminCommands::SetLevel, CommandFlags::SETLEVEL);
    adminCommands_["spectate"] = AdminCommandPair(AdminCommands::Spectate, CommandFlags::BASIC);
    adminCommands_["unban"] = AdminCommandPair(AdminCommands::Unban, CommandFlags::BAN);
    adminCommands_["unmute"] = AdminCommandPair(AdminCommands::Unmute, CommandFlags::MUTE);
    adminCommands_["userinfo"] = AdminCommandPair(AdminCommands::UserInfo, CommandFlags::EDIT);

    commands_["backup"] = ClientCommands::BackupLoad;
    commands_["save"] = ClientCommands::Save;
    commands_["load"] = ClientCommands::Load;
//    commands_["race"] = ClientCommands::Race;
    commands_["listinfo"] = ClientCommands::ListInfo;
    commands_["records"] = ClientCommands::Records;
    commands_["times"] = ClientCommands::Records;
    commands_["ranks"] = ClientCommands::Records;
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

bool Commands::List(gentity_t *ent)
{
    ConstAdminCommandIterator it = adminCommands_.begin(),
        end = adminCommands_.end();

    BeginBufferPrint();
    ChatPrintTo(ent, "^3help: ^7check console for more information.");
    int i = 1;
    std::bitset<256> perm = game.session->Permissions(ent);
    for (; it != end; it++)
    {
        if (perm[it->second.second] == false)
        {
            continue;
        }

        BufferPrint(ent, va("%-20s ", it->first.c_str()));
        if (i != 0 && i % 3 == 0)
        {
            BufferPrint(ent, "\n");
        }

        i++;
    }

    FinishBufferPrint(ent, true);
    return true;
}

bool Commands::AdminCommand(gentity_t* ent)
{
    std::string command = "",
                arg = SayArgv(0);
    int skip = 0;

    if (arg == "say" || arg == "enc_say")
    {
        arg = SayArgv(1);
        skip = 1;
    }
    else
    {
        if (ent && !game.session->HasPermission(ent, '/'))
        {
            return false;
        }
    }
    Arguments argv = GetSayArgs(skip);

    if (arg.length() == 0)
    {
        return false;
    }

    if (arg[0] == '!')
    {
        if (arg.length() == 1)
        {
            return false;
        }
        command = &arg[1];
    }
    else if (ent == NULL)
    {
        command = arg;
    }
    else
    {
        return false;
    }

    boost::to_lower(command);

    ConstAdminCommandIterator it = adminCommands_.lower_bound(command);

    if (it == adminCommands_.end())
    {
        return false;
    }
    std::bitset<256> permissions =
        game.session->Permissions(ent);
    std::vector<ConstAdminCommandIterator> foundCommands;
    while (it != adminCommands_.end() &&
        it->first.compare(0, command.length(), command) == 0) {
        if (permissions[it->second.second])
        {
            if (it->first == command)
            {
                foundCommands.clear();
                foundCommands.push_back(it);
                break;
            }
            foundCommands.push_back(it);
        }
        it++;
    }

    if (foundCommands.size() == 1)
    {
        if (ent)
        {
            G_ALog("Command: (%d) %s: %s",
                ClientNum(ent), ent->client->pers.netname,
                ConcatArgs(skip));
        }
        else
        {
            G_ALog("Command: console: %s", ConcatArgs(skip));
        }
        
        foundCommands[0]->second.first(ent, argv);
        return true;
    }
    
    if (foundCommands.size() > 1)
    {
        ChatPrintTo(ent, "^3server: ^7multiple matching commands found. Check console for more information");
        BeginBufferPrint();
        for (size_t i = 0; i < foundCommands.size(); i++)
        {
            BufferPrint(ent, va("* %s\n", foundCommands.at(i)->first.c_str()));
        }
        FinishBufferPrint(ent);
    }

    return false;
}

qboolean AdminCommandCheck(gentity_t *ent)
{
    return game.commands->AdminCommand(ent) ? qtrue : qfalse;
}

void Commands::ListCommandFlags(gentity_t* ent)
{
    ChatPrintTo(ent, "^3listflags: ^7check console for more information.");

    BeginBufferPrint();
    
    boost::format fmt("%c %s\n");
    for (ConstAdminCommandIterator it = adminCommands_.begin(), end = adminCommands_.end(); it != end; it++)
    {
        BufferPrint(ent, (fmt % it->second.second % it->first).str());
    }

    FinishBufferPrint(ent);
}

char Commands::FindCommandFlag(const std::string &command) {
    AdminCommandsIterator it = adminCommands_.begin();
    AdminCommandsIterator end = adminCommands_.end();
    while (it != end) {
        if ((*it).first == command) {
            return (*it).second.second;
        }
        it++;
    }
    return 0;
}
