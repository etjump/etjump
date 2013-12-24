#ifndef commands_h__
#define commands_h__

#include "../g_utilities.hpp"

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
    const char PASSVOTE = 'P';
    const char PUTTEAM = 'p';
    const char READCONFIG = 'G';
    const char RENAME = 'R';
    const char RESTART = 'r';
    const char SAVESYSTEM = 'T';
    const char SETLEVEL = 's';
}

bool Admintest( gentity_t *ent, Arguments argv );
bool Map( gentity_t *ent, Arguments argv );

#endif // commands_h__
