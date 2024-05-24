#include <vector>

#include "cg_local.h"
#include "etj_utilities.h"
#include "../game/etj_numeric_utilities.h"

panel_button_text_t fireteamTitleFont = {
    0.19f, 0.19f, {0.6f, 0.6f, 0.6f, 1.f}, 0, 0, &cgs.media.limboFont1_lo,
};

panel_button_text_t fireteamFont = {
    0.2f,
    0.2f,
    {0.6f, 0.6f, 0.6f, 1.f},
    ITEM_TEXTSTYLE_SHADOWED,
    0,
    &cgs.media.limboFont2,
};

panel_button_t fireteamTopBorder = {
    NULL,
    "",
    {10, 129, 204, 136},
    {1, (int)(255 * .5f), (int)(255 * .5f), (int)(255 * .5f), (int)(255 * .5f),
     1, 0, 0},
    NULL, /* font		*/
    NULL, /* keyDown	*/
    NULL, /* keyUp	*/
    BG_PanelButtonsRender_Img,
    NULL,
};

panel_button_t fireteamTopBorderBack = {
    "white",
    "",
    {11, 130, 202, 134},
    {1, 0, 0, 0, (int)(255 * 0.75f), 0, 0, 0},
    NULL, /* font		*/
    NULL, /* keyDown	*/
    NULL, /* keyUp	*/
    BG_PanelButtonsRender_Img,
    NULL,
};

panel_button_t fireteamTopBorderInner = {
    "white",
    "",
    {12, 131, 200, 12},
    {1, 41, 51, 43, 204, 0, 0, 0},
    NULL, /* font		*/
    NULL, /* keyDown	*/
    NULL, /* keyUp	*/
    BG_PanelButtonsRender_Img,
    NULL,
};

panel_button_t fireteamTopBorderInnerText = {
    NULL,
    "",
    {15, 141, 200, 12},
    {0, 0, 0, 0, 0, 0, 0, 0},
    &fireteamTitleFont, /* font		*/
    NULL,               /* keyDown	*/
    NULL,               /* keyUp	*/
    CG_Fireteams_MenuTitleText_Draw,
    NULL,
};

panel_button_t fireteamMenuItemText = {
    NULL,
    "",
    {16, 153, 128, 12},
    {0, 0, 0, 0, 0, 0, 0, 0},
    &fireteamFont, /* font		*/
    NULL,          /* keyDown	*/
    NULL,          /* keyUp	*/
    CG_Fireteams_MenuText_Draw,
    NULL,
};

panel_button_t *fireteamButtons[] = {&fireteamTopBorderBack,
                                     &fireteamTopBorder,
                                     &fireteamTopBorderInner,
                                     &fireteamTopBorderInnerText,

                                     &fireteamMenuItemText,

                                     NULL};

std::vector<panel_button_t> fireteamButtonsLayout;

const char *ftMenuRootStrings[] = {"Soldier",   "Medic",      "Engineer",
                                   "Field Ops", "Covert Ops", "General",
                                   "Attack",    "Fall Back",  NULL};

const char *ftMenuRootStringsMsg[] = {"", "",         "",           "",  "",
                                      "", "FTAttack", "FTFallBack", NULL};

const char *ftMenuRootStringsAlphachars[] = {"S", "M", "E", "F", "C",
                                             "G", "A", "B", NULL};

const char *ftMenuSoliderStrings[] = {"Cover Me", "Covering Fire", "Mortar",
                                      NULL};

const char *ftMenuSoliderStringsAlphachars[] = {"C", "F", "M", NULL};

const char *ftMenuSoliderStringsMsg[] = {"FTCoverMe", "FTCoveringFire",
                                         "FTMortar", NULL};

const char *ftMenuMedicStrings[] = {"Heal Squad", "Heal Me", "Revive Team Mate",
                                    "Revive Me", NULL};

const char *ftMenuMedicStringsAlphachars[] = {"H", "M", "R", "E", NULL};

const char *ftMenuMedicStringsMsg[] = {"FTHealSquad", "FTHealMe",
                                       "FTReviveTeamMate", "FTReviveMe", NULL};

const char *ftMenuEngineerStrings[] = {"Destroy Objective",
                                       "Repair Objective",
                                       "Construct Objective",
                                       "Disarm Dynamite",
                                       "Deploy Landmines",
                                       "Disarm Landmines",
                                       NULL};

const char *ftMenuEngineerStringsAlphachars[] = {"D", "R", "C", "A",
                                                 "L", "M", NULL};

const char *ftMenuEngineerStringsMsg[] = {"FTDestroyObjective",
                                          "FTRepairObjective",
                                          "FTConstructObjective",
                                          "FTDisarmDynamite",
                                          "FTDeployLandmines",
                                          "FTDisarmLandmines",
                                          NULL};

const char *ftMenuFieldOpsStrings[] = {"Call Air-Strike", "Call Artillery",
                                       "Resupply Squad", "Resupply Me", NULL};

const char *ftMenuFieldOpsStringsAlphachars[] = {"A", "T", "R", "S", NULL};

const char *ftMenuFieldOpsStringsMsg[] = {"FTCallAirStrike", "FTCallArtillery",
                                          "FTResupplySquad", "FTResupplyMe",
                                          NULL};

const char *ftMenuCovertOpsStrings[] = {
    "Explore Area",  "Destroy Objective",    "Infiltrate",
    "Go Undercover", "Provide Sniper Cover", NULL};

const char *ftMenuCovertOpsStringsAlphachars[] = {"E", "D", "I",
                                                  "U", "S", NULL};

const char *ftMenuCovertOpsStringsMsg[] = {
    "FTExploreArea",  "FTSatchelObjective",   "FTInfiltrate",
    "FTGoUndercover", "FTProvideSniperCover", NULL};

const char **ftMenuStrings[] = {
    ftMenuSoliderStrings,  ftMenuMedicStrings,     ftMenuEngineerStrings,
    ftMenuFieldOpsStrings, ftMenuCovertOpsStrings,
};

const char **ftMenuStringsAlphachars[] = {
    ftMenuSoliderStringsAlphachars,   ftMenuMedicStringsAlphachars,
    ftMenuEngineerStringsAlphachars,  ftMenuFieldOpsStringsAlphachars,
    ftMenuCovertOpsStringsAlphachars,
};

const char **ftMenuStringsMsg[] = {
    ftMenuSoliderStringsMsg,   ftMenuMedicStringsMsg,
    ftMenuEngineerStringsMsg,  ftMenuFieldOpsStringsMsg,
    ftMenuCovertOpsStringsMsg,
};

void CG_Fireteams_MenuTitleText_Draw(panel_button_t *button) {
  switch (static_cast<FTMenuMode>(cgs.ftMenuMode)) {
    case FTMenuMode::FT_VSAY:
      CG_Text_Paint_Ext(
          button->rect.x, button->rect.y + static_cast<float>(button->data[0]),
          button->font->scalex, button->font->scaley, button->font->colour,
          "MESSAGE", 0, 0, button->font->style, button->font->font);
      break;
    case FTMenuMode::FT_MANAGE:
      CG_Text_Paint_Ext(
          button->rect.x, button->rect.y + static_cast<float>(button->data[0]),
          button->font->scalex, button->font->scaley, button->font->colour,
          "FIRETEAMS", 0, 0, button->font->style, button->font->font);
      break;
    case FTMenuMode::FT_APPLY:
      CG_Text_Paint_Ext(
          button->rect.x, button->rect.y + static_cast<float>(button->data[0]),
          button->font->scalex, button->font->scaley, button->font->colour,
          "JOIN", 0, 0, button->font->style, button->font->font);
      break;
    case FTMenuMode::FT_PROPOSE:
      CG_Text_Paint_Ext(
          button->rect.x, button->rect.y + static_cast<float>(button->data[0]),
          button->font->scalex, button->font->scaley, button->font->colour,
          "PROPOSE", 0, 0, button->font->style, button->font->font);
      break;
    case FTMenuMode::FT_ADMIN:
      switch (static_cast<FTMenuPos>(cgs.ftMenuPos)) {
        case FTMenuPos::FT_MENUPOS_INVITE:
          CG_Text_Paint_Ext(
              button->rect.x,
              button->rect.y + static_cast<float>(button->data[0]),
              button->font->scalex, button->font->scaley, button->font->colour,
              "INVITE", 0, 0, button->font->style, button->font->font);
          break;
        case FTMenuPos::FT_MENUPOS_KICK:
          CG_Text_Paint_Ext(
              button->rect.x,
              button->rect.y + static_cast<float>(button->data[0]),
              button->font->scalex, button->font->scaley, button->font->colour,
              "KICK", 0, 0, button->font->style, button->font->font);
          break;
        case FTMenuPos::FT_MENUPOS_WARN:
          CG_Text_Paint_Ext(
              button->rect.x,
              button->rect.y + static_cast<float>(button->data[0]),
              button->font->scalex, button->font->scaley, button->font->colour,
              "WARN", 0, 0, button->font->style, button->font->font);
          break;
        case FTMenuPos::FT_MENUPOS_RULES:
          CG_Text_Paint_Ext(
              button->rect.x,
              button->rect.y + static_cast<float>(button->data[0]),
              button->font->scalex, button->font->scaley, button->font->colour,
              "RULES", 0, 0, button->font->style, button->font->font);
          break;
        default:
          break;
      }
  }
}

const char *ftOffMenuList[] = {
    "Apply",
    "Create",
    nullptr,
};

const char *ftOffMenuListAlphachars[] = {
    "A",
    "C",
    nullptr,
};

const char *ftOnMenuList[] = {
    "Propose",
    "Leave",
    nullptr,
};

const char *ftOnMenuListAlphachars[] = {
    "P",
    "L",
    nullptr,
};

const char *ftOnMenuRulesList[] = {
    "Reset savelimit",
    "Set savelimit",
    "%s player collision",
    nullptr,
};

const char *ftOnMenuRulesListAlphaChars[] = {
    "R",
    "S",
    "C",
    nullptr,
};

const char *ftLeaderMenuList[] = {
    "Disband", "Leave", "Invite",           "Kick",
    "Warn",    "Rules", "%s teamjump mode", nullptr,
};

const char *ftLeaderMenuListAlphachars[] = {
    "D", "L", "I", "K", "W", "R", "T", nullptr,
};

int CG_CountFireteamsByTeam(team_t t) {
  int cnt = 0;
  int i;

  for (i = 0; i < MAX_FIRETEAMS; i++) {
    if (!cg.fireTeams[i].inuse) {
      continue;
    }

    cnt++;
  }

  return cnt;
}

void CG_DrawFireteamsByTeam(panel_button_t *button, team_t t) {
  float y = button->rect.y;
  const char *str;
  int i;

  for (i = 0; i < MAX_FIRETEAMS; i++) {
    if (!cg.fireTeams[i].inuse) {
      continue;
    }

    if (cg_quickMessageAlt.integer) {
      str = va("%i. %s", (cg.fireTeams[i].ident + 1) % 10,
               bg_fireteamNames[cg.fireTeams[i].ident]);
    } else {
      str = va("%c. %s", 'A' + cg.fireTeams[i].ident,
               bg_fireteamNames[cg.fireTeams[i].ident]);
    }

    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour, str, 0, 0,
                      button->font->style, button->font->font);

    y += button->rect.h;
  }
}

int CG_CountPlayersSF(void) {
  int i, cnt = 0;

  for (i = 0; i < MAX_CLIENTS; i++) {
    if (i == cg.clientNum) {
      continue;
    }

    if (!cgs.clientinfo[i].infoValid) {
      continue;
    }

    if (CG_IsOnFireteam(i) != CG_IsOnFireteam(cg.clientNum)) {
      continue;
    }

    cnt++;
  }

  return cnt;
}

int CG_CountPlayersNF(void) {
  int i, cnt = 0;

  for (i = 0; i < MAX_CLIENTS; i++) {
    if (i == cg.clientNum) {
      continue;
    }

    if (!cgs.clientinfo[i].infoValid) {
      continue;
    }

    if (CG_IsOnFireteam(i)) {
      continue;
    }

    cnt++;
  }

  return cnt;
}

int CG_PlayerSFFromPos(int pos, int *pageofs) {
  int x, i;

  if (!CG_IsOnFireteam(cg.clientNum)) {
    *pageofs = 0;
    return -1;
  }

  x = CG_CountPlayersSF();
  if (x < ((*pageofs) * 8)) {
    *pageofs = 0;
  }

  x = 0;
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (i == cg.clientNum) {
      continue;
    }

    if (!cgs.clientinfo[i].infoValid) {
      continue;
    }

    if (CG_IsOnFireteam(i) != CG_IsOnFireteam(cg.clientNum)) {
      continue;
    }

    if (x >= ((*pageofs) * 8) && x < ((*pageofs + 1) * 8)) {
      int ofs = x - ((*pageofs) * 8);

      if (pos == ofs) {
        return i;
      }
    }

    x++;
  }

  return -1;
}

int CG_PlayerNFFromPos(int pos, int *pageofs) {
  int x, i;

  if (!CG_IsOnFireteam(cg.clientNum)) {
    *pageofs = 0;
    return -1;
  }

  x = CG_CountPlayersNF();
  if (x < ((*pageofs) * 8)) {
    *pageofs = 0;
  }

  x = 0;
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (i == cg.clientNum) {
      continue;
    }

    if (!cgs.clientinfo[i].infoValid) {
      continue;
    }

    if (CG_IsOnFireteam(i)) {
      continue;
    }

    if (x >= ((*pageofs) * 8) && x < ((*pageofs + 1) * 8)) {
      int ofs = x - ((*pageofs) * 8);

      if (pos == ofs) {
        return i;
      }
    }

    x++;
  }

  return -1;
}

void CG_DrawPlayerSF(panel_button_t *button, int *pageofs) {
  float y = button->rect.y;
  const char *str;
  int i, x;

  for (i = 0; i < MAX_FIRETEAM_USERS; i++) {
    x = CG_PlayerSFFromPos(i, pageofs);
    if (x == -1) {
      break;
    }

    if (cg_quickMessageAlt.integer) {
      str = va("%i. %s", (i + 1) % 10, cgs.clientinfo[x].name);
    } else {
      str = va("%c. %s", 'A' + i, cgs.clientinfo[x].name);
    }

    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour, str, 0, 0,
                      button->font->style, button->font->font);

    y += button->rect.h;
  }

  if (*pageofs) {
    if (cg_quickMessageAlt.integer) {
      str = va("%i. %s", (8 + 1) % 10, "Previous");
    } else {
      str = va("%c. %s", 'P', "Previous");
    }
    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour, str, 0, 0,
                      button->font->style, button->font->font);

    y += button->rect.h;
  }

  if (CG_CountPlayersSF() > (*pageofs + 1) * 8) {
    if (cg_quickMessageAlt.integer) {
      str = va("%i. %s", (9 + 1) % 10, "Next");
    } else {
      str = va("%c. %s", 'N', "Next");
    }
    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour, str, 0, 0,
                      button->font->style, button->font->font);

    y += button->rect.h;
  }
}

void CG_DrawPlayerNF(panel_button_t *button, int *pageofs) {
  float y = button->rect.y;
  const char *str;
  int i, x;

  for (i = 0; i < MAX_FIRETEAM_USERS; i++) {
    x = CG_PlayerNFFromPos(i, pageofs);
    if (x == -1) {
      break;
    }

    if (cg_quickMessageAlt.integer) {
      str = va("%i. %s", (i + 1) % 10, cgs.clientinfo[x].name);
    } else {
      str = va("%c. %s", 'A' + i, cgs.clientinfo[x].name);
    }

    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour, str, 0, 0,
                      button->font->style, button->font->font);

    y += button->rect.h;
  }

  if (*pageofs) {
    if (cg_quickMessageAlt.integer) {
      str = va("%i. %s", (8 + 1) % 10, "Previous");
    } else {
      str = va("%c. %s", 'P', "Previous");
    }
    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour, str, 0, 0,
                      button->font->style, button->font->font);

    y += button->rect.h;
  }

  if (CG_CountPlayersNF() > (*pageofs + 1) * 8) {
    if (cg_quickMessageAlt.integer) {
      str = va("%i. %s", (9 + 1) % 10, "Next");
    } else {
      str = va("%c. %s", 'N', "Next");
    }
    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour, str, 0, 0,
                      button->font->style, button->font->font);

    y += button->rect.h;
  }
}

void CG_DrawFireteamRules(panel_button_t *button) {
  float y = button->rect.y;
  const char *str;
  int i;

  for (i = 0; ftOnMenuRulesList[i]; i++) {
    if (cg_quickMessageAlt.integer) {
      str = va("%i. %s", (i + 1) % 10, ftOnMenuRulesList[i]);
    } else {
      str = va("%s. %s", ftOnMenuRulesListAlphaChars[i], ftOnMenuRulesList[i]);
    }

    if (i == static_cast<int>(FTMenuRulesPos::FT_RULES_NOGHOST)) {
      if (cg_ghostPlayers.integer == 1) {
        str = va(str, cgs.clientinfo[cg.clientNum].fireteamData->noGhost
                          ? "Disable"
                          : "Enable");
      } else {
        // this won't actually ever work, but for the sake of correct status
        str = va(str, "Disable");
      }
    }

    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour, str, 0, 0,
                      button->font->style, button->font->font);

    y += button->rect.h;
  }
}

void CG_Fireteams_MenuText_Draw(panel_button_t *button) {
  float y = button->rect.y;
  int i;

  switch (static_cast<FTMenuMode>(cgs.ftMenuMode)) {
    case FTMenuMode::FT_VSAY:
      if (cgs.ftMenuPos == static_cast<int>(FTMenuPos::FT_MENUPOS_NONE)) {
        for (i = 0; ftMenuRootStrings[i]; i++) {
          const char *str;

          if (i < NUM_PLAYER_CLASSES) {
            if (!CG_FireteamHasClass(i, qtrue)) {
              continue;
            }
          }

          if (cg_quickMessageAlt.integer) {
            str = va("%i. %s", (i + 1) % 10, ftMenuRootStrings[i]);
          } else {
            str = va("%s. %s", ftMenuRootStringsAlphachars[i],
                     ftMenuRootStrings[i]);
          }

          CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                            button->font->scaley, button->font->colour, str, 0,
                            0, button->font->style, button->font->font);

          y += button->rect.h;
        }
      } else {
        // spectators don't get class-specific vsays
        if (!ETJump::isPlaying(cg.clientNum)) {
          return;
        }

        // sanity check, cls should be valid but let's make sure
        const int idx = Numeric::clamp(cgs.clientinfo[cg.clientNum].cls, 0,
                                       NUM_PLAYER_CLASSES - 1);
        const char **strings = ftMenuStrings[idx];

        for (i = 0; strings[i]; i++) {
          const char *str;

          if (cg_quickMessageAlt.integer) {
            str = va("%i. %s", (i + 1) % 10, strings[i]);
          } else {
            str = va("%s. %s", (ftMenuStringsAlphachars[idx])[i], strings[i]);
          }

          CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                            button->font->scaley, button->font->colour, str, 0,
                            0, button->font->style, button->font->font);

          y += button->rect.h;
        }
      }
      break;
    case FTMenuMode::FT_MANAGE:
      if (!CG_IsOnFireteam(cg.clientNum)) {
        for (i = 0; ftOffMenuList[i]; i++) {
          const char *str;

          if (i == 0 &&
              !CG_CountFireteamsByTeam(cgs.clientinfo[cg.clientNum].team)) {
            continue;
          }

          if (cg_quickMessageAlt.integer) {
            str = va("%i. %s", (i + 1) % 10, ftOffMenuList[i]);
          } else {
            str = va("%s. %s", ftOffMenuListAlphachars[i], ftOffMenuList[i]);
          }

          CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                            button->font->scaley, button->font->colour, str, 0,
                            0, button->font->style, button->font->font);

          y += button->rect.h;
        }
      } else {
        if (!CG_IsFireTeamLeader(cg.clientNum)) {
          for (i = 0; ftOnMenuList[i]; i++) {
            const char *str;

            if (i == 0 && !CG_CountPlayersNF()) {
              continue;
            }

            if (cg_quickMessageAlt.integer) {
              str = va("%i. %s", (i + 1) % 10, ftOnMenuList[i]);
            } else {
              str = va("%s. %s", ftOnMenuListAlphachars[i], ftOnMenuList[i]);
            }

            CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                              button->font->scaley, button->font->colour, str,
                              0, 0, button->font->style, button->font->font);

            y += button->rect.h;
          }
        } else {
          for (i = 0; ftLeaderMenuList[i]; i++) {
            const char *str;

            if (i == 2 && !CG_CountPlayersNF()) {
              continue;
            }

            if ((i == 3 || i == 4) && !CG_CountPlayersSF()) {
              continue;
            }

            if (cg_quickMessageAlt.integer) {
              str = va("%i. %s", (i + 1) % 10, ftLeaderMenuList[i]);
            } else {
              str = va("%s. %s", ftLeaderMenuListAlphachars[i],
                       ftLeaderMenuList[i]);
            }

            if (i == static_cast<int>(FTMenuOptions::FT_TJMODE)) {
              str = va(str,
                       cgs.clientinfo[cg.clientNum].fireteamData->teamJumpMode
                           ? "Disable"
                           : "Enable");
            }

            CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                              button->font->scaley, button->font->colour, str,
                              0, 0, button->font->style, button->font->font);

            y += button->rect.h;
          }
        }
      }
      break;

    case FTMenuMode::FT_APPLY:
      if (!CG_CountFireteamsByTeam(cgs.clientinfo[cg.clientNum].team) ||
          CG_IsOnFireteam(cg.clientNum)) {
        cgs.ftMenuMode = static_cast<int>(FTMenuMode::FT_MANAGE);
        break;
      }

      CG_DrawFireteamsByTeam(button, cgs.clientinfo[cg.clientNum].team);
      break;

    case FTMenuMode::FT_PROPOSE:
      if (!CG_CountPlayersNF()) {
        cgs.ftMenuMode = static_cast<int>(FTMenuMode::FT_MANAGE);
        break;
      }

      CG_DrawPlayerNF(button, &cgs.ftMenuModeEx);
      break;

    case FTMenuMode::FT_ADMIN:
      switch (static_cast<FTMenuPos>(cgs.ftMenuPos)) {
        case FTMenuPos::FT_MENUPOS_INVITE:
          if (!CG_CountPlayersNF()) {
            cgs.ftMenuMode = static_cast<int>(FTMenuMode::FT_MANAGE);
            break;
          }

          CG_DrawPlayerNF(button, &cgs.ftMenuModeEx);
          break;
        case FTMenuPos::FT_MENUPOS_KICK:
        case FTMenuPos::FT_MENUPOS_WARN:
          if (!CG_CountPlayersSF()) {
            cgs.ftMenuMode = static_cast<int>(FTMenuMode::FT_MANAGE);
            break;
          }

          CG_DrawPlayerSF(button, &cgs.ftMenuModeEx);
          break;
        case FTMenuPos::FT_MENUPOS_RULES:
          CG_DrawFireteamRules(button);
          break;
        default:
          break;
      }
      break;
  }
}

void CG_Fireteams_Setup(void) {
  fireteamButtonsLayout.clear();

  for (auto btnptr : fireteamButtons) {
    if (btnptr) {
      fireteamButtonsLayout.push_back(*btnptr);
    }
  }

  BG_PanelButtonsSetup(fireteamButtonsLayout);
}

void CG_Fireteams_KeyHandling(int key, qboolean down) {
  if (down) {
    CG_FireteamCheckExecKey(key, qtrue);
  }
}

void CG_Fireteams_Draw(void) { BG_PanelButtonsRender(fireteamButtonsLayout); }

void CG_QuickFireteamMessage_f(void);

qboolean CG_FireteamCheckExecKey(int key, qboolean doaction) {
  if (key == K_ESCAPE) {
    return qtrue;
  }

  if ((key & K_CHAR_FLAG)) {
    return qfalse;
  }

  key &= ~K_CHAR_FLAG;

  switch (static_cast<FTMenuMode>(cgs.ftMenuMode)) {
    case FTMenuMode::FT_VSAY:
      if (cgs.ftMenuPos == static_cast<int>(FTMenuPos::FT_MENUPOS_NONE)) {
        if (cg_quickMessageAlt.integer) {
          if (key >= '0' && key <= '9') {
            int i = ((key - '0') + 9) % 10;

            if (i < NUM_PLAYER_CLASSES) {
              if (!CG_FireteamHasClass(i, qtrue)) {
                return qfalse;
              }
            }

            if (i > 7) {
              return qfalse;
            }

            if (doaction) {
              // class-specific vsays
              if (i < NUM_PLAYER_CLASSES) {
                cgs.ftMenuPos = i;
              } else if (i == 5) {
                CG_QuickFireteamMessage_f();
              } else {
                trap_SendClientCommand(va("vsay_buddy -1 %s %s",
                                          CG_BuildSelectedFirteamString(),
                                          ftMenuRootStringsMsg[i]));
                CG_EventHandling(CGAME_EVENT_NONE, qfalse);
              }
            }

            return qtrue;
          }
        } else {
          int i;

          for (i = 0; ftMenuRootStrings[i]; i++) {
            if (key == tolower(*ftMenuRootStringsAlphachars[i])) {
              if (i < NUM_PLAYER_CLASSES) {
                if (!CG_FireteamHasClass(i, qtrue)) {
                  return qfalse;
                }
              }

              if (doaction) {
                // class-specifc vsays
                if (i < NUM_PLAYER_CLASSES) {
                  cgs.ftMenuPos = i;
                } else if (i == 5) {
                  CG_QuickFireteamMessage_f();
                } else {
                  trap_SendClientCommand(va("vsay_buddy -1 %s %s",
                                            CG_BuildSelectedFirteamString(),
                                            ftMenuRootStringsMsg[i]));
                  CG_EventHandling(CGAME_EVENT_NONE, qfalse);
                }
              }
              return qtrue;
            }
          }
        }
      } else {
        // spectators don't get class-specific vsays
        if (!ETJump::isPlaying(cg.clientNum)) {
          return qfalse;
        }

        // sanity check, cls should be valid but let's make sure
        const int idx = Numeric::clamp(cgs.clientinfo[cg.clientNum].cls, 0,
                                       NUM_PLAYER_CLASSES - 1);

        if (cg_quickMessageAlt.integer) {
          if (key >= '0' && key <= '9') {
            int i = ((key - '0') + 9) % 10;
            int x;

            const char **strings = ftMenuStrings[idx];

            for (x = 0; strings[x]; x++) {
              if (x == i) {
                if (doaction) {
                  trap_SendClientCommand(va("vsay_buddy %i %s %s",
                                            cgs.ftMenuPos,
                                            CG_BuildSelectedFirteamString(),
                                            (ftMenuStringsMsg[idx])[i]));
                  CG_EventHandling(CGAME_EVENT_NONE, qfalse);
                }

                return qtrue;
              }
            }
          }
        } else {
          int i;
          const char **strings = ftMenuStrings[idx];

          for (i = 0; strings[i]; i++) {
            if (key == tolower(*ftMenuStringsAlphachars[idx][i])) {

              if (doaction) {

                trap_SendClientCommand(va("vsay_buddy %i %s %s", cgs.ftMenuPos,
                                          CG_BuildSelectedFirteamString(),
                                          (ftMenuStringsMsg[idx])[i]));
                CG_EventHandling(CGAME_EVENT_NONE, qfalse);
              }
              return qtrue;
            }
          }
        }
      }
      break;
    case FTMenuMode::FT_MANAGE: {
      int i = -1, x;

      if (cg_quickMessageAlt.integer) {
        if (key >= '0' && key <= '9') {
          i = ((key - '0') + 9) % 10;
        }
      } else {
        const char **strings;

        if (!CG_IsOnFireteam(cg.clientNum)) {
          strings = ftOffMenuListAlphachars;
        } else {
          if (!CG_IsFireTeamLeader(cg.clientNum)) {
            strings = ftOnMenuListAlphachars;
          } else {
            strings = ftLeaderMenuListAlphachars;
          }
        }

        for (x = 0; strings[x]; x++) {
          if (key == tolower(*strings[x])) {
            i = x;
            break;
          }
        }
      }

      if (i == -1) {
        break;
      }

      if (!CG_IsOnFireteam(cg.clientNum)) {
        if (i >= 2) {
          break;
        }

        if (i == 0 &&
            !CG_CountFireteamsByTeam(cgs.clientinfo[cg.clientNum].team)) {
          return qfalse;
        }

        if (doaction) {
          if (i == 1) {
            trap_SendConsoleCommand("fireteam create\n");
            CG_EventHandling(CGAME_EVENT_NONE, qfalse);
          } else {
            cgs.ftMenuMode = static_cast<int>(FTMenuMode::FT_APPLY);
            cgs.ftMenuModeEx = 0;
            cgs.ftMenuPos = i;
          }
        }

        return qtrue;
      } else {
        if (!CG_IsFireTeamLeader(cg.clientNum)) {
          if (i >= 2) {
            break;
          }

          if (i == 0 && !CG_CountPlayersNF()) {
            break;
          }

          if (doaction) {
            if (i == 1) {
              trap_SendConsoleCommand("fireteam leave\n");
              CG_EventHandling(CGAME_EVENT_NONE, qfalse);
            } else {
              cgs.ftMenuMode = static_cast<int>(FTMenuMode::FT_PROPOSE);
              cgs.ftMenuModeEx = 0;
              cgs.ftMenuPos = i;
            }
          }

          return qtrue;
        } else {
          if (i >= static_cast<int>(FTMenuOptions::FT_MAX_OPTIONS)) {
            break;
          }

          if (i == static_cast<int>(FTMenuOptions::FT_INVITE) &&
              !CG_CountPlayersNF()) {
            break;
          }

          if ((i == static_cast<int>(FTMenuOptions::FT_KICK) ||
               i == static_cast<int>(FTMenuOptions::FT_WARN)) &&
              !CG_CountPlayersSF()) {
            break;
          }

          if (doaction) {
            if (i == static_cast<int>(FTMenuOptions::FT_DISBAND_PROPOSE)) {
              trap_SendConsoleCommand("fireteam disband\n");
              CG_EventHandling(CGAME_EVENT_NONE, qfalse);
            } else if (i == static_cast<int>(FTMenuOptions::FT_CREATE_LEAVE)) {
              trap_SendConsoleCommand("fireteam leave\n");
              CG_EventHandling(CGAME_EVENT_NONE, qfalse);
            } else if (i == static_cast<int>(FTMenuOptions::FT_TJMODE)) {
              trap_SendConsoleCommand(va(
                  "fireteam teamjump %i",
                  cgs.clientinfo[cg.clientNum].fireteamData->teamJumpMode ? 0
                                                                          : 1));
              CG_EventHandling(CGAME_EVENT_NONE, qfalse);
            } else {
              cgs.ftMenuMode = static_cast<int>(FTMenuMode::FT_ADMIN);
              cgs.ftMenuModeEx = 0;
              cgs.ftMenuPos = i;
            }
          }

          return qtrue;
        }
      }
    }
    case FTMenuMode::FT_APPLY: {
      int i;

      for (i = 0; i < MAX_FIRETEAMS; i++) {
        if (!cg.fireTeams[i].inuse) {
          continue;
        }

        if (cg_quickMessageAlt.integer) {
          if (key >= '0' && key <= '9') {
            if (((key - '0') + 9) % 10 == cg.fireTeams[i].ident) {
              if (doaction) {
                trap_SendConsoleCommand(va("fireteam apply %i\n", i + 1));
                CG_EventHandling(CGAME_EVENT_NONE, qfalse);
              }
              return qtrue;
            }
          }
        } else {
          if (key - 'a' == cg.fireTeams[i].ident) {
            if (doaction) {
              trap_SendConsoleCommand(va("fireteam apply %i\n", i + 1));
              CG_EventHandling(CGAME_EVENT_NONE, qfalse);
            }
            return qtrue;
          }
        }
      }
    } break;
    case FTMenuMode::FT_PROPOSE: {
      int i = -1, x;

      if (cg_quickMessageAlt.integer) {
        if (key >= '0' && key <= '9') {
          i = ((key - '0') + 9) % 10;
        }
      } else {
        i = key - 'a';

        if (key == 'n') {
          i = 9;
        }

        if (key == 'p') {
          i = 0;
        }
      }

      if (i == -1) {
        break;
      }

      if (CG_CountPlayersNF() > (cgs.ftMenuModeEx + 1) * 8) {
        if (i == 0) {
          cgs.ftMenuModeEx++;
        }
      }
      if (cgs.ftMenuModeEx) {
        if (i == 9) {
          cgs.ftMenuModeEx--;
        }
      }
      x = CG_PlayerNFFromPos(i, &cgs.ftMenuModeEx);
      if (x != -1) {
        if (doaction) {
          trap_SendConsoleCommand(va("fireteam propose %i\n", x));
          CG_EventHandling(CGAME_EVENT_NONE, qfalse);
        }

        return qtrue;
      }
      break;
    }
    case FTMenuMode::FT_ADMIN: {
      int i = -1, x;

      if (cg_quickMessageAlt.integer) {
        if (key >= '0' && key <= '9') {
          i = ((key - '0') + 9) % 10;
        }
      } else {
        i = key - 'a';

        if (key == 'n') {
          i = 9;
        }

        if (key == 'p') {
          i = 8;
        }
      }

      if (i == -1) {
        break;
      }

      switch (static_cast<FTMenuPos>(cgs.ftMenuPos)) {
        case FTMenuPos::FT_MENUPOS_INVITE:
          if (CG_CountPlayersNF() > (cgs.ftMenuModeEx + 1) * 8) {
            if (i == 9) {
              if (doaction) {
                cgs.ftMenuModeEx++;
              }

              return qtrue;
            }
          }
          if (cgs.ftMenuModeEx) {
            if (i == 8) {
              if (doaction) {
                cgs.ftMenuModeEx--;
              }

              return qtrue;
            }
          }
          x = CG_PlayerNFFromPos(i, &cgs.ftMenuModeEx);
          if (x != -1) {
            if (doaction) {
              trap_SendConsoleCommand(va("fireteam invite %i\n", x));
              CG_EventHandling(CGAME_EVENT_NONE, qfalse);
            }

            return qtrue;
          }
          break;
        case FTMenuPos::FT_MENUPOS_KICK:
        case FTMenuPos::FT_MENUPOS_WARN:
          if (CG_CountPlayersSF() > (cgs.ftMenuModeEx + 1) * 8) {
            if (i == 0) {
              cgs.ftMenuModeEx++;
            }
          }
          if (cgs.ftMenuModeEx) {
            if (i == 9) {
              cgs.ftMenuModeEx--;
            }
          }
          x = CG_PlayerSFFromPos(i, &cgs.ftMenuModeEx);
          if (x != -1) {
            if (doaction) {
              switch (static_cast<FTMenuPos>(cgs.ftMenuPos)) {
                case FTMenuPos::FT_MENUPOS_KICK:
                  trap_SendConsoleCommand(va("fireteam kick %i\n", x));
                  CG_EventHandling(CGAME_EVENT_NONE, qfalse);
                  break;
                case FTMenuPos::FT_MENUPOS_WARN:
                  trap_SendConsoleCommand(va("fireteam warn %i\n", x));
                  CG_EventHandling(CGAME_EVENT_NONE, qfalse);
                  break;
                default:
                  break;
              }
            }

            return qtrue;
          }
          break;
        case FTMenuPos::FT_MENUPOS_RULES:
          if (doaction) {
            switch (static_cast<FTMenuRulesPos>(i)) {
              case FTMenuRulesPos::FT_RULES_RESET:
                trap_SendConsoleCommand("fireteam rules savelimit reset\n");
                CG_EventHandling(CGAME_EVENT_NONE, qfalse);
                break;
              case FTMenuRulesPos::FT_RULES_SAVELIMIT:
                trap_UI_Popup(UIMENU_INGAME_FT_SAVELIMIT);
                CG_EventHandling(CGAME_EVENT_NONE, qfalse);
                break;
              case FTMenuRulesPos::FT_RULES_NOGHOST:
                trap_SendConsoleCommand(
                    va("fireteam rules noghost %i",
                       cgs.clientinfo[cg.clientNum].fireteamData->noGhost ? 0
                                                                          : 1));
                CG_EventHandling(CGAME_EVENT_NONE, qfalse);
                break;
              default:
                break;
            }
          }

          return qtrue;
        default:
          break;
      }
    } break;
  }

  return qfalse;
}
