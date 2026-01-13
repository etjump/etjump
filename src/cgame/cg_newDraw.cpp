
#include "cg_local.h"
#include "etj_rtv_drawable.h"

int CG_DrawField(int x, int y, int width, int value, int charWidth,
                 int charHeight, qboolean dodrawpic,
                 qboolean leftAlign); // NERVE - SMF

int CG_TrimLeftPixels(char *instr, float scale, float w, int size) {
  char buffer[1024];
  char *p, *s;
  int tw;
  int i;

  Q_strncpyz(buffer, instr, 1024);
  memset(instr, 0, size);

  for (i = 0, p = buffer; *p; p++, i++) {
    instr[i] = *p;
    tw = CG_Text_Width(instr, scale, 0);
    if (tw >= w) {
      memset(instr, 0, size);
      for (s = instr, p = &buffer[i + 1]; *p && ((s - instr) < size);
           p++, s++) {
        *s = *p;
      }
      return tw - w;
    }
  }

  return -1;
}

void CG_FitTextToWidth_Ext(char *instr, float scale, float w, int size,
                           fontInfo_t *font) {
  char buffer[1024];
  char *s, *p, *c, *ls;

  if (*instr == '\0') {
    return;
  }

  Q_strncpyz(buffer, instr, 1024);
  memset(instr, 0, size);

  c = s = instr;
  p = buffer;
  ls = NULL;
  while (*p) {
    *c = *p++;

    if (*c == ' ') {
      ls = c;
    } // store last space, to try not to break mid word

    c++;

    if (*p == '\n') {
      s = c + 1;
    } else if (CG_Text_Width_Ext(s, scale, 0, font) > w) {
      if (ls) {
        *ls = '\n';
        s = ls + 1;
      } else {
        *c = *(c - 1);
        *(c - 1) = '\n';
        s = c++;
      }

      ls = NULL;
    }
  }

  if (c != buffer && (*(c - 1) != '\n')) {
    *c++ = '\n';
  }

  *c = '\0';
}

/*
==============
weapIconDrawSize
==============
*/
static int weapIconDrawSize(int weap) {
  switch (weap) {

      // weapons to not draw
      //		case WP_KNIFE:
      //			return 0;

    // weapons with 'wide' icons
    case WP_THOMPSON:
    case WP_MP40:
    case WP_STEN:
    case WP_PANZERFAUST:
    case WP_FLAMETHROWER:
      //		case WP_SPEARGUN:
    case WP_GARAND:
    case WP_FG42:
    case WP_FG42SCOPE:
    case WP_KAR98:
    case WP_GPG40:
    case WP_CARBINE:
    case WP_M7:
    case WP_MOBILE_MG42:
    case WP_MOBILE_MG42_SET:
    case WP_K43:
    case WP_GARAND_SCOPE:
    case WP_K43_SCOPE:
    case WP_MORTAR:
    case WP_MORTAR_SET:
    case WP_PORTAL_GUN:
      return 2;
  }

  return 1;
}

/*
==============
CG_DrawPlayerWeaponIcon
==============
*/
void CG_DrawPlayerWeaponIcon(rectDef_t *rect, qboolean drawHighlighted,
                             int align, const vec4_t *refcolor) {
  int size;
  int realweap; // DHM - Nerve
  qhandle_t icon;
  float scale, halfScale;
  vec4_t hcolor;

  VectorCopy(*refcolor, hcolor);
  hcolor[3] = 1.f;

  if (cg.predictedPlayerEntity.currentState.eFlags & EF_MG42_ACTIVE ||
      cg.predictedPlayerEntity.currentState.eFlags & EF_MOUNTEDTANK) {
    realweap = WP_MOBILE_MG42;
  } else {
    realweap = cg.predictedPlayerState.weapon;
  }

  size = weapIconDrawSize(realweap);

  if (!size) {
    return;
  }

  if (cg.predictedPlayerEntity.currentState.eFlags & EF_MOUNTEDTANK &&
      cg_entities[cg_entities[cg_entities[cg.snap->ps.clientNum].tagParent]
                      .tankparent]
              .currentState.density &
          8) {
    icon = cgs.media.browningIcon;
  } else {
    if (drawHighlighted) {
      // icon = cg_weapons[ realweap ].weaponIcon[1];
      icon = cg_weapons[realweap].weaponIcon[1]; // we don't have icon[0];
    } else {
      icon = cg_weapons[realweap].weaponIcon[1];
    }
  }

  // pulsing grenade icon to help the player 'count' in their head
  if (cg.predictedPlayerState.grenadeTimeLeft) // grenades and dynamite set this
  {                                            // these time differently
    if (realweap == WP_DYNAMITE) {
      if (((cg.grenLastTime) % 1000) >
          ((cg.predictedPlayerState.grenadeTimeLeft) % 1000)) {
        trap_S_StartLocalSoundVControl(cgs.media.grenadePulseSound4,
                                       CHAN_LOCAL_SOUND,
                                       DEFAULT_VOLUME * etj_weaponVolume.value);
      }
    } else {
      if (((cg.grenLastTime) % 1000) <
          ((cg.predictedPlayerState.grenadeTimeLeft) % 1000)) {
        switch (cg.predictedPlayerState.grenadeTimeLeft / 1000) {
          case 3:
            trap_S_StartLocalSound(cgs.media.grenadePulseSound4,
                                   CHAN_LOCAL_SOUND);
            break;
          case 2:
            trap_S_StartLocalSound(cgs.media.grenadePulseSound3,
                                   CHAN_LOCAL_SOUND);
            break;
          case 1:
            trap_S_StartLocalSound(cgs.media.grenadePulseSound2,
                                   CHAN_LOCAL_SOUND);
            break;
          case 0:
            trap_S_StartLocalSound(cgs.media.grenadePulseSound1,
                                   CHAN_LOCAL_SOUND);
            break;
        }
      }
    }

    scale = (float)((cg.predictedPlayerState.grenadeTimeLeft) % 1000) / 100.0f;
    halfScale = scale * 0.5f;

    cg.grenLastTime = cg.predictedPlayerState.grenadeTimeLeft;
  } else {
    scale = halfScale = 0;
  }

  if (etj_HUD_weaponIcon.integer && icon) {
    float x, y, w, h;

    if (size == 1) // draw half width to match the icon asset
    {              // start at left
      x = rect->x - halfScale;
      y = rect->y - halfScale;
      w = rect->w / 2 + scale;
      h = rect->h + scale;

      switch (align) {
        case ITEM_ALIGN_CENTER:
          x += rect->w / 4;
          break;
        case ITEM_ALIGN_RIGHT:
          x += rect->w / 2;
          break;
        case ITEM_ALIGN_LEFT:
        default:
          break;
      }

    } else {
      x = rect->x - halfScale;
      y = rect->y - halfScale;
      w = rect->w + scale;
      h = rect->h + scale;
    }

    CG_DrawWeapHeat(rect, HUD_HORIZONTAL);
    trap_R_SetColor(hcolor); // JPW NERVE
    CG_DrawPic(x, y, w, h, icon);
  }
}

inline constexpr float CURSORHINT_SCALE = 10.0f;

/*
==============
CG_DrawCursorHints

  cg_cursorHints.integer ==
    0:	no hints
    1:	sin size pulse
    2:	one way size pulse
    3:	alpha pulse
    4+:	static image

==============
*/
void CG_DrawCursorhint(rectDef_t *rect) {
  float *color;
  qhandle_t icon;
  float scale, halfscale;
  bool yellowbar = false;

  if (!cg_cursorHints.integer) {
    return;
  }

  CG_CheckForCursorHints();

  switch (cg.cursorHintIcon) {
    case HINT_NONE:
    case HINT_FORCENONE:
      icon = 0;
      break;
    case HINT_DOOR:
      icon = cgs.media.doorHintShader;
      break;
    case HINT_DOOR_ROTATING:
      icon = cgs.media.doorRotateHintShader;
      break;
    case HINT_DOOR_LOCKED:
      icon = cgs.media.doorLockHintShader;
      break;
    case HINT_DOOR_ROTATING_LOCKED:
      icon = cgs.media.doorRotateLockHintShader;
      break;
    case HINT_MG42:
      icon = cgs.media.mg42HintShader;
      break;
    case HINT_BREAKABLE:
      icon = cgs.media.breakableHintShader;
      break;
    case HINT_BREAKABLE_DYNAMITE:
      icon = cgs.media.dynamiteHintShader;
      break;
    case HINT_TANK:
      icon = cgs.media.tankHintShader;
      break;
    case HINT_SATCHELCHARGE:
      icon = cgs.media.satchelchargeHintShader;
      break;
    case HINT_CONSTRUCTIBLE:
      icon = cgs.media.buildHintShader;
      break;
    case HINT_UNIFORM:
      icon = cgs.media.uniformHintShader;
      break;
    case HINT_LANDMINE:
      icon = cgs.media.landmineHintShader;
      break;
    case HINT_CHAIR:
      icon = cgs.media.notUsableHintShader;
      break;
    case HINT_ALARM:
      icon = cgs.media.alarmHintShader;
      break;
    case HINT_HEALTH:
      icon = cgs.media.healthHintShader;
      break;
    case HINT_TREASURE:
      icon = cgs.media.treasureHintShader;
      break;
    case HINT_KNIFE:
      icon = cgs.media.knifeHintShader;
      break;
    case HINT_LADDER:
      icon = cgs.media.ladderHintShader;
      break;
    case HINT_BUTTON:
      icon = cgs.media.buttonHintShader;
      break;
    case HINT_WATER:
      icon = cgs.media.waterHintShader;
      break;
    case HINT_CAUTION:
      icon = cgs.media.cautionHintShader;
      break;
    case HINT_DANGER:
      icon = cgs.media.dangerHintShader;
      break;
    case HINT_SECRET:
      icon = cgs.media.secretHintShader;
      break;
    case HINT_QUESTION:
      icon = cgs.media.qeustionHintShader;
      break;
    case HINT_EXCLAMATION:
      icon = cgs.media.exclamationHintShader;
      break;
    case HINT_CLIPBOARD:
      icon = cgs.media.clipboardHintShader;
      break;
    case HINT_WEAPON:
      icon = cgs.media.weaponHintShader;
      break;
    case HINT_AMMO:
      icon = cgs.media.ammoHintShader;
      break;
    case HINT_ARMOR:
      icon = cgs.media.armorHintShader;
      break;
    case HINT_POWERUP:
      icon = cgs.media.powerupHintShader;
      break;
    case HINT_HOLDABLE:
      icon = cgs.media.holdableHintShader;
      break;
    case HINT_INVENTORY:
      icon = cgs.media.inventoryHintShader;
      break;
    case HINT_PLYR_FRIEND:
      icon = cgs.media.hintPlrFriendShader;
      break;
    case HINT_PLYR_NEUTRAL:
      icon = cgs.media.hintPlrNeutralShader;
      break;
    case HINT_PLYR_ENEMY:
      icon = cgs.media.hintPlrEnemyShader;
      break;
    case HINT_PLYR_UNKNOWN:
      icon = cgs.media.hintPlrUnknownShader;
      break;
    // DHM - Nerve :: multiplayer hints
    case HINT_BUILD:
      icon = cgs.media.buildHintShader;
      break;
    case HINT_DISARM:
      icon = cgs.media.disarmHintShader;
      break;
    case HINT_REVIVE:
      icon = cgs.media.reviveHintShader;
      break;
    case HINT_DYNAMITE:
      icon = cgs.media.dynamiteHintShader;
      break;
    // dhm - end
    // Mad Doc - TDF
    case HINT_LOCKPICK:
      // TAT 1/30/2003 - use the locked door hint cursor
      icon = cgs.media.doorLockHintShader;
      yellowbar = true; // draw the status bar in yellow, so it shows up better
      break;
    case HINT_ACTIVATE:
    case HINT_PLAYER:
    default:
      icon = cgs.media.usableHintShader;
      break;
  }

  if (!icon) {
    return;
  }

  // color
  color = CG_FadeColor(cg.cursorHintTime, cg.cursorHintFade);

  if (!color) {
    trap_R_SetColor(nullptr);
    return;
  }

  if (cg_cursorHints.integer == 3) {
    color[3] *= static_cast<float>(0.5 + 0.5 * std::sin(cg.time / 150.0));
  }

  // size
  // no size pulsing
  if (cg_cursorHints.integer >= 3) {
    scale = halfscale = 0;
  } else {
    if (cg_cursorHints.integer == 2) {
      // one way size pulse
      scale = static_cast<float>((cg.cursorHintTime) % 1000) / 100.0f;
    } else {
      // sin pulse
      scale = CURSORHINT_SCALE * (0.5 + 0.5 * std::sin(cg.time / 150.0));
    }
    halfscale = scale * 0.5f;
  }

  // set color and draw the hint
  trap_R_SetColor(color);
  CG_DrawPic(rect->x - halfscale, rect->y - halfscale, rect->w + scale,
             rect->h + scale, icon);

  trap_R_SetColor(nullptr);

  // draw status bar under the cursor hint
  if (cg.cursorHintValue) {
    if (yellowbar) {
      Vector4Set(color, 1, 1, 0, 1.0f);
    } else {
      Vector4Set(color, 0, 0, 1, 0.5f);
    }

    const int flags =
        (FilledBarFlags::BAR_BORDER_SMALL | FilledBarFlags::BAR_LERP_COLOR);

    CG_FilledBar(rect->x, rect->y + rect->h + 4, rect->w, 8, colorRed,
                 colorGreen, nullptr,
                 static_cast<float>(cg.cursorHintValue) / 255.0f, flags);
  }
}

float CG_GetValue(int ownerDraw, int type) { return -1; }

qboolean CG_OtherTeamHasFlag() { return qfalse; }

qboolean CG_YourTeamHasFlag() { return qfalse; }

// THINKABOUTME: should these be exclusive or inclusive..
//
qboolean CG_OwnerDrawVisible(int flags) { return qfalse; }

/*
==============
CG_DrawWeapStability
    draw a bar showing current stability level (0-255), max at current
weapon/ability, and 'perfect' reference mark

    probably only drawn for scoped weapons
==============
*/
void CG_DrawWeapStability(rectDef_t *rect) {
  vec4_t goodColor = {0, 1, 0, 0.5f};
  const vec4_t badColor = {1, 0, 0, 0.5f};

  if (!cg_drawSpreadScale.integer) {
    return;
  }

  if (cg_drawSpreadScale.integer == 1 &&
      !BG_IsScopedWeapon(cg.predictedPlayerState.weapon)) {
    // cg_drawSpreadScale of '1' means only draw for scoped
    // weapons, '2' means draw all the time (for debugging)
    return;
  }

  if (cg.predictedPlayerState.weaponstate != WEAPON_READY) {
    return;
  }

  if (!(cg.snap->ps.aimSpreadScale)) {
    return;
  }

  if (cg.renderingThirdPerson) {
    return;
  }

  const int flags = (FilledBarFlags::BAR_CENTER | FilledBarFlags::BAR_VERT |
                     FilledBarFlags::BAR_LERP_COLOR);

  CG_FilledBar(rect->x, rect->y, rect->w, rect->h, goodColor, badColor, nullptr,
               static_cast<float>(cg.snap->ps.aimSpreadScale) / 255.0f, flags);
}

/*
==============
CG_DrawWeapHeat
==============
*/
void CG_DrawWeapHeat(rectDef_t *rect, int align) {
  vec4_t color = {1, 0, 0, 0.2f};
  const vec4_t color2 = {1, 0, 0, 0.5f};
  int flags = 0;

  if (!(cg.snap->ps.curWeapHeat)) {
    return;
  }

  if (align != HUD_HORIZONTAL) {
    flags |= FilledBarFlags::BAR_VERT;
  }

  flags |= FilledBarFlags::BAR_LEFT;
  flags |= FilledBarFlags::BAR_BG;
  flags |= FilledBarFlags::BAR_LERP_COLOR;

  CG_FilledBar(rect->x, rect->y, rect->w, rect->h, color, color2, nullptr,
               static_cast<float>(cg.snap->ps.curWeapHeat) / 255.0f, flags);
}

int old_mouse_x_pos = 0, old_mouse_y_pos = 0;

void CG_MouseEvent(int x, int y) {
  switch (cgs.eventHandling) {
    case CGAME_EVENT_DEMO:
    case CGAME_EVENT_SPEAKEREDITOR:
    case CGAME_EVENT_GAMEVIEW:
    case CGAME_EVENT_FIRETEAMMSG:
    case CGAME_EVENT_RTV:
      if (!cgs.demoCam.renderingFreeCam) {
        ETJump::computeCursorPosition(x, y);

        cgs.cursorX = cgDC.cursorx;
        cgs.cursorY = cgDC.cursory;
        cgs.realCursorX = cgDC.realCursorX;
        cgs.realCursorY = cgDC.realCursorY;

        if (cgs.eventHandling == CGAME_EVENT_SPEAKEREDITOR) {
          CG_SpeakerEditorMouseMove_Handling(x, y);
        }
      } else {
        // mouse movement *should* feel the same as in-game
        char buffer[64];
        int mx = 0, my = 0;
        int mouse_x_pos = 0, mouse_y_pos = 0;

        float sensitivity, m_pitch, m_yaw;
        int m_filter = 0;

        if (etj_demo_lookat.integer != -1) {
          return;
        }

        mx += x;
        my += y;

        trap_Cvar_VariableStringBuffer("m_filter", buffer, sizeof(buffer));
        m_filter = Q_atoi(buffer);

        trap_Cvar_VariableStringBuffer("sensitivity", buffer, sizeof(buffer));
        sensitivity = Q_atof(buffer);

        trap_Cvar_VariableStringBuffer("m_pitch", buffer, sizeof(buffer));
        m_pitch = Q_atof(buffer);

        trap_Cvar_VariableStringBuffer("m_yaw", buffer, sizeof(buffer));
        m_yaw = Q_atof(buffer);

        if (m_filter) {
          mouse_x_pos = (mx + old_mouse_x_pos) * 0.5;
          mouse_y_pos = (my + old_mouse_y_pos) * 0.5;
        } else {
          mouse_x_pos = mx;
          mouse_y_pos = my;
        }

        old_mouse_x_pos = mx;
        old_mouse_y_pos = my;

        mouse_x_pos *= sensitivity;
        mouse_y_pos *= sensitivity;

        cg.refdefViewAngles[YAW] -= m_yaw * mouse_x_pos;
        cg.refdefViewAngles[PITCH] += m_pitch * mouse_y_pos;

        if (cg.refdefViewAngles[PITCH] < -90) {
          cg.refdefViewAngles[PITCH] = -90;
        }

        if (cg.refdefViewAngles[PITCH] > 90) {
          cg.refdefViewAngles[PITCH] = 90;
        }
      }

      break;
    default:
      if (cg.snap->ps.pm_type == PM_INTERMISSION) {
        return;
      }

      // default handling
      if ((cg.predictedPlayerState.pm_type == PM_NORMAL ||
           cg.predictedPlayerState.pm_type == PM_SPECTATOR) &&
          cg.showScores == qfalse) {
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_CGAME);
        return;
      }
      break;
  }
}

/*
==================
CG_EventHandling
==================
*/
void CG_EventHandling(int type, qboolean fForced) {
  if (cg.demoPlayback && type == CGAME_EVENT_NONE && !fForced) {
    type = CGAME_EVENT_DEMO;
  }

  if (type != CGAME_EVENT_NONE) {
    trap_Cvar_Set("cl_bypassMouseInput", 0);
  }

  // assume we want cursor visible
  cgDC.cursorVisible = true;

  switch (type) {
    // OSP - Demo support
    case CGAME_EVENT_DEMO:
      cgs.fResize = qfalse;
      cgs.fSelect = qfalse;
      cgs.cursorUpdate = cg.time + 10000;
      cgs.timescaleUpdate = cg.time + 4000;
      CG_ScoresUp_f();
      break;

    case CGAME_EVENT_SPEAKEREDITOR:
    case CGAME_EVENT_GAMEVIEW:
    case CGAME_EVENT_NONE:
    case CGAME_EVENT_FIRETEAMMSG:
    case CGAME_EVENT_RTV:
    default:
      // default handling (cleanup mostly)
      if (cgs.eventHandling == CGAME_EVENT_GAMEVIEW) {
        cg.showGameView = qfalse;
        trap_S_FadeBackgroundTrack(0.0f, 500, 0);

        trap_S_StopStreamingSound(-1);
        cg.limboEndCinematicTime = 0;

        if (fForced) {
          if (cgs.limboLoadoutModified) {
            trap_SendClientCommand("rs");

            cgs.limboLoadoutSelected = qfalse;
          }
        }
      } else if (cgs.eventHandling == CGAME_EVENT_SPEAKEREDITOR) {
        if (type == -CGAME_EVENT_SPEAKEREDITOR) {
          type = CGAME_EVENT_NONE;
        } else {
          trap_Key_SetCatcher(KEYCATCH_CGAME);
          return;
        }
      } else if (cgs.eventHandling == CGAME_EVENT_FIRETEAMMSG) {
        cg.showFireteamMenu = qfalse;
        trap_Cvar_Set("cl_bypassmouseinput", "0");
      } else if (cgs.eventHandling == CGAME_EVENT_RTV) {
        cg.showRtvMenu = false;
        trap_Cvar_Set("cl_bypassmouseinput", "0");
      } else if (cg.snap && cg.snap->ps.pm_type == PM_INTERMISSION && fForced) {
        trap_UI_Popup(UIMENU_INGAME);
      }

      break;
  }

  cgs.eventHandling = type;

  if (type == CGAME_EVENT_NONE) {
    trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_CGAME);
    ccInitial = qfalse;
    if (cg.demoPlayback && cg.demohelpWindow != SHOW_OFF) {
      CG_ShowHelp_Off(&cg.demohelpWindow);
    }
  } else if (type == CGAME_EVENT_GAMEVIEW) {
    cg.showGameView = qtrue;
    CG_LimboPanel_Setup();
    trap_Key_SetCatcher(KEYCATCH_CGAME);
  } else if (type == CGAME_EVENT_FIRETEAMMSG) {
    cgs.ftMenuPos = static_cast<int>(FTMenuPos::FT_MENUPOS_NONE);
    cgs.ftMenuMode = static_cast<int>(FTMenuMode::FT_VSAY);
    cg.showFireteamMenu = qtrue;
    cgDC.cursorVisible = false;
    trap_Cvar_Set("cl_bypassmouseinput", "1");
    trap_Key_SetCatcher(KEYCATCH_CGAME);
  } else if (type == CGAME_EVENT_RTV) {
    cg.showRtvMenu = true;
    cgDC.cursorVisible = false;
    trap_Cvar_Set("cl_bypassmouseinput", "1");
    trap_Key_SetCatcher(KEYCATCH_CGAME);
  } else {
    trap_Key_SetCatcher(KEYCATCH_CGAME);
  }
}

void CG_KeyEvent(int key, qboolean down) {
  switch (cgs.eventHandling) {
    // Demos get their own keys
    case CGAME_EVENT_DEMO:
      if (etj_predefineddemokeys.integer) {
        CG_DemoClick(key, down);
      } else {
        CG_RunBinding(key, down);
      }
      return;

    case CGAME_EVENT_FIRETEAMMSG:
      CG_Fireteams_KeyHandling(key, down);
      break;

    case CGAME_EVENT_GAMEVIEW:
      CG_LimboPanel_KeyHandling(key, down);
      break;

    case CGAME_EVENT_SPEAKEREDITOR:
      CG_SpeakerEditor_KeyHandling(key, down);
      break;

    case CGAME_EVENT_RTV:
      ETJump::RtvDrawable::keyHandling(key, down);
      break;

    default:
      if (cg.snap->ps.pm_type == PM_INTERMISSION) {
        return;
      }

      // default handling
      if (!down) {
        return;
      }

      if ((cg.predictedPlayerState.pm_type == PM_NORMAL ||
           (cg.predictedPlayerState.pm_type == PM_SPECTATOR &&
            cg.showScores == qfalse))) {

        CG_EventHandling(CGAME_EVENT_NONE, qfalse);
        return;
      }
      break;
  }
}

int CG_ClientNumFromName(const char *p) {
  int i;
  for (i = 0; i < cgs.maxclients; i++) {
    if (cgs.clientinfo[i].infoValid &&
        Q_stricmp(cgs.clientinfo[i].name, p) == 0) {
      return i;
    }
  }
  return -1;
}

void CG_GetTeamColor(vec4_t *color) {
  if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_AXIS) {
    (*color)[0] = 1;
    (*color)[3] = .25f;
    (*color)[1] = (*color)[2] = 0;
  } else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_ALLIES) {
    (*color)[0] = (*color)[1] = 0;
    (*color)[2] = 1;
    (*color)[3] = .25f;
  } else {
    (*color)[0] = (*color)[2] = 0;
    (*color)[1] = .17f;
    (*color)[3] = .25f;
  }
}

void CG_RunMenuScript(const char **args) {}
