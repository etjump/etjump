#pragma once

#include "g_common.h"

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct
{
	ClientConnected connected;
	usercmd_t cmd;                  // we would lose angles if not persistant
	usercmd_t oldcmd;               // previous command processed by pmove()
	qboolean localClient;           // true if "ip" info key is "localhost"
	qboolean initialSpawn;          // the first spawn should be at a cool location
	qboolean predictItemPickup;     // based on cg_predictItems userinfo
	qboolean pmoveFixed;            //
	qboolean nofatigue;
	qboolean cgaz;
	qboolean loadViewAngles;

	unsigned int maxFPS;
	char netname[MAX_NETNAME];

	int autoActivate;               // based on cg_autoactivate userinfo		(uses the PICKUP_ values above)

	int maxHealth;                  // for handicapping
	int enterTime;                  // level.time the client entered the game
	int connectTime;                // DHM - Nerve :: level.time the client first connected to the server
	playerTeamState_t teamState;    // status in teamplay games
	int voteCount;                  // to prevent people from constantly calling votes
	int teamVoteCount;              // to prevent people from constantly calling votes

	int lastReinforceTime;              // DHM - Nerve :: last reinforcement

	qboolean teamInfo;              // send team overlay updates?

	qboolean bAutoReloadAux;            // TTimo - auxiliary storage for pmoveExt_t::bAutoReload, to achieve persistance

	playerStats_t playerStats;

	//gentity_t	*wayPoint;

	int lastBattleSenseBonusTime;
	int lastHQMineReportTime;
	int lastCCPulseTime;

	int lastSpawnTime;

	char botScriptName[MAX_NETNAME];

	// OSP
	unsigned int autoaction;            // End-of-match auto-requests
	unsigned int clientFlags;           // Client settings that need server involvement
	unsigned int clientMaxPackets;      // Client com_maxpacket settings
	unsigned int clientTimeNudge;       // Client cl_timenudge settings
	int cmd_debounce;                   // Dampening of command spam
	unsigned int invite;                // Invitation to a team to join
	int panzerDropTime;                 // Time which a player dropping panzer still "has it" if limiting panzer counts
	int panzerSelectTime;               // *when* a client selected a panzer as spawn weapon
	qboolean ready;                     // Ready state to begin play
										// OSP

	bg_character_t *character;
	int characterIndex;

	int hideMe;
	float noclipScale;

	qboolean enableTimeruns;

	int noclipCount;

	raceStruct_t race;

#define MAX_TOKENS_PER_DIFFICULTY 6
	qboolean collectedEasyTokens[MAX_TOKENS_PER_DIFFICULTY];
	qboolean collectedMediumTokens[MAX_TOKENS_PER_DIFFICULTY];
	qboolean collectedHardTokens[MAX_TOKENS_PER_DIFFICULTY];
	int tokenCollectionStartTime;

	int previousSetHealthTime;
} clientPersistant_t;