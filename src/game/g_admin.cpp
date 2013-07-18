#include "g_local.hpp"
#include "g_utilities.hpp"

bool TargetIsHigherLevel(gentity_t *ent, gentity_t *target, bool equalIsHigher = false)
{
    if(equalIsHigher)
    {
        return sessionDB.Level(target) >= sessionDB.Level(ent);
    } else
    {
        return sessionDB.Level(target) > sessionDB.Level(ent);
    }
}

namespace AdminCommands
{
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

    bool Setlevel(gentity_t *ent, Arguments argv)
    {
        // !setlevel player level
        // !setlevel -id id level
        if(argv->size() < 3)
        {
            ChatPrintTo(ent, "^3usage: ^7!setlevel <player> <level>");
            ChatPrintTo(ent, "^3usage: ^7!setlevel -id <id> <level>");
            return false;
        } else if(argv->size() == 3)
        {
            // Normal setlevel
            char err[MAX_TOKEN_CHARS] = "\0";
            gentity_t *target = PlayerGentityFromString(argv->at(1), 
                err, sizeof(err));

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

            if(TargetIsHigherLevel(ent, target)) 
            {
                ChatPrintTo(ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
                return false;
            }

            if(level > sessionDB.Level(ent))
            {
                ChatPrintTo(ent, "^3setlevel: ^7you're not allowed to setlevel higher than your own level.");
                return false;
            }

            // AdminDB does the level exists check
            return adminDB.SetLevel(ent, target, level);
            
//             if(!adminDB.LevelExists(level))
//             {
//                 ChatPrintTo(ent, "^3setlevel: ^7level does not exist.");
//                 return false;
//             }
// 
//             adminDB.UpdateUserLevel(sessionDB.Guid(target), level);
//             ChatPrintAll(va("^3setlevel: ^7%s is now a level %d user (%s)", 
//                 target->client->pers.netname,
//                 level, 
//                 adminDB.LevelName(level).c_str()));
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

            if(level > sessionDB.Level(ent))
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
}

// FIXME: move to .hpp
struct AdminCommand_s
{
    std::string keyword;
    bool (*handler)(gentity_t *ent, Arguments argv);
    char flag;
    std::string function;
    std::string syntax;
};

static AdminCommand_s AdminCommandList[] =
{
    {
        "admintest", AdminCommands::Admintest, 'a', "", ""
    }
    ,
    {
        "setlevel", AdminCommands::Setlevel, 's', "", ""
    }
    ,
    {
        "", 0, 0, "", ""
    }
};



qboolean CheckCommand( gentity_t *ent )
{
    // say "!command additional args"

    if(g_admin.integer == 0)
    {
        return qfalse;
    }

    std::string command = "";
    std::string arg = SayArgv(0);
    int skip = 0;
    if( arg == "say" )
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

    for(size_t i = 0; AdminCommandList[i].flag != 0; i++)
    {
        if(command != AdminCommandList[i].keyword)
        {
            continue;
        } else
        {
            if(sessionDB.HasPermission(ent, AdminCommandList[i].flag))
            {
                AdminCommandList[i].handler(ent, argv);
                return qtrue;  
            } else {
                ChatPrintTo(ent, va("^3%s: ^7permission denied", 
                    AdminCommandList[i].keyword.c_str()));
                return qtrue;
            }
        }
    }

    return qfalse;
}