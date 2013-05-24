#include "g_utilities.h"
#include "g_users.h"
#include "g_local.hpp"
#include "g_admin.h"

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
using std::string;
using std::vector;

namespace Commands {
    // used to add level to leveldatabase
    // syntax: !addlevel <new level>
    // TODO: should be possible to actually set name, greeting and commands
    // on create aswell
    bool AddLevel(gentity_t *ent, Arguments argv) {

        const size_t MIN_ADDLEVEL_ARGS = 2;
        if(argv->size() < MIN_ADDLEVEL_ARGS) {
            ChatPrintTo(ent, "^3usage:^7 !addlevel <level>");
            return false;
        }

        int level = 0;
        if(!StringToInt(argv->at(1), level)) {
            ChatPrintTo(ent, "^3addlevel: ^7invalid level " + argv->at(1));
            return false;
        }

        // let's try to add the level to database
        if(!levels.AddLevel(level)) {
            ChatPrintTo(ent, "^3addlevel: ^7" + levels.Error());
            return false;
        }

        ChatPrintTo(ent, "^3addlevel: ^7successfully added level " + argv->at(1));

        return true;
    }

    bool Admintest(gentity_t *ent, Arguments argv) {

        if(!ent) {
            ChatPrintAll("^3admintest: ^7console is a level 2147483648 user (Your daddy)");
            return false;
        } else {
			char buf[MAX_TOKEN_CHARS];
			Com_sprintf(buf, sizeof(buf), "^3admintest: ^7%s ^7is a level %d user (%s^7)",
				ent->client->pers.netname, users.Level(ent), levels.Name(users.Level(ent)).c_str());
			ChatPrintAll(buf);
		}

    	return true;
    }

    bool Ball8(gentity_t *ent, Arguments argv) {
        static const char *m8BallResponses[] = {
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

        const int RESPONSE_COUNT = 20;
        const unsigned DELAY_8BALL = 3000; // ms

        if( ent && ent->client->last8BallTime + DELAY_8BALL > level.time ) {
            ChatPrintTo(ent, va("^38ball: ^7you must wait %d seconds before"
                "using !8ball", 
                (ent->client->last8BallTime + DELAY_8BALL - level.time)/1000));
            return false;
        }

        if(argv->size() < 2) {
            ChatPrintTo(ent, "^3usage:^7 !8ball <question>");
            return false;
        }

        int random = rand() % RESPONSE_COUNT;
        if(random < 10) {
            ChatPrintAll(va("^3Magic 8 Ball: ^2%s.", m8BallResponses[random]));
        } else if (random < 15) {
            ChatPrintAll(va("^3Magic 8 Ball: ^3%s.", m8BallResponses[random]));
        } else {
            ChatPrintAll(va("^3Magic 8 Ball: ^1%s.", m8BallResponses[random]));
        }	

        if(ent) {
            ent->client->last8BallTime = level.time;
        }

    	return true;
    }

    bool Ban(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool Cancelvote(gentity_t *ent, Arguments argv) {
    	return true;
    }

    // used to edit admin levels. currently possible
    // to edit/append to commands, edit greeting and edit name
    // also possible to reset(clear) name, greeting & commands
    // examples:
    // !editlevel 5 -append -cmds abc
    // !editlevel -cmds newcommands
    // !editlevel -clear -cmds
    // !editlevel -clear -name
    // !editlevel -clear -greeting
    // !editlevel -greeting newgreeting is here -name this is a new name
    // it is not possible to clear a name then edit other two 
    bool EditLevel(gentity_t *ent, Arguments argv) {

        // !editlevel 5 -append -cmds abc
        // !editlevel 5 -cmds abcde
        // !editlevel 5 -clear -cmds
        // !editlevel 5 -clear -cmds -greeting -name
        const size_t MIN_EDITLEVEL_ARGS = 4;
        if(argv->size() < MIN_EDITLEVEL_ARGS) {
            ChatPrintTo(ent, "^3usage: ^7check console for more information.");
            BeginBufferPrint();

            BufferPrint(ent, "!editlevel <level> "
                "[-a/-append|-c/-clear|-r/-replace]"
                "[-cmds|-greeting|-name] <new value>");
            FinishBufferPrint(ent, true);
            return false;
        }

        int level = 0;
        if(!StringToInt(argv->at(1), level)) {
            ChatPrintTo(ent, "^3editlevel: ^7invalid level " + argv->at(1));
            return false;
        }

        EDIT_MODE editMode = REPLACE;
        
        if(argv->at(2) == "-c" || argv->at(2) == "-clear") {
            editMode = CLEAR;
        } else if(argv->at(2) == "-a" || argv->at(2) == "-append") {
            editMode = APPEND;
        } else if(argv->at(2) == "-r" || argv->at(2) == "-replace") {
            editMode = REPLACE;
        } 

        string newCommands;
        string newGreeting;
        string newName;

        int editing     = -1;
        int edited      = NONE;
        ConstArgIter it = argv->begin();

        while(it != argv->end())
        {
            if( editMode == CLEAR ) {
                if( *it == "-cmds" ) {
                    edited |= (1 << COMMANDS);
                } else if( *it == "-greeting" ) {
                    edited |= (1 << GREETING);
                } else if( *it == "-name" ) {
                    edited |= (1 << NAME);
                }
            } else {
                if( (*it == "-cmds" || *it == "-commands") 
                    && (it + 1) != argv->end() ) {
                    editing = COMMANDS;
                } else if( *it == "-greeting" && (it + 1) != argv->end() ) {
                    editing = GREETING;
                } else if( *it == "-name" && (it + 1) != argv->end() ) {
                    editing = NAME;
                } else if( editing == COMMANDS ) {
                    newCommands += *it;
                    edited |= (1 << COMMANDS);
                } else if( editing == GREETING ) {
                    newGreeting += *it;
                    newGreeting += ' ';
                    edited |= (1 << GREETING);
                } else if( editing == NAME ) {
                    newName += *it;
                    newName += " ";
                    edited |= (1 << NAME);
                }
            }
            it++;
        }

        if( edited == NONE ) {
            ChatPrintTo(ent, "^3editlevel: ^7nothing was edited.");
            return true;
        }

        boost::trim(newCommands);
        boost::trim(newGreeting);
        boost::trim(newName);

        if(!levels.UpdateLevel(level,
            newCommands, newName, newGreeting, edited, editMode))
        {
            ChatPrintTo(ent, "^3editlevel: ^7" + levels.Error());
            return false;
        }

        ChatPrintTo(ent, "^3editlevel: ^7successfully edited level " + argv->at(1));
        levels.PrintLevelToConsole(level, ent);

		users.UpdatePermissions();

        return true;
    }

    // prints admin information about user
    // syntax: !finger (-all) player
    // if switch -all is given, prints the information to everyone
    // else to just caller
    // -all is not necessary
    bool Finger(gentity_t *ent, Arguments argv) {
        const char FINGER_MSG[] = 
            "^3finger: ^7%s^7 is a level %d user (%s^7)";

        if(argv->size() < 2) {
            ChatPrintTo(ent, "^3usage: ^7!finger <player>");
            return false;
        }

        int skiparg = 0;
        bool printToAll = false;
        if(argv->size() == 3 && argv->at(1) == "-all") {
            skiparg = 1;
            printToAll = true;
        }

        char errMsg[MAX_TOKEN_CHARS];
        gentity_t *target = 
            PlayerGentityFromString(argv->at(1 + skiparg), errMsg, sizeof(errMsg));

        if(!target) {
            ChatPrintTo(ent, "^3finger: ^7couldn't find target player. "
                "Check console for more information");
            ConsolePrintTo(ent, errMsg);
            return false;
        }

        char toPrint[MAX_TOKEN_CHARS];
        Com_sprintf(toPrint, sizeof(toPrint), FINGER_MSG, 
            target->client->pers.netname, users.Level(target), 
            levels.Name(users.Level(target)).c_str());

        if(printToAll) {
            ChatPrintAll(toPrint);   
        } else {
            ChatPrintTo(ent, toPrint);
        }

    	return true;
    }

    bool Help(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool Kick(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool ListBans(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool ListMaps(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool ListPlayers(gentity_t *ent, Arguments argv) {
    	return true;
    }

    // changes to given map if it exists.
    bool Map(gentity_t *ent, Arguments argv) {

        if(argv->size() < 2) {
            ChatPrintTo(ent, "^3usage: ^7!map <map name>");
            return false;
        }

        char mapFilename[MAX_QPATH];

        Com_sprintf(mapFilename, sizeof(mapFilename), "maps/%s.bsp",
            argv->at(1).c_str());

        fileHandle_t f;
        int len = trap_FS_FOpenFile(mapFilename, &f, FS_READ);
        trap_FS_FCloseFile(f);

        if(!f) {
            ChatPrintTo(ent, "^3map: ^7couldn't find map " + argv->at(1));
            return false;
        }

        trap_SendConsoleCommand(EXEC_APPEND, va("map %s\n", argv->at(1).c_str()));

    	return true;
    }

    bool Mute(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool Noclip(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool Passvote(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool Putteam(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool Readconfig(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool Rename(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool Restart(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool Setlevel(gentity_t *ent, Arguments argv) {
        // !setlevel <player> <level>
        // TODO: !setlevel -id <id> <level>
        
        if( argv->size() < 3 ) {
            ChatPrintTo(ent, "^3usage:^7 !setlevel <player> <level>");
            return false;
        }

        int level = 0;
        if(!StringToInt(argv->at(2), level)) {
            ChatPrintTo(ent, "^3setlevel: ^7invalid level " + argv->at(1));
            return false;
        }
        
        char errmsg[MAX_TOKEN_CHARS];
        gentity_t *target = 
            PlayerGentityFromString(argv->at(1), errmsg, sizeof(errmsg));

        if(!target) {
            ChatPrintTo(ent, "^3setlevel: ^7couldn't find target player. "
                "Check console for more information");
            ConsolePrintTo(ent, errmsg);
            return false;
        }

        if( !IsAllowed(ent, target, true) ) {
            ChatPrintTo(ent, "^3setlevel: ^7you are not allowed "
                "to setlevel fellow admins.");
            return false;
        }

        if( !users.SetLevel(target, level) ) {
            ChatPrintTo(ent, "^3setlevel: ^7failed to update database. "
                "Users level is temporarily set to " + argv->at(1));
            return false;
        }

        char toPrint[MAX_TOKEN_CHARS];
        Com_sprintf(toPrint, sizeof(toPrint),
            "^3setlevel: ^7%s^7 is now level %s user",
            target->client->pers.netname, argv->at(2).c_str());
        ChatPrintTo(ent, toPrint);

    	return true;
    }

    bool Spectate(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool Unban(gentity_t *ent, Arguments argv) {
    	return true;
    }

    bool Unmute(gentity_t *ent, Arguments argv) {
    	return true;
    }
}

static const Command commands[] = {
    {"addlevel",    Commands::AddLevel,     'A', "", ""},
    {"admintest",   Commands::Admintest,    'a', "", ""},
    {"8ball",       Commands::Ball8,        '8', "", ""},
    {"ban",         Commands::Ban,          'b', "", ""},
    {"cancelvote",  Commands::Cancelvote,   'C', "", ""},
    {"editlevel",   Commands::EditLevel,    'A', "", ""},
    {"finger",      Commands::Finger,       '!', "", ""},
    {"help",        Commands::Help,         '!', "", ""},
    {"kick",        Commands::Kick,         '!', "", ""},
    {"listbans",    Commands::ListBans,     '!', "", ""},
    {"listmaps",    Commands::ListMaps,     '!', "", ""},
    {"listplayers", Commands::ListPlayers,  '!', "", ""},
    {"map",         Commands::Map,          '!', "", ""},
    {"mute",        Commands::Mute,         '!', "", ""},
    {"noclip",      Commands::Noclip,       '!', "", ""},
    {"passvote",    Commands::Passvote,     '!', "", ""},
    {"putteam",     Commands::Putteam,      '!', "", ""},
    {"readconfig",  Commands::Readconfig,   '!', "", ""},
    {"rename",      Commands::Rename,       '!', "", ""},
    {"restart",     Commands::Restart,      '!', "", ""},
    {"setlevel",    Commands::Setlevel,     '!', "", ""},
    {"spectate",    Commands::Spectate,     '!', "", ""},
    {"unban",       Commands::Unban,        '!', "", ""},
    {"unmute",      Commands::Unmute,       '!', "", ""},
};
static const unsigned COMMANDS_SIZE = sizeof(commands)/sizeof(Command); 

// Expects strings to be lowercased already
int FindMatchingCommands(const string& toMatch, int indices[]) 
{
    int foundMatches = 0;
    int commandIndex = 0;

    for(commandIndex = 0; commandIndex < COMMANDS_SIZE; commandIndex++)
    {
        if(!commands[commandIndex].keyword.compare
            (0, toMatch.length(), toMatch))
        {
            G_DPrintf("Found matching command: %s\n", commands[commandIndex].keyword.c_str());
            indices[foundMatches] = commandIndex;
            foundMatches++;
        }
    }
    return foundMatches;
}

bool CheckPermissions(gentity_t *ent, char flag) 
{
	if(!ent) {
		return true;
	}
	return users.Permissions(ent).test(flag);

	//return users.Permissions(ent).at(flag);
}

qboolean G_CheckPermissions(gentity_t *ent, char flag) {
	if(CheckPermissions(ent, flag)) {
		return qtrue;
	}
	return qfalse;
}

qboolean CommandCheck(gentity_t *ent) 
{
    if( g_admin.integer == 0 ) {
        return qfalse;
    }

    char    *cmd                    = NULL;
    char    arg1[MAX_TOKEN_CHARS]   = "\0";
    char    arg2[MAX_TOKEN_CHARS]   = "\0";
    int     argc                    = 0;
    int     skipargs                = 0;
    int     foundMatches            = 0;
    int     matchingIndices[COMMANDS_SIZE];

    static vector<string> argv;
    argv.clear();
    string error;

    // If there's just "say" in it do nothing
    if((argc = Q_SayArgc()) < 1) {
        return qfalse;
    }

    Q_SayArgv(0, arg1, sizeof(arg1));

    G_DPrintf("CommandCheck: %s\n", arg1);

    if(!Q_stricmp(arg1, "say")) {
        skipargs = 1;
        if(argc < 2) {
            return qfalse;
        }
        // if second arg does not start with ! it is not an admin cmd
        Q_SayArgv(1, arg2, sizeof(arg2));
        if(arg2[0] == '!') {
            cmd = &arg2[1];
        } else {
            return qfalse;
        }
    } 
    // if first arg starts with ! it was a console command
    // intended to be an admin cmd call
    // TODO: Check if allowed to do silent cmds
    else if(arg1[0] == '!') {
        cmd = &arg1[1];
    } 
    // if first (rcon/console) arg does not start with ! just check if it 
    // happens to be some command anyway
    else {
        cmd = &arg1[0];
    }

    // Match the arg to a command from the array    
    boost::algorithm::to_lower(cmd);
    if((foundMatches = FindMatchingCommands(cmd, matchingIndices)) != 1) {
        if(!foundMatches) {
            return qfalse;
        }
        if(ent) {
            ChatPrintTo(ent, "^3ETJump: ^7check console for more information.");
        }
        BeginBufferPrint();
        BufferPrint(ent, "^3ETJump: ^7more than one command matches."
            " Be more specific.\n");
        for(int i = 0; i < foundMatches; i++) {
            char lineToPrint[MAX_TOKEN_CHARS];
            Com_sprintf(lineToPrint, sizeof(lineToPrint),
                "%d - %s\n", i + 1, 
                commands[matchingIndices[i]].keyword.c_str());
            BufferPrint(ent, lineToPrint);
        }
        FinishBufferPrint(ent);
        return qfalse;
    }

	if(!CheckPermissions(ent, commands[matchingIndices[0]].flag)) {
		char buf[MAX_TOKEN_CHARS];
		Com_sprintf(buf, sizeof(buf), "^3%s: ^7permission denied.", 
			commands[matchingIndices[0]].keyword.c_str());
		ChatPrintTo(ent, buf);
		return qfalse;
	}

    // Push the first useful arg without ! to vector
    argv.push_back(cmd);

    // Push all except possible "say" and first arg to vector
    for(int i = 1 + skipargs; i < argc; i++) {
        Q_SayArgv(i, arg1, sizeof(arg1));
        argv.push_back(arg1);
    }

    commands[matchingIndices[0]].handler(ent, &argv);
    return qtrue;
}

    // Calculates the additional length needed for formating with color codes
int StringColorLen(const std::string& str) {
    return (str.length() - CleanStrlen(str.c_str()));
}

void Svcmd_ListAliases_f(void) {

    if( !g_admin.integer|| !g_aliasDB.integer ) {
        G_LogPrintf("listaliases: g_admin or g_aliasDatabase is not set.\n");
        return;
    }

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

    if( !aliases ) {
        G_LogPrintf("listaliases: failed to get aliases.\n");
        return;
    }

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

// Is user's level higher (or equal) than targets
bool IsAllowed( gentity_t *caller, gentity_t *target, bool equalIsHigher /*= true*/ )
{
    if( !caller ) {
        return true;
    }
    if(equalIsHigher) {
        // If targets level is higher or equal to callers, deny
        return users.Level(caller) > users.Level(target);
    } else {
        return users.Level(caller) >= users.Level(target);
    }
}
