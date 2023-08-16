#include "g_local.h"
#include "etj_numeric_utilities.h"
#include "etj_printer.h"

// Gordon
// What we need....
// invite <clientname|number>
// apply <fireteamname|number>
// create <name>
// leave

// player can only be on single fire team
// only leader can invite
// leaving a team causes the first person to join the team after the leader to
// become leader 32 char limit on fire team names, mebe reduce to 16..

// Application commad overview
//
// clientNum < 0 = special, otherwise is client that the command refers to
// -1 = Application sent
// -2 = Application Failed
// -3 = Application Approved
// -4 = Response sent

// Invitation commad overview
//
// clientNum < 0 = special, otherwise is client that the command refers to
// -1 = Invitation sent
// -2 = Invitation Rejected
// -3 = Invitation Accepted
// -4 = Response sent

// Proposition commad overview
//
// clientNum < 0 = special, otherwise is client that the command refers to
// -1 = Proposition sent
// -2 = Proposition Rejected
// -3 = Proposition Accepted
// -4 = Response sent

// Auto fireteam priv/pub
//
// -1 = ask
// -2 = confirmed
//

// Configstring for each fireteam "\\n\\%NAME%\\l\\%LEADERNUM%\\c\\%CLIENTS%"
// clients "compressed" using hex

#define G_ClientPrintAndReturn(entityNum, text)                                \
  trap_SendServerCommand(entityNum, "cpm \"" text "\"\n");                     \
  return;

// Utility functions
fireteamData_t *G_FindFreeFireteam() {
  int i;

  for (i = 0; i < MAX_FIRETEAMS; i++) {
    if (!level.fireTeams[i].inuse) {
      return &level.fireTeams[i];
    }
  }

  return NULL;
}

team_t G_GetFireteamTeam(fireteamData_t *ft) {
  if (!ft->inuse) {
    return static_cast<team_t>(-1);
  }

  if (ft->joinOrder[0] == -1 || !g_entities[(int)ft->joinOrder[0]].client) {
    G_Error("G_GetFireteamTeam: Fireteam leader is invalid\n");
  }

  return g_entities[(int)ft->joinOrder[0]].client->sess.sessionTeam;
}
// Confusing name, actually counts all fireteams
int G_CountTeamFireteams(team_t team) {
  int i, cnt = 0;

  for (i = 0; i < MAX_FIRETEAMS; i++) {
    // Zero: let's count all fireteams, not just team fireteams
    if (G_GetFireteamTeam(&level.fireTeams[i]) != -1) {
      cnt++;
    }
  }

  return cnt;
}

void G_UpdateFireteamConfigString(fireteamData_t *ft) {
  char buffer[128];
  int i;
  int clnts[2] = {0, 0};

  if (!ft->inuse) {
    Com_sprintf(buffer, 128, "\\id\\-1");
  } else {
    for (i = 0; i < MAX_CLIENTS; i++) {
      if (ft->joinOrder[i] != -1) {
        COM_BitSet(clnts, ft->joinOrder[i]);
      }
    }

    Com_sprintf(buffer, 128, "\\id\\%i\\l\\%i\\sl\\%i\\c\\%.8x%.8x",
                ft->ident - 1, ft->joinOrder[0], ft->saveLimit, clnts[1],
                clnts[0]);
    // G_Printf(va("%s\n", buffer));
  }

  trap_SetConfigstring(CS_FIRETEAMS + (ft - level.fireTeams), buffer);
}

qboolean G_IsOnFireteam(int entityNum, fireteamData_t **teamNum) {
  int i, j;

  if ((entityNum < 0 || entityNum >= MAX_CLIENTS) ||
      !g_entities[entityNum].client) {
    G_Error("G_IsOnFireteam: invalid client");
  }

  for (i = 0; i < MAX_FIRETEAMS; i++) {
    if (!level.fireTeams[i].inuse) {
      continue;
    }

    for (j = 0; j < MAX_CLIENTS; j++) {
      if (level.fireTeams[i].joinOrder[j] == -1) {
        break;
      }

      if (level.fireTeams[i].joinOrder[j] == entityNum) {
        if (teamNum) {
          *teamNum = &level.fireTeams[i];
        }
        return qtrue;
      }
    }
  }

  if (teamNum) {
    *teamNum = NULL;
  }
  return qfalse;
}

qboolean G_IsFireteamLeader(int entityNum, fireteamData_t **teamNum) {
  int i;

  if ((entityNum < 0 || entityNum >= MAX_CLIENTS) ||
      !g_entities[entityNum].client) {
    G_Error("G_IsFireteamLeader: invalid client");
  }

  for (i = 0; i < MAX_FIRETEAMS; i++) {
    if (!level.fireTeams[i].inuse) {
      continue;
    }

    if (level.fireTeams[i].joinOrder[0] == entityNum) {
      if (teamNum) {
        *teamNum = &level.fireTeams[i];
      }
      return qtrue;
    }
  }

  if (teamNum) {
    *teamNum = nullptr;
  }
  return qfalse;
}

int G_FindFreeFireteamIdent(team_t team) {
  bool freeIdent[MAX_FIRETEAMS];
  int i;

  // this was memset, which is wrong since it works on bytes
  // we need to set all elements to true initially instead
  std::fill_n(freeIdent, MAX_FIRETEAMS, true);

  for (i = 0; i < MAX_FIRETEAMS; i++) {
    if (!level.fireTeams[i].inuse) {
      continue;
    }
    // Set every team that is inuse not free
    freeIdent[level.fireTeams[i].ident - 1] = false;
  }

  for (i = 0; i < (MAX_FIRETEAMS); i++) {
    if (freeIdent[i]) {
      return i;
    }
  }

  // Gordon: this should never happen
  return -1;
}

// Should be the only function that ever creates a fireteam
void G_RegisterFireteam(int entityNum) {
  fireteamData_t *ft;
  gentity_t *leader;
  int count, ident;

  if (entityNum < 0 || entityNum >= MAX_CLIENTS) {
    G_Error("G_RegisterFireteam: invalid client");
  }

  leader = &g_entities[entityNum];
  if (!leader->client) {
    G_Error("G_RegisterFireteam: attempting to register a "
            "Fireteam to an "
            "entity with no client\n");
  }

  if (G_IsOnFireteam(entityNum, nullptr)) {
    G_ClientPrintAndReturn(entityNum,
                           "You are already on a fireteam, leave it first");
  }

  if ((ft = G_FindFreeFireteam()) == nullptr) {
    G_ClientPrintAndReturn(entityNum, "No free fireteams available");
  }

  count = G_CountTeamFireteams(leader->client->sess.sessionTeam);
  if (count >= MAX_FIRETEAMS) {
    G_ClientPrintAndReturn(entityNum,
                           "There is a maximum number of fireteams in use.");
  }

  ident = G_FindFreeFireteamIdent(leader->client->sess.sessionTeam) + 1;
  if (ident == 0) {
    G_ClientPrintAndReturn(entityNum, "Um, something is broken, spoink Gordon");
  }

  // good to go now, i hope!
  ft->inuse = qtrue;
  memset(ft->joinOrder, -1, sizeof(level.fireTeams[0].joinOrder));
  ft->joinOrder[0] = static_cast<char>(leader - g_entities);
  ft->ident = ident;
  ft->saveLimit = FT_SAVELIMIT_NOT_SET;
  ft->teamJumpMode = qfalse;

  if (g_autoFireteams.integer) {
    ft->priv = qfalse;

    trap_SendServerCommand(entityNum, "aft -1");
    leader->client->pers.autofireteamEndTime = level.time + 20500;
  } else {
    ft->priv = qfalse;
  }

  G_UpdateFireteamConfigString(ft);
}

// only way a client should ever join a fireteam, other than creating one
void G_AddClientToFireteam(int entityNum, int leaderNum) {
  fireteamData_t *ft;
  int i;

  if ((entityNum < 0 || entityNum >= MAX_CLIENTS) ||
      !g_entities[entityNum].client) {
    G_Error("G_AddClientToFireteam: invalid client");
  }

  if ((leaderNum < 0 || leaderNum >= MAX_CLIENTS) ||
      !g_entities[leaderNum].client) {
    G_Error("G_AddClientToFireteam: invalid client");
  }

  if (!G_IsFireteamLeader(leaderNum, &ft)) {
    G_ClientPrintAndReturn(
        entityNum, "The leader has now left the Fireteam you applied to");
  }

  if (G_IsOnFireteam(entityNum, NULL)) {
    G_ClientPrintAndReturn(entityNum, "You are already on a fireteam");
  }

  for (i = 0; i < MAX_CLIENTS; i++) {
    // Zero: changed to 20 instead of 6
    if (i >= MAX_FIRETEAM_USERS) {
      G_ClientPrintAndReturn(entityNum,
                             "Too many players already on this Fireteam");
      return;
    }

    if (ft->joinOrder[i] == -1) {
      gentity_t *otherEnt = g_entities + entityNum;
      // found a free position
      ft->joinOrder[i] = static_cast<char>(entityNum);

      otherEnt->client->sess.saveLimitFt = ft->saveLimit;

      G_UpdateFireteamConfigString(ft);

      return;
    }
  }
}

// The only way a client should be removed from a fireteam
void G_RemoveClientFromFireteams(int entityNum, qboolean update,
                                 qboolean print) {
  fireteamData_t *ft;
  int i, j;

  if ((entityNum < 0 || entityNum >= MAX_CLIENTS) ||
      !g_entities[entityNum].client) {
    G_Error("G_RemoveClientFromFireteams: invalid client");
  }

  if (G_IsOnFireteam(entityNum, &ft)) {
    for (i = 0; i < MAX_CLIENTS; i++) {
      if (ft->joinOrder[i] == entityNum) {
        if (i == 0) {
          if (ft->joinOrder[1] == -1) {
            ft->inuse = qfalse;
            ft->ident = -1;
          } else {
            // TODO: Inform
            // client of
            // promotion to
            // leader
          }
        }
        for (j = i; j < MAX_CLIENTS - 1; j++) {
          ft->joinOrder[j] = ft->joinOrder[j + 1];
        }
        ft->joinOrder[MAX_CLIENTS - 1] = -1;

        // invalidate "Make fireteam private?"
        // prompt response in case we joined a
        // fireteam and left without responding
        g_entities[entityNum].client->pers.autofireteamEndTime = level.time;

        break;
      }
    }
  } else {
    return;
  }

  if (ft->joinOrder[0] != -1) {
    if (g_entities[(int)ft->joinOrder[0]].r.svFlags & SVF_BOT) {
      G_RemoveClientFromFireteams(ft->joinOrder[0], qfalse, qfalse);
    }
  }

  if (print) {
    for (i = 0; i < MAX_CLIENTS; i++) {
      if (ft->joinOrder[i] == -1) {
        break;
      }

      trap_SendServerCommand(ft->joinOrder[i],
                             va("cpm \"%s ^7has left the Fireteam\"\n",
                                level.clients[entityNum].pers.netname));
    }
  }

  // if the leader leaves it seems the savelimit will be set to
  // 2^32-1 / -2^32-1 so this should fix it.
  if (ft->saveLimit != FT_SAVELIMIT_NOT_SET) {
    gentity_t *ent;
    for (i = 0; i < level.numConnectedClients; i++) {
      if (ft->joinOrder[i] == -1) {
        continue;
      } else {
        ent = g_entities + ft->joinOrder[i];
        ent->client->sess.saveLimitFt = ft->saveLimit;
      }
    }
  }

  if (update) {
    G_UpdateFireteamConfigString(ft);
  }
}

// The only way a client should ever be invitied to join a team
void G_InviteToFireTeam(int entityNum, int otherEntityNum) {
  fireteamData_t *ft;

  if ((entityNum < 0 || entityNum >= MAX_CLIENTS) ||
      !g_entities[entityNum].client) {
    G_Error("G_InviteToFireTeam: invalid client");
  }

  if ((otherEntityNum < 0 || otherEntityNum >= MAX_CLIENTS) ||
      !g_entities[otherEntityNum].client) {
    G_Error("G_InviteToFireTeam: invalid client");
  }

  if (!G_IsFireteamLeader(entityNum, &ft)) {
    G_ClientPrintAndReturn(entityNum, "You are not the leader of a fireteam");
  }

  if (G_IsOnFireteam(otherEntityNum, NULL)) {
    G_ClientPrintAndReturn(entityNum,
                           "The other player is already on a fireteam");
  }

  if (g_entities[otherEntityNum].r.svFlags & SVF_BOT) {
    // Gordon: bots auto join
    G_AddClientToFireteam(otherEntityNum, entityNum);
  } else {
    trap_SendServerCommand(entityNum, va("invitation -1"));
    trap_SendServerCommand(otherEntityNum, va("invitation %i", entityNum));
    g_entities[otherEntityNum].client->pers.invitationClient = entityNum;
    g_entities[otherEntityNum].client->pers.invitationEndTime =
        level.time + 20500;
  }
}

void G_DestroyFireteam(int entityNum) {
  fireteamData_t *ft;

  if ((entityNum < 0 || entityNum >= MAX_CLIENTS) ||
      !g_entities[entityNum].client) {
    G_Error("G_DestroyFireteam: invalid client");
  }

  if (!G_IsFireteamLeader(entityNum, &ft)) {
    G_ClientPrintAndReturn(entityNum, "You are not the leader of a fireteam");
  }

  while (ft->joinOrder[0] != -1) {
    if (ft->joinOrder[0] != entityNum) {
      trap_SendServerCommand(ft->joinOrder[0],
                             "cpm \"The Fireteam you are on has been "
                             "disbanded\"\n");
    }

    G_RemoveClientFromFireteams(ft->joinOrder[0], qfalse, qfalse);
  }

  G_UpdateFireteamConfigString(ft);
}

void G_WarnFireTeamPlayer(int entityNum, int otherEntityNum) {
  fireteamData_t *ft, *ft2;

  if (entityNum == otherEntityNum) {
    return; // ok, stop being silly :p
  }

  if ((entityNum < 0 || entityNum >= MAX_CLIENTS) ||
      !g_entities[entityNum].client) {
    G_Error("G_WarnFireTeamPlayer: invalid client");
  }

  if ((otherEntityNum < 0 || otherEntityNum >= MAX_CLIENTS) ||
      !g_entities[otherEntityNum].client) {
    G_Error("G_WarnFireTeamPlayer: invalid client");
  }

  if (!G_IsFireteamLeader(entityNum, &ft)) {
    G_ClientPrintAndReturn(entityNum, "You are not the leader of a fireteam");
  }

  if ((!G_IsOnFireteam(otherEntityNum, &ft2)) || ft != ft2) {
    G_ClientPrintAndReturn(
        entityNum, "You are not on the same Fireteam as the other player");
  }

  trap_SendServerCommand(
      otherEntityNum,
      "cpm \"You have been warned by your Fireteam Commander\n\"");
}

void G_KickFireTeamPlayer(int entityNum, int otherEntityNum) {
  fireteamData_t *ft, *ft2;
  ;

  if (entityNum == otherEntityNum) {
    return; // ok, stop being silly :p
  }

  if ((entityNum < 0 || entityNum >= MAX_CLIENTS) ||
      !g_entities[entityNum].client) {
    G_Error("G_KickFireTeamPlayer: invalid client");
  }

  if ((otherEntityNum < 0 || otherEntityNum >= MAX_CLIENTS) ||
      !g_entities[otherEntityNum].client) {
    G_Error("G_KickFireTeamPlayer: invalid client");
  }

  if (!G_IsFireteamLeader(entityNum, &ft)) {
    G_ClientPrintAndReturn(entityNum, "You are not the leader of a fireteam");
  }

  if ((!G_IsOnFireteam(otherEntityNum, &ft2)) || ft != ft2) {
    G_ClientPrintAndReturn(
        entityNum, "You are not on the same Fireteam as the other player");
  }

  G_RemoveClientFromFireteams(otherEntityNum, qtrue, qfalse);

  G_ClientPrintAndReturn(otherEntityNum,
                         "You have been kicked from the fireteam");
}

// The only way a client should ever apply to join a team
void G_ApplyToFireTeam(int entityNum, int fireteamNum) {
  gentity_t *leader;
  fireteamData_t *ft;

  if ((entityNum < 0 || entityNum >= MAX_CLIENTS) ||
      !g_entities[entityNum].client) {
    G_Error("G_AddClientToFireteam: invalid client");
  }

  if (G_IsOnFireteam(entityNum, NULL)) {
    G_ClientPrintAndReturn(entityNum, "You are already on a fireteam");
  }

  ft = &level.fireTeams[fireteamNum];
  if (!ft->inuse) {
    G_ClientPrintAndReturn(entityNum,
                           "The Fireteam you requested does not exist");
  }

  if (ft->joinOrder[0] < 0 || ft->joinOrder[0] >= MAX_CLIENTS) {
    G_Error("G_ApplyToFireTeam: Fireteam leader is invalid\n");
  }

  leader = &g_entities[(int)ft->joinOrder[0]];
  if (!leader->client) {
    G_Error("G_ApplyToFireTeam: Fireteam leader client is NULL\n");
  }

  // TEMP
  //	G_AddClientToFireteam( entityNum, ft->joinOrder[0] );

  trap_SendServerCommand(entityNum, va("application -1"));
  trap_SendServerCommand(leader - g_entities, va("application %i", entityNum));
  leader->client->pers.applicationClient = entityNum;
  leader->client->pers.applicationEndTime = level.time + 20000;
}

void G_ProposeFireTeamPlayer(int entityNum, int otherEntityNum) {
  fireteamData_t *ft;
  gentity_t *leader;

  if (entityNum == otherEntityNum) {
    return; // ok, stop being silly :p
  }

  if ((entityNum < 0 || entityNum >= MAX_CLIENTS) ||
      !g_entities[entityNum].client) {
    G_Error("G_ProposeFireTeamPlayer: invalid client");
  }

  if ((otherEntityNum < 0 || otherEntityNum >= MAX_CLIENTS) ||
      !g_entities[otherEntityNum].client) {
    G_Error("G_ProposeFireTeamPlayer: invalid client");
  }

  if (G_IsOnFireteam(otherEntityNum, NULL)) {
    G_ClientPrintAndReturn(entityNum,
                           "The other player is already on a fireteam");
  }

  if (!G_IsOnFireteam(entityNum, &ft)) {
    G_ClientPrintAndReturn(entityNum, "You are not on a fireteam");
  }

  if (ft->joinOrder[0] == entityNum) {
    // you are the leader so just invite them
    G_InviteToFireTeam(entityNum, otherEntityNum);
    return;
  }

  leader = &g_entities[(int)ft->joinOrder[0]];
  if (!leader->client) {
    G_Error("G_ProposeFireTeamPlayer: invalid client");
  }

  trap_SendServerCommand(entityNum, va("proposition -1"));
  trap_SendServerCommand(leader - g_entities,
                         va("proposition %i %i", otherEntityNum, entityNum));
  leader->client->pers.propositionClient = otherEntityNum;
  leader->client->pers.propositionClient2 = entityNum;
  leader->client->pers.propositionEndTime = level.time + 20000;
}

int G_FireteamNumberForString(const char *name, team_t team) {
  int i, fireteam = 0;

  for (i = 0; i < MAX_FIRETEAMS; i++) {
    if (!level.fireTeams[i].inuse) {
      continue;
    }

    if (!Q_stricmp(bg_fireteamNames[level.fireTeams[i].ident - 1], name)) {
      fireteam = i + 1;
    }

    /*		if(!Q_stricmp(level.fireTeams[i].name, name)) {
                fireteam = i+1;
            }*/
  }

  if (fireteam <= 0) {
    fireteam = Q_atoi(name);
  }

  return fireteam;
}

fireteamData_t *G_FindFreePublicFireteam(team_t team) {
  int i, j;

  for (i = 0; i < MAX_FIRETEAMS; i++) {
    if (!level.fireTeams[i].inuse) {
      continue;
    }

    if (level.fireTeams[i].priv) {
      continue;
    }

    for (j = 0; j < MAX_CLIENTS; j++) {
      if (j >= MAX_FIRETEAM_USERS || level.fireTeams[i].joinOrder[j] == -1) {
        break;
      }
    }
    if (j >= MAX_FIRETEAM_USERS) {
      continue;
    }

    return &level.fireTeams[i];
  }

  return NULL;
}

void G_TeamJumpMode(int clientNum) {
  int i = 0;
  char buf[MAX_TOKEN_CHARS] = "\0";
  char arg[MAX_TOKEN_CHARS] = "\0";
  fireteamData_t *ft = NULL;
  qboolean printChanges = qtrue;

  if (!G_IsOnFireteam(clientNum, &ft)) {
    G_ClientPrintAndReturn(clientNum, "You are not on a fireteam");
  }

  if (!G_IsFireteamLeader(clientNum, &ft)) {
    G_ClientPrintAndReturn(clientNum, "You are not the leader.");
  }

  if (trap_Argc() != 3) {
    G_ClientPrintAndReturn(clientNum, "usage: fireteam tj on");
  }

  trap_Argv(2, arg, sizeof(arg));

  if (!Q_stricmp(arg, "on")) {
    if (ft->teamJumpMode == qtrue) {
      printChanges = qfalse;
    } else {
      ft->teamJumpMode = qtrue;
      Com_sprintf(buf, sizeof(buf), "chat \"Fireteam: team jump mode ^2ON\"");
    }
  } else if (!Q_stricmp(arg, "off")) {
    if (ft->teamJumpMode == qfalse) {
      printChanges = qfalse;
    } else {
      ft->teamJumpMode = qfalse;
      Com_sprintf(buf, sizeof(buf), "chat \"Fireteam: team jump mode ^1OFF\"");
    }
  }

  for (i = 0; i < level.numConnectedClients; i++) {
    int cnum = level.sortedClients[i];
    fireteamData_t *ft2 = NULL;

    if (!G_IsOnFireteam(cnum, &ft2)) {
      continue;
    }

    if (ft != ft2) {
      continue;
    } else {
      if (printChanges) {
        trap_SendServerCommand(cnum, buf);
      }
    }
  }
}

void G_FireteamRace(int clientNum) {
  fireteamData_t *ft;
  if (!G_IsOnFireteam(clientNum, &ft)) {
    G_ClientPrintAndReturn(clientNum, "You are not on a fireteam");
  }

  if (!G_IsFireteamLeader(clientNum, &ft)) {
    G_ClientPrintAndReturn(clientNum, "You are not the leader.");
  }

  if (trap_Argc() < 3) {
    G_ClientPrintAndReturn(clientNum, "usage: fireteam race start");
  }

  G_ClientPrintAndReturn(clientNum, "Fireteam races are not supported yet.");

  //	trap_Argv(2, arg, sizeof(arg));
  //	if (!Q_stricmp(arg, "start"))
  //	{
  //		StartRace(g_entities + clientNum);
  //	}
}

void G_SetFireTeamSaveLimit(fireteamData_t *ft, int limit) {
  gentity_t *ent;

  for (int i = 0; i < level.numConnectedClients; i++) {
    if (ft->joinOrder[i] == -1) {
      continue;
    } else {
      ent = g_entities + ft->joinOrder[i];
      ent->client->sess.saveLimitFt = limit;
      Printer::SendPopupMessage(
          ClientNum(ent),
          va("^gFireteam rules: ^3savelimit ^gwas set to ^3%i^g.\n", limit));
    }
  }
}

void G_SetFireTeamRules(int clientNum) {
  char arg1[MAX_TOKEN_CHARS];
  char val[MAX_TOKEN_CHARS];
  fireteamData_t *ft;

  if (!G_IsOnFireteam(clientNum, &ft)) {
    G_ClientPrintAndReturn(clientNum, "You are not on a fireteam");
  }

  if (!G_IsFireteamLeader(clientNum, &ft)) {
    G_ClientPrintAndReturn(clientNum, "You are not the leader.");
  }

  if (trap_Argc() < 3) {
    G_ClientPrintAndReturn(clientNum, "usage: fireteam rules <savelimit|reset> "
                                      "<[optional] value>");
  }

  if (trap_Argc() == 3) {
    G_ClientPrintAndReturn(clientNum, "rules: savelimit.");
  }

  trap_Argv(2, arg1, sizeof(arg1));

  if (!Q_stricmp(arg1, "savelimit")) {
    if (level.limitedSaves > 0) {
      G_ClientPrintAndReturn(clientNum,
                             "fireteam: unable to set savelimit - save is "
                             "limited globally.");
    }

    trap_Argv(3, val, sizeof(val));

    if (!Q_stricmp(val, "reset")) {
      G_SetFireTeamSaveLimit(ft, ft->saveLimit);
      return;
    }

    int limit = Numeric::clamp(Q_atoi(val), -1, 100);
    ft->saveLimit = limit;
    G_SetFireTeamSaveLimit(ft, limit);

    G_UpdateFireteamConfigString(ft);
    return;
  }

  G_ClientPrintAndReturn(clientNum, "fireteam: failed to set rules.");
}

// Checks if given command buffer matches a valid client on the server
// Returns true if given argument matches a valid client
bool validClientForFireteam(gentity_t *ent, int *targetNum, char *numbuffer) {
  bool validClient = true;

  if ((*targetNum = ClientNumberFromString(ent, numbuffer)) == -1) {
    validClient = false;
  } else {
    gentity_t *other = g_entities + *targetNum;

    if (!other->inuse || !other->client) {
      validClient = false;
    }
  }

  return validClient;
}

// Command handler
void Cmd_FireTeam_MP_f(gentity_t *ent) {
  char command[MAX_NAME_LENGTH]; // more than enough to hold the commands
  auto selfNum = ClientNum(ent);
  int targetNum = -1;

  if (trap_Argc() < 2) {
    G_ClientPrintAndReturn(selfNum,
                           "usage: fireteam <create|leave|apply|invite|rules>");
  }

  trap_Argv(1, command, sizeof(command));

  if (!Q_stricmp(command, "create")) {
    G_RegisterFireteam(selfNum);
  } else if (!Q_stricmp(command, "disband")) {
    G_DestroyFireteam(selfNum);
  } else if (!Q_stricmp(command, "leave")) {
    G_RemoveClientFromFireteams(selfNum, qtrue, qtrue);
  } else if (!Q_stricmp(command, "apply")) {
    char namebuffer[MAX_NAME_LENGTH];
    int fireteam;

    if (trap_Argc() < 3) {
      G_ClientPrintAndReturn(
          selfNum, "usage: fireteam apply <fireteamname|fireteamnumber>");
    }

    trap_Argv(2, namebuffer, sizeof(namebuffer));
    fireteam =
        G_FireteamNumberForString(namebuffer, ent->client->sess.sessionTeam);

    if (fireteam <= 0) {
      G_ClientPrintAndReturn(
          selfNum, "usage: fireteam apply <fireteamname|fireteamnumber>");
    }

    G_ApplyToFireTeam(selfNum, fireteam - 1);
  } else if (!Q_stricmp(command, "invite")) {
    char numbuffer[MAX_NAME_LENGTH];

    if (trap_Argc() < 3) {
      G_ClientPrintAndReturn(
          selfNum, "usage: fireteam invite <clientname|clientnumber>");
    }
    trap_Argv(2, numbuffer, sizeof(numbuffer));

    targetNum = Q_atoi(numbuffer);
    if (!validClientForFireteam(ent, &targetNum, numbuffer)) {
      G_ClientPrintAndReturn(selfNum, "Invalid client selected");
    }

    G_InviteToFireTeam(selfNum, targetNum);
  } else if (!Q_stricmp(command, "warn")) {
    char numbuffer[MAX_NAME_LENGTH];

    if (trap_Argc() < 3) {
      G_ClientPrintAndReturn(selfNum,
                             "usage: fireteam warn <clientname|clientnumber>");
    }
    trap_Argv(2, numbuffer, sizeof(numbuffer));

    targetNum = Q_atoi(numbuffer);
    if (!validClientForFireteam(ent, &targetNum, numbuffer)) {
      G_ClientPrintAndReturn(selfNum, "Invalid client selected");
    }

    G_WarnFireTeamPlayer(selfNum, targetNum);
  } else if (!Q_stricmp(command, "kick")) {
    char numbuffer[MAX_NAME_LENGTH];

    if (trap_Argc() < 3) {
      G_ClientPrintAndReturn(selfNum,
                             "usage: fireteam kick <clientname|clientnumber>");
    }
    trap_Argv(2, numbuffer, sizeof(numbuffer));

    targetNum = Q_atoi(numbuffer);
    if (!validClientForFireteam(ent, &targetNum, numbuffer)) {
      G_ClientPrintAndReturn(selfNum, "Invalid client selected");
    }

    G_KickFireTeamPlayer(selfNum, targetNum);
  } else if (!Q_stricmp(command, "propose")) {
    char numbuffer[MAX_NAME_LENGTH];

    if (trap_Argc() < 3) {
      G_ClientPrintAndReturn(
          selfNum, "usage: fireteam propose <clientname|clientnumber>");
    }
    trap_Argv(2, numbuffer, sizeof(numbuffer));

    targetNum = Q_atoi(numbuffer);
    if (!validClientForFireteam(ent, &targetNum, numbuffer)) {
      G_ClientPrintAndReturn(selfNum, "Invalid client selected");
    }

    G_ProposeFireTeamPlayer(selfNum, targetNum);
  }
  // Challenge group.
  // Only leader
  else if (!Q_stricmp(command, "rules")) {
    G_SetFireTeamRules(selfNum);
  } else if (!Q_stricmp(command, "tj")) {
    G_TeamJumpMode(selfNum);
  } else if (!Q_stricmp(command, "race")) {
    G_FireteamRace(selfNum);
  }
}
