// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

#include <vector>
#include <ctime>
#include <chrono>

#include "cg_local.h"
#include "etj_init.h"
#include "etj_client_commands_handler.h"
#include "etj_client_rtv_handler.h"

#include "../game/etj_string_utilities.h"
#include "../game/etj_syscall_ext_shared.h"

#define SCOREPARSE_COUNT 9

void CG_LimboMenu_f();

/*
=================
CG_ParseScores

=================
*/
// Gordon: NOTE: team doesnt actually signify team, think i was on drugs that
// day.....
static void CG_ParseScore(team_t team) {
  int i, j, powerups;
  int numScores;
  int offset;

  if (team == TEAM_AXIS) {
    cg.numScores = 0;

    cg.teamScores[0] = Q_atoi(CG_Argv(1));
    cg.teamScores[1] = Q_atoi(CG_Argv(2));

    offset = 4;
  } else {
    offset = 2;
  }

  numScores = Q_atoi(CG_Argv(offset - 1));

  for (j = 0; j < numScores; j++) {
    i = cg.numScores;

    cg.scores[i].client = Q_atoi(CG_Argv(offset + 0 + (j * 7)));
    cg.scores[i].score = Q_atoi(CG_Argv(offset + 1 + (j * 7)));
    cg.scores[i].ping = Q_atoi(CG_Argv(offset + 2 + (j * 7)));
    cg.scores[i].time = Q_atoi(CG_Argv(offset + 3 + (j * 7)));
    powerups = Q_atoi(CG_Argv(offset + 4 + (j * 7)));
    cg.scores[i].playerClass = Q_atoi(CG_Argv(offset + 5 + (j * 7)));
    cg.scores[i].followedClient = Q_atoi(CG_Argv(offset + 6 + (j * 7)));

    if (cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS) {
      cg.scores[i].client = 0;
    }

    cgs.clientinfo[cg.scores[i].client].score = cg.scores[i].score;
    cgs.clientinfo[cg.scores[i].client].powerups = powerups;

    cg.scores[i].team = cgs.clientinfo[cg.scores[i].client].team;

    cg.numScores++;
  }
}

/*
=================
CG_ParseTeamInfo

=================
*/
#define NUMARGS 5
static void CG_ParseTeamInfo(void) {
  int i;
  int client;
  int numSortedTeamPlayers;

  numSortedTeamPlayers = Q_atoi(CG_Argv(1));

  for (i = 0; i < numSortedTeamPlayers; i++) {
    client = Q_atoi(CG_Argv(i * NUMARGS + 2));

    cgs.clientinfo[client].location[0] = Q_atoi(CG_Argv(i * NUMARGS + 3));
    cgs.clientinfo[client].location[1] = Q_atoi(CG_Argv(i * NUMARGS + 4));
    cgs.clientinfo[client].health = Q_atoi(CG_Argv(i * NUMARGS + 5));
    cgs.clientinfo[client].powerups = Q_atoi(CG_Argv(i * NUMARGS + 6));
  }
}

/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo(void) {
  const char *info;
  const char *mapname;

  info = CG_ConfigString(CS_SERVERINFO);
  cg_gameType.integer = cgs.gametype =
      (gametype_t)Q_atoi(Info_ValueForKey(info, "g_gametype"));
  cg_antilag.integer = cgs.antilag =
      Q_atoi(Info_ValueForKey(info, "g_antilag"));
  if (!cgs.localServer) {
    trap_Cvar_Set("g_gametype", va("%i", cgs.gametype));
    trap_Cvar_Set("g_antilag", va("%i", cgs.antilag));
    trap_Cvar_Update(&cg_antilag);
    trap_Cvar_Update(&cg_gameType);
  }
  cgs.timelimit = Q_atof(Info_ValueForKey(info, "timelimit"));
  cgs.maxclients = Q_atoi(Info_ValueForKey(info, "sv_maxclients"));
  mapname = Info_ValueForKey(info, "mapname");
  Q_strncpyz(cgs.rawmapname, mapname, sizeof(cgs.rawmapname));
  Com_sprintf(cgs.mapname, sizeof(cgs.mapname), "maps/%s.bsp", mapname);

  // prolly should parse all CS_SERVERINFO keys automagically, but I
  // don't want to break anything that might be improperly set for wolf
  // SP, so I'm just parsing MP relevant stuff here
  trap_Cvar_Set("g_redlimbotime", Info_ValueForKey(info, "g_redlimbotime"));
  cg_redlimbotime.integer = Q_atoi(Info_ValueForKey(info, "g_redlimbotime"));
  trap_Cvar_Set("g_bluelimbotime", Info_ValueForKey(info, "g_bluelimbotime"));
  cg_bluelimbotime.integer = Q_atoi(Info_ValueForKey(info, "g_bluelimbotime"));

  cg_ghostPlayers.integer = Q_atoi(Info_ValueForKey(info, "g_ghostPlayers"));

  etj_spectatorVote.integer = Q_atoi(Info_ValueForKey(info, "g_spectatorVote"));

  cgs.minclients = Q_atoi(Info_ValueForKey(
      info, "g_minGameClients")); // NERVE - SMF -- OSP:
                                  // overloaded for ready counts

  // TTimo - make this available for ingame_callvote
  trap_Cvar_Set("cg_ui_voteFlags", Info_ValueForKey(info, "voteFlags"));
}

void CG_ParseSysteminfo(void) {
  const char *info = CG_ConfigString(CS_SYSTEMINFO);

  cgs.shared = Q_atoi(Info_ValueForKey(info, "shared"));

  cgs.pmove_msec = Q_atoi(Info_ValueForKey(info, "pmove_msec"));
  if (cgs.pmove_msec < 8) {
    cgs.pmove_msec = 8;
  } else if (cgs.pmove_msec > 33) {
    cgs.pmove_msec = 33;
  }

  cgs.cheats = Q_atoi(Info_ValueForKey(info, "sv_cheats"));
  cgs.sv_fps = Q_atoi(Info_ValueForKey(info, "sv_fps"));

#ifdef ALLOW_GSYNC
  cgs.synchronousClients =
      (Q_atoi(Info_ValueForKey(info, "g_synchronousClients"))) ? qtrue : qfalse;
#endif
}

/*
==================
CG_ParseWarmup
==================
*/
static void CG_ParseWarmup(void) {
  const char *info;
  int warmup;

  info = CG_ConfigString(CS_WARMUP);

  warmup = Q_atoi(info);

  if (warmup == 0 && cg.warmup) {

  } else if (warmup > 0 && cg.warmup <= 0 && cgs.gamestate != GS_WARMUP) {
    //		if(cg_announcer.integer > 0)
    // trap_S_StartLocalSound(
    // cgs.media.countPrepare, CHAN_ANNOUNCER );
    if (!cg.demoPlayback && cg_autoAction.integer & AA_DEMORECORD) {
      CG_autoRecord_f();
    }

    if (cg.warmupCount >= 0) {
      Pri("^3All players ready!^7\nMatch "
          "starting...\n");
      CPri("^3All players ready!^7\nMatch starting...");
    }
  }

  if (cgs.gamestate != GS_WARMUP || cg.warmup > 0) {
    cg.warmup = warmup;
  }

  cg.warmupCount++;
}

/*
==================
CG_ParseOIDInfo
==================
*/

oidInfo_t *CG_OIDInfoForEntityNum(int num) {
  int i;

  for (i = 0; i < MAX_OID_TRIGGERS; i++) {
    if (cgs.oidInfo[i].entityNum == num) {
      return &cgs.oidInfo[i];
    }
  }

  return NULL;
}

void CG_ParseOIDInfo(int num) {
  const char *info;
  const char *cs;
  int index = num - CS_OID_DATA;

  info = CG_ConfigString(num);

  memset(&cgs.oidInfo[index], 0, sizeof(cgs.oidInfo[0]));

  if (!info || !*info) {
    return;
  }

  cs = Info_ValueForKey(info, "s");
  if (cs && *cs) {
    cgs.oidInfo[index].spawnflags = Q_atoi(cs);
  }

  cs = Info_ValueForKey(info, "cia");
  if (cs && *cs) {
    cgs.oidInfo[index].customimageallies = cgs.gameShaders[Q_atoi(cs)];
  }

  cs = Info_ValueForKey(info, "cix");
  if (cs && *cs) {
    cgs.oidInfo[index].customimageaxis = cgs.gameShaders[Q_atoi(cs)];
  }

  cs = Info_ValueForKey(info, "o");
  if (cs && *cs) {
    cgs.oidInfo[index].objflags = Q_atoi(cs);
  }

  cs = Info_ValueForKey(info, "e");
  if (cs && *cs) {
    cgs.oidInfo[index].entityNum = Q_atoi(cs);
  }

  cs = Info_ValueForKey(info, "n");
  if (cs && *cs) {
    Q_strncpyz(cgs.oidInfo[index].name, cs, sizeof(cgs.oidInfo[0].name));
  }

  cs = Info_ValueForKey(info, "x");
  if (cs && *cs) {
    cgs.oidInfo[index].origin[0] = Q_atof(cs);
  }

  cs = Info_ValueForKey(info, "y");
  if (cs && *cs) {
    cgs.oidInfo[index].origin[1] = Q_atof(cs);
  }

  cs = Info_ValueForKey(info, "z");
  if (cs && *cs) {
    cgs.oidInfo[index].origin[2] = Q_atof(cs);
  }
}

void CG_ParseOIDInfos(void) {
  int i;

  for (i = 0; i < MAX_OID_TRIGGERS; i++) {
    CG_ParseOIDInfo(CS_OID_DATA + i);
  }
}

/*
==================
CG_ParseSpawns
==================
*/
void CG_ParseSpawns(void) {
  const char *info;
  const char *s;
  int i;
  int newteam;

  info = CG_ConfigString(CS_MULTI_INFO);
  s = Info_ValueForKey(info, "numspawntargets");

  if (!s || !strlen(s)) {
    return;
  }

  // first index is for autopicking
  Q_strncpyz(cg.spawnPoints[0], CG_TranslateString("Auto Pick"), MAX_SPAWNDESC);

  cg.spawnCount = Q_atoi(s) + 1;

  for (i = 1; i < cg.spawnCount; i++) {
    info = CG_ConfigString(CS_MULTI_SPAWNTARGETS + i - 1);

    s = Info_ValueForKey(info, "spawn_targ");

    if (!s || !strlen(s)) {
      return;
    }

    Q_strncpyz(cg.spawnPoints[i], CG_TranslateString(s), MAX_SPAWNDESC);

    s = Info_ValueForKey(info, "x");
    if (!s || !strlen(s)) {
      return;
    }
    cg.spawnCoordsUntransformed[i][0] = cg.spawnCoords[i][0] = Q_atof(s);

    s = Info_ValueForKey(info, "y");
    if (!s || !strlen(s)) {
      return;
    }
    cg.spawnCoordsUntransformed[i][1] = cg.spawnCoords[i][1] = Q_atof(s);

    if (cgs.ccLayers) {
      s = Info_ValueForKey(info, "z");
      if (!s || !strlen(s)) {
        return;
      }
      cg.spawnCoordsUntransformed[i][2] = cg.spawnCoords[i][2] = Q_atof(s);
    }

    CG_TransformToCommandMapCoord(&cg.spawnCoords[i][0], &cg.spawnCoords[i][1]);

    s = Info_ValueForKey(info, "t");

    newteam = Q_atoi(s);
    if (cg.spawnTeams[i] != newteam) {
      cg.spawnTeams_old[i] = cg.spawnTeams[i];
      cg.spawnTeams_changeTime[i] = cg.time;
      cg.spawnTeams[i] = (team_t)newteam;
    }

    s = Info_ValueForKey(info, "c");
    cg.spawnPlayerCounts[i] = Q_atoi(s);
  }
}

/*
=====================
CG_ParseScreenFade
=====================
*/
static void CG_ParseScreenFade(void) {
  const char *info;
  const char *token;

  info = CG_ConfigString(CS_SCREENFADE);

  token = COM_Parse(&info);
  cgs.fadeAlpha = Q_atof(token);

  token = COM_Parse(&info);
  cgs.fadeStartTime = Q_atoi(token);
  token = COM_Parse(&info);
  cgs.fadeDuration = Q_atoi(token);

  if (cgs.fadeStartTime + cgs.fadeDuration < cg.time) {
    cgs.fadeAlphaCurrent = cgs.fadeAlpha;
  }
}

/*
==============
CG_ParseFog
    float near dist
    float far dist
    float density
    float[3] r,g,b
    int		time
==============
*/
static void CG_ParseFog(void) {
  const char *info;
  const char *token;
  float ne, fa, r, g, b, density;
  int time;

  info = CG_ConfigString(CS_FOGVARS);

  token = COM_Parse(&info);
  ne = Q_atof(token);
  token = COM_Parse(&info);
  fa = Q_atof(token);
  token = COM_Parse(&info);
  density = Q_atof(token);
  token = COM_Parse(&info);
  r = Q_atof(token);
  token = COM_Parse(&info);
  g = Q_atof(token);
  token = COM_Parse(&info);
  b = Q_atof(token);
  token = COM_Parse(&info);
  time = Q_atoi(token);

  if (fa) // far of '0' from a target_fog means "return to map fog"
  {
    trap_R_SetFog(FOG_SERVER, (int)ne, (int)fa, r, g, b, density + .1);
    trap_R_SetFog(FOG_CMD_SWITCHFOG, FOG_SERVER, time, 0, 0, 0, 0);
  } else {
    trap_R_SetFog(FOG_CMD_SWITCHFOG, FOG_MAP, time, 0, 0, 0, 0);
  }
}

static void CG_ParseGlobalFog(void) {
  const char *info;
  const char *token;
  qboolean restore;
  float r, g, b, depthForOpaque;
  int duration;

  info = CG_ConfigString(CS_GLOBALFOGVARS);

  token = COM_Parse(&info);
  restore = Q_atoi(token) ? qtrue : qfalse;
  token = COM_Parse(&info);
  duration = Q_atoi(token);

  if (restore) {
    trap_R_SetGlobalFog(qtrue, duration, 0.f, 0.f, 0.f, 0);
  } else {
    token = COM_Parse(&info);
    r = Q_atof(token);
    token = COM_Parse(&info);
    g = Q_atof(token);
    token = COM_Parse(&info);
    b = Q_atof(token);
    token = COM_Parse(&info);
    depthForOpaque = Q_atof(token);

    trap_R_SetGlobalFog(qfalse, duration, r, g, b, depthForOpaque);
  }
}

// Parse server version info (for demo playback compatibility)
void CG_ParseServerVersionInfo(const char *pszVersionInfo) {
  // This will expand to a tokenized string, eventually but for
  // now we only need to worry about 1 number :)
  cgs.game_versioninfo = Q_atoi(pszVersionInfo);
}

// Parse reinforcement offsets
void CG_ParseReinforcementTimes(const char *pszReinfSeedString) {
  const char *tmp = pszReinfSeedString, *tmp2;
  unsigned int i, j, dwOffset[TEAM_NUM_TEAMS];

#define GETVAL(x, y)                                                           \
  if ((tmp = strchr(tmp, ' ')) == NULL) {                                      \
    return;                                                                    \
  }                                                                            \
  x = Q_atoi(++tmp) / y;

  dwOffset[TEAM_ALLIES] = Q_atoi(pszReinfSeedString) >> REINF_BLUEDELT;
  GETVAL(dwOffset[TEAM_AXIS], (1 << REINF_REDDELT));
  tmp2 = tmp;

  for (i = TEAM_AXIS; i <= TEAM_ALLIES; i++) {
    tmp = tmp2;
    for (j = 0; j < MAX_REINFSEEDS; j++) {
      if (j == dwOffset[i]) {
        GETVAL(cgs.aReinfOffset[i], aReinfSeeds[j]);
        cgs.aReinfOffset[i] *= 1000;
        break;
      }
    }
  }
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues(void) {
  cgs.levelStartTime = Q_atoi(CG_ConfigString(CS_LEVEL_START_TIME));
  cgs.intermissionStartTime =
      Q_atoi(CG_ConfigString(CS_INTERMISSION_START_TIME));
  cg.warmup = Q_atoi(CG_ConfigString(CS_WARMUP));

  // rain - set all of this crap in cgs - it won't be set if it doesn't
  // change, otherwise.  consider:
  // vote was called 5 minutes ago for 'Match Reset'.  you connect.
  // you're sent that value for CS_VOTE_STRING, but ignore it, so
  // you have nothing to use if another 'Match Reset' vote is called
  // (no update will be sent because the string will be the same.)

  cgs.voteTime = Q_atoi(CG_ConfigString(CS_VOTE_TIME));
  cgs.voteYes = Q_atoi(CG_ConfigString(CS_VOTE_YES));
  cgs.voteNo = Q_atoi(CG_ConfigString(CS_VOTE_NO));
  Q_strncpyz(cgs.voteString, CG_ConfigString(CS_VOTE_STRING),
             sizeof(cgs.voteString));

  cg.teamFirstBlood = Q_atoi(CG_ConfigString(CS_FIRSTBLOOD));
  // rain - yes, the order is this way on purpose. not my fault!
  cg.teamWonRounds[1] = Q_atoi(CG_ConfigString(CS_ROUNDSCORES1));
  cg.teamWonRounds[0] = Q_atoi(CG_ConfigString(CS_ROUNDSCORES2));

  // OSP
  CG_ParseServerVersionInfo(CG_ConfigString(CS_VERSIONINFO));
  CG_ParseReinforcementTimes(CG_ConfigString(CS_REINFSEEDS));
  // OSP
}

/*
=====================
CG_ShaderStateChanged
=====================
*/
void CG_ShaderStateChanged(void) {
  char originalShader[MAX_QPATH];
  char newShader[MAX_QPATH];
  char timeOffset[16];
  const char *o;
  const char *n, *t;

  o = CG_ConfigString(CS_SHADERSTATE);
  while (o && *o) {
    n = strstr(o, "=");
    if (n && *n) {
      strncpy(originalShader, o, n - o);
      originalShader[n - o] = 0;
      n++;
      t = strstr(n, ":");
      if (t && *t) {
        strncpy(newShader, n, t - n);
        newShader[t - n] = 0;
      } else {
        break;
      }
      t++;
      o = strstr(t, "@");
      if (o) {
        strncpy(timeOffset, t, o - t);
        timeOffset[o - t] = 0;
        o++;
        trap_R_RemapShader(cgs.gameShaderNames[Q_atoi(originalShader)],
                           cgs.gameShaderNames[Q_atoi(newShader)], timeOffset);
      }
    } else {
      break;
    }
  }
}

/*
===============
CG_ChargeTimesChanged
===============
*/
void CG_ChargeTimesChanged(void) {
  const char *info;

  info = CG_ConfigString(CS_CHARGETIMES);

  cg.soldierChargeTime[0] = Q_atoi(Info_ValueForKey(info, "axs_sld"));
  cg.soldierChargeTime[1] = Q_atoi(Info_ValueForKey(info, "ald_sld"));
  cg.medicChargeTime[0] = Q_atoi(Info_ValueForKey(info, "axs_mdc"));
  cg.medicChargeTime[1] = Q_atoi(Info_ValueForKey(info, "ald_mdc"));
  cg.engineerChargeTime[0] = Q_atoi(Info_ValueForKey(info, "axs_eng"));
  cg.engineerChargeTime[1] = Q_atoi(Info_ValueForKey(info, "ald_eng"));
  cg.ltChargeTime[0] = Q_atoi(Info_ValueForKey(info, "axs_lnt"));
  cg.ltChargeTime[1] = Q_atoi(Info_ValueForKey(info, "ald_lnt"));
  cg.covertopsChargeTime[0] = Q_atoi(Info_ValueForKey(info, "axs_cvo"));
  cg.covertopsChargeTime[1] = Q_atoi(Info_ValueForKey(info, "ald_cvo"));
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified(void) {
  const char *str;
  int num;
  auto rtvHandler = ETJump::rtvHandler;

  num = Q_atoi(CG_Argv(1));

  // get the gamestate from the client system, which will have the
  // new configstring already integrated
  trap_GetGameState(&cgs.gameState);

  // look up the individual string that was modified
  str = CG_ConfigString(num);

  // do something with it if necessary
  if (num == CS_MUSIC) {
    CG_StartMusic();
  } else if (num == CS_MUSIC_QUEUE) {
    CG_QueueMusic();
  } else if (num == CS_SERVERINFO) {
    CG_ParseServerinfo();
  } else if (num == CS_SYSTEMINFO) {
    CG_ParseSysteminfo();
  } else if (num == CS_WARMUP) {
    CG_ParseWarmup();
  } else if (num == CS_FIRSTBLOOD) {
    cg.teamFirstBlood = Q_atoi(str);
  } else if (num == CS_ROUNDSCORES1) {
    cg.teamWonRounds[1] = Q_atoi(str);
  } else if (num == CS_ROUNDSCORES2) {
    cg.teamWonRounds[0] = Q_atoi(str);
  } else if (num >= CS_MULTI_SPAWNTARGETS &&
             num < CS_MULTI_SPAWNTARGETS + MAX_MULTI_SPAWNTARGETS) {
    CG_ParseSpawns();
  } else if (num == CS_VERSIONINFO) {
    CG_ParseServerVersionInfo(str); // OSP - set versioning info
                                    // for older demo playback
  } else if (num == CS_REINFSEEDS) {
    CG_ParseReinforcementTimes(
        str); // OSP - set reinforcement times for each team
  } else if (num == CS_LEVEL_START_TIME) {
    cgs.levelStartTime = Q_atoi(str);
  } else if (num == CS_INTERMISSION_START_TIME) {
    cgs.intermissionStartTime = Q_atoi(str);
  } else if (num == CS_VOTE_TIME) {
    cgs.voteTime = Q_atoi(str);

    if (!rtvHandler->rtvVoteActive()) {
      rtvHandler->resetRtvEventHandler();
    }

    cgs.voteModified = qtrue;
  } else if (num == CS_VOTE_YES) {
    // CS_VOTE_YES might be processed before CS_VOTE_STRING, so on initial
    // 'callvote rtv' command, the check for rtvVoteActive might return false
    if (rtvHandler->rtvVoteActive() || strlen(str) > 1) {
      rtvHandler->setRtvConfigStrings(str);
      rtvHandler->countRtvVotes();
    } else {
      cgs.voteYes = Q_atoi(str);
    }
    cgs.voteModified = qtrue;
  } else if (num == CS_VOTE_NO) {
    cgs.voteNo = Q_atoi(str);
    cgs.voteModified = qtrue;
  } else if (num == CS_VOTE_STRING) {
    Q_strncpyz(cgs.voteString, str, sizeof(cgs.voteString));
    rtvHandler->setRtvVoteStatus();
  } else if (num == CS_INTERMISSION) {
    cg.intermissionStarted = Q_atoi(str) ? qtrue : qfalse;
  } else if (num == CS_SCREENFADE) {
    CG_ParseScreenFade();
  } else if (num == CS_FOGVARS) {
    CG_ParseFog();
  } else if (num == CS_GLOBALFOGVARS) {
    CG_ParseGlobalFog();
  } else if (num >= CS_MODELS && num < CS_MODELS + MAX_MODELS) {
    cgs.gameModels[num - CS_MODELS] = trap_R_RegisterModel(str);

    if (!cgs.gameModels[num - CS_MODELS]) {
      CG_Printf(S_COLOR_YELLOW "WARNING: failed to register server model %s\n",
                str);
    }
  } else if (num >= CS_SOUNDS && num < CS_SOUNDS + MAX_SOUNDS) {
    if (str[0] != '*') // player specific sounds don't register here
    {                  // Ridah, register sound scripts seperately
      if (!strstr(str, ".wav")) {
        CG_SoundScriptPrecache(str);
      } else {
        cgs.gameSounds[num - CS_SOUNDS] =
            trap_S_RegisterSound(str, qfalse); // FIXME: add a
                                               // compress flag?
      }
    }
  } else if (num >= CS_SHADERS && num < CS_SHADERS + MAX_CS_SHADERS) {
    cgs.gameShaders[num - CS_SHADERS] = str[0] == '*'
                                            ? trap_R_RegisterShader(str + 1)
                                            : trap_R_RegisterShaderNoMip(str);
    Q_strncpyz(cgs.gameShaderNames[num - CS_SHADERS],
               str[0] == '*' ? str + 1 : str, MAX_QPATH);
  } else if (num >= CS_SKINS && num < CS_SKINS + MAX_CS_SKINS) {
    cgs.gameModelSkins[num - CS_SKINS] = trap_R_RegisterSkin(str);
  } else if (num >= CS_CHARACTERS && num < CS_CHARACTERS + MAX_CHARACTERS) {
    if (!BG_FindCharacter(str)) {
      cgs.gameCharacters[num - CS_CHARACTERS] = BG_FindFreeCharacter(str);

      Q_strncpyz(
          cgs.gameCharacters[num - CS_CHARACTERS]->characterFile, str,
          sizeof(cgs.gameCharacters[num - CS_CHARACTERS]->characterFile));

      if (!CG_RegisterCharacter(str, cgs.gameCharacters[num - CS_CHARACTERS])) {
        CG_Error("ERROR: CG_ConfigStringModified: "
                 "failed to load character "
                 "file '%s'\n",
                 str);
      }
    }
  } else if (num >= CS_PLAYERS && num < CS_PLAYERS + MAX_CLIENTS) {
    CG_NewClientInfo(num - CS_PLAYERS);
  } else if (num >= CS_DLIGHTS && num < CS_DLIGHTS + MAX_DLIGHT_CONFIGSTRINGS) {
    CG_SetupDlightstyles();
  } else if (num == CS_SHADERSTATE) {
    CG_ShaderStateChanged();
  } else if (num == CS_CHARGETIMES) {
    CG_ChargeTimesChanged();
  } else if (num >= CS_FIRETEAMS && num < CS_FIRETEAMS + MAX_FIRETEAMS) {
    CG_ParseFireteams();
  } else if (num == CS_SKYBOXORG) {
    CG_ParseSkyBox();
  } else if (num >= CS_TAGCONNECTS && num < CS_TAGCONNECTS + MAX_TAGCONNECTS) {
    CG_ParseTagConnect(num);
  } else if (num == CS_FILTERCAMS) {
    cg.filtercams = Q_atoi(str) ? qtrue : qfalse;
  } else if (num >= CS_OID_DATA && num < CS_OID_DATA + MAX_OID_TRIGGERS) {
    CG_ParseOIDInfo(num);
  }
}

constexpr int REPLAY_MSG = 1 << 0;
constexpr int SERVER_MSG = 1 << 1;

/*
=======================
CG_AddToTeamChat

=======================
*/
static void CG_AddToTeamChat(const char *str, int clientnum, int msgType) {
  int len;
  char *p, *ls;
  int lastcolor;
  const int chatHeight =
      std::clamp(cg_teamChatHeight.integer, 0, TEAMCHAT_HEIGHT);
  const int chatWidth =
      std::clamp(etj_chatLineWidth.integer, 1, TEAMCHAT_WIDTH);
  const team_t team = ((msgType & REPLAY_MSG || msgType & SERVER_MSG)
                           ? TEAM_SPECTATOR
                           : cgs.clientinfo[clientnum].team);

  if (chatHeight <= 0 || cg_teamChatTime.integer <= 0) {
    // team chat disabled, dump into normal chat
    cgs.teamChatPos = cgs.teamLastChatPos = 0;
    return;
  }

  len = 0;

  p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
  *p = 0;

  lastcolor = '7';

  ls = NULL;
  while (*str) {
    if (len > chatWidth - 1) {
      if (ls) {
        str -= (p - ls);
        str++;
        p -= (p - ls);
      }
      *p = 0;

      cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;
      cgs.teamChatMsgTeams[cgs.teamChatPos % chatHeight] = team;

      cgs.teamChatPos++;
      p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
      *p = 0;
      *p++ = Q_COLOR_ESCAPE;
      *p++ = lastcolor;
      len = 0;
      ls = NULL;
    }

    if (Q_IsColorString(str)) {
      *p++ = *str++;
      lastcolor = *str;
      *p++ = *str++;
      continue;
    }
    if (*str == ' ') {
      ls = p;
    }
    *p++ = *str++;
    len++;
  }
  *p = 0;

  cgs.teamChatMsgTeams[cgs.teamChatPos % chatHeight] = team;

  cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;
  cgs.teamChatPos++;

  if (cgs.teamChatPos - cgs.teamLastChatPos > chatHeight) {
    cgs.teamLastChatPos = cgs.teamChatPos - chatHeight;
  }
}

/*
=======================
CG_AddToNotify

=======================
*/
void CG_AddToNotify(const char *str) {
  int len;
  char *p, *ls;
  int lastcolor;
  int chatHeight;
  float notifytime;
  char var[MAX_TOKEN_CHARS];

  trap_Cvar_VariableStringBuffer("con_notifytime", var, sizeof(var));
  notifytime = Q_atof(var) * 1000;

  chatHeight = NOTIFY_HEIGHT;

  if (chatHeight <= 0 || notifytime <= 0) {
    // team chat disabled, dump into normal chat
    cgs.notifyPos = cgs.notifyLastPos = 0;
    return;
  }

  len = 0;

  p = cgs.notifyMsgs[cgs.notifyPos % chatHeight];
  *p = 0;

  lastcolor = '7';

  ls = NULL;
  while (*str) {
    if (len > NOTIFY_WIDTH - 1 || (*str == '\n' && (*(str + 1) != 0))) {
      if (ls) {
        str -= (p - ls);
        str++;
        p -= (p - ls);
      }
      *p = 0;

      cgs.notifyMsgTimes[cgs.notifyPos % chatHeight] = cg.time;

      cgs.notifyPos++;
      p = cgs.notifyMsgs[cgs.notifyPos % chatHeight];
      *p = 0;
      *p++ = Q_COLOR_ESCAPE;
      *p++ = lastcolor;
      len = 0;
      ls = NULL;
    }

    if (Q_IsColorString(str)) {
      *p++ = *str++;
      lastcolor = *str;
      *p++ = *str++;
      continue;
    }
    if (*str == ' ') {
      ls = p;
    }
    while (*str == '\n') {
      // TTimo gcc warning: value computed is not used
      // was *str++;
      str++;
    }

    if (*str) {
      *p++ = *str++;
      len++;
    }
  }
  *p = 0;

  cgs.notifyMsgTimes[cgs.notifyPos % chatHeight] = cg.time;
  cgs.notifyPos++;

  if (cgs.notifyPos - cgs.notifyLastPos > chatHeight) {
    cgs.notifyLastPos = cgs.notifyPos - chatHeight;
  }
}

namespace ETJump {
extern void initTimer();
}
/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.

A tournement restart will clear everything, but doesn't
require a reload of all the media
===============
*/
static void CG_MapRestart(void) {
  if (cg_showmiss.integer) {
    CG_Printf("CG_MapRestart\n");
  }

  memset(&cg.lastWeapSelInBank[0], 0,
         MAX_WEAP_BANKS_MP * sizeof(int)); // clear weapon bank selections

  cg.numbufferedSoundScripts = 0;

  cg.centerPrintTime = 0; // reset centerprint counter so previous
                          // messages don't re-appear
  cg.itemPickupTime = 0;  // reset item pickup counter so previous
                          // messages don't re-appear
  cg.cursorHintFade = 0;  // reset cursor hint timer

  // DHM - Nerve :: Reset complaint system
  cgs.complaintClient = -1;
  cgs.complaintEndTime = 0;

  CG_LimboPanel_RequestObjective();

  // (SA) clear zoom (so no warpies)
  cg.zoomedBinoc = qfalse;
  cg.zoomedScope = qfalse;
  cg.zoomTime = 0;
  cg.zoomval = 0;

  cgs.complaintEndTime = 0;
  cgs.invitationEndTime = 0;
  cgs.applicationEndTime = 0;
  cgs.propositionEndTime = 0;
  cgs.autoFireteamEndTime = 0;
  cgs.autoFireteamCreateEndTime = 0;

  // reset fog to world fog (if present)
  trap_R_SetFog(FOG_CMD_SWITCHFOG, FOG_MAP, 20, 0, 0, 0, 0);

  // clear pmext
  memset(&cg.pmext, 0, sizeof(cg.pmext));

  cg.pmext.bAutoReload = (cg_autoReload.integer > 0) ? qtrue : qfalse;
  cg.pmext.autoSprint = etj_autoSprint.integer;

  numSplinePaths = 0;
  numPathCorners = 0;
  cg.numOIDtriggers2 = 0;

  cgs.fadeStartTime = 0;
  cgs.fadeAlpha = 0;
  trap_Cvar_Set("cg_letterbox", "0");

  CG_ParseEntitiesFromString();

  CG_LoadObjectiveData();

  CG_InitLocalEntities();
  CG_InitMarkPolys();

  cg.editingSpeakers = qfalse;

  BG_BuildSplinePaths();

  InitSmokeSprites();

  // Rafael particles
  CG_ClearParticles();

  // Ridah, trails
  //	CG_ClearTrails ();
  // done.

  // Ridah
  CG_ClearFlameChunks();
  CG_SoundInit();
  // done.

  cg.intermissionStarted = qfalse;
  cg.lightstylesInited = qfalse;
  cg.mapRestart = qtrue;
  cg.timelimitWarnings = 0;
  cgs.voteTime = 0;
  cgs.dumpStatsTime = 0;

  CG_StartMusic();

  trap_S_ClearLoopingSounds();
  trap_S_ClearSounds(qfalse);

  // ydnar
  trap_R_ClearDecals();

  cg.latchAutoActions = qfalse;
  cg.latchVictorySound = qfalse; // NERVE - SMF
                                 // JPW NERVE -- reset render flags
  cg_fxflags = 0;
  // jpw

  // we really should clear more parts of cg here and stop sounds
  cg.v_dmg_time = 0;
  cg.v_noFireTime = 0;
  cg.v_fireTime = 0;

  cg.filtercams = Q_atoi(CG_ConfigString(CS_FILTERCAMS)) ? qtrue : qfalse;

  // reset on map restart to avoid de-sync from cheat protected cvars
  // on ETe/ETL due to them resetting cheat cvars on map start
  cg.shadowCvarsSet = false;

  CG_ChargeTimesChanged();

  CG_ParseFireteams();

  CG_ParseOIDInfos();

  CG_InitPM();

  CG_ParseSpawns();

  CG_ParseTagConnects();

  trap_Cvar_Set("cg_thirdPerson", "0");

  ETJump::initTimer();
}
// NERVE - SMF

#define MAX_VOICEFILES 8

// TAT - 10/28/2002 we've got some really big VO files now
#define MAX_VOICEFILESIZE 32768
#define MAX_VOICECHATS 272
// TAT - NOTE: If we're worried about space - do we really need 96 possible
// sounds for any one chat?
//			I think this is used to allow multiple sound clips for
// one command, so do we need 96 available selection sounds?
#define MAX_VOICESOUNDS 32

#define MAX_CHATSIZE 64
#define MAX_HEADMODELS 64

typedef struct voiceChat_s {
  char id[64];
  int numSounds;
  sfxHandle_t sounds[MAX_VOICESOUNDS];
  char chats[MAX_VOICESOUNDS][MAX_CHATSIZE];
  qhandle_t sprite[MAX_VOICESOUNDS]; // DHM - Nerve
} voiceChat_t;

typedef struct voiceChatList_s {
  char name[64];
  int gender;
  int numVoiceChats;
  voiceChat_t voiceChats[MAX_VOICECHATS];
} voiceChatList_t;

typedef struct headModelVoiceChat_s {
  char headmodel[64];
  int voiceChatNum;
} headModelVoiceChat_t;

voiceChatList_t voiceChatLists[MAX_VOICEFILES];
headModelVoiceChat_t headModelVoiceChat[MAX_HEADMODELS];

/*
=================
CG_ParseVoiceChats
=================
*/
int CG_ParseVoiceChats(const char *filename, voiceChatList_t *voiceChatList,
                       int maxVoiceChats) {
  int len, i;
  int current = 0;
  fileHandle_t f;
  auto buf = std::make_unique<char[]>(MAX_VOICEFILESIZE);
  const char **p, *ptr;
  const char *token;
  voiceChat_t *voiceChats;
  qboolean compress;

  compress = qtrue;
  if (cg_buildScript.integer) {
    compress = qfalse;
  }

  len = trap_FS_FOpenFile(filename, &f, FS_READ);
  if (!f) {
    trap_Print(va(S_COLOR_RED "voice chat file not found: %s\n", filename));
    return qfalse;
  }
  if (len >= MAX_VOICEFILESIZE) {
    trap_Print(va(S_COLOR_RED "voice chat file too large: %s "
                              "is %i, max allowed is %i",
                  filename, len, MAX_VOICEFILESIZE));
    trap_FS_FCloseFile(f);
    return qfalse;
  }

  trap_FS_Read(buf.get(), len, f);
  buf[len] = 0;
  trap_FS_FCloseFile(f);

  ptr = buf.get();
  p = &ptr;

  Com_sprintf(voiceChatList->name, sizeof(voiceChatList->name), "%s", filename);
  voiceChats = voiceChatList->voiceChats;
  for (i = 0; i < maxVoiceChats; i++) {
    voiceChats[i].id[0] = 0;
  }
  token = COM_ParseExt(p, qtrue);
  if (!token || token[0] == 0) {
    return qtrue;
  }
  if (!Q_stricmp(token, "female")) {
    voiceChatList->gender = GENDER_FEMALE;
  } else if (!Q_stricmp(token, "male")) {
    voiceChatList->gender = GENDER_MALE;
  } else if (!Q_stricmp(token, "neuter")) {
    voiceChatList->gender = GENDER_NEUTER;
  } else {
    trap_Print(va(S_COLOR_RED
                  "expected gender not found in voice chat file: %s\n",
                  filename));
    return qfalse;
  }

  // Gordon: setting before call so we can load multiple files into one
  // list TAT - 10/28/2002 - if you really want to be able to load
  // multiple files, you should take out the loop
  //		above that clears out all the commands "voiceChats[i].id[0]
  //=
  // 0;" 		We don't even want the MP voice chats in SP, so no
  // need anyway
  voiceChatList->numVoiceChats = 0;
  while (1) {
    token = COM_ParseExt(p, qtrue);
    if (!token || token[0] == 0) {
      return qtrue;
    }

    Com_sprintf(voiceChats[voiceChatList->numVoiceChats].id,
                sizeof(voiceChats[voiceChatList->numVoiceChats].id), "%s",
                token);
    token = COM_ParseExt(p, qtrue);
    if (Q_stricmp(token, "{")) {
      trap_Print(va(S_COLOR_RED "expected { found %s in voice chat "
                                "file: %s\n",
                    token, filename));
      return qfalse;
    }
    voiceChats[voiceChatList->numVoiceChats].numSounds = 0;
    current = voiceChats[voiceChatList->numVoiceChats].numSounds;

    while (1) {
      token = COM_ParseExt(p, qtrue);
      if (!token || token[0] == 0) {
        return qtrue;
      }
      if (!Q_stricmp(token, "}")) {
        break;
      }
      voiceChats[voiceChatList->numVoiceChats].sounds[current] =
          trap_S_RegisterSound(token, compress);
      token = COM_ParseExt(p, qtrue);
      if (!token || token[0] == 0) {
        return qtrue;
      }
      Com_sprintf(voiceChats[voiceChatList->numVoiceChats].chats[current],
                  MAX_CHATSIZE, "%s", token);

      // DHM - Nerve :: Specify sprite shader to show
      // above player's head
      token = COM_ParseExt(p, qfalse);
      if (!Q_stricmp(token, "}") || !token || token[0] == 0) {
        voiceChats[voiceChatList->numVoiceChats].sprite[current] =
            trap_R_RegisterShader("sprites/voiceChat");
        COM_RestoreParseSession(p);
      } else {
        voiceChats[voiceChatList->numVoiceChats].sprite[current] =
            trap_R_RegisterShader(token);
        if (voiceChats[voiceChatList->numVoiceChats].sprite[current] == 0) {
          voiceChats[voiceChatList->numVoiceChats].sprite[current] =
              trap_R_RegisterShader("sprites/voiceChat");
        }
      }
      // dhm - end

      voiceChats[voiceChatList->numVoiceChats].numSounds++;
      current = voiceChats[voiceChatList->numVoiceChats].numSounds;

      if (voiceChats[voiceChatList->numVoiceChats].numSounds >=
          MAX_VOICESOUNDS) {
        break;
      }
    }

    voiceChatList->numVoiceChats++;
    if (voiceChatList->numVoiceChats >= maxVoiceChats) {
      return qtrue;
    }
  }
  return qtrue;
}

/*
=================
CG_LoadVoiceChats
=================
*/
void CG_LoadVoiceChats(void) {
  int size;

  size = trap_MemoryRemaining();
  voiceChatLists[0].numVoiceChats = 0;
  voiceChatLists[1].numVoiceChats = 0;

  CG_ParseVoiceChats("scripts/wm_axis_chat.voice", &voiceChatLists[0],
                     MAX_VOICECHATS);
  CG_ParseVoiceChats("scripts/wm_allies_chat.voice", &voiceChatLists[1],
                     MAX_VOICECHATS);

  CG_Printf("voice chat memory size = %d\n", size - trap_MemoryRemaining());
}

/*
=================
CG_HeadModelVoiceChats
=================
*/
int CG_HeadModelVoiceChats(char *filename) {
  int len, i;
  fileHandle_t f;
  auto buf = std::make_unique<char[]>(MAX_VOICEFILESIZE);
  const char **p, *ptr;
  const char *token;

  len = trap_FS_FOpenFile(filename, &f, FS_READ);
  if (!f) {
    trap_Print(va("voice chat file not found: %s\n", filename));
    return -1;
  }
  if (len >= MAX_VOICEFILESIZE) {
    trap_Print(va(S_COLOR_RED "voice chat file too large: %s "
                              "is %i, max allowed is %i",
                  filename, len, MAX_VOICEFILESIZE));
    trap_FS_FCloseFile(f);
    return -1;
  }

  trap_FS_Read(buf.get(), len, f);
  buf[len] = 0;
  trap_FS_FCloseFile(f);

  ptr = buf.get();
  p = &ptr;

  token = COM_ParseExt(p, qtrue);
  if (!token || token[0] == 0) {
    return -1;
  }

  for (i = 0; i < MAX_VOICEFILES; i++) {
    if (!Q_stricmp(token, voiceChatLists[i].name)) {
      return i;
    }
  }

  // FIXME: maybe try to load the .voice file which name is stored in
  // token?

  return -1;
}

/*
=================
CG_GetVoiceChat
=================
*/
int CG_GetVoiceChat(voiceChatList_t *voiceChatList, vsayCmd_t *vsay,
                    sfxHandle_t *snd, qhandle_t *sprite, char **chat) {
  int i, rnd;

  for (i = 0; i < voiceChatList->numVoiceChats; i++) {
    if (!Q_stricmp(vsay->id, voiceChatList->voiceChats[i].id)) {
      if (vsay->variant > 0) {

        if (vsay->variant >= voiceChatList->voiceChats[i].numSounds) {
          rnd = voiceChatList->voiceChats[i].numSounds - 1;
        } else {
          rnd = vsay->variant - 1;
        }

      } else {
        rnd = vsay->random * voiceChatList->voiceChats[i].numSounds;
      }

      *snd = voiceChatList->voiceChats[i].sounds[rnd];
      *sprite = voiceChatList->voiceChats[i].sprite[rnd];
      *chat = voiceChatList->voiceChats[i].chats[rnd];
      return qtrue;
    }
  }
  return qfalse;
}

/*
=================
CG_VoiceChatListForClient
=================
*/
voiceChatList_t *CG_VoiceChatListForClient(int clientNum) {
  if (cgs.clientinfo[clientNum].team == TEAM_AXIS) {
    return &voiceChatLists[0];
  } else {
    return &voiceChatLists[1];
  }
}

#define MAX_VOICECHATBUFFER 32

typedef struct bufferedVoiceChat_s {
  int clientNum;
  sfxHandle_t snd;
  qhandle_t sprite;
  int voiceOnly;
  char cmd[MAX_SAY_TEXT];
  char message[MAX_SAY_TEXT];
  vec3_t origin; // NERVE - SMF
} bufferedVoiceChat_t;

bufferedVoiceChat_t voiceChatBuffer[MAX_VOICECHATBUFFER];

/*
=================
CG_PlayVoiceChat
=================
*/
void CG_PlayVoiceChat(bufferedVoiceChat_t *vchat) {
  // if we are going into the intermission, don't start any voices
  /*	// NERVE - SMF - don't do this in wolfMP
      if ( cg.intermissionStarted ) {
          return;
      }
  */

  if (!cg_noVoiceChats.integer) {
    trap_S_StartLocalSound(vchat->snd, CHAN_VOICE);

    // Arnout: don't show icons for the HQ (clientnum -1)
    if (vchat->clientNum != -1) {
      // DHM - Nerve :: Show icon above head
      if (vchat->clientNum == cg.snap->ps.clientNum) {
        cg.predictedPlayerEntity.voiceChatSprite = vchat->sprite;
        if (vchat->sprite == cgs.media.voiceChatShader) {
          cg.predictedPlayerEntity.voiceChatSpriteTime =
              cg.time + cg_voiceSpriteTime.integer;
        } else {
          cg.predictedPlayerEntity.voiceChatSpriteTime =
              cg.time + cg_voiceSpriteTime.integer * 2;
        }
      } else {
        cg_entities[vchat->clientNum].voiceChatSprite = vchat->sprite;
        VectorCopy(vchat->origin,
                   cg_entities[vchat->clientNum].lerpOrigin); // NERVE - SMF
        if (vchat->sprite == cgs.media.voiceChatShader) {
          cg_entities[vchat->clientNum].voiceChatSpriteTime =
              cg.time + cg_voiceSpriteTime.integer;
        } else {
          cg_entities[vchat->clientNum].voiceChatSpriteTime =
              cg.time + cg_voiceSpriteTime.integer * 2;
        }
      }
      // dhm - end
    }
  }
  if (!vchat->voiceOnly && !cg_noVoiceText.integer) {
    CG_AddToTeamChat(vchat->message, vchat->clientNum, 0);
    CG_Printf(va("[skipnotify]%s\n", vchat->message)); // JPW NERVE
  }
  voiceChatBuffer[cg.voiceChatBufferOut].snd = 0;
}

/*
=====================
CG_PlayBufferedVoieChats
=====================
*/
void CG_PlayBufferedVoiceChats(void) {
  if (cg.voiceChatTime < cg.time) {
    if (cg.voiceChatBufferOut != cg.voiceChatBufferIn &&
        voiceChatBuffer[cg.voiceChatBufferOut].snd) {
      //
      CG_PlayVoiceChat(&voiceChatBuffer[cg.voiceChatBufferOut]);
      //
      cg.voiceChatBufferOut = (cg.voiceChatBufferOut + 1) % MAX_VOICECHATBUFFER;
      cg.voiceChatTime = cg.time + 1000;
    }
  }
}

/*
=====================
CG_AddBufferedVoiceChat
=====================
*/
void CG_AddBufferedVoiceChat(bufferedVoiceChat_t *vchat) {
  // JPW NERVE new system doesn't buffer but overwrites vchats FIXME put
  // this on a cvar to choose which to use
  memcpy(&voiceChatBuffer[0], vchat, sizeof(bufferedVoiceChat_t));
  cg.voiceChatBufferIn = 0;
  CG_PlayVoiceChat(&voiceChatBuffer[0]);
}

/*
=================
CG_VoiceChatLocal
=================
*/
void CG_VoiceChatLocal(int mode, qboolean voiceOnly, int clientNum, int color,
                       vsayCmd_t *vsay, vec3_t origin) {
  char *chat;
  voiceChatList_t *voiceChatList;
  clientInfo_t *ci;
  sfxHandle_t snd;
  qhandle_t sprite;
  bufferedVoiceChat_t vchat;
  const char *msgColor = "^z";
  const char *msgTime = "";
  qtime_t t;

  trap_RealTime(&t);

  if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
    clientNum = 0;
  }
  ci = &cgs.clientinfo[clientNum];

  cgs.currentVoiceClient = clientNum;

  voiceChatList = CG_VoiceChatListForClient(clientNum);

  if (CG_GetVoiceChat(voiceChatList, vsay, &snd, &sprite, &chat)) {
    //
    if (mode == SAY_TEAM || !cg_teamChatsOnly.integer) {
      vchat.clientNum = clientNum;
      vchat.snd = snd;
      vchat.sprite = sprite;
      vchat.voiceOnly = voiceOnly;
      VectorCopy(origin, vchat.origin); // NERVE - SMF
      Q_strncpyz(vchat.cmd, vsay->id, sizeof(vchat.cmd));

      if (vsay->custom[0] != '\0') {
        chat = vsay->custom;
      }

      if (etj_drawMessageTime.integer) {
        if (cg.clientNum == clientNum) {
          msgColor = "^g";
        }
        if (etj_drawMessageTime.integer == 2) {
          msgTime = va("%s[%02d:%02d:%02d]^7 ", msgColor, t.tm_hour, t.tm_min,
                       t.tm_sec);
        } else {
          msgTime = va("%s[%02d:%02d]^7 ", msgColor, t.tm_hour, t.tm_min);
        }
      }

      if (mode == SAY_TEAM) {
        Com_sprintf(vchat.message, sizeof(vchat.message), "%s(%s^7)^3: %c%c%s",
                    msgTime, ci->name, Q_COLOR_ESCAPE, color,
                    CG_TranslateString(chat));
      } else if (mode == SAY_BUDDY) {
        Com_sprintf(vchat.message, sizeof(vchat.message), "%s<%s^7>^3: %c%c%s",
                    msgTime, ci->name, Q_COLOR_ESCAPE, color,
                    CG_TranslateString(chat));
      } else {
        Com_sprintf(vchat.message, sizeof(vchat.message), "%s%s^3: %c%c%s",
                    msgTime, ci->name, Q_COLOR_ESCAPE, color,
                    CG_TranslateString(chat));
      }
      CG_AddBufferedVoiceChat(&vchat);
    }
  }
}

/*
==================
ConcatArgs
==================
*/
// DO NOT use this anywhere but voice. Seems to be bugged. trap_argc() returns
// wrong values.
char *ConcatArgs(int start) {
  int i, c, tlen;
  static char line[MAX_STRING_CHARS];
  int len;
  char arg[MAX_STRING_CHARS];

  len = 0;
  // For some reason trap_Argc() returns argc + 1 on cgame
  c = trap_Argc() - 1;
  for (i = start; i < c; i++) {
    trap_Argv(i, arg, sizeof(arg));
    tlen = strlen(arg);
    if (len + tlen >= MAX_STRING_CHARS - 1) {
      break;
    }
    memcpy(line + len, arg, tlen);
    len += tlen;
    if (i != c - 1) {
      line[len] = ' ';
      len++;
    }
  }

  line[len] = 0;

  return line;
}

/*
=================
CG_VoiceChat
=================
*/
void CG_VoiceChat(int mode) {
  auto *cmd = "";
  int clientNum, color, variant = 5, rand = 6, custom = 7;
  qboolean voiceOnly;
  vec3_t origin; // NERVE - SMF
  vsayCmd_t vsay;

  memset(&vsay, 0, sizeof(vsay));

  voiceOnly = Q_atoi(CG_Argv(1)) ? qtrue : qfalse;
  clientNum = Q_atoi(CG_Argv(2));
  color = Q_atoi(CG_Argv(3));

  if (mode != SAY_ALL) {
    // NERVE - SMF - added origin
    origin[0] = Q_atof(CG_Argv(5));
    origin[1] = Q_atof(CG_Argv(6));
    origin[2] = Q_atof(CG_Argv(7));

    variant = 8;
    rand = 9;
    custom = 10;
  }

  vsay.variant = Q_atoi(CG_Argv(variant));
  trap_Argv(4, vsay.id, sizeof(vsay.id));
  vsay.random = Q_atof(CG_Argv(rand));
  trap_Argv(custom, vsay.custom, sizeof(vsay.custom));

  if (cg_noTaunt.integer != 0) {
    if (!strcmp(cmd, VOICECHAT_KILLINSULT) || !strcmp(cmd, VOICECHAT_TAUNT) ||
        !strcmp(cmd, VOICECHAT_DEATHINSULT) ||
        !strcmp(cmd, VOICECHAT_KILLGAUNTLET) ||
        !strcmp(cmd, VOICECHAT_PRAISE)) {
      return;
    }
  }

  CG_VoiceChatLocal(mode, voiceOnly, clientNum, color, &vsay, origin);
}
// -NERVE - SMF

/*
=================
CG_RemoveChatEscapeChar
=================
*/
static void CG_RemoveChatEscapeChar(char *text) {
  int i, l;

  l = 0;
  for (i = 0; text[i]; i++) {
    if (text[i] == '\x19' || text[i] == '\n') {
      continue;
    }
    text[l++] = text[i];
  }
  text[l] = '\0';
}

/*
=================
CG_LocalizeServerCommand

NERVE - SMF - localize string sent from server

- localization is ON by default.
- use [lof] in string to turn OFF
- use [lon] in string to turn back ON
=================
*/
const char *CG_LocalizeServerCommand(const char *buf) {
  static char token[MAX_TOKEN_CHARS];
  char temp[MAX_TOKEN_CHARS];
  qboolean togloc = qtrue;
  const char *s;
  int i, prev;

  memset(token, 0, sizeof(token));
  s = buf;
  prev = 0;

  for (i = 0; *s; i++, s++) {
    if (*s == '[' && (!Q_strncmp(s, "[lon]", 5) || !Q_strncmp(s, "[lof]", 5))) {
      if (togloc) {
        memset(temp, 0, sizeof(temp));
        strncpy(temp, buf + prev, i - prev);
        Q_strcat(token, sizeof(token), CG_TranslateString(temp));
      } else {
        strncat(token, buf + prev, i - prev);
      }

      if (s[3] == 'n') {
        togloc = qtrue;
      } else {
        togloc = qfalse;
      }

      i += 5;
      s += 5;
      prev = i;
    }
  }

  if (togloc) {
    memset(temp, 0, sizeof(temp));
    strncpy(temp, buf + prev, i - prev);
    Q_strcat(token, sizeof(token), CG_TranslateString(temp));
  } else {
    strncat(token, buf + prev, i - prev);
  }

  return token;
}
// -NERVE - SMF

// OSP
void CG_wstatsParse_cmd(void) {
  if (cg.showStats) {
    if (cg.statsWindow == NULL || cg.statsWindow->id != WID_STATS ||
        cg.statsWindow->inuse == qfalse) {
      CG_createStatsWindow();
    } else if (cg.statsWindow->state == WSTATE_SHUTDOWN) {
      cg.statsWindow->state = WSTATE_START;
      cg.statsWindow->time = trap_Milliseconds();
    }

    if (cg.statsWindow == NULL) {
      cg.showStats = qfalse;
    } else {
      cg.statsWindow->effects |= WFX_TEXTSIZING;
      cg.statsWindow->lineCount = 0;
      cg.windowCurrent = cg.statsWindow;
      CG_parseWeaponStats_cmd(CG_printWindow);
    }
  }
}

void CG_topshotsParse_cmd(qboolean doBest) {
  int iArg = 1;
  int iWeap = Q_atoi(CG_Argv(iArg++));
  topshotStats_t *ts = &cgs.topshots;

  ts->cWeapons = 0;

  while (iWeap) {
    int cnum = Q_atoi(CG_Argv(iArg++));
    int hits = Q_atoi(CG_Argv(iArg++));
    int atts = Q_atoi(CG_Argv(iArg++));
    int kills = Q_atoi(CG_Argv(iArg++));
    // rain - unused
    // int deaths = Q_atoi(CG_Argv(iArg++));
    float acc = (atts > 0) ? (float)(hits * 100) / (float)atts : 0.0f;
    char name[32];

    // rain - bump up iArg since we didn't push it into deaths,
    // above
    iArg++;

    if (ts->cWeapons < WS_MAX * 2) {
      BG_cleanName(cgs.clientinfo[cnum].name, name, 17, qfalse);
      Q_strncpyz(ts->strWS[ts->cWeapons++],
                 va("%-12s %5.1f %4d/%-4d %5d  %s",
                    aWeaponInfo[iWeap - 1].pszName, acc, hits, atts, kills,
                    name),
                 sizeof(ts->strWS[0]));
    }

    iWeap = Q_atoi(CG_Argv(iArg++));
  }
}

void CG_ParseWeaponStats(void) {
  cgs.ccWeaponShots = Q_atoi(CG_Argv(1));
  cgs.ccWeaponHits = Q_atoi(CG_Argv(2));
}

void CG_ParsePortalPos(void) {
  int i;

  cgs.ccCurrentCamObjective = Q_atoi(CG_Argv(1));
  cgs.ccPortalEnt = Q_atoi(CG_Argv(8));

  for (i = 0; i < 3; i++) {
    cgs.ccPortalPos[i] = Q_atof(CG_Argv(i + 2));
  }

  for (i = 0; i < 3; i++) {
    cgs.ccPortalAngles[i] = Q_atof(CG_Argv(i + 5));
  }
}

// Cached stats
void CG_parseWeaponStatsGS_cmd(void) {
  clientInfo_t *ci;
  gameStats_t *gs = &cgs.gamestats;
  int i, iArg = 1;
  int nClientID = Q_atoi(CG_Argv(iArg++));
  int nRounds = Q_atoi(CG_Argv(iArg++));
  int weaponMask = Q_atoi(CG_Argv(iArg++));
  int skillMask, xp = 0;

  gs->cWeapons = 0;
  gs->cSkills = 0;
  gs->fHasStats = qfalse;

  gs->nClientID = nClientID;
  gs->nRounds = nRounds;

  ci = &cgs.clientinfo[nClientID];

  //	Q_strncpyz(strName, ci->name, sizeof(strName));
  //	BG_cleanName(cgs.clientinfo[gs->nClientID].name, strName,
  // sizeof(strName), qfalse);

  if (weaponMask != 0) {
    char strName[MAX_STRING_CHARS];

    for (i = WS_KNIFE; i < WS_MAX; i++) {
      if (weaponMask & (1 << i)) {
        int nHits = Q_atoi(CG_Argv(iArg++));
        int nShots = Q_atoi(CG_Argv(iArg++));
        int nKills = Q_atoi(CG_Argv(iArg++));
        int nDeaths = Q_atoi(CG_Argv(iArg++));
        int nHeadshots = Q_atoi(CG_Argv(iArg++));

        Q_strncpyz(strName, va("%-12s  ", aWeaponInfo[i].pszName),
                   sizeof(strName));
        if (nShots > 0 || nHits > 0) {
          Q_strcat(
              strName, sizeof(strName),
              va("%5.1f %4d/%-4d ",
                 ((nShots == 0) ? 0.0 : (float)(nHits * 100.0 / (float)nShots)),
                 nHits, nShots));
        } else {
          Q_strcat(strName, sizeof(strName), va("                "));
        }

        Q_strncpyz(
            gs->strWS[gs->cWeapons++],
            va("%s%5d %6d%s", strName, nKills, nDeaths,
               ((aWeaponInfo[i].fHasHeadShots) ? va(" %9d", nHeadshots) : "")),
            sizeof(gs->strWS[0]));

        if (nShots > 0 || nHits > 0 || nKills > 0 || nDeaths) {
          gs->fHasStats = qtrue;
        }
      }
    }

    if (gs->fHasStats) {
      int dmg_given = Q_atoi(CG_Argv(iArg++));
      int dmg_rcvd = Q_atoi(CG_Argv(iArg++));
      int team_dmg = Q_atoi(CG_Argv(iArg++));

      Q_strncpyz(gs->strExtra[0],
                 va("Damage Given: %-6d  Team Damage: %d", dmg_given, team_dmg),
                 sizeof(gs->strExtra[0]));
      Q_strncpyz(gs->strExtra[1], va("Damage Recvd: %d", dmg_rcvd),
                 sizeof(gs->strExtra[0]));
    }
  }

  // Derive XP from individual skill XP
  skillMask = Q_atoi(CG_Argv(iArg++));
  for (i = SK_BATTLE_SENSE; i < SK_NUM_SKILLS; i++) {
    if (skillMask & (1 << i)) {
      ci->skillpoints[i] = Q_atoi(CG_Argv(iArg++));
      xp += ci->skillpoints[i];
    }
  }

  Q_strncpyz(gs->strRank,
             va("%-13s %d",
                ((ci->team == TEAM_AXIS) ? rankNames_Axis
                                         : rankNames_Allies)[ci->rank],
                xp),
             sizeof(gs->strRank));

  if (skillMask != 0) {
    char *str;

    for (i = SK_BATTLE_SENSE; i < SK_NUM_SKILLS; i++) {

      if ((skillMask & (1 << i)) == 0) {
        continue;
      }

      if (ci->skill[i] < NUM_SKILL_LEVELS - 1) {
        str = va("%4d/%-4d", ci->skillpoints[i], skillLevels[ci->skill[i] + 1]);
      } else {
        str = va("%d", ci->skillpoints[i]);
      }

      Q_strncpyz(gs->strSkillz[gs->cSkills++],
                 va("%-15s %3d %s", skillNames[i], ci->skill[i], str),
                 sizeof(gs->strSkillz[0]));
    }
  }
}

// Client-side stat presentation
void CG_parseWeaponStats_cmd(void(txt_dump)(const char *)) {
  clientInfo_t *ci;
  qboolean fFull = (txt_dump != CG_printWindow) ? qtrue : qfalse;
  qboolean fHasStats = qfalse;
  char strName[MAX_STRING_CHARS];
  int atts, deaths, dmg_given, dmg_rcvd, hits, kills, team_dmg, headshots;
  unsigned int i, iArg = 1;
  unsigned int nClient = Q_atoi(CG_Argv(static_cast<int>(iArg++)));
  unsigned int nRounds = Q_atoi(CG_Argv(static_cast<int>(iArg++)));
  unsigned int dwWeaponMask = Q_atoi(CG_Argv(static_cast<int>(iArg++)));
  unsigned int dwSkillPointMask, xp = 0;

  ci = &cgs.clientinfo[nClient];

  Q_strncpyz(strName, ci->name, sizeof(strName));
  BG_cleanName(cgs.clientinfo[nClient].name, strName, sizeof(strName), qfalse);
  txt_dump(va("^7Overall stats for: ^3%s ^7(^2%d^7 Round%s)\n\n", strName,
              nRounds, ((nRounds != 1) ? "s" : "")));
  //	txt_dump(va("^7Overall stats for: ^3%s\n\n", strName));

  if (fFull) {
    txt_dump("Weapon     Acrcy Hits/Atts Kills Deaths Headshots\n");
    txt_dump("-------------------------------------------------\n");
  } else {
    txt_dump("Weapon     Acrcy Hits/Atts Kll Dth HS\n");
    // txt_dump(     "-------------------------------------\n");
    txt_dump("\n");
  }

  if (!dwWeaponMask) {
    txt_dump("^3No weapon info available.\n");
  } else {
    for (i = WS_KNIFE; i < WS_MAX; i++) {
      if (dwWeaponMask & (1 << i)) {
        hits = Q_atoi(CG_Argv(static_cast<int>(iArg++)));
        atts = Q_atoi(CG_Argv(static_cast<int>(iArg++)));
        kills = Q_atoi(CG_Argv(static_cast<int>(iArg++)));
        deaths = Q_atoi(CG_Argv(static_cast<int>(iArg++)));
        headshots = Q_atoi(CG_Argv(static_cast<int>(iArg++)));

        Q_strncpyz(strName, va("^3%-9s: ", aWeaponInfo[i].pszName),
                   sizeof(strName));
        if (atts > 0 || hits > 0) {
          fHasStats = qtrue;
          Q_strcat(strName, sizeof(strName),
                   va("^7%5.1f ^5%4d/%-4d ",
                      ((atts == 0) ? 0.0 : (float)(hits * 100.0 / (float)atts)),
                      hits, atts));
        } else {
          Q_strcat(strName, sizeof(strName), va("                "));
          if (kills > 0 || deaths > 0) {
            fHasStats = qtrue;
          }
        }

        if (fFull) {
          txt_dump(va(
              "%s^2%5d ^1%6d%s\n", strName, kills, deaths,
              ((aWeaponInfo[i].fHasHeadShots) ? va(" ^3%9d", headshots) : "")));
        } else {
          txt_dump(va(
              "%s^2%3d ^1%3d%s\n", strName, kills, deaths,
              ((aWeaponInfo[i].fHasHeadShots) ? va(" ^3%2d", headshots) : "")));
        }
      }
    }

    if (fHasStats) {
      dmg_given = Q_atoi(CG_Argv(static_cast<int>(iArg++)));
      dmg_rcvd = Q_atoi(CG_Argv(static_cast<int>(iArg++)));
      team_dmg = Q_atoi(CG_Argv(static_cast<int>(iArg++)));

      if (!fFull) {
        txt_dump("\n\n");
      }

      txt_dump(va("\n^3Damage Given: ^7%-6d  ^3Team "
                  "Damage: ^7%d\n",
                  dmg_given, team_dmg));
      txt_dump(va("^3Damage Recvd: ^7%d\n", dmg_rcvd));
    }
  }

  if (!fFull) {
    txt_dump("\n\n\n");
  }

  // Derive XP from individual skill XP
  dwSkillPointMask = Q_atoi(CG_Argv(static_cast<int>(iArg++)));
  for (i = SK_BATTLE_SENSE; i < SK_NUM_SKILLS; i++) {
    if (dwSkillPointMask & (1 << i)) {
      ci->skillpoints[i] = Q_atoi(CG_Argv(static_cast<int>(iArg++)));
      xp += ci->skillpoints[i];
    }
  }

  txt_dump(va(
      "\n^2Rank: ^7%s (%d XP)\n",
      ((ci->team == TEAM_AXIS) ? rankNames_Axis : rankNames_Allies)[ci->rank],
      xp));

  if (!fFull) {
    txt_dump("\n\n\n");
  }

  txt_dump("Skills         Level/Points\n");
  if (fFull) {
    txt_dump("---------------------------\n");
  } else {
    txt_dump("\n");
  }

  if (dwSkillPointMask == 0) {
    txt_dump("^3No skills acquired!\n");
  } else {
    char *str;

    for (i = SK_BATTLE_SENSE; i < SK_NUM_SKILLS; i++) {

      if ((dwSkillPointMask & (1 << i)) == 0) {
        continue;
      }

      if (ci->skill[i] < NUM_SKILL_LEVELS - 1) {
        str = va("%d (%d/%d)", ci->skill[i], ci->skillpoints[i],
                 skillLevels[ci->skill[i] + 1]);
      } else {
        str = va("%d (%d)", ci->skill[i], ci->skillpoints[i]);
      }

      txt_dump(va("%-14s ^3%-12s\n", skillNames[i], str));
    }
  }
}

void CG_parseBestShotsStats_cmd(qboolean doTop, void(txt_dump)(const char *)) {
  int iArg = 1;
  qboolean fFull = (txt_dump != CG_printWindow) ? qtrue : qfalse;

  int iWeap = Q_atoi(CG_Argv(iArg++));
  if (!iWeap) {
    txt_dump(va("^3No qualifying %sshot info available.\n",
                ((doTop) ? "top" : "bottom")));
    return;
  }

  txt_dump(va("^2%s Match Accuracies:\n", (doTop) ? "BEST" : "WORST"));
  if (fFull) {
    txt_dump("\n^3WP   Acrcy Hits/Atts Kills Deaths\n");
    txt_dump("-------------------------------------------------"
             "------------\n");
  } else {
    txt_dump("^3WP   Acrcy Hits/Atts Kll Dth\n");
    //	txt_dump(
    //"-------------------------------------------\n");
    txt_dump("\n");
  }

  while (iWeap) {
    int cnum = Q_atoi(CG_Argv(iArg++));
    int hits = Q_atoi(CG_Argv(iArg++));
    int atts = Q_atoi(CG_Argv(iArg++));
    int kills = Q_atoi(CG_Argv(iArg++));
    int deaths = Q_atoi(CG_Argv(iArg++));
    float acc = (atts > 0) ? (float)(hits * 100) / (float)atts : 0.0;
    char name[32];

    if (fFull) {
      BG_cleanName(cgs.clientinfo[cnum].name, name, 30, qfalse);
      txt_dump(va("^3%s ^7%5.1f ^5%4d/%-4d ^2%5d ^1%6d ^7%s\n",
                  aWeaponInfo[iWeap - 1].pszCode, acc, hits, atts, kills,
                  deaths, name));
    } else {
      BG_cleanName(cgs.clientinfo[cnum].name, name, 12, qfalse);
      txt_dump(va("^3%s ^7%5.1f ^5%4d/%-4d ^2%3d ^1%3d ^7%s\n",
                  aWeaponInfo[iWeap - 1].pszCode, acc, hits, atts, kills,
                  deaths, name));
    }

    iWeap = Q_atoi(CG_Argv(iArg++));
  }
}

void CG_parseTopShotsStats_cmd(qboolean doTop, void(txt_dump)(const char *)) {
  int i, iArg = 1;
  int cClients = Q_atoi(CG_Argv(iArg++));
  int iWeap = Q_atoi(CG_Argv(iArg++));
  int wBestAcc = Q_atoi(CG_Argv(iArg++));

  txt_dump(va("Weapon accuracies for: ^3%s\n",
              (iWeap >= WS_KNIFE && iWeap < WS_MAX) ? aWeaponInfo[iWeap].pszName
                                                    : "UNKNOWN"));

  txt_dump("\n^3  Acc Hits/Atts Kills Deaths\n");
  txt_dump("----------------------------------------------------------\n");

  if (!cClients) {
    txt_dump("NO QUALIFYING WEAPON INFO AVAILABLE.\n");
    return;
  }

  for (i = 0; i < cClients; i++) {
    int cnum = Q_atoi(CG_Argv(iArg++));
    int hits = Q_atoi(CG_Argv(iArg++));
    int atts = Q_atoi(CG_Argv(iArg++));
    int kills = Q_atoi(CG_Argv(iArg++));
    int deaths = Q_atoi(CG_Argv(iArg++));
    float acc = (atts > 0) ? (float)(hits * 100) / (float)atts : 0.0;
    const char *color = (((doTop) ? acc : ((float)wBestAcc) + 0.999) >=
                         ((doTop) ? wBestAcc : acc))
                            ? "^3"
                            : "^7";
    char name[32];

    BG_cleanName(cgs.clientinfo[cnum].name, name, 30, qfalse);
    txt_dump(va("%s%5.1f ^5%4d/%-4d ^2%5d ^1%6d %s%s\n", color, acc, hits, atts,
                kills, deaths, color, name));
  }
}

void CG_scores_cmd(void) {
  const char *str = CG_Argv(1);

  CG_Printf("[skipnotify]%s", str);
  if (cgs.dumpStatsFile > 0) {
    char s[MAX_STRING_CHARS];

    BG_cleanName(str, s, sizeof(s), qtrue);
    trap_FS_Write(s, strlen(s), cgs.dumpStatsFile);
  }

  if (trap_Argc() > 2) {
    if (cgs.dumpStatsFile > 0) {
      qtime_t ct;

      trap_RealTime(&ct);
      str = va("\nStats recorded: %02d:%02d:%02d (%02d "
               "%s %d)\n\n\n",
               ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday, aMonths[ct.tm_mon],
               1900 + ct.tm_year);

      trap_FS_Write(str, strlen(str), cgs.dumpStatsFile);

      CG_Printf("[cgnotify]\n^3>>> Stats recorded to: "
                "^7%s\n\n",
                cgs.dumpStatsFileName);
      trap_FS_FCloseFile(cgs.dumpStatsFile);
      cgs.dumpStatsFile = 0;
    }
    cgs.dumpStatsTime = 0;
  }
}

void CG_printFile(const char *str) {
  CG_Printf(str);
  if (cgs.dumpStatsFile > 0) {
    char s[MAX_STRING_CHARS];

    BG_cleanName(str, s, sizeof(s), qtrue);
    trap_FS_Write(s, strlen(s), cgs.dumpStatsFile);
  }
}

void CG_dumpStats(void) {
  qtime_t ct;
  qboolean fDoScores = qfalse;
  const char *info = CG_ConfigString(CS_SERVERINFO);
  char *s = va("^3>>> %s: ^2%s\n\n", CG_TranslateString("Map"),
               Info_ValueForKey(info, "mapname"));

  trap_RealTime(&ct);
  // /me holds breath (using circular va() buffer)
  if (cgs.dumpStatsFile == 0) {
    fDoScores = qtrue;
    cgs.dumpStatsFileName =
        va("stats/%d.%02d.%02d/%02d%02d%02d.txt", 1900 + ct.tm_year,
           ct.tm_mon + 1, ct.tm_mday, ct.tm_hour, ct.tm_min, ct.tm_sec);
  }

  if (cgs.dumpStatsFile != 0) {
    trap_FS_FCloseFile(cgs.dumpStatsFile);
  }
  trap_FS_FOpenFile(cgs.dumpStatsFileName, &cgs.dumpStatsFile, FS_APPEND);

  CG_printFile(s);
  CG_parseWeaponStats_cmd(CG_printFile);
  if (cgs.dumpStatsFile == 0) {
    CG_Printf("[cgnotify]\n^3>>> %s: %s\n\n",
              CG_TranslateString("Could not create logfile"),
              cgs.dumpStatsFileName);
  }

  // Daisy-chain to scores info
  //	-- we play a game here for a statsall dump:
  //		1. we still have more ws entries in the queue to parse
  //		2. on the 1st ws entry, go ahead and send out the scores
  // request
  //		3. we'll just continue to parse the remaining ws entries
  // and
  // dump them to the log 		   before the scores result would
  // ever hit us.. thus, we still keep proper ordering :)
  if (fDoScores) {
    trap_SendClientCommand("scores");
  }
}
// -OSP

namespace ETJump {
static const char *addChatReplayModifications(const char *text) {
  static char msg[MAX_SAY_TEXT + 32];
  memset(msg, 0, sizeof(msg));

  const int diffTime = Q_atoi(CG_Argv(5));

  // don't prefix the replay notification message
  if (diffTime != -1) {
    Q_strcat(msg, sizeof(msg), "^g[REPLAY] ");
  }

  // no further modifications if timestamps are disabled,
  // or if this is the notify message
  if (!etj_drawMessageTime.integer || diffTime == -1) {
    Q_strcat(msg, sizeof(msg), text);
    return msg;
  }

  if (diffTime < 60) {
    Q_strcat(msg, sizeof(msg), stringFormat("^z[%is ago] ", diffTime).c_str());
  } else if (diffTime < 60 * 60) {
    const int min = diffTime / 60;
    Q_strcat(msg, sizeof(msg), stringFormat("^z[%im ago] ", min).c_str());
  } else {
    // up to 12h
    if (diffTime >= 60 * 60 * 12) {
      Q_strcat(msg, sizeof(msg), "^z[12h+ ago] ");
    } else {
      const int hours = diffTime / 3600;
      Q_strcat(msg, sizeof(msg), stringFormat("^z[%ih ago] ", hours).c_str());
    }
  }

  Q_strcat(msg, sizeof(msg), text);
  return msg;
}

static const char *addChatModifications(char *text, const int clientNum,
                                        const bool serverSay) {
  static char message[MAX_SAY_TEXT + 32];
  memset(message, 0, sizeof(message));

  if (etj_highlight.integer && (serverSay || cg.clientNum != clientNum) &&
      strstr(text + strlen(cgs.clientinfo[clientNum].name),
             cgs.clientinfo[cg.clientNum].name) != nullptr) {
    if (etj_highlight.integer &
        static_cast<int>(ChatHighlightFlags::HIGHLIGHT_BEEPER)) {
      Q_strcat(message, sizeof(message), etj_highlightText.string);
      Q_strcat(message, sizeof(message), "^7");

      trap_S_StartLocalSound(
          trap_S_RegisterSound(etj_highlightSound.string, qfalse), CHAN_LOCAL);
    }

    if (etj_highlight.integer &
        static_cast<int>(ChatHighlightFlags::HIGHLIGHT_FLASH)) {
      SyscallExt::trap_SysFlashWindowETLegacy(
          SyscallExt::FlashWindowState::SDL_FLASH_UNTIL_FOCUSED);
    }
  }

  if (etj_drawMessageTime.integer) {
    const char *msgColor =
        cg.clientNum == clientNum && !serverSay ? "^g" : "^z";

    time_t t = std::time(&t);
    const tm *lt = std::localtime(&t);

    if (etj_drawMessageTime.integer == 2) {
      Q_strcat(message, sizeof(message),
               va("%s[%02d:%02d:%02d]^7 ", msgColor, lt->tm_hour, lt->tm_min,
                  lt->tm_sec));
    } else {
      Q_strcat(message, sizeof(message),
               va("%s[%02d:%02d]^7 ", msgColor, lt->tm_hour, lt->tm_min));
    }
  }

  Q_strcat(message, sizeof(message), text);
  return message;
}

static void fixLinesEndingWithCaret(char *text, int size) {
  const size_t len = strlen(text);

  if (text[len - 1] != '^') {
    return;
  }

  // replace ^ with ^2
  text[size - 3] = '^';
  text[size - 2] = '2';
  text[size - 1] = 0;
}
} // namespace ETJump

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/

static void CG_ServerCommand(void) {
  const char *cmd;
  char text[MAX_SAY_TEXT];
  qboolean enc = qfalse; // used for enc_chat, enc_tchat

  cmd = CG_Argv(0);

  if (!cmd[0]) {
    // server claimed the command
    return;
  }

  if (!strcmp(cmd, "tinfo")) {
    CG_ParseTeamInfo();
    return;
  }
  if (!strcmp(cmd, "sc0")) {
    CG_ParseScore(TEAM_AXIS);
    return;
  } else if (!strcmp(cmd, "sc1")) {
    CG_ParseScore(TEAM_ALLIES);
    return;
  }

  if (!strcmp(cmd, "WeaponStats")) {
    int i, start = 1;

    for (i = 0; i < WP_NUM_WEAPONS; i++) {

      if (!BG_ValidStatWeapon((weapon_t)i)) {
        continue;
      }

      cgs.playerStats.weaponStats[i].kills = Q_atoi(CG_Argv(start++));
      cgs.playerStats.weaponStats[i].killedby = Q_atoi(CG_Argv(start++));
      cgs.playerStats.weaponStats[i].teamkills = Q_atoi(CG_Argv(start++));
    }

    cgs.playerStats.suicides = Q_atoi(CG_Argv(start++));

    for (i = 0; i < HR_NUM_HITREGIONS; i++) {
      cgs.playerStats.hitRegions[i] = Q_atoi(CG_Argv(start++));
    }

    cgs.numOIDtriggers = Q_atoi(CG_Argv(start++));

    for (i = 0; i < cgs.numOIDtriggers; i++) {
      cgs.playerStats.objectiveStats[i] = Q_atoi(CG_Argv(start++));
      cgs.teamobjectiveStats[i] = Q_atoi(CG_Argv(start++));
    }

    return;
  }

  if (!Q_stricmp(cmd, "hasTimerun")) {
    cg.hasTimerun = Q_atoi(CG_Argv(1)) ? qtrue : qfalse;
    return;
  }

  if (!Q_stricmpn(cmd, "cheatCvarsOff",
                  static_cast<int>(strlen("cheatCvarsOff")))) {
    const int flags = Q_atoi(CG_Argv(1));

    if (flags & static_cast<int>(ETJump::CheatCvarFlags::LookYaw)) {
      trap_SendConsoleCommand("set cl_freelook 1\n");
      trap_SendConsoleCommand("set cl_yawspeed 0\n");
    }

    if (flags & static_cast<int>(ETJump::CheatCvarFlags::PmoveFPS)) {
      trap_SendConsoleCommand("set pmove_fixed 1\n");
    }
    return;
  }

  if (!Q_stricmp(cmd, "cpm")) {
    CG_AddPMItem(PM_MESSAGE, CG_LocalizeServerCommand(CG_Argv(1)),
                 cgs.media.voiceChatShader);
    return;
  }

  // Banner Printing
  if (!Q_stricmp(cmd, "bp")) {
    CG_BannerPrint(CG_LocalizeServerCommand(CG_Argv(1)));
    return;
  }

  if (!Q_stricmp(cmd, "cp")) {
    // NERVE - SMF
    int args = trap_Argc();
    char *s;

    if (args >= 3) {
      s = CG_TranslateString(CG_Argv(1));

      if (args == 4) {
        s = va("%s%s", CG_Argv(3), s);
      }

      // OSP - for client logging
      if (cg_printObjectiveInfo.integer > 0 &&
          (args == 4 || Q_atoi(CG_Argv(2)) > 1)) {
        CG_Printf("[cgnotify]*** ^3INFO: ^5%s\n",
                  CG_LocalizeServerCommand(CG_Argv(1)));
      }
      CG_PriorityCenterPrint(s, SCREEN_HEIGHT - (SCREEN_HEIGHT * 0.20),
                             SMALLCHAR_WIDTH, Q_atoi(CG_Argv(2)));
    } else {
      CG_CenterPrint(CG_LocalizeServerCommand(CG_Argv(1)),
                     SCREEN_HEIGHT - (SCREEN_HEIGHT * 0.20),
                     SMALLCHAR_WIDTH); //----(SA)	modified
    }
    return;
  }

  if (!Q_stricmp(cmd, "sdbg")) {
    CG_StatsDebugAddText(CG_Argv(1));
    return;
  }

  if (!Q_stricmp(cmd, "cs")) {
    CG_ConfigStringModified();
    return;
  }

  if (!Q_stricmp(cmd, "print")) {
    CG_Printf("[cgnotify]%s", CG_LocalizeServerCommand(CG_Argv(1)));
    return;
  }

  if (!Q_stricmp(cmd, "entnfo")) {
    char buffer[16];
    int allied_number, axis_number;

    trap_Argv(1, buffer, 16);
    axis_number = Q_atoi(buffer);

    trap_Argv(2, buffer, 16);
    allied_number = Q_atoi(buffer);

    CG_ParseMapEntityInfo(axis_number, allied_number);

    return;
  }

  enc = !Q_stricmp(cmd, "enc_chat") ? qtrue : qfalse;
  if (!Q_stricmp(cmd, "chat") || enc) {
    int msgType = 0;

    if (Q_atoi(CG_Argv(4))) {
      msgType |= REPLAY_MSG;
    }

    const char *s = CG_Argv(2);

    if (s[0] == '\0') { // server sends empty clientNum
      msgType |= SERVER_MSG;
    }

    const int clientNum = Q_atoi(s);

    if (cg_teamChatsOnly.integer) {
      return;
    }

    if (Q_atoi(CG_Argv(3))) {
      s = CG_LocalizeServerCommand(CG_Argv(1));
    } else {
      s = CG_Argv(1);
    }

    Q_strncpyz(text, s, MAX_SAY_TEXT);

    if (enc) {
      CG_DecodeQP(text);
    }

    CG_RemoveChatEscapeChar(text);
    ETJump::fixLinesEndingWithCaret(text, MAX_SAY_TEXT);

    if (msgType & REPLAY_MSG) {
      s = ETJump::addChatReplayModifications(text);
    } else {
      s = ETJump::addChatModifications(text, clientNum, msgType);
    }

    CG_AddToTeamChat(s, clientNum, msgType);
    CG_Printf("%s\n", s);

    return;
  }

  enc = !Q_stricmp(cmd, "enc_tchat") ? qtrue : qfalse;
  if (!Q_stricmp(cmd, "tchat") || enc) {
    const char *s;

    if (Q_atoi(CG_Argv(3))) {
      s = CG_LocalizeServerCommand(CG_Argv(1));
    } else {
      s = CG_Argv(1);
    }

    Q_strncpyz(text, s, MAX_SAY_TEXT);
    if (enc) {
      CG_DecodeQP(text);
    }

    CG_RemoveChatEscapeChar(text);

    s = ETJump::addChatModifications(text, Q_atoi(CG_Argv(2)), false);
    Q_strncpyz(text, s, MAX_SAY_TEXT);

    ETJump::fixLinesEndingWithCaret(text, MAX_SAY_TEXT);
    CG_AddToTeamChat(text, Q_atoi(CG_Argv(2)), 0);
    CG_Printf("%s\n", text); // JPW NERVE

    return;
  }

  if (!Q_stricmp(cmd, "vchat")) {
    CG_VoiceChat(SAY_ALL); // NERVE - SMF - enabled support
    return;
  }

  if (!Q_stricmp(cmd, "vtchat")) {
    CG_VoiceChat(SAY_TEAM); // NERVE - SMF - enabled support
    return;
  }

  if (!Q_stricmp(cmd, "vbchat")) {
    CG_VoiceChat(SAY_BUDDY);
    return;
  }

  if (!Q_stricmp(cmd, "voted")) {
    cgs.votedYes = !Q_strncmp(CG_Argv(1), "y", 1);
    cgs.votedNo = !Q_strncmp(CG_Argv(1), "n", 1);
    return;
  }

  // DHM - Nerve :: Allow client to lodge a complaing
  if (!Q_stricmp(cmd, "complaint") && cgs.gamestate == GS_PLAYING) {
    cgs.complaintEndTime = cg.time + 20000;
    cgs.complaintClient = Q_atoi(CG_Argv(1));

    if (cgs.complaintClient < 0) {
      cgs.complaintEndTime = cg.time + 10000;
    }

    return;
  }
  // dhm

  if (!Q_stricmp(cmd, "map_restart")) {
    CG_MapRestart();
    return;
  }

  // OSP - match stats
  if (!Q_stricmp(cmd, "sc")) {
    CG_scores_cmd();
    return;
  }

  // OSP - weapon stats parsing
  if (!Q_stricmp(cmd, "ws")) {
    if (cgs.dumpStatsTime > cg.time) {
      CG_dumpStats();
    } else {
      CG_parseWeaponStats_cmd(CG_printConsoleString);
      cgs.dumpStatsTime = 0;
    }

    return;
  }
  if (!Q_stricmp(cmd, "wws")) {
    CG_wstatsParse_cmd();
    return;
  }
  if (!Q_stricmp(cmd, "gstats")) {
    CG_parseWeaponStatsGS_cmd();
    return;
  }

  // OSP - "topshots"-related commands
  if (!Q_stricmp(cmd, "astats")) {
    CG_parseTopShotsStats_cmd(qtrue, CG_printConsoleString);
    return;
  }
  if (!Q_stricmp(cmd, "astatsb")) {
    CG_parseTopShotsStats_cmd(qfalse, CG_printConsoleString);
    return;
  }
  if (!Q_stricmp(cmd, "bstats")) {
    CG_parseBestShotsStats_cmd(qtrue, CG_printConsoleString);
    return;
  }
  if (!Q_stricmp(cmd, "bstatsb")) {
    CG_parseBestShotsStats_cmd(qfalse, CG_printConsoleString);
    return;
  }
  //	if(!strcmp(cmd, "wastats")) {
  //		CG_wtopshotsParse_cmd(qfalse);
  //		return;
  //	}
  if (!Q_stricmp(cmd, "wbstats")) {
    CG_topshotsParse_cmd(qtrue);
    return;
  }

  // Gordon: single weapon stat (requested weapon stats)
  if (!Q_stricmp(cmd, "rws")) {
    CG_ParseWeaponStats();
    return;
  }
  if (!Q_stricmp(cmd, "portalcampos")) {
    CG_ParsePortalPos();
    return;
  }

  if (!Q_stricmp(cmd, "startCam")) {
    CG_StartCamera(CG_Argv(1), Q_atoi(CG_Argv(2)) ? qtrue : qfalse);
    return;
  }

  if (!Q_stricmp(cmd, "SetInitialCamera")) {
    CG_SetInitialCamera(CG_Argv(1), Q_atoi(CG_Argv(2)) ? qtrue : qfalse);
    return;
  }

  if (!Q_stricmp(cmd, "stopCam")) {
    CG_StopCamera();
    return;
  }

  if (!Q_stricmp(cmd, "setspawnpt")) {
    cg.selectedSpawnPoint = Q_atoi(CG_Argv(1)) + 1;
    return;
  }

  if (!strcmp(cmd,
              "rockandroll")) // map loaded, game is ready to begin.
  { // Arnout: FIXME: re-enable when we get menus that deal with fade
    // properly
    //		CG_Fade(0, 0, 0, 255, cg.time, 0);		// go
    // black
    // trap_UI_Popup("pregame");				// start
    // pregame menu trap_Cvar_Set("cg_norender", "1");	// don't
    // render the world until the player clicks in and the
    // 'playerstart' func has been called (g_main in
    // G_UpdateCvars() ~ilne 949)

    trap_S_FadeAllSound(1.0f, 1000, qfalse); // fade sound up

    return;
  }

  if (!Q_stricmp(cmd, "application")) {
    cgs.applicationEndTime = cg.time + 20000;
    cgs.applicationClient = Q_atoi(CG_Argv(1));

    if (cgs.applicationClient < 0) {
      cgs.applicationEndTime = cg.time + 10000;
    }

    return;
  }

  if (!Q_stricmp(cmd, "invitation")) {
    cgs.invitationEndTime = cg.time + 20000;
    cgs.invitationClient = Q_atoi(CG_Argv(1));

    if (cgs.invitationClient < 0) {
      cgs.invitationEndTime = cg.time + 10000;
    }

    return;
  }

  if (!Q_stricmp(cmd, "proposition")) {
    cgs.propositionEndTime = cg.time + 20000;
    cgs.propositionClient = Q_atoi(CG_Argv(1));
    cgs.propositionClient2 = Q_atoi(CG_Argv(2));

    if (cgs.propositionClient < 0) {
      cgs.propositionEndTime = cg.time + 10000;
    }

    return;
  }

  if (!Q_stricmp(cmd, "aft")) {
    cgs.autoFireteamEndTime = cg.time + 20000;
    cgs.autoFireteamNum = Q_atoi(CG_Argv(1));

    if (cgs.autoFireteamNum < -1) {
      cgs.autoFireteamEndTime = cg.time + 10000;
    }
    return;
  }

  if (!Q_stricmp(cmd, "aftc")) {
    cgs.autoFireteamCreateEndTime = cg.time;
    cgs.autoFireteamCreateNum = Q_atoi(CG_Argv(1));

    if (cgs.autoFireteamCreateNum < -1) {
      cgs.autoFireteamCreateEndTime = cg.time + 10000;
    }
    return;
  }

  if (!Q_stricmp(cmd, "aftj")) {
    cgs.autoFireteamJoinEndTime = cg.time + 20000;
    cgs.autoFireteamJoinNum = Q_atoi(CG_Argv(1));

    if (cgs.autoFireteamJoinNum < -1) {
      cgs.autoFireteamJoinEndTime = cg.time + 10000;
    }
    return;
  }

  if (Q_stricmp(cmd, "remapShader") == 0) {
    if (trap_Argc() == 4) {
      trap_R_RemapShader(CG_Argv(1), CG_Argv(2), CG_Argv(3));
    }
  }

  // GS Copied in code from old source for mu_start, mu_play & mu_stop
  //
  //  music
  //

  // loops \/
  if (!Q_stricmp(cmd,
                 "mu_start")) // has optional parameter for fade-up time
  {
    int fadeTime = 0; // default to instant start

    Q_strncpyz(text, CG_Argv(2), MAX_SAY_TEXT);
    if (strlen(text)) {
      fadeTime = Q_atoi(text);
    }

    trap_S_StartBackgroundTrack(CG_Argv(1), CG_Argv(1), fadeTime);
    return;
  }
  // plays once then back to whatever the loop was \/
  if (!Q_stricmp(cmd,
                 "mu_play")) // has optional parameter for fade-up time
  {
    int fadeTime = 0; // default to instant start

    Q_strncpyz(text, CG_Argv(2), MAX_SAY_TEXT);
    if (strlen(text)) {
      fadeTime = Q_atoi(text);
    }

    trap_S_StartBackgroundTrack(CG_Argv(1), "onetimeonly", fadeTime);
    return;
  }

  if (!Q_stricmp(cmd, "mu_stop")) // has optional parameter for fade-down time
  {
    int fadeTime = 0; // default to instant stop

    Q_strncpyz(text, CG_Argv(1), MAX_SAY_TEXT);
    if (strlen(text)) {
      fadeTime = Q_atoi(text);
    }

    trap_S_FadeBackgroundTrack(0.0f, fadeTime, 0);
    trap_S_StartBackgroundTrack("", "", -2); // '-2' for 'queue looping track'
                                             // (QUEUED_PLAY_LOOPED)
    return;
  }
  if (!Q_stricmp(cmd, "mu_fade")) {
    trap_S_FadeBackgroundTrack(Q_atof(CG_Argv(1)), Q_atoi(CG_Argv(2)), 0);
    return;
  }

  if (!Q_stricmp(cmd, "snd_fade")) {
    trap_S_FadeAllSound(Q_atof(CG_Argv(1)), Q_atoi(CG_Argv(2)),
                        Q_atoi(CG_Argv(3)) ? qtrue : qfalse);
    return;
  }

  if (!Q_stricmp(cmd, "ftCommands")) {
    char info[MAX_INFO_STRING];
    trap_Argv(1, info, sizeof(info));

    cg.botMenuIcons = Q_atoi(info);

    return;
  }

  if (!Q_stricmp(cmd, "manual")) {
    CG_Manual_f();
    return;
  }

  if (!Q_stricmp(cmd, "set_name")) {
    int argc, totlen, i, len;
    static char line[MAX_STRING_CHARS];
    char arg[MAX_STRING_CHARS];
    ;
    if (!CG_Argv(1)) {
      return;
    }

    len = 0;
    argc = trap_Argc();
    for (i = 1; i < argc; i++) {
      trap_Argv(i, arg, sizeof(arg));
      totlen = strlen(arg);
      if (len + totlen >= MAX_STRING_CHARS - 1) {
        break;
      }
      memcpy(line + len, arg, totlen);
      len += totlen;
      if (i != argc - 1) {
        line[len] = ' ';
        len++;
      }
    }

    line[len] = 0;

    trap_Cvar_Set("name", line);
    return;
  }

  // ensure a file gets into a build (mainly for scripted music calls)
  if (!Q_stricmp(cmd, "addToBuild")) {
    fileHandle_t f;

    if (!cg_buildScript.integer) {
      return;
    }

    // just open the file so it gets copied to the build dir
    // CG_FileTouchForBuild(CG_Argv(1));
    trap_FS_FOpenFile(CG_Argv(1), &f, FS_READ);
    trap_FS_FCloseFile(f);
    return;
  }

  // ydnar: bug 267: server sends this command when it's about to kill
  // the current server, before the client can reconnect
  if (!Q_stricmp(cmd, "spawnserver")) {
    // print message informing player the server is restarting
    // with a new map
    CG_PriorityCenterPrint(va("%s", CG_TranslateString("^3Server Restarting")),
                           SCREEN_HEIGHT - (SCREEN_HEIGHT * 0.25),
                           SMALLCHAR_WIDTH, 999999);

    // hack here
    cg.serverRespawning = qtrue;

    // fade out over the course of 5 seconds, should be enough
    // (nuking: atvi bug 3793)
    //%	CG_Fade( 0, 0, 0, 255, cg.time, 5000 );

    return;
  }

  std::vector<std::string> arguments;
  for (auto i = 1, argc = trap_Argc(); i < argc; ++i) {
    // Zero: CG_Argv cannot be used here as it uses a single
    // static buffer and breaks the CG_ServerCommandExt as cmd
    // will be replaced with whatever was the last argument
    char buf[MAX_TOKEN_CHARS]{};
    trap_Argv(i, buf, sizeof(buf));
    arguments.push_back(buf);
  }

  bool found = ETJump::serverCommandsHandler->check(cmd, arguments);

  if (CG_ServerCommandExt(cmd)) {
    return;
  }

  // shut up console when we send this over from save
  if (!Q_stricmp(cmd, "resetStrafeQuality")) {
    return;
  }

  if (!Q_stricmp(cmd, "savePrint")) {
    int pos = Q_atoi(CG_Argv(1));
    std::string saveMsg = etj_saveMsg.string;

    if (pos) {
      saveMsg += ' ' + std::to_string(pos);
    }
    if (trap_Argc() == 3) {
      int remainingSaves = Q_atoi(CG_Argv(2));
      std::string remainingSavesStr =
          va("^7(^3%d ^7remaining)\n", remainingSaves);
      saveMsg += '\n' + remainingSavesStr;
    }

    CG_CenterPrint(CG_LocalizeServerCommand(saveMsg.c_str()),
                   SCREEN_HEIGHT - SCREEN_HEIGHT * 0.2, SMALLCHAR_WIDTH, false);
    return;
  }

  if (!Q_stricmp(cmd, "openRtvMenu")) {
    trap_SendConsoleCommand("openRtvMenu");
    return;
  }

  // for !rtv admin command
  if (!Q_stricmp(cmd, "callvote")) {
    std::string command = va("%s %s", cmd, arguments[0].c_str());

    if (arguments.size() > 1) {
      command += " " + arguments[1];
    }

    trap_SendConsoleCommand(command.c_str());
    return;
  }

  if (found)
    return;

  CG_Printf("Unknown client game command: %s\n", cmd);
}

/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands(int latestSequence) {
  while (cgs.serverCommandSequence < latestSequence) {
    if (trap_GetServerCommand(++cgs.serverCommandSequence)) {
      CG_ServerCommand();
    }
  }
}
