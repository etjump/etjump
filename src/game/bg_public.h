/*
 * name:		bg_public.h
 *
 * desc:		definitions shared by both the server game and client
 * game modules
 *
 */

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#ifndef __BG_PUBLIC_H__
#define __BG_PUBLIC_H__

#include <array>

#define BUILD_TIME __DATE__ " " __TIME__

#if !defined(GAME_NAME)
  #define GAME_NAME "etjump"
#endif

#if !defined(GAME_URL)
  #define GAME_URL "etjump.com"
#endif

#if !defined(GAME_VERSION)
  #define GAME_VERSION "unversioned"
#endif

#if defined(_DEBUG)
  #define GAME_VERSION_DATED GAME_VERSION " " CPUSTRING " " __DATE__ " DEBUG"
  #define GAME_VERSION_N GAME_VERSION " DEBUG"
#else
  #define GAME_VERSION_DATED GAME_VERSION " " CPUSTRING " " __DATE__
  #define GAME_VERSION_N GAME_VERSION
#endif

#define GAME_TAG (GAME_NAME " " GAME_VERSION)

#define GAME_SIGNATURE                                                         \
  "      __    _               \n"                                             \
  " ___ / /_  (_)_ ____ _  ___ \n"                                             \
  "/ -_) __/ / / // /  ' \\/ _ \\\n"                                           \
  "\\__/\\__/_/ /\\_,_/_/_/_/ .__/\n"                                          \
  "      |___/          /_/    \n"

#define GAME_HEADER                                                            \
  GAME_SIGNATURE                                                               \
  "\n"                                                                         \
  "- MOD NAME:     " GAME_NAME "\n"                                            \
  "- MOD URL:      " GAME_URL "\n"                                             \
  "- MOD BINARY:   " GAME_BINARY_NAME "\n"                                     \
  "- MOD VERSION:  " GAME_VERSION_N "\n"                                       \
  "- BUILD DATE:   " __DATE__ "\n"                                             \
  "- BUILD COMMIT: " GIT_COMMIT_HASH "\n"                                      \
  "- BUILD ARCH:   " CPUSTRING "\n"

// bani
#ifdef __GNUC__
  #define _attribute(x) __attribute__(x)
#else
  #define _attribute(x)
#endif

inline constexpr float SPRINTTIME = 20000.0f;

inline constexpr int DEFAULT_GRAVITY = 800;
inline constexpr int FORCE_LIMBO_HEALTH = -75; // JPW NERVE
inline constexpr int GIB_HEALTH = -175;        // JPW NERVE

inline constexpr int HOLDBREATHTIME = 12000;

inline constexpr int MAX_ITEMS = 256;

// item sizes are needed for client side pickup detection
inline constexpr int ITEM_RADIUS = 10;

inline constexpr int FLAMETHROWER_RANGE = 2500;

inline constexpr int VOTE_TIME = 30000; // 30 seconds before vote times out

inline constexpr int DEFAULT_VIEWHEIGHT = 40;
inline constexpr int CROUCH_VIEWHEIGHT = 16;
inline constexpr int DEAD_VIEWHEIGHT = -16;
inline constexpr int PRONE_VIEWHEIGHT = -8;

extern vec3_t playerMins;
extern vec3_t playerMaxs;
extern vec3_t playerlegsProneMins;
extern vec3_t playerlegsProneMaxs;

// RF, on fire effects
inline constexpr int FIRE_FLASH_TIME = 2000;
inline constexpr int FIRE_FLASH_FADEIN_TIME = 1000;

inline constexpr int AAGUN_DAMAGE = 25;
inline constexpr int AAGUN_SPREAD = 10;

inline constexpr int MG42_SPREAD_MP = 100;
inline constexpr int MG42_DAMAGE_MP = 20;
inline constexpr int MG42_RATE_OF_FIRE_MP = 66;

inline constexpr float MAX_MG42_HEAT = 1500.0f;
inline constexpr int MG42_HEAT_RECOVERY = 2000;

inline constexpr int AAGUN_RATE_OF_FIRE = 100;
inline constexpr float MG42_YAWSPEED = 300.f; // degrees per second

enum ChatMode {
  SAY_ALL,
  SAY_TEAM,
  SAY_BUDDY,
  SAY_ADMIN,
};

// Arnout: different entity states
typedef enum {
  STATE_DEFAULT,          // ent is linked, can be used and is solid
  STATE_INVISIBLE,        // ent is unlinked, can't be used, doesn't think and
                          // is not solid
  STATE_UNDERCONSTRUCTION // ent is being constructed
} entState_t;

typedef enum {
  SELECT_BUDDY_ALL = 0,
  SELECT_BUDDY_1,
  SELECT_BUDDY_2,
  SELECT_BUDDY_3,
  SELECT_BUDDY_4,
  SELECT_BUDDY_5,
  SELECT_BUDDY_6,

  SELECT_BUDDY_LAST // must be the last one in the enum
} SelectBuddyFlag;

inline constexpr int MAX_TAGCONNECTS = 64;

// (SA) zoom sway values
inline constexpr float ZOOM_PITCH_AMPLITUDE = 0.13f;
inline constexpr float ZOOM_PITCH_FREQUENCY = 0.24f;
// minimum amount of sway even if completely settled on target
inline constexpr float ZOOM_PITCH_MIN_AMPLITUDE = 0.1f;

inline constexpr float ZOOM_YAW_AMPLITUDE = 0.7f;
inline constexpr float ZOOM_YAW_FREQUENCY = 0.12f;
inline constexpr float ZOOM_YAW_MIN_AMPLITUDE = 0.2f;

inline constexpr int MAX_OBJECTIVES = 8;
inline constexpr int MAX_OID_TRIGGERS = 18;

// ETJump: need to keep this at default so that 'gameinfo.txt' parses correctly,
// otherwise we cannot show the correct gametypes in the server browser
inline constexpr int MAX_GAMETYPES = 16;

typedef struct {
  const char *mapName;
  const char *mapLoadName;
  const char *imageName;

  int typeBits;
  int cinematic;

  // Gordon: FIXME: remove
  const char *opponentName;
  int teamMembers;
  int timeToBeat[MAX_GAMETYPES];

  qhandle_t levelShot;
  qboolean active;

  // NERVE - SMF
  int Timelimit;
  int AxisRespawnTime;
  int AlliedRespawnTime;
  // -NERVE - SMF

  vec2_t mappos;

  const char *briefing;
  const char *lmsbriefing;
  const char *objectives;
  const char *author;
} mapInfo;

// Random reinforcement seed settings
// FIXME: remove
inline constexpr int MAX_REINFSEEDS = 8;
inline constexpr int REINF_RANGE = 16;   // (0 to n-1 second offset)
inline constexpr int REINF_BLUEDELT = 3; // Allies shift offset
inline constexpr int REINF_REDDELT = 2;  // Axis shift offset
extern const unsigned int aReinfSeeds[MAX_REINFSEEDS];

// Client flags for server processing
inline constexpr int CGF_AUTORELOAD = 1 << 0;
inline constexpr int CGF_STATSDUMP = 1 << 1;
inline constexpr int CGF_AUTOACTIVATE = 1 << 2;
inline constexpr int CGF_NOFATIGUE = 1 << 3;
inline constexpr int CGF_PMOVEFIXED = 1 << 4;
inline constexpr int CGF_CGAZ = 1 << 5;
inline constexpr int CGF_LOADVIEWANGLES = 1 << 6;
inline constexpr int CGF_CHEATCVARSON = 1 << 7;
inline constexpr int CGF_HIDEME = 1 << 8;
inline constexpr int CGF_ENABLE_TIMERUNS = 1 << 9;
inline constexpr int CGF_NOACTIVATELEAN = 1 << 10;
inline constexpr int CGF_AUTO_LOAD = 1 << 11;
inline constexpr int CGF_QUICK_FOLLOW = 1 << 12;
inline constexpr int CGF_SNAPHUD = 1 << 13;
inline constexpr int CGF_NOPANZERSWITCH = 1 << 14;
inline constexpr int CGF_AUTOSPRINT = 1 << 15;

inline constexpr int MAX_MOTDLINES = 6;

// Multiview settings
// FIXME: remove
inline constexpr int MAX_MVCLIENTS = 32;
inline constexpr int MV_SCOREUPDATE_INTERVAL = 5000; // in msec

inline constexpr int MAX_CHARACTERS = 16;

inline constexpr int MAX_FIRETEAMS = 12;
inline constexpr int MAX_FIRETEAM_USERS = 15;
inline constexpr int FT_SAVELIMIT_NOT_SET = -1;

extern const char *bg_fireteamNames[MAX_FIRETEAMS];

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
//

inline constexpr int CS_MUSIC = 2;
// from the map worldspawn's message field
inline constexpr int CS_MESSAGE = 3;
// g_motd string for server message of the day
inline constexpr int CS_MOTD = 4;
// server time when the match will be restarted
inline constexpr int CS_WARMUP = 5;
inline constexpr int CS_VOTE_TIME = 6;
inline constexpr int CS_VOTE_STRING = 7;
inline constexpr int CS_VOTE_YES = 8;
inline constexpr int CS_VOTE_NO = 9;
inline constexpr int CS_GAME_VERSION = 10;

// so the timer only shows the current level
inline constexpr int CS_LEVEL_START_TIME = 11;
// when 1, intermission will start in a second or two
inline constexpr int CS_INTERMISSION = 12;
inline constexpr int CS_MULTI_INFO = 13;
inline constexpr int CS_MULTI_MAPWINNER = 14;
inline constexpr int CS_MULTI_OBJECTIVE = 15;
// 16 = unused

// Ridah, used to tell clients to fade their screen to black/normal
inline constexpr int CS_SCREENFADE = 17;
// used for saving the current state/settings of the fog
inline constexpr int CS_FOGVARS = 18;
// this is where we should view the skybox from
inline constexpr int CS_SKYBOXORG = 19;
inline constexpr int CS_TARGETEFFECT = 20;

// ETJump: this whole CS is useless for us and can be reused for something
// without messing up old demos, as it used to just hold
// campaign/stopwatch/LMS/gamestate data, which we have never supported
inline constexpr int CS_WOLFINFO = 21;
// Team that has first blood
inline constexpr int CS_FIRSTBLOOD = 22;
// ETJump: unused (Axis LMS round wins)
inline constexpr int CS_ROUNDSCORES1 = 23;
// ETJump: unused (Allied LMS round wins)
inline constexpr int CS_ROUNDSCORES2 = 24;
// Most important current objective
inline constexpr int CS_MAIN_AXIS_OBJECTIVE = 25;
// Most important current objective
inline constexpr int CS_MAIN_ALLIES_OBJECTIVE = 26;
inline constexpr int CS_MUSIC_QUEUE = 27;
inline constexpr int CS_SCRIPT_MOVER_NAMES = 28;
inline constexpr int CS_CONSTRUCTION_NAMES = 29;
// Versioning info for demo playback compatibility
inline constexpr int CS_VERSIONINFO = 30;

// note: in 2.60b, 34-35 are CS_AXIS_MAPS_XP and CS_ALLIED_MAPS_XP, respectively
// these have been removed in ETJump so long ago that this might as well be the
// "stock" layout and there's not much point in changing these back to match
// what 2.60b had as CS indices, especially since there are blank indices
// right after these anyway, before 'CS_MODELS'
// the indices that would show incorrect data for demos recorded prior
// to that change would likely not cause any playback issues anyway
// https://github.com/etjump/svn-dump/commit/2ac1242b1c598959bfe59c5802c00de1cf7594fa

// Reinforcement seeds
inline constexpr int CS_REINFSEEDS = 31;
// Shows current enable/disabled settings (for voting UI)
inline constexpr int CS_SERVERTOGGLES = 32;
inline constexpr int CS_GLOBALFOGVARS = 33;
inline constexpr int CS_INTERMISSION_START_TIME = 34;
inline constexpr int CS_ENDGAME_STATS = 35;
inline constexpr int CS_CHARGETIMES = 36;
inline constexpr int CS_FILTERCAMS = 37;

// 38-63 (or 40-63) = unused (see comment above)

inline constexpr int CS_MODELS = 64;
inline constexpr int CS_SOUNDS = CS_MODELS + MAX_MODELS;
inline constexpr int CS_SHADERS = CS_SOUNDS + MAX_SOUNDS;
inline constexpr int CS_SHADERSTATE = CS_SHADERS + MAX_CS_SHADERS;
inline constexpr int CS_SKINS = CS_SHADERSTATE + 1;
inline constexpr int CS_CHARACTERS = CS_SKINS + MAX_CS_SKINS;
inline constexpr int CS_PLAYERS = CS_CHARACTERS + MAX_CHARACTERS;
inline constexpr int CS_MULTI_SPAWNTARGETS = CS_PLAYERS + MAX_CLIENTS;
inline constexpr int CS_OID_TRIGGERS =
    CS_MULTI_SPAWNTARGETS + MAX_MULTI_SPAWNTARGETS;
inline constexpr int CS_OID_DATA = CS_OID_TRIGGERS + MAX_OID_TRIGGERS;
inline constexpr int CS_DLIGHTS = CS_OID_DATA + MAX_OID_TRIGGERS;
inline constexpr int CS_SPLINES = CS_DLIGHTS + MAX_DLIGHT_CONFIGSTRINGS;
inline constexpr int CS_TAGCONNECTS = CS_SPLINES + MAX_SPLINE_CONFIGSTRINGS;
inline constexpr int CS_FIRETEAMS = CS_TAGCONNECTS + MAX_TAGCONNECTS;
inline constexpr int CS_CUSTMOTD = CS_FIRETEAMS + MAX_FIRETEAMS;
inline constexpr int CS_STRINGS = CS_CUSTMOTD + MAX_MOTDLINES;
inline constexpr int CS_MAX = CS_STRINGS + MAX_CSSTRINGS;

static_assert(CS_MAX <= MAX_CONFIGSTRINGS,
              "CS_MAX cannot exceed MAX_CONFIGSTRINGS");

typedef int gametype_t;
// since we don't support gametypes, this is just an int instead of an enum
// matches old GT_WOLF gametype (objective)
// we still need to keep 'g_gametype' cvar around for server browser
inline constexpr gametype_t ETJUMP_GAMETYPE = 2;

typedef enum { GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER } gender_t;

/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new
player_state_t and some other output data.  Used for local prediction on the
client game and true movement on the server game.
===================================================================================
*/

typedef enum {
  PM_NORMAL,      // can accelerate and turn
  PM_NOCLIP,      // noclip movement
  PM_SPECTATOR,   // still run into walls
  PM_DEAD,        // no acceleration or turning, but free-falling
  PM_FREEZE,      // stuck in place with no control
  PM_INTERMISSION // no movement or status bar
} pmtype_t;

typedef enum {
  WEAPON_READY,
  WEAPON_RAISING,
  WEAPON_RAISING_TORELOAD,
  WEAPON_DROPPING,
  WEAPON_DROPPING_TORELOAD,
  WEAPON_READYING, // getting from 'ready' to 'firing'
  WEAPON_RELAXING, // weapon is ready, but since not firing, it's on
                   // it's way to a "relaxed" stance
  WEAPON_FIRING,
  WEAPON_FIRINGALT,
  WEAPON_RELOADING, //----(SA)	added
} weaponstate_t;

typedef enum {
  WSTATE_IDLE,
  WSTATE_SWITCH,
  WSTATE_FIRE,
  WSTATE_RELOAD
} weaponstateCompact_t;

// pmove->pm_flags	(sent as max 16 bits in msg.c)

inline constexpr int PMF_DUCKED = 1;
inline constexpr int PMF_JUMP_HELD = 2;
// player is on a ladder
inline constexpr int PMF_LADDER = 4;
// go into backwards land
inline constexpr int PMF_BACKWARDS_JUMP = 8;
// coast down to backwards run
inline constexpr int PMF_BACKWARDS_RUN = 16;
// pm_time is time before rejump
inline constexpr int PMF_TIME_LAND = 32;
// pm_time is an air-accelerate only time
inline constexpr int PMF_TIME_KNOCKBACK = 64;
// pm_time is waterjump
inline constexpr int PMF_TIME_WATERJUMP = 256;
// clear after attack and jump buttons come up
inline constexpr int PMF_RESPAWNED = 512;
// unused
inline constexpr int PMF_PRONE_BIPOD = 1024;
inline constexpr int PMF_FLAILING = 2048;
// spectate following another player
inline constexpr int PMF_FOLLOW = 4096;
// hold for this time after a load game, and prevent large thinks
inline constexpr int PMF_TIME_LOAD = 8192;
// JPW NERVE limbo state, pm_time is time until reinforce
inline constexpr int PMF_LIMBO = 16384;
// DHM - Nerve :: Lock all movement and view changes
inline constexpr int PMF_TIME_LOCKPLAYER = 32768;

inline constexpr int PMF_ALL_TIMES = PMF_TIME_WATERJUMP | PMF_TIME_LAND |
                                     PMF_TIME_KNOCKBACK | PMF_TIME_LOCKPLAYER;

typedef struct {
  qboolean bAutoReload; // do we predict autoreload of weapons

  int jumpTime; // used in MP to prevent jump accel

  int weapAnimTimer;   // don't change low priority animations until this
                       // runs out
                       // //----(SA)	added
  int silencedSideArm; // Gordon: Keep track of whether the luger/colt
                       // is silenced "in holster", prolly want to do
                       // this for the kar98 etc too
  float sprintTime;

  int airleft;

  // Arnout: MG42 aiming
  float varc, harc;
  vec3_t centerangles;

  int dtmove; // doubletap move

  int dodgeTime;
  int proneTime; // time a go-prone or stop-prone move starts, to sync
                 // the animation to

  int proneGroundTime;   // time a prone player last had ground under him
  float proneLegsOffset; // offset legs bounding box

  vec3_t mountedWeaponAngles; // mortar, mg42 (prone), etc

  int weapRecoilTime; // Arnout: time at which a weapon that has a
                      // recoil kickback has been fired last
  int weapRecoilDuration;
  float weapRecoilYaw;
  float weapRecoilPitch;
  int lastRecoilDeltaTime;

  qboolean releasedFire;
  float noclipScale;
  bool isJumpLand;

  // ETJump: exported values from pmove_t & pml_t for cgame drawing
  int tracemask;
  qboolean walking;
  qboolean groundPlane;
  trace_t groundTrace;
  int waterlevel;
  vec3_t mins, maxs;

  vec3_t previous_velocity;
  vec3_t forward, right, up;
  float frametime;
  qboolean ladder;

  vec3_t velocity; // we need to store this before PM_Accelerate scales
                   // it back to preserve the true effect friction has
                   // on ground speed

  float scale;
  float scaleAlt; // cmdScale without upmove component
  float accel;

  // timestamp adrenaline should expire at
  int adrenalineTime;

  float weapHeat[MAX_WEAPONS]; // weapon heat, used to be in ps
  float bobCycle;              // for fps-independent bobCycle

  bool autoSprint;

  // enable buggy nojumpdelay behavior on solstice and stonehalls2,
  // where jump time does not get updated when a player jumps on a NJD surface
  bool jumpDelayBug;
} pmoveExt_t; // data used both in client and server - store it here
              // instead of playerstate to prevent different engine versions of
              // playerstate between XP and MP

inline constexpr int MAXTOUCH = 32;

typedef struct {
  // state (in / out)
  playerState_t *ps;
  pmoveExt_t *pmext;
  struct bg_character_s *character;

  // command (in)
  usercmd_t cmd, oldcmd;
  int tracemask;        // collide against these types of surfaces
  int debugLevel;       // if set, diagnostic output will be printed
  qboolean noFootsteps; // if the game is setup for no footsteps by the
                        // server
  qboolean noWeapClips; // if the game is setup for no weapon clips by
                        // the server
  qboolean gauntletHit; // true if a gauntlet attack would actually hit
                        // something

  // NERVE - SMF (in)
  int gametype;
  int ltChargeTime;
  int soldierChargeTime;
  int engineerChargeTime;
  int medicChargeTime;
  // -NERVE - SMF
  int covertopsChargeTime;

  // results (out)
  int numtouch;
  int touchents[MAXTOUCH];

  vec3_t mins, maxs; // bounding box size

  int watertype;
  int waterlevel;

  float xyspeed;

  int *skill; // player skills

#ifdef GAMEDLL         // the whole stamina thing is only in qagame
  qboolean leadership; // within 512 units of a player with level 5
                       // Signals skill (that player has to be in PVS as
                       // well to make sue we can predict it)
#endif                 // GAMEDLL

  // for fixed msec Pmove
  int pmove_fixed;
  int pmove_msec;

  // ETJump: shared values between client & server
  int shared;
  // ETJump: enable/disable strafe + activate = lean
  qboolean noActivateLean;
  bool noPanzerAutoswitch;

  qboolean walking;
  qboolean groundPlane;
  trace_t groundTrace;

  vec3_t forward, right, up;

  // callbacks to test the world
  // these will be different functions during game and cgame
  void (*trace)(trace_t *results, const vec3_t start, const vec3_t mins,
                const vec3_t maxs, const vec3_t end, int passEntityNum,
                int contentMask);
  int (*pointcontents)(const vec3_t point, int passEntityNum);
} pmove_t;

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles(playerState_t *ps, pmoveExt_t *pmext, usercmd_t *cmd,
                         void(trace)(trace_t *results, const vec3_t start,
                                     const vec3_t mins, const vec3_t maxs,
                                     const vec3_t end, int passEntityNum,
                                     int contentMask),
                         int tracemask);
void Pmove(pmove_t *pmove);
void PmoveSingle(pmove_t *pmove);

inline constexpr int8_t CMDSCALE_DEFAULT = 127;
inline constexpr int8_t CMDSCALE_WALK = 64;

//===================================================================================

inline constexpr int PC_SOLDIER = 0;   //	shoot stuff
inline constexpr int PC_MEDIC = 1;     //	heal stuff
inline constexpr int PC_ENGINEER = 2;  //	build stuff
inline constexpr int PC_FIELDOPS = 3;  //	bomb stuff
inline constexpr int PC_COVERTOPS = 4; //	sneak about ;o

inline constexpr int NUM_PLAYER_CLASSES = 5;

// JPW NERVE
inline constexpr int MAX_WEAPS_IN_BANK_MP = 12;
// Feen: PGM - Changed from 10 to 11
inline constexpr int MAX_WEAP_BANKS_MP = 11;
// jpw

// Keys pressed
inline constexpr int UMOVE_FORWARD = 0x0001;
inline constexpr int UMOVE_BACKWARD = 0x0002;
inline constexpr int UMOVE_LEFT = 0x0004;
inline constexpr int UMOVE_RIGHT = 0x0008;
inline constexpr int UMOVE_UP = 0x0010;
inline constexpr int UMOVE_DOWN = 0x0020;

// player_state->stats[] indexes
typedef enum {
  STAT_HEALTH,
  STAT_KEYS,             // 16 bit fields
  STAT_DEAD_YAW,         // look this direction when dead (FIXME: get rid of?)
  STAT_CLIENTS_READY,    // bit mask of clients wishing to exit the
                         // intermission (FIXME: configstring?)
  STAT_MAX_HEALTH,       // health / armor limit, changable by handicap
  STAT_PLAYER_CLASS,     // DHM - Nerve :: player class in multiplayer
  STAT_CAPTUREHOLD_RED,  // JPW NERVE - red team score
  STAT_CAPTUREHOLD_BLUE, // JPW NERVE - blue team score
  STAT_XP, // Gordon: "realtime" version of xp that doesnt need to go
           // thru the scoreboard
  // setup - keys pressed (high byte contains buttons, low byte contains
  // wbuttons)
  STAT_USERCMD_BUTTONS,
  // setup - keys pressed (see UMOVE_* for flags)
  STAT_USERCMD_MOVE
} statIndex_t;

// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
typedef enum {
  PERS_SCORE, // !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
  PERS_HITS,  // total points damage inflicted so damage beeps can sound
              // on change
  PERS_RANK,
  PERS_TEAM,
  PERS_SPAWN_COUNT, // incremented every respawn
  PERS_ATTACKER,    // clientnum of last damage inflicter
  PERS_KILLED,      // count of the number of times you died
  // these were added for single player awards tracking

  PERS_REVIVE_COUNT,
  // PERS_BLEH_2,
  PERS_JUMP_SPEED, // ETJump: last jump speed
  PERS_BLEH_3,
  PERS_BLEH_4,

  // Rafael - mg42		// (SA) I don't understand these here.
  // can someone explain?
  PERS_HWEAPON_USE,
  // Rafael wolfkick
  PERS_WOLFKICK
} persEnum_t;

// entityState_t->eFlags

// clang-format off
inline constexpr uint32_t EF_DEAD = 0x00000001;                    // don't draw a foe marker over players with EF_DEAD
inline constexpr uint32_t EF_NONSOLID_BMODEL = 0x00000002;         // bmodel is visible, but not solid
inline constexpr uint32_t EF_FORCE_END_FRAME = EF_NONSOLID_BMODEL; // force client to end of current animation (after loading a savegame)
inline constexpr uint32_t EF_TELEPORT_BIT = 0x00000004;            // toggled every time the origin abruptly changes
inline constexpr uint32_t EF_READY = 0x00000008;                   // player is ready

inline constexpr uint32_t EF_CROUCHING = 0x00000010;    // player is crouching
inline constexpr uint32_t EF_MG42_ACTIVE = 0x00000020;  // currently using an MG42
inline constexpr uint32_t EF_NODRAW = 0x00000040;       // may have an event, but no model (unspawned items)
inline constexpr uint32_t EF_FIRING = 0x00000080;       // for lightning gun
inline constexpr uint32_t EF_INHERITSHADER = EF_FIRING; // some ents will never use EF_FIRING, hijack it for "USESHADER"

inline constexpr uint32_t EF_SPINNING = 0x00000100;     // (SA) added for level editor control of spinning pickup items
inline constexpr uint32_t EF_BREATH = EF_SPINNING;      // Characters will not have EF_SPINNING set, hijack for drawing character breath
inline constexpr uint32_t EF_TALK = 0x00000200;         // draw a talk balloon
inline constexpr uint32_t EF_CONNECTION = 0x00000400;   // draw a connection trouble sprite
inline constexpr uint32_t EF_SMOKINGBLACK = 0x00000800; // JPW NERVE -- like EF_SMOKING only darker & bigger

inline constexpr uint32_t EF_HEADSHOT = 0x00001000;                        // last hit to player was head shot (Gordon: NOTE: not last hit, but has BEEN shot in the head since respawn)
inline constexpr uint32_t EF_SMOKING = 0x00002000;                         // DHM - Nerve :: ET_GENERAL ents will emit smoke if set
                                                                           // JPW switched to this after my code change
inline constexpr uint32_t EF_OVERHEATING = (EF_SMOKING | EF_SMOKINGBLACK); // ydnar: light smoke/steam effect
inline constexpr uint32_t EF_VOTED = 0x00004000;                           // already cast a vote;
inline constexpr uint32_t EF_TAGCONNECT = 0x00008000;                      // connected to another entity via tag;
inline constexpr uint32_t EF_MOUNTEDTANK = EF_TAGCONNECT;                  // Gordon: duplicated for clarity

inline constexpr uint32_t EF_SPARE5 = 0x00010000;      // Gordon: freed
inline constexpr uint32_t EF_PATH_LINK = 0x00020000;   // Gordon: linking trains together
inline constexpr uint32_t EF_ZOOMING = 0x00040000;     // client is zooming;
inline constexpr uint32_t EF_PRONE = 0x00080000;       // player is prone

inline constexpr uint32_t EF_PRONE_MOVING = 0x00100000;   // player is prone and moving
inline constexpr uint32_t EF_VIEWING_CAMERA = 0x00200000; // player is viewing a camera
inline constexpr uint32_t EF_AAGUN_ACTIVE = 0x00400000;   // Gordon: player is manning an AA gun
inline constexpr uint32_t EF_SPARE0 = 0x00800000;         // Gordon: freed

// !! NOTE: only place flags that don't need to go to the client beyond 0x00800000
inline constexpr uint32_t EF_SPARE1 = 0x01000000;        // Gordon: freed
inline constexpr uint32_t EF_SPARE2 = 0x02000000;        // Gordon: freed
inline constexpr uint32_t EF_BOUNCE = 0x04000000;        // for missiles
inline constexpr uint32_t EF_BOUNCE_HALF = 0x08000000;   // for missiles

inline constexpr uint32_t EF_MOVER_STOP = 0x10000000;    // will push otherwise	// (SA) moved down to make space for one more client flag
inline constexpr uint32_t EF_MOVER_BLOCKED = 0x20000000; // mover was blocked dont lerp on the client
                                                         // xkan, moved down to make space for client flag
inline constexpr uint32_t EF_BOBBING = EF_SPARE0;        // controls bobbing for pickup items (using existed one because eFlags are transmited as 24 bit)
inline constexpr uint32_t EF_SPARE3 = 0x40000000;        // unused
inline constexpr uint32_t EF_SPARE4 = 0x80000000;        // unused

// etjump: flags that can be used for player effects that need prediction
inline constexpr uint32_t EF_PLAYER_UNUSED1 = EF_NONSOLID_BMODEL; // used only on entities
inline constexpr uint32_t EF_PLAYER_UNUSED2 = EF_SMOKING;         // player never gets this flag, only ent/weapon
inline constexpr uint32_t EF_PLAYER_UNUSED3 = EF_SMOKINGBLACK;    // player never gets this flag, only ent/weapon
inline constexpr uint32_t EF_PLAYER_UNUSED4 = EF_PATH_LINK;       // used only on entities
inline constexpr uint32_t EF_PLAYER_UNUSED5 = EF_SPARE0;          // used only on entities
// clang-format on

#define BG_PlayerMounted(eFlags)                                               \
  ((eFlags & EF_MG42_ACTIVE) || (eFlags & EF_MOUNTEDTANK) ||                   \
   (eFlags & EF_AAGUN_ACTIVE))

inline constexpr int ADRENALINE_TIME = 10000;

// !! NOTE: only place flags that don't need to go to the client beyond
// 0x00800000
typedef enum {
  PW_NONE,

  // (SA) for Wolf
  PW_INVULNERABLE,
  PW_FIRE,          //----(SA)
  PW_PUSHERPREDICT, // used for pusher prediction, was PW_ELECTRIC
  PW_BREATHER,      //----(SA)
  PW_NOFATIGUE,     //----(SA)

  PW_REDFLAG,
  PW_BLUEFLAG,

  PW_OPS_DISGUISED,
  PW_OPS_CLASS_1,
  PW_OPS_CLASS_2,
  PW_OPS_CLASS_3,

  PW_ADRENALINE,

  PW_BLACKOUT = 14,     // OSP - spec blackouts. FIXME: we don't need 32bits
                        // here...relocate
  PW_MVCLIENTLIST = 15, // OSP - MV client info.. need a full 32 bits

  PW_NUM_POWERUPS
} powerup_t;

typedef enum {
  KEY_NONE,
  KEY_1,      // skull
  KEY_2,      // chalice
  KEY_3,      // eye
  KEY_4,      // field radio
  KEY_5,      // satchel charge
  INV_BINOCS, // binoculars
  KEY_7,
  KEY_8,
  KEY_9,
  KEY_10,
  KEY_11,
  KEY_12,
  KEY_13,
  KEY_14,
  KEY_15,
  KEY_16,
  KEY_LOCKED_PICKABLE, // Mad Doc - TDF: ent can be unlocked with the
                       // WP_LOCKPICK.
  KEY_NUM_KEYS
} wkey_t; // conflicts with types.h

typedef enum {
  HI_NONE,

  //	HI_TELEPORTER,
  HI_MEDKIT,

  // new for Wolf
  HI_WINE,
  HI_SKULL,
  HI_WATER,
  HI_ELECTRIC,
  HI_FIRE,
  HI_STAMINA,
  HI_BOOK1, //----(SA)	added
  HI_BOOK2, //----(SA)	added
  HI_BOOK3, //----(SA)	added
  HI_11,
  HI_12,
  HI_13,
  HI_14,
  //	HI_15,	// ?

  HI_NUM_HOLDABLE = 16
} holdable_t;

enum weapon_t : int8_t {
  WP_NONE,              // 0
  WP_KNIFE,             // 1
  WP_LUGER,             // 2
  WP_MP40,              // 3
  WP_GRENADE_LAUNCHER,  // 4   // axis hand grenade
  WP_PANZERFAUST,       // 5
  WP_FLAMETHROWER,      // 6
  WP_COLT,              // 7   // equivalent american weapon to german luger
  WP_THOMPSON,          // 8   // equivalent american weapon to german mp40
  WP_GRENADE_PINEAPPLE, // 9   // allied hand grenade

  WP_STEN,          // 10   // silenced sten sub-machinegun
  WP_MEDIC_SYRINGE, // 11
  WP_AMMO,          // 12
  WP_ARTY,          // 13
  WP_SILENCER,      // 14   // silenced luger
  WP_DYNAMITE,      // 15
  WP_SMOKETRAIL,    // 16   // smoke grenade
  WP_MAPMORTAR,     // 17   // shooter_mortar
  VERYBIGEXPLOSION, // 18   // explosion effect for airplanes
  WP_MEDKIT,        // 19

  WP_BINOCULARS,   // 20
  WP_PLIERS,       // 21
  WP_SMOKE_MARKER, // 22
  WP_KAR98,        // 23	// axis engineer rifle
  WP_CARBINE,      // 24   // allied engineer rifle
  WP_GARAND,       // 25   // allied sniper rifle
  WP_LANDMINE,     // 26
  WP_SATCHEL,      // 27
  WP_SATCHEL_DET,  // 28
  WP_TRIPMINE,     // 29
  WP_SMOKE_BOMB,   // 30

  WP_MOBILE_MG42,  // 31
  WP_K43,          // 32   // axis sniper rifle
  WP_FG42,         // 33
  WP_DUMMY_MG42,   // 34   // Gordon: for storing heat on mounted mg42s...
  WP_MORTAR,       // 35
  WP_LOCKPICK,     // 36	// Mad Doc - TDF lockpick
  WP_AKIMBO_COLT,  // 37
  WP_AKIMBO_LUGER, // 38
  WP_PORTAL_GUN,   // 39   // Feen: Portal Gun Mod (PGM)

  // Gordon: ONLY secondaries below this mark,
  // as they are checked >= WP_GPG40 && < WP_NUM_WEAPONS

  WP_GPG40,                // 40   // axis rifle grenade
  WP_M7,                   // 41   // allied rifle grenade
  WP_SILENCED_COLT,        // 42
  WP_GARAND_SCOPE,         // 43
  WP_K43_SCOPE,            // 44
  WP_FG42SCOPE,            // 45
  WP_MORTAR_SET,           // 46
  WP_MEDIC_ADRENALINE,     // 47
  WP_AKIMBO_SILENCEDCOLT,  // 48
  WP_AKIMBO_SILENCEDLUGER, // 49
  WP_MOBILE_MG42_SET,      // 50

  WP_NUM_WEAPONS
};

static_assert(WP_NUM_WEAPONS < MAX_WEAPONS,
              "WP_NUM_WEAPONS must be less than MAX_WEAPONS");

// JPW NERVE moved from cg_weapons (now used in g_active) for drop command,
// actual array in bg_misc.c
extern int weapBanksMultiPlayer[MAX_WEAP_BANKS_MP][MAX_WEAPS_IN_BANK_MP];
// jpw

// TAT 10/4/2002
//		Using one unified list for which weapons can received ammo
//		This is used both by the ammo pack code and by the bot code to
// determine if reloads are needed
extern int reloadableWeapons[];

typedef struct {
  int kills, teamkills, killedby;
} weaponStats_t;

typedef enum {
  HR_HEAD,
  HR_ARMS,
  HR_BODY,
  HR_LEGS,
  HR_NUM_HITREGIONS,
} hitRegion_t;

typedef enum {
  SK_BATTLE_SENSE,
  SK_EXPLOSIVES_AND_CONSTRUCTION,
  SK_FIRST_AID,
  SK_SIGNALS,
  SK_LIGHT_WEAPONS,
  SK_HEAVY_WEAPONS,
  SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS,
  SK_NUM_SKILLS
} skillType_t;

extern const char *skillNames[SK_NUM_SKILLS];
extern const char *skillNamesLine1[SK_NUM_SKILLS];
extern const char *skillNamesLine2[SK_NUM_SKILLS];
extern const char *medalNames[SK_NUM_SKILLS];

inline constexpr int NUM_SKILL_LEVELS = 5;
extern const int skillLevels[NUM_SKILL_LEVELS];

typedef struct {
  weaponStats_t weaponStats[WP_NUM_WEAPONS];
  int suicides;
  int hitRegions[HR_NUM_HITREGIONS];
  int objectiveStats[MAX_OBJECTIVES];
} playerStats_t;

typedef struct ammotable_s {
  int maxammo;             //
  int uses;                //
  int maxclip;             //
  int defaultStartingAmmo; // Mad Doc - TDF
  int defaultStartingClip; // Mad Doc - TDF
  int reloadTime;          //
  int fireDelayTime;       //
  int nextShotTime;        //
                           //----(SA)	added
  int maxHeat;  // max active firing time before weapon 'overheats' (at
                // which point the weapon will fail)
  int coolRate; // how fast the weapon cools down. (per second)
                //----(SA)	end
  int mod;      // means of death
} ammotable_t;

// Lookup table to find ammo table entry
extern ammotable_t *GetAmmoTableData(int ammoIndex);

extern int weapAlts[]; // defined in bg_misc.c

//----(SA)
// for routines that need to check if a WP_ is </=/> a given set of weapons
inline constexpr weapon_t WP_BEGINSECONDARY = WP_GPG40;
inline constexpr weapon_t WP_LASTSECONDARY = WP_SILENCED_COLT;

// TTimo
// NOTE: what about WP_VENOM and other XP weapons?
// rain - #81 - added added akimbo weapons and deployed MG42
#define IS_AUTORELOAD_WEAPON(weapon)                                           \
  (weapon == WP_LUGER || weapon == WP_COLT || weapon == WP_MP40 ||             \
   weapon == WP_THOMPSON || weapon == WP_STEN || weapon == WP_KAR98 ||         \
   weapon == WP_CARBINE || weapon == WP_GARAND_SCOPE || weapon == WP_FG42 ||   \
   weapon == WP_K43 || weapon == WP_MOBILE_MG42 ||                             \
   weapon == WP_SILENCED_COLT || weapon == WP_SILENCER ||                      \
   weapon == WP_GARAND || weapon == WP_K43_SCOPE || weapon == WP_FG42SCOPE ||  \
   BG_IsAkimboWeapon(weapon) || weapon == WP_MOBILE_MG42_SET)

// shared flamethrower constants

inline constexpr float FLAME_START_SIZE = 1.0f;

// speed of flame as it leaves the nozzle
inline constexpr float FLAME_START_SPEED = 1200.0f;
inline constexpr float FLAME_MIN_SPEED = 60.0f;

// these are calculated (don't change)
// NOTE: only modify the range, since this should always reflect that range
inline constexpr int FLAME_LENGTH = FLAMETHROWER_RANGE + 50;

// life duration in milliseconds
inline constexpr int FLAME_LIFETIME =
    static_cast<int>((FLAME_LENGTH / FLAME_START_SPEED) * 1000);
inline constexpr float FLAME_FRICTION_PER_SEC = 2.0f * FLAME_START_SPEED;
// x is the current sizeMax
#define GET_FLAME_SIZE_SPEED(x) ((static_cast<float>(x) / FLAME_LIFETIME) / 0.3)

// entityState_t->event values
// entity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field will be incremented
// with each change in the event so that an identical event
// started twice in a row can be distinguished.
// And off the value with ~EV_EVENT_BITS to retrieve the actual event number
inline constexpr int EV_EVENT_BIT1 = 0x00000100;
inline constexpr int EV_EVENT_BIT2 = 0x00000200;
inline constexpr int EV_EVENT_BITS = EV_EVENT_BIT1 | EV_EVENT_BIT2;

typedef enum {
  EV_NONE,
  EV_FOOTSTEP,
  EV_FOOTSTEP_METAL,
  EV_FOOTSTEP_WOOD,
  EV_FOOTSTEP_GRASS,
  EV_FOOTSTEP_GRAVEL,
  EV_FOOTSTEP_ROOF,
  EV_FOOTSTEP_SNOW,
  EV_FOOTSTEP_CARPET,
  EV_FOOTSPLASH,
  EV_FOOTWADE,
  EV_SWIM,
  EV_STEP_4,
  EV_STEP_8,
  EV_STEP_12,
  EV_STEP_16,
  EV_FALL_SHORT,
  EV_FALL_MEDIUM,
  EV_FALL_FAR,
  EV_FALL_NDIE,
  EV_FALL_DMG_10,
  EV_FALL_DMG_15,
  EV_FALL_DMG_25,
  EV_FALL_DMG_50,
  EV_JUMP,
  EV_WATER_TOUCH,        // foot touches
  EV_WATER_LEAVE,        // foot leaves
  EV_WATER_UNDER,        // head touches
  EV_WATER_CLEAR,        // head leaves
  EV_ITEM_PICKUP,        // normal item pickups are predictable
  EV_ITEM_PICKUP_QUIET,  // (SA) same, but don't play the default pickup
                         // sound as it was specified in the ent
  EV_GLOBAL_ITEM_PICKUP, // powerup / team sounds are broadcast to
                         // everyone
  EV_NOAMMO,
  EV_WEAPONSWITCHED,
  EV_EMPTYCLIP,
  EV_FILL_CLIP,
  EV_MG42_FIXED, // JPW NERVE
  EV_WEAP_OVERHEAT,
  EV_CHANGE_WEAPON,
  EV_CHANGE_WEAPON_2,
  EV_FIRE_WEAPON,
  EV_FIRE_WEAPONB,
  EV_FIRE_WEAPON_LASTSHOT,
  EV_NOFIRE_UNDERWATER,
  EV_FIRE_WEAPON_MG42,
  EV_FIRE_WEAPON_MOUNTEDMG42,
  EV_ITEM_RESPAWN,
  EV_ITEM_POP,
  EV_PLAYER_TELEPORT_IN,
  EV_PLAYER_TELEPORT_OUT,
  EV_GRENADE_BOUNCE, // eventParm will be the soundindex
  EV_GENERAL_SOUND,
  EV_GENERAL_SOUND_VOLUME,
  EV_GENERAL_CLIENT_SOUND_VOLUME, // ETJump: play sound from the
                                  // specific entity to the client
  EV_GLOBAL_SOUND,                // no attenuation
  EV_GLOBAL_CLIENT_SOUND,         // DHM - Nerve :: no attenuation, only plays
                                  // for specified client
  EV_GLOBAL_TEAM_SOUND,           // no attenuation, team only
  EV_FX_SOUND,
  EV_BULLET_HIT_FLESH,
  EV_BULLET_HIT_WALL,
  EV_MISSILE_HIT,
  EV_MISSILE_MISS,
  EV_RAILTRAIL,
  EV_VENOM,
  EV_BULLET,   // otherEntity is the shooter
  EV_LOSE_HAT, //----(SA)
  EV_PAIN,
  EV_CROUCH_PAIN,
  EV_DEATH1,
  EV_DEATH2,
  EV_DEATH3,
  EV_OBITUARY,
  EV_STOPSTREAMINGSOUND, // JPW NERVE swiped from sherman
  EV_POWERUP_QUAD,
  EV_POWERUP_BATTLESUIT,
  EV_POWERUP_REGEN,
  EV_GIB_PLAYER, // gib a previously living player
  EV_DEBUG_LINE,
  EV_STOPLOOPINGSOUND,
  EV_TAUNT,
  EV_SMOKE,
  EV_SPARKS,
  EV_SPARKS_ELECTRIC,
  EV_EXPLODE, // func_explosive
  EV_RUBBLE,
  EV_EFFECT,    // target_effect
  EV_MORTAREFX, // mortar firing
  EV_SPINUP,    // JPW NERVE panzerfaust preamble
  EV_SNOW_ON,
  EV_SNOW_OFF,
  EV_MISSILE_MISS_SMALL,
  EV_MISSILE_MISS_LARGE,
  EV_MORTAR_IMPACT,
  EV_MORTAR_MISS,
  EV_SPIT_HIT,
  EV_SPIT_MISS,
  EV_SHARD,
  EV_JUNK,
  EV_EMITTER, //----(SA)	added // generic particle emitter that
              // uses
              // client-side particle scripts
  EV_OILPARTICLES,
  EV_OILSLICK,
  EV_OILSLICKREMOVE,
  EV_MG42EFX,
  EV_FLAKGUN1,
  EV_FLAKGUN2,
  EV_FLAKGUN3,
  EV_FLAKGUN4,
  EV_EXERT1,
  EV_EXERT2,
  EV_EXERT3,
  EV_SNOWFLURRY,
  EV_CONCUSSIVE,
  EV_DUST,
  EV_RUMBLE_EFX,
  EV_GUNSPARKS,
  EV_FLAMETHROWER_EFFECT,
  EV_POPUP,
  EV_POPUPBOOK,
  EV_GIVEPAGE,
  EV_MG42BULLET_HIT_FLESH, // Arnout: these two send the seed as well
  EV_MG42BULLET_HIT_WALL,
  EV_SHAKE,
  EV_DISGUISE_SOUND,
  EV_BUILDDECAYED_SOUND,
  EV_FIRE_WEAPON_AAGUN,
  EV_DEBRIS,
  EV_ALERT_SPEAKER,
  EV_POPUPMESSAGE,
  EV_ARTYMESSAGE,
  EV_AIRSTRIKEMESSAGE,
  EV_MEDIC_CALL,
  EV_PORTAL_TELEPORT,
  EV_LOAD_TELEPORT,
  EV_UPHILLSTEP,
  EV_PORTAL_TRAIL,
  EV_CUSHIONFALLSTEP,
  EV_MAX_EVENTS // just added as an 'endcap'
} entity_event_t;

static constexpr std::array<const char *, EV_MAX_EVENTS + 1> eventnames = {
    "EV_NONE",
    "EV_FOOTSTEP",
    "EV_FOOTSTEP_METAL",
    "EV_FOOTSTEP_WOOD",
    "EV_FOOTSTEP_GRASS",
    "EV_FOOTSTEP_GRAVEL",
    "EV_FOOTSTEP_ROOF",
    "EV_FOOTSTEP_SNOW",
    "EV_FOOTSTEP_CARPET",
    "EV_FOOTSPLASH",
    "EV_FOOTWADE",
    "EV_SWIM",
    "EV_STEP_4",
    "EV_STEP_8",
    "EV_STEP_12",
    "EV_STEP_16",
    "EV_FALL_SHORT",
    "EV_FALL_MEDIUM",
    "EV_FALL_FAR",
    "EV_FALL_NDIE",
    "EV_FALL_DMG_10",
    "EV_FALL_DMG_15",
    "EV_FALL_DMG_25",
    "EV_FALL_DMG_50",
    "EV_JUMP",
    "EV_WATER_TOUCH",
    "EV_WATER_LEAVE",
    "EV_WATER_UNDER",
    "EV_WATER_CLEAR",
    "EV_ITEM_PICKUP",
    "EV_ITEM_PICKUP_QUIET",
    "EV_GLOBAL_ITEM_PICKUP",
    "EV_NOAMMO",
    "EV_WEAPONSWITCHED",
    "EV_EMPTYCLIP",
    "EV_FILL_CLIP",
    "EV_MG42_FIXED",
    "EV_WEAP_OVERHEAT",
    "EV_CHANGE_WEAPON",
    "EV_CHANGE_WEAPON_2",
    "EV_FIRE_WEAPON",
    "EV_FIRE_WEAPONB",
    "EV_FIRE_WEAPON_LASTSHOT",
    "EV_NOFIRE_UNDERWATER",
    "EV_FIRE_WEAPON_MG42",
    "EV_FIRE_WEAPON_MOUNTEDMG42",
    "EV_ITEM_RESPAWN",
    "EV_ITEM_POP",
    "EV_PLAYER_TELEPORT_IN",
    "EV_PLAYER_TELEPORT_OUT",
    "EV_GRENADE_BOUNCE",
    "EV_GENERAL_SOUND",
    "EV_GENERAL_SOUND_VOLUME",
    "EV_GENERAL_CLIENT_SOUND_VOLUME",
    "EV_GLOBAL_SOUND",
    "EV_GLOBAL_CLIENT_SOUND",
    "EV_GLOBAL_TEAM_SOUND",
    "EV_FX_SOUND",
    "EV_BULLET_HIT_FLESH",
    "EV_BULLET_HIT_WALL",
    "EV_MISSILE_HIT",
    "EV_MISSILE_MISS",
    "EV_RAILTRAIL",
    "EV_VENOM",
    "EV_BULLET",
    "EV_LOSE_HAT",
    "EV_PAIN",
    "EV_CROUCH_PAIN",
    "EV_DEATH1",
    "EV_DEATH2",
    "EV_DEATH3",
    "EV_OBITUARY",
    "EV_STOPSTREAMINGSOUND",
    "EV_POWERUP_QUAD",
    "EV_POWERUP_BATTLESUIT",
    "EV_POWERUP_REGEN",
    "EV_GIB_PLAYER",
    "EV_DEBUG_LINE",
    "EV_STOPLOOPINGSOUND",
    "EV_TAUNT",
    "EV_SMOKE",
    "EV_SPARKS",
    "EV_SPARKS_ELECTRIC",
    "EV_EXPLODE",
    "EV_RUBBLE",
    "EV_EFFECT",
    "EV_MORTAREFX",
    "EV_SPINUP",
    "EV_SNOW_ON",
    "EV_SNOW_OFF",
    "EV_MISSILE_MISS_SMALL",
    "EV_MISSILE_MISS_LARGE",
    "EV_MORTAR_IMPACT",
    "EV_MORTAR_MISS",
    "EV_SPIT_HIT",
    "EV_SPIT_MISS",
    "EV_SHARD",
    "EV_JUNK",
    "EV_EMITTER",
    "EV_OILPARTICLES",
    "EV_OILSLICK",
    "EV_OILSLICKREMOVE",
    "EV_MG42EFX",
    "EV_FLAKGUN1",
    "EV_FLAKGUN2",
    "EV_FLAKGUN3",
    "EV_FLAKGUN4",
    "EV_EXERT1",
    "EV_EXERT2",
    "EV_EXERT3",
    "EV_SNOWFLURRY",
    "EV_CONCUSSIVE",
    "EV_DUST",
    "EV_RUMBLE_EFX",
    "EV_GUNSPARKS",
    "EV_FLAMETHROWER_EFFECT",
    "EV_POPUP",
    "EV_POPUPBOOK",
    "EV_GIVEPAGE",
    "EV_MG42BULLET_HIT_FLESH",
    "EV_MG42BULLET_HIT_WALL",
    "EV_SHAKE",
    "EV_DISGUISE_SOUND",
    "EV_BUILDDECAYED_SOUND",
    "EV_FIRE_WEAPON_AAGUN",
    "EV_DEBRIS",
    "EV_ALERT_SPEAKER",
    "EV_POPUPMESSAGE",
    "EV_ARTYMESSAGE",
    "EV_AIRSTRIKEMESSAGE",
    "EV_MEDIC_CALL",
    "EV_PORTAL_TELEPORT",
    "EV_LOAD_TELEPORT",
    "EV_UPHILLSTEP",
    "EV_SAVE",
    "EV_CUSHIONFALLSTEP",
    "EV_MAX_EVENTS",
};

static_assert(sizeof(eventnames) / sizeof(eventnames[0]) == EV_MAX_EVENTS + 1,
              "Event names array size does not match enum list");

// new (10/18/00)
typedef enum {
  BOTH_DEATH1,
  BOTH_DEAD1,
  BOTH_DEAD1_WATER,
  BOTH_DEATH2,
  BOTH_DEAD2,
  BOTH_DEAD2_WATER,
  BOTH_DEATH3,
  BOTH_DEAD3,
  BOTH_DEAD3_WATER,

  BOTH_CLIMB,
  /*10*/ BOTH_CLIMB_DOWN,
  BOTH_CLIMB_DISMOUNT,

  BOTH_SALUTE,

  BOTH_PAIN1,        // head
  BOTH_PAIN2,        // chest
  BOTH_PAIN3,        // groin
  BOTH_PAIN4,        // right shoulder
  BOTH_PAIN5,        // left shoulder
  BOTH_PAIN6,        // right knee
  BOTH_PAIN7,        // left knee
  /*20*/ BOTH_PAIN8, // dazed

  BOTH_GRAB_GRENADE,

  BOTH_ATTACK1,
  BOTH_ATTACK2,
  BOTH_ATTACK3,
  BOTH_ATTACK4,
  BOTH_ATTACK5,

  BOTH_EXTRA1,
  BOTH_EXTRA2,
  BOTH_EXTRA3,
  /*30*/ BOTH_EXTRA4,
  BOTH_EXTRA5,
  BOTH_EXTRA6,
  BOTH_EXTRA7,
  BOTH_EXTRA8,
  BOTH_EXTRA9,
  BOTH_EXTRA10,
  BOTH_EXTRA11,
  BOTH_EXTRA12,
  BOTH_EXTRA13,
  /*40*/ BOTH_EXTRA14,
  BOTH_EXTRA15,
  BOTH_EXTRA16,
  BOTH_EXTRA17,
  BOTH_EXTRA18,
  BOTH_EXTRA19,
  BOTH_EXTRA20,

  TORSO_GESTURE,
  TORSO_GESTURE2,
  TORSO_GESTURE3,
  /*50*/ TORSO_GESTURE4,

  TORSO_DROP,

  TORSO_RAISE, // (low)
  TORSO_ATTACK,
  TORSO_STAND,
  TORSO_STAND_ALT1,
  TORSO_STAND_ALT2,
  TORSO_READY,
  TORSO_RELAX,

  TORSO_RAISE2, // (high)
  /*60*/ TORSO_ATTACK2,
  TORSO_STAND2,
  TORSO_STAND2_ALT1,
  TORSO_STAND2_ALT2,
  TORSO_READY2,
  TORSO_RELAX2,

  TORSO_RAISE3, // (pistol)
  TORSO_ATTACK3,
  TORSO_STAND3,
  TORSO_STAND3_ALT1,
  /*70*/ TORSO_STAND3_ALT2,
  TORSO_READY3,
  TORSO_RELAX3,

  TORSO_RAISE4, // (shoulder)
  TORSO_ATTACK4,
  TORSO_STAND4,
  TORSO_STAND4_ALT1,
  TORSO_STAND4_ALT2,
  TORSO_READY4,
  TORSO_RELAX4,

  /*80*/ TORSO_RAISE5, // (throw)
  TORSO_ATTACK5,
  TORSO_ATTACK5B,
  TORSO_STAND5,
  TORSO_STAND5_ALT1,
  TORSO_STAND5_ALT2,
  TORSO_READY5,
  TORSO_RELAX5,

  TORSO_RELOAD1,        // (low)
  TORSO_RELOAD2,        // (high)
  /*90*/ TORSO_RELOAD3, // (pistol)
  TORSO_RELOAD4,        // (shoulder)

  TORSO_MG42, // firing tripod mounted weapon animation

  TORSO_MOVE, // torso anim to play while moving and not firing
              // (swinging arms type thing)
  TORSO_MOVE_ALT,

  TORSO_EXTRA,
  TORSO_EXTRA2,
  TORSO_EXTRA3,
  TORSO_EXTRA4,
  TORSO_EXTRA5,
  /*100*/ TORSO_EXTRA6,
  TORSO_EXTRA7,
  TORSO_EXTRA8,
  TORSO_EXTRA9,
  TORSO_EXTRA10,

  LEGS_WALKCR,
  LEGS_WALKCR_BACK,
  LEGS_WALK,
  LEGS_RUN,
  LEGS_BACK,
  /*110*/ LEGS_SWIM,
  LEGS_SWIM_IDLE,

  LEGS_JUMP,
  LEGS_JUMPB,
  LEGS_LAND,

  LEGS_IDLE,
  LEGS_IDLE_ALT, // LEGS_IDLE2
  LEGS_IDLECR,

  LEGS_TURN,

  LEGS_BOOT, // kicking animation

  /*120*/ LEGS_EXTRA1,
  LEGS_EXTRA2,
  LEGS_EXTRA3,
  LEGS_EXTRA4,
  LEGS_EXTRA5,
  LEGS_EXTRA6,
  LEGS_EXTRA7,
  LEGS_EXTRA8,
  LEGS_EXTRA9,
  LEGS_EXTRA10,

  /*130*/ MAX_ANIMATIONS
} animNumber_t;

// text represenation for scripting
extern const char *animStrings[];    // defined in bg_misc.c
extern const char *animStringsOld[]; // defined in bg_misc.c

typedef enum {
  WEAP_IDLE1,
  WEAP_IDLE2,
  WEAP_ATTACK1,
  WEAP_ATTACK2,
  WEAP_ATTACK_LASTSHOT, // used when firing the last round before having
                        // an empty clip.
  WEAP_DROP,
  WEAP_RAISE,
  WEAP_RELOAD1,
  WEAP_RELOAD2,
  WEAP_RELOAD3,
  WEAP_ALTSWITCHFROM, // switch from alt fire mode weap
                      // (scoped/silencer/etc)
  WEAP_ALTSWITCHTO,   // switch to alt fire mode weap
  WEAP_DROP2,
  MAX_WP_ANIMATIONS
} weapAnimNumber_t;

typedef enum hudHeadAnimNumber_s {
  HD_IDLE1,
  HD_IDLE2,
  HD_IDLE3,
  HD_IDLE4,
  HD_IDLE5,
  HD_IDLE6,
  HD_IDLE7,
  HD_IDLE8,
  HD_DAMAGED_IDLE1,
  HD_DAMAGED_IDLE2,
  HD_DAMAGED_IDLE3,
  HD_LEFT,
  HD_RIGHT,
  HD_ATTACK,
  HD_ATTACK_END,
  HD_PAIN,
  MAX_HD_ANIMATIONS
} hudHeadAnimNumber_t;

inline constexpr int ANIMFL_LADDERANIM = 0x1;
inline constexpr int ANIMFL_FIRINGANIM = 0x2;
inline constexpr int ANIMFL_REVERSED = 0x4;

typedef struct animation_s {
#ifdef CGAMEDLL
  qhandle_t mdxFile;
#else
  char mdxFileName[MAX_QPATH];
#endif // CGAMEDLL
  char name[MAX_QPATH];
  int firstFrame;
  int numFrames;
  int loopFrames;  // 0 to numFrames
  int frameLerp;   // msec between frames
  int initialLerp; // msec to get to first frame
  int moveSpeed;
  int animBlend; // take this long to blend to next anim

  //
  // derived
  //
  int duration;
  int nameHash;
  int flags;
  int movetype;
} animation_t;

// Ridah, head animations
typedef enum {
  HEAD_NEUTRAL_CLOSED,
  HEAD_NEUTRAL_A,
  HEAD_NEUTRAL_O,
  HEAD_NEUTRAL_I,
  HEAD_NEUTRAL_E,
  HEAD_HAPPY_CLOSED,
  HEAD_HAPPY_O,
  HEAD_HAPPY_I,
  HEAD_HAPPY_E,
  HEAD_HAPPY_A,
  HEAD_ANGRY_CLOSED,
  HEAD_ANGRY_O,
  HEAD_ANGRY_I,
  HEAD_ANGRY_E,
  HEAD_ANGRY_A,

  MAX_HEAD_ANIMS
} animHeadNumber_t;

typedef struct headAnimation_s {
  int firstFrame;
  int numFrames;
} headAnimation_t;
// done.

// flip the togglebit every time an animation
// changes so a restart of the same anim can be detected
inline constexpr int ANIM_TOGGLEBIT = 1 << (ANIM_BITS - 1);

// Gordon: renamed these to team_axis/allies, it really was awful....
enum team_t : int8_t {
  TEAM_FREE,
  TEAM_AXIS,
  TEAM_ALLIES,
  TEAM_SPECTATOR,

  TEAM_NUM_TEAMS
};

// OSP - weapon stat info: mapping between MOD_ and WP_ types (FIXME for new ET
// weapons)
typedef enum extWeaponStats_s {
  WS_KNIFE,        // 0
  WS_LUGER,        // 1
  WS_COLT,         // 2
  WS_MP40,         // 3
  WS_THOMPSON,     // 4
  WS_STEN,         // 5
  WS_FG42,         // 6	-- Also includes WS_BAR (allies version of fg42)
  WS_PANZERFAUST,  // 7
  WS_FLAMETHROWER, // 8
  WS_GRENADE,      // 9	-- Includes axis and allies grenade types
  WS_MORTAR,       // 10
  WS_DYNAMITE,     // 11
  WS_AIRSTRIKE,    // 12	-- Lt. smoke grenade attack
  WS_ARTILLERY,    // 13	-- Lt. binocular attack
  WS_SYRINGE,      // 14	-- Medic syringe uses/successes

  WS_SMOKE,           // 15
  WS_SATCHEL,         // 16
  WS_GRENADELAUNCHER, // 17
  WS_LANDMINE,        // 18
  WS_MG42,            // 19
  WS_GARAND,          // 20 // Gordon: (carbine and garand)
  WS_K43,             // 21 // Gordon: (kar98 and k43)
  WS_PORTAL_GUN,      // Feen: PGM

  WS_MAX
} extWeaponStats_t;

typedef struct {
  qboolean fHasHeadShots;
  const char *pszCode;
  const char *pszName;
} weap_ws_t;

extern const weap_ws_t aWeaponInfo[WS_MAX];
// OSP

// means of death
typedef enum {
  MOD_UNKNOWN,
  MOD_MACHINEGUN,
  MOD_BROWNING,
  MOD_MG42,
  MOD_GRENADE,
  MOD_ROCKET,

  // (SA) modified wolf weap mods
  MOD_KNIFE,
  MOD_LUGER,
  MOD_COLT,
  MOD_MP40,
  MOD_THOMPSON,
  MOD_STEN,
  MOD_GARAND,
  MOD_SNOOPERSCOPE,
  MOD_SILENCER, //----(SA)
  MOD_FG42,
  MOD_FG42SCOPE,
  MOD_PANZERFAUST,
  MOD_GRENADE_LAUNCHER,
  MOD_FLAMETHROWER,
  MOD_GRENADE_PINEAPPLE,
  MOD_CROSS,
  // end

  MOD_MAPMORTAR,
  MOD_MAPMORTAR_SPLASH,

  MOD_KICKED,
  MOD_GRABBER,

  MOD_DYNAMITE,
  MOD_AIRSTRIKE, // JPW NERVE
  MOD_SYRINGE,   // JPW NERVE
  MOD_AMMO,      // JPW NERVE
  MOD_ARTY,      // JPW NERVE

  MOD_WATER,
  MOD_SLIME,
  MOD_LAVA,
  MOD_CRUSH,
  MOD_TELEFRAG,
  MOD_FALLING,
  MOD_SUICIDE,
  MOD_TARGET_LASER,
  MOD_TRIGGER_HURT,
  MOD_EXPLOSIVE,

  MOD_CARBINE,
  MOD_KAR98,
  MOD_GPG40,
  MOD_M7,
  MOD_LANDMINE,
  MOD_SATCHEL,
  MOD_TRIPMINE,
  MOD_SMOKEBOMB,
  MOD_MOBILE_MG42,
  MOD_SILENCED_COLT,
  MOD_GARAND_SCOPE,

  MOD_CRUSH_CONSTRUCTION,
  MOD_CRUSH_CONSTRUCTIONDEATH,
  MOD_CRUSH_CONSTRUCTIONDEATH_NOATTACKER,

  MOD_K43,
  MOD_K43_SCOPE,

  MOD_MORTAR,

  MOD_AKIMBO_COLT,
  MOD_AKIMBO_LUGER,
  MOD_AKIMBO_SILENCEDCOLT,
  MOD_AKIMBO_SILENCEDLUGER,

  MOD_SMOKEGRENADE,

  // Feen: PGM
  MOD_PORTAL_GUN,

  // RF
  MOD_SWAP_PLACES,

  // OSP -- keep these 2 entries last
  MOD_SWITCHTEAM,

  MOD_NUM_MODS

} meansOfDeath_t;

//---------------------------------------------------------

// gitem_t->type
typedef enum {
  IT_BAD,
  IT_WEAPON, // EFX: rotate + upscale + minlight

  IT_AMMO,     // EFX: rotate
  IT_ARMOR,    // EFX: rotate + minlight
  IT_HEALTH,   // EFX: static external sphere + rotating internal
  IT_HOLDABLE, // single use, holdable item
               // EFX: rotate + bob
  IT_KEY,
  IT_TREASURE, // gold bars, etc.  things that can be picked up and
               // counted for a tally at end-level
  IT_TEAM,
} itemType_t;

inline constexpr int MAX_ITEM_MODELS = 3;
inline constexpr int MAX_ITEM_ICONS = 4;

// JOSEPH 4-17-00
typedef struct gitem_s {
  const char *classname; // spawning name
  const char *pickup_sound;
  const char *world_model[MAX_ITEM_MODELS];

  const char *icon;
  const char *ammoicon;
  const char *pickup_name; // for printing on pickup

  int quantity;      // for ammo how much, or duration of powerup (value not
                     // necessary for ammo/health.  that value set in
                     // gameskillnumber[] below)
  itemType_t giType; // IT_* flags

  int giTag;

  int giAmmoIndex; // type of weapon ammo this uses.  (ex. WP_MP40 and
                   // WP_LUGER share 9mm ammo, so they both have
                   // WP_LUGER for giAmmoIndex)
  int giClipIndex; // which clip this weapon uses.  this allows the
                   // sniper rifle to use the same clip as the garand,
                   // etc.

  const char *precaches; // string of all models and images this item will use
  const char *sounds;    // string of all sounds this item will use

  //	int			gameskillnumber[5];
} gitem_t;
// END JOSEPH

// included in both the game dll and the client
extern gitem_t bg_itemlist[];
extern int bg_numItems;

gitem_t *BG_FindItem(const char *pickupName);
gitem_t *BG_FindItemForClassName(const char *className);
gitem_t *BG_FindItemForWeapon(weapon_t weapon);
gitem_t *BG_FindItemForPowerup(powerup_t pw);
gitem_t *BG_FindItemForHoldable(holdable_t pw);
gitem_t *BG_FindItemForAmmo(int weapon);
weapon_t BG_FindAmmoForWeapon(weapon_t weapon);
weapon_t BG_FindClipForWeapon(weapon_t weapon);

qboolean BG_AkimboFireSequence(int weapon, int akimboClip, int mainClip);
qboolean BG_IsAkimboWeapon(int weaponNum);
qboolean BG_IsAkimboSideArm(int weaponNum, playerState_t *ps);
int BG_AkimboSidearm(int weaponNum);

#define ITEM_INDEX(x) ((x) - bg_itemlist)

qboolean BG_CanItemBeGrabbed(const entityState_t *ent, const playerState_t *ps,
                             int *skill, int teamNum);
qboolean BG_WeaponIsExplosive(int weap);
bool BG_WeaponDisallowedInTimeruns(int weap);
bool BG_WeaponHasAmmo(playerState_t *ps, int weap);

// content masks

inline constexpr uint32_t MASK_SOLID = CONTENTS_SOLID;

inline constexpr uint32_t MASK_PLAYERSOLID =
    CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY;
inline constexpr uint32_t MASK_DEADSOLID = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
inline constexpr uint32_t MASK_WATER =
    CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME;
inline constexpr uint32_t MASK_OPAQUE = CONTENTS_SOLID | CONTENTS_LAVA;
inline constexpr uint32_t MASK_SHOT =
    CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE;
inline constexpr uint32_t MASK_MISSILESHOT = MASK_SHOT | CONTENTS_MISSILECLIP;

// portalgun trace hits solid objects, players and portalclips
// corpses are ignored, and players only block portals if they are solid
inline constexpr uint32_t MASK_PORTAL =
    CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_PORTALCLIP;

//
// entityState_t->eType
//

// cursorhints (stored in ent->s.dmgFlags since that's only used for players at
// the moment)
typedef enum {
  HINT_NONE,      // reserved
  HINT_FORCENONE, // reserved
  HINT_PLAYER,
  HINT_ACTIVATE,
  HINT_DOOR,
  HINT_DOOR_ROTATING,
  HINT_DOOR_LOCKED,
  HINT_DOOR_ROTATING_LOCKED,
  HINT_MG42,
  HINT_BREAKABLE,
  HINT_BREAKABLE_DYNAMITE,
  HINT_CHAIR,
  HINT_ALARM,
  HINT_HEALTH,
  HINT_TREASURE,
  HINT_KNIFE,
  HINT_LADDER,
  HINT_BUTTON,
  HINT_WATER,
  HINT_CAUTION,
  HINT_DANGER,
  HINT_SECRET,
  HINT_QUESTION,
  HINT_EXCLAMATION,
  HINT_CLIPBOARD,
  HINT_WEAPON,
  HINT_AMMO,
  HINT_ARMOR,
  HINT_POWERUP,
  HINT_HOLDABLE,
  HINT_INVENTORY,
  HINT_SCENARIC,
  HINT_EXIT,
  HINT_NOEXIT,
  HINT_PLYR_FRIEND,
  HINT_PLYR_NEUTRAL,
  HINT_PLYR_ENEMY,
  HINT_PLYR_UNKNOWN,
  HINT_BUILD,    // DHM - Nerve
  HINT_DISARM,   // DHM - Nerve
  HINT_REVIVE,   // DHM - Nerve
  HINT_DYNAMITE, // DHM - Nerve
  HINT_CONSTRUCTIBLE,
  HINT_UNIFORM,
  HINT_LANDMINE,
  HINT_TANK,
  HINT_SATCHELCHARGE,
  HINT_LOCKPICK,

  HINT_BAD_USER, // invisible user with no target

  HINT_NUM_HINTS
} hintType_t;

static constexpr std::array<const char *, HINT_NUM_HINTS + 1> hintStrings = {
    "",          // HINT_NONE
    "HINT_NONE", // actually HINT_FORCENONE, but since this is being specified
                 // in the ent, the designer actually means HINT_FORCENONE
    "HINT_PLAYER", "HINT_ACTIVATE", "HINT_DOOR", "HINT_DOOR_ROTATING",
    "HINT_DOOR_LOCKED", "HINT_DOOR_ROTATING_LOCKED", "HINT_MG42",
    "HINT_BREAKABLE", "HINT_BREAKABLE_BIG", "HINT_CHAIR", "HINT_ALARM",
    "HINT_HEALTH", "HINT_TREASURE", "HINT_KNIFE", "HINT_LADDER", "HINT_BUTTON",
    "HINT_WATER", "HINT_CAUTION", "HINT_DANGER", "HINT_SECRET", "HINT_QUESTION",
    "HINT_EXCLAMATION", "HINT_CLIPBOARD", "HINT_WEAPON", "HINT_AMMO",
    "HINT_ARMOR", "HINT_POWERUP", "HINT_HOLDABLE", "HINT_INVENTORY",
    "HINT_SCENARIC", "HINT_EXIT", "HINT_NOEXIT", "HINT_PLYR_FRIEND",
    "HINT_PLYR_NEUTRAL", "HINT_PLYR_ENEMY", "HINT_PLYR_UNKNOWN",
    "HINT_BUILD",    // DHM - Nerve
    "HINT_DISARM",   // DHM - Nerve
    "HINT_REVIVE",   // DHM - Nerve
    "HINT_DYNAMITE", // DHM - Nerve

    "HINT_CONSTRUCTIBLE", "HINT_UNIFORM", "HINT_LANDMINE", "HINT_TANK",
    "HINT_SATCHELCHARGE",
    // START Mad Doc - TDF
    "HINT_LOCKPICK",
    // END Mad Doc - TDF

    "", // HINT_BAD_USER
};

// cursorhint trace distances
// most of these are server only and could live somewhere in qagame headers,
// but to avoid any accidents with mismatched distances, they should live here

inline constexpr float CH_DIST = 100.0f;
inline constexpr float CH_KNIFE_DIST = 48.0f;
inline constexpr float CH_LADDER_DIST = 100.0f;
inline constexpr float CH_WATER_DIST = 100.0f;
inline constexpr float CH_BREAKABLE_DIST = 64.0f;
inline constexpr float CH_DOOR_DIST = 96.0f;
inline constexpr float CH_ACTIVATE_DIST = 96.0f;
inline constexpr float CH_FRIENDLY_DIST = 1024.0f;

// use the largest value from above
inline constexpr float CH_MAX_DIST = 1024.0f;
// max dist for zooming hints
inline constexpr float CH_MAX_DIST_ZOOM = 8192.0f;

void BG_EvaluateTrajectory(const trajectory_t *tr, int atTime, vec3_t result,
                           qboolean isAngle, int splinePath);
void BG_EvaluateTrajectoryDelta(const trajectory_t *tr, int atTime,
                                vec3_t result, qboolean isAngle,
                                int splineData);
void BG_GetMarkDir(const vec3_t dir, const vec3_t normal, vec3_t out);

void BG_AddPredictableEventToPlayerstate(int newEvent, int eventParm,
                                         playerState_t *ps);

void BG_PlayerStateToEntityState(playerState_t *ps, entityState_t *s,
                                 qboolean snap);
void BG_PlayerStateToEntityStateExtraPolate(playerState_t *ps, entityState_t *s,
                                            int time, qboolean snap);
weapon_t BG_DuplicateWeapon(weapon_t weap);
gitem_t *BG_ValidStatWeapon(weapon_t weap);
weapon_t BG_WeaponForMOD(int MOD);

qboolean BG_WeaponInWolfMP(int weapon);
bool BG_PlayerTouchesItem(playerState_t *ps, entityState_t *item, int atTime);
int BG_GrenadesForClass(int cls, int *skills);
weapon_t BG_GrenadeTypeForTeam(team_t team);

qboolean BG_PlayerSeesItem(playerState_t *ps, entityState_t *item, int atTime);
qboolean BG_CheckMagicAmmo(const playerState_t *ps, int *skill, int teamNum);
qboolean BG_AddMagicAmmo(playerState_t *ps, int *skill, int teamNum,
                         int numOfClips);

bool BG_DropItems(int contents, int shared);

inline constexpr float OVERCLIP = 1.001f;

//----(SA)	removed PM_ammoNeeded 11/27/00
void PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce);

typedef enum {
  FOOTSTEP_NORMAL,
  FOOTSTEP_METAL,
  FOOTSTEP_WOOD,
  FOOTSTEP_GRASS,
  FOOTSTEP_GRAVEL,
  FOOTSTEP_SPLASH,
  FOOTSTEP_ROOF,
  FOOTSTEP_SNOW,
  FOOTSTEP_CARPET,

  FOOTSTEP_TOTAL
} footstep_t;

typedef enum {
  BRASSSOUND_METAL = 0,
  BRASSSOUND_SOFT,
  BRASSSOUND_STONE,
  BRASSSOUND_WOOD,
  BRASSSOUND_MAX,
} brassSound_t;

typedef enum {
  FXTYPE_WOOD = 0,
  FXTYPE_GLASS,
  FXTYPE_METAL,
  FXTYPE_GIBS,
  FXTYPE_BRICK,
  FXTYPE_STONE
} fxType_t;

//==================================================================
// New Animation Scripting Defines

inline constexpr int MAX_ANIMSCRIPT_MODELS = 32;
inline constexpr int MAX_ANIMSCRIPT_ITEMS_PER_MODEL = 2048;
inline constexpr int MAX_MODEL_ANIMATIONS = 512; // animations per model
inline constexpr int MAX_ANIMSCRIPT_ANIMCOMMANDS = 8;
inline constexpr int MAX_ANIMSCRIPT_ITEMS = 128;
// NOTE: these must all be in sync with string tables in bg_animation.c

typedef enum {
  ANIM_MT_UNUSED,
  ANIM_MT_IDLE,
  ANIM_MT_IDLECR,
  ANIM_MT_WALK,
  ANIM_MT_WALKBK,
  ANIM_MT_WALKCR,
  ANIM_MT_WALKCRBK,
  ANIM_MT_RUN,
  ANIM_MT_RUNBK,
  ANIM_MT_SWIM,
  ANIM_MT_SWIMBK,
  ANIM_MT_STRAFERIGHT,
  ANIM_MT_STRAFELEFT,
  ANIM_MT_TURNRIGHT,
  ANIM_MT_TURNLEFT,
  ANIM_MT_CLIMBUP,
  ANIM_MT_CLIMBDOWN,
  ANIM_MT_FALLEN, // DHM - Nerve :: dead, before limbo
  ANIM_MT_PRONE,
  ANIM_MT_PRONEBK,
  ANIM_MT_IDLEPRONE,
  ANIM_MT_FLAILING,
  //	ANIM_MT_TALK,
  ANIM_MT_SNEAK,
  ANIM_MT_AFTERBATTLE, // xkan, 1/8/2003, just finished battle

  NUM_ANIM_MOVETYPES
} scriptAnimMoveTypes_t;

typedef enum {
  ANIM_ET_PAIN,
  ANIM_ET_DEATH,
  ANIM_ET_FIREWEAPON,
  ANIM_ET_FIREWEAPON2,
  ANIM_ET_JUMP,
  ANIM_ET_JUMPBK,
  ANIM_ET_LAND,
  ANIM_ET_DROPWEAPON,
  ANIM_ET_RAISEWEAPON,
  ANIM_ET_CLIMB_MOUNT,
  ANIM_ET_CLIMB_DISMOUNT,
  ANIM_ET_RELOAD,
  ANIM_ET_PICKUPGRENADE,
  ANIM_ET_KICKGRENADE,
  ANIM_ET_QUERY,
  ANIM_ET_INFORM_FRIENDLY_OF_ENEMY,
  ANIM_ET_KICK,
  ANIM_ET_REVIVE,
  ANIM_ET_FIRSTSIGHT,
  ANIM_ET_ROLL,
  ANIM_ET_FLIP,
  ANIM_ET_DIVE,
  ANIM_ET_PRONE_TO_CROUCH,
  ANIM_ET_BULLETIMPACT,
  ANIM_ET_INSPECTSOUND,
  ANIM_ET_SECONDLIFE,
  ANIM_ET_DO_ALT_WEAPON_MODE,
  ANIM_ET_UNDO_ALT_WEAPON_MODE,
  ANIM_ET_DO_ALT_WEAPON_MODE_PRONE,
  ANIM_ET_UNDO_ALT_WEAPON_MODE_PRONE,
  ANIM_ET_FIREWEAPONPRONE,
  ANIM_ET_FIREWEAPON2PRONE,
  ANIM_ET_RAISEWEAPONPRONE,
  ANIM_ET_RELOADPRONE,
  ANIM_ET_TALK,
  ANIM_ET_NOPOWER,

  NUM_ANIM_EVENTTYPES
} scriptAnimEventTypes_t;

typedef enum {
  ANIM_BP_UNUSED,
  ANIM_BP_LEGS,
  ANIM_BP_TORSO,
  ANIM_BP_BOTH,

  NUM_ANIM_BODYPARTS
} animBodyPart_t;

typedef enum {
  ANIM_COND_WEAPON,
  ANIM_COND_ENEMY_POSITION,
  ANIM_COND_ENEMY_WEAPON,
  ANIM_COND_UNDERWATER,
  ANIM_COND_MOUNTED,
  ANIM_COND_MOVETYPE,
  ANIM_COND_UNDERHAND,
  ANIM_COND_LEANING,
  ANIM_COND_IMPACT_POINT,
  ANIM_COND_CROUCHING,
  ANIM_COND_STUNNED,
  ANIM_COND_FIRING,
  ANIM_COND_SHORT_REACTION,
  ANIM_COND_ENEMY_TEAM,
  ANIM_COND_PARACHUTE,
  ANIM_COND_CHARGING,
  ANIM_COND_SECONDLIFE,
  ANIM_COND_HEALTH_LEVEL,
  ANIM_COND_FLAILING_TYPE,
  ANIM_COND_GEN_BITFLAG, // xkan 1/15/2003 - general bit flags (to save
                         // some space)
  ANIM_COND_AISTATE,     // xkan 1/17/2003 - our current ai state (sometimes
                         // more convenient than creating a separate
                         // section)

  NUM_ANIM_CONDITIONS
} scriptAnimConditions_t;

//-------------------------------------------------------------------

typedef struct {
  const char *string;
  int hash;
} animStringItem_t;

typedef struct {
  int index;    // reference into the table of possible conditionals
  int value[2]; // can store anything from weapon bits, to position
                // enums, etc
} animScriptCondition_t;

typedef struct {
  short int bodyPart[2];  // play this animation on legs/torso/both
  short int animIndex[2]; // animation index in our list of animations
  short int animDuration[2];
  short int soundIndex;
} animScriptCommand_t;

typedef struct {
  int numConditions;
  animScriptCondition_t conditions[NUM_ANIM_CONDITIONS];
  int numCommands;
  animScriptCommand_t commands[MAX_ANIMSCRIPT_ANIMCOMMANDS];
} animScriptItem_t;

typedef struct {
  int numItems;
  animScriptItem_t *items[MAX_ANIMSCRIPT_ITEMS]; // pointers into a
                                                 // global list of items
} animScript_t;

typedef struct {
  char animationGroup[MAX_QPATH];
  char animationScript[MAX_QPATH];

  // parsed from the start of the cfg file (this is basically obsolete
  // now - need to get rid of it)
  gender_t gender;
  footstep_t footsteps;
  vec3_t headOffset;
  int version;
  qboolean isSkeletal;

  // parsed from animgroup file
  animation_t
      *animations[MAX_MODEL_ANIMATIONS]; // anim names, frame ranges, etc
  headAnimation_t headAnims[MAX_HEAD_ANIMS];
  int numAnimations, numHeadAnims;

  // parsed from script file
  animScript_t scriptAnims[MAX_AISTATES]
                          [NUM_ANIM_MOVETYPES];       // locomotive anims, etc
  animScript_t scriptCannedAnims[NUM_ANIM_MOVETYPES]; // played randomly
  animScript_t scriptEvents[NUM_ANIM_EVENTTYPES];     // events that trigger
                                                      // special anims

  // global list of script items for this model
  animScriptItem_t scriptItems[MAX_ANIMSCRIPT_ITEMS_PER_MODEL];
  int numScriptItems;

} animModelInfo_t;

// this is the main structure that is duplicated on the client and server
typedef struct {
  //	int					clientModels[MAX_CLIENTS];
  //// so we know which model each client is using
  animModelInfo_t modelInfo[MAX_ANIMSCRIPT_MODELS];
  int clientConditions[MAX_CLIENTS][NUM_ANIM_CONDITIONS][2];
  //
  // pointers to functions from the owning module
  //
  // TTimo: constify the arg
  int (*soundIndex)(const char *name);
  void (*playSound)(int soundIndex, vec3_t org, int clientNum);
} animScriptData_t;

//------------------------------------------------------------------
// Conditional Constants

typedef enum {
  POSITION_UNUSED,
  POSITION_BEHIND,
  POSITION_INFRONT,
  POSITION_RIGHT,
  POSITION_LEFT,

  NUM_ANIM_COND_POSITIONS
} animScriptPosition_t;

typedef enum {
  MOUNTED_UNUSED,
  MOUNTED_MG42,
  MOUNTED_AAGUN,

  NUM_ANIM_COND_MOUNTED
} animScriptMounted_t;

typedef enum {
  LEANING_UNUSED,
  LEANING_RIGHT,
  LEANING_LEFT,

  NUM_ANIM_COND_LEANING
} animScriptLeaning_t;

typedef enum {
  IMPACTPOINT_UNUSED,
  IMPACTPOINT_HEAD,
  IMPACTPOINT_CHEST,
  IMPACTPOINT_GUT,
  IMPACTPOINT_GROIN,
  IMPACTPOINT_SHOULDER_RIGHT,
  IMPACTPOINT_SHOULDER_LEFT,
  IMPACTPOINT_KNEE_RIGHT,
  IMPACTPOINT_KNEE_LEFT,

  NUM_ANIM_COND_IMPACTPOINT
} animScriptImpactPoint_t;

typedef enum {
  FLAILING_UNUSED,
  FLAILING_INAIR,
  FLAILING_VCRASH,
  FLAILING_HCRASH,

  NUM_ANIM_COND_FLAILING
} animScriptFlailingType_t;

typedef enum {
  /*	ANIM_BITFLAG_SNEAKING,
      ANIM_BITFLAG_AFTERBATTLE,*/
  ANIM_BITFLAG_ZOOMING,

  NUM_ANIM_COND_BITFLAG
} animScriptGenBitFlag_t;

typedef enum {
  ACC_BELT_LEFT,  // belt left (lower)
  ACC_BELT_RIGHT, // belt right (lower)
  ACC_BELT,       // belt (upper)
  ACC_BACK,       // back (upper)
  ACC_WEAPON,     // weapon (upper)
  ACC_WEAPON2,    // weapon2 (upper)
  ACC_HAT,        // hat (head)
  ACC_MOUTH2,     //
  ACC_MOUTH3,     //
  ACC_RANK,       //
  ACC_MAX         // this is bound by network limits, must change network stream
                  // to increase this
} accType_t;

inline constexpr int ACC_NUM_MOUTH = 3; // matches the above count

inline constexpr int MAX_GIB_MODELS = 16;

inline constexpr int MAX_WEAPS_PER_CLASS = 10;

typedef struct {
  int classNum;
  const char *characterFile;
  const char *iconName;
  const char *iconArrow;

  weapon_t classWeapons[MAX_WEAPS_PER_CLASS];

  qhandle_t icon;
  qhandle_t arrow;

} bg_playerclass_t;

typedef struct bg_character_s {
  char characterFile[MAX_QPATH];

#ifdef CGAMEDLL
  qhandle_t mesh;
  qhandle_t skin;

  qhandle_t headModel;
  qhandle_t headSkin;

  qhandle_t accModels[ACC_MAX];
  qhandle_t accSkins[ACC_MAX];

  qhandle_t gibModels[MAX_GIB_MODELS];

  qhandle_t undressedCorpseModel;
  qhandle_t undressedCorpseSkin;

  qhandle_t hudhead;
  qhandle_t hudheadskin;
  animation_t hudheadanimations[MAX_HD_ANIMATIONS];
#endif // CGAMEDLL

  animModelInfo_t *animModelInfo;
} bg_character_t;

//------------------------------------------------------------------
// Global Function Decs

void BG_InitWeaponStrings(void);
void BG_AnimParseAnimScript(animModelInfo_t *modelInfo,
                            animScriptData_t *scriptData, const char *filename,
                            char *input);
int BG_AnimScriptAnimation(playerState_t *ps, animModelInfo_t *modelInfo,
                           scriptAnimMoveTypes_t movetype, qboolean isContinue);
int BG_AnimScriptCannedAnimation(playerState_t *ps, animModelInfo_t *modelInfo);
int BG_AnimScriptEvent(playerState_t *ps, animModelInfo_t *modelInfo,
                       scriptAnimEventTypes_t event, qboolean isContinue,
                       qboolean force);
int BG_IndexForString(const char *token, animStringItem_t *strings,
                      qboolean allowFail);
int BG_PlayAnimName(playerState_t *ps, animModelInfo_t *animModelInfo,
                    char *animName, animBodyPart_t bodyPart, qboolean setTimer,
                    qboolean isContinue, qboolean force);
void BG_ClearAnimTimer(playerState_t *ps, animBodyPart_t bodyPart);
qboolean BG_ValidAnimScript(int clientNum);
char *BG_GetAnimString(animModelInfo_t *animModelInfo, int anim);
void BG_UpdateConditionValue(int client, int condition, int value,
                             qboolean checkConversion);
int BG_GetConditionValue(int client, int condition, qboolean checkConversion);
qboolean BG_GetConditionBitFlag(int client, int condition, int bitNumber);
void BG_SetConditionBitFlag(int client, int condition, int bitNumber);
void BG_ClearConditionBitFlag(int client, int condition, int bitNumber);
int BG_GetAnimScriptAnimation(int client, animModelInfo_t *animModelInfo,
                              aistateEnum_t aistate,
                              scriptAnimMoveTypes_t movetype);
void BG_AnimUpdatePlayerStateConditions(pmove_t *pmove);
animation_t *BG_AnimationForString(char *string,
                                   animModelInfo_t *animModelInfo);
animation_t *BG_GetAnimationForIndex(animModelInfo_t *animModelInfo, int index);
int BG_GetAnimScriptEvent(playerState_t *ps, scriptAnimEventTypes_t event);
int PM_IdleAnimForWeapon(int weapon);
int PM_RaiseAnimForWeapon(int weapon);
void PM_ContinueWeaponAnim(int anim);

extern animStringItem_t animStateStr[];
extern animStringItem_t animBodyPartsStr[];

bg_playerclass_t *BG_GetPlayerClassInfo(int team, int cls);
bg_playerclass_t *BG_PlayerClassForPlayerState(playerState_t *ps);
qboolean BG_ClassHasWeapon(bg_playerclass_t *classInfo, weapon_t weap);
qboolean BG_WeaponIsPrimaryForClassAndTeam(int classnum, team_t team,
                                           weapon_t weapon);
int BG_ClassWeaponCount(bg_playerclass_t *classInfo, team_t team);
const char *BG_ShortClassnameForNumber(int classNum);
const char *BG_ClassnameForNumber(int classNum);
const char *BG_ClassLetterForNumber(int classNum);

void BG_DisableClassWeapon(bg_playerclass_t *classinfo, int weapon);
void BG_DisableWeaponForAllClasses(int weapon);

extern bg_playerclass_t bg_allies_playerclasses[NUM_PLAYER_CLASSES];
extern bg_playerclass_t bg_axis_playerclasses[NUM_PLAYER_CLASSES];

const char *BG_TeamnameForNumber(team_t teamNum);

inline constexpr int MAX_PATH_CORNERS = 512;

typedef struct {
  char name[64];
  vec3_t origin;
} pathCorner_t;

extern int numPathCorners;
extern pathCorner_t pathCorners[MAX_PATH_CORNERS];

inline constexpr int NUM_EXPERIENCE_LEVELS = 11;

typedef enum {
  ME_PLAYER,
  ME_PLAYER_REVIVE,
  ME_PLAYER_DISGUISED,
  ME_CONSTRUCT,
  ME_DESTRUCT,
  ME_DESTRUCT_2,
  ME_LANDMINE,
  ME_TANK,
  ME_TANK_DEAD,
  // ME_LANDMINE_ARMED,
  ME_COMMANDMAP_MARKER,
} mapEntityType_t;

extern const char *rankNames_Axis[NUM_EXPERIENCE_LEVELS];
extern const char *rankNames_Allies[NUM_EXPERIENCE_LEVELS];
extern const char *miniRankNames_Axis[NUM_EXPERIENCE_LEVELS];
extern const char *miniRankNames_Allies[NUM_EXPERIENCE_LEVELS];
extern const char *rankSoundNames_Axis[NUM_EXPERIENCE_LEVELS];
extern const char *rankSoundNames_Allies[NUM_EXPERIENCE_LEVELS];

inline constexpr int MAX_SPLINE_PATHS = 512;
inline constexpr int MAX_SPLINE_CONTROLS = 4;
inline constexpr int MAX_SPLINE_SEGMENTS = 16;

typedef struct splinePath_s splinePath_t;

typedef struct {
  vec3_t start;
  vec3_t v_norm;
  float length;
} splineSegment_t;

struct splinePath_s {
  pathCorner_t point;

  char strTarget[64];

  splinePath_t *next;
  splinePath_t *prev;

  pathCorner_t controls[MAX_SPLINE_CONTROLS];
  int numControls;
  splineSegment_t segments[MAX_SPLINE_SEGMENTS];

  float length;

  qboolean isStart;
  qboolean isEnd;
};

extern int numSplinePaths;
extern splinePath_t splinePaths[MAX_SPLINE_PATHS];

pathCorner_t *BG_Find_PathCorner(const char *match);
splinePath_t *BG_GetSplineData(int number, qboolean *backwards);
void BG_AddPathCorner(const char *name, vec3_t origin);
splinePath_t *BG_AddSplinePath(const char *name, const char *target,
                               vec3_t origin);
void BG_BuildSplinePaths();
splinePath_t *BG_Find_Spline(const char *match);
float BG_SplineLength(splinePath_t *pSpline);
void BG_AddSplineControl(splinePath_t *spline, const char *name);
void BG_LinearPathOrigin2(float radius, splinePath_t **pSpline,
                          float *deltaTime, vec3_t result, qboolean backwards);

int BG_MaxAmmoForWeapon(weapon_t weaponNum, int *skill);

// START Mad Doc - TDF
typedef struct botpool_x {
  int num;
  int playerclass;
  int rank;
  struct botpool_x *next;
} botpool_t;

// END Mad Doc - TDF

typedef struct {
  int ident;
  char joinOrder[MAX_CLIENTS]; // order in which clients joined the fire
                               // team (server), client uses to store if
                               // a client is on this fireteam
  int leader; // leader = joinOrder[0] on server, stored here on client
  int saveLimit;
  // Toggle whether target_relay_fireteam will activate for all ft
  // members or just one
  bool teamJumpMode;
  qboolean inuse;
  qboolean priv;

  bool noGhost; // disable ghostplayers between fireteam members
} fireteamData_t;

long BG_StringHashValue(const char *fname);
long BG_StringHashValue_Lwr(const char *fname);

void BG_RotatePoint(vec3_t point, const vec3_t matrix[3]);
void BG_TransposeMatrix(const vec3_t matrix[3], vec3_t transpose[3]);
void BG_CreateRotationMatrix(const vec3_t angles, vec3_t matrix[3]);

int trap_PC_AddGlobalDefine(const char *define);
int trap_PC_LoadSource(const char *filename);
int trap_PC_FreeSource(int handle);
int trap_PC_ReadToken(int handle, pc_token_t *pc_token);
int trap_PC_SourceFileAndLine(int handle, char *filename, int *line);
int trap_PC_UnReadToken(int handle);

void PC_SourceError(int handle, const char *format, ...);
void PC_SourceWarning(int handle, const char *format, ...);

#ifdef GAMEDLL
const char *PC_String_Parse(int handle);
const char *PC_Line_Parse(int handle);
#else
const char *String_Alloc(const char *p);
qboolean PC_String_Parse(int handle, const char **out);
#endif
qboolean PC_String_ParseNoAlloc(int handle, char *out, size_t size);
qboolean PC_Int_Parse(int handle, int *i);
qboolean PC_Color_Parse(int handle, vec4_t *c);
qboolean PC_Vec_Parse(int handle, vec3_t *c);
qboolean PC_Float_Parse(int handle, float *f);

// for boolean parsing where the value must be explicitly set via int
// can be used for both bool and qboolean values
template <typename T>
qboolean PC_Boolean_Parse(int handle, T *value) {
  int temp;

  if (!PC_Int_Parse(handle, &temp)) {
    return qfalse;
  }

  *value = static_cast<T>(temp);
  return qtrue;
}

typedef enum {
  UIMENU_NONE,
  UIMENU_MAIN,
  UIMENU_INGAME,
  UIMENU_NEED_CD,
  UIMENU_BAD_CD_KEY,
  UIMENU_TEAM,
  UIMENU_POSTGAME,
  UIMENU_HELP,

  UIMENU_WM_QUICKMESSAGE,
  UIMENU_WM_QUICKMESSAGEALT,

  UIMENU_WM_FTQUICKMESSAGE,
  UIMENU_WM_FTQUICKMESSAGEALT,

  UIMENU_WM_TAPOUT,
  UIMENU_WM_TAPOUT_LMS,

  UIMENU_WM_AUTOUPDATE,

  // ydnar: say, team say, etc
  UIMENU_INGAME_MESSAGEMODE,

  // fireteam savelimit input box
  UIMENU_INGAME_FT_SAVELIMIT,
} uiMenuCommand_t;

void BG_AdjustAAGunMuzzleForBarrel(vec_t *origin, vec_t *forward, vec_t *right,
                                   vec_t *up, int barrel);

int BG_ClassTextToClass(char *token);
skillType_t BG_ClassSkillForClass(int classnum);

qboolean BG_isLightWeaponSupportingFastReload(int weapon);
qboolean BG_IsScopedWeapon(int weapon);

int BG_FootstepForSurface(int surfaceFlags);

// Minimum # of players needed to start a match
// FIXME: remove this
#define MATCH_MINPLAYERS "4"

// Multiview support
int BG_simpleHintsCollapse(int hint, int val);
int BG_simpleHintsExpand(int hint, int val);
int BG_simpleWeaponState(int ws);

// Color escape handling
int BG_colorstrncpyz(char *in, char *out, int str_max, int out_max);
int BG_drawStrlen(const char *str);
int BG_strRelPos(char *in, int index);
int BG_cleanName(const char *pszIn, char *pszOut, unsigned int dwMaxLength,
                 qboolean fCRLF);

// color support
void BG_setColor(char *colString, vec4_t col, float alpha,
                 const char *cvarName);

// Voting
#define VOTING_DISABLED ((1 << numVotesAvailable) - 1)

typedef struct {
  const char *pszCvar;
  int flag;
} voteType_t;

extern const voteType_t voteToggles[];
extern int numVotesAvailable;

// Tracemap
#ifdef CGAMEDLL
void CG_GenerateTracemap(void);
#endif // CGAMEDLL
qboolean BG_LoadTraceMap(char *rawmapname, vec2_t world_mins,
                         vec2_t world_maxs);
float BG_GetSkyHeightAtPoint(vec3_t pos);
float BG_GetSkyGroundHeightAtPoint(vec3_t pos);
float BG_GetGroundHeightAtPoint(vec3_t pos);
int BG_GetTracemapGroundFloor(void);
int BG_GetTracemapGroundCeil(void);

//
// bg_animgroup.c
//
void BG_ClearAnimationPool(void);
qboolean BG_R_RegisterAnimationGroup(const char *filename,
                                     animModelInfo_t *animModelInfo);

//
// bg_character.c
//

typedef struct bg_characterDef_s {
  char mesh[MAX_QPATH];
  char animationGroup[MAX_QPATH];
  char animationScript[MAX_QPATH];
  char skin[MAX_QPATH];
  char undressedCorpseModel[MAX_QPATH];
  char undressedCorpseSkin[MAX_QPATH];
  char hudhead[MAX_QPATH];
  char hudheadanims[MAX_QPATH];
  char hudheadskin[MAX_QPATH];
} bg_characterDef_t;

qboolean BG_ParseCharacterFile(const char *filename,
                               bg_characterDef_t *characterDef);
bg_character_t *BG_GetCharacter(int team, int cls);
bg_character_t *BG_GetCharacterForPlayerstate(playerState_t *ps);
void BG_ClearCharacterPool(void);
bg_character_t *BG_FindFreeCharacter(const char *characterFile);
bg_character_t *BG_FindCharacter(const char *characterFile);

//
// bg_sscript.c
//
typedef enum {
  S_LT_NOT_LOOPED,
  S_LT_LOOPED_ON,
  S_LT_LOOPED_OFF
} speakerLoopType_t;

typedef enum { S_BT_LOCAL, S_BT_GLOBAL, S_BT_NOPVS } speakerBroadcastType_t;

typedef struct bg_speaker_s {
  char filename[MAX_QPATH];
  qhandle_t noise;
  vec3_t origin;
  char targetname[32];
  long targetnamehash;

  speakerLoopType_t loop;
  speakerBroadcastType_t broadcast;
  int wait;
  int random;
  int volume;
  int range;

  qboolean activated;
  int nextActivateTime;
  int soundTime;
} bg_speaker_t;

void BG_ClearScriptSpeakerPool(void);
int BG_NumScriptSpeakers(void);
int BG_GetIndexForSpeaker(bg_speaker_t *speaker);
bg_speaker_t *BG_GetScriptSpeaker(int index);
qboolean BG_SS_DeleteSpeaker(int index);
qboolean BG_SS_StoreSpeaker(bg_speaker_t *speaker);
qboolean BG_LoadSpeakerScript(const char *filename);

// Lookup table to find ammo table entry
extern ammotable_t ammoTableMP[WP_NUM_WEAPONS];
#define GetAmmoTableData(ammoIndex) ((ammotable_t *)(&ammoTableMP[ammoIndex]))

inline constexpr int MAX_MAP_SIZE = 65536;

qboolean BG_BBoxCollision(vec3_t min1, vec3_t max1, vec3_t min2, vec3_t max2);

// #define VISIBLE_TRIGGERS

//
// bg_stats.c
//

typedef struct weap_ws_convert_s {
  weapon_t iWeapon;
  extWeaponStats_t iWS;
} weap_ws_convert_t;

extWeaponStats_t BG_WeapStatForWeapon(weapon_t iWeaponID);

typedef enum popupMessageType_e {
  PM_DYNAMITE,
  PM_CONSTRUCTION,
  PM_MINES,
  PM_DEATH,
  PM_MESSAGE,
  PM_OBJECTIVE,
  PM_DESTRUCTION,
  PM_TEAM,
  PM_NUM_TYPES
} popupMessageType_t;

typedef enum popupMessageBigType_e {
  PM_SKILL,
  PM_RANK,
  PM_DISGUISE,
  PM_BIG_NUM_TYPES
} popupMessageBigType_t;

inline constexpr int NUM_HEAVY_WEAPONS = 6;
extern weapon_t bg_heavyWeapons[NUM_HEAVY_WEAPONS];

int PM_AltSwitchFromForWeapon(int weapon);
int PM_AltSwitchToForWeapon(int weapon);

void PM_TraceLegs(trace_t *trace, float *legsOffset, vec3_t start, vec3_t end,
                  trace_t *bodytrace, vec3_t viewangles,
                  void(tracefunc)(trace_t *results, const vec3_t start,
                                  const vec3_t mins, const vec3_t maxs,
                                  const vec3_t end, int passEntityNum,
                                  int contentMask),
                  int ignoreent, int tracemask);
void PM_TraceAllLegs(trace_t *trace, float *legsOffset, vec3_t start,
                     vec3_t end);
void PM_TraceAll(trace_t *trace, vec3_t start, vec3_t end);

// Feen: Color Methods
void BG_ColorComplement(const vec4_t in_RGB, vec4_t *out_RGB);

struct Manual {
  const char *cmd;
  const char *usage;
  const char *description;
};

static constexpr Manual commandManuals[] = {
    {"8ball", "!8ball [question]",
     "Magical 8 Ball of pure awesomeness gives an answer to any question you "
     "might have!"},
    {"addlevel",
     "!addlevel [level] -cmds [commands] -greeting [greeting] -title [title]",
     "Adds a new level. Provide optional -switches to set level attributes."},
    {"admintest", "!admintest", "Displays your admin level."},
    {"ban", "!ban [player] [(optional) seconds] [(optional) reason]",
     "Bans target player from server. If seconds is 0, ban is permanent."},
    {"cancelvote", "!cancelvote", "Cancels current vote in progress."},
    {"deletelevel", "!deletelevel [level]", "Deletes a level."},
    //    {"deleteuser", "!deleteuser -id [user id]", "Deletes a user based on
    //    ID."},
    {"editcommands",
     "!editcommands [level] [+command|-command] [+command|-command...]",
     "Edits commands of a level."},
    {"editlevel",
     "!editlevel [level] -cmds [new commands] -greeting [new greeting] -title "
     "[new title]",
     "Edits an existing level. If level does not exist, creates it."},
    {"edituser",
     "!edituser [id] -cmds [personal commands] -title [personal title] "
     "-greeting [personal greeting] -clear [switch]\n",
     "Used to edit user admin attributes such as personal commands, personal "
     "title and personal greeting. "
     "All of the \"-cmds|-title|-greeting\" switches are optional, "
     "but at least one must be given. Use -clear [switch] to reset a switch."},
    {"finger", "!finger [target]", "Displays target's admin level."},
    {"finduser", "!finduser [name]", "Finds all matching users."},
    {"help", "!help\n!help [command]",
     "Prints useful information about commands."},
    {"kick",
     "!kick [target]\n!kick [target] [timeout]\n!kick [target] [timeout] "
     "[reason]",
     "Kicks target player."},
    {"levelinfo", "!levelinfo [level]",
     "Prints useful information about a level."},
    {"listbans", "!listbans [page]", "Lists 10 bans per page."},
    //	{ "listcmds",     "!listcmds", "Lists all commands or prints a command
    // manual."                                     },
    {"listflags", "!listflags", "Lists all command flags."},
    {"listmaps", "!listmaps", "Lists all maps."},
    {"listplayers", "!listplayers", "Lists all players on server."},
    {"listusernames", "!listusernames [id]",
     "Lists all usernames of specified user ID."},
    {"listusers", "!listusers [page]", "Lists 20 users per page."},
    {"map", "!map [map name]", "Changes map."},
    {"mapinfo", "!mapinfo [map name]", "Prints information about map."},
    {"mostplayed", "!mostplayed [(optional) count]",
     "Prints the most played maps on the server."},
    {"moverscale", "!moverscale [scale]", "Scales vehicle movement speed."},
    {"mute", "!mute [target]", "Mutes target player."},
    {"noclip", "!noclip [(optional) target] [(optional) count]",
     "Gives noclip to yourself or target player. Use count to specify how many "
     "times "
     "the target can use noclip."},
    {"passvote", "!passvote", "Passes current vote in progress."},
    //	{ "race",         "/race [start|end|checkpoint|clear|settings|undo]
    //[additional parameters]\n\n/race start [xyz size]\n/race start [xy size]
    //[z size]\n/race start [x size] [y size] [z size]\n\n",
    //	  "" },
    //	{ "readconfig",   "!readconfig", "Reads user, level and ban databases."
    //},
    {"rename", "!rename [target] [new name]", "Renames target player."},
    {"restart", "!restart", "Restarts map."},
    //    { "routemaker", "!routemaker [player]", "Makes target player the route
    //    maker." },
    {"setlevel", "!setlevel [target] [level]\n!setlevel -id [id] [level]",
     "Sets the target's admin level."},
    {"spectate", "!spectate [(optional) target]",
     "Puts yourself into spectators, optionally spectating targeted player."},
    {"tokens", "!tokens [create e|m|h] [move] [delete] [delete e|m|h 1-6]",
     "Creates/deletes/moves a collectible token with given parameters. Move "
     "and delete "
     "default to the token nearest to your position."},
    {"unban", "!unban [ban ID]", "Unbans a player based on ban ID"},
    {"unmute", "!unmute [target]", "Unmutes target player."},
    {"userinfo", "!userinfo [id]", "Prints user info."},
    {"ad_save", "/ad_save [(optional) name]",
     "Saves currently ongoing autodemo temp file manually with given name."},
    {"clearsaves", "/clearsaves", "Removes all your saved positions."},
    {"newmaps", "!newmaps [(optional) count]",
     "Lists latest ^3[count] ^7maps added to server, sorted from oldest to "
     "newest."},
    {"rtv", "!rtv", "Calls Rock The Vote."},

    {"savepos", "/savepos [(optional) name] [(optional) flags]",
     "Saves your current position to a savepos file.\n\n"
     "Flags:\n"
     "1 - don't save velocity\n"
     "2 - don't save pitch angle\n\n"
     "If no arguments are given, saves to 'savepos/default.dat'.\n"
     "If only one argument is given, the argument is treated as a 'flag' if "
     "it's numeric, otherwise as 'name'."},

    {"loadpos", "/loadpos [(optional) name]",
     "Loads position from a given savepos file. If 'name' isn't specified, "
     "loads 'savepos/default.dat'.\n"
     "Cheats must be enabled to use this command."},
};

typedef struct {
  char id[32];      // voice chat id
  char custom[128]; // voice chat custom text
  int variant;      // voice chat variation
  float random;     // recieve random from the server
} vsayCmd_t;

// Overbounce is disabled on current map
inline constexpr int BG_LEVEL_NO_OVERBOUNCE = 1 << 0;
// jump delay is disabled
inline constexpr int BG_LEVEL_NO_JUMPDELAY = 1 << 1;
// Save is disabled
inline constexpr int BG_LEVEL_NO_SAVE = 1 << 2;
// Fall damage is disabled
inline constexpr int BG_LEVEL_NO_FALLDAMAGE = 1 << 3;
// Fall damage is disabled (force)
inline constexpr int BG_LEVEL_NO_FALLDAMAGE_FORCE = 1 << 4;
// Prone is disabled
inline constexpr int BG_LEVEL_NO_PRONE = 1 << 5;
// Nodrop is enabled
inline constexpr int BG_LEVEL_NO_DROP = 1 << 6;
// Wallbugging is disabled
inline constexpr int BG_LEVEL_NO_WALLBUG = 1 << 7;
// Noclip is disabled
inline constexpr int BG_LEVEL_NO_NOCLIP = 1 << 8;

namespace ETJump {
inline constexpr char CUSTOMVOTE_TYPE[] = "type";
inline constexpr char CUSTOMVOTE_CVTEXT[] = "cvtext";
inline constexpr char CUSTOMVOTE_SERVERMAPS[] = "servermaps";
inline constexpr char CUSTOMVOTE_OTHERMAPS[] = "othermaps";

enum class CheatCvarFlags {
  None = 0,
  LookYaw = 1,
  PmoveFPS = 2,
};

// this can hold maximum of 10 bits and maps to currentState->density
// used to transmit information otherwise inaccessible in cgame
// from player to player, such as playerState_t fields that don't
// normally get mapped to entityState in BG_PlayerStateToEntityState
enum class PlayerDensityFlags {
  None = 0 << 0,
  Noclip = 1 << 0,
};

enum class TeleporterSpawnflags {
  None = 0,
  ResetSpeed = 1 << 0,
  ConvertSpeed = 1 << 1,
  RelativeYaw = 1 << 2,
  RelativePitchYaw = 1 << 3,
  Knockback = 1 << 4,
  NoZOffset = 1 << 5,
};

enum class PusherSpawnFlags {
  None = 0,
  // reserved by 'target_push' but unused as the sounds don't exist in ET
  AltSound = 1 << 0,
  AddXY = 1 << 1,
  AddZ = 1 << 2
};

// PERS_HWEAPON_USE
enum class HeavyWeaponState {
  MountedMG = 1,
  AAGun = 2, // not sure if this is actually used/functional
};

enum class ViewlockState {
  Jitter = 2,  // screen jitter (firing mounted MG42)
  Mounted = 3, // lock to direction of mounted gun
  Medic = 7,   // look at nearest medic
};

enum class PlayerStance {
  Stand = 0,
  Crouch = 1,
  Prone = 2,
};
} // namespace ETJump

inline constexpr int JUMP_VELOCITY = 270;
// FIXME: this is incorrect when ps.speed is modified
inline constexpr int MAX_GROUNDSTRAFE = 452;

// default sv_fps 20 frametime for framerate independent server timings
inline constexpr int DEFAULT_SV_FRAMETIME = 50;

#endif // __BG_PUBLIC_H__
