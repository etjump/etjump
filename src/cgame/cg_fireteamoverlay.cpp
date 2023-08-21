/******************************************************************************
***** teh firetams! (sic)
****/

#include "cg_local.h"
#include "../game/etj_numeric_utilities.h"
#include "../game/etj_string_utilities.h"

/******************************************************************************
***** Defines, constants, etc
****/

static int sortedFireTeamClients[MAX_CLIENTS];

/******************************************************************************
***** Support Routines
****/

int QDECL CG_SortFireTeam(const void *a, const void *b) {
  clientInfo_t *ca, *cb;
  int cna, cnb;

  cna = *(const int *)a;
  cnb = *(const int *)b;

  ca = &cgs.clientinfo[cna];
  cb = &cgs.clientinfo[cnb];

  // Not on our team, so shove back
  if (!CG_IsOnSameFireteam(cnb, cg.clientNum)) {
    return -1;
  }
  if (!CG_IsOnSameFireteam(cna, cg.clientNum)) {
    return 1;
  }

  // Leader comes first
  if (CG_IsFireTeamLeader(cna)) {
    return -1;
  }
  if (CG_IsFireTeamLeader(cnb)) {
    return 1;
  }

  // Then higher ranks
  if (ca->rank > cb->rank) {
    return -1;
  }
  if (cb->rank > ca->rank) {
    return 1;
  }

  // Then score
  /*	if ( ca->score > cb->score ) {
          return -1;
      }
      if ( cb->score > ca->score ) {
          return 1;
      }*/// not atm

  return 0;
}

// Sorts client's fireteam by leader then rank
void CG_SortClientFireteam() {
  int i;

  for (i = 0; i < MAX_CLIENTS; i++) {
    sortedFireTeamClients[i] = i;
  }

  qsort(sortedFireTeamClients, MAX_CLIENTS, sizeof(sortedFireTeamClients[0]),
        CG_SortFireTeam);

  /*	for(i = 0; i < MAX_CLIENTS; i++) {
          CG_Printf( "%i ", sortedFireTeamClients[i] );
      }

      CG_Printf( "\n" );*/
}

// Parses fireteam servercommand
void CG_ParseFireteams() {
  int i, j;
  const char *s;
  const char *p;
  int clnts[2];
  unsigned int tmp;

  for (i = 0; i < MAX_CLIENTS; i++) {
    cgs.clientinfo[i].fireteamData = NULL;
  }

  for (i = 0; i < MAX_FIRETEAMS; i++) {
    char hexbuffer[11] = "0x00000000";
    p = CG_ConfigString(CS_FIRETEAMS + i);

    j = Q_atoi(Info_ValueForKey(p, "id"));
    if (j == -1) {
      cg.fireTeams[i].inuse = qfalse;
      continue;
    } else {
      cg.fireTeams[i].inuse = qtrue;
      cg.fireTeams[i].ident = j;
    }

    s = Info_ValueForKey(p, "l");
    cg.fireTeams[i].leader = Q_atoi(s);

    s = Info_ValueForKey(p, "sl");
    cg.fireTeams[i].saveLimit = Q_atoi(s);

    s = Info_ValueForKey(p, "c");
    Q_strncpyz(hexbuffer + 2, s, 9);
    sscanf(hexbuffer, "%x", &tmp);
    clnts[1] = static_cast<int>(tmp);
    Q_strncpyz(hexbuffer + 2, s + 8, 9);
    sscanf(hexbuffer, "%x", &tmp);
    clnts[0] = static_cast<int>(tmp);

    for (j = 0; j < MAX_CLIENTS; j++) {
      if (COM_BitCheck(clnts, j)) {
        cg.fireTeams[i].joinOrder[j] = qtrue;
        cgs.clientinfo[j].fireteamData = &cg.fireTeams[i];
      } else {
        cg.fireTeams[i].joinOrder[j] = qfalse;
      }
    }
  }

  CG_SortClientFireteam();
}

// Fireteam that both specified clients are on, if they both are on the same
// team
fireteamData_t *CG_IsOnSameFireteam(int clientNum, int clientNum2) {
  if (CG_IsOnFireteam(clientNum) == CG_IsOnFireteam(clientNum2)) {
    return CG_IsOnFireteam(clientNum);
  }

  return NULL;
}

// Fireteam that specified client is leader of, or NULL if none
fireteamData_t *CG_IsFireTeamLeader(int clientNum) {
  fireteamData_t *f;

  f = CG_IsOnFireteam(clientNum);
  if (!f) {
    return NULL;
  }

  if (f->leader != clientNum) {
    return NULL;
  }

  return f;
}

// Client, not on a fireteam, not sorted, but on your team
clientInfo_t *CG_ClientInfoForPosition(int pos, int max) {
  int i, cnt = 0;

  for (i = 0; i < MAX_CLIENTS && cnt < max; i++) {
    if (cg.clientNum != i && cgs.clientinfo[i].infoValid &&
        !CG_IsOnFireteam(i) &&
        cgs.clientinfo[cg.clientNum].team == cgs.clientinfo[i].team) {
      if (cnt == pos) {
        return &cgs.clientinfo[i];
      }
      cnt++;
    }
  }

  return NULL;
}

// Fireteam, that's on your same team
fireteamData_t *CG_FireTeamForPosition(int pos, int max) {
  int i, cnt = 0;

  for (i = 0; i < MAX_FIRETEAMS && cnt < max; i++) {
    if (cg.fireTeams[i].inuse && cgs.clientinfo[cg.fireTeams[i].leader].team ==
                                     cgs.clientinfo[cg.clientNum].team) {
      if (cnt == pos) {
        return &cg.fireTeams[i];
      }
      cnt++;
    }
  }

  return NULL;
}

// Client, not sorted by rank, on CLIENT'S fireteam
clientInfo_t *CG_FireTeamPlayerForPosition(int pos, int max) {
  int i, cnt = 0;
  fireteamData_t *f = CG_IsOnFireteam(cg.clientNum);

  if (!f) {
    return NULL;
  }

  for (i = 0; i < MAX_CLIENTS && cnt < max; i++) {
    if (cgs.clientinfo[i].infoValid &&
        cgs.clientinfo[cg.clientNum].team == cgs.clientinfo[i].team) {
      if (!(f == CG_IsOnFireteam(i))) {
        continue;
      }

      if (cnt == pos) {
        return &cgs.clientinfo[i];
      }
      cnt++;
    }
  }

  return NULL;
}

// Client, sorted by rank, on CLIENT'S fireteam
clientInfo_t *CG_SortedFireTeamPlayerForPosition(int pos, int max) {
  int i, cnt = 0;
  fireteamData_t *f = CG_IsOnFireteam(cg.clientNum);

  if (!f) {
    return NULL;
  }

  for (i = 0; i < MAX_CLIENTS && cnt < max; i++) {
    if (!(f == CG_IsOnFireteam(sortedFireTeamClients[i]))) {
      return NULL;
    }

    if (cnt == pos) {
      return &cgs.clientinfo[sortedFireTeamClients[i]];
    }
    cnt++;
  }

  return NULL;
}

/******************************************************************************
***** Main Functions
****/

constexpr float FT_BAR_YSPACING = 2.0f;
constexpr float FT_BAR_HEIGHT = 10.0f;
constexpr int FT_WIDTH = 204;
constexpr int FT_HEADER_HEIGHT = 12;

void CG_DrawFireTeamOverlay(rectDef_t *rect) {
  float x = rect->x;
  float y = rect->y + 1 +
            etj_fireteamPosY.value; // +1, jitter it into place in 1024 :)
  float h;
  clientInfo_t *ci;
  std::string buffer;
  fireteamData_t *f;
  int i;
  vec4_t clr1 = {.16f, .2f, .17f, .8f};
  vec4_t clr2 = {0.f, 0.f, 0.f, .2f};
  vec4_t clr3 = {0.25f, 0.f, 0.f, 153 / 255.f};
  vec4_t tclr = {0.6f, 0.6f, 0.6f, 1.0f};
  vec4_t bgColor = {0.0f, 0.0f, 0.0f, 0.6f};     // window
  vec4_t borderColor = {0.5f, 0.5f, 0.5f, 0.5f}; // window

  const float fireteamOffsetX = ETJump_AdjustPosition(etj_fireteamPosX.value);
  x += fireteamOffsetX;

  const float fireteamAlpha =
      Numeric::clamp(etj_fireteamAlpha.value, 0.0f, 1.0f);

  clr1[3] *= fireteamAlpha;
  clr2[3] *= fireteamAlpha;
  clr3[3] *= fireteamAlpha;
  tclr[3] *= fireteamAlpha;
  bgColor[3] *= fireteamAlpha;
  borderColor[3] *= fireteamAlpha;
  // end ft alpha

  f = CG_IsOnFireteam(cg.clientNum);
  if (!f) {
    return;
  }

  const bool saveLimit = f->saveLimit != FT_SAVELIMIT_NOT_SET;
  h = FT_HEADER_HEIGHT + 2 + 2;

  for (i = 0; i < MAX_FIRETEAM_USERS; i++) {
    ci = CG_SortedFireTeamPlayerForPosition(i, MAX_FIRETEAM_USERS);
    if (!ci) {
      break;
    }

    h += FT_BAR_HEIGHT + FT_BAR_YSPACING;
  }

  CG_DrawRect(x, y, FT_WIDTH, h, 1, borderColor);
  CG_FillRect(x + 1, y + 1, FT_WIDTH - 2, h - 2, bgColor);

  x += 2;
  y += 2;

  CG_FillRect(x, y, FT_WIDTH - 4, FT_HEADER_HEIGHT, clr1);

  buffer = ETJump::StringUtil::toUpperCase(
      ETJump::stringFormat("Fireteam: %s", bg_fireteamNames[f->ident]));
  CG_Text_Paint_Ext(x + 3, y + FT_BAR_HEIGHT, .19f, .19f, tclr, buffer, 0, 0, 0,
                    &cgs.media.limboFont1);

  if (saveLimit) {
    if (f->saveLimit == 0) {
      vec4_t friendShaderColor = {1.0f, 1.0f, 1.0f, fireteamAlpha};
      ETJump::drawPic(rect->x + FT_WIDTH - 4 - 8 + fireteamOffsetX,
                      y + FT_BAR_HEIGHT - 8, 8, 8, cgs.media.saveIcon, tclr);
      ETJump::drawPic(rect->x + FT_WIDTH - 4 - 8 + fireteamOffsetX,
                      y + FT_BAR_HEIGHT - 8, 8, 8, cgs.media.friendShader,
                      friendShaderColor);

    } else {
      buffer = ETJump::stringFormat("%i", f->saveLimit);
      auto textW = static_cast<float>(
          CG_Text_Width_Ext(buffer, 0.19f, 0, &cgs.media.limboFont1));
      CG_Text_Paint_RightAligned_Ext(
          rect->x + FT_WIDTH - 4 + fireteamOffsetX, y + FT_BAR_HEIGHT, .19f,
          .19f, tclr, va("%i", f->saveLimit), 0, 0, 0, &cgs.media.limboFont1);
      ETJump::drawPic(rect->x + FT_WIDTH - 4 - textW - 12 + fireteamOffsetX,
                      y + FT_BAR_HEIGHT - 8, 8, 8, cgs.media.saveIcon, tclr);
    }
  }

  x += 2;

  for (i = 0; i < MAX_FIRETEAM_USERS; i++) {
    y += FT_BAR_HEIGHT + FT_BAR_YSPACING;
    x = rect->x + 2 + fireteamOffsetX;

    ci = CG_SortedFireTeamPlayerForPosition(i, MAX_FIRETEAM_USERS);
    if (!ci) {
      break;
    }

    if (ci->selected) {
      CG_FillRect(x, y + FT_BAR_YSPACING, FT_WIDTH - 4, FT_BAR_HEIGHT, clr3);
    } else {
      CG_FillRect(x, y + FT_BAR_YSPACING, FT_WIDTH - 4, FT_BAR_HEIGHT, clr2);
    }

    x += 4;

    CG_Text_Paint_Ext(
        x, y + FT_BAR_HEIGHT, .2f, .2f, tclr,
        ci->team != TEAM_SPECTATOR ? BG_ClassLetterForNumber(ci->cls) : "", 0,
        0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont2);
    x += 12;

    CG_Text_Paint_Ext(x, y + FT_BAR_HEIGHT, .2f, .2f, tclr, ci->name, 0, 25,
                      ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont2);
    x += 176;

    if (ci->team == TEAM_SPECTATOR) {
      CG_Text_Paint_Ext(x, y + FT_BAR_HEIGHT, .2f, .2f, tclr, "^3S", 0, 0,
                        ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont2);
    } else {
      vec4_t healthColor = {0.6f, 0.6f, 0.6f, 1.0f};
      const char *healthStr;

      if (ci->health > 80) {
        healthStr = va("%i", ci->health);
      } else if (ci->health > 0) {
        Vector4Copy(colorYellow, healthColor);
        healthStr = va("%i", ci->health);
      } else {
        Vector4Copy(colorRed, healthColor);
        healthStr = va("%i", ci->health < 0 ? 0 : ci->health);
      }

      healthColor[3] *= fireteamAlpha;

      x = rect->x +
          (FT_WIDTH - 4 -
           static_cast<float>(
               CG_Text_Width_Ext(healthStr, .2f, 0, &cgs.media.limboFont2))) +
          fireteamOffsetX;

      CG_Text_Paint_Ext(x, y + FT_BAR_HEIGHT, .2f, .2f, healthColor, healthStr,
                        0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont2);
    }
  }
}

qboolean CG_FireteamGetBoxNeedsButtons() {
  if (cgs.applicationEndTime > cg.time) {
    if (cgs.applicationClient < 0) {
      return qfalse;
    }
    return qtrue;
  }

  if (cgs.invitationEndTime > cg.time) {
    if (cgs.invitationClient < 0) {
      return qfalse;
    }
    return qtrue;
  }

  if (cgs.propositionEndTime > cg.time) {
    if (cgs.propositionClient < 0) {
      return qfalse;
    }
    return qtrue;
  }

  return qfalse;
}

const char *CG_FireteamGetBoxText() {
  if (cgs.applicationEndTime > cg.time) {
    if (cgs.applicationClient == -1) {
      return "Sent";
    }

    if (cgs.applicationClient == -2) {
      return "Failed";
    }

    if (cgs.applicationClient == -3) {
      return "Accepted";
    }

    if (cgs.applicationClient == -4) {
      return "Sent";
    }

    if (cgs.applicationClient < 0) {
      return NULL;
    }

    return va("Accept application from %s?",
              cgs.clientinfo[cgs.applicationClient].name);
  }

  if (cgs.invitationEndTime > cg.time) {
    if (cgs.invitationClient == -1) {
      return "Sent";
    }

    if (cgs.invitationClient == -2) {
      return "Failed";
    }

    if (cgs.invitationClient == -3) {
      return "Accepted";
    }

    if (cgs.invitationClient == -4) {
      return "Sent";
    }

    if (cgs.invitationClient < 0) {
      return NULL;
    }

    return va("Accept invitiation from %s?",
              cgs.clientinfo[cgs.invitationClient].name);
  }

  if (cgs.propositionEndTime > cg.time) {
    if (cgs.propositionClient == -1) {
      return "Sent";
    }

    if (cgs.propositionClient == -2) {
      return "Failed";
    }

    if (cgs.propositionClient == -3) {
      return "Accepted";
    }

    if (cgs.propositionClient == -4) {
      return "Sent";
    }

    if (cgs.propositionClient < 0) {
      return NULL;
    }

    return va("Accept %s's proposition to invite %s to join "
              "your fireteam?",
              cgs.clientinfo[cgs.propositionClient2].name,
              cgs.clientinfo[cgs.propositionClient].name);
  }

  return NULL;
}

qboolean CG_FireteamHasClass(int classnum, qboolean selectedonly) {
  fireteamData_t *ft;
  int i;

  ft = CG_IsOnFireteam(cg.clientNum);
  if (!ft) {
    return qfalse;
  }

  for (i = 0; i < MAX_CLIENTS; i++) {
    /*		if( i == cgs.clientinfo ) {
                continue;
            }*/

    if (!cgs.clientinfo[i].infoValid) {
      continue;
    }

    if (ft != CG_IsOnFireteam(i)) {
      continue;
    }

    if (cgs.clientinfo[i].cls != classnum) {
      continue;
    }

    if (selectedonly && !cgs.clientinfo[i].selected) {
      continue;
    }

    return qtrue;
  }

  return qfalse;
}

const char *CG_BuildSelectedFirteamString() {
  char buffer[256];
  clientInfo_t *ci;
  int cnt = 0;
  int i;

  *buffer = '\0';
  for (i = 0; i < MAX_FIRETEAM_USERS; i++) {
    ci = CG_SortedFireTeamPlayerForPosition(i, MAX_FIRETEAM_USERS);
    if (!ci) {
      break;
    }

    if (!ci->selected) {
      continue;
    }

    cnt++;

    Q_strcat(buffer, sizeof(buffer), va("%i ", ci->clientNum));
  }

  if (cnt == 0) {
    return "0";
  }

  if (!cgs.clientinfo[cg.clientNum].selected) {
    Q_strcat(buffer, sizeof(buffer), va("%i ", cg.clientNum));
    cnt++;
  }

  return va("%i %s", cnt, buffer);
}
