#pragma once

typedef struct entityState_s entityState_t;
typedef enum moverState_t;
typedef struct entityShared_t;
enum qboolean;
typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

struct gentity_s
{
	entityState_t s;                // communicated by server to clients
	entityShared_t r;               // shared by both the server system and game

									// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
									// EXPECTS THE FIELDS IN THAT ORDER!
									//================================

	struct gclient_s *client;               // NULL if not a client

	qboolean inuse;

	vec3_t instantVelocity;         // ydnar: per entity instantaneous velocity, set per frame

	const char *classname;              // set in QuakeEd
	int spawnflags;                 // set in QuakeEd

	qboolean neverFree;             // if true, FreeEntity will only unlink
									// bodyque uses this

	int flags;                      // FL_* variables

	char *model;
	char *model2;
	int freetime;                   // level.time when the object was freed

	int eventTime;                  // events will be cleared EVENT_VALID_MSEC after set
	qboolean freeAfterEvent;
	qboolean unlinkAfterEvent;

	qboolean physicsObject;         // if true, it can be pushed by movers and fall off edges
									// all game items are physicsObjects,
	float physicsBounce;            // 1.0 = continuous bounce, 0.0 = no bounce
	int clipmask;                   // brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance

	int realClipmask;               // Arnout: use these to backup the contents value when we go to state under construction
	int realContents;
	qboolean realNonSolidBModel;    // For script_movers with spawnflags 2 set

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

	int timestamp;              // body queue sinking, etc

	float angle;                // set in editor, -1 = up, -2 = down
	char *target;

	char *targetname;
	int targetnamehash;         // Gordon: adding a hash for this for faster lookups

	char *team;
	gentity_t *target_ent;

	float speed;
	float closespeed;           // for movers that close at a different speed than they open
	vec3_t movedir;

	int gDuration;
	int gDurationBack;
	vec3_t gDelta;
	vec3_t gDeltaBack;

	int nextthink;
	void(*free)(gentity_t *self);
	void(*think)(gentity_t *self);
	void(*reached)(gentity_t *self);           // movers call this when hitting endpoint
	void(*blocked)(gentity_t *self, gentity_t *other);
	void(*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
	void(*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
	void(*pain)(gentity_t *self, gentity_t *attacker, int damage, vec3_t point);
	void(*die)(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

	int pain_debounce_time;
	int fly_sound_debounce_time;            // wind tunnel

	int health;

	qboolean takedamage;

	int damage;
	int splashDamage;           // quad will increase this without increasing radius
	int splashRadius;
	int methodOfDeath;
	int splashMethodOfDeath;

	int count;

	gentity_t *chain;
	gentity_t *enemy;
	gentity_t *activator;
	gentity_t *teamchain;       // next entity in team
	gentity_t *teammaster;      // master of the team

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

	gitem_t *item;              // for bonus items

								// Ridah, AI fields
	char *aiName;
	int aiTeam;
	void(*AIScript_AlertEntity)(gentity_t *ent);
	// done.

	char *aiSkin;

	vec3_t dl_color;
	char *dl_stylestring;
	char *dl_shader;
	int dl_atten;


	int key;                    // used by:  target_speaker->nopvs,

	qboolean active;

	// Rafael - mg42
	float harc;
	float varc;

	int props_frame_state;

	// Ridah
	int missionLevel;               // mission we are currently trying to complete
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

	int grenadeExplodeTime;         // we've caught a grenade, which was due to explode at this time
	int grenadeFired;               // the grenade entity we last fired

	char *track;

	// entity scripting system
	char *scriptName;

	int numScriptEvents;
	g_script_event_t *scriptEvents;     // contains a list of actions to perform for each event type
	g_script_status_t scriptStatus;     // current status of scripting
										// the accumulation buffer
	int scriptAccumBuffer[G_MAX_SCRIPT_ACCUM_BUFFERS];

	qboolean AASblocking;
	vec3_t AASblocking_mins, AASblocking_maxs;
	float accuracy;

	char tagName[MAX_QPATH];            // name of the tag we are attached to
	gentity_t *tagParent;
	gentity_t *tankLink;

	int lastHintCheckTime;                  // DHM - Nerve
	int voiceChatSquelch;                   // DHM - Nerve
	int voiceChatPreviousTime;              // DHM - Nerve
	int lastBurnedFrameNumber;              // JPW - Nerve   : to fix FT instant-kill exploit

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
	int spawnCount;                         // incremented each time this entity is spawned
	int botIgnoreTime, awaitingHelpTime;
	int botIgnoreHealthTime, botIgnoreAmmoTime;
	int botAltGoalTime;
	vec3_t botGetAreaPos;
	int botGetAreaNum;
	int aiInactive;             // bots should ignore this goal
	int goalPriority[2];

	int tagNumber;              // Gordon: "handle" to a tag header

	int linkTagTime;

	splinePath_t *backspline;
	vec3_t backorigin;
	float backdelta;
	qboolean back;
	qboolean moving;

	int botLastAttackedTime;
	int botLastAttackedEnt;

	int botAreaNum;                 // last checked area num
	vec3_t botAreaPos;

	// TAT 10/13/2002 - for seek cover sequence - we need a pointer to a server entity
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

	//bani
	int etpro_misc_1;

	//etjump
	int ident;
	int reqident;
	int inc; // increment val for target_increase_ident
			 // How long in milliseconds it takes for the progress
			 // identifier to decay back to decayValue
	int decayTime;
	// Value that ident decays to
	int decayValue;

	//Feen - PGM
	gentity_t *portalBlue;
	gentity_t *portalRed;
	// Zero - other portal so we know where to go
	// when someone goes in a team portal
	gentity_t *linkedPortal;

	int lastPortalTime; //Last time we teleported using portal
	int portalTeam;

	int runIndex;

	float velocityLowerLimit;
	float velocityUpperLimit;

	TokenInformation *tokenInformation;
};