#include <memory>

#include "g_local.h"
#include "etj_deathrun_system.h"
#include "etj_database.h"
#include "etj_session.h"
#include "etj_save_system.h"
#include "etj_printer.h"
#include "etj_string_utilities.h"
#include "etj_progression_tracker.h"
#include "etj_timerun_entities.h"
#include "etj_entity_utilities.h"
#include "etj_numeric_utilities.h"
#include "etj_rtv.h"

level_locals_t level;

typedef struct {
  vmCvar_t *vmCvar;
  const char *cvarName;
  const char *defaultString;
  int cvarFlags;
  int modificationCount; // for tracking changes
  qboolean trackChange;  // track this variable, and announce if changed
  qboolean fConfigReset; // OSP: set this var to the default on a config
                         // reset
  qboolean teamShader;   // track and if changed, update shader state
} cvarTable_t;

///////////////////////////////////////////////////////////////////////////////
// ETJump global systems
///////////////////////////////////////////////////////////////////////////////

namespace ETJump {
std::shared_ptr<DeathrunSystem> deathrunSystem;
std::shared_ptr<SaveSystem> saveSystem;
std::shared_ptr<Database> database;
std::shared_ptr<Session> session;
std::shared_ptr<ProgressionTrackers> progressionTrackers;
} // namespace ETJump

///////////////////////////////////////////////////////////////////////////////
// ETJump initialization
///////////////////////////////////////////////////////////////////////////////

static void initializeETJump() {
  ETJump::deathrunSystem = std::make_shared<ETJump::DeathrunSystem>();
  ETJump::database = std::make_shared<Database>();
  ETJump::session = std::make_shared<Session>(ETJump::database);
  ETJump::saveSystem = std::make_shared<ETJump::SaveSystem>(ETJump::session);
  ETJump::progressionTrackers = std::make_shared<ETJump::ProgressionTrackers>();
}

///////////////////////////////////////////////////////////////////////////////
// ETJump shutdown
///////////////////////////////////////////////////////////////////////////////

static void shutdownETJump() {
  ETJump::deathrunSystem = nullptr;
  ETJump::database = nullptr;
  ETJump::session = nullptr;
  ETJump::saveSystem = nullptr;
  ETJump::progressionTrackers = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

gentity_t g_entities[MAX_GENTITIES];
gclient_t g_clients[MAX_CLIENTS];

mapEntityData_Team_t mapEntityData[2];

vmCvar_t g_gametype;
vmCvar_t g_timelimit;
vmCvar_t g_password;
vmCvar_t sv_privatepassword;
vmCvar_t g_maxclients;
vmCvar_t g_maxGameClients;
vmCvar_t g_minGameClients; // NERVE - SMF
vmCvar_t g_dedicated;
vmCvar_t g_cheats;
vmCvar_t g_knockback;
vmCvar_t g_forcerespawn;
vmCvar_t g_inactivity;
vmCvar_t g_debugMove;
vmCvar_t g_debugDamage;
vmCvar_t g_debugAlloc;
vmCvar_t g_debugBullets; //----(SA)	added
vmCvar_t g_motd;
vmCvar_t g_warmup;

// NERVE - SMF
vmCvar_t g_userTimeLimit;
vmCvar_t g_userAlliedRespawnTime;
vmCvar_t g_userAxisRespawnTime;
vmCvar_t g_gamestate;
vmCvar_t g_swapteams;
// -NERVE - SMF

vmCvar_t g_restarted;
vmCvar_t g_logFile;
vmCvar_t g_logSync;
vmCvar_t g_podiumDist;
vmCvar_t g_podiumDrop;
vmCvar_t voteFlags;
vmCvar_t g_filtercams;
vmCvar_t g_voiceChatsAllowed; // DHM - Nerve
vmCvar_t g_fastres;           // Xian
vmCvar_t g_knifeonly;         // Xian

vmCvar_t g_needpass;
vmCvar_t g_doWarmup;
vmCvar_t g_banIPs;
vmCvar_t g_filterBan;
vmCvar_t g_smoothClients;
vmCvar_t pmove_msec;
vmCvar_t sv_fps;

// Rafael
vmCvar_t g_scriptName; // name of script file to run (instead of default for
                       // that map)

vmCvar_t g_developer;

vmCvar_t g_userAim;

// JPW NERVE multiplayer reinforcement times
vmCvar_t g_redlimbotime;
vmCvar_t g_bluelimbotime;
// charge times for character class special weapons
vmCvar_t g_medicChargeTime;
vmCvar_t g_engineerChargeTime;
vmCvar_t g_LTChargeTime;
vmCvar_t g_soldierChargeTime;
// screen shakey magnitude multiplier

// Gordon
vmCvar_t g_antilag;

// OSP
vmCvar_t g_spectatorInactivity;
vmCvar_t match_latejoin;
vmCvar_t match_minplayers;
vmCvar_t match_readypercent;
vmCvar_t match_timeoutcount;
vmCvar_t match_timeoutlength;
vmCvar_t match_warmupDamage;
vmCvar_t server_autoconfig;
vmCvar_t team_maxPanzers;
vmCvar_t team_maxplayers;
vmCvar_t team_nocontrols;
vmCvar_t server_motd0;
vmCvar_t server_motd1;
vmCvar_t server_motd2;
vmCvar_t server_motd3;
vmCvar_t server_motd4;
vmCvar_t server_motd5;
vmCvar_t vote_allow_map;
vmCvar_t vote_allow_matchreset;
vmCvar_t vote_allow_randommap;
vmCvar_t vote_allow_rtv;
vmCvar_t vote_allow_autoRtv;
vmCvar_t vote_limit;
vmCvar_t vote_percent;

vmCvar_t g_covertopsChargeTime;
vmCvar_t refereePassword;
vmCvar_t g_debugConstruct;
vmCvar_t g_landminetimeout;

// Variable for setting the current level of debug printing/logging
// enabled in bot scripts and regular scripts.
// Added by Mad Doctor I, 8/23/2002
vmCvar_t g_scriptDebugLevel;
vmCvar_t g_movespeed;

vmCvar_t mod_url;
vmCvar_t url;

vmCvar_t g_letterbox;
vmCvar_t bot_enable;

vmCvar_t g_debugSkills;
vmCvar_t g_autoFireteams;

vmCvar_t g_nextmap;

vmCvar_t g_dailyLogs;

// Trickjump cvars.
vmCvar_t g_save;
vmCvar_t g_floodprotection;
vmCvar_t g_floodlimit;
vmCvar_t g_floodwait;
vmCvar_t g_ghostPlayers;
vmCvar_t g_nofatigue;
vmCvar_t g_blockCheatCvars;
vmCvar_t g_weapons;
vmCvar_t g_noclip;
vmCvar_t g_mapScriptDir;
vmCvar_t g_blockedMaps;
vmCvar_t g_nameChangeLimit;
vmCvar_t g_nameChangeInterval;
vmCvar_t g_allowSpeclock;

// ETJump admin system

vmCvar_t g_userConfig;
vmCvar_t g_levelConfig;
vmCvar_t g_adminLog;

// Banner
vmCvar_t g_bannerLocation;
vmCvar_t g_bannerTime;
vmCvar_t g_banner1;
vmCvar_t g_banner2;
vmCvar_t g_banner3;
vmCvar_t g_banner4;
vmCvar_t g_banner5;
vmCvar_t g_banners;

// Feen: PGM
vmCvar_t g_portalDebug; // View Portal BBoxes
vmCvar_t
    g_portalMode; // Defines portal mode.
                  // 0 = Freestyle, 1 = Standard,  2 = team (possible future)

// Bugfixes
vmCvar_t g_maxConnsPerIP;

vmCvar_t g_mute;
vmCvar_t g_goto;
vmCvar_t g_voteCooldown;

vmCvar_t mod_version;

vmCvar_t g_mapDatabase;
vmCvar_t g_banDatabase;

vmCvar_t g_disableVoteAfterMapChange;

vmCvar_t g_motdFile;
vmCvar_t g_customMapVotesFile;

// Start of timeruns support
vmCvar_t g_timerunsDatabase;
vmCvar_t g_timeruns2Database;
// End of timeruns support

vmCvar_t g_chatOptions;

// tokens
vmCvar_t g_tokensMode;
vmCvar_t g_tokensPath;
// end of tokens

// vchat customization
vmCvar_t g_customVoiceChat;

// ETJump client/server shared data
// TODO: refactor ghostPlayers into this
vmCvar_t shared;

// minimum time to wait before vote result will be checked
vmCvar_t vote_minVoteDuration;
vmCvar_t g_moverScale;
vmCvar_t g_debugTrackers;
vmCvar_t g_debugTimeruns;
vmCvar_t g_spectatorVote;
vmCvar_t g_enableVote;

vmCvar_t g_autoRtv;
vmCvar_t g_rtvMapCount;
vmCvar_t vote_minRtvDuration;

// ETLegacy server browser integration
// os support - this SERVERINFO cvar specifies supported client operating
// systems on server
vmCvar_t g_oss; //   0  - vanilla/unknown/ET:L auto setup
                //   1  - Windows x86
                //   2  - Linux x86
                //   4  - Linux x86_64
                //   8  - macOS x86_64
                //  16  - Android aarch64
                //  32  - Raspberry Pi arm
                //  64  - Raspberry Pi aarch64
                //  128 - macOS aarch64 (M1)
                //  256 - Windows x86_64

cvarTable_t gameCvarTable[] = {
    // don't override the cheat state set by the system
    {&g_cheats, "sv_cheats", "", 0, qfalse},

    // noset vars
    {NULL, "gamename", GAME_NAME, CVAR_SERVERINFO | CVAR_ROM, 0, qfalse},
    {NULL, "gamedate", __DATE__, CVAR_ROM, 0, qfalse},
    {&g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse},
    {NULL, "mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse},

    // latched vars
    {&g_gametype, "g_gametype", "2", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse},

    // JPW NERVE multiplayer stuffs
    {&g_redlimbotime, "g_redlimbotime", "30000", CVAR_SERVERINFO | CVAR_LATCH,
     0, qfalse},
    {&g_bluelimbotime, "g_bluelimbotime", "30000", CVAR_SERVERINFO | CVAR_LATCH,
     0, qfalse},
    {&g_medicChargeTime, "g_medicChargeTime", "1", CVAR_LATCH, 0, qfalse,
     qtrue},
    {&g_engineerChargeTime, "g_engineerChargeTime", "1", CVAR_LATCH, 0, qfalse,
     qtrue},
    {&g_LTChargeTime, "g_LTChargeTime", "1", CVAR_LATCH, 0, qfalse, qtrue},
    {&g_soldierChargeTime, "g_soldierChargeTime", "1", CVAR_LATCH, 0, qfalse,
     qtrue},
    // jpw

    {&g_covertopsChargeTime, "g_covertopsChargeTime", "1", CVAR_LATCH, 0,
     qfalse, qtrue},
    {&g_landminetimeout, "g_landminetimeout", "1", CVAR_ARCHIVE, 0, qfalse,
     qtrue},

    {&g_maxclients, "sv_maxclients", "20",
     CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0,
     qfalse}, // NERVE - SMF - made 20 from 8
    {&g_maxGameClients, "g_maxGameClients", "0",
     CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse},
    {&g_minGameClients, "g_minGameClients", "8", CVAR_SERVERINFO, 0,
     qfalse}, // NERVE - SMF

    // change anytime vars
    {&g_timelimit, "timelimit", "0",
     CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue},

    {&g_warmup, "g_warmup", "60", CVAR_ARCHIVE, 0, qtrue},
    {&g_doWarmup, "g_doWarmup", "0", CVAR_ARCHIVE, 0, qtrue},

    {&g_gamestate, "gamestate", "-1", CVAR_WOLFINFO | CVAR_ROM, 0, qfalse},

    {&g_userTimeLimit, "g_userTimeLimit", "0", 0, 0, qfalse, qtrue},
    {&g_userAlliedRespawnTime, "g_userAlliedRespawnTime", "0", 0, 0, qfalse,
     qtrue},
    {&g_userAxisRespawnTime, "g_userAxisRespawnTime", "0", 0, 0, qfalse, qtrue},

    {&g_swapteams, "g_swapteams", "0", CVAR_ROM, 0, qfalse, qtrue},
    // -NERVE - SMF

    {&g_logFile, "g_log", "", CVAR_ARCHIVE, 0, qfalse},
    {&g_logSync, "g_logSync", "0", CVAR_ARCHIVE, 0, qfalse},

    {&g_password, "g_password", "none", CVAR_TEMP, 0, qfalse},
    {&sv_privatepassword, "sv_privatepassword", "", CVAR_TEMP, 0, qfalse},
    {&g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, qfalse},
    // https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=500
    {&g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, qfalse},

    {&g_dedicated, "dedicated", "0", 0, 0, qfalse},

    {&g_knockback, "g_knockback", "1000", 0, 0, qtrue, qtrue},

    {&g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, qtrue},
    {&g_forcerespawn, "g_forcerespawn", "0", 0, 0, qtrue},
    {&g_forcerespawn, "g_forcerespawn", "0", 0, 0, qtrue},
    {&g_inactivity, "g_inactivity", "0", 0, 0, qtrue},
    {&g_debugMove, "g_debugMove", "0", 0, 0, qfalse},
    {&g_debugDamage, "g_debugDamage", "0", CVAR_CHEAT, 0, qfalse},
    {&g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse},
    {&g_debugBullets, "g_debugBullets", "0", CVAR_CHEAT, 0,
     qfalse}, //----(SA)	added
    {&g_motd, "g_motd", "", CVAR_ARCHIVE, 0, qfalse},

    {&g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse},
    {&g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse},

    {&voteFlags, "voteFlags", "0", CVAR_TEMP | CVAR_ROM | CVAR_SERVERINFO, 0,
     qfalse},

    {&g_filtercams, "g_filtercams", "0", CVAR_ARCHIVE, 0, qfalse},
    {&g_voiceChatsAllowed, "g_voiceChatsAllowed", "4", CVAR_ARCHIVE, 0,
     qfalse}, // DHM - Nerve

    {&g_fastres, "g_fastres", "0", CVAR_ARCHIVE, 0, qtrue,
     qtrue},                                         // Xian - Fast Medic Resing
    {&g_knifeonly, "g_knifeonly", "0", 0, 0, qtrue}, // Xian - Fast Medic Resing

    {&g_developer, "developer", "0", CVAR_TEMP, 0, qfalse},
    {&g_userAim, "g_userAim", "1", CVAR_CHEAT, 0, qfalse},

    {&g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse},
    {&pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, qfalse},
    {&sv_fps, "sv_fps", "20", CVAR_SYSTEMINFO, 0, qfalse},

    {&g_scriptName, "g_scriptName", "", CVAR_CHEAT, 0, qfalse},

    {&g_antilag, "g_antilag", "1", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse},

    // bani - #184
    {NULL, "P", "", CVAR_SERVERINFO_NOUPDATE, 0, qfalse, qfalse},

    {&refereePassword, "refereePassword", "none", 0, 0, qfalse},
    {&g_spectatorInactivity, "g_spectatorInactivity", "0", 0, 0, qfalse,
     qfalse},
    {&match_latejoin, "match_latejoin", "1", 0, 0, qfalse, qfalse},
    {&match_minplayers, "match_minplayers", MATCH_MINPLAYERS, 0, 0, qfalse,
     qfalse},
    {&match_readypercent, "match_readypercent", "100", 0, 0, qfalse, qtrue},
    {&match_timeoutcount, "match_timeoutcount", "3", 0, 0, qfalse, qtrue},
    {&match_timeoutlength, "match_timeoutlength", "180", 0, 0, qfalse, qtrue},
    {&match_warmupDamage, "match_warmupDamage", "1", 0, 0, qfalse},
    {&server_autoconfig, "server_autoconfig", "0", 0, 0, qfalse, qfalse},
    {&server_motd0, "server_motd0", " ^NEnemy Territory ^7MOTD ", 0, 0, qfalse,
     qfalse},
    {&server_motd1, "server_motd1", "", 0, 0, qfalse, qfalse},
    {&server_motd2, "server_motd2", "", 0, 0, qfalse, qfalse},
    {&server_motd3, "server_motd3", "", 0, 0, qfalse, qfalse},
    {&server_motd4, "server_motd4", "", 0, 0, qfalse, qfalse},
    {&server_motd5, "server_motd5", "", 0, 0, qfalse, qfalse},
    {&team_maxPanzers, "team_maxPanzers", "-1", 0, 0, qfalse, qfalse},
    {&team_maxplayers, "team_maxplayers", "0", 0, 0, qfalse, qfalse},
    {&team_nocontrols, "team_nocontrols", "1", 0, 0, qfalse, qfalse},
    {&vote_allow_map, "vote_allow_map", "1", 0, 0, qfalse, qfalse},
    {&vote_allow_matchreset, "vote_allow_matchreset", "1", 0, 0, qfalse,
     qfalse},
    {&vote_allow_randommap, "vote_allow_randommap", "1", 0, 0, qfalse, qfalse},
    {&vote_allow_rtv, "vote_allow_rtv", "1", 0, 0, qfalse, qfalse},
    {&vote_allow_autoRtv, "vote_allow_autoRtv", "1", 0, 0, qfalse, qfalse},
    {&vote_limit, "vote_limit", "5", 0, 0, qfalse, qfalse},
    {&vote_percent, "vote_percent", "50", 0, 0, qfalse, qfalse},

    {&g_debugConstruct, "g_debugConstruct", "0", CVAR_CHEAT, 0, qfalse},

    {&g_scriptDebug, "g_scriptDebug", "0", CVAR_CHEAT, 0, qfalse},

    // What level of detail do we want script printing to go to.
    {&g_scriptDebugLevel, "g_scriptDebugLevel", "0", CVAR_CHEAT, 0, qfalse},

    // How fast do we want Allied single player movement?
    //	{ &g_movespeed, "g_movespeed", "127", CVAR_CHEAT, 0, qfalse },
    {&g_movespeed, "g_movespeed", "76", CVAR_CHEAT, 0, qfalse},

    // points to the URL for mod information, should not be modified by server
    // admin
    {&mod_url, "mod_url", GAME_URL, CVAR_SERVERINFO | CVAR_ROM, 0, qfalse},
    // configured by the server admin, points to the web pages for the server
    {&url, "URL", "", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse},

    {&g_letterbox, "cg_letterbox", "0", CVAR_TEMP},
    {&bot_enable, "bot_enable", "0", 0},

    {&g_debugSkills, "g_debugSkills", "0", 0},
    {&g_autoFireteams, "g_autoFireteams", "1", CVAR_ARCHIVE},

    {&g_nextmap, "nextmap", "", CVAR_TEMP},

    {&g_dailyLogs, "g_dailyLogs", "1", CVAR_ARCHIVE},

    // Trickjump cvars

    {&g_save, "g_save", "1", CVAR_ARCHIVE},

    {&g_floodprotection, "g_floodprotection", "1", CVAR_ARCHIVE},
    {&g_floodlimit, "g_floodlimit", "5", CVAR_ARCHIVE},
    {&g_floodwait, "g_floodwait", "768", CVAR_ARCHIVE},

    {&g_ghostPlayers, "g_ghostPlayers", "1", CVAR_SERVERINFO | CVAR_LATCH},
    {&g_nofatigue, "g_nofatigue", "1", CVAR_ARCHIVE},
    {&g_blockCheatCvars, "g_blockCheatCvars", "0", CVAR_ARCHIVE},
    {&g_weapons, "g_weapons", "1", CVAR_ARCHIVE},
    {&g_noclip, "g_noclip", "0", CVAR_ARCHIVE},
    {&g_nameChangeLimit, "g_nameChangeLimit", "5", CVAR_ARCHIVE},
    {&g_nameChangeInterval, "g_nameChangeInterval", "60", CVAR_ARCHIVE},
    {&g_mapScriptDir, "g_mapScriptDir", "mapscripts", CVAR_ARCHIVE},
    {&g_blockedMaps, "g_blockedMaps", "", CVAR_ARCHIVE},
    {&g_allowSpeclock, "g_allowSpeclock", "1", CVAR_ARCHIVE, 0, qtrue},

    {&g_adminLog, "g_adminLog", "adminsystem.log", CVAR_ARCHIVE},
    {&g_userConfig, "g_userConfig", "users.db", CVAR_ARCHIVE},
    {&g_levelConfig, "g_levelConfig", "levels.cfg", CVAR_ARCHIVE},

    // BannerPrint location
    {&g_bannerLocation, "g_bannerLocation", "1", CVAR_ARCHIVE},
    {&g_bannerTime, "g_bannerTime", "60000", CVAR_ARCHIVE},
    // FIXME: dynamic banner count.
    {&g_banner1, "g_banner1", "ETJump", CVAR_ARCHIVE},
    {&g_banner2, "g_banner2",
     "A Wolfenstein: Enemy Territory Trickjump Modification", CVAR_ARCHIVE},
    {&g_banner3, "g_banner3", "www.etjump.com", CVAR_ARCHIVE},
    {&g_banner4, "g_banner4", "Developed by Zero, Feengur and Setup",
     CVAR_ARCHIVE},
    {&g_banner5, "g_banner5", "Thanks for choosing ETJump!", CVAR_ARCHIVE},
    {&g_banners, "g_banners", "1", CVAR_ARCHIVE},

    // Feen: PGM
    {&g_portalDebug, "g_portalDebug", "0", CVAR_CHEAT | CVAR_ARCHIVE},
    {&g_portalMode, "g_portalMode", "1",
     CVAR_ARCHIVE}, // 0 - freestyle, 1 - restricted

    {&g_maxConnsPerIP, "g_maxConnsPerIP", "2", CVAR_ARCHIVE},
    {&g_mute, "g_mute", "0", CVAR_ARCHIVE},
    {&g_goto, "g_goto", "1", CVAR_ARCHIVE},
    {&g_voteCooldown, "g_voteCooldown", "15", CVAR_ARCHIVE},
    {&mod_version, "mod_version", GAME_VERSION, CVAR_SERVERINFO},

    {&g_mapDatabase, "g_mapDatabase", "maps.dat", CVAR_ARCHIVE},
    {&g_banDatabase, "g_banDatabase", "bans.dat", CVAR_ARCHIVE},
    {&g_disableVoteAfterMapChange, "g_disableVoteAfterMapChange", "30000",
     CVAR_ARCHIVE},
    {&g_motdFile, "g_motdFile", "motd.json", CVAR_ARCHIVE},
    {&g_customMapVotesFile, "g_customMapVotesFile", "customvotes.json",
     CVAR_ARCHIVE},

    // Start of timeruns support
    {&g_timerunsDatabase, "g_timerunsDatabase", "timeruns.db", CVAR_ARCHIVE},
    {&g_timeruns2Database, "g_timeruns2Database", "timeruns.v2.db",
     CVAR_ARCHIVE},
    // End of timeruns support

    {&g_chatOptions, "g_chatOptions", "1", CVAR_ARCHIVE},

    // tokens
    {&g_tokensMode, "g_tokensMode", "1", CVAR_ARCHIVE | CVAR_LATCH},
    {&g_tokensPath, "g_tokensPath", "tokens", CVAR_ARCHIVE | CVAR_LATCH},
    // end of tokens

    {&g_customVoiceChat, "g_customVoiceChat", "1", CVAR_ARCHIVE},

    {&shared, "shared", "0", CVAR_SYSTEMINFO | CVAR_ROM},
    {&vote_minVoteDuration, "vote_minVoteDuration", "5000", CVAR_ARCHIVE},
    {&g_moverScale, "g_moverScale", "1.0", 0},
    {&g_debugTrackers, "g_debugTrackers", "0", CVAR_ARCHIVE | CVAR_LATCH},
    {&g_debugTimeruns, "g_debugTimeruns", "0", CVAR_ARCHIVE | CVAR_LATCH},
    {&g_spectatorVote, "g_spectatorVote", "0", CVAR_ARCHIVE | CVAR_SERVERINFO},
    {&g_enableVote, "g_enableVote", "1", CVAR_ARCHIVE},
    {&g_oss, "g_oss", "399", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse, qfalse},

    {&g_autoRtv, "g_autoRtv", "0", CVAR_ARCHIVE | CVAR_SERVERINFO},
    {&vote_minRtvDuration, "vote_minRtvDuration", "15000", CVAR_ARCHIVE},
    {&g_rtvMapCount, "g_rtvMapCount", "5", CVAR_ARCHIVE},
};

// bk001129 - made static to avoid aliasing
static int gameCvarTableSize = sizeof(gameCvarTable) / sizeof(gameCvarTable[0]);

void G_InitGame(int levelTime, int randomSeed, int restart);
void G_RunFrame(int levelTime);
void G_ShutdownGame(int restart);
void CheckExitRules(void);

qboolean G_SnapshotCallback(int entityNum, int clientNum) {
  gentity_t *ent = &g_entities[entityNum];

  if (ent->s.eType == ET_MISSILE) {
    if (ent->s.weapon == WP_LANDMINE) {
      return G_LandmineSnapshotCallback(entityNum, clientNum);
    }
  }

  return qtrue;
}

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/

extern "C" FN_PUBLIC intptr_t vmMain(int command, intptr_t arg0, intptr_t arg1,
                                     intptr_t arg2, intptr_t arg3,
                                     intptr_t arg4, intptr_t arg5,
                                     intptr_t arg6) {
  switch (command) {
    case GAME_INIT:
      G_InitGame(arg0, arg1, arg2);
      return 0;
    case GAME_SHUTDOWN:
      G_ShutdownGame(arg0);
      return 0;
    case GAME_CLIENT_CONNECT:
      return (intptr_t)ClientConnect(arg0, arg1 ? qtrue : qfalse,
                                     arg2 ? qtrue : qfalse);
    case GAME_CLIENT_THINK:
      ClientThink(arg0);
      return 0;
    case GAME_CLIENT_USERINFO_CHANGED:
      ClientUserinfoChanged(arg0);
      return 0;
    case GAME_CLIENT_DISCONNECT:
      ClientDisconnect(arg0);
      return 0;
    case GAME_CLIENT_BEGIN:
      ClientBegin(arg0);
      return 0;
    case GAME_CLIENT_COMMAND:
      ClientCommand(arg0);
      return 0;
    case GAME_RUN_FRAME:
      G_RunFrame(arg0);
      return 0;
    case GAME_CONSOLE_COMMAND:
      return ConsoleCommand();
    case BOTAI_START_FRAME:
      return 0;
    case BOT_VISIBLEFROMPOS:
      return qfalse;
    case BOT_CHECKATTACKATPOS:
      return qfalse;
    case GAME_SNAPSHOT_CALLBACK:
      return G_SnapshotCallback(arg0, arg1);
    case GAME_MESSAGERECEIVED:
      return -1;
  }

  return -1;
}

void QDECL G_Printf(const char *fmt, ...) {
  va_list argptr;
  char text[1024];

  va_start(argptr, fmt);
  Q_vsnprintf(text, sizeof(text), fmt, argptr);
  va_end(argptr);

  trap_Printf(text);
}
// bani
void QDECL G_Printf(const char *fmt, ...);

void QDECL G_DPrintf(const char *fmt, ...) {
  va_list argptr;
  char text[1024];

  if (!g_developer.integer) {
    return;
  }

  va_start(argptr, fmt);
  Q_vsnprintf(text, sizeof(text), fmt, argptr);
  va_end(argptr);

  trap_Printf(text);
}
// bani
void QDECL G_DPrintf(const char *fmt, ...);

void QDECL G_Error(const char *fmt, ...) {
  va_list argptr;
  char text[1024];

  va_start(argptr, fmt);
  Q_vsnprintf(text, sizeof(text), fmt, argptr);
  va_end(argptr);

  trap_Error(text);
}
// bani
void QDECL G_Error(const char *fmt, ...);

#define CH_KNIFE_DIST 48 // from g_weapon.c
#define CH_LADDER_DIST 100
#define CH_WATER_DIST 100
#define CH_BREAKABLE_DIST 64
#define CH_DOOR_DIST 96
#define CH_ACTIVATE_DIST 96
#define CH_EXIT_DIST 256
#define CH_FRIENDLY_DIST 1024

#define CH_MAX_DIST 1024      // use the largest value from above
#define CH_MAX_DIST_ZOOM 8192 // max dist for zooming hints

/*
==============
G_CursorHintIgnoreEnt: returns whether the ent should be ignored
for cursor hint purpose (because the ent may have the designed content type
but nevertheless should not display any cursor hint)
==============
*/
static qboolean G_CursorHintIgnoreEnt(gentity_t *traceEnt,
                                      gentity_t *clientEnt) {
  return (traceEnt->s.eType == ET_OID_TRIGGER ||
          traceEnt->s.eType == ET_TRIGGER_MULTIPLE)
             ? qtrue
             : qfalse;
}

/*
==============
G_CheckForCursorHints
    non-AI's check for cursor hint contacts

    server-side because there's info we want to show that the client
    just doesn't know about.  (health or other info of an
explosive,invisible_users,items,etc.)

    traceEnt is the ent hit by the trace, checkEnt is the ent that is being
    checked against (in case the traceent was an invisible_user or something)

==============
*/

qboolean G_EmplacedGunIsMountable(gentity_t *ent, gentity_t *other) {
  if (Q_stricmp(ent->classname, "misc_mg42") &&
      Q_stricmp(ent->classname, "misc_aagun")) {
    return qfalse;
  }

  if (!other->client) {
    return qfalse;
  }

  if (BG_IsScopedWeapon(other->client->ps.weapon)) {
    return qfalse;
  }

  if (other->client->ps.pm_flags & PMF_DUCKED) {
    return qfalse;
  }

  if (other->client->ps.persistant[PERS_HWEAPON_USE]) {
    return qfalse;
  }

  if (ent->r.currentOrigin[2] - other->r.currentOrigin[2] >= 40) {
    return qfalse;
  }

  if (ent->r.currentOrigin[2] - other->r.currentOrigin[2] < 0) {
    return qfalse;
  }

  if (ent->s.frame != 0) {
    return qfalse;
  }

  if (ent->active) {
    return qfalse;
  }

  if (other->client->ps.grenadeTimeLeft) {
    return qfalse;
  }

  if (infront(ent, other)) {
    return qfalse;
  }

  return qtrue;
}

qboolean G_EmplacedGunIsRepairable(gentity_t *ent, gentity_t *other) {
  if (Q_stricmp(ent->classname, "misc_mg42") &&
      Q_stricmp(ent->classname, "misc_aagun")) {
    return qfalse;
  }

  if (!other->client) {
    return qfalse;
  }

  if (BG_IsScopedWeapon(other->client->ps.weapon)) {
    return qfalse;
  }

  if (other->client->ps.persistant[PERS_HWEAPON_USE]) {
    return qfalse;
  }

  if (ent->s.frame == 0) {
    return qfalse;
  }

  return qtrue;
}

void G_CheckForCursorHints(gentity_t *ent) {
  vec3_t forward, right, up, offset, end;
  trace_t *tr;
  float dist;
  float chMaxDist = CH_MAX_DIST;
  gentity_t *checkEnt, *traceEnt = 0;
  playerState_t *ps;
  int hintType, hintDist, hintVal;
  qboolean zooming;   // indirectHit means the checkent was not the ent
                      // hit by the trace (checkEnt!=traceEnt)
  int trace_contents; // DHM - Nerve
  int numOfIgnoredEnts = 0;

  if (!ent->client) {
    return;
  }

  ps = &ent->client->ps;

  zooming = (qboolean)(ps->eFlags & EF_ZOOMING);

  AngleVectors(ps->viewangles, forward, right, up);

  VectorCopy(ps->origin, offset);
  offset[2] += ps->viewheight;

  // lean
  if (ps->leanf) {
    VectorMA(offset, ps->leanf, right, offset);
  }

  if (zooming) {
    VectorMA(offset, CH_MAX_DIST_ZOOM, forward, end);
  } else {
    VectorMA(offset, chMaxDist, forward, end);
  }

  tr = &ps->serverCursorHintTrace;

  trace_contents = (CONTENTS_TRIGGER | CONTENTS_SOLID | CONTENTS_MISSILECLIP |
                    CONTENTS_BODY | CONTENTS_CORPSE);
  trap_Trace(tr, offset, NULL, NULL, end, ps->clientNum, trace_contents);

  // reset all
  hintType = ps->serverCursorHint = HINT_NONE;
  hintVal = ps->serverCursorHintVal = 0;

  dist = VectorDistanceSquared(offset, tr->endpos);

  if (zooming) {
    hintDist = CH_MAX_DIST_ZOOM;
  } else {
    hintDist = chMaxDist;
  }

  // Arnout: building something - add this here because we don't have
  // anything solid to trace to - quite ugly-ish
  if (ent->client->touchingTOI && ps->stats[STAT_PLAYER_CLASS] == PC_ENGINEER) {
    gentity_t *constructible = G_IsConstructible(ent->client->sess.sessionTeam,
                                                 ent->client->touchingTOI);
    if (constructible) {
      ps->serverCursorHint = HINT_CONSTRUCTIBLE;
      ps->serverCursorHintVal = (int)constructible->s.angles2[0];
      return;
    }
  }

  if (ps->stats[STAT_PLAYER_CLASS] == PC_COVERTOPS) {
    if (ent->client->landmineSpottedTime &&
        level.time - ent->client->landmineSpottedTime < 500) {
      ps->serverCursorHint = HINT_LANDMINE;
      ps->serverCursorHintVal = ent->client->landmineSpotted
                                    ? ent->client->landmineSpotted->count2
                                    : 0;
      return;
    }
  }

  if (tr->fraction == 1) {
    return;
  }

  traceEnt = &g_entities[tr->entityNum];
  while (G_CursorHintIgnoreEnt(traceEnt, ent) && numOfIgnoredEnts < 10) {
    // xkan, 1/9/2003 - we may hit multiple invalid ents at the
    // same point count them to prevent too many loops
    numOfIgnoredEnts++;

    // xkan, 1/8/2003 - advance offset (start point) past the
    // entity to ignore
    VectorMA(tr->endpos, 0.1, forward, offset);

    trap_Trace(tr, offset, NULL, NULL, end, traceEnt->s.number, trace_contents);

    // xkan, 1/8/2003 - (hintDist - dist) is the actual distance
    // in the above trap_Trace call. update dist accordingly.
    dist += VectorDistanceSquared(offset, tr->endpos);
    if (tr->fraction == 1) {
      return;
    }
    traceEnt = &g_entities[tr->entityNum];
  }

  if (tr->entityNum == ENTITYNUM_WORLD) {
    if ((tr->contents & CONTENTS_WATER) && !(ps->powerups[PW_BREATHER])) {
      hintDist = CH_WATER_DIST;
      hintType = HINT_WATER;
    } else if ((tr->surfaceFlags & SURF_LADDER) &&
               !(ps->pm_flags & PMF_LADDER)) // ladder
    {
      hintDist = CH_LADDER_DIST;
      hintType = HINT_LADDER;
    }
  } else if (tr->entityNum < MAX_CLIENTS) {
    // Show medics a syringe if they can revive someone

    if (traceEnt->client &&
        traceEnt->client->sess.sessionTeam == ent->client->sess.sessionTeam) {
      if (ps->stats[STAT_PLAYER_CLASS] == PC_MEDIC &&
          traceEnt->client->ps.pm_type == PM_DEAD &&
          !(traceEnt->client->ps.pm_flags & PMF_LIMBO)) {
        hintDist = 48; // JPW NERVE matches
                       // weapon_syringe in g_weapon.c
        hintType = HINT_REVIVE;
      }
    } else if (traceEnt->client && traceEnt->client->isCivilian) {
      // xkan, 1/6/2003 - check for civilian, show
      // neutral cursor (no matter which team)
      hintType = HINT_PLYR_NEUTRAL;
      hintDist = CH_FRIENDLY_DIST; // far, since this will be
                                   // used to determine whether
                                   // to shoot bullet weaps or
                                   // not
    }
  } else {
    checkEnt = traceEnt;

    // Arnout: invisible entities don't show hints
    if (traceEnt->entstate == STATE_INVISIBLE ||
        traceEnt->entstate == STATE_UNDERCONSTRUCTION) {
      return;
    }

    // check invisible_users first since you don't want to draw
    // a hint based on that ent, but rather on what they are
    // targeting. so find the target and set checkEnt to that to
    // show the proper hint.
    if (traceEnt->s.eType == ET_GENERAL) {

      // ignore trigger_aidoor.  can't just not trace
      // for triggers, since I need invisible_users...
      // damn, I would like to ignore some of these
      // triggers though.

      if (!Q_stricmp(traceEnt->classname, "trigger_aidoor")) {
        return;
      }

      if (!Q_stricmp(traceEnt->classname, "func_invisible_user")) {
        // DHM - Nerve :: Put this back in only
        // in multiplayer
        if (traceEnt->s.dmgFlags) // hint icon
                                  // specified
                                  // in entity
        {
          hintType = traceEnt->s.dmgFlags;
          hintDist = CH_ACTIVATE_DIST;
          checkEnt = 0;
        } else // use target for hint icon
        {
          checkEnt = G_FindByTargetname(NULL, traceEnt->target);
          if (!checkEnt) // no target
                         // found
          {
            hintType = HINT_BAD_USER;
            hintDist =
                CH_MAX_DIST_ZOOM; // show this one from super far for debugging
          }
        }
      }
    }

    if (checkEnt) {

      // TDF This entire function could be the poster
      // boy for converting to OO programming!!! I'm
      // making this into a switch in a vain attempt to
      // make this readable so I can find which brackets
      // don't match!!!

      switch (checkEnt->s.eType) {
        case ET_CORPSE:
          if (!ent->client->ps.powerups[PW_BLUEFLAG] &&
              !ent->client->ps.powerups[PW_REDFLAG] &&
              !ent->client->ps.powerups[PW_OPS_DISGUISED]) {
            if (BODY_TEAM(traceEnt) < 4 &&
                BODY_TEAM(traceEnt) != ent->client->sess.sessionTeam &&
                traceEnt->nextthink == traceEnt->timestamp + BODY_TIME) {
              if (ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_COVERTOPS) {
                hintDist = 48;
                hintType = HINT_UNIFORM;
                hintVal = BODY_VALUE(traceEnt);
                if (hintVal > 255) {
                  hintVal = 255;
                }
              }
            }
          }
          break;
        case ET_GENERAL:
        case ET_MG42_BARREL:
        case ET_AAGUN:
          hintType = HINT_FORCENONE;

          if (G_EmplacedGunIsMountable(traceEnt, ent)) {
            hintDist = CH_ACTIVATE_DIST;
            hintType = HINT_MG42;
            hintVal = 0;
          } else {
            if (ps->stats[STAT_PLAYER_CLASS] == PC_ENGINEER &&
                G_EmplacedGunIsRepairable(traceEnt, ent)) {
              hintType = HINT_BUILD;
              hintDist = CH_BREAKABLE_DIST;
              hintVal = traceEnt->health;
              if (hintVal > 255) {
                hintVal = 255;
              }
            } else {
              hintDist = 0;
              hintType = ps->serverCursorHint = HINT_FORCENONE;
              hintVal = ps->serverCursorHintVal = 0;
            }
          }
          break;
        case ET_EXPLOSIVE: {
          if (checkEnt->spawnflags & EXPLOSIVE_TANK) {
            hintDist = CH_BREAKABLE_DIST * 2;
            hintType = HINT_TANK;
            hintVal = ps->serverCursorHintVal = 0; // no
                                                   // health
                                                   // for
                                                   // tank
                                                   // destructibles
          } else {
            switch (checkEnt->constructibleStats.weaponclass) {
              case 0:
                hintDist = CH_BREAKABLE_DIST;
                hintType = HINT_BREAKABLE;
                hintVal = checkEnt->health; // also send health to client for
                                            // visualization
                break;
              case 1:
                hintDist = CH_BREAKABLE_DIST * 2;
                hintType = HINT_SATCHELCHARGE;
                hintVal = ps->serverCursorHintVal =
                    0; // no health for satchel charges
                break;
              case 2:
                hintDist = 0;
                hintType = ps->serverCursorHint = HINT_FORCENONE;
                hintVal = ps->serverCursorHintVal = 0;

                if (checkEnt->parent &&
                    checkEnt->parent->s.eType == ET_OID_TRIGGER) {
                  if (((ent->client->sess.sessionTeam == TEAM_AXIS) &&
                       (checkEnt->parent->spawnflags & ALLIED_OBJECTIVE)) ||
                      ((ent->client->sess.sessionTeam == TEAM_ALLIES) &&
                       (checkEnt->parent->spawnflags & AXIS_OBJECTIVE))) {
                    hintDist = CH_BREAKABLE_DIST * 2;
                    hintType = HINT_BREAKABLE_DYNAMITE;
                    hintVal = ps->serverCursorHintVal =
                        0; // no health for dynamite
                  }
                }
                break;
              default:
                if (checkEnt->health > 0) {
                  hintDist = CH_BREAKABLE_DIST;
                  hintType = HINT_BREAKABLE;
                  hintVal = checkEnt->health; // also send health to client for
                                              // visualization
                } else {
                  hintDist = 0;
                  hintType = ps->serverCursorHint = HINT_FORCENONE;
                  hintVal = ps->serverCursorHintVal = 0;
                }
                break;
            }
          }

          break;
        }
        case ET_CONSTRUCTIBLE:
          if (G_ConstructionIsPartlyBuilt(checkEnt) &&
              !(checkEnt->spawnflags & CONSTRUCTIBLE_INVULNERABLE)) {
            // only show hint
            // for players who
            // can blow it up
            if (checkEnt->s.teamNum != ent->client->sess.sessionTeam) {
              switch (checkEnt->constructibleStats.weaponclass) {
                case 0:
                  hintDist = CH_BREAKABLE_DIST;
                  hintType = HINT_BREAKABLE;
                  hintVal = checkEnt->health; // also send health to client for
                                              // visualization
                  break;
                case 1:
                  hintDist = CH_BREAKABLE_DIST * 2;
                  hintType = HINT_SATCHELCHARGE;
                  hintVal = ps->serverCursorHintVal =
                      0; // no health for satchel charges
                  break;
                case 2:
                  hintDist = CH_BREAKABLE_DIST * 2;
                  hintType = HINT_BREAKABLE_DYNAMITE;
                  hintVal = ps->serverCursorHintVal =
                      0; // no health for dynamite
                  break;
                default:
                  hintDist = 0;
                  hintType = ps->serverCursorHint = HINT_FORCENONE;
                  hintVal = ps->serverCursorHintVal = 0;
                  break;
              }
            } else {
              hintDist = 0;
              hintType = ps->serverCursorHint = HINT_FORCENONE;
              hintVal = ps->serverCursorHintVal = 0;
              return;
            }
          }

          break;
        case ET_ALARMBOX:
          if (checkEnt->health > 0) {
            hintType = HINT_ACTIVATE;
          }
          break;

        case ET_ITEM: {
          gitem_t *it = &bg_itemlist[checkEnt->item - bg_itemlist];

          hintDist = CH_ACTIVATE_DIST;

          switch (it->giType) {
            case IT_HEALTH:
              hintType = HINT_HEALTH;
              break;
            case IT_TREASURE:
              hintType = HINT_TREASURE;
              break;
            case IT_WEAPON: {
              qboolean canPickup =
                  COM_BitCheck(ent->client->ps.weapons, it->giTag);

              if (!canPickup) {
                if (it->giTag == WP_AMMO) {
                  canPickup = qtrue;
                }
              }

              if (!canPickup) {
                canPickup =
                    G_CanPickupWeapon(static_cast<weapon_t>(it->giTag), ent);
              }

              if (canPickup) {
                hintType = HINT_WEAPON;
              }
              break;
            }
            case IT_AMMO:
              hintType = HINT_AMMO;
              break;
            case IT_ARMOR:
              hintType = HINT_ARMOR;
              break;
            case IT_HOLDABLE:
              hintType = HINT_HOLDABLE;
              break;
            case IT_KEY:
              hintType = HINT_INVENTORY;
              break;
            case IT_TEAM:
              if (!Q_stricmp(traceEnt->classname, "team_CTF_redflag") &&
                  ent->client->sess.sessionTeam == TEAM_ALLIES) {
                hintType = HINT_POWERUP;
              } else if (!Q_stricmp(traceEnt->classname, "team_CTF_blueflag") &&
                         ent->client->sess.sessionTeam == TEAM_AXIS) {
                hintType = HINT_POWERUP;
              }
              break;
            case IT_BAD:
            default:
              break;
          }

          break;
        }
        case ET_MOVER:
          if (!Q_stricmp(checkEnt->classname, "script_mover")) {
            if (G_TankIsMountable(checkEnt, ent)) {
              hintDist = CH_ACTIVATE_DIST;
              hintType = HINT_ACTIVATE;
            }
          } else if (!Q_stricmp(checkEnt->classname, "func_door_"
                                                     "rotating")) {
            if (checkEnt->moverState == MOVER_POS1ROTATE) // stationary/closed
            {
              hintDist = CH_DOOR_DIST;
              hintType = HINT_DOOR_ROTATING;
              if (checkEnt->key == -1 || !G_AllowTeamsAllowed(checkEnt,
                                                              ent)) // locked
              {
                hintType = HINT_DOOR_ROTATING_LOCKED;
              }
            }
          } else if (!Q_stricmp(checkEnt->classname, "func_door")) {
            if (checkEnt->moverState == MOVER_POS1) // stationary/closed
            {
              hintDist = CH_DOOR_DIST;
              hintType = HINT_DOOR;

              if (checkEnt->key == -1 || !G_AllowTeamsAllowed(checkEnt,
                                                              ent)) // locked
              {
                hintType = HINT_DOOR_LOCKED;
              }
            }
          } else if (!Q_stricmp(checkEnt->classname, "func_button")) {
            hintDist = CH_ACTIVATE_DIST;
            hintType = HINT_BUTTON;
          } else if (!Q_stricmp(checkEnt->classname, "props_"
                                                     "flamebarrel")) {
            hintDist = CH_BREAKABLE_DIST * 2;
            hintType = HINT_BREAKABLE;
          } else if (!Q_stricmp(checkEnt->classname, "props_statue")) {
            hintDist = CH_BREAKABLE_DIST * 2;
            hintType = HINT_BREAKABLE;
          }

          break;
        case ET_MISSILE:
        case ET_BOMB:
          if (ps->stats[STAT_PLAYER_CLASS] == PC_ENGINEER) {
            hintDist = CH_BREAKABLE_DIST;
            hintType = HINT_DISARM;
            hintVal =
                checkEnt
                    ->health; // also send health to client for visualization
            if (hintVal > 255) {
              hintVal = 255;
            }
          }

          // hint icon specified in
          // entity (and proper contact
          // was made, so hintType was
          // set) first try the
          // checkent...
          if (checkEnt->s.dmgFlags && hintType) {
            hintType = checkEnt->s.dmgFlags;
          }

          // then the traceent
          if (traceEnt->s.dmgFlags && hintType) {
            hintType = traceEnt->s.dmgFlags;
          }

          break;
        default:
          break;
      }

      if (zooming) {
        hintDist = CH_MAX_DIST_ZOOM;

        // zooming can eat a lot of potential
        // hints
        switch (hintType) {

          // allow while zooming
          case HINT_PLAYER:
          case HINT_TREASURE:
          case HINT_LADDER:
          case HINT_EXIT:
          case HINT_NOEXIT:
          case HINT_PLYR_FRIEND:
          case HINT_PLYR_NEUTRAL:
          case HINT_PLYR_ENEMY:
          case HINT_PLYR_UNKNOWN:
            break;

          default:
            return;
        }
      }
    }
  }

  if (dist <= Square(hintDist)) {
    ps->serverCursorHint = hintType;
    ps->serverCursorHintVal = hintVal;
  }
}

void G_SetTargetName(gentity_t *ent, char *targetname) {
  if (targetname && *targetname) {
    ent->targetname = targetname;
    ent->targetnamehash = BG_StringHashValue(targetname);
  } else {
    ent->targetnamehash = -1;
  }
}

/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams(void) {
  gentity_t *e, *e2;
  int i, j;
  int c, c2;

  c = 0;
  c2 = 0;
  for (i = 1, e = g_entities + i; i < level.num_entities; i++, e++) {
    if (!e->inuse) {
      continue;
    }

    if (!e->team) {
      continue;
    }

    if (e->flags & FL_TEAMSLAVE) {
      continue;
    }

    if (!Q_stricmp(e->classname, "func_tramcar")) {
      if (e->spawnflags & 8) // leader
      {
        e->teammaster = e;
      } else {
        continue;
      }
    }

    c++;
    c2++;
    for (j = i + 1, e2 = e + 1; j < level.num_entities; j++, e2++) {
      if (!e2->inuse) {
        continue;
      }
      if (!e2->team) {
        continue;
      }
      if (e2->flags & FL_TEAMSLAVE) {
        continue;
      }
      if (!strcmp(e->team, e2->team)) {
        c2++;
        e2->teamchain = e->teamchain;
        e->teamchain = e2;
        e2->teammaster = e;
        //				e2->key =
        // e->key;	// (SA) I can't set the
        // key here since the master door hasn't
        // finished spawning yet
        // and
        // therefore has a key of -1
        e2->flags |= FL_TEAMSLAVE;

        if (!Q_stricmp(e2->classname, "func_tramcar")) {
          trap_UnlinkEntity(e2);
        }

        // make sure that targets only point at
        // the master
        if (e2->targetname) {
          G_SetTargetName(e, e2->targetname);

          // Rafael
          // note to self: added this
          // because of problems
          // pertaining to keys and
          // double doors
          if (Q_stricmp(e2->classname, "func_door_"
                                       "rotating")) {
            e2->targetname = NULL;
          }
        }
      }
    }
  }

  G_Printf("%i teams with %i entities\n", c, c2);
}

/*
==============
G_RemapTeamShaders
==============
*/
void G_RemapTeamShaders() {}

/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars(void) {
  int i;
  cvarTable_t *cv;
  qboolean remapped = qfalse;

  level.server_settings = 0;

  for (i = 0, cv = gameCvarTable; i < gameCvarTableSize; i++, cv++) {
    trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString,
                       cv->cvarFlags);
    if (cv->vmCvar) {
      cv->modificationCount = cv->vmCvar->modificationCount;
      // OSP - Update vote info for clients, if necessary
      G_checkServerToggle(cv->vmCvar);
    }

    remapped = (remapped || cv->teamShader) ? qtrue : qfalse;
  }

  if (remapped) {
    G_RemapTeamShaders();
  }

  // Zero: make sure gametype is always 2
  if (g_gametype.integer != ETJUMP_GAMETYPE) {
    trap_Cvar_Set("g_gametype", va("%i", ETJUMP_GAMETYPE));
    trap_Cvar_Update(&g_gametype);
  }

  // OSP
  trap_SetConfigstring(CS_SERVERTOGGLES, va("%d", level.server_settings));
  if (match_readypercent.integer < 1) {
    trap_Cvar_Set("match_readypercent", "1");
  }

  if (pmove_msec.integer < 8) {
    trap_Cvar_Set("pmove_msec", "8");
  } else if (pmove_msec.integer > 33) {
    trap_Cvar_Set("pmove_msec", "33");
  }
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars(void) {
  int i;
  cvarTable_t *cv;
  qboolean fToggles = qfalse;
  qboolean fVoteFlags = qfalse;
  qboolean remapped = qfalse;
  qboolean chargetimechanged = qfalse;

  for (i = 0, cv = gameCvarTable; i < gameCvarTableSize; i++, cv++) {
    if (cv->vmCvar) {
      trap_Cvar_Update(cv->vmCvar);

      if (cv->modificationCount != cv->vmCvar->modificationCount) {
        cv->modificationCount = cv->vmCvar->modificationCount;

        if (cv->trackChange && !(cv->cvarFlags & CVAR_LATCH)) {
          trap_SendServerCommand(
              -1, va("cpm \"Server:[lof] ^3%s [lon]^7changed to[lof] ^3%s\n\"",
                     cv->cvarName, cv->vmCvar->string));
        }

        if (cv->teamShader) {
          remapped = qtrue;
        }

        if (cv->vmCvar == &g_filtercams) {
          trap_SetConfigstring(CS_FILTERCAMS, va("%i", g_filtercams.integer));
        }

        if (cv->vmCvar == &g_soldierChargeTime) {
          level.soldierChargeTime[0] =
              g_soldierChargeTime.integer * level.soldierChargeTimeModifier[0];
          level.soldierChargeTime[1] =
              g_soldierChargeTime.integer * level.soldierChargeTimeModifier[1];
          chargetimechanged = qtrue;
        } else if (cv->vmCvar == &g_medicChargeTime) {
          level.medicChargeTime[0] =
              g_medicChargeTime.integer * level.medicChargeTimeModifier[0];
          level.medicChargeTime[1] =
              g_medicChargeTime.integer * level.medicChargeTimeModifier[1];
          chargetimechanged = qtrue;
        } else if (cv->vmCvar == &g_engineerChargeTime) {
          level.engineerChargeTime[0] = g_engineerChargeTime.integer *
                                        level.engineerChargeTimeModifier[0];
          level.engineerChargeTime[1] = g_engineerChargeTime.integer *
                                        level.engineerChargeTimeModifier[1];
          chargetimechanged = qtrue;
        } else if (cv->vmCvar == &g_LTChargeTime) {
          level.lieutenantChargeTime[0] =
              g_LTChargeTime.integer * level.lieutenantChargeTimeModifier[0];
          level.lieutenantChargeTime[1] =
              g_LTChargeTime.integer * level.lieutenantChargeTimeModifier[1];
          chargetimechanged = qtrue;
        } else if (cv->vmCvar == &g_covertopsChargeTime) {
          level.covertopsChargeTime[0] = g_covertopsChargeTime.integer *
                                         level.covertopsChargeTimeModifier[0];
          level.covertopsChargeTime[1] = g_covertopsChargeTime.integer *
                                         level.covertopsChargeTimeModifier[1];
          chargetimechanged = qtrue;
        } else if (cv->vmCvar == &match_readypercent) {
          if (match_readypercent.integer < 1) {
            trap_Cvar_Set(cv->cvarName, "1");
          } else if (match_readypercent.integer > 100) {
            trap_Cvar_Set(cv->cvarName, "100");
          }
        } else if (cv->vmCvar == &g_warmup) {
          if (g_gamestate.integer != GS_PLAYING) {
            level.warmupTime =
                level.time +
                (((g_warmup.integer < 10) ? 11 : g_warmup.integer + 1) * 1000);
            trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
          }
        } else if (cv->vmCvar == &pmove_msec) {
          if (pmove_msec.integer < 8) {
            trap_Cvar_Set(cv->cvarName, "8");
          } else if (pmove_msec.integer > 33) {
            trap_Cvar_Set(cv->cvarName, "33");
          }
        } else if (cv->vmCvar == &sv_fps) {
          if (sv_fps.integer < 10) {
            trap_Cvar_Set(cv->cvarName, "10");
          } else if (sv_fps.integer > 125) {
            trap_Cvar_Set(cv->cvarName, "125");
          }

          level.frameTime = 1000 / sv_fps.integer;
        } else if (cv->vmCvar == &vote_allow_map ||
                   cv->vmCvar == &vote_allow_matchreset ||
                   cv->vmCvar == &vote_allow_randommap ||
                   cv->vmCvar == &vote_allow_rtv ||
                   cv->vmCvar == &vote_allow_autoRtv ||
                   cv->vmCvar == &g_enableVote) {
          fVoteFlags = qtrue;
        } else if (cv->vmCvar == &g_allowSpeclock) {
          if (!g_allowSpeclock.integer) {
            for (int i = 0; i < level.numConnectedClients; i++) {
              gentity_t *ent = g_entities + level.sortedClients[i];

              if (ent->client->sess.specLocked) {
                ent->client->sess.specLocked = qfalse;
                Printer::SendPopupMessage(
                    ClientNum(ent),
                    "You are no longer locked from spectators.");
                ETJump::UpdateClientConfigString(*ent);
              }
            }
          }
        } else {
          fToggles =
              (G_checkServerToggle(cv->vmCvar) || fToggles) ? qtrue : qfalse;
        }
      }
    }
  }

  if (fVoteFlags) {
    G_voteFlags();
  }

  if (fToggles) {
    trap_SetConfigstring(CS_SERVERTOGGLES, va("%d", level.server_settings));
  }

  if (remapped) {
    G_RemapTeamShaders();
  }

  if (chargetimechanged) {
    char cs[MAX_INFO_STRING];
    cs[0] = '\0';
    Info_SetValueForKey(cs, "axs_sld", va("%i", level.soldierChargeTime[0]));
    Info_SetValueForKey(cs, "ald_sld", va("%i", level.soldierChargeTime[1]));
    Info_SetValueForKey(cs, "axs_mdc", va("%i", level.medicChargeTime[0]));
    Info_SetValueForKey(cs, "ald_mdc", va("%i", level.medicChargeTime[1]));
    Info_SetValueForKey(cs, "axs_eng", va("%i", level.engineerChargeTime[0]));
    Info_SetValueForKey(cs, "ald_eng", va("%i", level.engineerChargeTime[1]));
    Info_SetValueForKey(cs, "axs_lnt", va("%i", level.lieutenantChargeTime[0]));
    Info_SetValueForKey(cs, "ald_lnt", va("%i", level.lieutenantChargeTime[1]));
    Info_SetValueForKey(cs, "axs_cvo", va("%i", level.covertopsChargeTime[0]));
    Info_SetValueForKey(cs, "ald_cvo", va("%i", level.covertopsChargeTime[1]));
    trap_SetConfigstring(CS_CHARGETIMES, cs);
  }
}

// Reset particular server variables back to defaults if a config is voted in.
void G_wipeCvars(void) {
  int i;
  cvarTable_t *pCvars;

  for (i = 0, pCvars = gameCvarTable; i < gameCvarTableSize; i++, pCvars++) {
    if (pCvars->vmCvar && pCvars->fConfigReset) {
      G_Printf("set %s %s\n", pCvars->cvarName, pCvars->defaultString);
      trap_Cvar_Set(pCvars->cvarName, pCvars->defaultString);
    }
  }

  G_UpdateCvars();
}

// bani - #113
#define SNIPSIZE 250

void G_ExecMapSpecificConfig() {
  int len;
  fileHandle_t f;

  len = trap_FS_FOpenFile(va("autoexec_%s.cfg", level.rawmapname), &f, FS_READ);
  if (len > 0) {
    // autoexec_mapname.cfg file found
    trap_SendConsoleCommand(EXEC_APPEND,
                            va("exec autoexec_%s.cfg\n", level.rawmapname));
    return;
  }

  len = trap_FS_FOpenFile("autoexec_default.cfg", &f, FS_READ);
  if (len > 0) {
    // autoexec_default.cfg file found
    trap_SendConsoleCommand(EXEC_APPEND, "exec autoexec_default.cfg\n");
  }
}

// copies max num chars from beginning of dest into src and returns pointer to
// new src
char *strcut(char *dest, char *src, int num) {
  int i;

  if (!dest || !src || !num) {
    return NULL;
  }
  for (i = 0; i < num; i++) {
    if ((char)*src) {
      *dest = *src;
      dest++;
      src++;
    } else {
      break;
    }
  }
  *dest = (char)0;
  return src;
}

void InitGhosting() {
  // Handle g_ghostPlayers values so it'll always be either 1 or 0
  // except when noghost worldspawn key is used to modify it. This is to
  // make sure no server admin sets g_ghostPlayers to anything but 0-1
  if (g_ghostPlayers.integer) {
    if (g_ghostPlayers.integer == 2) {
      trap_Cvar_Set("g_ghostPlayers", "0");
    } else {
      trap_Cvar_Set("g_ghostPlayers", "1");
    }

    trap_Cvar_Update(&g_ghostPlayers);
  }
}
// Fixes MAX_NUM_SPAWNTARGETS EXCEEDED
void ResetNumSpawnTargets();
void ETJump_InitGame(int levelTime, int randomSeed, int restart);

static bool G_PatchFixEnabled() {
  char patchFix[MAX_QPATH];
  trap_Cvar_VariableStringBuffer("cm_optimizePatchPlanes", patchFix,
                                 sizeof(patchFix));

  // if the cvar doesn't exist (2.60b/old versions of ETL/ETe),
  // we get a null char
  if (patchFix[0] != '\0' && Q_atoi(patchFix) == 0) {
    return true;
  }

  return false;
}

/*
============
G_InitGame

============
*/
void G_InitGame(int levelTime, int randomSeed, int restart) {
  int i;
  char cs[MAX_INFO_STRING];
  const char *Months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  qtime_t ct;

  G_Printf(S_COLOR_LTGREY GAME_HEADER);
  G_Printf(S_COLOR_LTGREY "____________________________\n");
  G_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                    " " S_COLOR_LTGREY GAME_BINARY_NAME
                                    " init...\n");

  trap_RealTime(&ct);

  G_Printf("\n\n------- Game Initialization -------\n");
  G_Printf("gamename: %s %s\n", GAME_NAME, GAME_VERSION);
  G_Printf("gamedate: %s\n", __DATE__ " " __TIME__);

  if (g_gametype.integer != ETJUMP_GAMETYPE) {
    trap_Cvar_Set("g_gametype", va("%i", ETJUMP_GAMETYPE));
    trap_Cvar_Update(&g_gametype);
    G_Printf("Gametype forced to 2.\n");
  }

  trap_Cvar_Set("sv_floodprotect", "0");

  trap_Cvar_Set("mod_version", GAME_VERSION);
  // reset moverscale on each map load
  trap_Cvar_Set("g_moverScale", "1.0");

  srand(randomSeed);

  // bani - make sure pak2.pk3 gets referenced on server so pure checks
  // pass
  trap_FS_FOpenFile("pak2.dat", &i, FS_READ);
  trap_FS_FCloseFile(i);

  G_RegisterCvars();

  G_ClearIPMutes();

  G_ProcessIPBans();

  G_InitMemory();

  // NERVE - SMF - intialize gamestate
  if (g_gamestate.integer == GS_INITIALIZE) {
    // OSP
    trap_Cvar_Set("gamestate", va("%i", GS_PLAYING));
  }

  // set some level globals
  i = level.server_settings;
  {
    qboolean oldspawning = level.spawning;
    voteInfo_t votedata;

    memcpy(&votedata, &level.voteInfo, sizeof(voteInfo_t));

    memset(&level, 0, sizeof(level));

    memcpy(&level.voteInfo, &votedata, sizeof(voteInfo_t));

    level.spawning = oldspawning;
  }
  level.time = levelTime;
  level.startTime = levelTime;
  level.server_settings = i;
  level.frameTime = 1000 / sv_fps.integer;

  for (i = 0; i < level.numConnectedClients; i++) {
    level.clients[level.sortedClients[i]].sess.spawnObjectiveIndex = 0;
  }

  // RF, init the anim scripting
  level.animScriptData.soundIndex = G_SoundIndex;
  level.animScriptData.playSound = G_AnimScriptSound;

  level.warmupModificationCount = g_warmup.modificationCount;

  level.soldierChargeTime[0] = level.soldierChargeTime[1] =
      g_soldierChargeTime.integer;
  level.medicChargeTime[0] = level.medicChargeTime[1] =
      g_medicChargeTime.integer;
  level.engineerChargeTime[0] = level.engineerChargeTime[1] =
      g_engineerChargeTime.integer;
  level.lieutenantChargeTime[0] = level.lieutenantChargeTime[1] =
      g_LTChargeTime.integer;

  level.covertopsChargeTime[0] = level.covertopsChargeTime[1] =
      g_covertopsChargeTime.integer;

  level.soldierChargeTimeModifier[0] = level.soldierChargeTimeModifier[1] = 1.f;
  level.medicChargeTimeModifier[0] = level.medicChargeTimeModifier[1] = 1.f;
  level.engineerChargeTimeModifier[0] = level.engineerChargeTimeModifier[1] =
      1.f;
  level.lieutenantChargeTimeModifier[0] =
      level.lieutenantChargeTimeModifier[1] = 1.f;
  level.covertopsChargeTimeModifier[0] = level.covertopsChargeTimeModifier[1] =
      1.f;

  level.hasTimerun = false;

  cs[0] = '\0';
  Info_SetValueForKey(cs, "axs_sld", va("%i", level.soldierChargeTime[0]));
  Info_SetValueForKey(cs, "ald_sld", va("%i", level.soldierChargeTime[1]));
  Info_SetValueForKey(cs, "axs_mdc", va("%i", level.medicChargeTime[0]));
  Info_SetValueForKey(cs, "ald_mdc", va("%i", level.medicChargeTime[1]));
  Info_SetValueForKey(cs, "axs_eng", va("%i", level.engineerChargeTime[0]));
  Info_SetValueForKey(cs, "ald_eng", va("%i", level.engineerChargeTime[1]));
  Info_SetValueForKey(cs, "axs_lnt", va("%i", level.lieutenantChargeTime[0]));
  Info_SetValueForKey(cs, "ald_lnt", va("%i", level.lieutenantChargeTime[1]));
  Info_SetValueForKey(cs, "axs_cvo", va("%i", level.covertopsChargeTime[0]));
  Info_SetValueForKey(cs, "ald_cvo", va("%i", level.covertopsChargeTime[1]));
  trap_SetConfigstring(CS_CHARGETIMES, cs);
  trap_SetConfigstring(CS_FILTERCAMS, va("%i", g_filtercams.integer));

  G_SoundIndex("sound/misc/referee.wav");
  G_SoundIndex("sound/misc/vote.wav");
  G_SoundIndex("sound/player/gurp1.wav");
  G_SoundIndex("sound/player/gurp2.wav");

  trap_GetServerinfo(cs, sizeof(cs));

  // ensure map name is always lowercase, even if map is loaded
  // with upper/mixed case name (e.g. /map OASIS)
  // callvote already turns the name to all lower case, which means
  // custom mapscripts are required to be lower case on Linux
  const std::string &mapName =
      ETJump::StringUtil::toLowerCase(Info_ValueForKey(cs, "mapname"));
  Info_SetValueForKey(cs, "mapname", mapName.c_str());
  trap_Cvar_Set("mapname", mapName.c_str());
  Q_strncpyz(level.rawmapname, mapName.c_str(), sizeof(level.rawmapname));

  G_ExecMapSpecificConfig();

  trap_SetConfigstring(CS_SCRIPT_MOVER_NAMES, ""); // clear out

  G_DebugOpenSkillLog();

  if (g_dailyLogs.integer) {
    trap_Cvar_Set("g_log", va("logs/%s-%02d-%02d.log", Months[ct.tm_mon],
                              ct.tm_mday, 1900 + ct.tm_year));
    trap_Cvar_Set("g_adminLog",
                  va("logs/admin-%s-%02d-%02d.log", Months[ct.tm_mon],
                     ct.tm_mday, 1900 + ct.tm_year));
    trap_Cvar_Update(&g_logFile);
    trap_Cvar_Update(&g_adminLog);
  }

  if (g_logFile.string[0]) {
    if (g_logSync.integer) {
      trap_FS_FOpenFile(g_logFile.string, &level.logFile, FS_APPEND_SYNC);
      trap_FS_FOpenFile(g_adminLog.string, &level.adminLogFile, FS_APPEND_SYNC);
    } else {
      trap_FS_FOpenFile(g_logFile.string, &level.logFile, FS_APPEND);
      trap_FS_FOpenFile(g_adminLog.string, &level.adminLogFile, FS_APPEND);
    }
    if (!level.logFile || !level.adminLogFile) {
      G_Printf("WARNING: Couldn't open logfile: %s\n", g_logFile.string);
    } else {
      G_LogPrintf("------------------------------------"
                  "------------------------\n");
      G_LogPrintf("InitGame: %s\n", cs);
      G_LogPrintf("%s %s %s %s\n", GAME_NAME, GAME_VERSION, __DATE__, __TIME__);
    }
  } else {
    G_Printf("Not logging to disk.\n");
  }

  G_InitWorldSession();

  // DHM - Nerve :: Clear out spawn target config strings
  trap_GetConfigstring(CS_MULTI_INFO, cs, sizeof(cs));
  Info_SetValueForKey(cs, "numspawntargets", "0");
  trap_SetConfigstring(CS_MULTI_INFO, cs);
  ResetNumSpawnTargets();
  for (i = CS_MULTI_SPAWNTARGETS;
       i < CS_MULTI_SPAWNTARGETS + MAX_MULTI_SPAWNTARGETS; i++) {
    trap_SetConfigstring(i, "");
  }

  G_ResetTeamMapData();

  // initialize all entities for this game
  memset(g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]));
  level.gentities = g_entities;

  // initialize all clients for this game
  level.maxclients = g_maxclients.integer;
  memset(g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]));
  level.clients = g_clients;

  // set client fields on player ents
  for (i = 0; i < level.maxclients; i++) {
    g_entities[i].client = level.clients + i;
  }

  // always leave room for the max number of clients,
  // even if they aren't all used, so numbers inside that
  // range are NEVER anything but clients
  level.num_entities = MAX_CLIENTS;

  // let the server system know where the entites are
  trap_LocateGameData(level.gentities, level.num_entities, sizeof(gentity_t),
                      &level.clients[0].ps, sizeof(level.clients[0]));

  // Reset the amount of timerun timers
  level.timerunNamesCount = 0;

  // must be called before scripts
  ETJump::initRemappedShaders();

  // load level script
  G_Script_ScriptLoad();

  // reserve some spots for dead player bodies
  InitBodyQue();

  numSplinePaths = 0;
  numPathCorners = 0;

  // START	Mad Doctor I changes, 8/21/2002
  // This needs to be called before G_SpawnEntitiesFromString, or the
  // bot entities get trashed.
  // initialize the bot game entities
  //	BotInitBotGameEntities();
  // END		Mad Doctor I changes, 8/21/2002

  // TAT 11/13/2002
  //		similarly set up the Server entities
  InitServerEntities();

  // This must be called before G_SpawnEntitiesFromString, else
  // it'll mess up the g_ghostPlayers value.
  InitGhosting();

  // Must be called before entities are created
  initializeETJump();

  // parse the key/value pairs and spawn gentities
  G_SpawnEntitiesFromString();

  ETJump::TimerunEntity::validateTimerunEntities();

  // TAT 11/13/2002 - entities are spawned, so now we can do setup
  InitialServerEntitySetup();

  // Gordon: debris test
  G_LinkDebris();

  // Gordon: link up damage parents
  G_LinkDamageParents();

  BG_ClearScriptSpeakerPool();

  BG_LoadSpeakerScript(va("sound/maps/%s.sps", level.rawmapname));

  level.tracemapLoaded = qfalse;
  if (!BG_LoadTraceMap(level.rawmapname, level.mapcoordsMins,
                       level.mapcoordsMaxs)) {
    G_Printf("^1ERROR No tracemap found for map\n");
  } else {
    level.tracemapLoaded = qtrue;
  }

  // Link all the splines up
  BG_BuildSplinePaths();

  // create the camera entity that will communicate with the scripts
  //	G_SpawnScriptCamera();

  // general initialization
  G_FindTeams();

  trap_PbStat(-1, "INIT", "GAME");

#ifndef NO_BOT_SUPPORT
  if (bot_enable.integer) {
    BotAISetup(restart);
    //		BotAILoadMap( restart );
    G_InitBots(restart);
  }
#endif // NO_BOT_SUPPORT

  G_RemapTeamShaders();

  BG_ClearAnimationPool();

  BG_ClearCharacterPool();

  BG_InitWeaponStrings();

  G_RegisterPlayerClasses();

  // Match init work
  G_loadMatchGame();

  // Reinstate any MV views for clients -- need to do this after all
  // init is complete
  // --- maybe not the best place to do this... seems to be some race
  // conditions on map_restart
  G_spawnPrintf(DP_MVSPAWN, level.time + 2000, NULL);

  OnGameInit();
  ETJump_InitGame(levelTime, randomSeed, restart);

  if (G_PatchFixEnabled()) {
    G_Printf("\n^7--------- ^1!!! WARNING !!! ^7---------\n\n^7Server started "
             "with ^3cm_optimizePatchPlanes 0\n^7Patch collision is different "
             "from vanilla and prediction errors might occur!\n\n^7Please "
             "start the server with ^3+set cm_optimizePatchPlanes "
             "1\n\n^7-----------------------------------\n");
  }

  G_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                    " " S_COLOR_LTGREY GAME_BINARY_NAME
                                    " init... " S_COLOR_GREEN "DONE\n");
}

void ETJump_ShutdownGame(int restart);

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame(int restart) {

  OnGameShutdown();
  ETJump_ShutdownGame(restart);

  if (g_gametype.integer != ETJUMP_GAMETYPE) {
    trap_Cvar_Set("g_gametype", va("%i", ETJUMP_GAMETYPE));
    trap_Cvar_Update(&g_gametype);
  }

  G_Printf("==== ShutdownGame ====\n");

  G_DebugCloseSkillLog();

  shutdownETJump();
  if (level.logFile) {
    G_LogPrintf("ShutdownGame:\n");
    G_LogPrintf("----------------------------------------------"
                "--------------\n");
    trap_FS_FCloseFile(level.logFile);
    level.logFile = 0;
  }
  if (level.adminLogFile) {
    trap_FS_FCloseFile(level.adminLogFile);
    level.adminLogFile = 0;
  }

  // write all the client session data so we can get it back
  G_WriteSessionData(restart ? qtrue : qfalse);
}

//===================================================================

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link

void QDECL Com_Error(int unused, const char *error, ...) {
  va_list argptr;
  char text[1024];

  va_start(argptr, error);
  Q_vsnprintf(text, sizeof(text), error, argptr);
  va_end(argptr);

  G_Error("%s", text);
}
// bani
void QDECL Com_Error(int level, const char *error, ...);

void QDECL Com_Printf(const char *msg, ...) {
  va_list argptr;
  char text[1024];

  va_start(argptr, msg);
  Q_vsnprintf(text, sizeof(text), msg, argptr);
  va_end(argptr);

  G_Printf("%s", text);
}
// bani
void QDECL Com_Printf(const char *msg, ...);

#endif

/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/

/*
=============
SortRanks

=============
*/
int QDECL SortRanks(const void *a, const void *b) {
  gclient_t *ca, *cb;

  ca = &level.clients[*(const int *)a];
  cb = &level.clients[*(const int *)b];

  // sort special clients last
  if (/*ca->sess.spectatorState == SPECTATOR_SCOREBOARD ||*/ ca->sess
          .spectatorClient < 0) {
    return 1;
  }
  if (/*cb->sess.spectatorState == SPECTATOR_SCOREBOARD ||*/ cb->sess
          .spectatorClient < 0) {
    return -1;
  }

  // then connecting clients
  if (ca->pers.connected == CON_CONNECTING) {
    return 1;
  }
  if (cb->pers.connected == CON_CONNECTING) {
    return -1;
  }

  // then spectators
  if (ca->sess.sessionTeam == TEAM_SPECTATOR &&
      cb->sess.sessionTeam == TEAM_SPECTATOR) {
    if (ca->sess.spectatorTime < cb->sess.spectatorTime) {
      return -1;
    }
    if (ca->sess.spectatorTime > cb->sess.spectatorTime) {
      return 1;
    }
    return 0;
  }
  if (ca->sess.sessionTeam == TEAM_SPECTATOR) {
    return 1;
  }
  if (cb->sess.sessionTeam == TEAM_SPECTATOR) {
    return -1;
  }

  int i, totalXP[2];

  for (totalXP[0] = totalXP[1] = 0, i = 0; i < SK_NUM_SKILLS; i++) {
    totalXP[0] += static_cast<int>(ca->sess.skillpoints[i]);
    totalXP[1] += static_cast<int>(cb->sess.skillpoints[i]);
  }

  // then sort by xp
  if (totalXP[0] > totalXP[1]) {
    return -1;
  }
  if (totalXP[0] < totalXP[1]) {
    return 1;
  }
  return 0;
}

// bani - #184
//(relatively) sane replacement for OSP's Players_Axis/Players_Allies
void etpro_PlayerInfo(void) {
  // 128 bits
  char playerinfo[MAX_CLIENTS + 1];
  gentity_t *e;
  team_t playerteam;
  int i;
  int lastclient;

  memset(playerinfo, 0, sizeof(playerinfo));

  lastclient = -1;
  e = &g_entities[0];
  for (i = 0; i < MAX_CLIENTS; i++, e++) {
    if (e->client == NULL || e->client->pers.connected == CON_DISCONNECTED) {
      playerinfo[i] = '-';
      continue;
    }

    // keep track of highest connected/connecting client
    lastclient = i;

    if (e->inuse == qfalse) {
      playerteam = TEAM_FREE;
    } else {
      playerteam = e->client->sess.sessionTeam;
    }
    playerinfo[i] = (char)'0' + playerteam;
  }
  // terminate the string, if we have any non-0 clients
  if (lastclient != -1) {
    playerinfo[lastclient + 1] = (char)0;
  } else {
    playerinfo[0] = (char)0;
  }

  trap_Cvar_Set("P", playerinfo);
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks(void) {
  int i;
  //	int		rank;
  //	int		score;
  //	int		newScore;
  char teaminfo[TEAM_NUM_TEAMS][256]; // OSP
  gclient_t *cl;

  level.follow1 = -1;
  level.follow2 = -1;
  level.numConnectedClients = 0;
  level.numNonSpectatorClients = 0;
  level.numPlayingClients = 0;
  level.voteInfo.numVotingClients = 0; // don't count bots

  level.numFinalDead[0] = 0; // NERVE - SMF
  level.numFinalDead[1] = 0; // NERVE - SMF

  level.voteInfo.numVotingTeamClients[0] = 0;
  level.voteInfo.numVotingTeamClients[1] = 0;

  for (i = 0; i < TEAM_NUM_TEAMS; i++) {
    if (i < 2) {
      level.numTeamClients[i] = 0;
    }
    teaminfo[i][0] = 0; // OSP
  }

  for (i = 0; i < level.maxclients; i++) {
    if (level.clients[i].pers.connected != CON_DISCONNECTED) {
      int team = level.clients[i].sess.sessionTeam;

      level.sortedClients[level.numConnectedClients] = i;
      level.numConnectedClients++;

      if (team == TEAM_SPECTATOR) {
        if (level.clients[i].ps.eFlags & EF_VOTED) {
          level.voteInfo.numVotingClients++;
        }
      }

      if (team != TEAM_SPECTATOR) {
        level.numNonSpectatorClients++;

        // OSP
        Q_strcat(teaminfo[team], sizeof(teaminfo[team]) - 1,
                 va("%d ", level.numConnectedClients));

        // decide if this should be
        // auto-followed
        if (level.clients[i].pers.connected == CON_CONNECTED) {
          int teamIndex =
              level.clients[i].sess.sessionTeam == TEAM_AXIS ? 0 : 1;
          level.numPlayingClients++;
          if (!(g_entities[i].r.svFlags & SVF_BOT)) {
            level.voteInfo.numVotingClients++;
          }

          if (level.clients[i].sess.sessionTeam == TEAM_AXIS ||
              level.clients[i].sess.sessionTeam == TEAM_ALLIES) {

            level.numTeamClients[teamIndex]++;
            if (!(g_entities[i].r.svFlags & SVF_BOT)) {
              level.voteInfo.numVotingTeamClients[teamIndex]++;
            }
          }

          if (level.follow1 == -1) {
            level.follow1 = i;
          } else if (level.follow2 == -1) {
            level.follow2 = i;
          }
        }
      }
    }
  }

  // OSP
  for (i = 0; i < TEAM_NUM_TEAMS; i++) {
    if (0 == teaminfo[i][0]) {
      Q_strncpyz(teaminfo[i], "(None)", sizeof(teaminfo[i]));
    }
  }

  qsort(level.sortedClients, level.numConnectedClients,
        sizeof(level.sortedClients[0]), SortRanks);

  // set the rank value for all clients that are connected and not
  // spectators in team games, rank is just the order of the teams,
  // 0=red, 1=blue, 2=tied
  for (i = 0; i < level.numConnectedClients; i++) {
    cl = &level.clients[level.sortedClients[i]];
    if (level.teamScores[TEAM_AXIS] == level.teamScores[TEAM_ALLIES]) {
      cl->ps.persistant[PERS_RANK] = 2;
    } else if (level.teamScores[TEAM_AXIS] > level.teamScores[TEAM_ALLIES]) {
      cl->ps.persistant[PERS_RANK] = 0;
    } else {
      cl->ps.persistant[PERS_RANK] = 1;
    }
  }

  // set the CS_SCORES1/2 configstrings, which will be visible to
  // everyone
  //	trap_SetConfigstring( CS_SCORES1, va("%i",
  // level.teamScores[TEAM_AXIS] )
  //); 	trap_SetConfigstring( CS_SCORES2, va("%i",
  // level.teamScores[TEAM_ALLIES] ) );

  trap_SetConfigstring(CS_FIRSTBLOOD, va("%i", level.firstbloodTeam));
  trap_SetConfigstring(CS_ROUNDSCORES1, "0");
  trap_SetConfigstring(CS_ROUNDSCORES2, "0");

  // bani - #184
  etpro_PlayerInfo();

  // if we are at the intermission, send the new info to everyone
  if (g_gamestate.integer == GS_INTERMISSION) {
    SendScoreboardMessageToAllClients();
  } else {
    // see if it is time to end the level
    CheckExitRules();
  }
}

/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients(void) {
  int i;

  for (i = 0; i < level.numConnectedClients; i++) {
    if (level.clients[level.sortedClients[i]].pers.connected == CON_CONNECTED) {
      level.clients[level.sortedClients[i]].wantsscore = qtrue;
      //			G_SendScore(g_entities +
      // level.sortedClients[i]);
    }
  }
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission(gentity_t *ent) {
  //	float			timeLived;

  // take out of follow mode if needed
  if (ent->client->sess.spectatorState == SPECTATOR_FOLLOW) {
    StopFollowing(ent);
  }

  /*if ( ent->client->sess.sessionTeam == TEAM_AXIS ||
  ent->client->sess.sessionTeam == TEAM_ALLIES ) { timeLived =
  (level.time - ent->client->pers.lastSpawnTime) * 0.001f;

      G_AddExperience( ent, min((timeLived * timeLived) * 0.00005f, 5)
  );
  }*/

  // move to the spot
  VectorCopy(level.intermission_origin, ent->s.origin);
  VectorCopy(level.intermission_origin, ent->client->ps.origin);
  VectorCopy(level.intermission_angle, ent->client->ps.viewangles);
  ent->client->ps.pm_type = PM_INTERMISSION;

  // clean up powerup info
  // memset( ent->client->ps.powerups, 0,
  // sizeof(ent->client->ps.powerups) );

  ent->client->ps.eFlags = 0;
  ent->s.eFlags = 0;
  ent->s.eType = ET_GENERAL;
  ent->s.modelindex = 0;
  ent->s.loopSound = 0;
  ent->s.event = 0;
  ent->s.events[0] = ent->s.events[1] = ent->s.events[2] = ent->s.events[3] =
      0; // DHM - Nerve
  ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint(void) {
  gentity_t *ent = NULL, *target;
  vec3_t dir;
  char cs[MAX_STRING_CHARS]; // DHM - Nerve
  const char *buf;           // DHM - Nerve
  int winner;                // DHM - Nerve

  // NERVE - SMF - if the match hasn't ended yet, and we're just a
  // spectator
  if (!level.intermissiontime) {
    // try to find the intermission spawnpoint with no team
    // flags set
    ent = G_Find(NULL, FOFS(classname), "info_player_intermission");

    for (; ent;
         ent = G_Find(ent, FOFS(classname), "info_player_intermission")) {
      if (!ent->spawnflags) {
        break;
      }
    }
  }

  trap_GetConfigstring(CS_MULTI_MAPWINNER, cs, sizeof(cs));
  buf = Info_ValueForKey(cs, "winner");
  winner = Q_atoi(buf);

  // Change from scripting value for winner (0==AXIS, 1==ALLIES) to
  // spawnflag value
  if (winner == 0) {
    winner = TEAM_AXIS;
  } else {
    winner = TEAM_ALLIES;
  }

  if (!ent) {
    ent = G_Find(NULL, FOFS(classname), "info_player_intermission");
    while (ent) {
      if (ent->spawnflags & winner) {
        break;
      }

      ent = G_Find(ent, FOFS(classname), "info_player_intermission");
    }
  }

  if (!ent) // the map creator forgot to put in an intermission point...
  {
    SelectSpawnPoint(vec3_origin, level.intermission_origin,
                     level.intermission_angle);
  } else {
    VectorCopy(ent->s.origin, level.intermission_origin);
    VectorCopy(ent->s.angles, level.intermission_angle);
    // if it has a target, look towards it
    if (ent->target) {
      target = G_PickTarget(ent->target);
      if (target) {
        VectorSubtract(target->s.origin, level.intermission_origin, dir);
        vectoangles(dir, level.intermission_angle);
      }
    }
  }
}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission(void) {
  int i;
  gentity_t *client;

  if (g_gamestate.integer == GS_INTERMISSION) {
    return; // already active
  }

  level.intermissiontime = level.time;
  trap_SetConfigstring(CS_INTERMISSION_START_TIME,
                       va("%i", level.intermissiontime));
  trap_Cvar_Set("gamestate", va("%i", GS_INTERMISSION));
  trap_Cvar_Update(&g_gamestate);

  FindIntermissionPoint();

  // move all clients to the intermission point
  for (i = 0; i < level.maxclients; i++) {
    client = g_entities + i;
    if (!client->inuse) {
      continue;
    }
    MoveClientToIntermission(client);
  }

  // send the current scoring to all clients
  SendScoreboardMessageToAllClients();
}

/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar

=============
*/
void ExitLevel(void) {
  int i;
  gclient_t *cl;

  trap_SendConsoleCommand(EXEC_APPEND, "vstr nextmap\n");
  level.changemap = NULL;
  level.intermissiontime = 0;

  // reset all the scores so we don't enter the intermission again
  level.teamScores[TEAM_AXIS] = 0;
  level.teamScores[TEAM_ALLIES] = 0;

  for (i = 0; i < g_maxclients.integer; i++) {
    cl = level.clients + i;
    if (cl->pers.connected != CON_CONNECTED) {
      continue;
    }
    cl->ps.persistant[PERS_SCORE] = 0;
  }

  // we need to do this here before chaning to CON_CONNECTING
  G_WriteSessionData(qfalse);

  // change all client states to connecting, so the early players into
  // the next level will know the others aren't done reconnecting
  for (i = 0; i < g_maxclients.integer; i++) {

    if (level.clients[i].pers.connected == CON_CONNECTED) {
      level.clients[i].pers.connected = CON_CONNECTING;
      trap_UnlinkEntity(&g_entities[i]);
    }
  }

  G_LogPrintf("ExitLevel: executed\n");
}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf(const char *fmt, ...) {
  va_list argptr;
  char string[1024];
  qtime_t rt;
  int min, hour, sec, l;
  trap_RealTime(&rt);

  hour = rt.tm_hour;
  min = rt.tm_min;
  sec = rt.tm_sec;

  Com_sprintf(string, sizeof(string), "%02i:%02i:%02i ", hour, min, sec);

  l = strlen(string);

  va_start(argptr, fmt);
  Q_vsnprintf(string + l, sizeof(string) - l, fmt, argptr);
  va_end(argptr);

  if (g_dedicated.integer) {
    G_Printf("%s", string + l);
  }

  if (!level.logFile) {
    return;
  }

  trap_FS_Write(string, strlen(string), level.logFile);
}
// bani

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit(const char *string) {
  int i;
  gclient_t *cl;

  // NERVE - SMF - do not allow LogExit to be called in non-playing
  // gamestate
  if (g_gamestate.integer != GS_PLAYING) {
    return;
  }

  G_LogPrintf("Exit: %s\n", string);

  level.intermissionQueued = level.time;

  // this will keep the clients from playing any voice sounds
  // that will get cut off when the queued intermission starts
  trap_SetConfigstring(CS_INTERMISSION, "1");

  G_LogPrintf("red:%i  blue:%i\n", level.teamScores[TEAM_AXIS],
              level.teamScores[TEAM_ALLIES]);

  // NERVE - SMF - send gameCompleteStatus message to master servers
  trap_SendConsoleCommand(EXEC_APPEND, "gameCompleteStatus\n");

  for (i = 0; i < level.numConnectedClients; i++) {
    int ping;

    cl = &level.clients[level.sortedClients[i]];

    // rain - #105 - use G_MakeUnready instead
    G_MakeUnready(&g_entities[level.sortedClients[i]]);

    if (cl->sess.sessionTeam == TEAM_SPECTATOR) {
      continue;
    }
    if (cl->pers.connected == CON_CONNECTING) {
      continue;
    }

    ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

    G_LogPrintf("score: %i  ping: %i  client: %i %s\n",
                cl->ps.persistant[PERS_SCORE], ping, level.sortedClients[i],
                cl->pers.netname);
  }

  G_BuildEndgameStats();
}

/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit(void) {
  static int fActions = 0;
  qboolean exit = qtrue;
  int i;
  // rain - for #105
  gclient_t *cl;
  int ready = 0, notReady = 0;

  // OSP - end-of-level auto-actions
  //		  maybe make the weapon stats dump available to single
  // player?
  if (!(fActions & EOM_WEAPONSTATS) &&
      level.time - level.intermissiontime > 300) {
    G_matchInfoDump(EOM_WEAPONSTATS);
    fActions |= EOM_WEAPONSTATS;
  }
  if (!(fActions & EOM_MATCHINFO) &&
      level.time - level.intermissiontime > 800) {
    G_matchInfoDump(EOM_MATCHINFO);
    fActions |= EOM_MATCHINFO;
  }

  for (i = 0; i < level.numConnectedClients; i++) {
    // rain - #105 - spectators and people who are still loading
    // don't have to be ready at the end of the round.
    // additionally, make readypercent apply here.

    cl = level.clients + level.sortedClients[i];

    if (cl->pers.connected != CON_CONNECTED ||
        cl->sess.sessionTeam == TEAM_SPECTATOR) {
      continue;
    } else if (cl->pers.ready ||
               (g_entities[level.sortedClients[i]].r.svFlags & SVF_BOT)) {
      ready++;
    } else {
      notReady++;
    }
  }

  // rain - #105 - use the same code as the beginning of round ready to
  // decide whether enough players are ready to exceed
  // match_readypercent
  if (level.ref_allready ||
      ((ready + notReady > 0) &&
       100 * ready / (ready + notReady) >= match_readypercent.integer)) {
    level.ref_allready = qfalse;
    exit = qtrue;
  } else {
    exit = qfalse;
  }

  // Gordon: changing this to a minute for now
  if (!exit && (level.time < level.intermissiontime + 60000)) {
    return;
  }

  ExitLevel();
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied(void) {
  int a /*, b*/;
  char cs[MAX_STRING_CHARS];
  const char *buf;

  // DHM - Nerve :: GT_WOLF checks the current value of
  trap_GetConfigstring(CS_MULTI_MAPWINNER, cs, sizeof(cs));

  buf = Info_ValueForKey(cs, "winner");
  a = Q_atoi(buf);

  return a == -1 ? qtrue : qfalse;
}

qboolean G_ScriptAction_SetWinner(gentity_t *ent, char *params);

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules(void) {

  // if at the intermission, wait for all non-bots to
  // signal ready, then go to next level
  if (g_gamestate.integer == GS_INTERMISSION) {
    CheckIntermissionExit();
    return;
  }

  if (level.intermissionQueued) {
    level.intermissionQueued = 0;
    BeginIntermission();
    return;
  }

  if (g_timelimit.value && !level.warmupTime) {
    // OSP
    if ((level.timeCurrent - level.startTime) >= (g_timelimit.value * 60000)) {

      if (level.gameManager) {
        G_Script_ScriptEvent(level.gameManager, "trigger", "timelimit_hit");
      }

      // NERVE - SMF - do not allow LogExit to be called
      // in non-playing gamestate
      // - This already happens in LogExit, but we need
      // it for the print command
      if (g_gamestate.integer != GS_PLAYING) {
        return;
      }

      trap_SendServerCommand(-1, "print \"Timelimit hit.\n\"");
      LogExit("Timelimit hit.");

      return;
    }
  }
}

void CheckWolfMP() {
  // check because we run 6 game frames before calling Connect and/or
  // ClientBegin for clients on a map_restart
  if (g_gamestate.integer == GS_PLAYING ||
      g_gamestate.integer == GS_INTERMISSION) {
    if (level.intermissiontime && g_gamestate.integer != GS_INTERMISSION) {
      trap_Cvar_Set("gamestate", va("%i", GS_INTERMISSION));
    }
    return;
  }

  // check warmup latch
  if (g_gamestate.integer == GS_WARMUP) {
    if (!g_doWarmup.integer ||
        (level.numPlayingClients >= match_minplayers.integer &&
         level.lastRestartTime + 1000 < level.time && G_readyMatchState())) {
      // 1 Sec warmup.
      int delay = 1;

      level.warmupTime = level.time + (delay * 1000);
      trap_Cvar_Set("gamestate", va("%i", GS_WARMUP_COUNTDOWN));
      trap_Cvar_Update(&g_gamestate);
      trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
    }
  }

  // if the warmup time has counted down, restart
  if (g_gamestate.integer == GS_WARMUP_COUNTDOWN) {
    if (level.time > level.warmupTime) {
      level.warmupTime += 10000;
      trap_Cvar_Set("g_restarted", "1");
      trap_SendConsoleCommand(EXEC_APPEND, "map_restart 0\n");
      level.restarted = qtrue;
      return;
    }
  }
}

int getNumPlayingClients() {
  auto numPlayingClients = 0;
  for (auto i = 0, len = level.numConnectedClients; i < len; ++i) {
    auto clientNum = level.sortedClients[i];
    if ((g_entities + clientNum)->client->sess.sessionTeam != TEAM_SPECTATOR) {
      ++numPlayingClients;
    }
  }
  return numPlayingClients;
}

int getNumValidVoters() {
  auto numValidPlayers = getNumPlayingClients();
  auto numVotedSpecs = 0;
  if (g_spectatorVote.integer > 0) {
    for (auto i = 0, len = level.numConnectedClients; i < len; ++i) {
      auto clientNum = level.sortedClients[i];
      auto player = g_entities + clientNum;
      if (player->client->sess.sessionTeam == TEAM_SPECTATOR &&
          player->client->ps.eFlags & EF_VOTED) {
        ++numVotedSpecs;
      }
    }
  }

  return numValidPlayers + numVotedSpecs;
}

void resetVote() {
  level.voteInfo.voteTime = 0;
  level.voteInfo.forcePass = qfalse;
  level.voteInfo.voteCanceled = qfalse;
  level.voteInfo.isAutoRtvVote = false;
  game.rtv->setRtvStatus(false);

  for (int i = 0; i < level.numConnectedClients; i++) {
    gentity_t *client = g_entities + level.sortedClients[i];
    memset(&client->client->pers.votingInfo, 0, sizeof(ETJump::votingInfo_t));
  }

  trap_SetConfigstring(CS_VOTE_TIME, "");
}

/*
==================
CheckVote
==================
*/
void CheckVote() {
  if (level.voteInfo.voteCanceled) {
    G_LogPrintf("Vote cancelled: %s\n", level.voteInfo.voteString);
    resetVote();
    return;
  }

  if (level.voteInfo.forcePass) {
    G_LogPrintf("Vote passed: %s\n", level.voteInfo.voteString);
    level.voteInfo.vote_fn(nullptr, 0, nullptr, nullptr);
    resetVote();
    return;
  }

  int minVoteDuration;
  const bool isRtvVote = game.rtv->rtvVoteActive();
  const bool isAutoRtvVote = level.voteInfo.isAutoRtvVote;

  // vote_minVoteDuration is likely low on most servers, so we use a separate
  // duration for rtv to give everyone some time to pick a map
  // I'd really like this to be seconds but bleh, consistency I guess...
  if (isRtvVote) {
    minVoteDuration = Numeric::clamp(vote_minRtvDuration.integer, 1000, 29000);
  } else {
    minVoteDuration = Numeric::clamp(vote_minVoteDuration.integer, 1000, 29000);
  }

  if (!level.voteInfo.voteTime || level.voteInfo.vote_fn == nullptr ||
      level.time - level.voteInfo.voteTime < minVoteDuration) {
    return;
  }

  const int requiredPercentage = Numeric::clamp(vote_percent.integer, 1, 99);
  const int validVotingClients = getNumValidVoters();
  const int requiredClients = validVotingClients * requiredPercentage / 100;
  const auto voter =
      isAutoRtvVote ? nullptr : g_entities + level.voteInfo.voter_cn;

  if (!isAutoRtvVote &&
      level.voteInfo.voter_team != voter->client->sess.sessionTeam) {
    Printer::BroadcastPopupMessage("^7Vote canceled: caller switched team.");
    G_LogPrintf("Vote canceled: %s (caller %s switched teams)\n",
                level.voteInfo.voteString, voter->client->pers.netname);

    level.voteInfo.voteYes = 0;
    level.voteInfo.voteNo = level.numConnectedClients;
  } else if (level.voteInfo.voteYes > requiredClients) {
    Printer::BroadcastPopupMessage("^5Vote passed!");
    G_LogPrintf("Vote Passed: %s\n", level.voteInfo.voteString);

    level.voteInfo.voteTime = 0;
    level.voteInfo.voteCanceled = qfalse;
    level.voteInfo.vote_fn(nullptr, 0, nullptr, nullptr);
  } else if (level.voteInfo.voteNo >=
                 level.numConnectedClients - requiredClients ||
             level.time - level.voteInfo.voteTime >= VOTE_TIME) {
    std::string voteFailedMsg = ETJump::stringFormat("^3Vote FAILED! ^3(%s)",
                                                     level.voteInfo.voteString);
    Printer::BroadcastPopupMessage(voteFailedMsg);
    G_LogPrintf("Vote Failed: %s\n", level.voteInfo.voteString);

    level.voteInfo.voteTime = 0;
    level.voteInfo.voteCanceled = qfalse;
  } else {
    return;
  }
  resetVote();
}

/*
==================
CheckCvars
==================
*/
void CheckCvars(void) {
  static int g_password_lastMod = -1;

  if (g_password.modificationCount != g_password_lastMod) {
    g_password_lastMod = g_password.modificationCount;
    if (*g_password.string && Q_stricmp(g_password.string, "none")) {
      trap_Cvar_Set("g_needpass", "1");
    } else {
      trap_Cvar_Set("g_needpass", "0");
    }
  }
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink(gentity_t *ent) {
  float thinktime;

  // OSP - If paused, push nextthink
  if (level.match_pause != PAUSE_NONE &&
      (ent - g_entities) >= g_maxclients.integer &&
      ent->nextthink > level.time &&
      strstr(ent->classname, "DPRINTF_") == NULL) {
    ent->nextthink += level.time - level.previousTime;
  }

  // RF, run scripting
  if (ent->s.number >= MAX_CLIENTS) {
    G_Script_ScriptRun(ent);
  }

  thinktime = ent->nextthink;
  if (thinktime <= 0) {
    return;
  }
  if (thinktime > level.time) {
    return;
  }

  ent->nextthink = 0;
  if (!ent->think) {
    G_Error("NULL ent->think");
  }
  ent->think(ent);
}

void G_RunEntity(gentity_t *ent, int msec);

/*
======================
G_PositionEntityOnTag
======================
*/
qboolean G_PositionEntityOnTag(gentity_t *entity, gentity_t *parent,
                               char *tagName) {
  int i;
  orientation_t tag;
  vec3_t axis[3];
  AnglesToAxis(parent->r.currentAngles, axis);

  VectorCopy(parent->r.currentOrigin, entity->r.currentOrigin);

  if (!trap_GetTag(-1, parent->tagNumber, tagName, &tag)) {
    return qfalse;
  }

  for (i = 0; i < 3; i++) {
    VectorMA(entity->r.currentOrigin, tag.origin[i], axis[i],
             entity->r.currentOrigin);
  }

  if (entity->client && entity->s.eFlags & EF_MOUNTEDTANK) {
    // zinx - moved tank hack to here
    // bani - fix tank bb
    // zinx - figured out real values, only tag_player is
    // applied, so there are two left: mg42upper attaches to
    // tag_mg42nest[mg42base] at: 0.03125, -1.171875, 27.984375
    // player attaches to tag_playerpo[mg42upper] at:
    // 3.265625, -1.359375, 2.96875
    // this is a hack, by the way.
    entity->r.currentOrigin[0] += 0.03125 + 3.265625;
    entity->r.currentOrigin[1] += -1.171875 + -1.359375;
    entity->r.currentOrigin[2] += 27.984375 + 2.96875;
  }

  G_SetOrigin(entity, entity->r.currentOrigin);

  if (entity->r.linked && !entity->client) {
    if (!VectorCompare(entity->oldOrigin, entity->r.currentOrigin)) {
      trap_LinkEntity(entity);
    }
  }

  return qtrue;
}

void G_TagLinkEntity(gentity_t *ent, int msec) {
  gentity_t *parent = &g_entities[ent->s.torsoAnim];
  vec3_t move, amove;
  gentity_t *obstacle;
  vec3_t origin, angles{};
  vec3_t v;

  if (ent->linkTagTime >= level.time) {
    return;
  }

  G_RunEntity(parent, msec);

  if (!(parent->s.eFlags & EF_PATH_LINK)) {
    if (parent->s.pos.trType == TR_LINEAR_PATH) {
      int pos;
      float frac;

      if ((ent->backspline = BG_GetSplineData(parent->s.effect2Time,
                                              &ent->back)) == nullptr) {
        return;
      }

      ent->backdelta =
          parent->s.pos.trDuration
              ? static_cast<float>(level.time - parent->s.pos.trTime) /
                    static_cast<float>(parent->s.pos.trDuration)
              : 0;

      if (ent->backdelta < 0.f) {
        ent->backdelta = 0.f;
      } else if (ent->backdelta > 1.f) {
        ent->backdelta = 1.f;
      }

      if (ent->back) {
        ent->backdelta = 1 - ent->backdelta;
      }

      pos = std::floor(ent->backdelta * (MAX_SPLINE_SEGMENTS));
      if (pos >= MAX_SPLINE_SEGMENTS) {
        pos = MAX_SPLINE_SEGMENTS - 1;
        frac = ent->backspline->segments[pos].length;
      } else {
        frac = ((ent->backdelta * (MAX_SPLINE_SEGMENTS)) -
                static_cast<float>(pos)) *
               ent->backspline->segments[pos].length;
      }

      VectorMA(ent->backspline->segments[pos].start, frac,
               ent->backspline->segments[pos].v_norm, v);
      if (parent->s.apos.trBase[0] != 0) {
        BG_LinearPathOrigin2(parent->s.apos.trBase[0], &ent->backspline,
                             &ent->backdelta, v, ent->back);
      }

      VectorCopy(v, origin);

      if (ent->s.angles2[0] != 0) {
        BG_LinearPathOrigin2(ent->s.angles2[0], &ent->backspline,
                             &ent->backdelta, v, ent->back);
      }

      VectorCopy(v, ent->backorigin);

      if (ent->s.angles2[0] < 0) {
        VectorSubtract(v, origin, v);
        vectoangles(v, angles);
      } else if (ent->s.angles2[0] > 0) {
        VectorSubtract(origin, v, v);
        vectoangles(v, angles);
      } else {
        VectorCopy(vec3_origin, origin);
      }

      ent->moving = qtrue;
    } else {
      ent->moving = qfalse;
    }
  } else {
    if (parent->moving) {
      VectorCopy(parent->backorigin, v);

      ent->back = parent->back;
      ent->backdelta = parent->backdelta;
      ent->backspline = parent->backspline;

      VectorCopy(v, origin);

      if (ent->s.angles2[0] != 0) {
        BG_LinearPathOrigin2(ent->s.angles2[0], &ent->backspline,
                             &ent->backdelta, v, ent->back);
      }

      VectorCopy(v, ent->backorigin);

      if (ent->s.angles2[0] < 0) {
        VectorSubtract(v, origin, v);
        vectoangles(v, angles);
      } else if (ent->s.angles2[0] > 0) {
        VectorSubtract(origin, v, v);
        vectoangles(v, angles);
      } else {
        VectorCopy(vec3_origin, origin);
      }

      ent->moving = qtrue;
    } else {
      ent->moving = qfalse;
    }
  }

  if (ent->moving) {
    VectorSubtract(origin, ent->r.currentOrigin, move);
    VectorSubtract(angles, ent->r.currentAngles, amove);

    if (!G_MoverPush(ent, move, amove, &obstacle)) {
      script_mover_blocked(ent, obstacle);
    }

    VectorCopy(origin, ent->s.pos.trBase);
    VectorCopy(angles, ent->s.apos.trBase);
  } else {
    memset(&ent->s.pos, 0, sizeof(ent->s.pos));
    memset(&ent->s.apos, 0, sizeof(ent->s.apos));

    VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
    VectorCopy(ent->r.currentAngles, ent->s.apos.trBase);
  }

  ent->linkTagTime = level.time;
}

void G_RunEntity(gentity_t *ent, int msec) {
  if (ent->runthisframe) {
    return;
  }

  ent->runthisframe = qtrue;

  if (!ent->inuse) {
    return;
  }

  if (g_scriptDebug.integer) {
    ETJump::EntityUtilities::checkForRailBox(ent);
  }

  if (ent->tagParent) {

    G_RunEntity(ent->tagParent, msec);

    if (ent->tagParent) {
      if (G_PositionEntityOnTag(ent, ent->tagParent, ent->tagName)) {
        if (!ent->client) {
          if (!ent->s.density) {
            BG_EvaluateTrajectory(&ent->s.apos, level.time,
                                  ent->r.currentAngles, qtrue,
                                  ent->s.effect2Time);
            VectorAdd(ent->tagParent->r.currentAngles, ent->r.currentAngles,
                      ent->r.currentAngles);
          } else {
            BG_EvaluateTrajectory(&ent->s.apos, level.time,
                                  ent->r.currentAngles, qtrue,
                                  ent->s.effect2Time);
          }
        }
      }
    }
  } else if (ent->s.eFlags & EF_PATH_LINK) {

    G_TagLinkEntity(ent, msec);
  }

  // ydnar: hack for instantaneous velocity
  VectorCopy(ent->r.currentOrigin, ent->oldOrigin);

  // check EF_NODRAW status for non-clients
  if (ent - g_entities > level.maxclients) {
    if (ent->flags & FL_NODRAW) {
      ent->s.eFlags |= EF_NODRAW;
    } else {
      ent->s.eFlags &= ~EF_NODRAW;
    }
  }

  // clear events that are too old
  if (level.time - ent->eventTime > EVENT_VALID_MSEC) {
    if (ent->s.event) {
      ent->s.event = 0;
    }
    if (ent->freeAfterEvent) {
      // tempEntities or dropped items completely go
      // away after their event
      G_FreeEntity(ent);
      return;
    } else if (ent->unlinkAfterEvent) {
      // items that will respawn will hide themselves
      // after their pickup event
      ent->unlinkAfterEvent = qfalse;
      trap_UnlinkEntity(ent);
    }
  }

  // temporary entities don't think
  if (ent->freeAfterEvent) {
    return;
  }

  // Arnout: invisible entities don't think
  // NOTE: hack - constructible one does
  if (ent->s.eType != ET_CONSTRUCTIBLE &&
      (ent->entstate == STATE_INVISIBLE ||
       ent->entstate == STATE_UNDERCONSTRUCTION)) {
    // Gordon: we want them still to run scripts tho :p
    if (ent->s.number >= MAX_CLIENTS) {
      G_Script_ScriptRun(ent);
    }
    return;
  }

  if (!ent->r.linked && ent->neverFree) {
    return;
  }

  if (ent->s.eType == ET_MISSILE || ent->s.eType == ET_FLAMEBARREL ||
      ent->s.eType == ET_FP_PARTS || ent->s.eType == ET_FIRE_COLUMN ||
      ent->s.eType == ET_FIRE_COLUMN_SMOKE || ent->s.eType == ET_EXPLO_PART ||
      ent->s.eType == ET_RAMJET) {

    // OSP - pausing
    if (level.match_pause == PAUSE_NONE) {
      G_RunMissile(ent);
    } else {
      // During a pause, gotta keep track of stuff in
      // the air
      ent->s.pos.trTime += level.time - level.previousTime;
      // Keep pulsing right for dynmamite
      if (ent->methodOfDeath == MOD_DYNAMITE) {
        ent->s.effect1Time += level.time - level.previousTime;
      }
      G_RunThink(ent);
    }
    // OSP

    return;
  }

  // DHM - Nerve :: Server-side collision for flamethrower
  if (ent->s.eType == ET_FLAMETHROWER_CHUNK) {
    G_RunFlamechunk(ent);

    // ydnar: hack for instantaneous velocity
    VectorSubtract(ent->r.currentOrigin, ent->oldOrigin, ent->instantVelocity);
    VectorScale(ent->instantVelocity, 1000.0f / msec, ent->instantVelocity);

    return;
  }

  if (ent->s.eType == ET_ITEM || ent->physicsObject) {
    G_RunItem(ent);

    // ydnar: hack for instantaneous velocity
    VectorSubtract(ent->r.currentOrigin, ent->oldOrigin, ent->instantVelocity);
    VectorScale(ent->instantVelocity, 1000.0f / msec, ent->instantVelocity);

    return;
  }

  if (ent->s.eType == ET_MOVER || ent->s.eType == ET_PROP) {
    G_RunMover(ent);

    // ydnar: hack for instantaneous velocity
    VectorSubtract(ent->r.currentOrigin, ent->oldOrigin, ent->instantVelocity);
    VectorScale(ent->instantVelocity, 1000.0f / msec, ent->instantVelocity);

    return;
  }

  if (ent - g_entities < MAX_CLIENTS) {
    G_RunClient(ent);

    // ydnar: hack for instantaneous velocity
    VectorSubtract(ent->r.currentOrigin, ent->oldOrigin, ent->instantVelocity);
    VectorScale(ent->instantVelocity, 1000.0f / msec, ent->instantVelocity);

    return;
  }

  if ((ent->s.eType == ET_HEALER || ent->s.eType == ET_SUPPLIER) &&
      ent->target_ent) {
    ent->target_ent->s.onFireStart = ent->health;
    ent->target_ent->s.onFireEnd = ent->count;
  }

  G_RunThink(ent);

  // ydnar: hack for instantaneous velocity
  VectorSubtract(ent->r.currentOrigin, ent->oldOrigin, ent->instantVelocity);
  VectorScale(ent->instantVelocity, 1000.0f / msec, ent->instantVelocity);
}

void ETJump_RunFrame(int levelTime);

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame(int levelTime) {
  int i;

  // if we are waiting for the level to restart, do nothing
  if (level.restarted) {
    return;
  }

  // Handling of pause offsets
  if (level.match_pause == PAUSE_NONE) {
    level.timeCurrent = levelTime - level.timeDelta;
  } else {
    level.timeDelta = levelTime - level.timeCurrent;
    if ((level.time % 500) == 0) {
      // FIXME: set a PAUSE cs and let the client adjust
      //  their local start times instead of this spam
      trap_SetConfigstring(CS_LEVEL_START_TIME,
                           va("%i", level.startTime + level.timeDelta));
    }
  }

  level.framenum++;
  level.previousTime = level.time;
  level.time = levelTime;

  level.axisBombCounter -= level.frameTime;
  level.alliedBombCounter -= level.frameTime;

  if (level.axisBombCounter < 0) {
    level.axisBombCounter = 0;
  }
  if (level.alliedBombCounter < 0) {
    level.alliedBombCounter = 0;
  }

#if 0
	if (trap_Cvar_VariableIntegerValue("dbg_spam"))
	{
		trap_SetConfigstring(CS_VOTE_STRING, va(
		                         "vvvvvvvvvvvvvvvvvvvwiubgfiwebxqbwigb3qir4gviqrbegiuertbgiuoeyvqrugyveru\
qogyjvuqeyrvguqoehvrguorevqguoveruygqueorvguoqeyrvguyervguverougvequryvg\
uoerqvgouqevrguoerqvguoerqvguoyqevrguoyvreuogvqeuogiyvureoyvnguoeqvguoqe\
rvguoeqrvuoeqvrguoyvqeruogverquogvqoeurvgouqervguerqvgqiertbgiqerubgipqe\
rbgipqebgierqviqrviertqyvbgyerqvgieqrbgipqebrgpiqbergibepbreqgupqruiperq\
ubgipqeurbgpiqjefgpkeiueripgberipgubreugqeirpqgbipeqygbibgpibqpebiqgefpi\
mgbqepigjbriqpirbgipvbiqpgvbpqiegvbiepqbgqiebgipqgjebiperqbgpiqebpireqbg\
ipqbgipjqfebzipjgbqipqervbgiyreqvbgipqertvgbiprqbgipgbipertqjgbipubriuqi\
pjgpifjbqzpiebgipuerqbgpibuergpijfebgqiepgbiupreqbgpqegjfebzpigu4ebrigpq\
uebrgpiebrpgibqeripgubeqrpigubqifejbgipegbrtibgurepqgbn%i", level.time)
		                     );
	}
#endif

  // get any cvar changes
  G_UpdateCvars();

  for (i = 0; i < level.num_entities; i++) {
    g_entities[i].runthisframe = qfalse;
  }

  // go through all allocated objects
  for (i = 0; i < level.num_entities; i++) {
    G_RunEntity(&g_entities[i], level.frameTime);
  }

  for (i = 0; i < level.numConnectedClients; i++) {
    ClientEndFrame(&g_entities[level.sortedClients[i]]);
  }

  // NERVE - SMF
  CheckWolfMP();

  // see if it is time to end the level
  CheckExitRules();

  // update to team status?
  CheckTeamStatus();

  // cancel vote if timed out
  CheckVote();

  // for tracking changes
  CheckCvars();

  G_UpdateTeamMapData();

  if (level.gameManager) {
    level.gameManager->s.otherEntityNum =
        MAX_TEAM_LANDMINES - G_CountTeamLandmines(TEAM_AXIS);
    level.gameManager->s.otherEntityNum2 =
        MAX_TEAM_LANDMINES - G_CountTeamLandmines(TEAM_ALLIES);
  }

  RunFrame(levelTime);

  ETJump_RunFrame(levelTime);
}
