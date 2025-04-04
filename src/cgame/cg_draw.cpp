// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include <algorithm>

#include "cg_local.h"
#include "etj_utilities.h"
#include "etj_crosshair.h"
#include "etj_client_rtv_handler.h"
#include "etj_demo_compatibility.h"
#include "etj_cvar_parser.h"

#include "../game/etj_string_utilities.h"

char *BindingFromName(const char *cvar);
void Controls_GetConfig(void);
void SetHeadOrigin(clientInfo_t *ci, playerInfo_t *pi);
void CG_DrawOverlays();
int activeFont;

////////////////////////
////////////////////////
////// new hud stuff
///////////////////////
///////////////////////

void CG_Text_SetActiveFont(int font) { activeFont = font; }

int CG_Text_Width_Ext(const char *text, float scale, int limit,
                      fontInfo_t *font) {
  int count, len;
  glyphInfo_t *glyph;
  const char *s = text;
  float out, useScale = scale * font->glyphScale;

  out = 0;
  if (text) {
    len = strlen(text);
    if (limit > 0 && len > limit) {
      len = limit;
    }
    count = 0;
    while (s && *s && count < len) {
      if (Q_IsColorString(s)) {
        s += 2;
        continue;
      } else {
        glyph = &font->glyphs[(unsigned char)*s];
        out += glyph->xSkip;
        s++;
        count++;
      }
    }
  }

  return out * useScale;
}

int CG_Text_Width_Ext(const std::string &text, float scale, int limit,
                      fontInfo_t *font) {
  return CG_Text_Width_Ext(text.c_str(), scale, limit, font);
}

int CG_Text_Width(const char *text, float scale, int limit) {
  fontInfo_t *font = &cgDC.Assets.fonts[activeFont];

  return CG_Text_Width_Ext(text, scale, limit, font);
}

int CG_Text_Height_Ext(const char *text, float scale, int limit,
                       fontInfo_t *font) {
  int len, count;
  float max;
  glyphInfo_t *glyph;
  float useScale;
  const char *s = text;

  useScale = scale * font->glyphScale;
  max = 0;
  if (text) {
    len = strlen(text);
    if (limit > 0 && len > limit) {
      len = limit;
    }
    count = 0;
    while (s && *s && count < len) {
      if (Q_IsColorString(s)) {
        s += 2;
        continue;
      } else {
        glyph = &font->glyphs[(unsigned char)*s];
        if (max < glyph->height) {
          max = glyph->height;
        }
        s++;
        count++;
      }
    }
  }
  return max * useScale;
}

int CG_Text_Height_Ext(const std::string &text, float scale, int limit,
                       fontInfo_t *font) {
  return CG_Text_Height_Ext(text.c_str(), scale, limit, font);
}

int CG_Text_Height(const char *text, float scale, int limit) {
  fontInfo_t *font = &cgDC.Assets.fonts[activeFont];

  return CG_Text_Height_Ext(text, scale, limit, font);
}

void CG_Text_PaintChar_Ext(float x, float y, float w, float h, float scalex,
                           float scaley, float s, float t, float s2, float t2,
                           qhandle_t hShader) {
  w *= scalex;
  h *= scaley;
  CG_AdjustFrom640(&x, &y, &w, &h);

  if (w < 1) {
    w = 1;
  }

  if (h < 1) {
    h = 1;
  }

  trap_R_DrawStretchPic(x, y, w, h, s, t, s2, t2, hShader);
}

void CG_Text_PaintChar(float x, float y, float width, float height, float scale,
                       float s, float t, float s2, float t2,
                       qhandle_t hShader) {
  float w, h;
  w = width * scale;
  h = height * scale;
  CG_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, w, h, s, t, s2, t2, hShader);
}

void CG_Text_Paint_Centred_Ext(float x, float y, float scalex, float scaley,
                               vec4_t color, const char *text, float adjust,
                               int limit, int style, fontInfo_t *font) {
  x -= CG_Text_Width_Ext(text, scalex, limit, font) * 0.5f;

  CG_Text_Paint_Ext(x, y, scalex, scaley, color, text, adjust, limit, style,
                    font);
}

void CG_Text_Paint_Centred_Ext(float x, float y, float scalex, float scaley,
                               vec4_t color, const std::string &text,
                               float adjust, int limit, int style,
                               fontInfo_t *font) {
  x -= CG_Text_Width_Ext(text, scalex, limit, font) * 0.5f;

  CG_Text_Paint_Ext(x, y, scalex, scaley, color, text, adjust, limit, style,
                    font);
}

void CG_Text_Paint_RightAligned_Ext(float x, float y, float scalex,
                                    float scaley, vec4_t color,
                                    const char *text, float adjust, int limit,
                                    int style, fontInfo_t *font) {
  x -= CG_Text_Width_Ext(text, scalex, limit, font);

  CG_Text_Paint_Ext(x, y, scalex, scaley, color, text, adjust, limit, style,
                    font);
}

void CG_Text_Paint_RightAligned_Ext(float x, float y, float scalex,
                                    float scaley, vec4_t color,
                                    const std::string &text, float adjust,
                                    int limit, int style, fontInfo_t *font) {
  x -= CG_Text_Width_Ext(text, scalex, limit, font);

  CG_Text_Paint_Ext(x, y, scalex, scaley, color, text, adjust, limit, style,
                    font);
}

void CG_Text_Paint_Ext(float x, float y, float scalex, float scaley,
                       vec4_t color, const char *text, float adjust, int limit,
                       int style, fontInfo_t *font) {
  int len, count;
  vec4_t newColor;
  glyphInfo_t *glyph;

  scalex *= font->glyphScale;
  scaley *= font->glyphScale;

  if (text) {
    const char *s = text;
    trap_R_SetColor(color);
    memcpy(&newColor[0], &color[0], sizeof(vec4_t));
    len = strlen(text);
    if (limit > 0 && len > limit) {
      len = limit;
    }
    count = 0;
    while (s && *s && count < len) {
      glyph = &font->glyphs[(unsigned char)*s];
      if (Q_IsColorString(s)) {
        if (*(s + 1) == COLOR_NULL) {
          memcpy(newColor, color, sizeof(newColor));
        } else {
          memcpy(newColor, g_color_table[ColorIndex(*(s + 1))],
                 sizeof(newColor));
          newColor[3] = color[3];
        }
        trap_R_SetColor(newColor);
        s += 2;
        continue;
      } else {
        float yadj = scaley * glyph->top;
        if (style == ITEM_TEXTSTYLE_SHADOWED) {
          constexpr float ofs = 2.5f;
          colorBlack[3] = newColor[3];
          trap_R_SetColor(colorBlack);
          CG_Text_PaintChar_Ext(x + (glyph->pitch * scalex) + ofs * scalex,
                                y - yadj + ofs * scaley, glyph->imageWidth,
                                glyph->imageHeight, scalex, scaley, glyph->s,
                                glyph->t, glyph->s2, glyph->t2, glyph->glyph);
          colorBlack[3] = 1.0;
          trap_R_SetColor(newColor);
        }
        CG_Text_PaintChar_Ext(x + (glyph->pitch * scalex), y - yadj,
                              glyph->imageWidth, glyph->imageHeight, scalex,
                              scaley, glyph->s, glyph->t, glyph->s2, glyph->t2,
                              glyph->glyph);
        x += (glyph->xSkip * scalex) + adjust;
        s++;
        count++;
      }
    }
    trap_R_SetColor(NULL);
  }
}

void CG_Text_Paint_Ext(float x, float y, float scalex, float scaley,
                       vec4_t color, const std::string &text, float adjust,
                       int limit, int style, fontInfo_t *font) {
  CG_Text_Paint_Ext(x, y, scalex, scaley, color, text.c_str(), adjust, limit,
                    style, font);
}

void CG_Text_Paint(float x, float y, float scale, vec4_t color,
                   const char *text, float adjust, int limit, int style) {
  fontInfo_t *font = &cgDC.Assets.fonts[activeFont];

  CG_Text_Paint_Ext(x, y, scale, scale, color, text, adjust, limit, style,
                    font);
}

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model,
                    qhandle_t skin, vec3_t origin, vec3_t angles) {
  refdef_t refdef;
  refEntity_t ent;

  CG_AdjustFrom640(&x, &y, &w, &h);

  memset(&refdef, 0, sizeof(refdef));

  memset(&ent, 0, sizeof(ent));
  AnglesToAxis(angles, ent.axis);
  VectorCopy(origin, ent.origin);
  ent.hModel = model;
  ent.customSkin = skin;
  ent.renderfx = RF_NOSHADOW; // no stencil shadows

  refdef.rdflags = RDF_NOWORLDMODEL;

  AxisClear(refdef.viewaxis);

  refdef.fov_x = 30;
  refdef.fov_y = 30;

  refdef.x = x;
  refdef.y = y;
  refdef.width = w;
  refdef.height = h;

  refdef.time = cg.time;

  trap_R_ClearScene();
  trap_R_AddRefEntityToScene(&ent);
  trap_R_RenderScene(&refdef);
}

/*
==============
CG_DrawKeyModel
==============
*/
void CG_DrawKeyModel(int keynum, float x, float y, float w, float h,
                     int fadetime) {
  qhandle_t cm;
  float len;
  vec3_t origin, angles;
  vec3_t mins, maxs;

  VectorClear(angles);

  cm = cg_items[keynum].models[0];

  // offset the origin y and z to center the model
  trap_R_ModelBounds(cm, mins, maxs);

  origin[2] = -0.5 * (mins[2] + maxs[2]);
  origin[1] = 0.5 * (mins[1] + maxs[1]);

  //	len = 0.5 * ( maxs[2] - mins[2] );
  len = 0.75 * (maxs[2] - mins[2]);
  origin[0] = len / 0.268; // len / tan( fov/2 )

  angles[YAW] = 30 * sin(cg.time / 2000.0);
  ;

  CG_Draw3DModel(x, y, w, h, cg_items[keynum].models[0], 0, origin, angles);
}

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground(int x, int y, int w, int h, float alpha, int team) {
  vec4_t hcolor;

  hcolor[3] = alpha;
  if (team == TEAM_AXIS) {
    hcolor[0] = 1;
    hcolor[1] = 0;
    hcolor[2] = 0;
  } else if (team == TEAM_ALLIES) {
    hcolor[0] = 0;
    hcolor[1] = 0;
    hcolor[2] = 1;
  } else {
    return;
  }
  trap_R_SetColor(hcolor);
  CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
  trap_R_SetColor(NULL);
}

/*
===========================================================================================

  LOWER RIGHT CORNER

===========================================================================================
*/

inline constexpr float CHATLOC_X = 160.0f;
inline constexpr float CHATLOC_Y = 478.0f;
inline constexpr float CHATLOC_TEXT_X = CHATLOC_X + 0.25f * TINYCHAR_WIDTH;

// Calculate chat background width based on etj_chatWidth value
float calcBackgroundWidth(int maxChars, float fontScale, fontInfo_t *font) {
  return maxChars * fontScale * font->glyphScale * font->glyphs[0].xSkip;
}

/*
=================
CG_DrawTeamInfo
=================
*/
static void CG_DrawTeamInfo() {
  const auto scale = ETJump::CvarValueParser::parse<ETJump::CvarValue::Scale>(
      etj_chatScale, 0, 5);
  const float fontSizeX = 0.2f * scale.x;
  const float fontSizeY = 0.2f * scale.y;
  const float lineHeight = 9.0f * scale.y;

  const int maxLineLength =
      std::clamp(etj_chatLineWidth.integer, 1, TEAMCHAT_WIDTH);
  const auto chatWidth = static_cast<int>(
      calcBackgroundWidth(maxLineLength, fontSizeX, &cgs.media.limboFont2) + 5);
  const int chatHeight =
      std::clamp(cg_teamChatHeight.integer, 0, TEAMCHAT_HEIGHT);

  int textStyle = ITEM_TEXTSTYLE_NORMAL;
  float textAlpha = etj_chatAlpha.value;

  if (etj_chatShadow.integer > 0) {
    textStyle = ITEM_TEXTSTYLE_SHADOWED;
  }

  if (textAlpha > 1.0) {
    textAlpha = 1.0;
  } else if (textAlpha < 0.0) {
    textAlpha = 0.0;
  }
  if (chatHeight <= 0) {
    return; // disabled
  }

  if (cgs.teamLastChatPos != cgs.teamChatPos) {
    vec4_t hcolor;
    int i;
    qhandle_t flag;

    if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] >
        cg_teamChatTime.integer) {
      cgs.teamLastChatPos++;
    }

    int w = 0;

    for (i = cgs.teamLastChatPos; i < cgs.teamChatPos; i++) {
      const int len = CG_Text_Width_Ext(cgs.teamChatMsgs[i % chatHeight],
                                        fontSizeX, 0, &cgs.media.limboFont2);
      if (len > w) {
        w = len;
      }
    }

    w *= TINYCHAR_WIDTH;
    w += TINYCHAR_WIDTH * 2;

    for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--) {
      auto linePosX = CHATLOC_TEXT_X + etj_chatPosX.value;
      const auto linePosY =
          CHATLOC_Y + etj_chatPosY.value -
          static_cast<float>(cgs.teamChatPos - i - 1) * lineHeight - 1;

      if (linePosY <= 0 || linePosY >= 480 + lineHeight) {
        continue;
      }

      const float alphapercent = std::clamp(
          1.0f - static_cast<float>(cg.time -
                                    cgs.teamChatMsgTimes[i % chatHeight]) /
                     static_cast<float>(cg_teamChatTime.integer),
          0.0f, 1.0f);

      if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_AXIS) {
        hcolor[0] = 1;
        hcolor[1] = 0;
        hcolor[2] = 0;
      } else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_ALLIES) {
        hcolor[0] = 0;
        hcolor[1] = 0;
        hcolor[2] = 1;
      } else {
        hcolor[0] = 0;
        hcolor[1] = 1;
        hcolor[2] = 0;
      }

      const float chatbgalpha =
          std::clamp(etj_chatBackgroundAlpha.value, 0.0f, 1.0f);

      hcolor[3] = chatbgalpha * alphapercent;

      trap_R_SetColor(hcolor);
      CG_DrawPic(CHATLOC_X + etj_chatPosX.value,
                 CHATLOC_Y + etj_chatPosY.value -
                     static_cast<float>(cgs.teamChatPos - i) * lineHeight,
                 static_cast<float>(chatWidth), lineHeight,
                 cgs.media.teamStatusBar);

      hcolor[0] = hcolor[1] = hcolor[2] = 1.0;
      hcolor[3] = alphapercent * textAlpha;
      trap_R_SetColor(hcolor);

      if (etj_chatFlags.integer) {
        if (cgs.teamChatMsgTeams[i % chatHeight] == TEAM_AXIS) {
          flag = cgs.media.axisFlag;
        } else if (cgs.teamChatMsgTeams[i % chatHeight] == TEAM_ALLIES) {
          flag = cgs.media.alliedFlag;
        } else {
          flag = 0;
        }

        if (flag) {
          const float flagScaleX = 12.0f * scale.x;
          const float flagScaleY = 9.0f * scale.y;
          const float flagPosX =
              CHATLOC_TEXT_X + etj_chatPosX.value - 13 * scale.x;
          const float flagPosY =
              CHATLOC_Y + etj_chatPosY.value -
              static_cast<float>(cgs.teamChatPos - i - 1) * lineHeight -
              9 * scale.y;
          CG_DrawPic(flagPosX, flagPosY, flagScaleX, flagScaleY, flag);
        }
      }

      CG_Text_Paint_Ext(linePosX, linePosY - 0.5f, fontSizeX, fontSizeY, hcolor,
                        cgs.teamChatMsgs[i % chatHeight], 0, 0, textStyle,
                        &cgs.media.limboFont2);
    }
  }
}

const char *CG_PickupItemText(int item) {
  if (bg_itemlist[item].giType == IT_HEALTH) {
    if (bg_itemlist[item].world_model[2]) // this is a multi-stage item
    { // FIXME: print the correct amount for multi-stage
      return va("a %s", bg_itemlist[item].pickup_name);
    } else {
      return va("%i %s", bg_itemlist[item].quantity,
                bg_itemlist[item].pickup_name);
    }
  } else if (bg_itemlist[item].giType == IT_TEAM) {
    return "an Objective";
  } else {
    if (bg_itemlist[item].pickup_name[0] == 'a' ||
        bg_itemlist[item].pickup_name[0] == 'A') {
      return va("an %s", bg_itemlist[item].pickup_name);
    } else {
      // hack for allies grenade
      // FIXME: should edit animations/scripts/human_base.script instead
      //  but need to make sure it doesn't cause compatibility issues in demos
      if (bg_itemlist[item].giTag == WP_GRENADE_PINEAPPLE) {
        return "a Grenade";
      }

      return va("a %s", bg_itemlist[item].pickup_name);
    }
  }
}

/*
===============================================================================

LAGOMETER

===============================================================================
*/

// lagometer sample count, enough for ~5ms server frame intervals
inline constexpr int LAG_SAMPLES = 1024;
inline constexpr int LAG_PERIOD = 5000;

struct sample_t {
  int elapsed;
  int time;
};

struct sampledStat_t {
  unsigned int count;
  int avg;
  int lastSampleTime;
  int samplesTotalElapsed;

  sample_t samples[LAG_SAMPLES];
};

static sampledStat_t sampledStat;

typedef struct {
  int frameSamples[LAG_SAMPLES];
  int frameCount;
  int snapshotFlags[LAG_SAMPLES];
  int snapshotSamples[LAG_SAMPLES];
  int snapshotCount;
} lagometer_t;

static lagometer_t lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo(void) {
  int offset;

  offset = cg.time - cg.latestSnapshotTime;
  lagometer.frameSamples[lagometer.frameCount & (LAG_SAMPLES - 1)] = offset;
  lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo(snapshot_t *snap) {
  unsigned int index = lagometer.snapshotCount & (LAG_SAMPLES - 1);

  // dropped packet
  if (!snap) {
    lagometer.snapshotSamples[index] = -1;
    lagometer.snapshotCount++;
    return;
  }

  // add this snapshot's info
  // demo playback displays snapshot delta values instead of ping (ala ETPro)
  // https://bani.anime.net/banimod/forums/viewtopic.php?t=6381
  if (cg.demoPlayback) {
    static int lastUpdate = 0;
    const int interval = 1000 / ETJump::getSvFps();

    snap->ping = (snap->serverTime - snap->ps.commandTime) - interval;
    lagometer.snapshotSamples[index] = snap->serverTime - lastUpdate;
    lastUpdate = snap->serverTime;
  } else {
    lagometer.snapshotSamples[index] = snap->ping;
  }

  lagometer.snapshotFlags[index] = snap->snapFlags;
  lagometer.snapshotCount++;

  // calculate received snapshots
  index = sampledStat.count;

  if (sampledStat.count < LAG_SAMPLES) {
    sampledStat.count++;
  } else {
    index--;
  }

  sampledStat.samples[index].elapsed =
      snap->serverTime - sampledStat.lastSampleTime;
  sampledStat.samples[index].elapsed =
      std::max(sampledStat.samples[index].elapsed, 0);

  sampledStat.samples[index].time = snap->serverTime;
  sampledStat.lastSampleTime = snap->serverTime;
  sampledStat.samplesTotalElapsed += sampledStat.samples[index].elapsed;

  const int oldest = snap->serverTime - LAG_PERIOD;

  for (index = 0; index < sampledStat.count; index++) {
    if (sampledStat.samples[index].time > oldest) {
      break;
    }

    sampledStat.samplesTotalElapsed -= sampledStat.samples[index].elapsed;
  }

  if (index) {
    std::memmove(sampledStat.samples, sampledStat.samples + index,
                 sizeof(sample_t) * (sampledStat.count - index));
    sampledStat.count -= index;
  }

  sampledStat.avg =
      sampledStat.samplesTotalElapsed > 0
          ? static_cast<int>(std::round(
                static_cast<float>(sampledStat.count) /
                (static_cast<float>(sampledStat.samplesTotalElapsed) /
                 1000.0f)))
          : 0;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect(void) {
  float x, y;
  int cmdNum;
  usercmd_t cmd;
  const char *s;
  int w; // bk010215 - FIXME char message[1024];

  if (etj_drawConnectionIssues.integer == 0) {
    return;
  }
  // OSP - dont draw if a demo and we're running at a different
  // timescale
  if (cg.demoPlayback && cg_timescale.value != 1.0f) {
    return;
  }

  // ydnar: don't draw if the server is respawning
  if (cg.serverRespawning) {
    return;
  }

  // draw the phone jack if we are completely past our buffers
  cmdNum = trap_GetCurrentCmdNumber() - cg.cmdBackup + 1;
  trap_GetUserCmd(cmdNum, &cmd);
  if (cmd.serverTime <= cg.snap->ps.commandTime ||
      cmd.serverTime > cg.time) // special check for map_restart // bk
                                // 0102165 - FIXME
  {
    return;
  }

  // also add text in center of screen
  s = CG_TranslateString("Connection Interrupted"); // bk 010215 - FIXME
  w = ETJump::DrawStringWidth(s, 0.3f);
  ETJump::DrawBigString(SCREEN_CENTER_X - w / 2, 100, s, 1.0F);

  // blink the icon
  if ((cg.time >> 9) & 1) {
    return;
  }

  x = SCREEN_WIDTH - 48 + ETJump_AdjustPosition(etj_lagometerX.value);
  y = 480 - 200 + etj_lagometerY.value;

  CG_DrawPic(x, y, 48, 48, cgs.media.disconnectIcon);
}

inline constexpr int MAX_LAGOMETER_PING = 900;
inline constexpr int MAX_LAGOMETER_RANGE = 300;

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer() {
  int a, x, y, i;
  float v;
  float ax, ay, aw, ah, mid, range;
  int color;
  float vscale;

  if (!cg_lagometer.integer || (cg_lagometer.integer == 1 && cgs.localServer)) {
    CG_DrawDisconnect();
    return;
  }

  //
  // draw the graph
  //
  x = SCREEN_WIDTH - 48 + ETJump_AdjustPosition(etj_lagometerX.value);
  y = 480 - 200 + etj_lagometerY.value;

  const float alpha = std::clamp(etj_lagometerAlpha.value, 0.0f, 1.0f);

  if (etj_lagometerShader.integer) {
    vec4_t mainColor;
    Vector4Copy(colorWhite, mainColor);
    mainColor[3] *= alpha;

    ETJump::drawPic(x, y, 48, 48, cgs.media.lagometerShader, mainColor);
  } else {
    vec4_t borderColor = {0.5f, 0.5f, 0.5f, 0.5f};
    vec4_t backgroundColor = {0.16f, 0.2f, 0.17f, 0.8f};

    borderColor[3] *= alpha;
    backgroundColor[3] *= alpha;

    CG_FillRect(x, y, 48, 48, backgroundColor);
    CG_DrawRect_FixedBorder(x, y, 48, 48, 1, borderColor);
  }

  ax = x;
  ay = y;
  aw = 48;
  ah = 48;
  CG_AdjustFrom640(&ax, &ay, &aw, &ah);

  color = -1;
  range = ah / 3;
  mid = ay + range;

  vscale = range / MAX_LAGOMETER_RANGE;

  // draw the frame interpoalte / extrapolate graph
  for (a = 0; a < aw; a++) {
    i = (lagometer.frameCount - 1 - a) & (LAG_SAMPLES - 1);
    v = lagometer.frameSamples[i];
    v *= vscale;
    if (v > 0) {
      if (color != 1) {
        color = 1;
        trap_R_SetColor(colorYellow);
      }
      if (v > range) {
        v = range;
      }
      trap_R_DrawStretchPic(ax + aw - a, mid - v, 1, v, 0, 0, 0, 0,
                            cgs.media.whiteShader);
    } else if (v < 0) {
      if (color != 2) {
        color = 2;
        trap_R_SetColor(colorBlue);
      }
      v = -v;
      if (v > range) {
        v = range;
      }
      trap_R_DrawStretchPic(ax + aw - a, mid, 1, v, 0, 0, 0, 0,
                            cgs.media.whiteShader);
    }
  }

  // draw the snapshot latency / drop graph
  range = ah / 2;
  vscale = range / MAX_LAGOMETER_PING;

  for (a = 0; a < aw; a++) {
    i = (lagometer.snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
    v = lagometer.snapshotSamples[i];
    if (v > 0) {
      if (lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED) {
        if (color != 5) {
          color = 5; // YELLOW for rate
                     // delay
          trap_R_SetColor(colorYellow);
        }
      } else {
        if (color != 3) {
          color = 3;
          trap_R_SetColor(colorGreen);
        }
      }
      v = v * vscale;
      if (v > range) {
        v = range;
      }
      trap_R_DrawStretchPic(ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0,
                            cgs.media.whiteShader);
    } else if (v < 0) {
      if (color != 4) {
        color = 4; // RED for dropped snapshots
        trap_R_SetColor(colorRed);
      }
      trap_R_DrawStretchPic(ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0,
                            cgs.media.whiteShader);
    }
  }

  trap_R_SetColor(nullptr);

  if (cg_nopredict.integer
#ifdef ALLOW_GSYNC
      || cgs.synchronousClients
#endif // ALLOW_GSYNC
  ) {
    CG_DrawBigString(ax, ay, "snc", 1.0);
  }

  // snapshot display
  vec4_t textColor = {0.625f, 0.625f, 0.6f, 1.0f};
  const auto fps = static_cast<float>(ETJump::getSvFps());

  const size_t pad = std::strlen(std::to_string(static_cast<int>(fps)).c_str());

  // server snapshot rate (sv_fps)
  std::string svStr = ETJump::stringFormat(
      "sv: %*s", pad, std::to_string(static_cast<int>(fps)));
  CG_Text_Paint_Ext(x + 2, y + 13, 0.16f, 0.16f, textColor, svStr.c_str(), 0, 0,
                    ITEM_TEXTSTYLE_NORMAL, &cgs.media.limboFont2);

  // client received snapshots
  const auto avg = static_cast<float>(sampledStat.avg);

  if (avg < fps * 0.5f) {
    Vector4Copy(colorRed, textColor);
  } else if (avg < fps * 0.75f) {
    Vector4Copy(colorYellow, textColor);
  }

  std::string clStr = ETJump::stringFormat(
      "cl: %*s", pad, std::to_string(static_cast<int>(avg)));
  CG_Text_Paint_Ext(x + 2, y + 7, 0.16f, 0.16f, textColor, clStr.c_str(), 0, 0,
                    ITEM_TEXTSTYLE_NORMAL, &cgs.media.limboFont2);

  CG_DrawDisconnect();
}

/*
==============
CG_BannerPrint

Called for server banner messages that stay in the top center of the screen
for a few moments
==============
*/

inline constexpr int BP_LINEWIDTH = 80;
inline constexpr int BP_TIME = 10000;

void CG_BannerPrint(const char *str) {
  char buff[MAX_STRING_CHARS];
  int i, len, textlen;
  qboolean neednewline = qfalse;

  Q_strncpyz(cg.bannerPrint, str, sizeof(cg.bannerPrint));

  // turn spaces into newlines, if we've run over the linewidth
  len = strlen(cg.bannerPrint);
  for (i = 0, textlen = 0; i < len; i++, textlen++) {

    // "\n" in center/banner prints are seen as new lines.
    // kw: this is also done serverside in etpub
    if (cg.bannerPrint[i] == '\\' && cg.bannerPrint[i + 1] == 'n') {
      Q_strncpyz(buff, &cg.bannerPrint[i + 2], sizeof(buff));
      cg.bannerPrint[i] = '\n';
      cg.bannerPrint[i + 1] = 0;
      Q_strcat(cg.bannerPrint, sizeof(cg.bannerPrint), buff);
    }
    if (cg.bannerPrint[i] == '\n') {
      textlen = 0;
    }

    if (Q_IsColorString(&cg.bannerPrint[i])) {
      textlen -= 2;
    }

    // NOTE: subtracted a few chars here so long words still
    // get displayed properly
    if (textlen % (BP_LINEWIDTH - 10) == 0 && textlen > 0) {
      neednewline = qtrue;
    }

    if (cg.bannerPrint[i] == ' ' && neednewline) {
      cg.bannerPrint[i] = '\n';
      textlen = 0;
      neednewline = qfalse;
    }

    // if still to long just cut it at BP_LINEWIDTH
    if (textlen % BP_LINEWIDTH == 0 && textlen > 0) {
      Q_strncpyz(buff, &cg.bannerPrint[i], sizeof(buff));
      cg.bannerPrint[i] = '\n';
      cg.bannerPrint[i + 1] = 0;
      Q_strcat(cg.bannerPrint, sizeof(cg.bannerPrint), buff);
      textlen = 0;
      neednewline = qfalse;
    }
  }

  // post-editing to print text correctly into the console
  for (i = 0, len = 0; i < static_cast<int>(strlen(cg.bannerPrint)); i++) {
    // replace newlines with spaces
    if (cg.bannerPrint[i] == '\n') {
      if (len != 0 && buff[len - 1] != ' ') {
        buff[len] = ' ';
        len++;
      }
      continue;
    }
    // no spaces at the beginning of the string
    if (len == 0 && cg.bannerPrint[i] == ' ') {
      continue;
    }
    buff[len] = cg.bannerPrint[i];
    len++;
  }
  buff[len] = 0;

  // show the banner in the console
  // Dens: only if the client wants that
  if (etj_logBanner.integer & CONLOG_BANNERPRINT) {
    CG_Printf("^9banner: ^7%s\n", buff);
  }

  cg.bannerPrintTime = cg.time;
}

static void CG_DrawBannerPrint(void) {
  char *start;
  int l;
  int x, y, w;
  float *color;
  char lastcolor = COLOR_WHITE;
  int charHeight;

  if (!cg.bannerPrintTime) {
    return;
  }

  if (!etj_drawBanners.integer) {
    return;
  }

  color = CG_FadeColor(cg.bannerPrintTime, BP_TIME);
  if (!color) {
    cg.bannerPrintTime = 0;
    return;
  }

  trap_R_SetColor(color);

  start = cg.bannerPrint;

  y = 0;
  charHeight = CG_Text_Height_Ext("A", 0.25, 0, &cgs.media.limboFont1);

  while (1) {
    char linebuffer[1024];
    char colorchar = lastcolor;

    for (l = 0; l < static_cast<int>(strlen(cg.bannerPrint)); l++) {
      if (!start[l] || start[l] == '\n') {
        break;
      }
      if (Q_IsColorString(&start[l])) {
        lastcolor = start[l + 1];
      }
      linebuffer[l] = start[l];
    }
    linebuffer[l] = 0;

    // w = MINICHAR_WIDTH * CG_DrawStrlen( linebuffer );
    w = CG_Text_Width_Ext(linebuffer, 0.25, 0, &cgs.media.limboFont1);

    x = SCREEN_CENTER_X - w / 2;

    // colors are saved over newlines in bannerprint messages.
    // CG_DrawStringExt( x, y, va( "^%c%s", colorchar,
    // linebuffer) , color,
    //		qfalse, qtrue, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0
    //);
    y += charHeight * 1.5;
    CG_Text_Paint_Ext(x, y, 0.25, 0.25, color,
                      va("^%c%s", colorchar, linebuffer), 0, 0,
                      ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont1);
    // y += MINICHAR_HEIGHT;

    while (*start && (*start != '\n')) {
      start++;
    }
    if (!*start) {
      break;
    }
    start++;
  }

  trap_R_SetColor(NULL);
} // bannerprinting

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

namespace ETJump {
void logCenterPrint() {
  std::string msg = cg.centerPrint;

  // we don't want newlines in the console
  // this is called before center print gets automatically word wrapped
  // to fit on screen properly, so it's fairly safe to assume that
  // explicit newlines should be replaced with whitespace
  StringUtil::replaceAll(msg, "\n", " ");

  // it's possible to send an empty center print/only whitespace to "clear"
  // whatever is being displayed, but we don't want to log that obviously
  if (msg.empty() || std::all_of(msg.begin(), msg.end(),
                                 [](const char c) { return isspace(c); })) {
    return;
  }

  // don't log consecutive messages that are being re-triggered
  if (cg.time + cg_centertime.integer * 1000 > cg.lastCenterPrintLogTime &&
      Q_stricmp(msg.c_str(), cg.lastLoggedCenterPrint)) {
    cg.lastCenterPrintLogTime = cg.time;
    Q_strncpyz(cg.lastLoggedCenterPrint, msg.c_str(),
               sizeof(cg.lastLoggedCenterPrint));
    CG_Printf("%s\n", msg.c_str());
  }
}
}

/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
inline constexpr int CP_LINEWIDTH = 56; // NERVE - SMF

void CG_CenterPrint(const char *str, const int y, const int charWidth,
                    const bool log) {
  char *s;
  int i, len;                    // NERVE - SMF
  qboolean neednewline = qfalse; // NERVE - SMF
  int priority = 0;

  // NERVE - SMF - don't draw if this print message is less important
  if (cg.centerPrintTime && priority < cg.centerPrintPriority) {
    return;
  }

  Q_strncpyz(cg.centerPrint, str, sizeof(cg.centerPrint));
  cg.centerPrintPriority = priority; // NERVE - SMF

  if (log && etj_logCenterPrint.integer) {
    ETJump::logCenterPrint();
  }

  // NERVE - SMF - turn spaces into newlines, if we've run over the
  // linewidth
  len = strlen(cg.centerPrint);
  for (i = 0; i < len; i++) {

    // NOTE: subtract a few chars here so long words still get
    // displayed properly
    if (i % (CP_LINEWIDTH - 20) == 0 && i > 0) {
      neednewline = qtrue;
    }
    if (cg.centerPrint[i] == ' ' && neednewline) {
      cg.centerPrint[i] = '\n';
      neednewline = qfalse;
    }
  }
  // -NERVE - SMF

  cg.centerPrintTime = cg.time;
  cg.centerPrintY = y;
  cg.centerPrintCharWidth = charWidth;

  // count the number of lines for centering
  cg.centerPrintLines = 1;
  s = cg.centerPrint;
  while (*s) {
    if (*s == '\n') {
      cg.centerPrintLines++;
    }
    s++;
  }
}

// NERVE - SMF
/*
==============
CG_PriorityCenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_PriorityCenterPrint(const char *str, const int y, const int charWidth,
                            const int priority, const bool log) {
  char *s;
  int i, len;                    // NERVE - SMF
  qboolean neednewline = qfalse; // NERVE - SMF

  // NERVE - SMF - don't draw if this print message is less important
  if (cg.centerPrintTime && priority < cg.centerPrintPriority) {
    return;
  }

  Q_strncpyz(cg.centerPrint, str, sizeof(cg.centerPrint));
  cg.centerPrintPriority = priority; // NERVE - SMF

  if (log && etj_logCenterPrint.integer) {
    ETJump::logCenterPrint();
  }

  // NERVE - SMF - turn spaces into newlines, if we've run over the
  // linewidth
  len = strlen(cg.centerPrint);
  for (i = 0; i < len; i++) {

    // NOTE: subtract a few chars here so long words still get
    // displayed properly
    if (i % (CP_LINEWIDTH - 20) == 0 && i > 0) {
      neednewline = qtrue;
    }
    if (cg.centerPrint[i] == ' ' && neednewline) {
      cg.centerPrint[i] = '\n';
      neednewline = qfalse;
    }
  }
  // -NERVE - SMF

  cg.centerPrintTime = cg.time + 2000;
  cg.centerPrintY = y;
  cg.centerPrintCharWidth = charWidth;

  // count the number of lines for centering
  cg.centerPrintLines = 1;
  s = cg.centerPrint;
  while (*s) {
    if (*s == '\n') {
      cg.centerPrintLines++;
    }
    s++;
  }
}
// -NERVE - SMF

/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString(void) {
  char *start;
  int l;
  int x, y, w;
  float *color;

  if (!cg.centerPrintTime) {
    return;
  }

  color = CG_FadeColor(cg.centerPrintTime, 1000 * cg_centertime.value);
  if (!color) {
    cg.centerPrintTime = 0;
    cg.centerPrintPriority = 0;

    // center print has faded, clear last logged message
    // to allow same message to be re-logged
    memset(cg.lastLoggedCenterPrint, 0, sizeof(cg.lastLoggedCenterPrint));
    return;
  }

  trap_R_SetColor(color);

  start = cg.centerPrint;

  y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

  while (1) {
    char linebuffer[1024];

    for (l = 0; l < CP_LINEWIDTH; l++) // NERVE - SMF - added CP_LINEWIDTH
    {
      if (!start[l] || start[l] == '\n') {
        break;
      }
      linebuffer[l] = start[l];
    }
    linebuffer[l] = 0;

    w = cg.centerPrintCharWidth * CG_DrawStrlen(linebuffer);

    x = SCREEN_CENTER_X - w / 2;

    CG_DrawStringExt(x, y, linebuffer, color, qfalse, qtrue,
                     cg.centerPrintCharWidth,
                     (int)(cg.centerPrintCharWidth * 1.5), 0);

    y += cg.centerPrintCharWidth * 1.5;

    while (*start && (*start != '\n')) {
      start++;
    }
    if (!*start) {
      break;
    }
    start++;
  }

  trap_R_SetColor(NULL);
}

/*
================================================================================

CROSSHAIRS

================================================================================
*/

/*
==============
CG_DrawWeapReticle
==============
*/
static void CG_DrawWeapReticle(void) {
  vec4_t color = {0, 0, 0, 1};
  bool fg42;

  // DHM - Nerve :: So that we will draw reticle
  if ((cg.snap->ps.pm_flags & PMF_FOLLOW) || cg.demoPlayback) {
    fg42 = static_cast<bool>(cg.snap->ps.weapon == WP_FG42SCOPE);
  } else {
    fg42 = static_cast<bool>(cg.weaponSelect == WP_FG42SCOPE);
  }

  if (fg42) {
    // sides
    CG_FillRect(0, 0, SCREEN_OFFSET_X + 80, 480, color);
    CG_FillRect(SCREEN_OFFSET_X + 560, 0, SCREEN_OFFSET_X + 80, 480, color);

    // center
    if (cgs.media.reticleShaderSimple) {
      CG_DrawPic(SCREEN_OFFSET_X + 80, 0, 480, 480,
                 cgs.media.reticleShaderSimple);
    }

    /*		if(cgs.media.reticleShaderSimpleQ) {
                trap_R_DrawStretchPic( x,	0, w, h, 0, 0,
       1, 1, cgs.media.reticleShaderSimpleQ );	// tl
       trap_R_DrawStretchPic( x+w, 0, w, h, 1, 0, 0, 1,
       cgs.media.reticleShaderSimpleQ );	// tr
                trap_R_DrawStretchPic( x,	h, w, h, 0, 1,
       1, 0, cgs.media.reticleShaderSimpleQ );	// bl
       trap_R_DrawStretchPic( x+w, h, w, h, 1, 1, 0, 0,
       cgs.media.reticleShaderSimpleQ );	// br
            }*/

    // hairs
    if (!ETJump::showingScores()) {
      CG_FillRect(SCREEN_OFFSET_X + 84, 239, 150, 3,
                  color); // left
      CG_FillRect(SCREEN_OFFSET_X + 234, 240, 173, 1,
                  color); // horiz center
      CG_FillRect(SCREEN_OFFSET_X + 407, 239, 150, 3,
                  color); // right

      CG_FillRect(SCREEN_OFFSET_X + 319, 2, 3, 151,
                  color); // top center top
      CG_FillRect(SCREEN_OFFSET_X + 320, 153, 1, 114,
                  color); // top center bot

      CG_FillRect(SCREEN_OFFSET_X + 320, 241, 1, 87,
                  color); // bot center top
      CG_FillRect(SCREEN_OFFSET_X + 319, 327, 3, 151,
                  color); // bot center bot
    }
  } else // garand/k43
  {
    // sides
    CG_FillRect(0, 0, SCREEN_OFFSET_X + 80, 480, color);
    CG_FillRect(SCREEN_OFFSET_X + 560, 0, SCREEN_OFFSET_X + 80, 480, color);

    // center
    if (cgs.media.reticleShaderSimple) {
      CG_DrawPic(SCREEN_OFFSET_X + 80, 0, 480, 480,
                 cgs.media.reticleShaderSimple);
    }

    // hairs
    if (!ETJump::showingScores()) {
      CG_FillRect(SCREEN_OFFSET_X + 84, 239, 177, 2,
                  color); // left
      CG_FillRect(SCREEN_OFFSET_X + 320, 242, 1, 58,
                  color); // center top
      CG_FillRect(SCREEN_OFFSET_X + 319, 300, 2, 178,
                  color); // center bot
      CG_FillRect(SCREEN_OFFSET_X + 380, 239, 177, 2,
                  color); // right
    }
  }
}

/*
==============
CG_DrawMortarReticle
==============
*/
void CG_DrawMortarReticle() {
  vec4_t color = {1.f, 1.f, 1.f, .5f};
  vec4_t color_back = {0.f, 0.f, 0.f, .25f};
  vec4_t color_extends = {.77f, .73f, .1f, 1.f};
  vec4_t color_lastfire = {.77f, .1f, .1f, 1.f};
  // vec4_t	color_firerequest = { .23f, 1.f, .23f, 1.f };
  vec4_t color_firerequest = {1.f, 1.f, 1.f, 1.f};
  float offset, localOffset;
  int i, min, majorOffset, val, printval, fadeTime;
  char *s;
  float angle, angleMin, angleMax;
  qboolean hasRightTarget, hasLeftTarget;

  // Background
  CG_FillRect(SCREEN_OFFSET_X + 136, 236, 154, 38, color_back);
  CG_FillRect(SCREEN_OFFSET_X + 290, 160, 60, 208, color_back);
  CG_FillRect(SCREEN_OFFSET_X + 350, 236, 154, 38, color_back);

  // Horizontal bar

  // bottom
  CG_FillRect(SCREEN_OFFSET_X + 140, 264, 150, 1, color); // left
  CG_FillRect(SCREEN_OFFSET_X + 350, 264, 150, 1, color); // right

  // 10 units - 5 degrees
  // total of 360 units
  // nothing displayed between 150 and 210 units
  // 360 / 10 = 36 bits, means 36 * 5 = 180 degrees
  // that means left is cg.predictedPlayerState.viewangles[YAW] - .5f * 180
  angle =
      360 - AngleNormalize360(cg.predictedPlayerState.viewangles[YAW] - 90.f);

  offset = (5.f / 65536) * ((int)(angle * (65536 / 5.f)) & 65535);
  min = (int)(AngleNormalize360(angle - .5f * 180) / 15.f) * 15;
  majorOffset =
      (int)(floor((int)floor(AngleNormalize360(angle - .5f * 180)) % 15) / 5.f);

  for (val = i = 0; i < 36; i++) {
    localOffset = i * 10.f + (offset * 2.f);

    if (localOffset >= 150 && localOffset <= 210) {
      if (i % 3 == majorOffset) {
        val++;
      }
      continue;
    }

    if (i % 3 == majorOffset) {
      printval = min - val * 15 + 180;

      // rain - old tertiary abuse was nasty and had
      // undefined result
      if (printval < 0) {
        printval += 360;
      } else if (printval >= 360) {
        printval -= 360;
      }

      s = va("%i", printval);
      // CG_Text_Paint_Ext( 140 + localOffset - .5f *
      // CG_Text_Width_Ext( s, .15f, 0,
      // &cgs.media.limboFont1 ), 244, .15f, .15f,
      // color, s, 0, 0, 0, &cgs.media.limboFont1 );
      // CG_FillRect( 140 + localOffset, 248, 1, 16,
      // color);
      CG_Text_Paint_Ext(
          SCREEN_OFFSET_X + 500 - localOffset -
              .5f * CG_Text_Width_Ext(s, .15f, 0, &cgs.media.limboFont1),
          244, .15f, .15f, color, s, 0, 0, 0, &cgs.media.limboFont1);
      CG_FillRect(SCREEN_OFFSET_X + 500 - localOffset, 248, 1, 16, color);
      val++;
    } else {
      // CG_FillRect( 140 + localOffset, 256, 1, 8,
      // color);
      CG_FillRect(SCREEN_OFFSET_X + 500 - localOffset, 256, 1, 8, color);
    }
  }

  // the extremes
  // 30 degrees plus a 15 degree border
  angleMin = AngleNormalize360(
      360 - (cg.pmext.mountedWeaponAngles[YAW] - 90.f) - (30.f + 15.f));
  angleMax = AngleNormalize360(
      360 - (cg.pmext.mountedWeaponAngles[YAW] - 90.f) + (30.f + 15.f));

  // right
  localOffset = (AngleNormalize360(angle - angleMin) / 5.f) * 10.f;
  // CG_FillRect( 320 + localOffset, 252, 2, 18, color_extends);
  CG_FillRect(SCREEN_OFFSET_X + 320 - localOffset, 252, 2, 18, color_extends);

  // left
  localOffset = (AngleNormalize360(angleMax - angle) / 5.f) * 10.f;
  // CG_FillRect( 320 - localOffset, 252, 2, 18, color_extends);
  CG_FillRect(SCREEN_OFFSET_X + 320 + localOffset, 252, 2, 18, color_extends);

  // last fire pos
  fadeTime = 0;
  if (cg.lastFiredWeapon == WP_MORTAR_SET && cg.mortarImpactTime >= -1) {
    fadeTime = cg.time - (cg.predictedPlayerEntity.muzzleFlashTime + 5000);

    if (fadeTime < 3000) {
      float lastfireAngle;

      if (fadeTime > 0) {
        color_lastfire[3] = 1.f - (fadeTime / 3000.f);
      }

      lastfireAngle =
          AngleNormalize360(360 - (cg.mortarFireAngles[YAW] - 90.f));

      localOffset = ((AngleSubtract(angle, lastfireAngle)) / 5.f) * 10.f;
      // CG_FillRect( 320 + localOffset, 252, 2, 18,
      // color_lastfire);
      CG_FillRect(SCREEN_OFFSET_X + 320 - localOffset, 252, 2, 18,
                  color_lastfire);
    }
  }

  // mortar attack requests
  hasRightTarget = hasLeftTarget = qfalse;
  for (i = 0; i < MAX_CLIENTS; i++) {
    int requestFadeTime = cg.time - (cg.artilleryRequestTime[i] + 25000);

    if (requestFadeTime < 5000) {
      vec3_t dir;
      float yaw;
      float attackRequestAngle;

      VectorSubtract(cg.artilleryRequestPos[i],
                     cg.predictedPlayerEntity.lerpOrigin, dir);

      // ripped this out of vectoangles
      if (dir[1] == 0 && dir[0] == 0) {
        yaw = 0;
      } else {
        if (dir[0]) {
          yaw = (atan2(dir[1], dir[0]) * 180 / M_PI);
        } else if (dir[1] > 0) {
          yaw = 90;
        } else {
          yaw = 270;
        }
        if (yaw < 0) {
          yaw += 360;
        }
      }

      if (requestFadeTime > 0) {
        color_firerequest[3] = 1.f - (requestFadeTime / 5000.f);
      }

      attackRequestAngle = AngleNormalize360(360 - (yaw - 90.f));

      yaw = AngleSubtract(attackRequestAngle, angleMin);

      if (yaw < 0) {
        if (!hasLeftTarget) {
          // CG_FillRect( 136 + 2, 236 +
          // 38 - 6, 4, 4,
          // color_firerequest );

          trap_R_SetColor(color_firerequest);
          CG_DrawPic(SCREEN_OFFSET_X + 136 + 2, 236 + 38 - 10 + 1, 8, 8,
                     cgs.media.ccMortarTargetArrow);
          trap_R_SetColor(NULL);

          hasLeftTarget = qtrue;
        }
      } else if (yaw > 90) {
        if (!hasRightTarget) {
          // CG_FillRect( 350 + 154 - 6,
          // 236 + 38 - 6, 4, 4,
          // color_firerequest
          // );

          trap_R_SetColor(color_firerequest);
          CG_DrawPic(SCREEN_OFFSET_X + 350 + 154 - 10, 236 + 38 - 10 + 1, -8, 8,
                     cgs.media.ccMortarTargetArrow);
          trap_R_SetColor(NULL);

          hasRightTarget = qtrue;
        }
      } else {
        localOffset = ((AngleSubtract(angle, attackRequestAngle)) / 5.f) * 10.f;
        // CG_FillRect( 320 + localOffset - 3,
        // 264 - 3, 6, 6, color_firerequest
        // );

        trap_R_SetColor(color_firerequest);
        // CG_DrawPic( 320 + localOffset - 8,
        // 264 - 8, 16, 16,
        // cgs.media.ccMortarTarget );
        CG_DrawPic(SCREEN_OFFSET_X + 320 - localOffset - 8, 264 - 8, 16, 16,
                   cgs.media.ccMortarTarget);
        trap_R_SetColor(NULL);
      }
    }
  }

  /*s = va( "%.2f (%i / %i)",AngleNormalize360(angle - .5f * 180),
  majorOffset, min ); CG_Text_Paint( 140, 224, .25f, color, s, 0, 0, 0
  ); s = va(
  "%.2f",AngleNormalize360(angle) ); CG_Text_Paint( 320 - .5f *
  CG_Text_Width( s, .25f, 0), 224, .25f, color, s, 0, 0, 0 ); s = va(
  "%.2f", AngleNormalize360(angle + .5f * 180) ); CG_Text_Paint( 500 -
  CG_Text_Width( s, .25f, 0 ), 224, .25f, color, s, 0, 0, 0 );*/

  // Vertical bar

  // sides
  CG_FillRect(SCREEN_OFFSET_X + 295, 164, 1, 200, color); // left
  CG_FillRect(SCREEN_OFFSET_X + 345, 164, 1, 200, color); // right

  // 10 units - 2.5 degrees
  // total of 200 units
  // 200 / 10 = 20 bits, means 20 * 2.5 = 50 degrees
  // that means left is cg.predictedPlayerState.viewangles[PITCH] - .5f
  // * 50
  angle =
      AngleNormalize180(360 - (cg.predictedPlayerState.viewangles[PITCH] - 60));

  offset = (2.5f / 65536) * ((int)(angle * (65536 / 2.5f)) & 65535);
  min = floor((angle + .5f * 50) / 10.f) * 10;
  majorOffset = (int)(floor((int)((angle + .5f * 50) * 10.f) % 100) / 25.f);

  for (val = i = 0; i < 20; i++) {
    localOffset = i * 10.f + (offset * 4.f);

    /*if( localOffset >= 150 && localOffset <= 210 ) {
        if( i % 3 == majorOffset)
            val++;
        continue;
    }*/

    if (i % 4 == majorOffset) {
      printval = min - val * 10;

      // rain - old tertiary abuse was nasty and had
      // undefined result
      if (printval <= -180) {
        printval += 360;
      } else if (printval >= 180) {
        printval -= 180;
      }

      s = va("%i", printval);
      CG_Text_Paint_Ext(
          SCREEN_OFFSET_X + 320 -
              .5f * CG_Text_Width_Ext(s, .15f, 0, &cgs.media.limboFont1),
          164 + localOffset +
              .5f * CG_Text_Height_Ext(s, .15f, 0, &cgs.media.limboFont1),
          .15f, .15f, color, s, 0, 0, 0, &cgs.media.limboFont1);
      CG_FillRect(SCREEN_OFFSET_X + 295 + 1, 164 + localOffset, 12, 1, color);
      CG_FillRect(SCREEN_OFFSET_X + 345 - 12, 164 + localOffset, 12, 1, color);
      val++;
    } else {
      CG_FillRect(SCREEN_OFFSET_X + 295 + 1, 164 + localOffset, 8, 1, color);
      CG_FillRect(SCREEN_OFFSET_X + 345 - 8, 164 + localOffset, 8, 1, color);
    }
  }

  // the extremes
  // 30 degrees up
  // 20 degrees down
  angleMin =
      AngleNormalize180(360 - (cg.pmext.mountedWeaponAngles[PITCH] - 60)) -
      20.f;
  angleMax =
      AngleNormalize180(360 - (cg.pmext.mountedWeaponAngles[PITCH] - 60)) +
      30.f;

  // top
  localOffset = angleMax - angle;
  if (localOffset < 0) {
    localOffset = 0;
  }
  localOffset = (AngleNormalize360(localOffset) / 2.5f) * 10.f;
  if (localOffset < 100) {
    CG_FillRect(SCREEN_OFFSET_X + 295 - 2, 264 - localOffset, 6, 2,
                color_extends);
    CG_FillRect(SCREEN_OFFSET_X + 345 - 4 + 1, 264 - localOffset, 6, 2,
                color_extends);
  }

  // bottom
  localOffset = angle - angleMin;
  if (localOffset < 0) {
    localOffset = 0;
  }
  localOffset = (AngleNormalize360(localOffset) / 2.5f) * 10.f;
  if (localOffset < 100) {
    CG_FillRect(SCREEN_OFFSET_X + 295 - 2, 264 + localOffset, 6, 2,
                color_extends);
    CG_FillRect(SCREEN_OFFSET_X + 345 - 4 + 1, 264 + localOffset, 6, 2,
                color_extends);
  }

  // last fire pos
  if (cg.lastFiredWeapon == WP_MORTAR_SET && cg.mortarImpactTime >= -1) {
    if (fadeTime < 3000) {
      float lastfireAngle;

      lastfireAngle =
          AngleNormalize180(360 - (cg.mortarFireAngles[PITCH] - 60));

      if (lastfireAngle > angle) {
        localOffset = lastfireAngle - angle;
        if (localOffset < 0) {
          localOffset = 0;
        }
        localOffset = (AngleNormalize360(localOffset) / 2.5f) * 10.f;
        if (localOffset < 100) {
          CG_FillRect(SCREEN_OFFSET_X + 295 - 2, 264 - localOffset, 6, 2,
                      color_lastfire);
          CG_FillRect(SCREEN_OFFSET_X + 345 - 4 + 1, 264 - localOffset, 6, 2,
                      color_lastfire);
        }
      } else {
        localOffset = angle - lastfireAngle;
        if (localOffset < 0) {
          localOffset = 0;
        }
        localOffset = (AngleNormalize360(localOffset) / 2.5f) * 10.f;
        if (localOffset < 100) {
          CG_FillRect(SCREEN_OFFSET_X + 295 - 2, 264 + localOffset, 6, 2,
                      color_lastfire);
          CG_FillRect(SCREEN_OFFSET_X + 345 - 4 + 1, 264 + localOffset, 6, 2,
                      color_lastfire);
        }
      }
    }
  }

  /*s = va( "%.2f (%i / %i)", angle + .5f * 50, majorOffset, min );
  CG_Text_Paint( 348, 164, .25f, color, s, 0, 0, 0 );
  s = va( "%.2f",angle );
  CG_Text_Paint( 348, 264, .25f, color, s, 0, 0, 0 );
  s = va( "%.2f", angle - .5f * 50 );
  CG_Text_Paint( 348, 364, .25f, color, s, 0, 0, 0 );*/
}

/*
==============
CG_DrawBinocReticle
==============
*/
static void CG_DrawBinocReticle(void) {
  // an alternative.  This gives nice sharp lines at the expense of a
  // few extra polys
  vec4_t color;
  color[0] = color[1] = color[2] = 0;
  color[3] = 1;

  if (cgs.media.binocShaderSimple) {
    CG_DrawPic(SCREEN_OFFSET_X, 0, 640, 480, cgs.media.binocShaderSimple);
    CG_FillRect(0, 0, SCREEN_OFFSET_X, 480, color);
    CG_FillRect(SCREEN_OFFSET_X + 640, 0, SCREEN_OFFSET_X, 480, color);
  }

  if (!ETJump::showingScores()) {
    CG_FillRect(SCREEN_OFFSET_X + 146, 239, 348, 1, color);

    CG_FillRect(SCREEN_OFFSET_X + 188, 234, 1, 13, color); // ll
    CG_FillRect(SCREEN_OFFSET_X + 234, 226, 1, 29, color); // l
    CG_FillRect(SCREEN_OFFSET_X + 274, 234, 1, 13, color); // lr
    CG_FillRect(SCREEN_OFFSET_X + 320, 213, 1, 55,
                color);                                    // center
    CG_FillRect(SCREEN_OFFSET_X + 360, 234, 1, 13, color); // rl
    CG_FillRect(SCREEN_OFFSET_X + 406, 226, 1, 29, color); // r
    CG_FillRect(SCREEN_OFFSET_X + 452, 234, 1, 13, color); // rr
  }
}

static void CG_CheckForReticle() {
  // using binoculars
  if (cg.zoomedBinoc && cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR) {
    CG_DrawBinocReticle();
    return;
  }

  // FIXME: spectators/chasing?
  if (cg.predictedPlayerState.weapon == WP_MORTAR_SET &&
      cg.predictedPlayerState.weaponstate != WEAPON_RAISING) {
    CG_DrawMortarReticle();
    return;
  }

  // DHM - Nerve :: show reticle in limbo and spectator
  int weapnum = ETJump::weapnumForClient();
  switch (weapnum) {

    // weapons that get no reticle
    case WP_NONE: // no weapon, no crosshair
      if (cg.zoomedBinoc &&
          cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR) {
        CG_DrawBinocReticle();
      }

      if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR) {
        return;
      }
      break;

    // special reticle for weapon
    case WP_FG42SCOPE:
    case WP_GARAND_SCOPE:
    case WP_K43_SCOPE:
      if (!BG_PlayerMounted(cg.snap->ps.eFlags)) {
        // JPW NERVE -- don't let players run with rifles
        // -- speed 80 == crouch, 128 == walk, 256 == run
        if (VectorLengthSquared(cg.snap->ps.velocity) > SQR(127)) {
          if (cg.snap->ps.weapon == WP_FG42SCOPE) {
            CG_FinishWeaponChange(WP_FG42SCOPE, WP_FG42);
          }
          if (cg.snap->ps.weapon == WP_GARAND_SCOPE) {
            CG_FinishWeaponChange(WP_GARAND_SCOPE, WP_GARAND);
          }
          if (cg.snap->ps.weapon == WP_K43_SCOPE) {
            CG_FinishWeaponChange(WP_K43_SCOPE, WP_K43);
          }
        }

        // OSP
        if (cg.mvTotalClients < 1 || cg.snap->ps.stats[STAT_HEALTH] > 0) {
          CG_DrawWeapReticle();
        }

        return;
      }
      break;
    default:
      break;
  }
}

static void CG_DrawNoShootIcon(void) {
  float x, y, w, h;
  float *color;

  if (cg.predictedPlayerState.eFlags & EF_PRONE &&
      cg.snap->ps.weapon == WP_PANZERFAUST) {
    trap_R_SetColor(colorRed);
  } else if (cg.crosshairClientNoShoot
             // xkan, 1/6/2003 - don't shoot friend or civilian
             || cg.snap->ps.serverCursorHint == HINT_PLYR_NEUTRAL ||
             cg.snap->ps.serverCursorHint == HINT_PLYR_FRIEND) {
    color = CG_FadeColor(cg.crosshairClientTime, 1000);

    if (!color) {
      trap_R_SetColor(NULL);
      return;
    } else {
      trap_R_SetColor(color);
    }
  } else {
    return;
  }

  w = h = 48.f;

  x = cg_crosshairX.integer + 1;
  y = cg_crosshairY.integer + 1;
  CG_AdjustFrom640(&x, &y, &w, &h);

  // FIXME precache
  trap_R_DrawStretchPic(x + 0.5 * (cg.refdef_current->width - w),
                        y + 0.5 * (cg.refdef_current->height - h), w, h, 0, 0,
                        1, 1, cgs.media.friendShader);
}

/*
=================
CG_ScanForCrosshairEntity
=================

Returns the distance to the entity

*/
static float CG_ScanForCrosshairEntity(float *zChange, qboolean *hitClient) {
  trace_t trace;
  //	gentity_t	*traceEnt;
  vec3_t start, end;
  float dist;
  centity_t *cent;

  // We haven't hit a client yet
  *hitClient = qfalse;

  VectorCopy(cg.refdef.vieworg, start);
  VectorMA(start, 8192, cg.refdef.viewaxis[0],
           end); //----(SA)	changed from 8192

  cg.crosshairClientNoShoot = qfalse;

  CG_Trace(&trace, start, NULL, NULL, end, cg.snap->ps.clientNum,
           CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_ITEM);

  // How far from start to end of trace?
  dist = VectorDistance(start, trace.endpos);

  // How far up or down are we looking?
  *zChange = trace.endpos[2] - start[2];

  if (trace.entityNum >= MAX_CLIENTS) {
    if (cg_entities[trace.entityNum].currentState.eFlags & EF_TAGCONNECT) {
      trace.entityNum = cg_entities[trace.entityNum].tagParent;
    }

    // is a tank with a healthbar
    // this might have some side-effects, but none right now as
    // the script_mover is the only one that sets effect1Time
    if ((cg_entities[trace.entityNum].currentState.eType == ET_MOVER &&
         cg_entities[trace.entityNum].currentState.effect1Time) ||
        cg_entities[trace.entityNum].currentState.eType ==
            ET_CONSTRUCTIBLE_MARKER) {
      // update the fade timer
      cg.crosshairClientNum = trace.entityNum;
      cg.crosshairClientTime = cg.time;
      cg.identifyClientRequest = cg.crosshairClientNum;
    }

    // Default: We're not looking at a client
    cg.crosshairNotLookingAtClient = qtrue;

    return dist;
  }

  //	traceEnt = &g_entities[trace.entityNum];

  // Reset the draw time for the SP crosshair
  cg.crosshairSPClientTime = cg.time;

  // Default: We're not looking at a client
  cg.crosshairNotLookingAtClient = qfalse;

  // We hit a client
  *hitClient = qtrue;

  // update the fade timer
  cg.crosshairClientNum = trace.entityNum;
  cg.crosshairClientTime = cg.time;
  if (cg.crosshairClientNum != cg.snap->ps.identifyClient &&
      cg.crosshairClientNum != ENTITYNUM_WORLD) {
    cg.identifyClientRequest = cg.crosshairClientNum;
  }

  cent = &cg_entities[cg.crosshairClientNum];

  if (cent && cent->currentState.powerups & (1 << PW_OPS_DISGUISED)) {
    if (cgs.clientinfo[cg.crosshairClientNum].team ==
        cgs.clientinfo[cg.clientNum].team) {
      cg.crosshairClientNoShoot = qtrue;
    }
  }

  return dist;
}

namespace ETJump {
void cursorhintTrace(trace_t *trace, vec3_t start, vec3_t end) {
  CG_Trace(trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum,
           MASK_PLAYERSOLID);

  if (trace->entityNum >= MAX_CLIENTS) {
    return;
  }

  while (trace->entityNum < MAX_CLIENTS &&
         !playerIsSolid(cg.snap->ps.clientNum, trace->entityNum)) {
    tempTraceIgnoreClient(trace->entityNum);
    CG_Trace(trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum,
             MASK_PLAYERSOLID);
  }

  resetTempTraceIgnoredClients();
}
} // namespace ETJump

/*
==============
CG_CheckForCursorHints
    concept in progress...
==============
*/
void CG_CheckForCursorHints() {
  trace_t trace;
  vec3_t start, end;

  if (cg.renderingThirdPerson) {
    return;
  }

  // server is dictating a cursor hint, use it.
  if (cg.snap->ps.serverCursorHint) {
    cg.cursorHintTime = cg.time;
    cg.cursorHintFade = 500; // fade out time
    cg.cursorHintIcon = cg.snap->ps.serverCursorHint;
    cg.cursorHintValue = cg.snap->ps.serverCursorHintVal;
    return;
  }

  // From here on it's client-side cursor hints.  So if the server isn't
  // sending that info (as an option) then it falls into here and you
  // can get basic cursorhint info if you want, but not the detailed
  // info the server sends.

  // the trace
  VectorCopy(cg.refdef_current->vieworg, start);
  VectorMA(start, CH_DIST, cg.refdef_current->viewaxis[0], end);

  ETJump::cursorhintTrace(&trace, start, end);

  if (trace.fraction == 1.0f) {
    return;
  }

  const float dist = trace.fraction * CH_DIST;
  const centity_t *tracent = &cg_entities[trace.entityNum];

  // Arnout: invisible entities don't show hints
  if (trace.entityNum >= MAX_CLIENTS &&
      (tracent->currentState.powerups == STATE_INVISIBLE ||
       tracent->currentState.powerups == STATE_UNDERCONSTRUCTION)) {
    return;
  }

  // world
  if (trace.entityNum == ENTITYNUM_WORLD) {
    if (trace.surfaceFlags & SURF_LADDER &&
        !(cg.snap->ps.pm_flags & PMF_LADDER)) {
      if (dist <= CH_LADDER_DIST) {
        cg.cursorHintIcon = HINT_LADDER;
        cg.cursorHintTime = cg.time;
        cg.cursorHintFade = 500;
        cg.cursorHintValue = 0;
      }
    }
  }

  // for players, we only care about knife for backstab hint
  if (trace.entityNum >= MAX_CLIENTS || cg.snap->ps.weapon != WP_KNIFE) {
    return;
  }

  // too far
  if (dist > CH_KNIFE_DIST) {
    return;
  }

  vec3_t eforward;
  vec3_t pforward;

  AngleVectors(cg.snap->ps.viewangles, pforward, nullptr, nullptr);
  AngleVectors(tracent->lerpAngles, eforward, nullptr, nullptr);

  // from behind(-ish)
  if (DotProduct(eforward, pforward) > 0.6f) {
    cg.cursorHintIcon = HINT_KNIFE;
    cg.cursorHintTime = cg.time;
    cg.cursorHintFade = 100;
    cg.cursorHintValue = 0;
  }
}

static void CG_DrawMoverHealthBar(float frac, const vec4_t color) {
  vec4_t bgcolor;
  vec4_t c;
  const float barFrac = std::clamp(frac, 0.0f, 1.0f);

  c[0] = 1.0f;
  c[1] = c[2] = barFrac;
  c[3] = (0 + barFrac * 0.5f) * color[3];

  Vector4Set(bgcolor, 1.f, 1.f, 1.f, .25f * color[3]);
  CG_FilledBar(SCREEN_CENTER_X - 110 / 2.0f, 190, 110, 10, c, nullptr, bgcolor,
               barFrac, FilledBarFlags::BAR_BG);
}

static void CG_DrawPlayerName(vec4_t color) {
  const char *s = va("%s", cgs.clientinfo[cg.crosshairClientNum].name);
  float w = ETJump::DrawStringWidth(s, 0.23f);

  ETJump::DrawString(SCREEN_CENTER_X - w / 2, 182, 0.23f, 0.25f, color, qfalse,
                     s, 0, ITEM_TEXTSTYLE_SHADOWED);
  trap_R_SetColor(nullptr);
}

/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames() {
  if (!cg_drawCrosshairNames.integer) {
    return;
  }

  if (cg.renderingThirdPerson || cg_drawCrosshair.integer < 0) {
    return;
  }

  if (ETJump::showingScores()) {
    return;
  }

  qboolean hitClient = qfalse;
  // Distance to the entity under the crosshair
  // scan the known entities to see if the crosshair is sighted on one
  float zChange = 0;
  float dist = CG_ScanForCrosshairEntity(&zChange, &hitClient);

  // draw the name of the player being looked at
  float *color = CG_FadeColor(cg.crosshairClientTime, 1000);

  if (!color) {
    trap_R_SetColor(nullptr);
    return;
  }

  float playerHealth = 0.f;
  float maxHealth = 1.f;
  bool isMover = false;

  // NERVE - SMF
  if (cg.crosshairClientNum >= MAX_CLIENTS) {
    if (cg_entities[cg.crosshairClientNum].currentState.eType == ET_MOVER &&
        cg_entities[cg.crosshairClientNum].currentState.effect1Time) {
      isMover = qtrue;

      playerHealth =
          cg_entities[cg.crosshairClientNum].currentState.dl_intensity;
      maxHealth = 255.f;

      const char *s = Info_ValueForKey(CG_ConfigString(CS_SCRIPT_MOVER_NAMES),
                                       va("%i", cg.crosshairClientNum));
      if (!*s) {
        return;
      }

      float w = ETJump::DrawStringWidth(s, 0.23f);
      ETJump::DrawString(SCREEN_CENTER_X - w / 2, 182, 0.23f, 0.25f, color,
                         qfalse, s, 0, ITEM_TEXTSTYLE_SHADOWED);
    } else if (cg_entities[cg.crosshairClientNum].currentState.eType ==
               ET_CONSTRUCTIBLE_MARKER) {
      const char *s = Info_ValueForKey(CG_ConfigString(CS_CONSTRUCTION_NAMES),
                                       va("%i", cg.crosshairClientNum));
      if (*s) {
        float w = ETJump::DrawStringWidth(s, 0.23f);
        ETJump::DrawString(SCREEN_CENTER_X - w / 2, 182, 0.23f, 0.25f, color,
                           qfalse, s, 0, ITEM_TEXTSTYLE_SHADOWED);
      }
      return;
    }

    if (!isMover) {
      return;
    }
  }

  if (isMover) {
    CG_DrawMoverHealthBar(playerHealth / maxHealth, color);
  } else {
    // fireteam noghost ignores etj_hide/etj_hideDistance, so always draw names
    if (CG_IsOnSameFireteam(cg.snap->ps.clientNum, cg.crosshairClientNum) &&
        cgs.clientinfo[cg.snap->ps.clientNum].fireteamData->noGhost) {
      CG_DrawPlayerName(color);
    } else {
      if ((etj_hide.integer == 1 && dist < etj_hideDistance.integer) ||
          etj_hide.integer == 2 ||
          cgs.clientinfo[cg.crosshairClientNum].hideMe) {
        return;
      }

      CG_DrawPlayerName(color);
    }
  }
}

//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void) {
  const char *s;

  if (ETJump::showingScores()) {
    return;
  }

  if (cgs.demoCam.renderingFreeCam) {
    s = CG_TranslateString("FREECAM");
  } else {
    s = CG_TranslateString("SPECTATOR");
  }
  auto textWidth = ETJump::DrawStringWidth(s, 0.3f);
  ETJump::DrawBigString(SCREEN_CENTER_X - textWidth / 2, 440 + 10, s, 0.95f);
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote() {
  char str1[32], str2[32];
  vec4_t color{1, 1, 0, 1};

  std::string line_a;
  std::string line_b;
  float x_a = 8.0f;
  float x_b = 8.0f;

  const auto rtvHandler = ETJump::rtvHandler;

  Q_strncpyz(str1, BindingFromName("vote yes"), 32);
  Q_strncpyz(str2, BindingFromName("vote no"), 32);

  if (cgs.applicationEndTime > cg.time && cgs.applicationClient >= 0) {
    line_a =
        ETJump::stringFormat("Accept %s's application to join your fireteam?",
                             cgs.clientinfo[cgs.applicationClient].name);
    line_b =
        ETJump::stringFormat("Press '%s' for YES, or '%s' for No", str1, str2);
  } else if (cgs.propositionEndTime > cg.time && cgs.propositionClient >= 0) {
    line_a = ETJump::stringFormat(
        "Accept %s's proposition to invite %s to join your "
        "fireteam?",
        cgs.clientinfo[cgs.propositionClient2].name,
        cgs.clientinfo[cgs.propositionClient].name);
    line_b =
        ETJump::stringFormat("Press '%s' for YES, or '%s' for No", str1, str2);
  } else if (cgs.invitationEndTime > cg.time && cgs.invitationClient >= 0) {
    line_a =
        ETJump::stringFormat("Accept %s's invitation to join their fireteam?",
                             cgs.clientinfo[cgs.invitationClient].name);
    line_b =
        ETJump::stringFormat("Press '%s' for YES, or '%s' for No", str1, str2);
  } else if (cgs.autoFireteamEndTime > cg.time && cgs.autoFireteamNum == -1) {
    // make sure we're still on the fireteam before displaying
    // this prompt
    if (CG_IsOnFireteam(cg.clientNum)) {
      line_a = "Make Fireteam private?";
      line_b = ETJump::stringFormat("Press '%s' for YES, or '%s' for No", str1,
                                    str2);
    }
    // we have left, so reset the timer
    else {
      cgs.autoFireteamEndTime = cg.time;
    }
  } else if (cgs.voteTime) {
    // play a talk beep whenever it is modified
    if (cgs.voteModified) {
      cgs.voteModified = qfalse;
    }

    int sec = (VOTE_TIME - (cg.time - cgs.voteTime)) / 1000;
    if (sec < 0) {
      sec = 0;
    }

    if (!Q_stricmpn(cgs.voteString, "kick", 4)) {
      if (strlen(cgs.voteString) > 5) {
        int nameindex;
        char buffer[128];
        Q_strncpyz(buffer, cgs.voteString + 5, sizeof(buffer));
        Q_CleanStr(buffer);

        for (nameindex = 0; nameindex < MAX_CLIENTS; nameindex++) {
          if (!cgs.clientinfo[nameindex].infoValid) {
            continue;
          }

          if (!Q_stricmp(cgs.clientinfo[nameindex].cleanname, buffer)) {
            if (cgs.clientinfo[nameindex].team != TEAM_SPECTATOR &&
                cgs.clientinfo[nameindex].team !=
                    cgs.clientinfo[cg.clientNum].team) {
              return;
            }
          }
        }
      }
    }

    const bool canVote = cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR ||
                         etj_spectatorVote.integer;
    const bool isRtvVote = rtvHandler->rtvVoteActive();
    const int rtvYesVotes = rtvHandler->getRtvYesVotes();

    if (isRtvVote) {
      if (!(cg.snap->ps.eFlags & EF_VOTED)) {
        line_a = ETJump::stringFormat("VOTE(%i): %s", sec, cgs.voteString);
        line_b = ETJump::stringFormat(
            "Change map(%s):%i, Keep current map(%s):%i%s", str1, rtvYesVotes,
            str2, cgs.voteNo, canVote ? "" : " (Spectators can't vote)");
      } else {
        line_a =
            ETJump::stringFormat("(%i) YOU VOTED ON: %s", sec, cgs.voteString);
        line_b = ETJump::stringFormat("Change map:%i, Keep current map:%i",
                                      rtvYesVotes, cgs.voteNo);

        x_b = 13;

        if (cgs.votedYes) {
          const char *yesVotes = "Change map";
          auto strWidth =
              static_cast<float>(ETJump::DrawStringWidth(yesVotes, 0.23f));
          CG_DrawRect_FixedBorder(x_b - 2, 214 - 10 + 12, strWidth + 4, 12, 1,
                                  color);
        } else if (cgs.votedNo) {
          std::string yesVotes =
              ETJump::stringFormat("Change map:%i", rtvYesVotes);
          std::string noVotes =
              ETJump::stringFormat("Keep current map", cgs.voteNo);
          auto yesStrWidth = static_cast<float>(
              ETJump::DrawStringWidth(yesVotes.c_str(), 0.23f));
          auto noStrWidth = static_cast<float>(
              ETJump::DrawStringWidth(noVotes.c_str(), 0.23f));
          CG_DrawRect_FixedBorder(x_b - 2 + yesStrWidth + 15, 214 - 10 + 12,
                                  noStrWidth + 4, 12, 1, color);
        }
      }
    } else {
      if (!(cg.snap->ps.eFlags & EF_VOTED)) {
        line_a = ETJump::stringFormat("VOTE(%i): %s", sec, cgs.voteString);
        line_b =
            ETJump::stringFormat("YES:%i, NO:%i%s", cgs.voteYes, cgs.voteNo,
                                 canVote ? "" : " (Spectators can't vote)");
      } else {
        line_a =
            ETJump::stringFormat("(%i) YOU VOTED ON: %s", sec, cgs.voteString);
        line_b = ETJump::stringFormat("Y:%i, N:%i", cgs.voteYes, cgs.voteNo);

        x_b = 13;

        if (cgs.votedYes) {
          CG_DrawRect_FixedBorder(x_b - 2, 214 - 10 + 12, 11, 12, 1, color);
        } else {
          std::string yesVotes = ETJump::stringFormat("Y:%i", cgs.voteYes);
          auto strWidth = static_cast<float>(
              ETJump::DrawStringWidth(yesVotes.c_str(), 0.23f));
          CG_DrawRect_FixedBorder(x_b + strWidth + 13, 214 - 10 + 12, 11, 12, 1,
                                  color);
        }
      }
    }
  } else if (cgs.applicationEndTime > cg.time && cgs.applicationClient < 0) {
    switch (cgs.applicationClient) {
      case -1:
        line_a = "Your application has been "
                 "submitted";
        break;
      case -2:
        line_a = "Your application failed";
        break;
      case -3:
        line_a = "Your application has been "
                 "approved";
        break;
      case -4:
        line_a = "Your application reply has "
                 "been sent";
        break;
    }
  } else if (cgs.propositionEndTime > cg.time && cgs.propositionClient < 0) {
    switch (cgs.propositionClient) {
      case -1:
        line_a = "Your proposition has been "
                 "submitted";
        break;
      case -2:
        line_a = "Your proposition was rejected";
        break;
      case -3:
        line_a = "Your proposition was accepted";
        break;
      case -4:
        line_a = "Your proposition reply has "
                 "been sent";
        break;
    }
  } else if (cgs.invitationEndTime > cg.time && cgs.invitationClient < 0) {
    switch (cgs.invitationClient) {
      case -1:
        line_a = "Your invitation has been "
                 "submitted";
        break;
      case -2:
        line_a = "Your invitation was rejected";
        break;
      case -3:
        line_a = "Your invitation was accepted";
        break;
      case -4:
        line_a = "Your invitation reply has "
                 "been sent";
        break;
    }
  } else if ((cgs.autoFireteamEndTime > cg.time && cgs.autoFireteamNum == -2) ||
             (cgs.autoFireteamCreateEndTime > cg.time &&
              cgs.autoFireteamCreateNum == -2) ||
             (cgs.autoFireteamJoinEndTime > cg.time &&
              cgs.autoFireteamJoinNum == -2)) {
    line_a = "Response Sent";
  }

  if (!line_a.empty()) {
    ETJump::DrawString(x_a, 212, 0.23f, 0.25f, color, qtrue, line_a.c_str(), 80,
                       ITEM_TEXTSTYLE_SHADOWED);
  }

  if (!line_b.empty()) {
    ETJump::DrawString(x_b, 226, 0.23f, 0.25f, color, qtrue, line_b.c_str(), 80,
                       ITEM_TEXTSTYLE_SHADOWED);
  }
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission(void) {
  // End-of-level autoactions
  if (!cg.demoPlayback) {
    static int doScreenshot = 0, doDemostop = 0;

    if (!cg.latchAutoActions) {
      cg.latchAutoActions = qtrue;

      if (cg_autoAction.integer & AA_SCREENSHOT) {
        doScreenshot = cg.time + 1000;
      }

      if (cg_autoAction.integer & AA_STATSDUMP) {
        CG_dumpStats_f();
      }

      if (cg_autoAction.integer & AA_DEMORECORD) {
        doDemostop = cg.time + 5000; // stats should show up within 5 seconds
      }
    }

    if (doScreenshot > 0 && doScreenshot < cg.time) {
      CG_autoScreenShot_f();
      doScreenshot = 0;
    }

    if (doDemostop > 0 && doDemostop < cg.time) {
      trap_SendConsoleCommand("stoprecord\n");
      doDemostop = 0;
    }
  }
}

/*
=================
CG_ActivateLimboMenu

NERVE - SMF
=================
*/
static void CG_ActivateLimboMenu(void) {
  /*	static qboolean latch = qfalse;
      qboolean test;

      // should we open the limbo menu (make allowances for MV clients)
      test = ((cg.snap->ps.pm_flags & PMF_LIMBO) ||
              ( (cg.mvTotalClients < 1 && (
                  (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
     || (cg.warmup))
                )
              && cg.snap->ps.pm_type != PM_INTERMISSION ) );


      // auto open/close limbo mode
      if(cg_popupLimboMenu.integer && !cg.demoPlayback) {
          if(test && !latch) {
              CG_LimboMenu_f();
              latch = qtrue;
          } else if(!test && latch && cg.showGameView) {
              CG_EventHandling(CGAME_EVENT_NONE, qfalse);
              latch = qfalse;
          }
      }*/
}

/*
=================
CG_DrawSpectatorMessage
=================
*/
static void CG_DrawSpectatorMessage(void) {
  std::string str;
  const char *str2;
  static int lastconfigGet = 0;

  if (cgs.demoCam.renderingFreeCam) {
    return;
  }

  if (!cg_descriptiveText.integer) {
    return;
  }

  if (!(cg.snap->ps.pm_flags & PMF_LIMBO ||
        cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)) {
    return;
  }

  if (trap_Key_GetCatcher() & KEYCATCH_UI && !cg.chatMenuOpen) {
    return;
  }

  if (cg.time - lastconfigGet > 1000) {
    Controls_GetConfig();

    lastconfigGet = cg.time;
  }

  str2 = BindingFromName("openlimbomenu");
  if (!Q_stricmp(str2, "(openlimbomenu)")) {
    str2 = "ESCAPE";
  }
  str = ETJump::stringFormat("Press %s to open Limbo Menu", str2);
  ETJump::DrawString(8, 154 + 12, 0.23f, 0.25f, colorWhite, qtrue, str.c_str(),
                     0, ITEM_TEXTSTYLE_SHADOWED);

  str2 = BindingFromName("+attack");
  str = ETJump::stringFormat("Press %s to follow next player", str2);
  ETJump::DrawString(8, 172 + 12, 0.23f, 0.25f, colorWhite, qtrue, str.c_str(),
                     0, ITEM_TEXTSTYLE_SHADOWED);

#ifdef MV_SUPPORT
  str2 = BindingFromName("mvactivate");
  str = va(CG_TranslateString("- Press %s to %s multiview mode"), str2,
           ((cg.mvTotalClients > 0) ? "disable" : "activate"));
  CG_DrawStringExt(0, 408 - 2 * TINYCHAR_HEIGHT, str, colorWhite, qtrue, qtrue,
                   TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
  y += TINYCHAR_HEIGHT;
#endif
}

float CG_CalculateReinfTime_Float(qboolean menu) {
  team_t team;
  int dwDeployTime;

  if (menu) {
    if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR) {
      team = cgs.ccSelectedTeam == 0 ? TEAM_AXIS : TEAM_ALLIES;
    } else {
      team = cgs.clientinfo[cg.clientNum].team;
    }
  } else {
    team = cgs.clientinfo[cg.snap->ps.clientNum].team;
  }

  dwDeployTime =
      (team == TEAM_AXIS) ? cg_redlimbotime.integer : cg_bluelimbotime.integer;
  return (1 + (dwDeployTime -
               ((cgs.aReinfOffset[team] + cg.time - cgs.levelStartTime) %
                dwDeployTime)) *
                  0.001f);
}

int CG_CalculateReinfTime(qboolean menu) {
  return ((int)CG_CalculateReinfTime_Float(menu));
}

/*
=================
CG_DrawLimboMessage
=================
*/

inline constexpr float INFOTEXT_STARTX = 8.0f;

static void CG_DrawLimboMessage(void) {
  float color[4] = {1, 1, 1, 1};
  std::string str;
  playerState_t *ps = &cg.snap->ps;
  int y = 130;

  if (cgs.demoCam.renderingFreeCam) {
    return;
  }

  if (ps->stats[STAT_HEALTH] > 0) {
    return;
  }

  if (cg.snap->ps.pm_flags & PMF_LIMBO ||
      cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR) {
    return;
  }

  if (trap_Key_GetCatcher() & KEYCATCH_UI && !cg.chatMenuOpen) {
    return;
  }

  if (!cg_descriptiveText.integer) {
    return;
  }

  str = "You are wounded and waiting for a medic.";
  ETJump::DrawString(INFOTEXT_STARTX, y, 0.23f, 0.25f, color, qfalse,
                     str.c_str(), 0, 0);
  y += 18;

  str = "Press JUMP to go into reinforcement queue.";
  ETJump::DrawString(INFOTEXT_STARTX, y, 0.23f, 0.25f, color, qfalse,
                     str.c_str(), 0, 0);
  y += 18;

  // JPW NERVE
  str = "Reinforcements deploy in " +
        ETJump::getSecondsString(CG_CalculateReinfTime(qfalse)) + ".";
  ETJump::DrawString(INFOTEXT_STARTX, y, 0.23f, 0.25f, color, qfalse,
                     str.c_str(), 0, 0);
}
// -NERVE - SMF

static void CG_DrawSlick(void) {
  trace_t trace;
  vec3_t start, end;
  const float minWalkNormal = 0.7;
  float x;

  if (!etj_drawSlick.integer) {
    return;
  }

  int traceContents = ETJump::checkExtraTrace(ETJump::SLICK_DETECTOR);

  playerState_t *ps = ETJump::getValidPlayerState();

  x = etj_slickX.value;

  ETJump_AdjustPosition(&x);

  VectorCopy(cg.refdef.vieworg, start);
  VectorMA(start, MAX_MAP_SIZE * 2, cg.refdef.viewaxis[0], end);

  CG_Trace(&trace, start, NULL, NULL, end, ps->clientNum, traceContents);

  if ((trace.fraction != 1.0 && trace.surfaceFlags & SURF_SLICK) ||
      (trace.plane.normal[2] > 0 && trace.plane.normal[2] < minWalkNormal)) {
    CG_DrawStringExt(x, etj_slickY.integer, "S", colorWhite, qfalse, qtrue,
                     TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
  }
}

// ETJump: jump delay surface dector
static void CG_DrawJumpDelay(void) {
  trace_t trace;
  vec3_t start, end;
  float x = etj_noJumpDelayX.integer;
  float y = etj_noJumpDelayY.integer;

  if (!etj_drawNoJumpDelay.integer) {
    return;
  }

  int traceContents = ETJump::checkExtraTrace(ETJump::NJD_DETECTOR);

  playerState_t *ps = ETJump::getValidPlayerState();

  ETJump_AdjustPosition(&x);
  VectorCopy(cg.refdef.vieworg, start);
  VectorMA(start, MAX_MAP_SIZE * 2, cg.refdef.viewaxis[0], end);

  CG_Trace(&trace, start, nullptr, nullptr, end, ps->clientNum, traceContents);

  if (trace.surfaceFlags & SURF_NOJUMPDELAY) {
    if (cgs.shared & BG_LEVEL_NO_JUMPDELAY) {
      CG_DrawStringExt(x, y, "D", colorWhite, qfalse, qtrue, TINYCHAR_WIDTH,
                       TINYCHAR_HEIGHT, 0);
    } else {
      CG_DrawStringExt(x, y, "ND", colorWhite, qfalse, qtrue, TINYCHAR_WIDTH,
                       TINYCHAR_HEIGHT, 0);
    }
  }
}

/*
=================
CG_DrawFollow
=================
*/
static void CG_DrawFollow() {
  // MV following info for mainview
  if (CG_ViewingDraw()) {
    return;
  }

  if (!(cg.snap->ps.pm_flags & PMF_FOLLOW)) {
    return;
  }

  if (trap_Key_GetCatcher() & KEYCATCH_UI && !cg.chatMenuOpen) {
    return;
  }

  if (cg.snap->ps.clientNum == cg.clientNum) {
    return;
  }

  const std::string str = ETJump::stringFormat(
      "^7Following %s^7", cgs.clientinfo[cg.snap->ps.clientNum].name);
  ETJump::DrawString(INFOTEXT_STARTX, 118 + 12, 0.23f, 0.25f, colorWhite,
                     qfalse, str.c_str(), 0, ITEM_TEXTSTYLE_SHADOWED);
}

/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup(void) {
  int w;
  int sec;
  int cw;
  const char *s, *s1, *s2;

  sec = cg.warmup;
  if (!sec) {
    if ((cgs.gamestate == GS_WARMUP && !cg.warmup) ||
        cgs.gamestate == GS_WAITING_FOR_PLAYERS) {
      cw = 10;

      s1 = va(CG_TranslateString("^3WARMUP:^7 Waiting on ^2%i^7 %s"),
              cgs.minclients, cgs.minclients == 1 ? "player" : "players");
      w = CG_DrawStrlen(s1);
      CG_DrawStringExt(SCREEN_CENTER_X - w * 12 / 2, 188, s1, colorWhite,
                       qfalse, qtrue, 12, 18, 0);

      if (!cg.demoPlayback &&
          cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR &&
          (!(cg.snap->ps.pm_flags & PMF_FOLLOW) ||
           (cg.snap->ps.pm_flags & PMF_LIMBO))) {
        char str1[32];
        Q_strncpyz(str1, BindingFromName("ready"), 32);
        if (!Q_stricmp(str1, "(?"
                             "?"
                             "?)")) {
          s2 = CG_TranslateString("Type ^3\\ready^* in the "
                                  "console to start");
        } else {
          s2 = va("Press ^3%s^* to start", str1);
          s2 = CG_TranslateString(s2);
        }
        w = CG_DrawStrlen(s2);
        CG_DrawStringExt(SCREEN_CENTER_X - w * cw / 2, 208, s2, colorWhite,
                         qfalse, qtrue, cw, (int)(cw * 1.5), 0);
      }
      return;
    }

    return;
  }

  sec = (sec - cg.time) / 1000;
  if (sec < 0) {
    sec = 0;
  }

  s = va("%s %i", CG_TranslateString("(WARMUP) Match begins in:"), sec + 1);

  w = CG_DrawStrlen(s);
  CG_DrawStringExt(SCREEN_CENTER_X - w * 6, 120, s, colorYellow, qfalse, qtrue,
                   12, 18, 0);
}

//==================================================================================

/*
=================
CG_DrawFlashFade
=================
*/
static void CG_DrawFlashFade(void) {
  static int lastTime;
  int elapsed, time;
  vec4_t col;
  qboolean fBlackout = (int_ui_blackout.integer > 0) ? qtrue : qfalse;

  if (cgs.fadeStartTime + cgs.fadeDuration < cg.time) {
    cgs.fadeAlphaCurrent = cgs.fadeAlpha;
  } else if (cgs.fadeAlphaCurrent != cgs.fadeAlpha) {
    elapsed = (time = trap_Milliseconds()) -
              lastTime; // we need to use trap_Milliseconds()
                        // here since the cg.time gets modified
                        // upon reloading
    lastTime = time;
    if (elapsed < 500 && elapsed > 0) {
      if (cgs.fadeAlphaCurrent > cgs.fadeAlpha) {
        cgs.fadeAlphaCurrent -= ((float)elapsed / (float)cgs.fadeDuration);
        if (cgs.fadeAlphaCurrent < cgs.fadeAlpha) {
          cgs.fadeAlphaCurrent = cgs.fadeAlpha;
        }
      } else {
        cgs.fadeAlphaCurrent += ((float)elapsed / (float)cgs.fadeDuration);
        if (cgs.fadeAlphaCurrent > cgs.fadeAlpha) {
          cgs.fadeAlphaCurrent = cgs.fadeAlpha;
        }
      }
    }
  }

  // OSP - ugh, have to inform the ui that we need to remain blacked out
  // (or not)
  if (int_ui_blackout.integer == 0) {
    if (cg.mvTotalClients < 1 && cg.snap->ps.powerups[PW_BLACKOUT] > 0) {
      trap_Cvar_Set("ui_blackout", va("%d", cg.snap->ps.powerups[PW_BLACKOUT]));
    }
  } else if (cg.snap->ps.powerups[PW_BLACKOUT] == 0 || cg.mvTotalClients > 0) {
    trap_Cvar_Set("ui_blackout", "0");
  }

  // now draw the fade
  if (cgs.fadeAlphaCurrent > 0.0 || fBlackout) {
    VectorClear(col);
    col[3] = (fBlackout) ? 1.0f : cgs.fadeAlphaCurrent;
    //		CG_FillRect( -10, -10, 650, 490, col );
    CG_FillRect(0, 0, SCREEN_WIDTH, 480,
                col); // why do a bunch of these extend outside 640x480?

    // bani - #127 - bail out if we're a speclocked spectator
    // with cg_draw2d = 0
    if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR &&
        !cg_draw2D.integer) {
      return;
    }

    // OSP - Show who is speclocked
    if (fBlackout) {
      int i, nOffset = 90;
      const char *str, *format = "The %s team is speclocked!";
      const char *teams[TEAM_NUM_TEAMS] = {"??", "AXIS", "ALLIES", "???"};
      float color[4] = {1, 1, 0, 1};

      for (i = TEAM_AXIS; i <= TEAM_ALLIES; i++) {
        if (cg.snap->ps.powerups[PW_BLACKOUT] & i) {
          str = va(format, teams[i]);
          CG_DrawStringExt(INFOTEXT_STARTX, nOffset, str, color, qtrue, qfalse,
                           10, 10, 0);
          nOffset += 12;
        }
      }
    }
  }
}

/*
==============
CG_DrawFlashZoomTransition
    hide the snap transition from regular view to/from zoomed

  FIXME: TODO: use cg_fade?
==============
*/
static void CG_DrawFlashZoomTransition(void) {
  vec4_t color;
  float frac;
  int fadeTime;

  if (!cg.snap) {
    return;
  }

  // ETJump: no transition for spectator zoom
  if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
    return;
  }

  if (BG_PlayerMounted(cg.snap->ps.eFlags)) {
    // don't draw when on mg_42
    // keep the timer fresh so when you remove yourself from the
    // mg42, it'll fade
    cg.zoomTime = cg.time;
    return;
  }

  if (cg.renderingThirdPerson) {
    return;
  }

  fadeTime = 400;

  frac = cg.time - cg.zoomTime;

  if (frac < fadeTime) {
    frac = frac / (float)fadeTime;
    Vector4Set(color, 0, 0, 0, 1.0f - frac);
    CG_FillRect(-10, -10, SCREEN_WIDTH + 10, 490, color);
  }
}

/*
=================
CG_DrawFlashDamage
=================
*/
static void CG_DrawFlashDamage(void) {
  vec4_t col;
  float redFlash;

  if (!cg.snap) {
    return;
  }

  if (cg.v_dmg_time > cg.time) {
    redFlash =
        std::fabs(cg.v_dmg_pitch * ((cg.v_dmg_time - cg.time) / DAMAGE_TIME));

    // blend the entire screen red
    if (redFlash > 5) {
      redFlash = 5;
    }

    VectorSet(col, 0.2, 0, 0);
    col[3] = 0.7 * (redFlash / 5.0) *
             ((cg_bloodFlash.value > 1.0)   ? 1.0
              : (cg_bloodFlash.value < 0.0) ? 0.0
                                            : cg_bloodFlash.value);

    CG_FillRect(-10, -10, SCREEN_WIDTH + 10, 490, col);
  }
}

/*
=================
CG_DrawFlashFire
=================
*/
static void CG_DrawFlashFire(void) {
  vec4_t col = {1, 1, 1, 1};
  float alpha, max, f;

  if (!cg.snap) {
    return;
  }

  if (cg.renderingThirdPerson) {
    return;
  }

  if (!cg.snap->ps.onFireStart) {
    cg.v_noFireTime = cg.time;
    return;
  }

  alpha =
      (float)((FIRE_FLASH_TIME - 1000) - (cg.time - cg.snap->ps.onFireStart)) /
      (FIRE_FLASH_TIME - 1000);
  if (alpha > 0) {
    if (alpha >= 1.0) {
      alpha = 1.0;
    }

    // fade in?
    f = (float)(cg.time - cg.v_noFireTime) / FIRE_FLASH_FADEIN_TIME;
    if (f >= 0.0 && f < 1.0) {
      alpha = f;
    }

    max = 0.5 + 0.5 * sin((float)((cg.time / 10) % 1000) / 1000.0);
    if (alpha > max) {
      alpha = max;
    }
    col[0] = alpha;
    col[1] = alpha;
    col[2] = alpha;
    col[3] = alpha;
    trap_R_SetColor(col);
    CG_DrawPic(-10, -10, SCREEN_WIDTH + 10, 490,
               cgs.media.viewFlashFire[(cg.time / 50) % 16]);
    trap_R_SetColor(NULL);

    trap_S_AddLoopingSound(cg.snap->ps.origin, vec3_origin,
                           cgs.media.flameSound, (int)(255.0 * alpha), 0);
    trap_S_AddLoopingSound(cg.snap->ps.origin, vec3_origin,
                           cgs.media.flameCrackSound, (int)(255.0 * alpha), 0);
  } else {
    cg.v_noFireTime = cg.time;
  }
}

/*
==============
CG_DrawFlashBlendBehindHUD
    screen flash stuff drawn first (on top of world, behind HUD)
==============
*/
static void CG_DrawFlashBlendBehindHUD(void) {
  CG_DrawFlashZoomTransition();
  CG_DrawFlashFade();
}

/*
=================
CG_DrawFlashBlend
    screen flash stuff drawn last (on top of everything)
=================
*/
static void CG_DrawFlashBlend(void) {
  // Gordon: no flash blends if in limbo or spectator, and in the limbo
  // menu
  if ((cg.snap->ps.pm_flags & PMF_LIMBO ||
       cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR) &&
      cg.showGameView) {
    return;
  }

  CG_DrawFlashFire();
  CG_DrawFlashDamage();
}

// NERVE - SMF
/*
=================
CG_DrawObjectiveInfo
=================
*/
inline constexpr int OID_TOP = 360;

void CG_ObjectivePrint(const char *str, int charWidth) {
  char *s;
  int i, len;                    // NERVE - SMF
  qboolean neednewline = qfalse; // NERVE - SMF

  if (cg.centerPrintTime) {
    return;
  }

  s = CG_TranslateString(str);

  Q_strncpyz(cg.oidPrint, s, sizeof(cg.oidPrint));

  // NERVE - SMF - turn spaces into newlines, if we've run over the
  // linewidth
  len = strlen(cg.oidPrint);
  for (i = 0; i < len; i++) {

    // NOTE: subtract a few chars here so long words still get
    // displayed properly
    if (i % (CP_LINEWIDTH - 20) == 0 && i > 0) {
      neednewline = qtrue;
    }
    if (cg.oidPrint[i] == ' ' && neednewline) {
      cg.oidPrint[i] = '\n';
      neednewline = qfalse;
    }
  }
  // -NERVE - SMF

  cg.oidPrintTime = cg.time;
  cg.oidPrintY = OID_TOP;
  cg.oidPrintCharWidth = charWidth;

  // count the number of lines for oiding
  cg.oidPrintLines = 1;
  s = cg.oidPrint;
  while (*s) {
    if (*s == '\n') {
      cg.oidPrintLines++;
    }
    s++;
  }
}

static void CG_DrawObjectiveInfo(void) {
  char *start;
  int l;
  int x, y, w;
  int x1, y1, x2, y2;
  float *color;
  vec4_t backColor;

  if (!cg.oidPrintTime) {
    return;
  }

  color = CG_FadeColor(cg.oidPrintTime, 250);
  if (!color) {
    cg.oidPrintTime = 0;
    return;
  }

  trap_R_SetColor(color);

  start = cg.oidPrint;

  // JPW NERVE
  //	y = cg.oidPrintY - cg.oidPrintLines * BIGCHAR_HEIGHT / 2;
  y = 400 - cg.oidPrintLines * BIGCHAR_HEIGHT / 2;

  x1 = SCREEN_CENTER_X - 1;
  y1 = y - 2;
  x2 = SCREEN_CENTER_X + 1;
  // jpw

  // first just find the bounding rect
  while (1) {
    char linebuffer[1024];

    for (l = 0; l < CP_LINEWIDTH; l++) {
      if (!start[l] || start[l] == '\n') {
        break;
      }
      linebuffer[l] = start[l];
    }
    linebuffer[l] = 0;

    w = cg.oidPrintCharWidth * CG_DrawStrlen(linebuffer) + 10;
    // JPW NERVE
    if (SCREEN_CENTER_X - w / 2 < x1) {
      x1 = SCREEN_CENTER_X - w / 2;
      x2 = SCREEN_CENTER_X + w / 2;
    }

    /*
            if ( x1 + w > x2 )
                x2 = x1 + w;
    */
    x = SCREEN_CENTER_X - w / 2;
    // jpw
    y += cg.oidPrintCharWidth * 1.5;

    while (*start && (*start != '\n')) {
      start++;
    }
    if (!*start) {
      break;
    }
    start++;
  }

  x2 = x2 + 4;
  y2 = y - cg.oidPrintCharWidth * 1.5 + 4;

  VectorCopy(color, backColor);
  backColor[3] = 0.5 * color[3];
  trap_R_SetColor(backColor);

  CG_DrawPic(x1, y1, x2 - x1, y2 - y1, cgs.media.teamStatusBar);

  VectorSet(backColor, 0, 0, 0);
  CG_DrawRect(x1, y1, x2 - x1, y2 - y1, 1, backColor);

  trap_R_SetColor(color);

  // do the actual drawing
  start = cg.oidPrint;
  //	y = cg.oidPrintY - cg.oidPrintLines * BIGCHAR_HEIGHT / 2;
  y = 400 - cg.oidPrintLines * BIGCHAR_HEIGHT / 2; // JPW NERVE

  while (1) {
    char linebuffer[1024];

    for (l = 0; l < CP_LINEWIDTH; l++) {
      if (!start[l] || start[l] == '\n') {
        break;
      }
      linebuffer[l] = start[l];
    }
    linebuffer[l] = 0;

    w = cg.oidPrintCharWidth * CG_DrawStrlen(linebuffer);
    if (x1 + w > x2) {
      x2 = x1 + w;
    }

    x = SCREEN_CENTER_X - w / 2; // JPW NERVE

    CG_DrawStringExt(x, y, linebuffer, color, qfalse, qtrue,
                     cg.oidPrintCharWidth, (int)(cg.oidPrintCharWidth * 1.5),
                     0);

    y += cg.oidPrintCharWidth * 1.5;

    while (*start && (*start != '\n')) {
      start++;
    }
    if (!*start) {
      break;
    }
    start++;
  }

  trap_R_SetColor(NULL);
}

//==================================================================================

void CG_DrawTimedMenus() {
  if (cg.voiceTime) {
    int t = cg.time - cg.voiceTime;
    if (t > 2500) {
      Menus_CloseByName("voiceMenu");
      trap_Cvar_Set("cl_conXOffset", "0");
      cg.voiceTime = 0;
    }
  }
}

/*
=================
CG_Fade
=================
*/
void CG_Fade(int r, int g, int b, int a, int time, int duration) {

  // incorporate this into the current fade scheme

  cgs.fadeAlpha = static_cast<float>(a) / 255.0f;
  cgs.fadeStartTime = time;
  cgs.fadeDuration = duration;

  if (cgs.fadeStartTime + cgs.fadeDuration <= cg.time) {
    cgs.fadeAlphaCurrent = cgs.fadeAlpha;
  }
}

/*
=================
CG_ScreenFade
=================
*/
static void CG_ScreenFade(void) {
  int msec;
  int i;
  float t, invt;
  vec4_t color;

  if (!cg.fadeRate) {
    return;
  }

  msec = cg.fadeTime - cg.time;
  if (msec <= 0) {
    cg.fadeColor1[0] = cg.fadeColor2[0];
    cg.fadeColor1[1] = cg.fadeColor2[1];
    cg.fadeColor1[2] = cg.fadeColor2[2];
    cg.fadeColor1[3] = cg.fadeColor2[3];

    if (!cg.fadeColor1[3]) {
      cg.fadeRate = 0;
      return;
    }

    CG_FillRect(0, 0, SCREEN_WIDTH, 480, cg.fadeColor1);

  } else {
    t = (float)msec * cg.fadeRate;
    invt = 1.0f - t;

    for (i = 0; i < 4; i++) {
      color[i] = cg.fadeColor1[i] * t + cg.fadeColor2[i] * invt;
    }

    if (color[3]) {
      CG_FillRect(0, 0, SCREEN_WIDTH, 480, color);
    }
  }
}

#if 0 // rain - unused
// JPW NERVE
void CG_Draw2D2(void)
{
	qhandle_t weapon;

	trap_R_SetColor(NULL);

	CG_DrawPic(0, 480, 640, -70, cgs.media.hud1Shader);

	if (!BG_PlayerMounted(cg.snap->ps.eFlags))
	{
		switch (cg.snap->ps.weapon)
		{
		case WP_COLT:
		case WP_LUGER:
			weapon = cgs.media.hud2Shader;
			break;
		case WP_KNIFE:
			weapon = cgs.media.hud5Shader;
			break;
		default:
			weapon = cgs.media.hud3Shader;
		}
		CG_DrawPic(220, 410, 200, -200, weapon);
	}
}
#endif

/*
=================
CG_DrawCompassIcon

NERVE - SMF
=================
*/
void CG_DrawCompassIcon(float x, float y, float w, float h, vec3_t origin,
                        vec3_t dest, qhandle_t shader) {
  float angle, pi2 = M_PI * 2;
  vec3_t v1, angles;
  float len;

  VectorCopy(dest, v1);
  VectorSubtract(origin, v1, v1);
  len = VectorLength(v1);
  VectorNormalize(v1);
  vectoangles(v1, angles);

  if (v1[0] == 0 && v1[1] == 0 && v1[2] == 0) {
    return;
  }

  angles[YAW] =
      AngleSubtract(cg.predictedPlayerState.viewangles[YAW], angles[YAW]);

  angle = ((angles[YAW] + 180.f) / 360.f - (0.50 / 2.f)) * pi2;

  w /= 2;
  h /= 2;

  x += w;
  y += h;

  {
    w = sqrt((w * w) + (h * h)) / 3.f * 2.f * 0.9f;
  }

  x = x + (cos(angle) * w);
  y = y + (sin(angle) * w);

  len = 1 - std::min(1.f, len / 2000.f);

  CG_DrawPic(x - (14 * len + 4) / 2, y - (14 * len + 4) / 2, 14 * len + 8,
             14 * len + 8, shader);
#ifdef SQUARE_COMPASS
}
else {
  int iconWidth, iconHeight;
  // START Mad Doc - TDF
  // talk about fitting a square peg into a round hole...
  // we're now putting the compass icons around the square automap
  // instead of the round compass

  while (angle < 0) angle += pi2;

  while (angle >= pi2) angle -= pi2;

  x = x + w / 2;
  y = y + h / 2;
  w /= 2; // = sqrt( ( w * w ) + ( h * h ) ) / 3.f * 2.f * 0.9f;

  if ((angle >= 0) && (angle < M_PI / 4.0)) {
    x += w;
    y += w * tan(angle);

  } else if ((angle >= M_PI / 4.0) && (angle < 3.0 * M_PI / 4.0)) {
    x += w / tan(angle);
    y += w;
  } else if ((angle >= 3.0 * M_PI / 4.0) && (angle < 5.0 * M_PI / 4.0)) {
    x -= w;
    y -= w * tan(angle);
  } else if ((angle >= 5.0 * M_PI / 4.0) && (angle < 7.0 * M_PI / 4.0)) {
    x -= w / tan(angle);
    y -= w;
  } else {
    x += w;
    y += w * tan(angle);
  }

  len = 1 - std::min(1.f, len / 2000.f);
  iconWidth = 14 * len + 4; // where did this calc. come from?
  iconHeight = 14 * len + 4;

  // adjust so that icon is always outside of the map
  if ((angle > 5.0 * M_PI / 4.0) && (angle < 2 * M_PI)) {

    y -= iconHeight;
  }

  if ((angle >= 3.0 * M_PI / 4.0) && (angle <= 5.0 * M_PI / 4.0)) {
    x -= iconWidth;
  }

  CG_DrawPic(x, y, iconWidth, iconHeight, shader);

  // END Mad Doc - TDF
}
#endif
}

/*
=================
CG_DrawNewCompass
=================
*/
static void CG_DrawNewCompass(void) {
  float basex, basey;
  float basew, baseh;
  snapshot_t *snap;
  float angle;
  int i;
  static float lastangle = 0;
  static float anglespeed = 0;
  float diff;

  if (cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport) {
    snap = cg.nextSnap;
  } else {
    snap = cg.snap;
  }

  if (snap->ps.pm_flags & PMF_LIMBO ||
      snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ||
      cg.mvTotalClients > 0) {
    return;
  }

  // Arnout: bit larger
  basex = SCREEN_WIDTH - 120 - 16;
  basey = 20 - 16;
  basew = 100 + 32;
  baseh = 100 + 32;

  CG_DrawAutoMap();

  if (!cg_drawCompass.integer) {
    return;
  }

  if (cg.zoomedBinoc || BG_IsScopedWeapon(ETJump::weapnumForClient())) {
    return;
  }

  if (cgs.autoMapExpanded) {
    if (cg.time - cgs.autoMapExpandTime < 100.f) {
      basey -= ((cg.time - cgs.autoMapExpandTime) / 100.f) * 128.f;
    } else {
      // basey -= 128.f;
      return;
    }
  } else {
    if (cg.time - cgs.autoMapExpandTime <= 150.f) {
      // basey -= 128.f;
      return;
    } else if ((cg.time - cgs.autoMapExpandTime > 150.f) &&
               (cg.time - cgs.autoMapExpandTime < 250.f)) {

      basey = (basey - 128.f) +
              ((cg.time - cgs.autoMapExpandTime - 150.f) / 100.f) * 128.f;
    } else {
      rectDef_t compassHintRect = {SCREEN_WIDTH - 22.0f, 128.0f, 20.0f, 20.0f};

      CG_DrawKeyHint(&compassHintRect, "+mapexpand");
    }
  }

  CG_DrawPic(basex + 4, basey + 4, basew - 8, baseh - 8,
             cgs.media.compassShader);

  angle = (cg.predictedPlayerState.viewangles[YAW] + 180.f) / 360.f - (0.125f);
  diff = AngleSubtract(angle * 360, lastangle * 360) / 360.f;
  anglespeed /= 1.08f;
  anglespeed += diff * 0.01f;
  if (std::abs(anglespeed) < 0.00001f) {
    anglespeed = 0;
  }
  lastangle += anglespeed;
  CG_DrawRotatedPic(basex + 4, basey + 4, basew - 8, baseh - 8,
                    cgs.media.compass2Shader, lastangle);

  //	if( !(cgs.ccFilter & CC_FILTER_REQUESTS) ) {
  // draw voice chats
  for (i = 0; i < MAX_CLIENTS; i++) {
    centity_t *cent = &cg_entities[i];

    if (cg.predictedPlayerState.clientNum == i ||
        !cgs.clientinfo[i].infoValid ||
        cg.predictedPlayerState.persistant[PERS_TEAM] !=
            cgs.clientinfo[i].team) {
      continue;
    }

    // also draw revive icons if cent is dead and player is a
    // medic
    if (cent->voiceChatSpriteTime < cg.time) {
      continue;
    }

    if (cgs.clientinfo[i].health <= 0) {
      // reset
      cent->voiceChatSpriteTime = cg.time;
      continue;
    }

    CG_DrawCompassIcon(basex, basey, basew, baseh,
                       cg.predictedPlayerState.origin, cent->lerpOrigin,
                       cent->voiceChatSprite);
  }
  //	}

  /*if( !(cgs.ccFilter & CC_FILTER_DESTRUCTIONS) ) {
      // draw explosives if an engineer
      if ( cg.predictedPlayerState.stats[ STAT_PLAYER_CLASS ] ==
  PC_ENGINEER ) { for ( i = 0; i < snap->numEntities; i++ ) { centity_t
  *cent = &cg_entities[ snap->entities[ i ].number ];

              if ( cent->currentState.eType != ET_EXPLOSIVE_INDICATOR )
  { continue;
              }

              if ( cent->currentState.teamNum == 1 &&
  cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_AXIS ) continue;
  else if ( cent->currentState.teamNum == 2 &&
  cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_ALLIES )
  continue;

              CG_DrawCompassIcon( basex, basey, basew, baseh,
  cg.predictedPlayerState.origin, cent->lerpOrigin,
  cgs.media.compassDestroyShader );
          }
      }
  }*/

  //	if( !(cgs.ccFilter & CC_FILTER_REQUESTS) ) {
  // draw revive medic icons
  if (cg.predictedPlayerState.stats[STAT_PLAYER_CLASS] == PC_MEDIC) {
    for (i = 0; i < snap->numEntities; i++) {
      entityState_t *ent = &snap->entities[i];

      if (ent->eType != ET_PLAYER) {
        continue;
      }

      if ((ent->eFlags & EF_DEAD) && ent->number == ent->clientNum) {
        if (!cgs.clientinfo[ent->clientNum].infoValid ||
            cg.predictedPlayerState.persistant[PERS_TEAM] !=
                cgs.clientinfo[ent->clientNum].team) {
          continue;
        }

        CG_DrawCompassIcon(basex, basey, basew, baseh,
                           cg.predictedPlayerState.origin, ent->pos.trBase,
                           cgs.media.medicReviveShader);
      }
    }
  }
  //	}

  /*	if( !(cgs.ccFilter & CC_FILTER_DESTRUCTIONS) ) {
          // draw constructibles if an engineer
          if ( cg.predictedPlayerState.stats[ STAT_PLAYER_CLASS ] ==
     PC_ENGINEER ) { for ( i = 0; i < snap->numEntities; i++ ) {
     centity_t *cent = &cg_entities[ snap->entities[ i ].number ];

                  if ( cent->currentState.eType !=
     ET_CONSTRUCTIBLE_INDICATOR ) { continue;
                  }

                  if ( cent->currentState.teamNum !=
     cg.predictedPlayerState.persistant[PERS_TEAM] &&
     cent->currentState.teamNum
     != 3 ) continue;

                  CG_DrawCompassIcon( basex, basey, basew, baseh,
     cg.predictedPlayerState.origin, cent->lerpOrigin,
     cgs.media.compassConstructShader );
              }
          }
      }*/

  /*	if( !(cgs.ccFilter & CC_FILTER_WAYPOINTS) ) {
          // draw waypoint icons
          for ( i = 0; i < snap->numEntities; i++ ) {
              centity_t *cent = &cg_entities[ snap->entities[ i ].number
     ];

              if( cent->currentState.eType != ET_WAYPOINT ) {
                  continue;
              }

              // see if the waypoint owner is someone that you accept
     waypoints from if( !CG_IsOnSameFireteam( cg.clientNum,
     cent->currentState.clientNum
     )) {  // TODO: change to fireteam continue;
              }

              switch( cent->currentState.frame ) {
              case WAYP_ATTACK: CG_DrawCompassIcon( basex, basey, basew,
     baseh, cg.predictedPlayerState.origin,
     cent->currentState.pos.trBase,
     cgs.media.waypointCompassAttackShader ); break; case WAYP_DEFEND:
     CG_DrawCompassIcon( basex, basey, basew, baseh,
     cg.predictedPlayerState.origin, cent->currentState.pos.trBase,
     cgs.media.waypointCompassDefendShader ); break; case WAYP_REGROUP:
     CG_DrawCompassIcon( basex, basey, basew, baseh,
     cg.predictedPlayerState.origin, cent->currentState.pos.trBase,
     cgs.media.waypointCompassRegroupShader ); break;
              }
          }
      }*/

  //	if( !(cgs.ccFilter & CC_FILTER_BUDDIES) ) {
  for (i = 0; i < snap->numEntities; i++) {
    entityState_t *ent = &snap->entities[i];

    if (ent->eType != ET_PLAYER) {
      continue;
    }

    if (ent->eFlags & EF_DEAD) {
      continue;
    }

    if (!cgs.clientinfo[ent->clientNum].infoValid ||
        cg.predictedPlayerState.persistant[PERS_TEAM] !=
            cgs.clientinfo[ent->clientNum].team) {
      continue;
    }

    if (!CG_IsOnSameFireteam(cg.clientNum, ent->clientNum)) {
      continue;
    }

    CG_DrawCompassIcon(basex, basey, basew, baseh,
                       cg.predictedPlayerState.origin, ent->pos.trBase,
                       cgs.media.buddyShader);
  }
  //	}
}

int CG_PlayerAmmoValue(int *ammo, int *clips, int *akimboammo) {
  centity_t *cent;
  playerState_t *ps;
  int weap;
  qboolean skipammo = qfalse;

  *ammo = *clips = *akimboammo = -1;

  if (cg.snap->ps.clientNum == cg.clientNum) {
    cent = &cg.predictedPlayerEntity;
  } else {
    cent = &cg_entities[cg.snap->ps.clientNum];
  }
  ps = &cg.snap->ps;

  weap = cent->currentState.weapon;

  if (!weap) {
    return weap;
  }

  switch (weap) // some weapons don't draw ammo count text
  {
    case WP_AMMO:
    case WP_MEDKIT:
    case WP_KNIFE:
    case WP_PLIERS:
    case WP_SMOKE_MARKER:
    case WP_DYNAMITE:
    case WP_SATCHEL:
    case WP_SATCHEL_DET:
    case WP_SMOKE_BOMB:
    case WP_BINOCULARS:
    case WP_PORTAL_GUN: // Feen: PGM
      return weap;

    case WP_LANDMINE:
    case WP_MEDIC_SYRINGE:
    case WP_MEDIC_ADRENALINE:
    case WP_GRENADE_LAUNCHER:
    case WP_GRENADE_PINEAPPLE:
    case WP_FLAMETHROWER:
    case WP_MORTAR:
    case WP_MORTAR_SET:
    case WP_PANZERFAUST:
      skipammo = qtrue;
      break;

    default:
      break;
  }

  if (cg.snap->ps.eFlags & EF_MG42_ACTIVE ||
      cg.snap->ps.eFlags & EF_MOUNTEDTANK) {
    return WP_MOBILE_MG42;
  }

  // total ammo in clips
  *clips = cg.snap->ps.ammo[BG_FindAmmoForWeapon((weapon_t)weap)];

  // current clip
  *ammo = ps->ammoclip[BG_FindClipForWeapon((weapon_t)weap)];

  if (BG_IsAkimboWeapon(weap)) {
    *akimboammo =
        ps->ammoclip[BG_FindClipForWeapon((weapon_t)BG_AkimboSidearm(weap))];
  } else {
    *akimboammo = -1;
  }

  if (weap == WP_LANDMINE) {
    if (!cgs.gameManager) {
      *ammo = 0;
    } else {
      if (cgs.clientinfo[ps->clientNum].team == TEAM_AXIS) {
        *ammo = cgs.gameManager->currentState.otherEntityNum;
      } else {
        *ammo = cgs.gameManager->currentState.otherEntityNum2;
      }
    }
  } else if (weap == WP_MORTAR || weap == WP_MORTAR_SET ||
             weap == WP_PANZERFAUST) {
    *ammo += *clips;
  }

  if (skipammo) {
    *clips = -1;
  }

  return weap;
}

static void CG_DrawPlayerStatusHead(void) {
  hudHeadAnimNumber_t anim;
  rectDef_t headRect = {44, 480 - 92, 62, 80};
  //	rectDef_t headHintRect =	{ 40, 480 - 22, 20, 20 };
  bg_character_t *character = CG_CharacterForPlayerstate(&cg.snap->ps);
  bg_character_t *headcharacter =
      BG_GetCharacter(cgs.clientinfo[cg.snap->ps.clientNum].team,
                      cgs.clientinfo[cg.snap->ps.clientNum].cls);

  qhandle_t painshader = 0;

  anim = (hudHeadAnimNumber_t)cg.idleAnim;

  if (cg.weaponFireTime > 500) {
    anim = HD_ATTACK;
  } else if (cg.time - cg.lastFiredWeaponTime < 500) {
    anim = HD_ATTACK_END;
  } else if (cg.time - cg.painTime <
             (character->hudheadanimations[HD_PAIN].numFrames *
              character->hudheadanimations[HD_PAIN].frameLerp)) {
    anim = HD_PAIN;
  } else if (cg.time > cg.nextIdleTime) {
    cg.nextIdleTime = cg.time + 7000 + rand() % 1000;
    if (cg.snap->ps.stats[STAT_HEALTH] < 40) {
      cg.idleAnim = (rand() % (HD_DAMAGED_IDLE3 - HD_DAMAGED_IDLE2 + 1)) +
                    HD_DAMAGED_IDLE2;
    } else {
      cg.idleAnim = (rand() % (HD_IDLE8 - HD_IDLE2 + 1)) + HD_IDLE2;
    }

    cg.lastIdleTimeEnd =
        cg.time + character->hudheadanimations[cg.idleAnim].numFrames *
                      character->hudheadanimations[cg.idleAnim].frameLerp;
  }

  if (cg.snap->ps.stats[STAT_HEALTH] < 5) {
    painshader = cgs.media.hudDamagedStates[3];
  } else if (cg.snap->ps.stats[STAT_HEALTH] < 20) {
    painshader = cgs.media.hudDamagedStates[2];
  } else if (cg.snap->ps.stats[STAT_HEALTH] < 40) {
    painshader = cgs.media.hudDamagedStates[1];
  } else if (cg.snap->ps.stats[STAT_HEALTH] < 60) {
    painshader = cgs.media.hudDamagedStates[0];
  }

  if (cg.time > cg.lastIdleTimeEnd) {
    if (cg.snap->ps.stats[STAT_HEALTH] < 40) {
      cg.idleAnim = HD_DAMAGED_IDLE1;
    } else {
      cg.idleAnim = HD_IDLE1;
    }
  }

  CG_DrawPlayerHead(&headRect, character, headcharacter, 180, 0,
                    cg.snap->ps.eFlags & EF_HEADSHOT ? qfalse : qtrue, anim,
                    painshader, cgs.clientinfo[cg.snap->ps.clientNum].rank,
                    qfalse);

  //	CG_DrawKeyHint( &headHintRect, "openlimbomenu" );
}

static void CG_DrawPlayerHealthBar(rectDef_t *rect) {
  const vec4_t bgcolour = {1.f, 1.f, 1.f, 0.3f};
  vec4_t colour;

  const int flags =
      (FilledBarFlags::BAR_LEFT | FilledBarFlags::BAR_VERT |
       FilledBarFlags::BAR_BG | FilledBarFlags::BAR_BGSPACING_X0Y0);
  float frac;

  CG_ColorForHealth(colour);
  colour[3] = 0.5f;

  if (cgs.clientinfo[cg.snap->ps.clientNum].cls == PC_MEDIC) {
    frac = cg.snap->ps.stats[STAT_HEALTH] /
           ((float)cg.snap->ps.stats[STAT_MAX_HEALTH] * 1.12f);
  } else {
    frac = cg.snap->ps.stats[STAT_HEALTH] /
           (float)cg.snap->ps.stats[STAT_MAX_HEALTH];
  }

  CG_FilledBar(rect->x, rect->y + (rect->h * 0.1f), rect->w, rect->h * 0.84f,
               colour, nullptr, bgcolour, frac, flags);

  trap_R_SetColor(nullptr);
  CG_DrawPic(rect->x, rect->y, rect->w, rect->h, cgs.media.hudSprintBar);
  CG_DrawPic(rect->x, rect->y + rect->h + 4, rect->w, rect->w,
             cgs.media.hudHealthIcon);
}

static void CG_DrawStaminaBar(rectDef_t *rect) {
  const vec4_t bgcolour = {1.f, 1.f, 1.f, 0.3f};
  vec4_t colourlow = {1.0f, 0.1f, 0.1f, 0.5f};
  vec4_t colour = {0.1f, 1.0f, 0.1f, 0.5f};
  vec_t *color = colour;
  const int flags =
      (FilledBarFlags::BAR_LEFT | FilledBarFlags::BAR_VERT |
       FilledBarFlags::BAR_BG | FilledBarFlags::BAR_BGSPACING_X0Y0);
  float frac = static_cast<float>(cg.pmext.sprintTime) / SPRINTTIME;

  // make sure we only draw adrenaline visual if we actually used adrenaline
  // and not when we simply have etj_nofatigue
  if (cg.snap->ps.powerups[PW_ADRENALINE] &&
      cg.pmext.adrenalineTime > cg.time) {
    if (cg.snap->ps.pm_flags & PMF_FOLLOW) {
      Vector4Average(colour, colorWhite, std::sin(cg.time * .005f), colour);
    } else {
      auto msec = static_cast<float>(cg.pmext.adrenalineTime - cg.time);

      if (msec < 0) {
        msec = 0;
      } else {
        Vector4Average(colour, colorWhite,
                       .5f + sin(.2f * std::sqrt(msec) * 2 * M_PI) * .5f,
                       colour);
      }
    }
  } else {
    if (frac < 0.25) {
      color = colourlow;
    }
  }

  CG_FilledBar(rect->x, rect->y + (rect->h * 0.1f), rect->w, rect->h * 0.84f,
               color, nullptr, bgcolour, frac, flags);

  trap_R_SetColor(nullptr);
  CG_DrawPic(rect->x, rect->y, rect->w, rect->h, cgs.media.hudSprintBar);
  CG_DrawPic(rect->x, rect->y + rect->h + 4, rect->w, rect->w,
             cgs.media.hudSprintIcon);
}

static void CG_DrawWeapRecharge(rectDef_t *rect) {
  float chargeTime;
  const vec4_t bgcolor = {1.0f, 1.0f, 1.0f, 0.25f};
  vec4_t color;

  const int flags = (FilledBarFlags::BAR_LEFT | FilledBarFlags::BAR_VERT |
                     FilledBarFlags::BAR_BG);

  // Draw power bar
  switch (cg.snap->ps.stats[STAT_PLAYER_CLASS]) {
    case PC_ENGINEER:
      chargeTime = static_cast<float>(
          cg.engineerChargeTime[cg.snap->ps.persistant[PERS_TEAM] - 1]);
      break;
    case PC_MEDIC:
      chargeTime = static_cast<float>(
          cg.medicChargeTime[cg.snap->ps.persistant[PERS_TEAM] - 1]);
      break;
    case PC_FIELDOPS:
      chargeTime = static_cast<float>(
          cg.ltChargeTime[cg.snap->ps.persistant[PERS_TEAM] - 1]);
      break;
    case PC_COVERTOPS:
      chargeTime = static_cast<float>(
          cg.covertopsChargeTime[cg.snap->ps.persistant[PERS_TEAM] - 1]);
      break;
    default:
      chargeTime = static_cast<float>(
          cg.soldierChargeTime[cg.snap->ps.persistant[PERS_TEAM] - 1]);
      break;
  }

  const float barFrac =
      (chargeTime == 0 ? 1.0f
                       : std::min(static_cast<float>(
                                      cg.time - cg.snap->ps.classWeaponTime) /
                                      chargeTime,
                                  1.0f));

  color[0] = 1.0f;
  color[1] = color[2] = barFrac;
  color[3] = 0.25f + barFrac * 0.5f;

  CG_FilledBar(rect->x, rect->y + (rect->h * 0.1f), rect->w, rect->h * 0.84f,
               color, nullptr, bgcolor, barFrac, flags);

  trap_R_SetColor(nullptr);
  CG_DrawPic(rect->x, rect->y, rect->w, rect->h, cgs.media.hudSprintBar);
  CG_DrawPic(rect->x + (rect->w * 0.25f) - 1, rect->y + rect->h + 4,
             (rect->w * 0.5f) + 2, rect->w + 2, cgs.media.hudPowerIcon);
}

static void CG_DrawPlayerStatus(void) {
  int value, value2, value3;
  char buffer[32];
  rectDef_t rect;
  //	vec4_t			colorFaded = { 1.f, 1.f, 1.f, 0.3f };

  // Draw weapon icon and overheat bar
  rect.x = SCREEN_WIDTH - 82;
  rect.y = 480 - 56;
  rect.w = 60;
  rect.h = 32;

  if (cg.mvTotalClients < 1 && cg_drawWeaponIconFlash.integer <= 0) {
    CG_DrawPlayerWeaponIcon(&rect, qtrue, ITEM_ALIGN_RIGHT, &colorWhite);
  } else {
    int ws = (cg.mvTotalClients > 0)
                 ? cgs.clientinfo[cg.snap->ps.clientNum].weaponState
                 : BG_simpleWeaponState(cg.snap->ps.weaponstate);

    if (cg_drawWeaponIconFlash.integer >=
        2) // ETPro's extended weapon icon flash
    {
      CG_DrawPlayerWeaponIcon(
          &rect, (ws != WSTATE_IDLE) ? qtrue : qfalse, ITEM_ALIGN_RIGHT,
          ((ws == WSTATE_SWITCH || ws == WSTATE_RELOAD) ? &colorYellow
           : (ws == WSTATE_FIRE)                        ? &colorRed
                                                        : &colorWhite));
    } else {
      CG_DrawPlayerWeaponIcon(&rect, (ws != WSTATE_IDLE) ? qtrue : qfalse,
                              ITEM_ALIGN_RIGHT,
                              ((ws == WSTATE_SWITCH) ? &colorWhite
                               : (ws == WSTATE_FIRE) ? &colorRed
                                                     : &colorYellow));
    }
  }

  if (etj_HUD_weaponIcon.integer) {
    // Draw ammo
    value = value2 = value3 = 0;
    CG_PlayerAmmoValue(&value, &value2, &value3);

    if (value3 >= 0) {
      Com_sprintf(buffer, sizeof(buffer), "%i|%i/%i", value3, value, value2);
      CG_Text_Paint_Ext(
          SCREEN_WIDTH - 22 -
              CG_Text_Width_Ext(buffer, .25f, 0, &cgs.media.limboFont1),
          480 - 1 * (16 + 2) + 12 - 4, .25f, .25f, colorWhite, buffer, 0, 0,
          ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont1);
      //		CG_DrawPic( 640 - 2 * ( 12 + 2 )
      //- 16 - 4, 480 - 1 * (
      // 16 + 2 ) - 4, 16, 16,
      // cgs.media.SPPlayerInfoAmmoIcon );
    } else if (value2 >= 0) {
      Com_sprintf(buffer, sizeof(buffer), "%i/%i", value, value2);
      CG_Text_Paint_Ext(
          SCREEN_WIDTH - 22 -
              CG_Text_Width_Ext(buffer, .25f, 0, &cgs.media.limboFont1),
          480 - 1 * (16 + 2) + 12 - 4, .25f, .25f, colorWhite, buffer, 0, 0,
          ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont1);
      //		CG_DrawPic( 640 - 2 * ( 12 + 2 )
      //- 16 - 4, 480 - 1 * (
      // 16 + 2 ) - 4, 16, 16,
      // cgs.media.SPPlayerInfoAmmoIcon );
    } else if (value >= 0) {
      Com_sprintf(buffer, sizeof(buffer), "%i", value);
      CG_Text_Paint_Ext(
          SCREEN_WIDTH - 22 -
              CG_Text_Width_Ext(buffer, .25f, 0, &cgs.media.limboFont1),
          480 - 1 * (16 + 2) + 12 - 4, .25f, .25f, colorWhite, buffer, 0, 0,
          ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont1);
      //		CG_DrawPic( 640 - 2 * ( 12 + 2 )
      //- 16 - 4, 480 - 1 * (
      // 16 + 2 ) - 4, 16, 16,
      // cgs.media.SPPlayerInfoAmmoIcon );
    }
  }

  // ==
  if (etj_HUD_healthBar.integer) {
    rect.x = 24;
    rect.y = 480 - 92;
    rect.w = 12;
    rect.h = 72;
    CG_DrawPlayerHealthBar(&rect);
  }
  // ==

  // ==
  if (etj_HUD_fatigueBar.integer) {
    rect.x = 4;
    rect.y = 480 - 92;
    rect.w = 12;
    rect.h = 72;
    CG_DrawStaminaBar(&rect);
  }
  // ==

  // ==
  if (etj_HUD_chargeBar.integer) {
    rect.x = SCREEN_WIDTH - 16;
    rect.y = 480 - 92;
    rect.w = 12;
    rect.h = 72;
    CG_DrawWeapRecharge(&rect);
  }
  // ==
}

static void CG_DrawSkillBar(float x, float y, float w, float h, int skill) {
  int i;
  float blockheight = (h - 4) / (float)(NUM_SKILL_LEVELS - 1);
  float draw_y;
  vec4_t colour;
  float x1, y1, w1, h1;

  draw_y = y + h - blockheight;
  for (i = 0; i < NUM_SKILL_LEVELS - 1; i++) {
    if (i >= skill) {
      Vector4Set(colour, 1.f, 1.f, 1.f, .15f);
    } else {
      Vector4Set(colour, 0.f, 0.f, 0.f, .4f);
    }

    CG_FillRect(x, draw_y, w, blockheight, colour);

    if (i < skill) {
      x1 = x;
      y1 = draw_y;
      w1 = w;
      h1 = blockheight;
      CG_AdjustFrom640(&x1, &y1, &w1, &h1);

      trap_R_DrawStretchPic(x1, y1, w1, h1, 0, 0, 1.f, 0.5f,
                            cgs.media.limboStar_roll);
    }

    CG_DrawRect_FixedBorder(x, draw_y, w, blockheight, 1, colorBlack);
    //		CG_DrawPic( x, draw_y, w, blockheight,
    // cgs.media.hudBorderVert2
    //);
    draw_y -= (blockheight + 1);
  }
}

inline constexpr float SKILL_ICON_SIZE = 14.0f;

inline constexpr float SKILLS_X = 112.0f;
inline constexpr float SKILLS_Y = 20.0f;

inline constexpr float SKILL_BAR_X_INDENT = 0.0f;
inline constexpr float SKILL_BAR_Y_INDENT = 6.0f;
inline constexpr float SKILL_BAR_OFFSET = 2 * SKILL_BAR_X_INDENT;

inline constexpr float SKILL_BAR_WIDTH = SKILL_ICON_SIZE - SKILL_BAR_OFFSET;
inline constexpr float SKILL_BAR_X =
    SKILL_BAR_OFFSET + SKILL_BAR_X_INDENT + SKILLS_X;
inline constexpr float SKILL_BAR_X_SCALE = SKILL_ICON_SIZE + 2;
inline constexpr float SKILL_ICON_X = SKILL_BAR_OFFSET + SKILLS_X;
inline constexpr float SKILL_ICON_X_SCALE = SKILL_ICON_SIZE + 2;
inline constexpr float SKILL_BAR_Y =
    SKILL_BAR_Y_INDENT - SKILL_BAR_OFFSET - SKILLS_Y;
inline constexpr float SKILL_BAR_Y_SCALE = SKILL_ICON_SIZE + 2;
inline constexpr float SKILL_ICON_Y =
    -(SKILL_ICON_SIZE + 2) - SKILL_BAR_OFFSET - SKILLS_Y;

skillType_t CG_ClassSkillForPosition(clientInfo_t *ci, int pos) {
  switch (pos) {
    case 0:
      return BG_ClassSkillForClass(ci->cls);
    case 1:
      return SK_BATTLE_SENSE;
    case 2:
      return SK_LIGHT_WEAPONS;
  }

  return SK_BATTLE_SENSE;
}

static void CG_DrawPlayerStats(void) {
  int value = 0;
  playerState_t *ps;
  clientInfo_t *ci;
  skillType_t skill;
  int i;
  const char *str;
  float w;
  vec_t *clr;

  if (etj_HUD_playerHealth.integer) {
    str = va("%i", cg.snap->ps.stats[STAT_HEALTH]);
    w = CG_Text_Width_Ext(str, 0.25f, 0, &cgs.media.limboFont1);
    CG_Text_Paint_Ext(SKILLS_X - 28 - w, 480 - 4, 0.25f, 0.25f, colorWhite, str,
                      0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont1);
    CG_Text_Paint_Ext(SKILLS_X - 28 + 2, 480 - 4, 0.2f, 0.2f, colorWhite, "HP",
                      0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont1);
  }

  ps = &cg.snap->ps;
  ci = &cgs.clientinfo[ps->clientNum];

  if (etj_HUD_xpInfo.integer) {

    for (i = 0; i < 3; i++) {
      skill = CG_ClassSkillForPosition(ci, i);

      CG_DrawSkillBar(i * SKILL_BAR_X_SCALE + SKILL_BAR_X,
                      480 - (5 * SKILL_BAR_Y_SCALE) + SKILL_BAR_Y,
                      SKILL_BAR_WIDTH, 4 * SKILL_ICON_SIZE, ci->skill[skill]);
      CG_DrawPic(i * SKILL_ICON_X_SCALE + SKILL_ICON_X, 480 + SKILL_ICON_Y,
                 SKILL_ICON_SIZE, SKILL_ICON_SIZE, cgs.media.skillPics[skill]);
    }

    if (cg.time - cg.xpChangeTime < 1000) {
      clr = colorYellow;
    } else {
      clr = colorWhite;
    }

    str = va("%i", cg.snap->ps.stats[STAT_XP]);
    w = CG_Text_Width_Ext(str, 0.25f, 0, &cgs.media.limboFont1);
    CG_Text_Paint_Ext(SKILLS_X + 28 - w, 480 - 4, 0.25f, 0.25f, clr, str, 0, 0,
                      ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont1);
    CG_Text_Paint_Ext(SKILLS_X + 28 + 2, 480 - 4, 0.2f, 0.2f, clr, "XP", 0, 0,
                      ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont1);
  }

  // draw treasure icon if we have the flag
  // rain - #274 - use the playerstate instead of the clientinfo
  if (ps->powerups[PW_REDFLAG] || ps->powerups[PW_BLUEFLAG]) {
    trap_R_SetColor(NULL);
    CG_DrawPic(SCREEN_WIDTH - 40, 480 - 140 - value, 36, 36,
               cgs.media.objectiveShader);
  } else if (ps->powerups[PW_OPS_DISGUISED]) // Disguised?
  {
    CG_DrawPic(SCREEN_WIDTH - 40, 480 - 140 - value, 36, 36,
               ps->persistant[PERS_TEAM] == TEAM_AXIS
                   ? cgs.media.alliedUniformShader
                   : cgs.media.axisUniformShader);
  }
}

static char statsDebugStrings[6][512];
static int statsDebugTime[6];
static int statsDebugTextWidth[6];
static int statsDebugPos;

void CG_InitStatsDebug(void) {
  memset(&statsDebugStrings, 0, sizeof(statsDebugStrings));
  memset(&statsDebugTime, 0, sizeof(statsDebugTime));
  statsDebugPos = -1;
}

void CG_StatsDebugAddText(const char *text) {
  if (cg_debugSkills.integer) {
    statsDebugPos++;

    if (statsDebugPos >= 6) {
      statsDebugPos = 0;
    }

    Q_strncpyz(statsDebugStrings[statsDebugPos], text, 512);
    statsDebugTime[statsDebugPos] = cg.time;
    statsDebugTextWidth[statsDebugPos] =
        CG_Text_Width_Ext(text, .15f, 0, &cgs.media.limboFont2);

    CG_Printf("%s\n", text);
  }
}

static void CG_DrawStatsDebug(void) {
  int textWidth = 0;
  int i, x, y, w, h;

  if (!cg_debugSkills.integer) {
    return;
  }

  for (i = 0; i < 6; i++) {
    if (statsDebugTime[i] + 9000 > cg.time) {
      if (statsDebugTextWidth[i] > textWidth) {
        textWidth = statsDebugTextWidth[i];
      }
    }
  }

  w = textWidth + 6;
  h = 9;
  x = SCREEN_WIDTH - w;
  y = (480 - 5 * (12 + 2) + 6 - 4) - 6 - h; // don't ask

  i = statsDebugPos;

  do {
    vec4_t colour;

    if (statsDebugTime[i] + 9000 <= cg.time) {
      break;
    }

    colour[0] = colour[1] = colour[2] = .5f;
    if (cg.time - statsDebugTime[i] > 5000) {
      colour[3] = .5f - .5f * ((cg.time - statsDebugTime[i] - 5000) / 4000.f);
    } else {
      colour[3] = .5f;
    }
    CG_FillRect(x, y, w, h, colour);

    colour[0] = colour[1] = colour[2] = 1.f;
    if (cg.time - statsDebugTime[i] > 5000) {
      colour[3] = 1.f - ((cg.time - statsDebugTime[i] - 5000) / 4000.f);
    } else {
      colour[3] = 1.f;
    }
    CG_Text_Paint_Ext(640.f - 3 - statsDebugTextWidth[i], y + h - 2, .15f, .15f,
                      colour, statsDebugStrings[i], 0, 0, ITEM_TEXTSTYLE_NORMAL,
                      &cgs.media.limboFont2);

    y -= h;

    i--;
    if (i < 0) {
      i = 6 - 1;
    }
  } while (i != statsDebugPos);
}

// bani
void CG_DrawDemoRecording() {
  if (!cl_demorecording.integer && !cl_waverecording.integer) {
    return;
  }

  if (!etj_drawRecordingStatus.integer) {
    return;
  }

  std::string demoStatus;
  std::string waveStatus;

  if (cl_demorecording.integer) {
    demoStatus = ETJump::stringFormat(" demo %s: %ik ", cl_demofilename.string,
                                      cl_demooffset.integer / 1024);
  }

  if (cl_waverecording.integer) {
    waveStatus = ETJump::stringFormat(" audio: %s %ik ", cl_wavefilename.string,
                                      cl_waveoffset.integer / 1024);
  }

  const std::string status =
      ETJump::stringFormat("RECORDING%s%s", demoStatus, waveStatus);

  const float x = ETJump_AdjustPosition(etj_recordingStatusX.value);
  const float y = etj_recordingStatusY.value;

  CG_Text_Paint_Ext(x, y, 0.2f, 0.2f, colorWhite, status, 0, 0,
                    ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont2);
}

/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D() {
  CG_ScreenFade();
  // Arnout: no 2d when in esc menu
  // FIXME: do allow for quickchat (bleh)
  // Gordon: Removing for now
  /*	if( trap_Key_GetCatcher() & KEYCATCH_UI ) {
          return;
      }*/

  if (cg.snap->ps.pm_type == PM_INTERMISSION) {
    CG_DrawIntermission();
    return;
  }

  if (cg.editingSpeakers) {
    CG_SpeakerEditorDraw();
    return;
  }

  // bani - #127 - no longer cheat protected, we draw crosshair/reticle
  // in non demoplayback
  if (cg_draw2D.integer == 0) {
    if (cg.demoPlayback) {
      return;
    }
    CG_CheckForReticle();

    // crosshair is the only renderable that should be drawn here
    for (const auto &r : ETJump::renderables) {
      if (const auto &crosshair =
              std::dynamic_pointer_cast<ETJump::Crosshair>(r)) {
        if (crosshair->beforeRender()) {
          crosshair->render();
        }

        break;
      }
    }
    CG_DrawFlashFade();
    return;
  }

  if (!cg.cameraMode) {
    if (!cgs.demoCam.renderingFreeCam) {
      CG_DrawFlashBlendBehindHUD();
    }

    if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ||
        cgs.demoCam.renderingFreeCam) {
      CG_DrawSpectator();
    } else {
      // don't draw any status if dead
      if (cg.snap->ps.stats[STAT_HEALTH] > 0 ||
          (cg.snap->ps.pm_flags & PMF_FOLLOW)) {
        CG_CheckForReticle();
        CG_DrawNoShootIcon();
      }

      if (cg_drawStatus.integer) {
        Menu_PaintAll();
        CG_DrawTimedMenus();
      }
    }
  }

  // don't draw center string if scoreboard is up
  if (!CG_DrawScoreboard()) {
    if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR &&
        !cgs.demoCam.renderingFreeCam) {
      rectDef_t rect;

      if (cg.snap->ps.stats[STAT_HEALTH] > 0) {
        if (etj_HUD_playerHead.integer) {
          CG_DrawPlayerStatusHead();
        }
        CG_DrawPlayerStatus();
        CG_DrawPlayerStats();
      }

      // Cursor hint
      rect.w = rect.h = 48;
      rect.x = .5f * SCREEN_WIDTH - .5f * rect.w;
      rect.y = 260;
      CG_DrawCursorhint(&rect);

      // Stability bar
      rect.x = SCREEN_OFFSET_X + 50;
      rect.y = 208;
      rect.w = 10;
      rect.h = 64;
      CG_DrawWeapStability(&rect);

      // Stats Debugging
      CG_DrawStatsDebug();
    }

    if (!cg.cameraMode) {
      CG_DrawTeamInfo();
      if (!cg.showRtvMenu) {
        CG_DrawVote();
      }
      CG_DrawLagometer();
    }

    CG_DrawBannerPrint();
    CG_DrawCenterString();
    CG_DrawPMItems();
    CG_DrawPMItemsBig();
    CG_DrawWarmup();

    CG_DrawObjectiveInfo();

    CG_DrawSpectatorMessage();

    CG_DrawLimboMessage();

    if (!cgs.demoCam.renderingFreeCam) {
      CG_DrawLagometer();
      CG_DrawFollow();
      CG_DrawSlick();
      CG_DrawJumpDelay();
    }

    CG_DrawCHS();
  } else {
    if (cgs.eventHandling != CGAME_EVENT_NONE) {
      //			qboolean old =
      // cg.showGameView;

      //			cg.showGameView =
      // qfalse;
      // draw cursor
      trap_R_SetColor(NULL);
      CG_DrawPic(cgDC.cursorx - 14, cgDC.cursory - 14, 32, 32,
                 cgs.media.cursorIcon);
      //			cg.showGameView = old;
    }
  }

  if (!cgs.demoCam.renderingFreeCam) {
    for (const auto &r : ETJump::renderables) {
      if (r->beforeRender()) {
        r->render();
      }
    }

    ETJump_DrawDrawables();
  }

  if (cg.showFireteamMenu) {
    CG_Fireteams_Draw();
  }

  if (!cg.cameraMode && (cg.snap->ps.stats[STAT_HEALTH] > 0 ||
                         (cg.snap->ps.pm_flags & PMF_FOLLOW))) {
    CG_DrawCrosshairNames();
  }

  // Info overlays
  CG_DrawOverlays();

  if (!cgs.demoCam.renderingFreeCam) {
    // OSP - window updates
    CG_windowDraw();

    // Ridah, draw flash blends now
    CG_DrawFlashBlend();
  }

  CG_DrawDemoRecording();

  if (!ETJump::showingScores() && !cgs.demoCam.renderingFreeCam) {
    CG_DrawNewCompass();
  }
}

// NERVE - SMF
void CG_StartShakeCamera(float p, entityState_t *es) {
  auto attacker = static_cast<char>(es->clientNum);

  // ETJump: never shake cam from explosions
  if (etj_explosivesShake.integer == 0) {
    return;
  }

  // ETJump: shake cam only from other players explosives
  if (etj_explosivesShake.integer == 1 && attacker == cg.clientNum) {
    return;
  }

  // ETJump: shake cam only from own explosives
  if (etj_explosivesShake.integer == 2 && attacker != cg.clientNum) {
    return;
  }

  cg.cameraShakeScale = p;
  cg.cameraShakeLength = 1000 * (p * p);
  cg.cameraShakeTime = cg.time + cg.cameraShakeLength;
  cg.cameraShakePhase = crandom() * M_PI; // start chain in random dir
}

void CG_ShakeCamera() {
  float x, val;

  if (cg.time > cg.cameraShakeTime) {
    cg.cameraShakeScale = 0; // JPW NERVE all pending explosions
                             // resolved, so reset shakescale
    return;
  }

  // JPW NERVE starts at 1, approaches 0 over time
  x = (cg.cameraShakeTime - cg.time) / cg.cameraShakeLength;

// ydnar: move the camera
#if 0
	// up/down
	val = sin(M_PI * 8 * x + cg.cameraShakePhase) * x * 18.0f * cg.cameraShakeScale;
	cg.refdefViewAngles[0] += val;

	// left/right
	val = sin(M_PI * 15 * x + cg.cameraShakePhase) * x * 16.0f * cg.cameraShakeScale;
	cg.refdefViewAngles[1] += val;
#else
  // move
  val =
      sin(M_PI * 7 * x + cg.cameraShakePhase) * x * 4.0f * cg.cameraShakeScale;
  cg.refdef.vieworg[2] += val;
  val =
      sin(M_PI * 13 * x + cg.cameraShakePhase) * x * 4.0f * cg.cameraShakeScale;
  cg.refdef.vieworg[1] += val;
  val =
      cos(M_PI * 17 * x + cg.cameraShakePhase) * x * 4.0f * cg.cameraShakeScale;
  cg.refdef.vieworg[0] += val;
#endif

  AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);
}
// -NERVE - SMF

void CG_DrawMiscGamemodels() {
  int i, j;
  refEntity_t ent;

  memset(&ent, 0, sizeof(ent));

  ETJump_SetEntityRGBA(&ent, 1.0f, 1.0f, 1.0f, 1.0f);

  ent.reType = RT_MODEL;
  ent.nonNormalizedAxes = qtrue;

  // ydnar: static gamemodels don't project shadows
  ent.renderfx = RF_NOSHADOW;

  for (i = 0; i < cg.numMiscGameModels; i++) {
    if (cgs.miscGameModels[i].radius) {
      if (CG_CullPointAndRadius(cgs.miscGameModels[i].org,
                                cgs.miscGameModels[i].radius)) {
        continue;
      }
    }

    if (!trap_R_inPVS(cg.refdef_current->vieworg, cgs.miscGameModels[i].org)) {
      continue;
    }

    VectorCopy(cgs.miscGameModels[i].org, ent.origin);
    VectorCopy(cgs.miscGameModels[i].org, ent.oldorigin);
    VectorCopy(cgs.miscGameModels[i].org, ent.lightingOrigin);

    for (j = 0; j < 3; j++) {
      VectorCopy(cgs.miscGameModels[i].axes[j], ent.axis[j]);
    }
    ent.hModel = cgs.miscGameModels[i].model;

    trap_R_AddRefEntityToScene(&ent);
  }
}

namespace ETJump {
static void drawCoronas() {
  if (demoCompatibility->flags.serverSideCoronas) {
    return;
  }

  for (int i = 0; i < cg.numCoronas; i++) {
    centity_t *corona = &cgs.coronas[i];

    // only coronas that are in your PVS are being added
    if (!trap_R_inPVS(cg.refdef_current->vieworg, corona->lerpOrigin)) {
      continue;
    }

    CG_Corona(corona);
  }
}

static void drawClientDlights() {
  if (demoCompatibility->flags.serverSideDlights) {
    return;
  }

  for (int i = 0; i < cg.numDlights; i++) {
    centity_t *dlight = &cgs.dlights[i];

    if (!trap_R_inPVS(cg.refdef_current->vieworg, dlight->lerpOrigin)) {
      continue;
    }

    CG_AddLightstyle(dlight);
  }
}
}

void SetFov(float fov) {
  char buf[16];

  cg.currentFovValue = fov;
  Com_sprintf(buf, sizeof(buf), "%f", fov);
  trap_Cvar_Set("cg_fov", buf);
  trap_Cvar_Update(&cg_fov);
}

static void CG_ChangeFovBasedOnSpeed() {
  float speed = sqrt(cg.predictedPlayerState.velocity[0] *
                         cg.predictedPlayerState.velocity[0] +
                     cg.predictedPlayerState.velocity[1] *
                         cg.predictedPlayerState.velocity[1]);
  float speedDiff = speed - movie_fovMinSpeed.value;
  float additionalFov = movie_fovMax.value - movie_fovMin.value;
  float minMaxSpeedDiff = movie_fovMaxSpeed.value - movie_fovMinSpeed.value;
  float currentSpeedFov = movie_fovMin.value;

  if (!movie_changeFovBasedOnSpeed.integer || !cg.demoPlayback ||
      cgs.demoCam.renderingFreeCam) {
    return;
  }

  if (speed < movie_fovMinSpeed.value) {
    if (cg.currentFovValue - movie_fovIncreasePerFrame.value <
        movie_fovMin.value) {
      SetFov(movie_fovMin.value);
    } else {
      SetFov(cg.currentFovValue - movie_fovIncreasePerFrame.value);
    }

    return;
  }

  if (speed > movie_fovMaxSpeed.value) {
    if (cg.currentFovValue + movie_fovIncreasePerFrame.value >
        movie_fovMin.value) {
      SetFov(movie_fovMax.value);
    } else {
      SetFov(cg.currentFovValue + movie_fovIncreasePerFrame.value);
    }

    return;
  }

  if (minMaxSpeedDiff < 0) {
    minMaxSpeedDiff = 0;
  }

  currentSpeedFov =
      movie_fovMin.value + additionalFov * (speedDiff / minMaxSpeedDiff);

  if (developer.integer) {
    CG_Printf("Speed: %f\nFov: %f\nCurrent fov: %f\n", speed, currentSpeedFov,
              cg.currentFovValue);
  }

  if (abs(cg.currentFovValue - currentSpeedFov) <
      movie_fovIncreasePerFrame.value) {
    return;
  }

  // We need to increase the current fov value
  if (cg.currentFovValue < currentSpeedFov) {
    SetFov(cg.currentFovValue + movie_fovIncreasePerFrame.value);
  } else if (cg.currentFovValue > currentSpeedFov) // We need to decrease the
                                                   // current fov value
  {
    SetFov(cg.currentFovValue - movie_fovIncreasePerFrame.value);
  }
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive(stereoFrame_t stereoView) {
  float separation;
  vec3_t baseOrg;

  // optionally draw the info screen instead
  if (!cg.snap) {
    CG_DrawInformation(qfalse);
    return;
  }

  // optionally draw the tournement scoreboard instead
  /*if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR &&
      ( cg.snap->ps.pm_flags & PMF_SCOREBOARD ) ) {
      CG_DrawTourneyScoreboard();
      return;
  }*/

  switch (stereoView) {
    case STEREO_CENTER:
      separation = 0;
      break;
    case STEREO_LEFT:
      separation = -cg_stereoSeparation.value / 2;
      break;
    case STEREO_RIGHT:
      separation = cg_stereoSeparation.value / 2;
      break;
    default:
      separation = 0;
      CG_Error("CG_DrawActive: Undefined stereoView");
  }

  // clear around the rendered view if sized down
  CG_TileClear();

  // offset vieworg appropriately if we're doing stereo separation
  VectorCopy(cg.refdef_current->vieworg, baseOrg);
  if (separation != 0) {
    VectorMA(cg.refdef_current->vieworg, -separation,
             cg.refdef_current->viewaxis[1], cg.refdef_current->vieworg);
  }

  cg.refdef_current->glfog.registered =
      qfalse; // make sure it doesn't use fog from another scene

  CG_ActivateLimboMenu();

  //	if( cgs.ccCurrentCamObjective == -1 ) {
  //		if( cg.showGameView ) {
  //			CG_FillRect( 0, 0, 640, 480, colorBlack );
  //			CG_LimboPanel_Draw();
  //			return;
  //		}
  //	}

  CG_ShakeCamera(); // NERVE - SMF

  // Gordon
  CG_PB_RenderPolyBuffers();

  // Gordon
  CG_DrawMiscGamemodels();

  ETJump::drawCoronas();
  ETJump::drawClientDlights();

  if (!(cg.limboEndCinematicTime > cg.time && cg.showGameView)) {
    for (int i = 0; i < MAX_RENDER_STRINGS; i++) {
      Q_strncpyz(cg.refdef_current->text[i], cg.deformText[i],
                 sizeof(cg.refdef_current->text[0]));
    }

    trap_R_RenderScene(cg.refdef_current);
  }

  // restore original viewpoint if running stereo
  if (separation != 0) {
    VectorCopy(baseOrg, cg.refdef_current->vieworg);
  }

  CG_ChangeFovBasedOnSpeed();

  if (!cg.showGameView) {
    // draw status bar and other floating elements
    CG_Draw2D();
  } else {
    CG_LimboPanel_Draw();
  }
}
