#ifndef G_LOCAL_H
#define G_LOCAL_H
// g_local.h -- local definitions for game module

#ifdef min
  #undef min
#endif
#ifdef max
  #undef max
#endif

#include <climits>
#include <memory>
#include <string>
#include <vector>
#include "q_shared.h"
#include "bg_public.h"
#include "g_public.h"
#include <stdint.h>

//==================================================================

// the "MOD_VERSION" client command will print this plus compile date
#ifndef PRE_RELEASE_DEMO

#else
  // #define MOD_VERSION			"You look like you need a
  //  monkey!"
  #define MOD_VERSION "ettest"
#endif // PRE_RELEASE_DEMO

#define BODY_QUEUE_SIZE 8

#define EVENT_VALID_MSEC 300
#define CARNAGE_REWARD_TIME 3000

#define INTERMISSION_DELAY_TIME 1000

#define MG42_MULTIPLAYER_HEALTH 350 // JPW NERVE
#define NO_BOT_SUPPORT

// How long do bodies last?
// SP : Axis: 20 seconds
//		Allies: 30 seconds
// MP : Both 10 seconds
#define BODY_TIME(t)                                                           \
  ((g_gametype.integer != GT_SINGLE_PLAYER || g_gametype.integer == GT_COOP)   \
       ? 10000                                                                 \
   : (t) == TEAM_AXIS ? 20000                                                  \
                      : 30000)

#define MAX_MG42_HEAT 1500.f

// gentity->flags
#define FL_GODMODE 0x00000010
#define FL_NOTARGET 0x00000020
#define FL_TEAMSLAVE 0x00000400 // not the first on the team
#define FL_NO_KNOCKBACK 0x00000800
#define FL_DROPPED_ITEM 0x00001000
#define FL_NO_BOTS 0x00002000   // spawn point not for bot use
#define FL_NO_HUMANS 0x00004000 // spawn point just for bots
#define FL_AI_GRENADE_KICK                                                     \
  0x00008000 // an AI has already decided to kick this grenade
// Rafael
#define FL_NOFATIGUE 0x00010000 // cheat flag no fatigue

#define FL_TOGGLE 0x00020000 //----(SA)	ent is toggling (doors use this for ex.)
#define FL_KICKACTIVATE                                                        \
  0x00040000 //----(SA)	ent has been activated by a kick (doors use this
             // too for
             // ex.)
#define FL_SOFTACTIVATE                                                        \
  0x00000040 //----(SA)	ent has been activated while 'walking' (doors
             // use this
             // too for ex.)
#define FL_DEFENSE_GUARD 0x00080000 // warzombie defense pose

#define FL_BLANK 0x00100000
#define FL_BLANK2 0x00200000
#define FL_NO_MONSTERSLICK 0x00400000
#define FL_NO_HEADCHECK 0x00800000

#define FL_NODRAW 0x01000000

#define TKFL_MINES 0x00000001
#define TKFL_AIRSTRIKE 0x00000002
#define TKFL_MORTAR 0x00000004

// movers are things like doors, plats, buttons, etc
typedef enum {
  MOVER_POS1,
  MOVER_POS2,
  MOVER_POS3,
  MOVER_1TO2,
  MOVER_2TO1,
  // JOSEPH 1-26-00
  MOVER_2TO3,
  MOVER_3TO2,
  // END JOSEPH

  // Rafael
  MOVER_POS1ROTATE,
  MOVER_POS2ROTATE,
  MOVER_1TO2ROTATE,
  MOVER_2TO1ROTATE
} moverState_t;

#define MAX_PASSWORD_LEN 40
#define MAX_ADMINS 16

// door AI sound ranges
#define HEAR_RANGE_DOOR_LOCKED 128 // really close since this is a cruel check
#define HEAR_RANGE_DOOR_KICKLOCKED 512
#define HEAR_RANGE_DOOR_OPEN 256
#define HEAR_RANGE_DOOR_KICKOPEN 768

// DHM - Nerve :: Worldspawn spawnflags to indicate if a gametype is not
// supported
#define NO_GT_WOLF 1
#define NO_STOPWATCH 2
#define NO_CHECKPOINT 4
#define NO_LMS 8

#define MAX_CONSTRUCT_STAGES 3

#define ALLOW_AXIS_TEAM 1
#define ALLOW_ALLIED_TEAM 2
#define ALLOW_DISGUISED_CVOPS 4

// RF, different types of dynamic area flags
#define AAS_AREA_ENABLED 0x0000
#define AAS_AREA_DISABLED 0x0001
#define AAS_AREA_AVOID 0x0010
#define AAS_AREA_TEAM_AXIS 0x0020
#define AAS_AREA_TEAM_ALLIES 0x0040
#define AAS_AREA_TEAM_AXIS_DISGUISED 0x0080
#define AAS_AREA_TEAM_ALLIES_DISGUISED 0x0100
// I have no idea how I managed to forget these :D
// Hopefully no1 will ever notice versions < 2.0.1 have the g_gravity
// & g_speed cvars :p
#define G_GRAVITY 800
#define G_SPEED 320

#define CHAT_OPTIONS_INTERPOLATE_NAME_TAGS 0x000001
// ETJump: defines window that stops players from freeing themselves from
// following right after the team change. Fixes issue when quick follow is used,
// and +activate is handled for both following and freeing on adjacent frames
#define SPECFREE_COOLDOWN 200

//============================================================================

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;
typedef struct g_serverEntity_s g_serverEntity_t;

//====================================================================
//
// Scripting, these structure are not saved into savegames (parsed each start)
typedef struct {
  const char *actionString;
  qboolean (*actionFunc)(gentity_t *ent, char *params);
  int hash;
} g_script_stack_action_t;
//
typedef struct {
  //
  // set during script parsing
  g_script_stack_action_t *action; // points to an action to perform
  char *params;
} g_script_stack_item_t;
//
// Gordon: need to up this, forest has a HUGE script for the tank.....
// #define	G_MAX_SCRIPT_STACK_ITEMS	128
// #define	G_MAX_SCRIPT_STACK_ITEMS	176
// RF, upped this again for the tank
// Gordon: and again...
#define G_MAX_SCRIPT_STACK_ITEMS 196
//
typedef struct {
  g_script_stack_item_t items[G_MAX_SCRIPT_STACK_ITEMS];
  int numItems;
} g_script_stack_t;
//
typedef struct {
  int eventNum; // index in scriptEvents[]
  char *params; // trigger targetname, etc
  g_script_stack_t stack;
} g_script_event_t;
//
typedef struct {
  const char *eventStr;
  qboolean (*eventMatch)(g_script_event_t *event, const char *eventParm);
  int hash;
} g_script_event_define_t;
//
// Script Flags
#define SCFL_GOING_TO_MARKER 0x1
#define SCFL_ANIMATING 0x2
#define SCFL_FIRST_CALL 0x4
//
// Scripting Status (NOTE: this MUST NOT contain any pointer vars)
typedef struct {
  int scriptStackHead, scriptStackChangeTime;
  int scriptEventIndex; // current event containing stack of actions to
                        // perform
  // scripting system variables
  int scriptId; // incremented each time the script changes
  int scriptFlags;
  int actionEndTime;     // time to end the current action
  char *animatingParams; // Gordon: read 8 lines up for why i love this
                         // code ;)
} g_script_status_t;
//
#define G_MAX_SCRIPT_ACCUM_BUFFERS 10
//
void G_Script_ScriptEvent(gentity_t *ent, const char *eventStr,
                          const char *params);
//====================================================================

typedef struct g_constructible_stats_s {
  float chargebarreq;
  float constructxpbonus;
  float destructxpbonus;
  int health;
  int weaponclass;
  int duration;
} g_constructible_stats_t;

#define NUM_CONSTRUCTIBLE_CLASSES 3

extern g_constructible_stats_t
    g_constructible_classes[NUM_CONSTRUCTIBLE_CLASSES];

qboolean G_WeaponIsExplosive(meansOfDeath_t mod);
int G_GetWeaponClassForMOD(meansOfDeath_t mod);

//====================================================================
// ETJump

#define MAX_SAVE_POSITIONS 3

typedef struct {
  qboolean isValid;
  vec3_t origin;
  vec3_t vangles;
  vec3_t velocity;
} save_position_t;

struct TokenInformation_s {
  // The index in the Tokens-class tokens array
  int idx;
  int difficulty;
  char name[256];
};
typedef struct TokenInformation_s TokenInformation;

constexpr int MAX_TIMERUNS = 20;
constexpr int MAX_TIMERUN_NAME_LENGTH = 64;

//====================================================================

#define MAX_NETNAME 36

#define CFOFS(x) ((int)&(((gclient_t *)0)->x))

#define MAX_COMMANDER_TEAM_SOUNDS 16

typedef struct commanderTeamChat_s {
  int index;
} commanderTeamChat_t;

struct gentity_s {
  entityState_t s;  // communicated by server to clients
  entityShared_t r; // shared by both the server system and game

  // DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
  // EXPECTS THE FIELDS IN THAT ORDER!
  //================================

  struct gclient_s *client; // NULL if not a client

  qboolean inuse;

  vec3_t instantVelocity; // ydnar: per entity instantaneous velocity,
                          // set per frame

  const char *classname; // set in QuakeEd
  int spawnflags;        // set in QuakeEd

  qboolean neverFree; // if true, FreeEntity will only unlink
                      // bodyque uses this

  int flags; // FL_* variables

  int id; // Identifier for the entity for other systems

  char *model;
  char *model2;
  int freetime; // level.time when the object was freed

  int eventTime; // events will be cleared EVENT_VALID_MSEC after set
  qboolean freeAfterEvent;
  qboolean unlinkAfterEvent;

  qboolean physicsObject; // if true, it can be pushed by movers and fall off
                          // edges all game items are physicsObjects,
  float physicsBounce;    // 1.0 = continuous bounce, 0.0 = no bounce
  int clipmask;           // brushes with this content value will be collided
                          // against when moving.  items and corpses do not
                          // collide against players, for instance

  int realClipmask; // Arnout: use these to backup the contents value
                    // when we go to state under construction
  int realContents;
  qboolean realNonSolidBModel; // For script_movers with spawnflags 2 set

  // movers
  moverState_t moverState;
  int soundPos1;
  int sound1to2;
  int sound2to1;
  int soundPos2;
  int soundLoop;
  int sound2to3;
  int sound3to2;
  int soundPos3;

  int soundSoftopen;
  int soundSoftendo;
  int soundSoftclose;
  int soundSoftendc;

  gentity_t *parent;
  gentity_t *nextTrain;
  gentity_t *prevTrain;
  vec3_t pos1, pos2, pos3;

  char *message;

  int timestamp; // body queue sinking, etc

  float angle; // set in editor, -1 = up, -2 = down
  char *target;

  char *targetname;
  int targetnamehash; // Gordon: adding a hash for this for faster
                      // lookups

  char *team;
  gentity_t *target_ent;

  float speed;
  float closespeed; // for movers that close at a different speed than
                    // they open
  vec3_t movedir;

  int gDuration;
  int gDurationBack;
  vec3_t gDelta;
  vec3_t gDeltaBack;

  int nextthink;
  void (*free)(gentity_t *self);
  void (*think)(gentity_t *self);
  void (*reached)(gentity_t *self); // movers call this when hitting endpoint
  void (*blocked)(gentity_t *self, gentity_t *other);
  void (*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
  void (*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
  void (*pain)(gentity_t *self, gentity_t *attacker, int damage, vec3_t point);
  void (*die)(gentity_t *self, gentity_t *inflictor, gentity_t *attacker,
              int damage, int mod);

  int pain_debounce_time;
  int fly_sound_debounce_time; // wind tunnel

  int health;

  qboolean takedamage;

  int damage;
  int splashDamage; // quad will increase this without increasing radius
  int splashRadius;
  int methodOfDeath;
  int splashMethodOfDeath;

  int count;

  gentity_t *chain;
  gentity_t *enemy;
  gentity_t *activator;
  gentity_t *teamchain;  // next entity in team
  gentity_t *teammaster; // master of the team

  meansOfDeath_t deathType;

  int watertype;
  int waterlevel;

  int noise_index;

  // timing variables
  float wait;
  float random;

  // Rafael - sniper variable
  // sniper uses delay, random, radius
  int radius;
  int force;
  float delay;

  // JOSEPH 10-11-99
  int TargetFlag;
  float duration;
  vec3_t rotate;
  vec3_t TargetAngles;
  // END JOSEPH

  gitem_t *item; // for bonus items

  // Ridah, AI fields
  char *aiName;
  int aiTeam;
  void (*AIScript_AlertEntity)(gentity_t *ent);
  // done.

  char *aiSkin;

  vec3_t dl_color;
  const char *dl_stylestring;
  char *dl_shader;
  int dl_atten;

  int key; // used by:  target_speaker->nopvs,

  qboolean active;

  // Rafael - mg42
  float harc;
  float varc;

  int props_frame_state;

  // Ridah
  int missionLevel; // mission we are currently trying to complete
                    // gets reset each new level
  int start_size;
  int end_size;

  // Rafael props

  qboolean isProp;

  int mg42BaseEnt;

  gentity_t *melee;

  char *spawnitem;

  int flameQuota, flameQuotaTime, flameBurnEnt;

  int count2;

  int grenadeExplodeTime; // we've caught a grenade, which was due to
                          // explode at this time
  int grenadeFired;       // the grenade entity we last fired

  char *track;

  // entity scripting system
  const char *scriptName;

  int numScriptEvents;
  g_script_event_t *scriptEvents; // contains a list of actions to
                                  // perform for each event type
  g_script_status_t scriptStatus; // current status of scripting
  // the accumulation buffer
  int scriptAccumBuffer[G_MAX_SCRIPT_ACCUM_BUFFERS];

  qboolean AASblocking;
  vec3_t AASblocking_mins, AASblocking_maxs;
  float accuracy;

  char tagName[MAX_QPATH]; // name of the tag we are attached to
  gentity_t *tagParent;
  gentity_t *tankLink;

  int lastHintCheckTime;     // DHM - Nerve
  int voiceChatSquelch;      // DHM - Nerve
  int voiceChatPreviousTime; // DHM - Nerve
  int lastBurnedFrameNumber; // JPW - Nerve   : to fix FT instant-kill
                             // exploit

  entState_t entstate;
  char *constages;
  char *desstages;
  char *damageparent;
  int conbmodels[MAX_CONSTRUCT_STAGES + 1];
  int desbmodels[MAX_CONSTRUCT_STAGES];
  int partofstage;

  int allowteams;

  int spawnTime;

  gentity_t *dmgparent;
  qboolean dmginloop;

  // RF, used for linking of static entities for faster searching
  gentity_t *botNextStaticEntity;
  int spawnCount; // incremented each time this entity is spawned
  int botIgnoreTime, awaitingHelpTime;
  int botIgnoreHealthTime, botIgnoreAmmoTime;
  int botAltGoalTime;
  vec3_t botGetAreaPos;
  int botGetAreaNum;
  int aiInactive; // bots should ignore this goal
  int goalPriority[2];

  int tagNumber; // Gordon: "handle" to a tag header

  int linkTagTime;

  splinePath_t *backspline;
  vec3_t backorigin;
  float backdelta;
  qboolean back;
  qboolean moving;

  int botLastAttackedTime;
  int botLastAttackedEnt;

  int botAreaNum; // last checked area num
  vec3_t botAreaPos;

  // TAT 10/13/2002 - for seek cover sequence - we need a pointer to a
  // server entity
  //		@ARNOUT - does this screw up the save game?
  g_serverEntity_t *serverEntity;

  // What sort of surface are we standing on?
  int surfaceFlags;

  char tagBuffer[16];

  // bleh - ugly
  int backupWeaponTime;
  int mg42weapHeat;

  vec3_t oldOrigin;

  qboolean runthisframe;

  g_constructible_stats_t constructibleStats;

  // bani
  int etpro_misc_1;

  // etjump
  int ident;
  int reqident;
  int inc; // increment val for target_increase_ident
  // How long in milliseconds it takes for the progress
  // identifier to decay back to decayValue
  int decayTime;
  // Value that ident decays to
  int decayValue;

  // Feen - PGM
  gentity_t *portalBlue;
  gentity_t *portalRed;
  // Zero - other portal so we know where to go
  // when someone goes in a team portal
  gentity_t *linkedPortal;

  int lastPortalTime; // Last time we teleported using portal
  int portalTeam;

  int runIndex;
  char runName[MAX_TIMERUN_NAME_LENGTH];
  int checkpointIndex;

  float velocityLowerLimit;
  float velocityUpperLimit;

  TokenInformation *tokenInformation;

  int tjlLineNumber;
  char *tjlLineName;

  int outSpeed;

  float scaleTime;

  int entityOwner; // Owner data for etj_touchPickupWeapons

  char *targetShaderName;
  char *targetShaderNewName;
};

// Ridah
// #include "ai_cast_global.h"
// done.

typedef enum {
  CON_DISCONNECTED,
  CON_CONNECTING,
  CON_CONNECTED
} clientConnected_t;

typedef enum {
  SPECTATOR_NOT,
  SPECTATOR_FREE,
  SPECTATOR_FOLLOW /*,
   SPECTATOR_SCOREBOARD*/
} spectatorState_t;

typedef enum {
  COMBATSTATE_COLD,
  COMBATSTATE_DAMAGEDEALT,
  COMBATSTATE_DAMAGERECEIVED,
  COMBATSTATE_KILLEDPLAYER
} combatstate_t;

typedef enum {
  TEAM_BEGIN, // Beginning a team game, spawn at base
  TEAM_ACTIVE // Now actively playing
} playerTeamStateState_t;

typedef struct {
  playerTeamStateState_t state;

  int location[2];

  int captures;
  int basedefense;
  int carrierdefense;
  int flagrecovery;
  int fragcarrier;
  int assists;

  float lasthurtcarrier;
  float lastreturnedflag;
  float flagsince;
  float lastfraggedcarrier;
} playerTeamState_t;

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define FOLLOW_ACTIVE1 -1
#define FOLLOW_ACTIVE2 -2

// OSP - weapon stat counters
typedef struct {
  unsigned int atts;
  unsigned int deaths;
  unsigned int headshots;
  unsigned int hits;
  unsigned int kills;
} weapon_stat_t;

///////////////////////////////////////////////////////////////////////////////
// Death run
///////////////////////////////////////////////////////////////////////////////

enum class DeathrunFlags {
  None = 0,
  Active = INT_MAX, // Run is currently active
  NoDamageRuns = 1 << 0,
  NoSave = 1 << 1
};

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Trigger multiple
///////////////////////////////////////////////////////////////////////////////

enum class TriggerMultipleFlags {
  None = 0,
  AxisOnly = 1 << 0,
  AlliesOnly = 1 << 1,
  NoBots = 1 << 2,
  BotsOnly = 1 << 3,
  SoldierOnly = 1 << 4,
  FieldOpsOnly = 1 << 5,
  MedicOnly = 1 << 6,
  EngineerOnly = 1 << 7,
  CvOpsOnly = 1 << 8,
  Constant = 1 << 9, // deprecated
  DeathrunOnly = 1 << 10,
  MultiActivator = 1 << 11, // deprecated (default behavior now)
};

///////////////////////////////////////////////////////////////////////////////

#define MAX_IP_LEN 15

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() /
// G_WriteSessionData()
typedef struct {
  team_t sessionTeam;
  int spectatorTime; // for determining next-in-line to play
  spectatorState_t spectatorState;
  int spectatorClient;         // for chasecam and follow mode
  int playerType;              // DHM - Nerve :: for GT_WOLF
  int playerWeapon;            // DHM - Nerve :: for GT_WOLF
  int playerWeapon2;           // Gordon: secondary weapon
  int spawnObjectiveIndex;     // JPW NERVE index of objective to spawn
                               // nearest to (returned from UI)
  int autoSpawnObjectiveIndex; // ETJump: player basis auto spawn
                               // support
  int latchPlayerType;         // DHM - Nerve :: for GT_WOLF not archived
  int latchPlayerWeapon;       // DHM - Nerve :: for GT_WOLF not archived
  int latchPlayerWeapon2;      // Gordon: secondary weapon
  int ignoreClients[MAX_CLIENTS / (sizeof(int) * 8)];
  qboolean muted;
  float skillpoints[SK_NUM_SKILLS];      // Arnout: skillpoints
  float startskillpoints[SK_NUM_SKILLS]; // Gordon: initial skillpoints
                                         // at map beginning
  float startxptotal;
  int skill[SK_NUM_SKILLS];  // Arnout: skill
  int rank;                  // Arnout: rank
  int medals[SK_NUM_SKILLS]; // Arnout: medals

  // OSP
  int coach_team;
  int damage_given;
  int damage_received;
  int deaths;
  int game_points;
  int kills;
  int rounds;
  int spec_invite;
  int spec_team;
  int suicides;
  int team_damage;
  int team_kills;

  weapon_stat_t aWeaponStats[WS_MAX + 1]; // Weapon stats.  +1 to avoid
                                          // invalid weapon check
  // OSP

  qboolean noGoto;
  qboolean noCall;

  // goto_allowed for both call & goto
  qboolean gotoAllowed;
  qboolean saveAllowed;
  // SpamProtection

  int nextReliableTime;
  int numReliableCmds;
  int thresholdTime;

  // Kill cmd spam prot
  int lastKillTime;
  // Name change spam prot
  int nameChangeCount;
  int lastNameChangeTime;

  int lastTeamSwitch;

  qboolean specLocked;
  int specInvitedClients[MAX_CLIENTS / (sizeof(int) * 8)];
  // Fireteam save limit
  int saveLimitFt;
  // Global map savelimit
  int saveLimit;
  // Map ident
  int clientMapProgression;
  // What client progression will be when time hits
  // nextProgressionDecayEvent
  int upcomingClientMapProgression;
  int previousClientMapProgression;
  int nextProgressionDecayEvent;
  qboolean decayProgression;

  int lastListmapsTime;
  int lastMostPlayedListTime;

  // Position of user before inactivity
  vec3_t posBeforeInactivity;
  // Did the client join back to team already (after inactivity)?
  qboolean loadedPosBeforeInactivity;
  team_t teamBeforeInactivitySpec;

  qboolean firstTime;
  qboolean loadPreviousSavedPositions;

  qboolean versionOK;

  int portalTeam;
  char ip[MAX_IP_LEN + 1];
  qboolean motdPrinted;

  int runSpawnflags;

  qboolean timerunActive;
  qboolean receivedTimerunStates;
  bool timerunCheatsNotified;

  // new implementation of progression
#define MAX_PROGRESSION_TRACKERS 50
  int progression[MAX_PROGRESSION_TRACKERS];
  int deathrunFlags;

  float velocityScale;
  int clientLastActive;

  int weaponsOnSpawn[MAX_WEAPONS /
                     (sizeof(int) * 8)]; // state of ps.weapons on spawn
  int ammoOnSpawn[MAX_WEAPONS];          // total amount of ammo on spawn
  int ammoclipOnSpawn[MAX_WEAPONS];      // ammo in clip on spawn
} clientSession_t;

//
#define MAX_VOTE_COUNT 3

#define PICKUP_ACTIVATE 0 // pickup items only when using "+activate"
#define PICKUP_TOUCH 1    // pickup items when touched
#define PICKUP_FORCE                                                           \
  2 // pickup the next item when touched (and reset to PICKUP_ACTIVATE
    // when done)

// OSP -- multiview handling
#define MULTIVIEW_MAXVIEWS 16
typedef struct {
  qboolean fActive;
  int entID;
  gentity_t *camera;
} mview_t;

typedef struct ipFilter_s {
  unsigned mask;
  unsigned compare;
} ipFilter_t;

typedef struct ipXPStorage_s {
  ipFilter_t filter;
  float skills[SK_NUM_SKILLS];
  int timeadded;
} ipXPStorage_t;

#define MAX_RACE_CHECKPOINTS 20
typedef struct raceStruct_s {
  int startTime;
  int endTime;
  int visitedCheckpoints[MAX_RACE_CHECKPOINTS];
  qboolean isRacing;
  qboolean isRouteMaker;
  int saveLimit;
} raceStruct_t;

// stores client voting information
namespace ETJump {
struct votingInfo_t {
  bool isVotedYes; // is the client voted yes
  int time;        // last time client voted, for timeouts between re-votes
  int attempts;    // re-vote attempts
  bool isWarned;   // if client attempts to re-vote but timeout doesn't
                   // allow yet, notification will be sent

  int lastRtvMapVoted; // used for re-votes, the last map number we voted on
};
} // namespace ETJump

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct {
  clientConnected_t connected;
  usercmd_t cmd;         // we would lose angles if not persistant
  usercmd_t oldcmd;      // previous command processed by pmove()
  qboolean localClient;  // true if "ip" info key is "localhost"
  qboolean initialSpawn; // the first spawn should be at a cool location
  qboolean pmoveFixed;
  qboolean nofatigue;
  qboolean cgaz;
  qboolean loadViewAngles;
  qboolean noActivateLean;
  qboolean autoLoad;
  qboolean quickFollow;
  bool snaphud;
  bool noPanzerAutoswitch;

  unsigned int maxFPS;
  char netname[MAX_NETNAME];

  int autoActivate; // based on cg_autoactivate userinfo
                    // (uses the PICKUP_ values above)

  int maxHealth;               // for handicapping
  int enterTime;               // level.time the client entered the game
  int connectTime;             // DHM - Nerve :: level.time the client first
                               // connected to the server
  playerTeamState_t teamState; // status in teamplay games
  int voteCount;     // to prevent people from constantly calling votes
  int teamVoteCount; // to prevent people from constantly calling votes

  int lastReinforceTime; // DHM - Nerve :: last reinforcement

  qboolean teamInfo; // send team overlay updates?

  qboolean bAutoReloadAux; // TTimo - auxiliary storage for
                           // pmoveExt_t::bAutoReload, to achieve persistance

  int applicationClient;  // Gordon: this client has requested to join
                          // your fireteam
  int applicationEndTime; // Gordon: you have X seconds to reply or this
                          // message will self destruct!

  int invitationClient;  // Gordon: you have been invited to join this
                         // client's fireteam
  int invitationEndTime; // Gordon: quickly now, chop chop!.....

  int propositionClient;  // Gordon: propositionClient2 has requested you
                          // invite this client to join the fireteam
  int propositionClient2; // Gordon:
  int propositionEndTime; // Gordon: tick, tick, tick....

  int autofireteamEndTime;
  int autofireteamCreateEndTime;
  int autofireteamJoinEndTime;

  playerStats_t playerStats;

  // gentity_t	*wayPoint;

  int lastBattleSenseBonusTime;
  int lastHQMineReportTime;
  int lastCCPulseTime;

  int lastSpawnTime;

  char botScriptName[MAX_NETNAME];

  // OSP
  unsigned int autoaction;       // End-of-match auto-requests
  unsigned int clientFlags;      // Client settings that need server involvement
  unsigned int clientMaxPackets; // Client com_maxpacket settings
  unsigned int clientTimeNudge;  // Client cl_timenudge settings
  int cmd_debounce;              // Dampening of command spam
  unsigned int invite;           // Invitation to a team to join
  int panzerDropTime;   // Time which a player dropping panzer still "has
                        // it" if limiting panzer counts
  int panzerSelectTime; // *when* a client selected a panzer as spawn
                        // weapon
  qboolean ready;       // Ready state to begin play
  // OSP

  bg_character_t *character;
  int characterIndex;

  int hideMe;
  float noclipScale;

  qboolean enableTimeruns;

  int noclipCount;

  int touchPickupWeapons;

  raceStruct_t race;

#define MAX_TOKENS_PER_DIFFICULTY 6
  qboolean collectedEasyTokens[MAX_TOKENS_PER_DIFFICULTY];
  qboolean collectedMediumTokens[MAX_TOKENS_PER_DIFFICULTY];
  qboolean collectedHardTokens[MAX_TOKENS_PER_DIFFICULTY];
  int tokenCollectionStartTime;

  int previousSetHealthTime;

  ETJump::votingInfo_t votingInfo;
} clientPersistant_t;

typedef struct {
  vec3_t mins;
  vec3_t maxs;

  vec3_t origin;

  int time;
} clientMarker_t;

#define MAX_CLIENT_MARKERS 10

#define LT_SPECIAL_PICKUP_MOD                                                  \
  3 // JPW NERVE # of times (minus one for modulo) LT must drop ammo
    // before scoring a point
#define MEDIC_SPECIAL_PICKUP_MOD 4 // JPW NERVE same thing for medic

// Gordon: debris test
typedef struct debrisChunk_s {
  vec3_t origin;
  int model;
  vec3_t velocity;
  char target[32];
  char targetname[32];
} debrisChunk_t;

#define MAX_DEBRISCHUNKS 256

// ===================

// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s {
  // ps MUST be the first element, because the server expects it
  playerState_t ps; // communicated by server to clients

  // the rest of the structure is private to game
  clientPersistant_t pers;
  clientSession_t sess;

  qboolean noclip;
  bool noclipThisLife;

  int lastCmdTime; // level.time of last usercmd_t, for EF_CONNECTION
                   // we can't just use pers.lastCommand.time, because
                   // of the g_sycronousclients case
  int buttons;
  int oldbuttons;
  int latched_buttons;

  int wbuttons;
  int oldwbuttons;
  int latched_wbuttons;
  vec3_t oldOrigin;

  // sum up damage over an entire frame, so
  // shotgun blasts give a single big kick
  int damage_blood;          // damage taken out of health
  int damage_knockback;      // impact damage
  vec3_t damage_from;        // origin for vector calculation
  qboolean damage_fromWorld; // if true, don't use the damage_from vector

  //	int			accurateCount;		// for
  //"impressive"
  // reward sound

  //	int			accuracy_shots;		// total number of
  // shots 	int			accuracy_hits;		// total number
  // of hits

  //
  int lastkilled_client; // last client that this client killed
  int lasthurt_client;   // last client that damaged this client
  int lasthurt_mod;      // type of damage the client did

  // timers
  int respawnTime;            // can respawn when time > this, force after
                              // g_forcerespwan
  int inactivityTime;         // kick players when time > this
  bool inactive;              // level.time >= clientLastActive
                              // + 1000 * clientInactivityTimer
  qboolean inactivityWarning; // qtrue if the five seoond warning has
                              // been given
  int rewardTime; // clear the EF_AWARD_IMPRESSIVE, etc when time > this

  int airOutTime;

  int lastKillTime; // for multiple kill rewards

  qboolean fireHeld; // used for hook
  gentity_t *hook;   // grapple hook if out

  int switchTeamTime; // time the player switched teams

  // timeResidual is used to handle events that happen every second
  // like health / armor countdowns and regeneration
  int timeResidual;

  float currentAimSpreadScale;

  gentity_t *persistantPowerup;
  int portalID;
  int ammoTimes[WP_NUM_WEAPONS];
  int invulnerabilityTime;

  gentity_t *cameraPortal; // grapple hook if out
  vec3_t cameraOrigin;

  int dropWeaponTime;         // JPW NERVE last time a weapon was dropped
  int limboDropWeapon;        // JPW NERVE weapon to drop in limbo
  int deployQueueNumber;      // JPW NERVE player order in reinforcement FIFO
                              // queue
  int lastBurnTime;           // JPW NERVE last time index for flamethrower burn
  int PCSpecialPickedUpCount; // JPW NERVE used to count # of times
                              // somebody's picked up this LTs ammo (or
                              // medic health) (for scoring)
  int saved_persistant[MAX_PERSISTANT]; // DHM - Nerve :: Save
                                        // ps->persistant here during
                                        // Limbo

  gentity_t *touchingTOI; // Arnout: the trigger_objective_info a player
                          // is touching this frame

  int lastConstructibleBlockingWarnTime;
  int lastConstructibleBlockingWarnEnt;

  int landmineSpottedTime;
  gentity_t *landmineSpotted;

  int speedScale;

  combatstate_t combatState;

  int topMarker;
  clientMarker_t clientMarkers[MAX_CLIENT_MARKERS];
  clientMarker_t backupMarker;

  gentity_t *tempHead; // Gordon: storing a temporary head for bullet
                       // head shot detection
  gentity_t *tempLeg;  // Arnout: storing a temporary leg for bullet head
                       // shot detection

  int botSlotNumber; // the slot the bot falls into (set up in the
                     // initial UI screen)
  // END		xkan, 8/27/2002

  int flagParent;

  // the next 2 are used to play the proper animation on the body
  int torsoDeathAnim;
  int legsDeathAnim;

  int lastSpammyCentrePrintTime;
  pmoveExt_t pmext;
  qboolean isCivilian; // whether this is a civilian
  int deathTime;       // if we are dead, when did we die

  int lastHealTimes[2];
  int lastAmmoTimes[2];

  char disguiseNetname[MAX_NETNAME];
  int disguiseRank;

  int medals;
  float acc;

  qboolean hasaward;
  qboolean wantsscore;
  qboolean maxlivescalced;

  int last8BallTime; // Last level.time client used !8ball.
  int lastVoteTime;
  qboolean cheatDetected;

  // Time when client activated trigger_multiple
  int multiTriggerActivationTime;

  // Time when client activated trigger_push
  int pushTriggerActivationTime;

  // Whether the client already activated target_set_health
  qboolean alreadyActivatedSetHealth;

  // Amount of portalgun portals shot since last reset
  int numPortals;
};

typedef struct {
  char modelname[32];
  int model;
} brushmodelInfo_t;

typedef struct limbo_cam_s {
  qboolean hasEnt;
  int targetEnt;
  vec3_t angles;
  vec3_t origin;
  qboolean spawn;
  int info;
} limbo_cam_t;

#define MAX_LIMBO_CAMS 32

#define MAX_IP_LEN 15
#define MAX_IP_MUTES 16

typedef struct ipMute_s {
  qboolean inuse;
  char ip[MAX_IP_LEN + 1];
} ipMute_t;

// this structure is cleared as each map is entered
#define MAX_SPAWN_VARS 128
#define MAX_SPAWN_VARS_CHARS 2048
#define VOTE_MAXSTRING 256 // Same value as MAX_STRING_TOKENS

#define MAX_SCRIPT_ACCUM_BUFFERS 8

#define MAX_BUFFERED_CONFIGSTRINGS 128

typedef struct voteInfo_s {
  char voteString[MAX_STRING_CHARS];
  int voteTime; // level.time vote was called
  int voteYes;
  int voteNo;
  int numVotingClients; // set by CalculateRanks
  int numVotingTeamClients[2];
  int (*vote_fn)(gentity_t *ent, unsigned int dwVoteIndex, char *arg,
                 char *arg2);
  char vote_value[VOTE_MAXSTRING]; // Desired vote item setting.
  char voter_team;
  int voter_cn;
  qboolean voteCanceled;
  qboolean forcePass;

  bool isAutoRtvVote;
} voteInfo_t;

typedef struct {
  struct gclient_s *clients; // [maxclients]

  struct gentity_s *gentities;
  int gentitySize;
  int num_entities; // current number, <= MAX_GENTITIES

  int warmupTime; // restart match at this time

  fileHandle_t logFile;
  fileHandle_t adminLogFile;

  char rawmapname[MAX_QPATH];

  // store latched cvars here that we want to get at often
  int maxclients;

  int framenum;
  int time;         // in msec
  int previousTime; // so movers can back up when blocked
  int frameTime;    // Gordon: time the frame started, for antilag stuff
  int frameMsec;    // server frame time

  int startTime; // level.time the map was started

  int teamScores[TEAM_NUM_TEAMS];
  int lastTeamLocationTime; // last time of client team location update

  qboolean newSession; // don't use any old session data, because
                       // we changed gametype

  qboolean restarted; // waiting for a map_restart to fire

  int numConnectedClients;
  int numNonSpectatorClients;     // includes connecting clients
  int numPlayingClients;          // connected, non-spectators
  int sortedClients[MAX_CLIENTS]; // sorted by score
  int follow1, follow2;           // clientNums for auto-follow spectators

  //	int			snd_fry;				// sound
  // index for standing in lava

  int warmupModificationCount; // for detecting if g_warmup is changed

  voteInfo_t voteInfo;

  int numTeamClients[2];
  int numVotingTeamClients[2];

  // spawn variables
  qboolean spawning; // the G_Spawn*() functions are valid
  int numSpawnVars;
  char *spawnVars[MAX_SPAWN_VARS][2]; // key / value pairs
  int numSpawnVarChars;
  char spawnVarChars[MAX_SPAWN_VARS_CHARS];

  // intermission state
  int intermissionQueued; // intermission was qualified, but
                          // wait INTERMISSION_DELAY_TIME before
                          // actually going there so the last
                          // frag can be watched.  Disable future
                          // kills during this delay
  int intermissiontime;   // time the intermission was started
  char *changemap;
  int exitTime;
  vec3_t intermission_origin; // also used for spectator spawns
  vec3_t intermission_angle;
  qboolean lmsDoNextMap; // should LMS do a map_restart or a vstr nextmap

  int bodyQueIndex; // dead bodies
  gentity_t *bodyQue[BODY_QUEUE_SIZE];

  int portalSequence;
  // Ridah
  char *scriptAI;
  int reloadPauseTime; // don't think AI/client's until this time has
                       // elapsed
  int reloadDelayTime; // don't start loading the savegame until this
                       // has expired

  int capturetimes[4]; // red, blue, none, spectator for WOLF_MP_CPH
  int redReinforceTime,
      blueReinforceTime; // last time reinforcements arrived in ms
  int redNumWaiting,
      blueNumWaiting; // number of reinforcements in queue
  vec3_t spawntargets[MAX_MULTI_SPAWNTARGETS]; // coordinates of spawn
                                               // targets
  int numspawntargets;                         // # spawntargets in this map

  // RF, entity scripting
  char *scriptEntity;

  // player/AI model scripting (server repository)
  animScriptData_t animScriptData;

  int totalHeadshots;
  int missedHeadshots;
  int lastRestartTime;

  int numFinalDead[2]; // DHM - Nerve :: unable to respawn and in limbo
                       // (per team)
  int numOidTriggers;  // DHM - Nerve

  qboolean latchGametype; // DHM - Nerve

  // RF
  int attackingTeam;       // which team is attacking
  int explosiveTargets[2]; // attackers need to explode something to get
                           // through
  qboolean captureFlagMode;
  qboolean initStaticEnts;
  qboolean initSeekCoverChains;
  char *botScriptBuffer;
  int globalAccumBuffer[MAX_SCRIPT_ACCUM_BUFFERS];

  int soldierChargeTime[2];
  int medicChargeTime[2];
  int engineerChargeTime[2];
  int lieutenantChargeTime[2];

  int covertopsChargeTime[2];

  int lastMapEntityUpdate;
  int objectiveStatsAllies[MAX_OBJECTIVES];
  int objectiveStatsAxis[MAX_OBJECTIVES];

  int lastSystemMsgTime[2];

  float soldierChargeTimeModifier[2];
  float medicChargeTimeModifier[2];
  float engineerChargeTimeModifier[2];
  float lieutenantChargeTimeModifier[2];
  float covertopsChargeTimeModifier[2];

  int firstbloodTeam;
  int teamEliminateTime;
  int lmsWinningTeam;

  int campaignCount;
  int currentCampaign;
  qboolean newCampaign;

  brushmodelInfo_t brushModelInfo[128];
  int numBrushModels;
  gentity_t *gameManager;

  // record last time we loaded, so we can hack around sighting issues
  // on reload
  int lastLoadTime;

  qboolean doorAllowTeams; // used by bots to decide whether or not to
                           // use team travel flags

  // Gordon: for multiplayer fireteams
  fireteamData_t fireTeams[MAX_FIRETEAMS];

  qboolean ccLayers;

  // OSP
  int dwBlueReinfOffset;
  int dwRedReinfOffset;
  qboolean fLocalHost;
  qboolean fResetStats;
  int match_pause;       // Paused state of the match
  qboolean ref_allready; // Referee forced match start
  int server_settings;
  int sortedStats[MAX_CLIENTS]; // sorted by weapon stat
  int timeCurrent;              // Real game clock
  int timeDelta; // Offset from internal clock - used to calculate real
                 // match time
  // OSP

  qboolean mapcoordsValid, tracemapLoaded;
  vec2_t mapcoordsMins, mapcoordsMaxs;

  char tinfoAxis[1400];
  char tinfoAllies[1400];

  // Gordon: debris test
  int numDebrisChunks;
  debrisChunk_t debrisChunks[MAX_DEBRISCHUNKS];
  // ===================

  qboolean disableTankExit;
  qboolean disableTankEnter;

  int axisBombCounter, alliedBombCounter;
  int axisAutoSpawn, alliesAutoSpawn;
  int axisMG42Counter, alliesMG42Counter;

  int lastClientBotThink;

  limbo_cam_t limboCams[MAX_LIMBO_CAMS];
  int numLimboCams;

  int numActiveAirstrikes[2];

  float teamXP[SK_NUM_SKILLS][2];

  commanderTeamChat_t commanderSounds[2][MAX_COMMANDER_TEAM_SOUNDS];
  int commanderSoundInterval[2];
  int commanderLastSoundTime[2];

  qboolean tempTraceIgnoreEnts[MAX_GENTITIES];

  // Deprecated
  int noExplosives;

  qboolean noSave;
  qboolean noNoclip;
  qboolean noGod;
  qboolean noGoto;
  bool noOverbounce;
  bool noJumpDelay;
  bool noFallDamage;
  bool noProne;
  bool noDrop;

  int portalEnabled; // Feen: PGM - Enabled/Disabled by map key
  qboolean portalSurfaces;

  ipMute_t ipMutes[MAX_IP_MUTES]; // I don't think we need more than 16

  qboolean ghostPlayers;
  int limitedSaves;
  int portalTeam;

#define MAX_TIMERUNS 20
#define MAX_TIMERUN_NAME_LENGTH 64
  int timerunNamesCount;
  char timerunNames[MAX_TIMERUNS][MAX_TIMERUN_NAME_LENGTH];
  bool hasTimerun;
  int saveLoadRestrictions;
  int checkpointsCount[MAX_TIMERUNS];
} level_locals_t;

typedef struct {
  char mapnames[MAX_MAPS_PER_CAMPAIGN][MAX_QPATH];
  // arenaInfo_t	arenas[MAX_MAPS_PER_CAMPAIGN];
  int mapCount;
  int current;

  char shortname[256];
  char next[256];
  int typeBits;
} g_campaignInfo_t;

//
// g_spawn.c
//
#define G_SpawnString(key, def, out)                                           \
  G_SpawnStringExt(key, def, out, __FILE__, __LINE__)
#define G_SpawnFloat(key, def, out)                                            \
  G_SpawnFloatExt(key, def, out, __FILE__, __LINE__)
#define G_SpawnInt(key, def, out)                                              \
  G_SpawnIntExt(key, def, out, __FILE__, __LINE__)
#define G_SpawnVector(key, def, out)                                           \
  G_SpawnVectorExt(key, def, out, __FILE__, __LINE__)
#define G_SpawnVector2D(key, def, out)                                         \
  G_SpawnVector2DExt(key, def, out, __FILE__, __LINE__)

qboolean
G_SpawnStringExt(const char *key, const char *defaultString, char **out,
                 const char *file,
                 int line); // spawn string returns a temporary reference, you
                            // must CopyString() if you want to keep it
qboolean G_SpawnFloatExt(const char *key, const char *defaultString, float *out,
                         const char *file, int line);
qboolean G_SpawnIntExt(const char *key, const char *defaultString, int *out,
                       const char *file, int line);
qboolean G_SpawnVectorExt(const char *key, const char *defaultString,
                          float *out, const char *file, int line);
qboolean G_SpawnVector2DExt(const char *key, const char *defaultString,
                            float *out, const char *file, int line);

void G_SpawnEntitiesFromString(void);
char *G_NewString(const char *string);
// Ridah
qboolean G_CallSpawn(gentity_t *ent);
// done.
char *G_AddSpawnVarToken(const char *string);
void G_ParseField(const char *key, const char *value, gentity_t *ent);
//
// g_cmds.c
//
void Cmd_Score_f(gentity_t *ent);
void StopFollowing(gentity_t *ent);
// void BroadcastTeamChange( gclient_t *client, int oldTeam );
void G_TeamDataForString(const char *teamstr, int clientNum, team_t *team,
                         spectatorState_t *sState, int *specClient);
qboolean SetTeam(gentity_t *ent, const char *s, qboolean force, weapon_t w1,
                 weapon_t w2, qboolean setweapons);
void G_SetClientWeapons(gentity_t *ent, weapon_t w1, weapon_t w2,
                        qboolean updateclient);
weapon_t G_GetDefaultWeaponForClass(gentity_t *ent, char *s, bool primary);
void Cmd_FollowCycle_f(gentity_t *ent, int dir);
void Cmd_Kill_f(gentity_t *ent);
void Cmd_SwapPlacesWithBot_f(gentity_t *ent, int botNum);
void G_EntitySound(gentity_t *ent, const char *soundId, int volume);
void G_EntitySoundNoCut(gentity_t *ent, const char *soundId, int volume);
int ClientNumberFromString(gentity_t *to, char *s);
int ClientNumbersFromString(const char *s, int *plist);
qboolean G_MatchOnePlayer(int *plist, char *err, int len,
                          team_t filter = TEAM_FREE);
void SanitizeString(char *in, char *out, qboolean fToLower);
void SanitizeConstString(const char *in, char *out, qboolean fToLower);
int CleanStrlen(const char *in);
void Cmd_PrivateMessage_f(gentity_t *ent);
void Cmd_noGoto_f(gentity_t *ent);
void Cmd_noCall_f(gentity_t *ent);
qboolean G_AllowFollow(gentity_t *ent, gentity_t *other);
qboolean G_DesiredFollow(gentity_t *ent, gentity_t *other);
void Cmd_FollowCycle_f(gentity_t *ent, int dir);
qboolean ClientIsFlooding(gentity_t *ent);
char *ConcatArgs(int start);

//
// g_items.c
//
void G_RunItem(gentity_t *ent);
void RespawnItem(gentity_t *ent);

void UseHoldableItem(gentity_t *ent, int item);
gentity_t *Drop_Item(gentity_t *ent, gitem_t *item, float angle,
                     qboolean novelocity);
gentity_t *LaunchItem(gitem_t *item, vec3_t origin, vec3_t velocity,
                      int ownerNum);
void G_SpawnItem(gentity_t *ent, gitem_t *item);
void FinishSpawningItem(gentity_t *ent);
void Fill_Clip(playerState_t *ps, int weapon);
int Add_Ammo(gentity_t *ent, int weapon, int count, qboolean fillClip);
void Touch_Item(gentity_t *ent, gentity_t *other, trace_t *trace);
qboolean AddMagicAmmo(gentity_t *receiver, int numOfClips);
weapon_t G_GetPrimaryWeaponForClient(gclient_t *client);
void G_DropWeapon(gentity_t *ent, weapon_t weapon);

// Touch_Item_Auto is bound by the rules of autoactivation (if cg_autoactivate
// is 0, only touch on "activate")
void Touch_Item_Auto(gentity_t *ent, gentity_t *other, trace_t *trace);

void Touch_Item_Give(gentity_t *ent, gentity_t *other,
                     trace_t *trace); // Called by target_give

void Prop_Break_Sound(gentity_t *ent);
void Spawn_Shard(gentity_t *ent, gentity_t *inflictor, int quantity, int type);

//
// g_utils.c
//
// Ridah
int G_FindConfigstringIndex(const char *name, int start, int max,
                            qboolean create);
// done.
int G_ModelIndex(const char *name);
int G_SoundIndex(const char *name);
int G_SkinIndex(const char *name);
int G_ShaderIndex(char *name);
int G_CharacterIndex(const char *name);
int G_StringIndex(const char *string);
qboolean G_AllowTeamsAllowed(gentity_t *ent, gentity_t *activator);
void G_UseEntity(gentity_t *ent, gentity_t *other, gentity_t *activator);
qboolean G_IsWeaponDisabled(gentity_t *ent, weapon_t weapon);
void G_TeamCommand(team_t team, char *cmd);
void G_KillBox(gentity_t *ent);
gentity_t *G_Find(gentity_t *from, int fieldofs, const char *match);
gentity_t *G_FindByTargetname(gentity_t *from, const char *match);
gentity_t *G_FindByTargetnameFast(gentity_t *from, const char *match, int hash);
gentity_t *G_PickTarget(char *targetname);
void G_UseTargets(gentity_t *ent, gentity_t *activator);
void G_UseTargetedEntities(gentity_t *ent, gentity_t *activator);
void G_SetMovedir(vec3_t angles, vec3_t movedir);

void G_InitGentity(gentity_t *e);
gentity_t *G_Spawn(void);
gentity_t *G_TempEntity(vec3_t origin, int event);
gentity_t *G_PopupMessage(popupMessageType_t type);
void G_Sound(gentity_t *ent, int soundIndex);
void G_AnimScriptSound(int soundIndex, vec3_t org, int client);
void G_FreeEntity(gentity_t *e);
// qboolean	G_EntitiesFree( void );

void G_TouchTriggers(gentity_t *ent);

void G_AddPredictableEvent(gentity_t *ent, int event, int eventParm);
void G_AddEvent(gentity_t *ent, int event, int eventParm);
void G_SetOrigin(gentity_t *ent, vec3_t origin);
void AddRemap(const char *oldShader, const char *newShader, float timeOffset);
const char *BuildShaderStateConfig();
void G_SetAngle(gentity_t *ent, vec3_t angle);

qboolean infront(gentity_t *self, gentity_t *other);

void G_ProcessTagConnect(gentity_t *ent, qboolean clearAngles);

void G_SetEntState(gentity_t *ent, entState_t state);
void G_ParseCampaigns(void);
qboolean G_MapIsValidCampaignStartMap(void);

team_t G_GetTeamFromEntity(gentity_t *ent);
const char *ClientIPAddr(gentity_t *ent);
namespace ETJump {
gentity_t *soundEvent(vec3_t origin, entity_event_t eventType, int soundIndex);
void initRemappedShaders();
} // namespace ETJump

//
// g_combat.c
//
void G_AdjustedDamageVec(gentity_t *ent, vec3_t origin, vec3_t vec);
qboolean CanDamage(gentity_t *targ, vec3_t origin);
void G_Damage(gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
              vec3_t dir, vec3_t point, int damage, int dflags, int mod);
qboolean G_RadiusDamage(vec3_t origin, gentity_t *inflictor,
                        gentity_t *attacker, float damage, float radius,
                        gentity_t *ignore, int mod);
qboolean etpro_RadiusDamage(vec3_t origin, gentity_t *inflictor,
                            gentity_t *attacker, float damage, float radius,
                            gentity_t *ignore, int mod, qboolean clientsonly);
void body_die(gentity_t *self, gentity_t *inflictor, gentity_t *attacker,
              int damage, int meansOfDeath);
void TossClientItems(gentity_t *self);
gentity_t *G_BuildHead(gentity_t *ent);
gentity_t *G_BuildLeg(gentity_t *ent);

// damage flags
#define DAMAGE_RADIUS 0x00000001         // damage was indirect
#define DAMAGE_HALF_KNOCKBACK 0x00000002 // Gordon: do less knockback
#define DAMAGE_NO_KNOCKBACK                                                    \
  0x00000008 // do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION                                                   \
  0x00000020 // armor, shields, invulnerability, and godmode have no
             // effect
#define DAMAGE_NO_TEAM_PROTECTION                                              \
  0x00000010 // armor, shields, invulnerability, and godmode have no
             // effect
#define DAMAGE_DISTANCEFALLOFF 0x00000040 // distance falloff

//
// g_missile.c
//
void G_RunMissile(gentity_t *ent);
void G_RunBomb(gentity_t *ent);
int G_PredictMissile(gentity_t *ent, int duration, vec3_t endPos,
                     qboolean allowBounce);
void G_TripMinePrime(gentity_t *ent);
qboolean G_HasDroppedItem(gentity_t *ent, int modType);

// DHM - Nerve :: server side flamethrower collision
void G_RunFlamechunk(gentity_t *ent);

//----(SA) removed unused q3a weapon firing
gentity_t *fire_flamechunk(gentity_t *self, vec3_t start, vec3_t dir);

gentity_t *fire_grenade(gentity_t *self, const vec3_t start,
                        const vec3_t aimdir, int grenadeWPID);
gentity_t *fire_rocket(gentity_t *self, vec3_t start, vec3_t dir);

#define Fire_Lead(ent, activator, spread, damage, muzzle, forward, right, up)  \
  Fire_Lead_Ext(ent, activator, spread, damage, muzzle, forward, right, up,    \
                MOD_MACHINEGUN)
void Fire_Lead_Ext(gentity_t *ent, gentity_t *activator, float spread,
                   int damage, vec3_t muzzle, vec3_t forward, vec3_t right,
                   vec3_t up, int mod);
void fire_lead(gentity_t *self, vec3_t start, vec3_t dir, int damage);
qboolean visible(gentity_t *self, gentity_t *other);

gentity_t *fire_mortar(gentity_t *self, vec3_t start, vec3_t dir);
gentity_t *fire_flamebarrel(gentity_t *self, vec3_t start, vec3_t dir);
// done

//
// g_mover.c
//
gentity_t *G_TestEntityPosition(gentity_t *ent);
void G_RunMover(gentity_t *ent);
qboolean G_MoverPush(gentity_t *pusher, vec3_t move, vec3_t amove,
                     gentity_t **obstacle);
void Use_BinaryMover(gentity_t *ent, gentity_t *other, gentity_t *activator);
void G_Activate(gentity_t *ent, gentity_t *activator);

void G_TryDoor(gentity_t *ent, gentity_t *other,
               gentity_t *activator); //----(SA)	added

void InitMoverRotate(gentity_t *ent);

void InitMover(gentity_t *ent);
void SetMoverState(gentity_t *ent, moverState_t moverState, int time);

void func_constructible_underconstructionthink(gentity_t *ent);

//
// g_trigger.c
//
void Think_SetupObjectiveInfo(gentity_t *ent);
void InitTrigger(gentity_t *self);

//
// g_misc.c
//
void TeleportPlayer(gentity_t *player, vec3_t origin, vec3_t angles);
void TeleportPlayerExt(gentity_t *player, vec3_t origin, vec3_t angles);
void TeleportPlayerKeepAngles_Clank(gentity_t *player, gentity_t *trigger,
                                    const vec3_t origin, const vec3_t angles);
void TeleportPlayerKeepAngles(gentity_t *player, gentity_t *trigger,
                              const vec3_t origin, const vec3_t angles);
void DirectTeleport(gentity_t *player, vec3_t origin, vec3_t angles);
void PortalTeleport(gentity_t *player, vec3_t origin,
                    vec3_t angles); // Feen: PGM
void mg42_fire(gentity_t *other);
void mg42_stopusing(gentity_t *self);
void aagun_fire(gentity_t *other);

//
// g_weapon.c
//
qboolean AccuracyHit(gentity_t *target, gentity_t *attacker);
void CalcMuzzlePoint(gentity_t *ent, int weapon, vec3_t forward, vec3_t right,
                     vec3_t up, vec3_t muzzlePoint);
void SnapVectorTowards(vec3_t v, vec3_t to);
gentity_t *weapon_grenadelauncher_fire(gentity_t *ent, int grenadeWPID);
void G_PlaceTripmine(gentity_t *ent);
void G_FadeItems(gentity_t *ent, int modType);
gentity_t *G_FindSatchel(gentity_t *ent);
void G_ExplodeMines(gentity_t *ent);
qboolean G_ExplodeSatchels(gentity_t *ent);
qboolean G_ExplodeSatchelsExt(gentity_t *ent);
void G_FreeSatchel(gentity_t *ent);
int G_GetWeaponDamage(int weapon);

void CalcMuzzlePoints(gentity_t *ent, int weapon);
void CalcMuzzlePointForActivate(gentity_t *ent, vec3_t forward, vec3_t right,
                                vec3_t up, vec3_t muzzlePoint);

//
// g_client.c
//
team_t TeamCount(int ignoreClientNum,
                 int team); // NERVE - SMF - merge from team arena
team_t PickTeam(int ignoreClientNum);
void SetClientViewAngle(gentity_t *ent, vec3_t angle);
gentity_t *SelectSpawnPoint(vec3_t avoidPoint, vec3_t origin, vec3_t angles);
void respawn(gentity_t *ent);
void BeginIntermission(void);
void InitBodyQue(void);
void ClientSpawn(gentity_t *ent, qboolean revived);
void player_die(gentity_t *self, gentity_t *inflictor, gentity_t *attacker,
                int damage, int mod);
void AddScore(gentity_t *ent, int score);
void AddKillScore(gentity_t *ent, int score);
void CalculateRanks(void);
qboolean SpotWouldTelefrag(gentity_t *spot);
void G_StartPlayerAppropriateSound(gentity_t *ent, char *soundType);
void SetWolfSpawnWeapons(gclient_t *client);
void limbo(gentity_t *ent, qboolean makeCorpse); // JPW NERVE
void reinforce(gentity_t *ent);                  // JPW NERVE
qboolean AddWeaponToPlayer(gclient_t *client, weapon_t weapon, int ammo,
                           int ammoclip, qboolean setcurrent);
void ResetPlayerAmmo(gclient_t *client, gentity_t *ent);
void ClearPortals(gentity_t *ent);
//
// g_character.c
//

qboolean G_RegisterCharacter(const char *characterFile,
                             bg_character_t *character);
void G_RegisterPlayerClasses(void);
// void G_SetCharacter( gclient_t *client, bg_character_t *character, qboolean
// custom );
void G_UpdateCharacter(gclient_t *client);

//
// g_svcmds.c
//
qboolean ConsoleCommand(void);
void G_ProcessIPBans(void);
qboolean G_FilterIPBanPacket(const char *from);
void AddIPBan(const char *str);

//
// g_weapon.c
//
void FireWeapon(gentity_t *ent);
void G_BurnMeGood(gentity_t *self, gentity_t *body);

//
// IsSilencedWeapon
//
// Description: Is the specified weapon a silenced weapon?
// Written: 12/26/2002
//
qboolean IsSilencedWeapon(
    // The type of weapon in question.  Is it silenced?
    int weaponType);

//
// p_hud.c
//
void MoveClientToIntermission(gentity_t *client);
void G_SendScore(gentity_t *client);

//
// g_cmds.c
//
void G_SayTo(
    gentity_t *ent, gentity_t *other, int mode, int color, const char *name,
    const char *message, qboolean localize,
    qboolean encoded); // JPW NERVE removed static declaration so it would link
void Cmd_CallVote_f(gentity_t *ent, unsigned int dwCommand, qboolean fValue);
void Cmd_Follow_f(gentity_t *ent, unsigned int dwCommand, qboolean fValue);
void Cmd_Say_f(gentity_t *ent, int mode, qboolean arg0, qboolean encoded);
void Cmd_Team_f(gentity_t *ent);
void Cmd_Team2_f(gentity_t *ent, unsigned int dwCommand, qboolean fValue);
void Cmd_SetWeapons_f(gentity_t *ent, unsigned int dwCommand, qboolean fValue);
void Cmd_SetClass_f(gentity_t *ent, unsigned int dwCommand, qboolean fValue);

//
// g_main.c
//
void FindIntermissionPoint(void);
void G_RunThink(gentity_t *ent);
void QDECL G_LogPrintf(const char *fmt, ...);
void SendScoreboardMessageToAllClients(void);
void QDECL G_Printf(const char *fmt, ...);
void QDECL G_DPrintf(const char *fmt, ...);
void QDECL G_Error(const char *fmt, ...);
// Is this a single player type game - sp or coop?
qboolean G_IsSinglePlayerGame();
void resetVote();

//
// g_client.c
//
const char *ClientConnect(int clientNum, qboolean firstTime, qboolean isBot);
void ClientUserinfoChanged(int clientNum);
void ClientDisconnect(int clientNum);
void ClientBegin(int clientNum);
void ClientCommand(int clientNum);
namespace ETJump {
bool UpdateClientConfigString(gentity_t &gent);
}

//
// g_active.c
//
void ClientThink(int clientNum);
void ClientEndFrame(gentity_t *ent);
void G_RunClient(gentity_t *ent);
qboolean ClientNeedsAmmo(int client);

// Does ent have enough "energy" to call artillery?
qboolean ReadyToCallArtillery(gentity_t *ent);
// Are we ready to construct?  Optionally, will also update the time while we
// are constructing
qboolean ReadyToConstruct(gentity_t *ent, gentity_t *constructible,
                          qboolean updateState);

//
// g_team.c
//
qboolean OnSameTeam(gentity_t *ent1, gentity_t *ent2);
qboolean OnAnyTeam(gentity_t *ent1, gentity_t *ent2);
int Team_ClassForString(char *string);
void Team_ResetFlags(void);

//
// g_mem.c
//
void *G_Alloc(int size);
void G_InitMemory(void);
void Svcmd_GameMem_f(void);

//
// g_session.c
//
void G_ReadSessionData(gclient_t *client);
void G_InitSessionData(gclient_t *client, char *userinfo);

void G_InitWorldSession(void);
void G_WriteSessionData(qboolean restart);

void G_CalcRank(gclient_t *client);

// ai_main.c
#define MAX_FILEPATH 144

// g_cmd.c
void Cmd_Activate_f(gentity_t *ent);
void Cmd_Activate2_f(gentity_t *ent);
qboolean Do_Activate_f(gentity_t *ent, gentity_t *traceEnt);
void G_LeaveTank(gentity_t *ent, qboolean position);

namespace ETJump {
void longRangeActivate(gentity_t *ent);
}

// g_script.c
void G_Script_ScriptParse(gentity_t *ent);
qboolean G_Script_ScriptRun(gentity_t *ent);
void G_Script_ScriptEvent(gentity_t *ent, const char *eventStr,
                          const char *params);
void G_Script_ScriptLoad(void);
void G_Script_EventStringInit(void);

void mountedmg42_fire(gentity_t *other);
void script_mover_use(gentity_t *ent, gentity_t *other, gentity_t *activator);
void script_mover_blocked(gentity_t *ent, gentity_t *other);

float AngleDifference(float ang1, float ang2);

// g_props.c
void Props_Chair_Skyboxtouch(gentity_t *ent);

#include "g_team.h" // teamplay specific stuff
extern level_locals_t level;
extern gentity_t g_entities[]; // DAJ was explicit set to MAX_ENTITIES
extern g_campaignInfo_t g_campaigns[];
extern int saveGamePending;

#define FOFS(x) (int(((intptr_t) & (((gentity_t *)0)->x))))

extern vmCvar_t g_gametype;

extern vmCvar_t g_logFile;
extern vmCvar_t g_dedicated;
extern vmCvar_t g_cheats;
extern vmCvar_t g_maxclients;     // allow this many total, including spectators
extern vmCvar_t g_maxGameClients; // allow this many active
extern vmCvar_t g_minGameClients; // NERVE - SMF - we need at least this many
                                  // before match actually starts
extern vmCvar_t g_restarted;

extern vmCvar_t g_timelimit;
extern vmCvar_t g_password;
extern vmCvar_t sv_privatepassword;
extern vmCvar_t g_knockback;
extern vmCvar_t g_forcerespawn;
extern vmCvar_t g_inactivity;
extern vmCvar_t g_debugMove;
extern vmCvar_t g_debugAlloc;
extern vmCvar_t g_debugDamage;
extern vmCvar_t g_debugBullets; //----(SA)	added
#ifdef ALLOW_GSYNC
extern vmCvar_t g_synchronousClients;
#endif // ALLOW_GSYNC
extern vmCvar_t g_motd;
extern vmCvar_t g_warmup;
extern vmCvar_t voteFlags;

// DHM - Nerve :: The number of complaints allowed before kick/ban
extern vmCvar_t g_filtercams;
extern vmCvar_t
    g_voiceChatsAllowed;     // DHM - Nerve :: number before spam control
extern vmCvar_t g_fastres;   // Xian - Fast medic res'ing
extern vmCvar_t g_knifeonly; // Xian - Wacky Knife-Only rounds

extern vmCvar_t g_needpass;
extern vmCvar_t g_doWarmup;
extern vmCvar_t g_banIPs;
extern vmCvar_t g_filterBan;
extern vmCvar_t g_smoothClients;
extern vmCvar_t pmove_msec;

// Rafael
extern vmCvar_t g_scriptName; // name of script file to run (instead of default
                              // for that map)

extern vmCvar_t g_scriptDebug;

extern vmCvar_t g_userAim;
extern vmCvar_t g_developer;

// JPW NERVE multiplayer
extern vmCvar_t g_redlimbotime;
extern vmCvar_t g_bluelimbotime;
extern vmCvar_t g_medicChargeTime;
extern vmCvar_t g_engineerChargeTime;
extern vmCvar_t g_LTChargeTime;
extern vmCvar_t g_soldierChargeTime;
// jpw

extern vmCvar_t g_covertopsChargeTime;
extern vmCvar_t g_debugConstruct;
extern vmCvar_t g_landminetimeout;

// What level of detail do we want script printing to go to.
extern vmCvar_t g_scriptDebugLevel;

// How fast do SP player and allied bots move?
extern vmCvar_t g_movespeed;

extern vmCvar_t g_oldCampaign;
extern vmCvar_t g_currentCampaign;
extern vmCvar_t g_currentCampaignMap;

// Arnout: for LMS
extern vmCvar_t g_axiswins;
extern vmCvar_t g_alliedwins;

#ifdef SAVEGAME_SUPPORT
extern vmCvar_t g_reloading;
#endif // SAVEGAME_SUPPORT

// NERVE - SMF
extern vmCvar_t g_nextTimeLimit;
extern vmCvar_t g_userTimeLimit;
extern vmCvar_t g_userAlliedRespawnTime;
extern vmCvar_t g_userAxisRespawnTime;
extern vmCvar_t g_currentRound;
extern vmCvar_t g_altStopwatchMode;
extern vmCvar_t g_gamestate;
extern vmCvar_t g_swapteams;
// -NERVE - SMF

// Gordon
extern vmCvar_t g_antilag;

// OSP
extern vmCvar_t refereePassword;
extern vmCvar_t g_spectatorInactivity;
extern vmCvar_t match_latejoin;
extern vmCvar_t match_minplayers;
extern vmCvar_t match_readypercent;
extern vmCvar_t match_timeoutcount;
extern vmCvar_t match_timeoutlength;
extern vmCvar_t match_warmupDamage;
extern vmCvar_t server_autoconfig;
extern vmCvar_t server_motd0;
extern vmCvar_t server_motd1;
extern vmCvar_t server_motd2;
extern vmCvar_t server_motd3;
extern vmCvar_t server_motd4;
extern vmCvar_t server_motd5;
extern vmCvar_t team_maxPanzers;
extern vmCvar_t team_maxplayers;
extern vmCvar_t team_nocontrols;
//
// NOTE!!! If any vote flags are added, MAKE SURE to update the voteFlags struct
// in bg_misc.c w/appropriate info,
//         menudef.h for the mask and g_main.c for vote_allow_* flag updates
//
extern vmCvar_t vote_allow_map;
extern vmCvar_t vote_allow_matchreset;
extern vmCvar_t vote_allow_randommap;
extern vmCvar_t vote_allow_rtv;
extern vmCvar_t vote_allow_autoRtv;
extern vmCvar_t vote_limit;
extern vmCvar_t vote_percent;
extern vmCvar_t g_letterbox;
extern vmCvar_t bot_enable;

extern vmCvar_t g_debugSkills;
extern vmCvar_t g_autoFireteams;

extern vmCvar_t g_nextmap;
extern vmCvar_t g_nextcampaign;

extern vmCvar_t g_dailyLogs;

extern vmCvar_t g_save;
extern vmCvar_t g_floodprotection;
extern vmCvar_t g_floodlimit;
extern vmCvar_t g_floodwait;
extern vmCvar_t g_ghostPlayers;
extern vmCvar_t g_nofatigue;
extern vmCvar_t g_blockCheatCvars;
extern vmCvar_t g_weapons;
extern vmCvar_t g_noclip;
extern vmCvar_t g_nameChangeLimit;
extern vmCvar_t g_nameChangeInterval;

extern vmCvar_t g_mapScriptDir;
extern vmCvar_t g_blockedMaps;

extern vmCvar_t g_adminLog;
extern vmCvar_t g_userConfig;
extern vmCvar_t g_levelConfig;

extern vmCvar_t g_bannerLocation;
extern vmCvar_t g_bannerTime;
extern vmCvar_t g_banner1;
extern vmCvar_t g_banner2;
extern vmCvar_t g_banner3;
extern vmCvar_t g_banner4;
extern vmCvar_t g_banner5;
extern vmCvar_t g_banners;

// Feen: PGM
extern vmCvar_t g_portalDebug;
extern vmCvar_t g_portalMode;

extern vmCvar_t g_maxConnsPerIP;
extern vmCvar_t g_mute;
extern vmCvar_t g_goto;
extern vmCvar_t g_voteCooldown;

extern vmCvar_t mod_version;

extern vmCvar_t g_mapDatabase;
extern vmCvar_t g_banDatabase;

extern vmCvar_t g_disableVoteAfterMapChange;

extern vmCvar_t g_motdFile;
extern vmCvar_t g_customMapVotesFile;

// Start of timeruns support
extern vmCvar_t g_timerunsDatabase;
extern vmCvar_t g_timeruns2Database;
// End of timeruns support

// tokens
extern vmCvar_t g_tokensMode;
extern vmCvar_t g_tokensPath;
// end of tokens

extern vmCvar_t g_chatOptions;

// vchat customization
extern vmCvar_t g_customVoiceChat;

extern vmCvar_t shared;
extern vmCvar_t g_moverScale;
extern vmCvar_t g_debugTrackers;
extern vmCvar_t g_debugTimeruns;
extern vmCvar_t g_spectatorVote;
extern vmCvar_t g_enableVote;

extern vmCvar_t g_autoRtv;
extern vmCvar_t g_rtvMapCount;
extern vmCvar_t vote_minRtvDuration;

void trap_Printf(const char *fmt);
void trap_Error(const char *fmt);
int trap_Milliseconds(void);
int trap_Argc(void);
void trap_Argv(int n, char *buffer, int bufferLength);
void trap_Args(char *buffer, int bufferLength);
int trap_FS_FOpenFile(const char *qpath, fileHandle_t *f, fsMode_t mode);
void trap_FS_Read(void *buffer, int len, fileHandle_t f);
int trap_FS_Write(const void *buffer, int len, fileHandle_t f);
int trap_FS_Rename(const char *from, const char *to);
void trap_FS_FCloseFile(fileHandle_t f);
int trap_FS_GetFileList(const char *path, const char *extension, char *listbuf,
                        int bufsize);
void trap_SendConsoleCommand(int exec_when, const char *text);
void trap_Cvar_Register(vmCvar_t *cvar, const char *var_name, const char *value,
                        int flags);
void trap_Cvar_Update(vmCvar_t *cvar);
void trap_Cvar_Set(const char *var_name, const char *value);
int trap_Cvar_VariableIntegerValue(const char *var_name);
float trap_Cvar_VariableValue(const char *var_name);
void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer,
                                    int bufsize);
void trap_Cvar_LatchedVariableStringBuffer(const char *var_name, char *buffer,
                                           int bufsize);
void trap_LocateGameData(gentity_t *gEnts, int numGEntities,
                         int sizeofGEntity_t, playerState_t *gameClients,
                         int sizeofGameClient);
void trap_DropClient(int clientNum, const char *reason, int length);
void trap_SendServerCommand(int clientNum, const char *text);
void trap_SetConfigstring(int num, const char *string);
void trap_GetConfigstring(int num, char *buffer, int bufferSize);
void trap_GetUserinfo(int num, char *buffer, int bufferSize);
void trap_SetUserinfo(int num, const char *buffer);
void trap_GetServerinfo(char *buffer, int bufferSize);
void trap_SetBrushModel(gentity_t *ent, const char *name);
void trap_Trace(trace_t *results, const vec3_t start, const vec3_t mins,
                const vec3_t maxs, const vec3_t end, int passEntityNum,
                int contentmask);
void trap_TraceCapsule(trace_t *results, const vec3_t start, const vec3_t mins,
                       const vec3_t maxs, const vec3_t end, int passEntityNum,
                       int contentmask);
void trap_TraceCapsuleNoEnts(trace_t *results, const vec3_t start,
                             const vec3_t mins, const vec3_t maxs,
                             const vec3_t end, int passEntityNum,
                             int contentmask);
void trap_TraceNoEnts(trace_t *results, const vec3_t start, const vec3_t mins,
                      const vec3_t maxs, const vec3_t end, int passEntityNum,
                      int contentmask);
int trap_PointContents(const vec3_t point, int passEntityNum);
qboolean trap_InPVS(const vec3_t p1, const vec3_t p2);
qboolean trap_InPVSIgnorePortals(const vec3_t p1, const vec3_t p2);
void trap_AdjustAreaPortalState(gentity_t *ent, qboolean open);
qboolean trap_AreasConnected(int area1, int area2);
void trap_LinkEntity(gentity_t *ent);
void trap_UnlinkEntity(gentity_t *ent);
int trap_EntitiesInBox(const vec3_t mins, const vec3_t maxs, int *entityList,
                       int maxcount);
qboolean trap_EntityContact(const vec3_t mins, const vec3_t maxs,
                            const gentity_t *ent);
qboolean trap_EntityContactCapsule(const vec3_t mins, const vec3_t maxs,
                                   const gentity_t *ent);
int trap_BotAllocateClient(int clientNum);
void trap_BotFreeClient(int clientNum);
void trap_GetUsercmd(int clientNum, usercmd_t *cmd);
qboolean trap_GetEntityToken(char *buffer, int bufferSize);
qboolean trap_GetTag(int clientNum, int tagFileNumber, const char *tagName,
                     orientation_t *orientation);
int trap_LoadTag(const char *filename);

int trap_RealTime(qtime_t *qtime);

int trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points);
void trap_DebugPolygonDelete(int id);

int trap_BotLibSetup(void);
int trap_BotLibShutdown(void);
int trap_BotLibVarSet(char *var_name, char *value);
int trap_BotLibVarGet(char *var_name, char *value, int size);
int trap_BotLibDefine(char *string);
int trap_BotLibStartFrame(float time);
int trap_BotLibLoadMap(const char *mapname);
int trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue);
int trap_BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);

int trap_BotGetSnapshotEntity(int clientNum, int sequence);
int trap_BotGetServerCommand(int clientNum, char *message, int size);
// int		trap_BotGetConsoleMessage(int clientNum, char *message, int
// size);
void trap_BotUserCommand(int client, usercmd_t *ucmd);

void trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info);

int trap_AAS_Initialized(void);
void trap_AAS_PresenceTypeBoundingBox(int presencetype, vec3_t mins,
                                      vec3_t maxs);
float trap_AAS_Time(void);

// Ridah
void trap_AAS_SetCurrentWorld(int index);
// done.

int trap_AAS_PointAreaNum(vec3_t point);
int trap_AAS_TraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t *points,
                        int maxareas);
int trap_AAS_BBoxAreas(vec3_t absmins, vec3_t absmaxs, int *areas,
                       int maxareas);
void trap_AAS_AreaCenter(int areanum, vec3_t center);
qboolean trap_AAS_AreaWaypoint(int areanum, vec3_t center);

int trap_AAS_PointContents(vec3_t point);
int trap_AAS_NextBSPEntity(int ent);
int trap_AAS_ValueForBSPEpairKey(int ent, char *key, char *value, int size);
int trap_AAS_VectorForBSPEpairKey(int ent, char *key, vec3_t v);
int trap_AAS_FloatForBSPEpairKey(int ent, char *key, float *value);
int trap_AAS_IntForBSPEpairKey(int ent, char *key, int *value);

int trap_AAS_AreaReachability(int areanum);
int trap_AAS_AreaLadder(int areanum);

int trap_AAS_AreaTravelTimeToGoalArea(int areanum, vec3_t origin,
                                      int goalareanum, int travelflags);

int trap_AAS_Swimming(vec3_t origin);
int trap_AAS_PredictClientMovement(void /* aas_clientmove_s */ *move,
                                   int entnum, vec3_t origin, int presencetype,
                                   int onground, vec3_t velocity,
                                   vec3_t cmdmove, int cmdframes, int maxframes,
                                   float frametime, int stopevent,
                                   int stopareanum, int visualize);

// Ridah, route-tables
void trap_AAS_RT_ShowRoute(vec3_t srcpos, int srcnum, int destnum);
int trap_AAS_NearestHideArea(int srcnum, vec3_t origin, int areanum,
                             int enemynum, vec3_t enemyorigin, int enemyareanum,
                             int travelflags, float maxdist, vec3_t distpos);
int trap_AAS_ListAreasInRange(vec3_t srcpos, int srcarea, float range,
                              int travelflags, float **outareas, int maxareas);
int trap_AAS_AvoidDangerArea(vec3_t srcpos, int srcarea, vec3_t dangerpos,
                             int dangerarea, float range, int travelflags);
int trap_AAS_Retreat(
    // Locations of the danger spots (AAS area numbers)
    int *dangerSpots,
    // The number of danger spots
    int dangerSpotCount, vec3_t srcpos, int srcarea, vec3_t dangerpos,
    int dangerarea,
    // Min range from startpos
    float range,
    // Min range from danger
    float dangerRange, int travelflags);

void trap_AAS_SetAASBlockingEntity(vec3_t absmin, vec3_t absmax, int blocking);
void trap_AAS_RecordTeamDeathArea(vec3_t srcpos, int srcarea, int team,
                                  int teamCount, int travelflags);
// done.

void trap_EA_Say(int client, char *str);
void trap_EA_SayTeam(int client, char *str);
void trap_EA_UseItem(int client, char *it);
void trap_EA_DropItem(int client, char *it);
void trap_EA_UseInv(int client, char *inv);
void trap_EA_DropInv(int client, char *inv);
void trap_EA_Gesture(int client);
void trap_EA_Command(int client, char *command);

void trap_EA_SelectWeapon(int client, int weapon);
void trap_EA_Talk(int client);
void trap_EA_Attack(int client);
void trap_EA_Reload(int client);
void trap_EA_Activate(int client);
void trap_EA_Respawn(int client);
void trap_EA_Jump(int client);
void trap_EA_DelayedJump(int client);
void trap_EA_Crouch(int client);
void trap_EA_Walk(int client);
void trap_EA_MoveUp(int client);
void trap_EA_MoveDown(int client);
void trap_EA_MoveForward(int client);
void trap_EA_MoveBack(int client);
void trap_EA_MoveLeft(int client);
void trap_EA_MoveRight(int client);
void trap_EA_Move(int client, vec3_t dir, float speed);
void trap_EA_View(int client, vec3_t viewangles);
void trap_EA_Prone(int client);

void trap_EA_EndRegular(int client, float thinktime);
void trap_EA_GetInput(int client, float thinktime,
                      void /* struct bot_input_s */ *input);
void trap_EA_ResetInput(int client, void *init);

int trap_BotLoadCharacter(char *charfile, int skill);
void trap_BotFreeCharacter(int character);
float trap_Characteristic_Float(int character, int index);
float trap_Characteristic_BFloat(int character, int index, float min,
                                 float max);
int trap_Characteristic_Integer(int character, int index);
int trap_Characteristic_BInteger(int character, int index, int min, int max);
void trap_Characteristic_String(int character, int index, char *buf, int size);

int trap_BotAllocChatState(void);
void trap_BotFreeChatState(int handle);
void trap_BotQueueConsoleMessage(int chatstate, int type, char *message);
void trap_BotRemoveConsoleMessage(int chatstate, int handle);
int trap_BotNextConsoleMessage(int chatstate,
                               void /* struct bot_consolemessage_s */ *cm);
int trap_BotNumConsoleMessages(int chatstate);
void trap_BotInitialChat(int chatstate, char *type, int mcontext, char *var0,
                         char *var1, char *var2, char *var3, char *var4,
                         char *var5, char *var6, char *var7);
int trap_BotNumInitialChats(int chatstate, char *type);
int trap_BotReplyChat(int chatstate, char *message, int mcontext, int vcontext,
                      char *var0, char *var1, char *var2, char *var3,
                      char *var4, char *var5, char *var6, char *var7);
int trap_BotChatLength(int chatstate);
void trap_BotEnterChat(int chatstate, int client, int sendto);
void trap_BotGetChatMessage(int chatstate, char *buf, int size);
int trap_StringContains(char *str1, char *str2, int casesensitive);
int trap_BotFindMatch(char *str, void /* struct bot_match_s */ *match,
                      unsigned long int context);
void trap_BotMatchVariable(void /* struct bot_match_s */ *match, int variable,
                           char *buf, int size);
void trap_UnifyWhiteSpaces(char *string);
void trap_BotReplaceSynonyms(char *string, unsigned long int context);
int trap_BotLoadChatFile(int chatstate, char *chatfile, char *chatname);
void trap_BotSetChatGender(int chatstate, int gender);
void trap_BotSetChatName(int chatstate, char *name);
void trap_BotResetGoalState(int goalstate);
void trap_BotRemoveFromAvoidGoals(int goalstate, int number);
void trap_BotResetAvoidGoals(int goalstate);
void trap_BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal);
void trap_BotPopGoal(int goalstate);
void trap_BotEmptyGoalStack(int goalstate);
void trap_BotDumpAvoidGoals(int goalstate);
void trap_BotDumpGoalStack(int goalstate);
void trap_BotGoalName(int number, char *name, int size);
int trap_BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int trap_BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int trap_BotChooseLTGItem(int goalstate, vec3_t origin, int *inventory,
                          int travelflags);
int trap_BotChooseNBGItem(int goalstate, vec3_t origin, int *inventory,
                          int travelflags, void /* struct bot_goal_s */ *ltg,
                          float maxtime);
int trap_BotTouchingGoal(vec3_t origin, void /* struct bot_goal_s */ *goal);
int trap_BotItemGoalInVisButNotVisible(int viewer, vec3_t eye,
                                       vec3_t viewangles,
                                       void /* struct bot_goal_s */ *goal);
int trap_BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal);
int trap_BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal);
int trap_BotGetLevelItemGoal(int index, char *classname,
                             void /* struct bot_goal_s */ *goal);
float trap_BotAvoidGoalTime(int goalstate, int number);
void trap_BotInitLevelItems(void);
void trap_BotUpdateEntityItems(void);
int trap_BotLoadItemWeights(int goalstate, char *filename);
void trap_BotFreeItemWeights(int goalstate);
void trap_BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child);
void trap_BotSaveGoalFuzzyLogic(int goalstate, char *filename);
void trap_BotMutateGoalFuzzyLogic(int goalstate, float range);
int trap_BotAllocGoalState(int state);
void trap_BotFreeGoalState(int handle);

void trap_BotResetMoveState(int movestate);
void trap_BotMoveToGoal(void /* struct bot_moveresult_s */ *result,
                        int movestate, void /* struct bot_goal_s */ *goal,
                        int travelflags);
int trap_BotMoveInDirection(int movestate, vec3_t dir, float speed, int type);
void trap_BotResetAvoidReach(int movestate);
void trap_BotResetLastAvoidReach(int movestate);
int trap_BotReachabilityArea(vec3_t origin, int testground);
int trap_BotMovementViewTarget(int movestate,
                               void /* struct bot_goal_s */ *goal,
                               int travelflags, float lookahead, vec3_t target);
int trap_BotPredictVisiblePosition(vec3_t origin, int areanum,
                                   void /* struct bot_goal_s */ *goal,
                                   int travelflags, vec3_t target);
int trap_BotAllocMoveState(void);
void trap_BotFreeMoveState(int handle);
void trap_BotInitMoveState(int handle,
                           void /* struct bot_initmove_s */ *initmove);
// Ridah
void trap_BotInitAvoidReach(int handle);
// done.

int trap_BotChooseBestFightWeapon(int weaponstate, int *inventory);
void trap_BotGetWeaponInfo(int weaponstate, int weapon,
                           void /* struct weaponinfo_s */ *weaponinfo);
int trap_BotLoadWeaponWeights(int weaponstate, char *filename);
int trap_BotAllocWeaponState(void);
void trap_BotFreeWeaponState(int weaponstate);
void trap_BotResetWeaponState(int weaponstate);

int trap_GeneticParentsAndChildSelection(int numranks, float *ranks,
                                         int *parent1, int *parent2,
                                         int *child);

void trap_SnapVector(float *v);

void trap_SendMessage(int clientNum, char *buf, int buflen);
messageStatus_t trap_MessageStatus(int clientNum);

void G_ExplodeMissile(gentity_t *ent);

void Svcmd_ResetMatch_f(qboolean fDoRestart);

void trap_PbStat(int clientNum, const char *category, const char *values);

// g_antilag.c
void G_StoreClientPosition(gentity_t *ent);
void G_AdjustClientPositions(gentity_t *ent, int time, qboolean forward);
void G_ResetMarkers(gentity_t *ent);
void G_HistoricalTrace(gentity_t *ent, trace_t *results, const vec3_t start,
                       const vec3_t mins, const vec3_t maxs, const vec3_t end,
                       int passEntityNum, int contentmask);
void G_HistoricalTraceBegin(gentity_t *ent);
void G_HistoricalTraceEnd(gentity_t *ent);
void G_Trace(gentity_t *ent, trace_t *results, const vec3_t start,
             const vec3_t mins, const vec3_t maxs, const vec3_t end,
             int passEntityNum, int contentmask);

#define BODY_VALUE(ENT) ENT->watertype
#define BODY_TEAM(ENT) ENT->s.modelindex
#define BODY_CLASS(ENT) ENT->s.modelindex2
#define BODY_CHARACTER(ENT) ENT->s.onFireStart

// g_buddy_list.c

#define MAX_FIRE_TEAMS 8

typedef struct {
  char name[32];
  char clientbits[8];
  char requests[8];
  int leader;
  qboolean open;
  qboolean valid;
} fireteam_t;

void Cmd_FireTeam_MP_f(gentity_t *ent);

/*
void G_SetWayPoint( gentity_t* ent, wayPointType_t wayPointType, vec3_t loc );
void G_RemoveWayPoint( gclient_t *client );
*/

void G_RemoveFromAllIgnoreLists(int clientNum);

// g_teammapdata.c

typedef struct mapEntityData_s {
  vec3_t org;
  int yaw;
  int data;
  char type;
  int startTime;
  int singleClient;

  int status;
  int entNum;
  struct mapEntityData_s *next, *prev;
} mapEntityData_t;

typedef struct mapEntityData_Team_s {
  mapEntityData_t mapEntityData_Team[MAX_GENTITIES];
  mapEntityData_t *freeMapEntityData;  // single linked list
  mapEntityData_t activeMapEntityData; // double linked list
} mapEntityData_Team_t;

extern mapEntityData_Team_t mapEntityData[2];

void G_InitMapEntityData(mapEntityData_Team_t *teamList);
mapEntityData_t *G_FreeMapEntityData(mapEntityData_Team_t *teamList,
                                     mapEntityData_t *mEnt);
mapEntityData_t *G_AllocMapEntityData(mapEntityData_Team_t *teamList);
mapEntityData_t *G_FindMapEntityData(mapEntityData_Team_t *teamList,
                                     int entNum);
mapEntityData_t *G_FindMapEntityDataSingleClient(mapEntityData_Team_t *teamList,
                                                 mapEntityData_t *start,
                                                 int entNum, int clientNum);

void G_ResetTeamMapData();
void G_UpdateTeamMapData();

void G_SetupFrustum(gentity_t *ent);
void G_SetupFrustum_ForBinoculars(gentity_t *ent);
qboolean G_VisibleFromBinoculars(gentity_t *viewer, gentity_t *ent,
                                 vec3_t origin);

void G_LogTeamKill(gentity_t *ent, weapon_t weap);
void G_LogDeath(gentity_t *ent, weapon_t weap);
void G_LogKill(gentity_t *ent, weapon_t weap);
void G_LogRegionHit(gentity_t *ent, hitRegion_t hr);
// void G_SetPlayerRank(	gentity_t* ent );
// void G_AddExperience(	gentity_t* ent, float exp );

// Skills
void G_SetPlayerScore(gclient_t *client);
void G_SetPlayerSkill(gclient_t *client, skillType_t skill);
void G_AddSkillPoints(gentity_t *ent, skillType_t skill, float points);
void G_LoseSkillPoints(gentity_t *ent, skillType_t skill, float points);
void G_AddKillSkillPoints(gentity_t *attacker, meansOfDeath_t mod,
                          hitRegion_t hr, qboolean splash);
void G_AddKillSkillPointsForDestruction(
    gentity_t *attacker, meansOfDeath_t mod,
    g_constructible_stats_t *constructibleStats);
void G_LoseKillSkillPoints(gentity_t *tker, meansOfDeath_t mod, hitRegion_t hr,
                           qboolean splash);

void G_DebugOpenSkillLog(void);
void G_DebugCloseSkillLog(void);
void G_DebugAddSkillLevel(gentity_t *ent, skillType_t skill);
void G_DebugAddSkillPoints(gentity_t *ent, skillType_t skill, float points,
                           const char *reason);

typedef enum {
  SM_NEED_MEDIC,
  SM_NEED_ENGINEER,
  SM_NEED_LT,
  SM_NEED_COVERTOPS,
  SM_LOST_MEN,
  SM_OBJ_CAPTURED,
  SM_OBJ_LOST,
  SM_OBJ_DESTROYED,
  SM_CON_COMPLETED,
  SM_CON_FAILED,
  SM_CON_DESTROYED,
  SM_NUM_SYS_MSGS,
} sysMsg_t;

void G_SendMapEntityInfo(gentity_t *e);
void G_SendSystemMessage(sysMsg_t message, int team);
int G_GetSysMessageNumber(const char *sysMsg);
int G_CountTeamLandmines(team_t team);
qboolean G_SweepForLandmines(vec3_t origin, float radius, int team);

void G_AddClientToFireteam(int entityNum, int leaderNum);
void G_InviteToFireTeam(int entityNum, int otherEntityNum);
void G_UpdateFireteamConfigString(fireteamData_t *ft);
void G_RemoveClientFromFireteams(int entityNum, qboolean update,
                                 qboolean print);

void G_PrintClientSpammyCenterPrint(int entityNum, const char *text);

void aagun_fire(gentity_t *other);

// TAT 11/13/2002
//		Server only entities

struct g_serverEntity_s {
  qboolean inuse;

  char *classname; // set in QuakeEd
  int spawnflags;  // set in QuakeEd

  // our parent entity
  g_serverEntity_t *parent;

  // pointer to the next server entity
  g_serverEntity_t *nextServerEntity;
  g_serverEntity_t *target_ent;
  g_serverEntity_t *chain;

  char *name;
  char *target;

  vec3_t origin;     // Let's try keeping this simple, and just having an
                     // origin
  vec3_t angles;     // facing angle (for a seek cover spot)
  int number;        // identifier for this entity
  int team;          // which team?  seek cover spots need this
  int areaNum;       // This thing doesn't move, so we should only need to
                     // calc the areanum once
  int botIgnoreTime; // So that multiple bots don't use the same seek
                     // cover spot

  void (*setup)(g_serverEntity_t *self); // Setup function called once after all
                                         // server objects are created
};

// clear out all the sp entities
void InitServerEntities(void);
// get the server entity with the passed in number
g_serverEntity_t *GetServerEntity(int num);
// Give a gentity_t, create a sp entity, copy all pertinent data, and return it
g_serverEntity_t *CreateServerEntity(gentity_t *ent);
// These server entities don't get to update every frame, but some of them have
// to set themselves up
//		after they've all been created
//		So we want to give each entity the chance to set itself up after
// it has been created
void InitialServerEntitySetup();
// Like G_Find, but for server entities
g_serverEntity_t *FindServerEntity(g_serverEntity_t *from, int fieldofs,
                                   char *match);

#define SE_FOFS(x) ((int)&(((g_serverEntity_t *)0)->x))

// Match settings
#define PAUSE_NONE 0x00      // Match is NOT paused.
#define PAUSE_UNPAUSING 0x01 // Pause is about to expire

// HRESULTS
#define G_OK 0
#define G_INVALID -1
#define G_NOTFOUND -2

#define AP(x) trap_SendServerCommand(-1, x)               // Print to all
#define CP(x) trap_SendServerCommand(ent - g_entities, x) // Print to an ent
#define CPx(x, y) trap_SendServerCommand(x, y)            // Print to id = x

#define PAUSE_NONE 0x00      // Match is NOT paused.
#define PAUSE_UNPAUSING 0x01 // Pause is about to expire

#define ZSF_COMP 0x01 // Have comp settings loaded for current gametype?

#define HELP_COLUMNS 4

#define CMD_DEBOUNCE 5000 // 5s between cmds

#define EOM_WEAPONSTATS 0x01 // Dump of player weapon stats at end of match.
#define EOM_MATCHINFO 0x02   // Dump of match stats at end of match.

#define AA_STATSALL 0x01  // Client AutoAction: Dump ALL player stats
#define AA_STATSTEAM 0x02 // Client AutoAction: Dump TEAM player stats

// "Delayed Print" ent enumerations
typedef enum {
  DP_PAUSEINFO,   // Print current pause info
  DP_UNPAUSING,   // Print unpause countdown + unpause
  DP_CONNECTINFO, // Display OSP info on connect
  DP_MVSPAWN      // Set up MV views for clients who need them
} enum_t_dp;

// Remember: Axis = RED, Allies = BLUE ... right?!

// Team extras
typedef struct {
  qboolean spec_lock;
  qboolean team_lock;
  char team_name[24];
  int team_score;
  int timeouts;
} team_info;

///////////////////////
// g_main.c
//
void G_UpdateCvars(void);
void G_wipeCvars(void);

///////////////////////
// g_cmds_ext.c
//
qboolean G_commandHelp(gentity_t *ent, const char *pszCommand,
                       unsigned int dwCommand);
qboolean G_cmdDebounce(gentity_t *ent, const char *pszCommand);
void G_commands_cmd(gentity_t *ent, unsigned int dwCommand, qboolean fValue);
void G_players_cmd(gentity_t *ent, unsigned int dwCommand, qboolean fDump);
void G_ready_cmd(gentity_t *ent, unsigned int dwCommand, qboolean fDump);
void G_say_teamnl_cmd(gentity_t *ent, unsigned int dwCommand, qboolean fValue);
void G_scores_cmd(gentity_t *ent, unsigned int dwCommand, qboolean fValue);
void G_specinvite_cmd(gentity_t *ent, unsigned int dwCommand, qboolean fLock);
void G_statsall_cmd(gentity_t *ent, unsigned int dwCommand, qboolean fDump);
void G_teamready_cmd(gentity_t *ent, unsigned int dwCommand, qboolean fDump);
void G_weaponRankings_cmd(gentity_t *ent, unsigned int dwCommand,
                          qboolean state);
void G_weaponStats_cmd(gentity_t *ent, unsigned int dwCommand, qboolean fDump);
void G_weaponStatsLeaders_cmd(gentity_t *ent, qboolean doTop,
                              qboolean doWindow);
void G_VoiceTo(gentity_t *ent, gentity_t *other, int mode, vsayCmd_t *vsay,
               qboolean voiceonly);

///////////////////////
// g_match.c
//
void G_addStats(gentity_t *targ, gentity_t *attacker, int dmg_ref, int mod);
void G_addStatsHeadShot(gentity_t *attacker, int mod);
int G_checkServerToggle(vmCvar_t *cv);
char *G_createStats(gentity_t *refEnt);
void G_deleteStats(int nClient);
void G_globalSound(const char *sound);
void G_initMatch(void);
void G_loadMatchGame(void);
void G_matchInfoDump(unsigned int dwDumpType);
void G_printMatchInfo(gentity_t *ent);
void G_parseStats(char *pszStatsInfo);
void G_printFull(const char *str, gentity_t *ent);
void G_spawnPrintf(int print_type, int print_time, gentity_t *owner);
void G_statsPrint(gentity_t *ent, int nType);
unsigned int G_weapStatIndex_MOD(int iWeaponMOD);

///////////////////////
// g_team.c
//
extern const char *aTeams[TEAM_NUM_TEAMS];
extern team_info teamInfo[TEAM_NUM_TEAMS];

qboolean G_allowFollow(gentity_t *ent, int nTeam);
int G_blockoutTeam(gentity_t *ent, int nTeam);
qboolean G_checkReady(void);
qboolean G_readyMatchState(void);
void G_removeSpecInvite(int team);
void G_shuffleTeams(void);
void G_swapTeams(void);
qboolean G_teamJoinCheck(int team_num, gentity_t *ent);
int G_teamID(gentity_t *ent);
void G_teamReset(int team_num, qboolean fClearSpecLock);
void G_verifyMatchState(int team_id);

///////////////////////
// g_vote.c
//
void G_cpmPrintf(gentity_t *ent, const char *fmt, ...);
int G_voteCmdCheck(gentity_t *ent, char *arg, char *arg2);
void G_voteFlags(void);
void G_voteHelp(gentity_t *ent, qboolean fShowVote);
void G_playersMessage(gentity_t *ent);
// Actual voting commands
int G_Map_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2,
            const bool &cheats);
int G_MapRestart_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg,
                   char *arg2);
int G_RandomMap_v(gentity_t *ent, unsigned dwVoteIndex, char *arg, char *arg2);
namespace ETJump {
int G_RockTheVote_v(gentity_t *ent, unsigned dwVoteIndex, char *arg,
                    char *arg2);
int G_AutoRtv_v(gentity_t *ent, unsigned dwVoteIndex, char *arg, char *arg2);
} // namespace ETJump

void G_LinkDebris(void);
void G_LinkDamageParents(void);
int EntsThatRadiusCanDamage(vec3_t origin, float radius, int *damagedList);

qboolean G_LandmineTriggered(gentity_t *ent);
qboolean G_LandmineArmed(gentity_t *ent);
qboolean G_LandmineUnarmed(gentity_t *ent);
team_t G_LandmineTeam(gentity_t *ent);
qboolean G_LandmineSpotted(gentity_t *ent);
gentity_t *G_FindSmokeBomb(gentity_t *start);
gentity_t *G_FindLandmine(gentity_t *start);
gentity_t *G_FindDynamite(gentity_t *start);
gentity_t *G_FindSatchels(gentity_t *start);
void G_SetTargetName(gentity_t *ent, char *targetname);
void G_KillEnts(const char *target, gentity_t *ignore, gentity_t *killer,
                meansOfDeath_t mod);
void trap_EngineerTrace(trace_t *results, const vec3_t start, const vec3_t mins,
                        const vec3_t maxs, const vec3_t end, int passEntityNum,
                        int contentmask);

qboolean G_ConstructionIsPartlyBuilt(gentity_t *ent);

int G_CountTeamMedics(team_t team, qboolean alivecheck);
qboolean G_TankIsOccupied(gentity_t *ent);
qboolean G_TankIsMountable(gentity_t *ent, gentity_t *other);

qboolean G_ConstructionBegun(gentity_t *ent);
qboolean G_ConstructionIsFullyBuilt(gentity_t *ent);
qboolean G_ConstructionIsPartlyBuilt(gentity_t *ent);
gentity_t *G_ConstructionForTeam(gentity_t *toi, team_t team);
gentity_t *G_IsConstructible(team_t team, gentity_t *toi);
qboolean G_EmplacedGunIsRepairable(gentity_t *ent, gentity_t *other);
qboolean G_EmplacedGunIsMountable(gentity_t *ent, gentity_t *other);
void G_CheckForCursorHints(gentity_t *ent);
void G_CalcClientAccuracies(void);
void G_BuildEndgameStats(void);
int G_TeamCount(gentity_t *ent, weapon_t weap);

qboolean G_IsFireteamLeader(int entityNum, fireteamData_t **teamNum);
fireteamData_t *G_FindFreePublicFireteam(team_t team);
void G_RegisterFireteam(/*const char* name,*/ int entityNum);

void weapon_callAirStrike(gentity_t *ent);
void weapon_checkAirStrikeThink2(gentity_t *ent);
void weapon_checkAirStrikeThink1(gentity_t *ent);
void weapon_callSecondPlane(gentity_t *ent);
qboolean weapon_checkAirStrike(gentity_t *ent);

void G_MakeReady(gentity_t *ent);
void G_MakeUnready(gentity_t *ent);

void SetPlayerSpawn(gentity_t *ent, int spawn, qboolean update);
void G_UpdateSpawnCounts(void);

void G_SetConfigStringValue(int num, const char *key, const char *value);
void G_GlobalClientEvent(int event, int param, int client);

void G_InitTempTraceIgnoreEnts(void);
void G_ResetTempTraceIgnoreEnts(void);
void G_TempTraceIgnoreEntity(gentity_t *ent);
void G_TempTraceIgnorePlayersAndBodies(void);

qboolean G_CanPickupWeapon(weapon_t weapon, gentity_t *ent);

qboolean G_LandmineSnapshotCallback(int entityNum, int clientNum);

void G_AddIpMute(char *ip);
void G_RemoveIPMute(char *ip);
qboolean G_isIPMuted(const char *ip);
void G_ClearIPMutes();
qboolean G_commandCheck(gentity_t *ent, char *cmd, qboolean fDoAnytime);
// g_admin.c
const char *G_SHA1(const char *string);
char *Q_SayConcatArgs(int start);
void DecolorString(char *in, char *out);
int Q_SayArgc();
qboolean Q_SayArgv(int n, char *buffer, int bufferLength);

// Feen: PGM

// g_portalgun.c
// gentity_t *fire_portalgun(gentity_t *self, vec3_t start, vec3_t dir);

void Portal_Think(gentity_t *self);
void Portal_Touch(gentity_t *self, gentity_t *other, trace_t *trace);

// g_weapon.c
void Weapon_Portal_Fire(
    gentity_t *ent, int portalNum); // TODO add switch for different portals....

// Feen: END PGM

void Use_target_remove_powerups(gentity_t *ent, gentity_t *other,
                                gentity_t *activator);

// g_utilities.cpp
// C versions of printing functions
void C_BPAll(const char *msg);
void C_BPTo(gentity_t *target, const char *msg);
void C_CPAll(const char *msg);
void C_CPMAll(const char *msg);
void C_CPMTo(gentity_t *target, const char *msg);
void C_CPTo(gentity_t *target, const char *msg);
void C_ChatPrintAll(const char *msg);
void C_ChatPrintTo(gentity_t *target, const char *msg);
void C_ConsolePrintAll(const char *msg);
void C_ConsolePrintTo(gentity_t *target, const char *msg);
const char *EscapeString(const char *in);
const char *interpolateNametags(const char *text);
const char *findAndReplaceNametags(const char *text, const char *name);

// Returns clientnum from ent
int ClientNum(gentity_t *ent);
// Returns clientnum from client
int ClientNum(gclient_t *client);

// mainext.cpp
void OnClientConnect(int clientNum, qboolean firstTime, qboolean isBot);
void OnClientBegin(gentity_t *ent);
void OnClientDisconnect(gentity_t *ent);
qboolean OnConnectedClientCommand(gentity_t *ent);
qboolean OnClientCommand(gentity_t *ent);
qboolean OnConsoleCommand();
void OnGameInit();
void OnGameShutdown();
qboolean G_MapExists(const char *map);
const char *GetRandomMap();
const char *GetRandomMapByType(const char *customType);
qboolean AdminCommandCheck(gentity_t *ent);
// void StartRace(gentity_t *ent);
const char *CustomMapTypeExists(const char *mapType);
void ClientNameChanged(gentity_t *ent);
void G_increaseCallvoteCount(const char *mapName);
void G_increasePassedCount(const char *mapName);
void LogServerState();

namespace ETJump {
// finds an entity with a scriptname, if not found,
// we spawn in 'etjump_game_manager' so we always have
// access to mapscripting
bool checkEntsForScriptname();
void spawnGameManager();
} // namespace ETJump

qboolean G_IsOnFireteam(int entityNum, fireteamData_t **teamNum);

namespace ETJump {
enum class TimerunSpawnflags {
  // ResetEnd is unused in code, but it's used by mappers to specify a run
  // which does not end either on team change or death, since those flags
  // are also checked against having no flags at all, so that "default"
  // behavior is to reset a run on team change or death

  None = 0,
  ResetTeamChange = 1,
  ResetDeath = 2,
  ResetEnd = 4,
  ResetNoPmove = 8,
  NoBackups = 16,
  NoExplosivesPickup = 32,
  NoPortalgunPickup = 64,
  NoSave = 128,
};

bool checkCheatCvars(gclient_s *client, int flags);
} // namespace ETJump

void TimerunConnectNotify(gentity_t *ent);

void InterruptRun(gentity_t *ent);

void RunFrame(int levelTime);
const char *G_MatchOneMap(const char *arg);
std::vector<std::string> G_MatchAllMaps(const char *arg);

///////////////////////////////////////////////////////////////////////////////
// ETJump global systems
///////////////////////////////////////////////////////////////////////////////

class Session;
class Database;
namespace ETJump {
class DeathrunSystem;
extern std::shared_ptr<ETJump::DeathrunSystem> deathrunSystem;
class SaveSystem;
extern std::shared_ptr<ETJump::SaveSystem> saveSystem;
extern std::shared_ptr<Session> session;
extern std::shared_ptr<Database> database;
class ProgressionTrackers;
extern std::shared_ptr<ProgressionTrackers> progressionTrackers;

struct GameLogicException : public std::exception {
private:
  std::string message;

public:
  GameLogicException(const std::string &message) : message(message) {}
  ~GameLogicException() {}

  const char *what() const throw() { return message.c_str(); }
};
} // namespace ETJump

///////////////////////////////////////////////////////////////////////////////

const size_t BYTES_PER_PACKET = 998;

#endif // G_LOCAL_H
