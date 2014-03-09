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
    const char NOGOTO = 'K';
    const char PASSVOTE = 'P';
    const char PUTTEAM = 'p';
    const char READCONFIG = 'G';
    const char RENAME = 'R';
    const char RESTART = 'r';
    const char SAVESYSTEM = 'T';
    const char SETLEVEL = 's';
}

bool AddLevel( gentity_t *ent, Arguments argv );
bool Admintest( gentity_t *ent, Arguments argv );
bool Ball8( gentity_t *ent, Arguments argv );
bool Ban( gentity_t *ent, Arguments argv );
bool Cancelvote( gentity_t *ent, Arguments argv );
bool DeleteLevel( gentity_t *ent, Arguments argv );
bool DeleteUser( gentity_t *ent, Arguments argv );
bool EditCommands( gentity_t *ent, Arguments argv );
bool EditLevel( gentity_t *ent, Arguments argv );
bool EditUser( gentity_t *ent, Arguments argv );
bool Finger( gentity_t *ent, Arguments argv );
bool Help( gentity_t *ent, Arguments argv );
bool Kick( gentity_t *ent, Arguments argv );
bool LevelInfo( gentity_t *ent, Arguments argv );
bool ListBans( gentity_t *ent, Arguments argv );
bool ListCommands( gentity_t *ent, Arguments argv );
bool ListFlags( gentity_t *ent, Arguments argv );
bool ListMaps( gentity_t *ent, Arguments argv );
bool ListPlayers( gentity_t *ent, Arguments argv );
bool ListUsers( gentity_t *ent, Arguments argv );
bool Map( gentity_t *ent, Arguments argv );
bool MapInfo( gentity_t *ent, Arguments argv );
bool Mute( gentity_t *ent, Arguments argv );
bool Noclip( gentity_t *ent, Arguments argv );
bool NoGoto( gentity_t *ent, Arguments argv );
bool NoSave( gentity_t *ent, Arguments argv );
bool Passvote( gentity_t *ent, Arguments argv );
bool Putteam( gentity_t *ent, Arguments argv );
bool ReadConfig( gentity_t *ent, Arguments argv );
bool RemoveSaves( gentity_t *ent, Arguments argv );
bool Rename( gentity_t *ent, Arguments argv );
bool Restart( gentity_t *ent, Arguments argv );
bool SetLevel( gentity_t *ent, Arguments argv );
bool Spectate( gentity_t *ent, Arguments argv );
bool Unban( gentity_t *ent, Arguments argv );
bool Unmute( gentity_t *ent, Arguments argv );
bool UserInfo( gentity_t *ent, Arguments argv );

#endif // commands_h__
