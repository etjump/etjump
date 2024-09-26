

/*
=======================================================================

USER INTERFACE MAIN

=======================================================================
*/

#include <cmath>
#include <memory>

#include "ui_local.h"
#include "etj_colorpicker.h"

#include "../game/etj_string_utilities.h"
#include "../cgame/etj_utilities.h"
#include "../game/etj_numeric_utilities.h"
#include "../game/etj_filesystem.h"

// NERVE - SMF
#define AXIS_TEAM 0
#define ALLIES_TEAM 1
#define SPECT_TEAM 2
// -NERVE - SMF

extern qboolean g_waitingForKey;
extern qboolean g_editingField;
extern itemDef_t *g_editItem;

uiInfo_t uiInfo;

static const char *MonthAbbrev[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static const serverFilter_t serverFilters[] = {{"All", ""}};

static const int numServerFilters =
    sizeof(serverFilters) / sizeof(serverFilter_t);

static const char *netnames[] = {"???", "UDP", "IPX", NULL};

static int gamecodetoui[] = {4, 2, 3, 0, 5, 1, 6};
static int uitogamecode[] = {4, 6, 2, 3, 1, 5, 7};

static int ui_serverFilterType = 0;

static char uiPreviousMenu[256]{};

static constexpr char DEFAULT_MENU_FILE[] = "ui/menus.txt";

// NERVE - SMF - enabled for multiplayer
static void UI_StartServerRefresh(qboolean full);
static void UI_StopServerRefresh(void);
static void UI_DoServerRefresh(void);
static void UI_FeederSelection(float feederID, int index);
qboolean UI_FeederSelectionClick(itemDef_t *item);
static void UI_BuildServerDisplayList(int force);
static void UI_BuildServerStatus(qboolean force);
static void UI_BuildFindPlayerList(qboolean force);
static int QDECL UI_ServersQsortCompare(const void *arg1, const void *arg2);
static int UI_MapCountByGameType();
static const char *UI_SelectedMap(int index, int *actual);

qboolean UI_CheckExecKey(int key);
// -NERVE - SMF - enabled for multiplayer

static void UI_ParseGameInfo(const char *teamFile);
// static void UI_ParseTeamInfo(const char *teamFile); // TTimo: unused

// int ProcessNewUI( int command, int arg0, int arg1, int arg2, int arg3, int
// arg4, int arg5, int arg6 );

itemDef_t *Menu_FindItemByName(menuDef_t *menu, const char *p);
void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow);

#define ITEM_GRENADES 1
#define ITEM_MEDKIT 2

#define ITEM_PISTOL 1

#define DEFAULT_PISTOL

#define PT_KNIFE (1)
#define PT_PISTOL (1 << 2)
#define PT_RIFLE (1 << 3)
#define PT_LIGHTONLY (1 << 4)
#define PT_GRENADES (1 << 5)
#define PT_EXPLOSIVES (1 << 6)
#define PT_MEDKIT (1 << 7)

// TTimo
static char translated_yes[4], translated_no[4];

typedef struct {
  const char *name;
  int items;
} playerType_t;

/*typedef struct {
    int			weapindex;

    const char	*desc;
//	int			flags;
    const char	*cvar;
    int			value;
    const char	*shadername;

    const char	*torso_anim;
    const char	*legs_anim;

//	const char	*large_shader;
} weaponType_t;*/

#define ENG_WEAPMASK_1 (0 | 1 | 2)
#define ENG_WEAPMASK_2 (4 | 8)

// NERVE - SMF - this is the weapon info list [what can and can't be used by
// character classes]
//   - This list is seperate from the actual text names in the listboxes for
//   localization purposes.
//   - The list boxes look up this list by the cvar value.
// Gordon: stripped out some useless stuff, and moved some other stuff to
// generic class stuff
/*static weaponType_t weaponTypes[] = {
    { 0,					"NULL",
"none", -1, "none",
"",						""
},

    { WP_COLT,				"1911 pistol",
"mp_weapon", -1, "ui/assets/weapon_colt1911.tga",
"firing_pistolB_1", "stand_pistolB"			}, { WP_LUGER,
"Luger pistol",				"mp_weapon",	-1,
"ui/assets/weapon_luger.tga",			"firing_pistolB_1",
"stand_pistolB"			},

    { WP_MP40,				"MP 40",
"mp_weapon", 0, "ui/assets/weapon_mp40.tga",
"relaxed_idle_2h_1", "relaxed_idle_2h_1"		}, { WP_THOMPSON,
"Thompson",					"mp_weapon", 1,
"ui/assets/weapon_thompson.tga",		"relaxed_idle_2h_1",
"relaxed_idle_2h_1"		}, { WP_STEN,				"Sten",
"mp_weapon",	2,	"ui/assets/weapon_sten.tga",
"relaxed_idle_2h_1",	"relaxed_idle_2h_1"		},

    { WP_PANZERFAUST,		"Panzerfaust",
"mp_weapon", 4, "ui/assets/weapon_panzerfaust.tga",		"stand_panzer",
"stand_panzer"
}, { WP_FLAMETHROWER,		"Flamethrower", "mp_weapon",	6,
"ui/assets/weapon_flamethrower.tga",
"stand_machinegun",		"stand_machinegun"		},

    { WP_GRENADE_PINEAPPLE,	"Pineapple grenade",		"mp_weapon_2",
8, "ui/assets/weapon_grenade.tga",			"firing_pistolB_1",
"stand_pistolB"			}, { WP_GRENADE_LAUNCHER,	"Stick grenade",
"mp_weapon_2",	8,	"ui/assets/weapon_grenade_ger.tga",
"firing_pistolB_1",		"stand_pistolB"			},

    { WP_DYNAMITE,			"Explosives",
"mp_item2", -1, "ui/assets/weapon_dynamite.tga",
"firing_pistolB_1", "stand_pistolB"			},

    { WP_KAR98,				"Kar98",                    "mp_weapon",
2, "ui/assets/weapon_kar98.tga",			"stand_rifle",
"stand_rifle"			},
    { WP_CARBINE,			"M1 Garand",                "mp_weapon",
2, "ui/assets/weapon_carbine.tga",			"stand_rifle",
"stand_rifle"			},

    { WP_FG42,				"FG42",
"mp_weapon", 7, "ui/assets/weapon_fg42.tga",			"stand_rifle",
"stand_rifle"
}, { WP_GARAND,			"M1 Garand", "mp_weapon",	8,
"ui/assets/weapon_carbine.tga", "stand_rifle",
"stand_rifle"			}, { WP_MOBILE_MG42,		"Mobile MG42",
"mp_weapon",	9, "ui/assets/weapon_mg42.tga",			"stand_rifle",
"stand_rifle"			},

    { WP_LANDMINE,			"Land Mines",
"mp_weapon_2", 4, "ui/assets/weapon_landmine.tga",
"firing_pistolB_1", "stand_pistolB"			},

    { WP_K43,				"K43",
"mp_weapon", 8, "ui/assets/weapon_kar98.tga",			"stand_rifle",
"stand_rifle"			},
//	{ WP_SATCHEL,			"Satchel Charges",
"mp_weapon",	10, "ui/assets/weapon_satchel.tga",
"firing_pistolB_1",		"stand_pistolB"			}, {
WP_TRIPMINE,			"Trip Mines",
"mp_weapon",	9,	"ui/assets/weapon_tripmine.tga",
"firing_pistolB_1",		"stand_pistolB"			},

    { 0,					NULL,
NULL, -1, NULL,					NULL },
};*/

typedef struct {
  char *name;
  int flags;
  char *shader;
} uiitemType_t;

#define UI_KNIFE_PIC "window_knife_pic"
#define UI_PISTOL_PIC "window_pistol_pic"
#define UI_WEAPON_PIC "window_weapon_pic"
#define UI_ITEM1_PIC "window_item1_pic"
#define UI_ITEM2_PIC "window_item2_pic"

#if 0 // rain - not used
static uiitemType_t itemTypes[] =
{
	{ UI_KNIFE_PIC,  PT_KNIFE,      "ui/assets/weapon_knife.tga"    },
	{ UI_PISTOL_PIC, PT_PISTOL,     "ui/assets/weapon_colt1911.tga" },

	{ UI_WEAPON_PIC, PT_RIFLE,      "ui/assets/weapon_mauser.tga"   },

	{ UI_ITEM1_PIC,  PT_MEDKIT,     "ui/assets/item_medkit.tga"     },

	{ UI_ITEM1_PIC,  PT_GRENADES,   "ui/assets/weapon_grenade.tga"  },
	{ UI_ITEM2_PIC,  PT_EXPLOSIVES, "ui/assets/weapon_dynamite.tga" },

	{ NULL,          0,             NULL                            }
};
#endif

extern displayContextDef_t *DC;

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .qvm file
================
*/
vmCvar_t ui_new;
vmCvar_t ui_debug;
vmCvar_t ui_initialized;
vmCvar_t ui_teamArenaFirstRun;

extern itemDef_t *g_bindItem;

void _UI_Init(int legacyClient, int clientVersion);
void _UI_Shutdown(void);
void _UI_KeyEvent(int key, qboolean down);
void _UI_MouseEvent(int dx, int dy);
void _UI_Refresh(int realtime);
qboolean _UI_IsFullscreen(void);

extern "C" FN_PUBLIC intptr_t vmMain(int command, intptr_t arg0, intptr_t arg1,
                                     intptr_t arg2, intptr_t arg3,
                                     intptr_t arg4, intptr_t arg5,
                                     intptr_t arg6) {
  switch (command) {
    case UI_GETAPIVERSION:
      return UI_API_VERSION;

    case UI_INIT:
      _UI_Init(arg1, arg2);
      return 0;

    case UI_SHUTDOWN:
      _UI_Shutdown();
      return 0;

    case UI_KEY_EVENT:
      _UI_KeyEvent(arg0, arg1 ? qtrue : qfalse);
      return 0;

    case UI_MOUSE_EVENT:
      _UI_MouseEvent(arg0, arg1);
      return 0;

    case UI_REFRESH:
      _UI_Refresh(arg0);
      return 0;

    case UI_IS_FULLSCREEN:
      return _UI_IsFullscreen();

    case UI_SET_ACTIVE_MENU:
      _UI_SetActiveMenu(static_cast<uiMenuCommand_t>(arg0));
      return 0;

    case UI_GET_ACTIVE_MENU:
      return _UI_GetActiveMenu();

    case UI_CONSOLE_COMMAND:
      return UI_ConsoleCommand(arg0);

    case UI_DRAW_CONNECT_SCREEN:
      UI_DrawConnectScreen(arg0 ? qtrue : qfalse);
      return 0;
    case UI_HASUNIQUECDKEY: // mod authors need to observe this
      return qtrue;
    // NERVE - SMF
    case UI_CHECKEXECKEY:
      return UI_CheckExecKey(arg0);
    case UI_WANTSBINDKEYS:
      return (g_waitingForKey && g_bindItem) ? qtrue : qfalse;
  }

  return -1;
}

void AssetCache() {
  int n;
  // if (Assets.textFont == NULL) {
  // }
  // Assets.background = trap_R_RegisterShaderNoMip( ASSET_BACKGROUND );
  // Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
  uiInfo.uiDC.Assets.gradientBar =
      trap_R_RegisterShaderNoMip(ASSET_GRADIENTBAR);
  uiInfo.uiDC.Assets.fxBasePic = trap_R_RegisterShaderNoMip(ART_FX_BASE);
  uiInfo.uiDC.Assets.fxPic[0] = trap_R_RegisterShaderNoMip(ART_FX_RED);
  uiInfo.uiDC.Assets.fxPic[1] = trap_R_RegisterShaderNoMip(ART_FX_YELLOW);
  uiInfo.uiDC.Assets.fxPic[2] = trap_R_RegisterShaderNoMip(ART_FX_GREEN);
  uiInfo.uiDC.Assets.fxPic[3] = trap_R_RegisterShaderNoMip(ART_FX_TEAL);
  uiInfo.uiDC.Assets.fxPic[4] = trap_R_RegisterShaderNoMip(ART_FX_BLUE);
  uiInfo.uiDC.Assets.fxPic[5] = trap_R_RegisterShaderNoMip(ART_FX_CYAN);
  uiInfo.uiDC.Assets.fxPic[6] = trap_R_RegisterShaderNoMip(ART_FX_WHITE);
  uiInfo.uiDC.Assets.scrollBar = trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR);
  uiInfo.uiDC.Assets.scrollBarArrowDown =
      trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWDOWN);
  uiInfo.uiDC.Assets.scrollBarArrowUp =
      trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWUP);
  uiInfo.uiDC.Assets.scrollBarArrowLeft =
      trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWLEFT);
  uiInfo.uiDC.Assets.scrollBarArrowRight =
      trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWRIGHT);
  uiInfo.uiDC.Assets.scrollBarThumb =
      trap_R_RegisterShaderNoMip(ASSET_SCROLL_THUMB);
  uiInfo.uiDC.Assets.sliderBar = trap_R_RegisterShaderNoMip(ASSET_SLIDER_BAR);
  uiInfo.uiDC.Assets.sliderThumb =
      trap_R_RegisterShaderNoMip(ASSET_SLIDER_THUMB);
  uiInfo.uiDC.Assets.checkboxCheck =
      trap_R_RegisterShaderNoMip(ASSET_CHECKBOX_CHECK);
  uiInfo.uiDC.Assets.checkboxCheckNot =
      trap_R_RegisterShaderNoMip(ASSET_CHECKBOX_CHECK_NOT);
  uiInfo.uiDC.Assets.checkboxCheckNo =
      trap_R_RegisterShaderNoMip(ASSET_CHECKBOX_CHECK_NO);

  for (n = 0; n < NUM_CROSSHAIRS; n++) {
    uiInfo.uiDC.Assets.crosshairShader[n] =
        ETJump::shaderForCrosshair(n, false);
    uiInfo.uiDC.Assets.crosshairAltShader[n] =
        ETJump::shaderForCrosshair(n, true);
  }

  uiInfo.uiDC.Assets.replayDirectory =
      trap_R_RegisterShaderNoMip(ASSET_REPLAY_DIRECTORY);
  uiInfo.uiDC.Assets.replayHome = trap_R_RegisterShaderNoMip(ASSET_REPLAY_HOME);
  uiInfo.uiDC.Assets.replayUp = trap_R_RegisterShaderNoMip(ASSET_REPLAY_UP);

  uiInfo.uiDC.Assets.colorPickerMask =
      trap_R_RegisterShaderNoMip(ASSET_COLORPICKER_MASK);
}

void _UI_DrawSides(float x, float y, float w, float h, float size) {
  UI_AdjustFrom640(&x, &y, &w, &h);
  size *= uiInfo.uiDC.xscale;
  trap_R_DrawStretchPic(x, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader);
  trap_R_DrawStretchPic(x + w - size, y, size, h, 0, 0, 0, 0,
                        uiInfo.uiDC.whiteShader);
}

void _UI_DrawTopBottom(float x, float y, float w, float h, float size) {
  UI_AdjustFrom640(&x, &y, &w, &h);
  size *= uiInfo.uiDC.yscale;
  trap_R_DrawStretchPic(x, y, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader);
  trap_R_DrawStretchPic(x, y + h - size, w, size, 0, 0, 0, 0,
                        uiInfo.uiDC.whiteShader);
}

void _UI_DrawSides_NoScale(float x, float y, float w, float h, float size) {
  UI_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader);
  trap_R_DrawStretchPic(x + w - size, y, size, h, 0, 0, 0, 0,
                        uiInfo.uiDC.whiteShader);
}

void _UI_DrawTopBottom_NoScale(float x, float y, float w, float h, float size) {
  UI_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader);
  trap_R_DrawStretchPic(x, y + h - size, w, size, 0, 0, 0, 0,
                        uiInfo.uiDC.whiteShader);
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void _UI_DrawRect(float x, float y, float width, float height, float size,
                  const float *color) {
  trap_R_SetColor(color);

  _UI_DrawTopBottom(x, y, width, height, size);
  _UI_DrawSides(x, y, width, height, size);

  trap_R_SetColor(NULL);
}

void _UI_DrawRect_DrawRect_FixedBorder(float x, float y, float width,
                                       float height, int size,
                                       const float *color) {
  trap_R_SetColor(color);

  _UI_DrawSides_NoScale(x, y, width, height, size);
  _UI_DrawTopBottom_NoScale(x, y, width, height, size);

  trap_R_SetColor(NULL);
}

// NERVE - SMF
void Text_SetActiveFont(int font) { uiInfo.activeFont = font; }

int Text_Width_Ext(const char *text, float scale, int limit, fontInfo_t *font) {
  int count, len;
  float out = 0;
  glyphInfo_t *glyph;
  const char *s = text;

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
  return out * scale * font->glyphScale;
}

int Text_Width(const char *text, float scale, int limit) {
  fontInfo_t *font = &uiInfo.uiDC.Assets.fonts[uiInfo.activeFont];
  return Text_Width_Ext(text, scale, limit, font);
}

fontInfo_t *GetActiveFont() {
  return &uiInfo.uiDC.Assets.fonts[uiInfo.activeFont];
}

int Multiline_Text_Width(const char *text, float scale, int limit) {
  const auto lines = ETJump::StringUtil::split(text, "\n");

  // if no linebreaks, just return the text width
  if (lines.size() == 1) {
    return Text_Width(text, scale, limit);
  }

  const auto linecount = static_cast<int>(lines.size());
  auto maxWidth = 0;

  for (int i = 0; i < linecount; i++) {
    const int width = Text_Width(lines[i].c_str(), scale, limit);

    if (width > maxWidth) {
      maxWidth = width;
    }
  }

  // add a bit of padding to account for uneven glyph spacing
  return maxWidth + Text_Width(" ", scale, limit);
}

int Text_Height_Ext(const char *text, float scale, int limit,
                    fontInfo_t *font) {
  int len, count;
  float max;
  glyphInfo_t *glyph;
  const char *s = text;

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
        glyph = &font->glyphs[(unsigned char)*s]; // NERVE - SMF -
                                                  // this needs to
                                                  // be an unsigned
                                                  // cast for
                                                  // localization
        if (max < glyph->height) {
          max = glyph->height;
        }
        s++;
        count++;
      }
    }
  }

  return max * scale * font->glyphScale;
}

int Text_Height(const char *text, float scale, int limit) {
  fontInfo_t *font = &uiInfo.uiDC.Assets.fonts[uiInfo.activeFont];

  return Text_Height_Ext(text, scale, limit, font);
}

// despite the name, this is also used for single line tooltips
int Multiline_Text_Height(const char *text, float scale, int limit) {
  const auto lines = ETJump::StringUtil::split(text, "\n");

  // single line, just return the text height
  if (lines.size() == 1) {
    return Text_Height(text, scale, limit);
  }

  glyphInfo_t *glyph;
  fontInfo_t *font = &uiInfo.uiDC.Assets.fonts[uiInfo.activeFont];
  const auto linecount = static_cast<int>(lines.size());
  int height = 0;

  // some glyphs (notably '_') have their top position on a negative offset,
  // we keep track of this so that on the last line, we can add this offset
  // to the total height, because Text_Height only looks for max glyph height
  // this is only done on last line, to keep line heights consistent
  int minTop = 0;

  for (int i = 0; i < linecount; i++) {
    if (i == linecount - 1) {
      for (const auto &c : lines[i]) {
        glyph = &font->glyphs[static_cast<unsigned char>(c)];

        if (glyph->top < minTop) {
          minTop = glyph->top;
        }
      }
    }

    height += Text_Height(lines[i].c_str(), scale, limit) + std::abs(minTop);
    minTop = 0;
  }

  return height + (AUTOWRAP_OFFSET * (linecount - 1));
}

void Text_PaintCharExt(float x, float y, float w, float h, float scalex,
                       float scaley, float s, float t, float s2, float t2,
                       qhandle_t hShader) {
  w *= scalex;
  h *= scaley;
  UI_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, w, h, s, t, s2, t2, hShader);
}

void Text_PaintChar(float x, float y, float w, float h, float scale, float s,
                    float t, float s2, float t2, qhandle_t hShader) {
  w *= scale;
  h *= scale;
  UI_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, w, h, s, t, s2, t2, hShader);
}

void Text_Paint_Ext(float x, float y, float scalex, float scaley, vec4_t color,
                    const char *text, float adjust, int limit, int style,
                    fontInfo_t *font) {
  int len, count;
  vec4_t newColor;
  glyphInfo_t *glyph;
  int index;

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
      index = (unsigned char)*s;

      // NERVE - SMF - don't draw tabs and newlines
      if (index < 20) {
        s++;
        count++;
        continue;
      }

      glyph = &font->glyphs[index];
      if (Q_IsColorString(s)) {
        if (*(s + 1) == COLOR_NULL) {
          memcpy(&newColor[0], &color[0], sizeof(vec4_t));
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
          Text_PaintCharExt(x + (glyph->pitch * scalex) + ofs * scalex,
                            y - yadj + ofs * scaley, glyph->imageWidth,
                            glyph->imageHeight, scalex, scaley, glyph->s,
                            glyph->t, glyph->s2, glyph->t2, glyph->glyph);
          trap_R_SetColor(newColor);

          colorBlack[3] = 1.0;
        }
        Text_PaintCharExt(x + (glyph->pitch * scalex), y - yadj,
                          glyph->imageWidth, glyph->imageHeight, scalex, scaley,
                          glyph->s, glyph->t, glyph->s2, glyph->t2,
                          glyph->glyph);

        x += (glyph->xSkip * scalex) + adjust;
        s++;
        count++;
      }
    }
    trap_R_SetColor(NULL);
  }
}

void Text_Paint(float x, float y, float scale, vec4_t color, const char *text,
                float adjust, int limit, int style) {
  fontInfo_t *font = &uiInfo.uiDC.Assets.fonts[uiInfo.activeFont];

  Text_Paint_Ext(x, y, scale, scale, color, text, adjust, limit, style, font);
}

void Text_PaintWithCursor(float x, float y, float scale, vec4_t color,
                          vec4_t cursorColor, const char *text, int cursorPos,
                          char cursor, int limit, int style) {
  int len, count;
  vec4_t newColor;
  glyphInfo_t *glyph, *glyph2;
  float yadj;
  float useScale;
  fontInfo_t *font = &uiInfo.uiDC.Assets.fonts[uiInfo.activeFont];

  useScale = scale * font->glyphScale;

  if (text) {
    const char *s = text;
    trap_R_SetColor(color);
    memcpy(&newColor[0], &color[0], sizeof(vec4_t));

    len = strlen(text);
    if (limit > 0 && len > limit) {
      len = limit;
    }
    count = 0;
    glyph2 = &font->glyphs[(unsigned char)cursor];
    while (s && *s && count < len) {
      glyph = &font->glyphs[(unsigned char)*s]; // NERVE - SMF - this needs
                                                // to be an unsigned cast
                                                // for localization
      // int yadj =
      // Assets.textFont.glyphs[text[i]].bottom +
      // Assets.textFont.glyphs[text[i]].top; float yadj
      // = scale *
      // (Assets.textFont.glyphs[text[i]].imageHeight -
      // Assets.textFont.glyphs[text[i]].height);
      /*if ( Q_IsColorString( s ) ) {
          memcpy( newColor,
      g_color_table[ColorIndex(*(s+1))], sizeof(
      newColor ) ); newColor[3] = color[3];
      trap_R_SetColor( newColor ); s += 2; continue; }
      else*/
      {
        yadj = useScale * glyph->top;
        if (style == ITEM_TEXTSTYLE_SHADOWED) {
          constexpr float ofs = 2.5f;
          colorBlack[3] = newColor[3];
          trap_R_SetColor(colorBlack);
          Text_PaintChar(x + (glyph->pitch * useScale) + ofs * useScale,
                         y - yadj + ofs * useScale, glyph->imageWidth,
                         glyph->imageHeight, useScale, glyph->s, glyph->t,
                         glyph->s2, glyph->t2, glyph->glyph);
          colorBlack[3] = 1.0;
          trap_R_SetColor(newColor);
        }
        Text_PaintChar(x + (glyph->pitch * useScale), y - yadj,
                       glyph->imageWidth, glyph->imageHeight, useScale,
                       glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);

        // CG_DrawPic(x, y - yadj, scale *
        // uiDC.Assets.textFont.glyphs[text[i]].imageWidth,
        // scale *
        // uiDC.Assets.textFont.glyphs[text[i]].imageHeight,
        // uiDC.Assets.textFont.glyphs[text[i]].glyph);
        yadj = useScale * glyph2->top;
        if (count == cursorPos &&
            !((uiInfo.uiDC.realTime / BLINK_DIVISOR) & 1)) {
          trap_R_SetColor(cursorColor);
          Text_PaintChar(x + (glyph->pitch * useScale), y - yadj,
                         glyph2->imageWidth, glyph2->imageHeight, useScale,
                         glyph2->s, glyph2->t, glyph2->s2, glyph2->t2,
                         glyph2->glyph);
          trap_R_SetColor(color);
        }

        x += (glyph->xSkip * useScale);
        s++;
        count++;
      }
    }
    // need to paint cursor at end of text
    if (cursorPos == len && !((uiInfo.uiDC.realTime / BLINK_DIVISOR) & 1)) {
      trap_R_SetColor(cursorColor);
      yadj = useScale * glyph2->top;
      Text_PaintChar(x + (glyph2->pitch * useScale), y - yadj,
                     glyph2->imageWidth, glyph2->imageHeight, useScale,
                     glyph2->s, glyph2->t, glyph2->s2, glyph2->t2,
                     glyph2->glyph);
    }

    trap_R_SetColor(NULL);
  }
}

static void Text_Paint_Limit(float *maxX, float x, float y, float scale,
                             vec4_t color, const char *text, float adjust,
                             int limit) {
  int len, count;
  vec4_t newColor;
  glyphInfo_t *glyph;
  if (text) {
    const char *s = text;
    float max = *maxX;
    float useScale;
    fontInfo_t *font = &uiInfo.uiDC.Assets.fonts[uiInfo.activeFont];

    useScale = scale * font->glyphScale;

    trap_R_SetColor(color);
    len = strlen(text);
    if (limit > 0 && len > limit) {
      len = limit;
    }
    count = 0;
    while (s && *s && count < len) {
      glyph = &font->glyphs[(unsigned char)*s]; // NERVE - SMF - this needs
                                                // to be an unsigned cast
                                                // for localization
      if (Q_IsColorString(s)) {
        if (*(s + 1) == COLOR_NULL) {
          memcpy(&newColor[0], &color[0], sizeof(vec4_t));
        } else {
          memcpy(newColor, g_color_table[ColorIndex(*(s + 1))],
                 sizeof(newColor));
          newColor[3] = color[3];
        }
        trap_R_SetColor(newColor);
        s += 2;
        continue;
      } else {
        float yadj = useScale * glyph->top;
        if (Text_Width(s, useScale, 1) + x > max) {
          *maxX = 0;
          break;
        }
        Text_PaintChar(x + (glyph->pitch * useScale), y - yadj,
                       glyph->imageWidth, glyph->imageHeight, useScale,
                       glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
        x += (glyph->xSkip * useScale) + adjust;
        *maxX = x;
        count++;
        s++;
      }
    }
    trap_R_SetColor(NULL);
  }
}

void UI_ShowPostGame(qboolean newHigh) {
  trap_Cvar_Set("cg_cameraOrbit", "0");
  trap_Cvar_Set("cg_thirdPerson", "0");
  trap_Cvar_Set("sv_killserver", "1");
  uiInfo.soundHighScore = newHigh;
  _UI_SetActiveMenu(UIMENU_POSTGAME);
}

namespace ETJump {
std::unique_ptr<ColorPicker> colorPicker;

static void initColorPicker() {
  colorPicker = std::make_unique<ColorPicker>();

  uiInfo.uiDC.updateSliderState = [p = colorPicker.get()](itemDef_t *item) {
    p->updateSliderState(item);
  };

  uiInfo.uiDC.cvarToColorPickerState =
      [p = colorPicker.get()](const std::string &cvar) {
        p->cvarToColorPickerState(cvar);
      };

  uiInfo.uiDC.resetColorPickerState = [p = colorPicker.get()] {
    p->resetColorPickerState();
  };

  uiInfo.uiDC.colorPickerDragFunc =
      [p = colorPicker.get()](itemDef_t *item, const float cursorX,
                              const float cursorY, const int key) {
        p->colorPickerDragFunc(item, cursorX, cursorY, key);
      };

  uiInfo.uiDC.toggleRGBSliderValues = [p = colorPicker.get()] {
    p->toggleRGBSliderValues();
  };

  uiInfo.uiDC.RGBSlidersAreNormalized = [p = colorPicker.get()] {
    return p->RGBSlidersAreNormalized();
  };

  uiInfo.uiDC.getColorSliderString = &ColorPicker::getColorSliderString;
  uiInfo.uiDC.setColorSliderType = &ColorPicker::setColorSliderType;
  uiInfo.uiDC.getColorSliderValue = &ColorPicker::getColorSliderValue;
  uiInfo.uiDC.setColorSliderValue = &ColorPicker::setColorSliderValue;
}

static void drawLevelshotPreview(rectDef_t &rect) {
  // unregistered levelshot is -1, not 0
  if (uiInfo.mapList[ui_mapIndex.integer].levelShot == -1) {
    uiInfo.mapList[ui_mapIndex.integer].levelShot = trap_R_RegisterShaderNoMip(
        va("levelshots/%s", uiInfo.mapList[ui_mapIndex.integer].mapLoadName));

    if (!uiInfo.mapList[ui_mapIndex.integer].levelShot) {
      uiInfo.mapList[ui_mapIndex.integer].levelShot =
          trap_R_RegisterShaderNoMip("levelshots/unknownmap");
    }
  }

  uiInfo.uiDC.drawHandlePic(rect.x, rect.y, rect.w, rect.h,
                            uiInfo.mapList[ui_mapIndex.integer].levelShot);
}

static void drawMapname(rectDef_t &rect, float scale, vec4_t color,
                        float text_x, int textStyle, int align) {
  rectDef_t textRect = {0, 0, rect.w, rect.h};

  const int map = ui_currentNetMap.integer;
  std::string mapname;

  if (uiInfo.mapList[map].mapLoadName != nullptr) {
    mapname = uiInfo.mapList[map].mapLoadName;
  } else {
    mapname = "unknownmap";
  }

  const auto width = static_cast<float>(Text_Width(mapname.c_str(), scale, 0));

  switch (align) {
    case ITEM_ALIGN_LEFT:
      textRect.x = text_x;
      break;
    case ITEM_ALIGN_RIGHT:
      textRect.x = text_x - width;
      break;
    case ITEM_ALIGN_CENTER:
      textRect.x = text_x - (width * 0.5f);
      break;
    default:
      break;
  }

  textRect.x += rect.x;
  textRect.y += rect.y;
  Text_Paint(textRect.x, textRect.y, scale, color, mapname.c_str(), 0, 0,
             textStyle);
}
} // namespace ETJump

/*
=================
_UI_Refresh
=================
*/

void UI_DrawCenteredPic(qhandle_t image, int w, int h) {
  int x, y;
  x = (SCREEN_WIDTH - w) / 2;
  y = (SCREEN_HEIGHT - h) / 2;
  UI_DrawHandlePic(x, y, w, h, image);
}

int frameCount = 0;
int startTime;

#define UI_FPS_FRAMES 4
void _UI_Refresh(int realtime) {
  static int index;
  static int previousTimes[UI_FPS_FRAMES];

  // if ( !( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
  //	return;
  // }

  uiInfo.uiDC.frameTime = realtime - uiInfo.uiDC.realTime;
  uiInfo.uiDC.realTime = realtime;

  previousTimes[index % UI_FPS_FRAMES] = uiInfo.uiDC.frameTime;
  index++;
  if (index > UI_FPS_FRAMES) {
    int i, total;
    // average multiple frames together to smooth changes out a
    // bit
    total = 0;
    for (i = 0; i < UI_FPS_FRAMES; i++) {
      total += previousTimes[i];
    }
    if (!total) {
      total = 1;
    }
    uiInfo.uiDC.FPS = 1000 * UI_FPS_FRAMES / total;
  }

  UI_UpdateCvars();

  if (trap_Cvar_VariableValue("ui_connecting")) {
    UI_DrawLoadPanel(qtrue, qfalse, qtrue);
    if (!trap_Cvar_VariableValue("ui_connecting")) {
      trap_Cvar_Set("ui_connecting", "1");
    }
    return;
  }

  // OSP - blackout if speclocked
  if (ui_blackout.integer > 0) {
    UI_FillRect(-10, -10, SCREEN_WIDTH + 10, 490, colorBlack);
  }

  if (Menu_Count() > 0) {
    // paint all the menus
    Menu_PaintAll();
    // refresh server browser list
    UI_DoServerRefresh();
    // refresh server status
    UI_BuildServerStatus(qfalse);
    // refresh find player list
    UI_BuildFindPlayerList(qfalse);
  }

  // draw cursor
  UI_SetColor(NULL);
  if (Menu_Count() > 0) {
    uiClientState_t cstate;
    trap_GetClientState(&cstate);
    if (cstate.connState <= CA_DISCONNECTED || cstate.connState >= CA_ACTIVE) {
      UI_DrawHandlePic(uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory, 32, 32,
                       uiInfo.uiDC.Assets.cursor);
    }
  }
}

/*
=================
_UI_Shutdown
=================
*/
void _UI_Shutdown(void) {
  trap_LAN_SaveCachedServers();

  ETJump::colorPicker = nullptr;

  Shutdown_Display();
}

char *defaultMenu = NULL;

char *GetMenuBuffer(const char *filename) {
  int len;
  fileHandle_t f;
  static char buf[MAX_MENUFILE];

  len = trap_FS_FOpenFile(filename, &f, FS_READ);
  if (!f) {
    trap_Print(
        va(S_COLOR_RED "menu file not found: %s, using default\n", filename));
    return defaultMenu;
  }
  if (len >= MAX_MENUFILE) {
    trap_Print(va(S_COLOR_RED
                  "menu file too large: %s is %i, max allowed is %i",
                  filename, len, MAX_MENUFILE));
    trap_FS_FCloseFile(f);
    return defaultMenu;
  }

  trap_FS_Read(buf, len, f);
  buf[len] = 0;
  trap_FS_FCloseFile(f);
  // COM_Compress(buf);
  return buf;
}

qboolean Asset_Parse(int handle) {
  pc_token_t token;
  const char *tempStr;

  if (!trap_PC_ReadToken(handle, &token)) {
    return qfalse;
  }
  if (Q_stricmp(token.string, "{") != 0) {
    return qfalse;
  }

  while (1) {

    memset(&token, 0, sizeof(pc_token_t));

    if (!trap_PC_ReadToken(handle, &token)) {
      return qfalse;
    }

    if (Q_stricmp(token.string, "}") == 0) {
      return qtrue;
    }

    // font
    if (Q_stricmp(token.string, "font") == 0) {
      int pointSize, fontIndex;
      if (!PC_Int_Parse(handle, &fontIndex) ||
          !PC_String_Parse(handle, &tempStr) ||
          !PC_Int_Parse(handle, &pointSize)) {
        return qfalse;
      }
      if (fontIndex < 0 || fontIndex >= 6) {
        return qfalse;
      }
      trap_R_RegisterFont(tempStr, pointSize,
                          &uiInfo.uiDC.Assets.fonts[fontIndex]);
      uiInfo.uiDC.Assets.fontRegistered = qtrue;
      continue;
    }

    // gradientbar
    if (Q_stricmp(token.string, "gradientbar") == 0) {
      if (!PC_String_Parse(handle, &tempStr)) {
        return qfalse;
      }
      uiInfo.uiDC.Assets.gradientBar = trap_R_RegisterShaderNoMip(tempStr);
      continue;
    }

    // enterMenuSound
    if (Q_stricmp(token.string, "menuEnterSound") == 0) {
      if (!PC_String_Parse(handle, &tempStr)) {
        return qfalse;
      }
      uiInfo.uiDC.Assets.menuEnterSound = trap_S_RegisterSound(tempStr, qtrue);
      continue;
    }

    // exitMenuSound
    if (Q_stricmp(token.string, "menuExitSound") == 0) {
      if (!PC_String_Parse(handle, &tempStr)) {
        return qfalse;
      }
      uiInfo.uiDC.Assets.menuExitSound = trap_S_RegisterSound(tempStr, qtrue);
      continue;
    }

    // itemFocusSound
    if (Q_stricmp(token.string, "itemFocusSound") == 0) {
      if (!PC_String_Parse(handle, &tempStr)) {
        return qfalse;
      }
      uiInfo.uiDC.Assets.itemFocusSound = trap_S_RegisterSound(tempStr, qtrue);
      continue;
    }

    // menuBuzzSound
    if (Q_stricmp(token.string, "menuBuzzSound") == 0) {
      if (!PC_String_Parse(handle, &tempStr)) {
        return qfalse;
      }
      uiInfo.uiDC.Assets.menuBuzzSound = trap_S_RegisterSound(tempStr, qtrue);
      continue;
    }

    if (Q_stricmp(token.string, "cursor") == 0) {
      if (!PC_String_Parse(handle, &uiInfo.uiDC.Assets.cursorStr)) {
        return qfalse;
      }
      uiInfo.uiDC.Assets.cursor =
          trap_R_RegisterShaderNoMip(uiInfo.uiDC.Assets.cursorStr);
      continue;
    }

    if (Q_stricmp(token.string, "fadeClamp") == 0) {
      if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.fadeClamp)) {
        return qfalse;
      }
      continue;
    }

    if (Q_stricmp(token.string, "fadeCycle") == 0) {
      if (!PC_Int_Parse(handle, &uiInfo.uiDC.Assets.fadeCycle)) {
        return qfalse;
      }
      continue;
    }

    if (Q_stricmp(token.string, "fadeAmount") == 0) {
      if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.fadeAmount)) {
        return qfalse;
      }
      continue;
    }

    if (Q_stricmp(token.string, "shadowX") == 0) {
      if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.shadowX)) {
        return qfalse;
      }
      continue;
    }

    if (Q_stricmp(token.string, "shadowY") == 0) {
      if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.shadowY)) {
        return qfalse;
      }
      continue;
    }

    if (Q_stricmp(token.string, "shadowColor") == 0) {
      if (!PC_Color_Parse(handle, &uiInfo.uiDC.Assets.shadowColor)) {
        return qfalse;
      }
      uiInfo.uiDC.Assets.shadowFadeClamp = uiInfo.uiDC.Assets.shadowColor[3];
      continue;
    }
  }
  return qfalse;
}

void UI_Report() { String_Report(); }

void QDECL Com_DPrintf(const char *fmt, ...);
qboolean UI_ParseMenu(const char *menuFile) {
  int handle;
  pc_token_t token;

  Com_DPrintf("Parsing menu file: %s\n", menuFile);

  handle = trap_PC_LoadSource(menuFile);
  if (!handle) {
    return qfalse;
  }

  while (1) {
    memset(&token, 0, sizeof(pc_token_t));
    if (!trap_PC_ReadToken(handle, &token)) {
      break;
    }

    // if ( Q_stricmp( token, "{" ) ) {
    //	Com_Printf( "Missing { in menu file\n" );
    //	break;
    // }

    // if ( menuCount == MAX_MENUS ) {
    //	Com_Printf( "Too many menus!\n" );
    //	break;
    // }

    if (token.string[0] == '}') {
      break;
    }

    if (Q_stricmp(token.string, "assetGlobalDef") == 0) {
      if (Asset_Parse(handle)) {
        continue;
      } else {
        break;
      }
    }

    if (Q_stricmp(token.string, "menudef") == 0) {
      // start a new menu
      Menu_New(handle);
    }
  }
  trap_PC_FreeSource(handle);
  return qtrue;
}

qboolean Load_Menu(int handle) {
  pc_token_t token;
#ifdef LOCALIZATION_SUPPORT
  int cl_language; // NERVE - SMF
#endif             // LOCALIZATION_SUPPORT

  if (!trap_PC_ReadToken(handle, &token)) {
    return qfalse;
  }
  if (token.string[0] != '{') {
    return qfalse;
  }

  while (1) {

    if (!trap_PC_ReadToken(handle, &token)) {
      return qfalse;
    }

    if (token.string[0] == 0) {
      return qfalse;
    }

    if (token.string[0] == '}') {
      return qtrue;
    }

#ifdef LOCALIZATION_SUPPORT
    // NERVE - SMF - localization crap
    cl_language = Q_atoi(UI_Cvar_VariableString("cl_language"));

    if (cl_language) {
      const char *s = NULL; // TTimo: init
      const char *filename;
      char out[256];
      //			char filename[256];

      COM_StripFilename(token.string, out);

      filename = COM_SkipPath(token.string);

      if (cl_language == 1) {
        s = va("%s%s", out, "french/");
      } else if (cl_language == 2) {
        s = va("%s%s", out, "german/");
      } else if (cl_language == 3) {
        s = va("%s%s", out, "italian/");
      } else if (cl_language == 4) {
        s = va("%s%s", out, "spanish/");
      }

      if (UI_ParseMenu(va("%s%s", s, filename))) {
        continue;
      }
    }
    // -NERVE
#endif // LOCALIZATION_SUPPORT

    UI_ParseMenu(token.string);
  }
  return qfalse;
}

void UI_LoadMenus(const char *menuFile, qboolean reset) {
  pc_token_t token;
  int handle;
  int start;
  uiClientState_t cstate;

  start = trap_Milliseconds();
  trap_GetClientState(&cstate);

  if (cstate.connState <= CA_DISCONNECTED) {
    trap_PC_AddGlobalDefine("FUI");
  }

  if (uiInfo.vetClient) {
    trap_PC_AddGlobalDefine("VET");
  }

  if (uiInfo.eteClient) {
    trap_PC_AddGlobalDefine("ETE");
  }

  if (uiInfo.etLegacyClient) {
    trap_PC_AddGlobalDefine("ETLEGACY");
  }

  handle = trap_PC_LoadSource(menuFile);

  if (!handle) {
    Com_Printf(va(S_COLOR_YELLOW
                  "%s: menu file '%s' not found, using default\n",
                  __func__, menuFile));
    handle = trap_PC_LoadSource(DEFAULT_MENU_FILE);

    if (!handle) {
      trap_Error(
          va(S_COLOR_RED
             "%s: default menu file '%s' not found, unable to continue!\n",
             __func__, DEFAULT_MENU_FILE));
    }
  }

  ui_new.integer = 1;

  if (reset) {
    Menu_Reset();
  }

  while (1) {
    if (!trap_PC_ReadToken(handle, &token)) {
      break;
    }
    if (token.string[0] == 0 || token.string[0] == '}') {
      break;
    }

    if (token.string[0] == '}') {
      break;
    }

    if (Q_stricmp(token.string, "loadmenu") == 0) {
      if (Load_Menu(handle)) {
        continue;
      } else {
        break;
      }
    }
  }

  uiInfo.serverMaplist.clear();
  uiInfo.customVotes.clear();

  // if we're already in-game, force a re-request for map list and customvotes
  // this only ever executes if we do 'ui/vid_restart' while in-game
  if (cstate.connState == CA_ACTIVE) {
    trap_Cmd_ExecuteText(EXEC_APPEND, "forceMaplistRefresh\n");
    trap_Cmd_ExecuteText(EXEC_APPEND, "forceCustomvoteRefresh\n");
  }

  Com_DPrintf("UI menu load time = %d milli seconds\n",
              trap_Milliseconds() - start);

  trap_PC_FreeSource(handle);
}

void UI_Load() {
  char lastName[1024];
  menuDef_t *menu = Menu_GetFocused();
  const char *menuSet = UI_Cvar_VariableString("ui_menuFiles");
  if (menu && menu->window.name) {
    Q_strncpyz(lastName, menu->window.name, sizeof(lastName));
  }
  if (menuSet == nullptr || menuSet[0] == '\0') {
    menuSet = DEFAULT_MENU_FILE;
  } else {
    lastName[0] = '\0';
  }

  String_Init();

  UI_ParseGameInfo("gameinfo.txt");
  UI_LoadArenas();

  UI_LoadMenus(menuSet, qtrue);
  Menus_CloseAll();
  Menus_ActivateByName(lastName, qtrue);
}

// ui_gameType assumes gametype 0 is -1 ALL and will not show
static void UI_DrawGameType(rectDef_t *rect, float scale, vec4_t color,
                            int textStyle) {
  Text_Paint(
      rect->x, rect->y, scale, color,
      uiInfo
          .gameTypes[Numeric::clamp(ui_gameType.integer, 0, MAX_GAMETYPES - 1)]
          .gameType,
      0, 0, textStyle);
}

static int UI_TeamIndexFromName(const char *name) {
  int i;

  if (name && *name) {
    for (i = 0; i < uiInfo.teamCount; i++) {
      if (Q_stricmp(name, uiInfo.teamList[i].teamName) == 0) {
        return i;
      }
    }
  }

  return 0;
}

static void UI_DrawTeamName(rectDef_t *rect, float scale, vec4_t color,
                            qboolean blue, int textStyle) {
  int i;
  i = UI_TeamIndexFromName(
      UI_Cvar_VariableString((blue) ? "ui_blueTeam" : "ui_redTeam"));
  if (i >= 0 && i < uiInfo.teamCount) {
    Text_Paint(
        rect->x, rect->y, scale, color,
        va("%s: %s", (blue) ? "Blue" : "Red", uiInfo.teamList[i].teamName), 0,
        0, textStyle);
  }
}

static void UI_DrawTeamMember(rectDef_t *rect, float scale, vec4_t color,
                              qboolean blue, int num, int textStyle) {}

static void UI_DrawEffects(rectDef_t *rect, float scale, vec4_t color) {
  UI_DrawHandlePic(rect->x, rect->y, 128, 8, uiInfo.uiDC.Assets.fxBasePic);
  UI_DrawHandlePic(rect->x + uiInfo.effectsColor * 16 + 8, rect->y, 16, 12,
                   uiInfo.uiDC.Assets.fxPic[uiInfo.effectsColor]);
}

void UI_DrawMapPreview(rectDef_t *rect, float scale, vec4_t color,
                       qboolean net) {
  int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;

  if (map < 0 || map > uiInfo.mapCount) {
    if (net) {
      ui_currentNetMap.integer = 0;
      trap_Cvar_Set("ui_currentNetMap", "0");
    } else {
      ui_currentMap.integer = 0;
      trap_Cvar_Set("ui_currentMap", "0");
    }
    map = 0;
  }

  if (uiInfo.mapList[map].mappos[0] && uiInfo.mapList[map].mappos[1]) {
    float x, y, w, h;
    vec2_t tl, br;
    vec4_t colourFadedBlack = {0.f, 0.f, 0.f, 0.4f};

    tl[0] = uiInfo.mapList[map].mappos[0] - .5 * 650.f;
    if (tl[0] < 0) {
      tl[0] = 0;
    }
    br[0] = tl[0] + 650.f;
    if (br[0] > 1024.f) {
      br[0] = 1024.f;
      tl[0] = br[0] - 650.f;
    }

    tl[1] = uiInfo.mapList[map].mappos[1] - .5 * 650.f;
    if (tl[1] < 0) {
      tl[1] = 0;
    }
    br[1] = tl[1] + 650.f;
    if (br[1] > 1024.f) {
      br[1] = 1024.f;
      tl[1] = br[1] - 650.f;
    }

    x = rect->x;
    y = rect->y;
    w = rect->w;
    h = rect->h;
    UI_AdjustFrom640(&x, &y, &w, &h);
    trap_R_DrawStretchPic(x, y, w, h, tl[0] / 1024.f, tl[1] / 1024.f,
                          br[0] / 1024.f, br[1] / 1024.f, uiInfo.campaignMap);

    x = rect->x + ((uiInfo.mapList[map].mappos[0] - tl[0]) / 650.f * rect->w);
    y = rect->y + ((uiInfo.mapList[map].mappos[1] - tl[1]) / 650.f * rect->h);

    w = Text_Width(uiInfo.mapList[map].mapName, scale, 0);

    if (x + 10 + w > rect->x + rect->w) {
      UI_FillRect(x - w - 12 + 1, y - 6 + 1, 12 + w, 12, colourFadedBlack);
      UI_FillRect(x - w - 12, y - 6, 12 + w, 12, colorBlack);
    } else {
      UI_FillRect(x + 1, y - 6 + 1, 12 + w, 12, colourFadedBlack);
      UI_FillRect(x, y - 6, 12 + w, 12, colorBlack);
    }

    UI_DrawHandlePic(x - 8, y - 8, 16, 16,
                     trap_R_RegisterShaderNoMip("gfx/loading/pin_neutral"));

    if (x + 10 + w > rect->x + rect->w) {
      Text_Paint(x - w - 10, y + 3, scale, colorWhite,
                 uiInfo.mapList[map].mapName, 0, 0, 0);
    } else {
      Text_Paint(x + 10, y + 3, scale, colorWhite, uiInfo.mapList[map].mapName,
                 0, 0, 0);
    }
  } else {
    UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h,
                     trap_R_RegisterShaderNoMip("levelshots/unknownmap"));
  }

  /*if (uiInfo.mapList[map].levelShot == -1) {
      uiInfo.mapList[map].levelShot =
  trap_R_RegisterShaderNoMip(uiInfo.mapList[map].imageName);
  }

  if (uiInfo.mapList[map].levelShot > 0) {
      UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h,
  uiInfo.mapList[map].levelShot); } else { UI_DrawHandlePic( rect->x,
  rect->y, rect->w, rect->h,
  trap_R_RegisterShaderNoMip("levelshots/unknownmap"));
  }*/
}

void UI_DrawNetMapPreview(rectDef_t *rect, float scale, vec4_t color,
                          qboolean net) {
  if (uiInfo.serverStatus.currentServerPreview > 0) {
    UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h,
                     uiInfo.serverStatus.currentServerPreview);
  } else {
    UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h,
                     trap_R_RegisterShaderNoMip("levelshots/unknownmap"));
  }
}

static void UI_DrawMapCinematic(rectDef_t *rect, float scale, vec4_t color,
                                qboolean net) {

  int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;

  if (map < 0 || map > uiInfo.mapCount) {
    if (net) {
      ui_currentNetMap.integer = 0;
      trap_Cvar_Set("ui_currentNetMap", "0");
    } else {
      ui_currentMap.integer = 0;
      trap_Cvar_Set("ui_currentMap", "0");
    }
    map = 0;
  }

  if (uiInfo.mapList[map].cinematic >= -1) {
    if (uiInfo.mapList[map].cinematic == -1) {
      uiInfo.mapList[map].cinematic =
          trap_CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[map].mapLoadName),
                                 0, 0, 0, 0, (CIN_loop | CIN_silent));
    }
    if (uiInfo.mapList[map].cinematic >= 0) {
      trap_CIN_RunCinematic(uiInfo.mapList[map].cinematic);
      trap_CIN_SetExtents(uiInfo.mapList[map].cinematic, rect->x, rect->y,
                          rect->w, rect->h);
      trap_CIN_DrawCinematic(uiInfo.mapList[map].cinematic);
    } else {
      uiInfo.mapList[map].cinematic = -2;
    }
  } else {
    UI_DrawMapPreview(rect, scale, color, net);
  }
}

void UI_DrawGametypeDescription(rectDef_t *rect, float scale, vec4_t color,
                                float text_x, float text_y, int textStyle,
                                int align, qboolean net) {
  const char *p, *textPtr = nullptr, *newLinePtr = nullptr;
  char buff[1024];
  int height, len, textWidth, newLine, newLineWidth, i;
  float y;
  rectDef_t textRect;

  int game = ui_netGameType.integer;

  for (i = 0; i < uiInfo.numGameTypes; i++) {
    if (uiInfo.gameTypes[i].gtEnum == game) {
      textPtr = uiInfo.gameTypes[i].gameTypeDescription;
      break;
    }
  }

  if (i == uiInfo.numGameTypes) {
    textPtr = "Unknown";
  }

  height = Text_Height(textPtr, scale, 0);

  textRect.x = 0;
  textRect.y = 0;
  textRect.w = rect->w;
  textRect.h = rect->h;

  // y = text_y;
  y = 0;
  len = 0;
  buff[0] = '\0';
  newLine = 0;
  newLineWidth = 0;
  p = textPtr;

  while (p) {
    textWidth = DC->textWidth(buff, scale, 0);
    if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\0') {
      newLine = len;
      newLinePtr = p + 1;
      newLineWidth = textWidth;
    } else if (*p == '*' && *(p + 1) == '*') {
      newLine = len;
      newLinePtr = p + 2;
      newLineWidth = textWidth;
    }
    if ((newLine && textWidth > rect->w) || *p == '\n' || *p == '\0' ||
        (*p == '*' && *(p + 1) == '*')) {
      if (len) {
        if (align == ITEM_ALIGN_LEFT) {
          textRect.x = text_x;
        } else if (align == ITEM_ALIGN_RIGHT) {
          textRect.x = text_x - newLineWidth;
        } else if (align == ITEM_ALIGN_CENTER) {
          textRect.x = text_x - newLineWidth / 2;
        }
        textRect.y = y;

        textRect.x += rect->x;
        textRect.y += rect->y;
        //
        buff[newLine] = '\0';
        DC->drawText(textRect.x, textRect.y, scale, color, buff, 0, 0,
                     textStyle);
      }
      if (*p == '\0') {
        break;
      }
      //
      y += height + AUTOWRAP_OFFSET;
      p = newLinePtr;
      len = 0;
      newLine = 0;
      newLineWidth = 0;
      continue;
    }
    buff[len++] = *p++;

    if (buff[len - 1] == 13) {
      buff[len - 1] = ' ';
    }

    buff[len] = '\0';
  }
}

void UI_DrawMapDescription(rectDef_t *rect, float scale, const vec4_t color,
                           float text_x, int textStyle, int align) {
  fontInfo_t *font = &uiInfo.uiDC.Assets.fonts[uiInfo.activeFont];
  rectDef_t textRect = {0, 0, rect->w, rect->h};

  const int map = ui_currentNetMap.integer;
  std::string briefing;

  if (uiInfo.mapList[map].briefing != nullptr) {
    briefing = uiInfo.mapList[map].briefing;

    if (!briefing.empty()) {
      // replace any carriage returns with whitespace
      ETJump::StringUtil::replaceAll(briefing, "\r", " ");

      // replace special '*' char with a regular linebreak char
      ETJump::StringUtil::replaceAll(briefing, "*", "\n");
    }
  } else {
    briefing = "^1No text supplied";
  }

  BG_FitTextToWidth_Ext(briefing, scale, textRect.w, font);
  std::vector<std::string> lines = ETJump::StringUtil::split(briefing, "\n");

  // remove any potential empty strings from the back of the vector
  while (!lines.empty() && lines.back().empty()) {
    lines.pop_back();
  }

  // nothing left to draw, exit
  if (lines.empty()) {
    return;
  }

  static textScroll_t ts;

  if (ts.scrollStartTime == 0 ||
      (!ts.scrolling && DC->realTime > ts.scrollEndTime) ||
      ts.scrollItem != ui_currentNetMap.integer) {
    ts.scrollStartTime = DC->realTime + 2000;
    ts.scrolling = true;
    ts.y = 0;
    ts.scrollItem = ui_currentNetMap.integer;
  }

  static constexpr int lineHeight = 10;
  const float totalHeight = static_cast<float>(lines.size()) * lineHeight;
  float y = 0.0f + ts.y;

  ts.scrollDeltaTime = static_cast<float>(DC->frameTime) * 0.001f;

  if (totalHeight > rect->h && ts.scrolling &&
      DC->realTime > ts.scrollStartTime) {
    static constexpr float scrollSpeed = 15.0f; // pixels per second
    ts.y -= scrollSpeed * ts.scrollDeltaTime;
  }

  vec4_t textColor;

  for (const auto &line : lines) {
    const auto width = static_cast<float>(Text_Width(line.c_str(), scale, 0));

    switch (align) {
      case ITEM_ALIGN_LEFT:
        textRect.x = text_x;
        break;
      case ITEM_ALIGN_RIGHT:
        textRect.x = text_x - width;
        break;
      case ITEM_ALIGN_CENTER:
        textRect.x = text_x - (width * 0.5f);
        break;
      default:
        break;
    }

    textRect.y = y;
    textRect.x += rect->x;
    textRect.y += rect->y;

    // fade out if the line is going out of rect bounds
    if (textRect.y < rect->y || textRect.y + lineHeight > rect->y + rect->h) {
      float fadeFactor = 1.0f;

      if (textRect.y < rect->y) {
        // text is above the rect
        float oobDist = rect->y - textRect.y;
        fadeFactor = 1.0f - (oobDist / (lineHeight * 0.5f));
      } else if (textRect.y + lineHeight > rect->y + rect->h) {
        // text is below the rect
        float oobDist = (textRect.y + lineHeight) - (rect->y + rect->h);
        fadeFactor = 1.0f - (oobDist / (lineHeight * 0.5f));
      }

      Vector4Copy(color, textColor);
      textColor[3] *= Numeric::clamp(fadeFactor, 0.0f, 1.0f);
    } else {
      Vector4Copy(color, textColor);
    }

    Text_Paint(textRect.x, textRect.y, scale, textColor, line.c_str(), 0, 0,
               textStyle);
    y += lineHeight;
  }

  if (ts.scrolling && ts.y + totalHeight < textRect.h) {
    ts.scrollEndTime = DC->realTime + 2000;
    ts.scrolling = false;
  }
}

static void UI_DrawMissionBriefingMap(rectDef_t *rect) {
  char buffer[64];
  static qhandle_t image = -1;

  if (image == -1) {
    trap_Cvar_VariableStringBuffer("mapname", buffer, 64);
    image = trap_R_RegisterShaderNoMip(va("levelshots/%s_cc.tga", buffer));
  }

  if (image) {
    UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, image);
  } else {
    UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h,
                     trap_R_RegisterShaderNoMip("levelshots/unknownmap"));
  }
}

static void UI_DrawMissionBriefingTitle(rectDef_t *rect, float scale,
                                        vec4_t color, int textStyle) {
  char buffer[64];
  mapInfo *mi;

  trap_Cvar_VariableStringBuffer("mapname", buffer, 64);

  mi = UI_FindMapInfoByMapname(buffer);
  if (!mi) {
    return;
  }

  Text_Paint(rect->x, rect->y, scale, color, va("%s Objectives", mi->mapName),
             0, 0, textStyle);
}

static void UI_DrawMissionBriefingObjectives(rectDef_t *rect, float scale,
                                             vec4_t color, float text_x,
                                             float text_y, int textStyle,
                                             int align) {
  const char *p, *textPtr, *newLinePtr;
  char buff[1024];
  int height, len, textWidth, newLine, newLineWidth;
  float y;
  rectDef_t textRect;

  char buffer[64];
  mapInfo *mi;

  trap_Cvar_VariableStringBuffer("mapname", buffer, 64);

  mi = UI_FindMapInfoByMapname(buffer);
  if (!mi) {
    return;
  }

  textPtr = mi->objectives;

  height = Text_Height(textPtr, scale, 0);

  textRect.x = 0;
  textRect.y = 0;
  textRect.w = rect->w;
  textRect.h = rect->h;

  textWidth = 0;
  newLinePtr = NULL;
  y = text_y;
  len = 0;
  buff[0] = '\0';
  newLine = 0;
  newLineWidth = 0;
  p = textPtr;
  while (p) {
    if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\0') {
      newLine = len;
      newLinePtr = p + 1;
      newLineWidth = textWidth;
    }
    textWidth = Text_Width(buff, scale, 0);
    if ((newLine && textWidth > rect->w) || *p == '\n' || *p == '\0') {
      if (len) {
        if (align == ITEM_ALIGN_LEFT) {
          textRect.x = text_x;
        } else if (align == ITEM_ALIGN_RIGHT) {
          textRect.x = text_x - newLineWidth;
        } else if (align == ITEM_ALIGN_CENTER) {
          textRect.x = text_x - newLineWidth / 2;
        }
        textRect.y = y;

        textRect.x += rect->x;
        textRect.y += rect->y;

        //
        buff[newLine] = '\0';
        Text_Paint(textRect.x, textRect.y, scale, color, buff, 0, 0, textStyle);
      }
      if (*p == '\0') {
        break;
      }
      //
      y += height + AUTOWRAP_OFFSET;
      p = newLinePtr;
      len = 0;
      newLine = 0;
      newLineWidth = 0;
      continue;
    }
    buff[len++] = *p++;

    if (buff[len - 1] == 13) {
      buff[len - 1] = ' ';
    }

    buff[len] = '\0';
  }
}

static void UI_DrawNetFilter(rectDef_t *rect, float scale, vec4_t color,
                             int textStyle) {
  if (ui_serverFilterType < 0 || ui_serverFilterType >= numServerFilters) {
    ui_serverFilterType = 0;
  }
  Text_Paint(rect->x, rect->y, scale, color,
             va("Filter: %s", serverFilters[ui_serverFilterType].description),
             0, 0, textStyle);
}

static void UI_NextOpponent() {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
  int j = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
  i++;
  if (i >= uiInfo.teamCount) {
    i = 0;
  }
  if (i == j) {
    i++;
    if (i >= uiInfo.teamCount) {
      i = 0;
    }
  }
  trap_Cvar_Set("ui_opponentName", uiInfo.teamList[i].teamName);
}

static void UI_PriorOpponent() {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
  int j = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
  i--;
  if (i < 0) {
    i = uiInfo.teamCount - 1;
  }
  if (i == j) {
    i--;
    if (i < 0) {
      i = uiInfo.teamCount - 1;
    }
  }
  trap_Cvar_Set("ui_opponentName", uiInfo.teamList[i].teamName);
}

static void UI_DrawPlayerLogo(rectDef_t *rect, vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));

  if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon =
        trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(
        va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name =
        trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h,
                   uiInfo.teamList[i].teamIcon);
  trap_R_SetColor(NULL);
}

static void UI_DrawPlayerLogoMetal(rectDef_t *rect, vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
  if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon =
        trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(
        va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name =
        trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h,
                   uiInfo.teamList[i].teamIcon_Metal);
  trap_R_SetColor(NULL);
}

static void UI_DrawPlayerLogoName(rectDef_t *rect, vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
  if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon =
        trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(
        va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name =
        trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h,
                   uiInfo.teamList[i].teamIcon_Name);
  trap_R_SetColor(NULL);
}

static void UI_DrawOpponentLogo(rectDef_t *rect, vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
  if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon =
        trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(
        va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name =
        trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h,
                   uiInfo.teamList[i].teamIcon);
  trap_R_SetColor(NULL);
}

static void UI_DrawOpponentLogoMetal(rectDef_t *rect, vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
  if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon =
        trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(
        va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name =
        trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h,
                   uiInfo.teamList[i].teamIcon_Metal);
  trap_R_SetColor(NULL);
}

static void UI_DrawOpponentLogoName(rectDef_t *rect, vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
  if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon =
        trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(
        va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name =
        trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h,
                   uiInfo.teamList[i].teamIcon_Name);
  trap_R_SetColor(NULL);
}

static void UI_DrawAllMapsSelection(rectDef_t *rect, float scale, vec4_t color,
                                    int textStyle, qboolean net) {}

static void UI_DrawOpponentName(rectDef_t *rect, float scale, vec4_t color,
                                int textStyle) {
  Text_Paint(rect->x, rect->y, scale, color,
             UI_Cvar_VariableString("ui_opponentName"), 0, 0, textStyle);
}

static int UI_OwnerDrawWidth(int ownerDraw, float scale) {
  const char *s = nullptr;

  switch (ownerDraw) {
    case UI_GAMETYPE:
      s = uiInfo
              .gameTypes[Numeric::clamp(ui_gameType.integer, 0,
                                        MAX_GAMETYPES - 1)]
              .gameType;
      break;
    case UI_NETFILTER:
      if (ui_serverFilterType < 0 || ui_serverFilterType >= numServerFilters) {
        ui_serverFilterType = 0;
      }
      s = va("Filter: %s", serverFilters[ui_serverFilterType].description);
      break;
    case UI_KEYBINDSTATUS:
      if (Display_KeyBindPending()) {
        s = trap_TranslateString("Waiting for new key..."
                                 "Press ESCAPE to cancel");
      } else {
        s = trap_TranslateString("Press ENTER or CLICK to change, "
                                 "Press BACKSPACE to clear");
      }
      break;
    case UI_SERVERREFRESHDATE:
      s = UI_Cvar_VariableString(
          va("ui_lastServerRefresh_%i", ui_netSource.integer));
      break;
    default:
      break;
  }

  if (s) {
    return Text_Width(s, scale, 0);
  }
  return 0;
}

static void UI_DrawBotName(rectDef_t *rect, float scale, vec4_t color,
                           int textStyle) {}

/*static void UI_DrawBotSkill(rectDef_t *rect, float scale, vec4_t color, int
textStyle) { if (uiInfo.skillIndex >= 0 && uiInfo.skillIndex < numSkillLevels) {
      Text_Paint(rect->x, rect->y, scale, color, skillLevels[uiInfo.skillIndex],
0, 0, textStyle);
    }
}*/

static void UI_DrawRedBlue(rectDef_t *rect, float scale, vec4_t color,
                           int textStyle) {
  Text_Paint(rect->x, rect->y, scale, color,
             (uiInfo.redBlue == 0) ? "Red" : "Blue", 0, 0, textStyle);
}

static void UI_DrawCrosshair(rectDef_t *rect, float scale, vec4_t color) {
  float size = cg_crosshairSize.value;

  // Make sure currentCrosshair is updated if crosshair is changed via
  // console
  uiInfo.currentCrosshair = ui_drawCrosshair.integer;

  if (uiInfo.currentCrosshair < 0 ||
      uiInfo.currentCrosshair >= NUM_CROSSHAIRS) {
    uiInfo.currentCrosshair = 0;
  }

  size = (rect->w / 96.0f) *
         ((size > 96.0f) ? 96.0f : ((size < 24.0f) ? 24.0f : size));

  vec4_t crosshairColor = {1.0, 1.0, 1.0, 1.0};

  if (uiInfo.currentCrosshair < 10) {
    ETJump::parseColorString(cg_crosshairColor.string, crosshairColor);
    crosshairColor[3] = Numeric::clamp(cg_crosshairAlpha.value, 0.0f, 1.0f);
    trap_R_SetColor(crosshairColor);
    UI_DrawHandlePic(
        rect->x + (rect->w - size) / 2, rect->y + (rect->h - size) / 2, size,
        size, uiInfo.uiDC.Assets.crosshairShader[uiInfo.currentCrosshair]);

    vec4_t crosshairColorAlt = {1.0, 1.0, 1.0, 1.0};

    ETJump::parseColorString(cg_crosshairColorAlt.string, crosshairColorAlt);
    crosshairColorAlt[3] =
        Numeric::clamp(cg_crosshairAlphaAlt.value, 0.0f, 1.0f);
    trap_R_SetColor(crosshairColorAlt);
    UI_DrawHandlePic(
        rect->x + (rect->w - size) / 2, rect->y + (rect->h - size) / 2, size,
        size, uiInfo.uiDC.Assets.crosshairAltShader[uiInfo.currentCrosshair]);
  } else {
    const auto text = va("Crosshair %d", uiInfo.currentCrosshair);
    const auto width = static_cast<float>(
        Text_Width_Ext(text, 0.12f, 0, &uiInfo.loadscreenfont1));
    const auto height = static_cast<float>(
        Text_Height_Ext(text, 0.12f, 0, &uiInfo.loadscreenfont1));
    Text_Paint_Ext(rect->x + (rect->w - width) * 0.5f,
                   rect->y + (rect->h + height) * 0.5f, 0.12f, 0.12f,
                   colorWhite, text, 0, 0, ITEM_TEXTSTYLE_SHADOWED,
                   &uiInfo.loadscreenfont1);
  }

  trap_R_SetColor(nullptr);
}

/*
===============
UI_BuildPlayerList
===============
*/
static void UI_BuildPlayerList() {
  uiClientState_t cs;
  int n, count, team, team2, playerTeamNumber, muted;
  char info[MAX_INFO_STRING];
  char namebuf[64];

  trap_GetClientState(&cs);
  trap_GetConfigString(CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING);
  uiInfo.playerNumber = cs.clientNum;
  uiInfo.teamLeader = Q_atoi(Info_ValueForKey(info, "tl")) ? qtrue : qfalse;
  team = Q_atoi(Info_ValueForKey(info, "t"));
  trap_GetConfigString(CS_SERVERINFO, info, sizeof(info));
  count = Q_atoi(Info_ValueForKey(info, "sv_maxclients"));
  uiInfo.playerCount = 0;
  uiInfo.myTeamCount = 0;
  playerTeamNumber = 0;
  for (n = 0; n < count; n++) {
    trap_GetConfigString(CS_PLAYERS + n, info, MAX_INFO_STRING);

    if (info[0]) {
      Q_strncpyz(namebuf, Info_ValueForKey(info, "n"), sizeof(namebuf));
      // fretn - dont expand colors twice, so: foo^^xbar
      // -> foo^bar -> fooar
      //			Q_CleanStr( namebuf );
      Q_strncpyz(uiInfo.playerNames[uiInfo.playerCount], namebuf,
                 sizeof(uiInfo.playerNames[0]));
      muted = Q_atoi(Info_ValueForKey(info, "mu"));
      if (muted) {
        uiInfo.playerMuted[uiInfo.playerCount] = qtrue;
      } else {
        uiInfo.playerMuted[uiInfo.playerCount] = qfalse;
      }
      uiInfo.playerCount++;
      team2 = Q_atoi(Info_ValueForKey(info, "t"));
      if (team2 == team) {
        Q_strncpyz(namebuf, Info_ValueForKey(info, "n"), sizeof(namebuf));
        // fretn - dont expand colors twice, so:
        // foo^^xbar -> foo^bar -> fooar
        //				Q_CleanStr(
        // namebuf );
        Q_strncpyz(uiInfo.teamNames[uiInfo.myTeamCount], namebuf,
                   sizeof(uiInfo.teamNames[0]));
        uiInfo.teamClientNums[uiInfo.myTeamCount] = n;
        if (uiInfo.playerNumber == n) {
          playerTeamNumber = uiInfo.myTeamCount;
        }
        uiInfo.myTeamCount++;
      }
    }
  }

  if (!uiInfo.teamLeader) {
    trap_Cvar_Set("cg_selectedPlayer", va("%d", playerTeamNumber));
  }

  n = trap_Cvar_VariableValue("cg_selectedPlayer");
  if (n < 0 || n > uiInfo.myTeamCount) {
    n = 0;
  }
  if (n < uiInfo.myTeamCount) {
    trap_Cvar_Set("cg_selectedPlayerName", uiInfo.teamNames[n]);
  }
}

static void UI_DrawSelectedPlayer(rectDef_t *rect, float scale, vec4_t color,
                                  int textStyle) {
  if (uiInfo.uiDC.realTime > uiInfo.playerRefresh) {
    uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
    UI_BuildPlayerList();
  }
  Text_Paint(rect->x, rect->y, scale, color,
             (uiInfo.teamLeader)
                 ? UI_Cvar_VariableString("cg_selectedPlayerName")
                 : UI_Cvar_VariableString("name"),
             0, 0, textStyle);
}

static void UI_DrawServerRefreshDate(rectDef_t *rect, float scale, vec4_t color,
                                     int textStyle) {
  int serverCount; // NERVE - SMF
  if (uiInfo.serverStatus.refreshActive) {
    vec4_t lowLight, newColor;
    lowLight[0] = 0.8 * color[0];
    lowLight[1] = 0.8 * color[1];
    lowLight[2] = 0.8 * color[2];
    lowLight[3] = 0.8 * color[3];
    LerpColor(color, lowLight, newColor,
              0.5 + 0.5 * sin(uiInfo.uiDC.realTime / PULSE_DIVISOR));
    // NERVE - SMF
    serverCount = trap_LAN_GetServerCount(ui_netSource.integer);
    if (serverCount >= 0) {
      Text_Paint(rect->x, rect->y, scale, newColor,
                 va(trap_TranslateString("Getting info for %d servers "
                                         "(ESC to cancel)"),
                    serverCount),
                 0, 0, textStyle);
    } else {
      Text_Paint(rect->x, rect->y, scale, newColor,
                 trap_TranslateString("Waiting for response "
                                      "from Master Server"),
                 0, 0, textStyle);
    }
  } else {
    char buff[64];
    Q_strncpyz(buff,
               UI_Cvar_VariableString(
                   va("ui_lastServerRefresh_%i", ui_netSource.integer)),
               64);
    Text_Paint(rect->x, rect->y, scale, color,
               va(trap_TranslateString("Refresh Time: %s"), buff), 0, 0,
               textStyle);
  }
}

static void UI_DrawServerMOTD(rectDef_t *rect, float scale, vec4_t color) {
  if (uiInfo.serverStatus.motdLen) {
    float maxX;

    if (uiInfo.serverStatus.motdWidth == -1) {
      uiInfo.serverStatus.motdWidth = 0;
      uiInfo.serverStatus.motdPaintX = rect->x + 1;
      uiInfo.serverStatus.motdPaintX2 = -1;
    }

    if (uiInfo.serverStatus.motdOffset > uiInfo.serverStatus.motdLen) {
      uiInfo.serverStatus.motdOffset = 0;
      uiInfo.serverStatus.motdPaintX = rect->x + 1;
      uiInfo.serverStatus.motdPaintX2 = -1;
    }

    if (uiInfo.uiDC.realTime > uiInfo.serverStatus.motdTime) {
      uiInfo.serverStatus.motdTime = uiInfo.uiDC.realTime + 10;
      if (uiInfo.serverStatus.motdPaintX <= rect->x + 2) {
        if (uiInfo.serverStatus.motdOffset < uiInfo.serverStatus.motdLen) {
          uiInfo.serverStatus.motdPaintX +=
              Text_Width(
                  &uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset],
                  scale, 1) -
              1;
          uiInfo.serverStatus.motdOffset++;
        } else {
          uiInfo.serverStatus.motdOffset = 0;
          if (uiInfo.serverStatus.motdPaintX2 >= 0) {
            uiInfo.serverStatus.motdPaintX = uiInfo.serverStatus.motdPaintX2;
          } else {
            uiInfo.serverStatus.motdPaintX = rect->x + rect->w - 2;
          }
          uiInfo.serverStatus.motdPaintX2 = -1;
        }
      } else {
        // serverStatus.motdPaintX--;
        uiInfo.serverStatus.motdPaintX -= 2;
        if (uiInfo.serverStatus.motdPaintX2 >= 0) {
          // serverStatus.motdPaintX2--;
          uiInfo.serverStatus.motdPaintX2 -= 2;
        }
      }
    }

    maxX = rect->x + rect->w - 2;
    Text_Paint_Limit(
        &maxX, uiInfo.serverStatus.motdPaintX, rect->y, scale, color,
        &uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset], 0, 0);
    if (uiInfo.serverStatus.motdPaintX2 >= 0) {
      float maxX2 = rect->x + rect->w - 2;
      Text_Paint_Limit(&maxX2, uiInfo.serverStatus.motdPaintX2, rect->y, scale,
                       color, uiInfo.serverStatus.motd, 0,
                       uiInfo.serverStatus.motdOffset);
    }
    if (uiInfo.serverStatus.motdOffset && maxX > 0) {
      // if we have an offset ( we are skipping the
      // first part of the string ) and we fit the
      // string
      if (uiInfo.serverStatus.motdPaintX2 == -1) {
        uiInfo.serverStatus.motdPaintX2 = rect->x + rect->w - 2;
      }
    } else {
      uiInfo.serverStatus.motdPaintX2 = -1;
    }
  }
}

static void UI_DrawKeyBindStatus(rectDef_t *rect, float scale, vec4_t color,
                                 int textStyle, float text_x, float text_y) {
  // int ofs = 0; // TTimo: unused
  if (Display_KeyBindPending()) {
    Text_Paint(
        rect->x + text_x, rect->y + text_y, scale, color,
        trap_TranslateString("Waiting for new key... Press ESCAPE to cancel"),
        0, 0, textStyle);
  } else {
    Text_Paint(rect->x + text_x, rect->y + text_y, scale, color,
               trap_TranslateString("Press ENTER or CLICK to change, "
                                    "Press BACKSPACE to clear"),
               0, 0, textStyle);
  }
}

static void UI_ParseGLConfig(void) {
  char *eptr;

  uiInfo.numGlInfoLines = 0;

  eptr = uiInfo.uiDC.glconfig.extensions_string;

  while (*eptr) {
    while (*eptr && *eptr == ' ') *eptr++ = '\0';

    // track start of valid string
    if (*eptr && *eptr != ' ') {
      uiInfo.glInfoLines[uiInfo.numGlInfoLines++] = eptr;
    }

    if (uiInfo.numGlInfoLines == GLINFO_LINES) {
      break; // Arnout: failsafe
    }

    while (*eptr && *eptr != ' ') eptr++;
  }

  uiInfo.numGlInfoLines += 4; // vendor, version and pixelformat + a whiteline
}

// FIXME: table drive
//
static void UI_OwnerDraw(float x, float y, float w, float h, float text_x,
                         float text_y, int ownerDraw, int ownerDrawFlags,
                         int align, float special, float scale, vec4_t color,
                         qhandle_t shader, int textStyle) {
  rectDef_t rect;

  rect.x = x + text_x;
  rect.y = y + text_y;
  rect.w = w;
  rect.h = h;

  switch (ownerDraw) {
    case UI_EFFECTS:
      UI_DrawEffects(&rect, scale, color);
      break;
    case UI_GAMETYPE:
      UI_DrawGameType(&rect, scale, color, textStyle);
      break;
    case UI_MAPPREVIEW:
      UI_DrawMapPreview(&rect, scale, color, qtrue);
      break;
    case UI_NETMAPPREVIEW:
      UI_DrawNetMapPreview(&rect, scale, color, qtrue);
      break;
    case UI_MAPCINEMATIC:
      UI_DrawMapCinematic(&rect, scale, color, qfalse);
      break;
    case UI_STARTMAPCINEMATIC:
      UI_DrawMapCinematic(&rect, scale, color, qtrue);
      break;
    case UI_MAPDESCRIPTION:
      UI_DrawMapDescription(&rect, scale, color, text_x, textStyle, align);
      break;
    case UI_GAMETYPEDESCRIPTION:
      UI_DrawGametypeDescription(&rect, scale, color, text_x, text_y, textStyle,
                                 align, qtrue);
      break;
    case UI_MB_MAP:
      UI_DrawMissionBriefingMap(&rect);
      break;
    case UI_MB_TITLE:
      UI_DrawMissionBriefingTitle(&rect, scale, color, textStyle);
      break;
    case UI_MB_OBJECTIVES:
      UI_DrawMissionBriefingObjectives(&rect, scale, color, text_x, text_y,
                                       textStyle, align);
      break;
    case UI_BLUETEAMNAME:
      UI_DrawTeamName(&rect, scale, color, qtrue, textStyle);
      break;
    case UI_REDTEAMNAME:
      UI_DrawTeamName(&rect, scale, color, qfalse, textStyle);
      break;
    case UI_BLUETEAM1:
    case UI_BLUETEAM2:
    case UI_BLUETEAM3:
    case UI_BLUETEAM4:
    case UI_BLUETEAM5:
      UI_DrawTeamMember(&rect, scale, color, qtrue,
                        ownerDraw - UI_BLUETEAM1 + 1, textStyle);
      break;
    case UI_REDTEAM1:
    case UI_REDTEAM2:
    case UI_REDTEAM3:
    case UI_REDTEAM4:
    case UI_REDTEAM5:
      UI_DrawTeamMember(&rect, scale, color, qfalse,
                        ownerDraw - UI_REDTEAM1 + 1, textStyle);
      break;
    case UI_NETFILTER:
      UI_DrawNetFilter(&rect, scale, color, textStyle);
      break;
    case UI_PLAYERLOGO:
      UI_DrawPlayerLogo(&rect, color);
      break;
    case UI_PLAYERLOGO_METAL:
      UI_DrawPlayerLogoMetal(&rect, color);
      break;
    case UI_PLAYERLOGO_NAME:
      UI_DrawPlayerLogoName(&rect, color);
      break;
    case UI_OPPONENTLOGO:
      UI_DrawOpponentLogo(&rect, color);
      break;
    case UI_OPPONENTLOGO_METAL:
      UI_DrawOpponentLogoMetal(&rect, color);
      break;
    case UI_OPPONENTLOGO_NAME:
      UI_DrawOpponentLogoName(&rect, color);
      break;
    case UI_ALLMAPS_SELECTION:
      UI_DrawAllMapsSelection(&rect, scale, color, textStyle, qtrue);
      break;
    case UI_MAPS_SELECTION:
      UI_DrawAllMapsSelection(&rect, scale, color, textStyle, qfalse);
      break;
    case UI_OPPONENT_NAME:
      UI_DrawOpponentName(&rect, scale, color, textStyle);
      break;
    case UI_BOTNAME:
      UI_DrawBotName(&rect, scale, color, textStyle);
      break;
    case UI_REDBLUE:
      UI_DrawRedBlue(&rect, scale, color, textStyle);
      break;
    case UI_CROSSHAIR:
      UI_DrawCrosshair(&rect, scale, color);
      break;
    case UI_SELECTEDPLAYER:
      UI_DrawSelectedPlayer(&rect, scale, color, textStyle);
      break;
    case UI_SERVERREFRESHDATE:
      UI_DrawServerRefreshDate(&rect, scale, color, textStyle);
      break;
    case UI_SERVERMOTD:
      UI_DrawServerMOTD(&rect, scale, color);
      break;
    case UI_KEYBINDSTATUS:
      UI_DrawKeyBindStatus(&rect, scale, color, textStyle, text_x, text_y);
      break;
    case UI_LOADPANEL:
      UI_DrawLoadPanel(qfalse, qtrue, qfalse);
      break;
    case UI_COLOR_PICKER:
      ETJump::ColorPicker::shrinkRectForColorPicker(rect);
      ETJump::colorPicker->drawColorPicker(&rect);
      break;
    case UI_COLOR_PICKER_PREVIEW_OLD:
      ETJump::ColorPicker::shrinkRectForColorPicker(rect);
      ETJump::colorPicker->drawPreviewOld(&rect);
      break;
    case UI_COLOR_PICKER_PREVIEW_NEW:
      ETJump::ColorPicker::shrinkRectForColorPicker(rect);
      ETJump::colorPicker->drawPreviewNew(&rect);
      break;
    case UI_MAPNAME:
      ETJump::drawMapname(rect, scale, color, text_x, textStyle, align);
      break;
    case UI_LEVELSHOT_PREVIEW:
      ETJump::drawLevelshotPreview(rect);
      break;
    default:
      break;
  }
}

qboolean UI_OwnerDrawVisible(int flags) {
  qboolean vis = qtrue;

  while (flags) {

    if (flags & UI_SHOW_FFA) {
      flags &= ~UI_SHOW_FFA;
    }

    if (flags & UI_SHOW_NOTFFA) {
      vis = qfalse;
      flags &= ~UI_SHOW_NOTFFA;
    }

    if (flags & UI_SHOW_LEADER) {
      // these need to show when this client can give
      // orders to a player or a group
      if (!uiInfo.teamLeader) {
        vis = qfalse;
      } else {
        // if showing yourself
        if (ui_selectedPlayer.integer < uiInfo.myTeamCount &&
            uiInfo.teamClientNums[ui_selectedPlayer.integer] ==
                uiInfo.playerNumber) {
          vis = qfalse;
        }
      }
      flags &= ~UI_SHOW_LEADER;
    }
    if (flags & UI_SHOW_NOTLEADER) {
      // these need to show when this client is
      // assigning their own status or they are NOT the
      // leader
      if (uiInfo.teamLeader) {
        // if not showing yourself
        if (!(ui_selectedPlayer.integer < uiInfo.myTeamCount &&
              uiInfo.teamClientNums[ui_selectedPlayer.integer] ==
                  uiInfo.playerNumber)) {
          vis = qfalse;
        }
      }
      flags &= ~UI_SHOW_NOTLEADER;
    }
    if (flags & UI_SHOW_FAVORITESERVERS) {
      // this assumes you only put this type of display
      // flag on something showing in the proper context
      if (ui_netSource.integer != AS_FAVORITES) {
        vis = qfalse;
      }
      flags &= ~UI_SHOW_FAVORITESERVERS;
    }
    if (flags & UI_SHOW_NOTFAVORITESERVERS) {
      // this assumes you only put this type of display
      // flag on something showing in the proper context
      if (ui_netSource.integer == AS_FAVORITES) {
        vis = qfalse;
      }
      flags &= ~UI_SHOW_NOTFAVORITESERVERS;
    }
    if (flags & UI_SHOW_ANYTEAMGAME) {
      flags &= ~UI_SHOW_ANYTEAMGAME;
    }
    if (flags & UI_SHOW_ANYNONTEAMGAME) {
      vis = qfalse;
      flags &= ~UI_SHOW_ANYNONTEAMGAME;
    }
    if (flags & UI_SHOW_NETANYTEAMGAME) {
      flags &= ~UI_SHOW_NETANYTEAMGAME;
    }
    if (flags & UI_SHOW_NETANYNONTEAMGAME) {
      vis = qfalse;
      flags &= ~UI_SHOW_NETANYNONTEAMGAME;
    }
    if (flags & UI_SHOW_NEWHIGHSCORE) {
      if (uiInfo.newHighScoreTime < uiInfo.uiDC.realTime) {
        vis = qfalse;
      } else {
        if (uiInfo.soundHighScore) {
          if (trap_Cvar_VariableValue("sv_killserver") == 0) {
            // wait on server to
            // go down before
            // playing sound
            trap_S_StartLocalSound(uiInfo.newHighScoreSound, CHAN_ANNOUNCER);
            uiInfo.soundHighScore = qfalse;
          }
        }
      }
      flags &= ~UI_SHOW_NEWHIGHSCORE;
    }
    if (flags & UI_SHOW_NEWBESTTIME) {
      if (uiInfo.newBestTime < uiInfo.uiDC.realTime) {
        vis = qfalse;
      }
      flags &= ~UI_SHOW_NEWBESTTIME;
    }
    if (flags & UI_SHOW_PLAYERMUTED) {
      if (!uiInfo.playerMuted[uiInfo.playerIndex]) {
        vis = qfalse;
      }
      flags &= ~UI_SHOW_PLAYERMUTED;
    }
    if (flags & UI_SHOW_PLAYERNOTMUTED) {
      if (uiInfo.playerMuted[uiInfo.playerIndex]) {
        vis = qfalse;
      }
      flags &= ~UI_SHOW_PLAYERNOTMUTED;
    }

    if (flags & UI_SHOW_PLAYERNOREFEREE) {
      flags &= ~UI_SHOW_PLAYERNOREFEREE;
    }
    if (flags & UI_SHOW_PLAYERREFEREE) {
      vis = qfalse;
      flags &= ~UI_SHOW_PLAYERREFEREE;
    }

    if (flags & UI_SHOW_DEMOAVAILABLE) {
      if (!uiInfo.demoAvailable) {
        vis = qfalse;
      }
      flags &= ~UI_SHOW_DEMOAVAILABLE;
    } else {
      flags = 0;
    }
  }
  return vis;
}

static qboolean UI_Handicap_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {
    int h;
    h = Com_Clamp(5, 100, trap_Cvar_VariableValue("handicap"));
    if (key == K_MOUSE2) {
      h -= 5;
    } else {
      h += 5;
    }
    if (h > 100) {
      h = 5;
    } else if (h < 0) {
      h = 100;
    }
    trap_Cvar_Set("handicap", va("%i", h));
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_Effects_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {

    if (key == K_MOUSE2) {
      uiInfo.effectsColor--;
    } else {
      uiInfo.effectsColor++;
    }

    if (uiInfo.effectsColor > 6) {
      uiInfo.effectsColor = 0;
    } else if (uiInfo.effectsColor < 0) {
      uiInfo.effectsColor = 6;
    }

    trap_Cvar_SetValue("color", uitogamecode[uiInfo.effectsColor]);
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_ClanName_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {
    int i;
    i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
    if (uiInfo.teamList[i].cinematic >= 0) {
      trap_CIN_StopCinematic(uiInfo.teamList[i].cinematic);
      uiInfo.teamList[i].cinematic = -1;
    }
    if (key == K_MOUSE2) {
      i--;
    } else {
      i++;
    }
    if (i >= uiInfo.teamCount) {
      i = 0;
    } else if (i < 0) {
      i = uiInfo.teamCount - 1;
    }
    trap_Cvar_Set("ui_teamName", uiInfo.teamList[i].teamName);
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_GameType_HandleKey(int flags, float *special, int key,
                                      qboolean resetMap) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {
    int oldCount = UI_MapCountByGameType();

    // hard coded mess here
    if (key == K_MOUSE2) {
      ui_gameType.integer--;
      if (ui_gameType.integer == 2) {
        ui_gameType.integer = 1;
      } else if (ui_gameType.integer < 2) {
        ui_gameType.integer = uiInfo.numGameTypes - 1;
      }
    } else {
      ui_gameType.integer++;
      if (ui_gameType.integer >= uiInfo.numGameTypes) {
        ui_gameType.integer = 1;
      } else if (ui_gameType.integer == 2) {
        ui_gameType.integer = 3;
      }
    }

    trap_Cvar_Set("ui_Q3Model", "0");

    trap_Cvar_Set("ui_gameType", va("%d", ui_gameType.integer));
    UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName,
                      uiInfo
                          .gameTypes[Numeric::clamp(ui_gameType.integer, 0,
                                                    MAX_GAMETYPES - 1)]
                          .gtEnum);
    if (resetMap && oldCount != UI_MapCountByGameType()) {
      trap_Cvar_Set("ui_currentMap", "0");
      Menu_SetFeederSelection(nullptr, FEEDER_MAPS, 0, nullptr);
    }
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_TeamName_HandleKey(int flags, float *special, int key,
                                      qboolean blue) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {
    int i;
    i = UI_TeamIndexFromName(
        UI_Cvar_VariableString((blue) ? "ui_blueTeam" : "ui_redTeam"));

    if (key == K_MOUSE2) {
      i--;
    } else {
      i++;
    }

    if (i >= uiInfo.teamCount) {
      i = 0;
    } else if (i < 0) {
      i = uiInfo.teamCount - 1;
    }

    trap_Cvar_Set((blue) ? "ui_blueTeam" : "ui_redTeam",
                  uiInfo.teamList[i].teamName);

    return qtrue;
  }
  return qfalse;
}

static qboolean UI_TeamMember_HandleKey(int flags, float *special, int key,
                                        qboolean blue, int num) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {
    // 0 - None
    // 1 - Human
    // 2..NumCharacters - Bot
    char *cvar = va(blue ? "ui_blueteam%i" : "ui_redteam%i", num);
    int value = trap_Cvar_VariableValue(cvar);

    if (key == K_MOUSE2) {
      value--;
    } else {
      value++;
    }

    if (value >= uiInfo.characterCount + 2) {
      value = 0;
    } else if (value < 0) {
      value = uiInfo.characterCount + 2 - 1;
    }

    trap_Cvar_Set(cvar, va("%i", value));
    return qtrue;
  }
  return qfalse;
}

/*static qboolean UI_NetSource_HandleKey(int flags, float *special, int key) {
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key ==
K_KP_ENTER) { if (key == K_MOUSE2) { ui_netSource.integer--; } else {
            ui_netSource.integer++;
        }

        if (ui_netSource.integer >= numNetSources) {
            ui_netSource.integer = 0;
        } else if (ui_netSource.integer < 0) {
            ui_netSource.integer = numNetSources - 1;
        }

        UI_BuildServerDisplayList(qtrue);
        if (ui_netSource.integer != AS_GLOBAL) {
            UI_StartServerRefresh(qtrue);
        }
        trap_Cvar_Set( "ui_netSource", va("%d", ui_netSource.integer));
        return qtrue;
    }
    return qfalse;
}*/

static qboolean UI_NetFilter_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {
    if (key == K_MOUSE2) {
      ui_serverFilterType--;
    } else {
      ui_serverFilterType++;
    }

    if (ui_serverFilterType >= numServerFilters) {
      ui_serverFilterType = 0;
    } else if (ui_serverFilterType < 0) {
      ui_serverFilterType = numServerFilters - 1;
    }
    UI_BuildServerDisplayList(qtrue);
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_OpponentName_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {
    if (key == K_MOUSE2) {
      UI_PriorOpponent();
    } else {
      UI_NextOpponent();
    }
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_BotName_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {
    //		int game =
    // trap_Cvar_VariableValue("g_gametype");
    int value = uiInfo.botIndex;

    if (key == K_MOUSE2) {
      value--;
    } else {
      value++;
    }

    if (value >= uiInfo.characterCount + 2) {
      value = 0;
    } else if (value < 0) {
      value = uiInfo.characterCount + 2 - 1;
    }
    uiInfo.botIndex = value;
    return qtrue;
  }
  return qfalse;
}

/*static qboolean UI_BotSkill_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
{ if (key == K_MOUSE2) { uiInfo.skillIndex--; } else { uiInfo.skillIndex++;
        }
        if (uiInfo.skillIndex >= numSkillLevels) {
            uiInfo.skillIndex = 0;
        } else if (uiInfo.skillIndex < 0) {
            uiInfo.skillIndex = numSkillLevels-1;
        }
    return qtrue;
  }
    return qfalse;
}*/

static qboolean UI_RedBlue_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {
    uiInfo.redBlue ^= 1;
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_Crosshair_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {
    if (key == K_MOUSE2) {
      uiInfo.currentCrosshair--;
    } else {
      uiInfo.currentCrosshair++;
    }

    if (uiInfo.currentCrosshair >= NUM_CROSSHAIRS) {
      uiInfo.currentCrosshair = 0;
    } else if (uiInfo.currentCrosshair < 0) {
      uiInfo.currentCrosshair = NUM_CROSSHAIRS - 1;
    }
    trap_Cvar_Set("cg_drawCrosshair", va("%d", uiInfo.currentCrosshair));
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_SelectedPlayer_HandleKey(int flags, float *special,
                                            int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER ||
      key == K_KP_ENTER) {
    int selected;

    UI_BuildPlayerList();
    if (!uiInfo.teamLeader) {
      return qfalse;
    }
    selected = trap_Cvar_VariableValue("cg_selectedPlayer");

    if (key == K_MOUSE2) {
      selected--;
    } else {
      selected++;
    }

    if (selected > uiInfo.myTeamCount) {
      selected = 0;
    } else if (selected < 0) {
      selected = uiInfo.myTeamCount;
    }

    if (selected == uiInfo.myTeamCount) {
      trap_Cvar_Set("cg_selectedPlayerName", "Everyone");
    } else {
      trap_Cvar_Set("cg_selectedPlayerName", uiInfo.teamNames[selected]);
    }
    trap_Cvar_Set("cg_selectedPlayer", va("%d", selected));
  }
  return qfalse;
}

static qboolean UI_OwnerDrawHandleKey(int ownerDraw, int flags, float *special,
                                      int key) {
  switch (ownerDraw) {
    case UI_HANDICAP:
      return UI_Handicap_HandleKey(flags, special, key);
      break;
    case UI_EFFECTS:
      return UI_Effects_HandleKey(flags, special, key);
      break;
    case UI_CLANNAME:
      return UI_ClanName_HandleKey(flags, special, key);
      break;
    case UI_GAMETYPE:
      return UI_GameType_HandleKey(flags, special, key, qtrue);
      break;
    case UI_BLUETEAMNAME:
      return UI_TeamName_HandleKey(flags, special, key, qtrue);
      break;
    case UI_REDTEAMNAME:
      return UI_TeamName_HandleKey(flags, special, key, qfalse);
      break;
    case UI_BLUETEAM1:
    case UI_BLUETEAM2:
    case UI_BLUETEAM3:
    case UI_BLUETEAM4:
    case UI_BLUETEAM5:
      UI_TeamMember_HandleKey(flags, special, key, qtrue,
                              ownerDraw - UI_BLUETEAM1 + 1);
      break;
    case UI_REDTEAM1:
    case UI_REDTEAM2:
    case UI_REDTEAM3:
    case UI_REDTEAM4:
    case UI_REDTEAM5:
      UI_TeamMember_HandleKey(flags, special, key, qfalse,
                              ownerDraw - UI_REDTEAM1 + 1);
      break;
    /*		case UI_NETSOURCE:
          UI_NetSource_HandleKey(flags, special, key);
                break;*/
    case UI_NETFILTER:
      UI_NetFilter_HandleKey(flags, special, key);
      break;
    case UI_OPPONENT_NAME:
      UI_OpponentName_HandleKey(flags, special, key);
      break;
    case UI_BOTNAME:
      return UI_BotName_HandleKey(flags, special, key);
      break;
    /*case UI_BOTSKILL:
        return UI_BotSkill_HandleKey(flags, special, key);
        break;*/
    case UI_REDBLUE:
      UI_RedBlue_HandleKey(flags, special, key);
      break;
    case UI_CROSSHAIR:
      UI_Crosshair_HandleKey(flags, special, key);
      break;
    case UI_SELECTEDPLAYER:
      UI_SelectedPlayer_HandleKey(flags, special, key);
      break;
    default:
      break;
  }

  return qfalse;
}

static float UI_GetValue(int ownerDraw, int type) { return 0; }

/*
=================
UI_ServersQsortCompare
=================
*/
static int QDECL UI_ServersQsortCompare(const void *arg1, const void *arg2) {
  return trap_LAN_CompareServers(
      ui_netSource.integer, uiInfo.serverStatus.sortKey,
      uiInfo.serverStatus.sortDir, *(const int *)arg1, *(const int *)arg2);
}

/*
=================
UI_ServersSort
=================
*/
void UI_ServersSort(int column, qboolean force) {

  if (!force) {
    if (uiInfo.serverStatus.sortKey == column) {
      return;
    }
  }

  uiInfo.serverStatus.sortKey = column;
  qsort(&uiInfo.serverStatus.displayServers[0],
        uiInfo.serverStatus.numDisplayServers, sizeof(int),
        UI_ServersQsortCompare);
}

/*
===============
UI_LoadMods
===============
*/
static void UI_LoadMods() {
  int numdirs;
  char dirlist[2048];
  char *dirptr;
  char *descptr;
  int i;
  int dirlen;

  uiInfo.modCount = 0;
  numdirs = trap_FS_GetFileList("$modlist", "", dirlist, sizeof(dirlist));
  dirptr = dirlist;
  for (i = 0; i < numdirs; i++) {
    dirlen = strlen(dirptr) + 1;
    descptr = dirptr + dirlen;
    uiInfo.modList[uiInfo.modCount].modName = String_Alloc(dirptr);
    uiInfo.modList[uiInfo.modCount].modDescr = String_Alloc(descptr);
    dirptr += dirlen + strlen(descptr) + 1;
    uiInfo.modCount++;
    if (uiInfo.modCount >= MAX_MODS) {
      break;
    }
  }
}

/*
===============
UI_LoadProfiles
===============
*/
static void UI_LoadProfiles() {
  int numdirs;
  char dirlist[2048];
  char *dirptr;
  // char	*descptr;
  int i;
  int dirlen;

  uiInfo.profileCount = 0;
  uiInfo.profileIndex = -1;
  numdirs = trap_FS_GetFileList("profiles", "/", dirlist, sizeof(dirlist));
  dirptr = dirlist;

  for (i = 0; i < numdirs; i++) {
    dirlen = strlen(dirptr) + 1;

    if (dirptr[0] && Q_stricmp(dirptr, ".") && Q_stricmp(dirptr, "..")) {
      int handle;
      pc_token_t token;

      if (!(handle =
                trap_PC_LoadSource(va("profiles/%s/profile.dat", dirptr)))) {
        dirptr += dirlen;
        continue;
      }

      if (!trap_PC_ReadToken(handle, &token)) {
        trap_PC_FreeSource(handle);
        dirptr += dirlen;
        continue;
      }

      uiInfo.profileList[uiInfo.profileCount].name = String_Alloc(token.string);
      trap_PC_FreeSource(handle);

      uiInfo.profileList[uiInfo.profileCount].dir = String_Alloc(dirptr);
      uiInfo.profileCount++;

      /*if( uiInfo.profileCount == 1 ) {
          int j;

          uiInfo.profileIndex = 0;
          trap_Cvar_Set( "ui_profile", token.string );

          for( j = 0; j < Menu_Count(); j++ ) {
              Menu_SetFeederSelection( Menu_Get(j),
      FEEDER_PROFILES, 0, NULL );
          }
      }*/
      if (uiInfo.profileIndex == -1) {
        Q_CleanStr(token.string);
        Q_CleanDirName(token.string);
        if (!Q_stricmp(token.string, cl_profile.string)) {
          int j;

          uiInfo.profileIndex = i;
          trap_Cvar_Set("ui_profile", uiInfo.profileList[0].name);
          trap_Cvar_Update(&ui_profile);

          for (j = 0; j < Menu_Count(); j++) {
            Menu_SetFeederSelection(Menu_Get(j), FEEDER_PROFILES,
                                    uiInfo.profileIndex, NULL);
          }
        }
      }

      if (uiInfo.profileCount >= MAX_PROFILES) {
        break;
      }
    }

    dirptr += dirlen;
  }

  if (uiInfo.profileIndex == -1) {
    int j;

    uiInfo.profileIndex = 0;
    trap_Cvar_Set("ui_profile", uiInfo.profileList[0].name);
    trap_Cvar_Update(&ui_profile);

    for (j = 0; j < Menu_Count(); j++) {
      Menu_SetFeederSelection(Menu_Get(j), FEEDER_PROFILES, 0, NULL);
    }
  }
}

/*
===============
UI_LoadMovies
===============
*/
static void UI_LoadMovies() {
  char movielist[4096];
  char *moviename;
  int i, len;

  uiInfo.movieCount = trap_FS_GetFileList("video", "roq", movielist, 4096);

  if (uiInfo.movieCount) {
    if (uiInfo.movieCount > MAX_MOVIES) {
      uiInfo.movieCount = MAX_MOVIES;
    }
    moviename = movielist;
    for (i = 0; i < uiInfo.movieCount; i++) {
      len = strlen(moviename);
      if (!Q_stricmp(moviename + len - 4, ".roq")) {
        moviename[len - 4] = '\0';
      }
      Q_strupr(moviename);
      uiInfo.movieList[i] = String_Alloc(moviename);
      moviename += len + 1;
    }
  }
}

/*
===============
UI_LoadDemos
===============
*/
static void UI_LoadDemos() {
  if (uiInfo.currentDemoPath.empty()) {
    uiInfo.currentDemoPath.emplace_back("demos");
  }

  const std::string path =
      ETJump::StringUtil::join(uiInfo.currentDemoPath, PATH_SEP_STRING);
  const std::string ext =
      "dm_" +
      std::to_string(static_cast<int>(trap_Cvar_VariableValue("protocol")));

  const std::vector<std::string> demoDirs =
      ETJump::FileSystem::getFileList(path, PATH_SEP_STRING, true);
  std::vector<FileSystemObjectInfo> directories;

  for (const auto &dir : demoDirs) {
    FileSystemObjectInfo objectInfo;
    objectInfo.type = FileSystemObjectType::Folder;
    objectInfo.name = dir;
    objectInfo.displayName = "^7" + ETJump::sanitize(objectInfo.name, false);

    if (dir == "." || dir == "..") {
      continue;
    }

    directories.emplace_back(objectInfo);
  }

  const std::vector<std::string> demoFiles =
      ETJump::FileSystem::getFileList(path, ext, true);
  std::vector<FileSystemObjectInfo> files;

  for (const auto &demo : demoFiles) {
    FileSystemObjectInfo objectInfo;
    objectInfo.type = FileSystemObjectType::Item;
    objectInfo.name = demo;
    objectInfo.displayName = ETJump::sanitize(objectInfo.name, false);
    files.emplace_back(objectInfo);
  }

  uiInfo.demoObjects = std::vector<FileSystemObjectInfo>();

  if (uiInfo.currentDemoPath.size() > 1) {
    FileSystemObjectInfo back;
    back.type = FileSystemObjectType::Folder;
    back.name = ".";

    std::string currentPath;

    for (size_t i = 0; i < uiInfo.currentDemoPath.size() - 1; i++) {
      currentPath += uiInfo.currentDemoPath[i] + '/';
    }

    // remove trailing slash
    currentPath.pop_back();
    back.displayName = "^7" + ETJump::sanitize(currentPath, false);
    FileSystemObjectInfo beginning;
    beginning.type = FileSystemObjectType::Folder;
    beginning.name = "..";
    beginning.displayName = "^7demos";

    uiInfo.demoObjects.push_back(beginning);
    uiInfo.demoObjects.push_back(back);
  }

  std::copy(std::begin(directories), std::end(directories),
            std::back_inserter(uiInfo.demoObjects));
  std::copy(std::begin(files), std::end(files),
            std::back_inserter(uiInfo.demoObjects));
}

/*
==============
UI_SetNextMap
==============
*/
/*
// TTimo: unused
static qboolean UI_SetNextMap(int actual, int index) {
    int i;
    for (i = actual + 1; i < uiInfo.mapCount; i++) {
        if (uiInfo.mapList[i].active) {
            Menu_SetFeederSelection(NULL, FEEDER_MAPS, index + 1, "skirmish");
            return qtrue;
        }
    }
    return qfalse;
}
*/

/*
==============
UI_StartSkirmish
==============
*/
static void UI_StartSkirmish(qboolean next) {}

void WM_setItemPic(char *name, const char *shader) {
  menuDef_t *menu = Menu_GetFocused();
  itemDef_t *item;

  item = Menu_FindItemByName(menu, name);
  if (item) {
    item->window.background = DC->registerShaderNoMip(shader);
  }
}

void WM_setVisibility(char *name, qboolean show) {
  menuDef_t *menu = Menu_GetFocused();
  itemDef_t *item;

  item = Menu_FindItemByName(menu, name);
  if (item) {
    if (show) {
      item->window.flags |= WINDOW_VISIBLE;
    } else {
      item->window.flags &= ~(WINDOW_VISIBLE | WINDOW_MOUSEOVER);
    }
  }
}

qboolean UI_CheckExecKey(int key) {
  menuDef_t *menu = Menu_GetFocused();

  if (g_editingField) {
    return qtrue;
  }

  if (key >= static_cast<int>(sizeof(menu->onKey) / sizeof(*menu->onKey))) {
    return qfalse;
  }

  if (!menu) {
    if (cl_bypassMouseInput.integer) {
      if (!trap_Key_GetCatcher()) {
        trap_Cvar_Set("cl_bypassMouseInput", "0");
      }
    }
    return qfalse;
  }

  if (menu->onKey[key]) {
    return qtrue;
  }

  return qfalse;
}

void WM_ActivateLimboChat() {
  menuDef_t *menu;
  itemDef_t *itemdef;

  menu = Menu_GetFocused();
  menu = Menus_ActivateByName("wm_limboChat", qtrue);

  if (!menu || g_editItem) {
    return;
  }

  itemdef = Menu_FindItemByName(menu, "window_limbo_chat");

  if (itemdef) {
    itemdef->cursorPos = 0;
    g_editingField = qtrue;
    g_editItem = itemdef;
    DC->setOverstrikeMode(qtrue);
  }
}
// -NERVE - SMF

/*
==============
UI_Update
==============
*/
void UI_Update(const char *name) {
  int val = trap_Cvar_VariableValue(name);

  if (Q_stricmp(name, "ui_SetName") == 0) {
    trap_Cvar_Set("name", UI_Cvar_VariableString("ui_Name"));
  } else if (Q_stricmp(name, "ui_setRate") == 0) {
    float rate = trap_Cvar_VariableValue("ui_rate");
    if (rate >= 5000) {
      trap_Cvar_Set("ui_cl_maxpackets", "30");
      trap_Cvar_Set("ui_cl_packetdup", "1");
    } else if (rate >= 4000) {
      trap_Cvar_Set("ui_cl_maxpackets", "15");
      trap_Cvar_Set("ui_cl_packetdup",
                    "2"); // favor less prediction errors when
                          // there's packet loss
    } else {
      trap_Cvar_Set("ui_cl_maxpackets", "15");
      trap_Cvar_Set("ui_cl_packetdup",
                    "1"); // favor lower bandwidth
    }
  } else if (Q_stricmp(name, "ui_GetName") == 0) {
    trap_Cvar_Set("ui_Name", UI_Cvar_VariableString("name"));
  } else if (Q_stricmp(name, "r_colorbits") == 0) {
    switch (val) {
      case 0:
        trap_Cvar_SetValue("r_depthbits", 0);
        trap_Cvar_SetValue("r_stencilbits", 0);
        break;
      case 16:
        trap_Cvar_SetValue("r_depthbits", 16);
        trap_Cvar_SetValue("r_stencilbits", 0);
        break;
      case 32:
        trap_Cvar_SetValue("r_depthbits", 24);
        break;
    }
  } else if (Q_stricmp(name, "ui_r_lodbias") == 0) {
    switch (val) {
      case 0:
        trap_Cvar_SetValue("ui_r_subdivisions", 4);
        break;
      case 1:
        trap_Cvar_SetValue("ui_r_subdivisions", 12);
        break;
      case 2:
        trap_Cvar_SetValue("ui_r_subdivisions", 20);
        break;
    }
  } else if (Q_stricmp(name, "ui_glCustom") == 0) {
    switch (val) {
      case 0: // high quality
        trap_Cmd_ExecuteText(EXEC_APPEND, "exec preset_high_ui.cfg\n");
        break;
      case 1: // normal
        trap_Cmd_ExecuteText(EXEC_APPEND, "exec preset_normal_ui.cfg\n");
        break;
      case 2: // fast
        trap_Cmd_ExecuteText(EXEC_APPEND, "exec preset_fast_ui.cfg\n");
        break;
      case 3: // fastest
        trap_Cmd_ExecuteText(EXEC_APPEND, "exec preset_fastest_ui.cfg\n");
        break;
    }
  } else if (Q_stricmp(name, "ui_mousePitch") == 0) {
    if (val == 0) {
      trap_Cvar_SetValue("m_pitch", 0.022f);
    } else {
      trap_Cvar_SetValue("m_pitch", -0.022f);
    }
  }
}

/*
==============
UI_RunMenuScript
==============
*/
void UI_RunMenuScript(const char **args) {
  const char *name, *name2;
  char *s;
  char buff[1024];
  int val; // NERVE - SMF
  menuDef_t *menu;

  if (String_Parse(args, &name)) {

    if (Q_stricmp(name, "StartServer") == 0) {
      int pb_sv, pb_cl;

      // DHM - Nerve
      if (!ui_dedicated.integer) {
        pb_sv = static_cast<int>(trap_Cvar_VariableValue("sv_punkbuster"));
        pb_cl = static_cast<int>(trap_Cvar_VariableValue("cl_punkbuster"));

        if (pb_sv && !pb_cl) {
          trap_Cvar_Set("com_errorMessage",
                        "You must either disable PunkBuster on the Server, or "
                        "enable PunkBuster on the Client before starting a "
                        "non-dedicated server.");
          Menus_ActivateByName("hostGamePopupError", qtrue);
          return;
        }
      }
      // dhm - Nerve

      trap_Cvar_Set("ui_connecting", "1");
      trap_Cvar_Set("cg_thirdPerson", "0");
      trap_Cvar_Set("cg_cameraOrbit", "0");
      trap_Cvar_SetValue("dedicated", Com_Clamp(0, 2, ui_dedicated.integer));
      trap_Cvar_SetValue("g_gametype", Com_Clamp(0, 8, ui_netGameType.integer));

      trap_Cmd_ExecuteText(
          EXEC_APPEND,
          va("wait ; wait ; map %s\n",
             uiInfo.mapList[ui_currentNetMap.integer].mapLoadName));

      // NERVE - SMF - set user cvars here
      // set timelimit
      val = static_cast<int>(trap_Cvar_VariableValue("ui_userTimelimit"));

      if (val && val != uiInfo.mapList[ui_mapIndex.integer].Timelimit) {
        trap_Cvar_Set("g_userTimelimit", va("%i", val));
      } else {
        trap_Cvar_Set("g_userTimelimit", "0");
      }

      // set axis respawn time
      val = static_cast<int>(trap_Cvar_VariableValue("ui_userAxisRespawnTime"));

      if (val && val != uiInfo.mapList[ui_mapIndex.integer].AxisRespawnTime) {
        trap_Cvar_Set("g_userAxisRespawnTime", va("%i", val));
      } else {
        trap_Cvar_Set("g_userAxisRespawnTime", "0");
      }

      // set allied respawn time
      val =
          static_cast<int>(trap_Cvar_VariableValue("ui_userAlliedRespawnTime"));

      if (val && val != uiInfo.mapList[ui_mapIndex.integer].AlliedRespawnTime) {
        trap_Cvar_Set("g_userAlliedRespawnTime", va("%i", val));
      } else {
        trap_Cvar_Set("g_userAlliedRespawnTime", "0");
      }
      // -NERVE - SMF
      return;
    }
    if (Q_stricmp(name, "resetDefaults") == 0) {
      trap_Cmd_ExecuteText(EXEC_APPEND,
                           "cvar_restart\n"); // NERVE - SMF - changed order
      trap_Cmd_ExecuteText(EXEC_APPEND, "exec default.cfg\n");
      trap_Cmd_ExecuteText(EXEC_APPEND, "exec language.cfg\n"); // NERVE - SMF
      trap_Cmd_ExecuteText(EXEC_APPEND, "setRecommended\n");    // NERVE - SMF
      Controls_SetDefaults(qfalse);
      trap_Cvar_Set("com_introPlayed", "1");
      trap_Cvar_Set("com_recommendedSet", "1"); // NERVE - SMF
      trap_Cmd_ExecuteText(EXEC_APPEND, "vid_restart\n");
      return;
    }
    if (Q_stricmp(name, "getCDKey") == 0) {
      char out[17];
      trap_GetCDKey(buff, 17);
      trap_Cvar_Set("cdkey1", "");
      trap_Cvar_Set("cdkey2", "");
      trap_Cvar_Set("cdkey3", "");
      trap_Cvar_Set("cdkey4", "");
      if (strlen(buff) == CDKEY_LEN) {
        Q_strncpyz(out, buff, 5);
        trap_Cvar_Set("cdkey1", out);
        Q_strncpyz(out, buff + 4, 5);
        trap_Cvar_Set("cdkey2", out);
        Q_strncpyz(out, buff + 8, 5);
        trap_Cvar_Set("cdkey3", out);
        Q_strncpyz(out, buff + 12, 5);
        trap_Cvar_Set("cdkey4", out);
      }
      return;
    }
    if (Q_stricmp(name, "verifyCDKey") == 0) {
      buff[0] = '\0';
      Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey1"));
      Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey2"));
      Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey3"));
      Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey4"));
      trap_Cvar_Set("cdkey", buff);
      if (trap_VerifyCDKey(buff, UI_Cvar_VariableString("cdkeychecksum"))) {
        trap_Cvar_Set("ui_cdkeyvalid",
                      trap_TranslateString("CD key appears to be valid."));
        trap_SetCDKey(buff);
      } else {
        trap_Cvar_Set(
            "ui_cdkeyvalid",
            trap_TranslateString("CD key does not appear to be valid."));
      }
      return;
    }
    if (Q_stricmp(name, "loadArenas") == 0) {
      UI_LoadArenas();
      UI_MapCountByGameType();
      Menu_SetFeederSelection(nullptr, FEEDER_ALLMAPS, 0, nullptr);
      return;
    }
    if (Q_stricmp(name, "updateNetMap") == 0) {
      Menu_SetFeederSelection(nullptr, FEEDER_ALLMAPS, ui_currentNetMap.integer,
                              nullptr);
      return;
    }
    if (Q_stricmp(name, "saveControls") == 0) {
      Controls_SetConfig(qtrue);
      return;
    }
    if (Q_stricmp(name, "loadControls") == 0) {
      Controls_GetConfig();
      return;
    }
    if (Q_stricmp(name, "clearError") == 0) {
      trap_Cvar_Set("com_errorMessage", "");
      trap_Cvar_Set("com_errorDiagnoseIP", "");
      trap_Cvar_Set("com_missingFiles", "");
      return;
    }
    if (Q_stricmp(name, "loadGameInfo") == 0) {
      UI_ParseGameInfo("gameinfo.txt");
      //			UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName,
      // uiInfo.gameTypes[Numeric::clamp(ui_gameType.integer, 0,
      // MAX_GAMETYPES-1)].gtEnum);
      return;
    }
    if (Q_stricmp(name, "resetScores") == 0) {
      UI_ClearScores();
      return;
    }
    if (Q_stricmp(name, "RefreshServers") == 0) {
      UI_StartServerRefresh(qtrue);
      UI_BuildServerDisplayList(qtrue);
      return;
    }
    if (Q_stricmp(name, "RefreshFilter") == 0) {
      // if we don't have any valid servers,
      // it's kinda safe to assume we would like to get a full new list
      UI_StartServerRefresh(uiInfo.serverStatus.numDisplayServers ? qfalse
                                                                  : qtrue);
      UI_BuildServerDisplayList(qtrue);
      return;
    }
    if (Q_stricmp(name, "LoadDemos") == 0) {
      UI_LoadDemos();
      return;
    }
    if (Q_stricmp(name, "LoadMovies") == 0) {
      UI_LoadMovies();
      return;

      //----(SA)	added
    }
    if (Q_stricmp(name, "LoadMods") == 0) {
      UI_LoadMods();
      return;
    }
    if (Q_stricmp(name, "playMovie") == 0) {
      if (uiInfo.previewMovie >= 0) {
        trap_CIN_StopCinematic(uiInfo.previewMovie);
      }
      trap_Cmd_ExecuteText(
          EXEC_APPEND,
          va("cinematic %s.roq 2\n", uiInfo.movieList[uiInfo.movieIndex]));
      return;
    }
    if (Q_stricmp(name, "RunMod") == 0) {
      trap_Cvar_Set("fs_game", uiInfo.modList[uiInfo.modIndex].modName);
      trap_Cmd_ExecuteText(EXEC_APPEND, "vid_restart;");
      return;
    }
    if (Q_stricmp(name, "RunDemo") == 0) {
      if (uiInfo.demoIndex >= 0 &&
          uiInfo.demoIndex < static_cast<int>(uiInfo.demoObjects.size())) {
        const auto &selected = uiInfo.demoObjects[uiInfo.demoIndex];
        if (selected.type == FileSystemObjectType::Folder) {
          if (selected.name == ".") {
            uiInfo.currentDemoPath.pop_back();
          } else if (selected.name == "..") {
            uiInfo.currentDemoPath.clear();
            uiInfo.currentDemoPath.emplace_back("demos");
          } else {
            uiInfo.currentDemoPath.push_back(selected.name);
          }
          UI_LoadDemos();
        } else {
          // pop front because demo command automatically
          // appends demos/ to the beginning of the path
          const auto &front = uiInfo.currentDemoPath.front();
          uiInfo.currentDemoPath.pop_front();

          std::string demoPath;

          // only append '/' in front of the name if we're inside a subfolder
          // otherwise the demo command will be '/demo /demoname.dm_84'
          // and ET: Legacy will try to open it as an absolute
          // system file system path and will fail on Linux
          if (uiInfo.currentDemoPath.empty()) {
            demoPath = uiInfo.demoObjects[uiInfo.demoIndex].name;
          } else {
            demoPath = ETJump::StringUtil::join(uiInfo.currentDemoPath, "/") +
                       "/" + uiInfo.demoObjects[uiInfo.demoIndex].name;
          }

          uiInfo.currentDemoPath.push_front(front);
          trap_Cmd_ExecuteText(EXEC_APPEND,
                               va("demo \"%s\"\n", demoPath.c_str()));
        }
      }
      return;
    }
    if (Q_stricmp(name, "deleteDemo") == 0) {
      if (uiInfo.demoIndex >= 0 &&
          uiInfo.demoIndex < static_cast<int>(uiInfo.demoObjects.size())) {
        auto demoPath = ETJump::StringUtil::join(uiInfo.currentDemoPath, "/") +
                        "/" + uiInfo.demoObjects[uiInfo.demoIndex].name;
        trap_FS_Delete(demoPath.c_str());
      }
      return;
    }
    if (Q_stricmp(name, "closeJoin") == 0) {
      if (uiInfo.serverStatus.refreshActive) {
        UI_StopServerRefresh();
        uiInfo.serverStatus.nextDisplayRefresh = 0;
        uiInfo.nextServerStatusRefresh = 0;
        uiInfo.nextFindPlayerRefresh = 0;
        UI_BuildServerDisplayList(qtrue);
      } else {
        Menus_CloseByName("joinserver");
        Menus_OpenByName("main");
      }
      return;
    }
    if (Q_stricmp(name, "StopRefresh") == 0) {
      UI_StopServerRefresh();
      uiInfo.serverStatus.nextDisplayRefresh = 0;
      uiInfo.nextServerStatusRefresh = 0;
      uiInfo.nextFindPlayerRefresh = 0;
      return;
    }
    if (Q_stricmp(name, "UpdateFilter") == 0) {
      trap_Cvar_Update(&ui_netSource);
      if (ui_netSource.integer == AS_LOCAL ||
          !uiInfo.serverStatus.numDisplayServers) {
        UI_StartServerRefresh(qtrue);
      }
      UI_BuildServerDisplayList(qtrue);
      UI_FeederSelection(FEEDER_SERVERS, 0);
      return;
    }
    if (Q_stricmp(name, "check_ServerStatus") == 0) {
      s = UI_Cvar_VariableString("com_errorDiagnoseIP");
      menu = Menus_FindByName("ingame_options");
      if (strlen(s) && strcmp(s, "localhost") != 0) {
        if (menu) {
          Menu_ShowItemByName(menu, "ctr_serverinfo", qtrue);
        }
      } else {
        if (menu) {
          Menu_ShowItemByName(menu, "ctr_serverinfo", qfalse);
        }
      }
      return;
    }
    if (Q_stricmp(name, "ServerStatus") == 0) {
      // the server info dialog has been turned into a modal thing
      // it can be called in several situations
      if (trap_Cvar_VariableValue("ui_serverBrowser") == 1) {
        // legacy, from the server browser
        trap_LAN_GetServerAddressString(
            ui_netSource.integer,
            uiInfo.serverStatus
                .displayServers[uiInfo.serverStatus.currentServer],
            uiInfo.serverStatusAddress, sizeof(uiInfo.serverStatusAddress));
        UI_BuildServerStatus(qtrue);
      } else {
        // use com_errorDiagnoseIP otherwise
        s = UI_Cvar_VariableString("com_errorDiagnoseIP");
        if (strlen(s) && strcmp(s, "localhost") != 0) {
          trap_Cvar_VariableStringBuffer("com_errorDiagnoseIP",
                                         uiInfo.serverStatusAddress,
                                         sizeof(uiInfo.serverStatusAddress));
          // this is ugly, have to force a non-zero display server count
          // to emit the query
          uiInfo.serverStatus.numDisplayServers = 1;
          UI_BuildServerStatus(qtrue);
        } else {
          // we can't close the menu from here, it's not open yet...
          // (that's the onOpen script)
          Com_Printf("Can't show Server Info (not found, or local server)\n");
        }
      }
      return;
    }
    if (Q_stricmp(name, "InGameServerStatus") == 0) {
      uiClientState_t cstate;
      trap_GetClientState(&cstate);
      Q_strncpyz(uiInfo.serverStatusAddress, cstate.servername,
                 sizeof(uiInfo.serverStatusAddress));
      UI_BuildServerStatus(qtrue);
      return;
    }
    if (Q_stricmp(name, "ServerStatus_diagnose") == 0) {
      // query server and display the URL buttons if the
      // error happened during a server connection
      // situation
      s = UI_Cvar_VariableString("com_errorDiagnoseIP");
      menu = Menus_FindByName("error_popmenu_diagnose");
      if (strlen(s) && strcmp(s, "localhost") != 0) {
        trap_Cvar_VariableStringBuffer("com_errorDiagnoseIP",
                                       uiInfo.serverStatusAddress,
                                       sizeof(uiInfo.serverStatusAddress));
        // this is ugly, have to force a non-zero display server count
        // to emit the query toggle the "Server Info" button
        uiInfo.serverStatus.numDisplayServers = 1;
        if (menu) {
          Menu_ShowItemByName(menu, "serverinfo", qtrue);
        }
        UI_BuildServerStatus(qtrue);
      } else {
        // don't send getinfo packet, hide
        // "Server Info" button
        if (menu) {
          Menu_ShowItemByName(menu, "serverinfo", qfalse);
        }
      }
      return;
    }
    if (Q_stricmp(name, "FoundPlayerServerStatus") == 0) {
      Q_strncpyz(
          uiInfo.serverStatusAddress,
          uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer],
          sizeof(uiInfo.serverStatusAddress));
      UI_BuildServerStatus(qtrue);
      Menu_SetFeederSelection(nullptr, FEEDER_FINDPLAYER, 0, nullptr);
      return;
    }
    if (Q_stricmp(name, "FindPlayer") == 0) {
      UI_BuildFindPlayerList(qtrue);
      // clear the displayed server status info
      uiInfo.serverStatusInfo.numLines = 0;
      Menu_SetFeederSelection(nullptr, FEEDER_FINDPLAYER, 0, nullptr);
      return;
    }
    if (Q_stricmp(name, "JoinServer") == 0) {
      if (uiInfo.serverStatus.currentServer >= 0 &&
          uiInfo.serverStatus.currentServer <
              uiInfo.serverStatus.numDisplayServers) {
        Menus_CloseAll();
        trap_Cvar_Set("ui_connecting", "1");
        trap_Cvar_Set("cg_thirdPerson", "0 ");
        trap_Cvar_Set("cg_cameraOrbit", "0");
        trap_LAN_GetServerAddressString(
            ui_netSource.integer,
            uiInfo.serverStatus
                .displayServers[uiInfo.serverStatus.currentServer],
            buff, 1024);
        trap_Cmd_ExecuteText(EXEC_APPEND, va("connect %s\n", buff));
      }
      return;
    }
    if (Q_stricmp(name, "JoinDirectServer") == 0) {
      Menus_CloseAll();
      trap_Cvar_Set("ui_connecting", "1");
      trap_Cvar_Set("cg_thirdPerson", "0");
      trap_Cvar_Set("cg_cameraOrbit", "0");
      trap_Cmd_ExecuteText(
          EXEC_APPEND,
          va("connect %s\n", UI_Cvar_VariableString("ui_connectToIPAddress")));
      return;
    }
    if (Q_stricmp(name, "FoundPlayerJoinServer") == 0) {
      if (uiInfo.currentFoundPlayerServer >= 0 &&
          uiInfo.currentFoundPlayerServer < uiInfo.numFoundPlayerServers) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("connect %s\n", uiInfo.foundPlayerServerAddresses
                                   [uiInfo.currentFoundPlayerServer]));
      }
      return;
    }
    if (Q_stricmp(name, "Quit") == 0) {
      trap_Cmd_ExecuteText(EXEC_NOW, "quit");
      return;
    }
    if (Q_stricmp(name, "Controls") == 0) {
      trap_Cvar_Set("cl_paused", "1");
      trap_Key_SetCatcher(KEYCATCH_UI);
      Menus_CloseAll();
      Menus_ActivateByName("setup_menu2", qtrue);
      return;
    }
    if (Q_stricmp(name, "Leave") == 0) {
      // ATVI Wolfenstein Misc #460
      // if we are running a local server, make sure we
      // kill it cleanly for other clients
      if (trap_Cvar_VariableValue("sv_running") != 0) {
        trap_Cvar_Set("sv_killserver", "1");
      } else {
        trap_Cmd_ExecuteText(EXEC_APPEND, "disconnect\n");
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_CloseAll();

        Menus_ActivateByName("backgroundmusic", qtrue);
        Menus_ActivateByName("main_opener", qtrue);
      }
      return;
    }
    if (Q_stricmp(name, "ServerSort") == 0) {
      int sortColumn;
      if (Int_Parse(args, &sortColumn)) {
        // if same column we're already sorting on then flip the direction
        if (sortColumn == uiInfo.serverStatus.sortKey) {
          uiInfo.serverStatus.sortDir = !uiInfo.serverStatus.sortDir;
        }
        // make sure we sort again
        UI_ServersSort(sortColumn, qtrue);
      }
      return;
    }
    if (Q_stricmp(name, "ServerSortDown") == 0) {
      int sortColumn;
      if (Int_Parse(args, &sortColumn)) {
        uiInfo.serverStatus.sortDir = 0;

        // make sure we sort again
        UI_ServersSort(sortColumn, qtrue);
      }
      return;
    }
    if (Q_stricmp(name, "nextSkirmish") == 0) {
      UI_StartSkirmish(qtrue);
      return;
    }
    if (Q_stricmp(name, "SkirmishStart") == 0) {
      UI_StartSkirmish(qfalse);
      return;
    }
    if (Q_stricmp(name, "closeingame") == 0) {
      trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
      trap_Key_ClearStates();
      trap_Cvar_Set("cl_paused", "0");
      Menus_CloseAll();
      return;
    }
    if (Q_stricmp(name, "voteMap") == 0) {
      if (ui_currentNetMap.integer >= 0 &&
          ui_currentNetMap.integer < uiInfo.mapCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("callvote %s %s\n", ui_voteCheats.integer ? "devmap" : "map",
               uiInfo.mapList[ui_currentNetMap.integer].mapLoadName));
      }
      return;
    }
    if (Q_stricmp(name, "voteKick") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(EXEC_APPEND,
                             va("callvote kick \"%s\"\n",
                                uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "voteMute") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(EXEC_APPEND,
                             va("callvote mute \"%s\"\n",
                                uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "voteUnMute") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(EXEC_APPEND,
                             va("callvote unmute \"%s\"\n",
                                uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "voteReferee") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(EXEC_APPEND,
                             va("callvote referee \"%s\"\n",
                                uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "voteUnReferee") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(EXEC_APPEND,
                             va("callvote unreferee \"%s\"\n",
                                uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "voteGame") == 0) {
      int ui_voteGameType =
          static_cast<int>(trap_Cvar_VariableValue("ui_voteGameType"));
      if (ui_voteGameType >= 0 && ui_voteGameType < uiInfo.numGameTypes) {
        trap_Cmd_ExecuteText(EXEC_APPEND,
                             va("callvote gametype %i\n", ui_voteGameType));
      }
      return;
    }
    if (Q_stricmp(name, "refGame") == 0) {
      int ui_voteGameType =
          static_cast<int>(trap_Cvar_VariableValue("ui_voteGameType"));
      if (ui_voteGameType >= 0 && ui_voteGameType < uiInfo.numGameTypes) {
        trap_Cmd_ExecuteText(EXEC_APPEND,
                             va("ref gametype %i\n", ui_voteGameType));
      }
      return;
    }
    if (Q_stricmp(name, "voteTimelimit") == 0) {
      trap_Cmd_ExecuteText(EXEC_APPEND,
                           va("callvote timelimit %f\n",
                              trap_Cvar_VariableValue("ui_voteTimelimit")));
      return;
    }
    if (Q_stricmp(name, "voteWarmupDamage") == 0) {
      trap_Cmd_ExecuteText(
          EXEC_APPEND,
          va("callvote warmupdamage %d\n",
             static_cast<int>(trap_Cvar_VariableValue("ui_voteWarmupDamage"))));
      return;
    }
    if (Q_stricmp(name, "refTimelimit") == 0) {
      trap_Cmd_ExecuteText(EXEC_APPEND,
                           va("ref timelimit %f\n",
                              trap_Cvar_VariableValue("ui_voteTimelimit")));
      return;
    }
    if (Q_stricmp(name, "refWarmupDamage") == 0) {
      trap_Cmd_ExecuteText(
          EXEC_APPEND,
          va("ref warmupdamage %d\n",
             static_cast<int>(trap_Cvar_VariableValue("ui_voteWarmupDamage"))));
      return;
    }
    if (Q_stricmp(name, "voteInitToggles") == 0) {
      char info[MAX_INFO_STRING];

      trap_GetConfigString(CS_SERVERTOGGLES, info, sizeof(info));
      trap_Cvar_Set("ui_voteWarmupDamage",
                    va("%d", ((Q_atoi(info) & CV_SVS_WARMUPDMG) >> 2)));

      trap_GetConfigString(CS_SERVERINFO, info, sizeof(info));
      trap_Cvar_Set("ui_voteTimelimit",
                    va("%i", Q_atoi(Info_ValueForKey(info, "timelimit"))));
      trap_Cvar_Set("ui_voteAutoRtv",
                    va("%i", Q_atoi(Info_ValueForKey(info, "g_autoRtv"))));

      return;
    }
    if (Q_stricmp(name, "voteLeader") == 0) {
      if (uiInfo.teamIndex >= 0 && uiInfo.teamIndex < uiInfo.myTeamCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("callteamvote leader %s\n", uiInfo.teamNames[uiInfo.teamIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "addBot") == 0) {
      trap_Cmd_ExecuteText(
          EXEC_APPEND,
          va("addbot %s %i %s\n", uiInfo.characterList[uiInfo.botIndex].name,
             uiInfo.skillIndex + 1, (uiInfo.redBlue == 0) ? "Red" : "Blue"));
      return;
    }
    if (Q_stricmp(name, "addFavorite") == 0) {
      if (ui_netSource.integer != AS_FAVORITES) {
        char serverName[MAX_NAME_LENGTH];
        char addr[MAX_NAME_LENGTH];
        int res;

        trap_LAN_GetServerInfo(
            ui_netSource.integer,
            uiInfo.serverStatus
                .displayServers[uiInfo.serverStatus.currentServer],
            buff, MAX_STRING_CHARS);
        serverName[0] = addr[0] = '\0';
        Q_strncpyz(serverName, Info_ValueForKey(buff, "hostname"),
                   MAX_NAME_LENGTH);
        Q_strncpyz(addr, Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
        if (strlen(serverName) > 0 && strlen(addr) > 0) {
          res = trap_LAN_AddServer(AS_FAVORITES, serverName, addr);
          if (res == 0) {
            // server already in the list
            Com_Printf(trap_TranslateString("Favorite already in list\n"));
          } else if (res == -1) {
            // list full
            Com_Printf(trap_TranslateString("Favorite list full\n"));
          } else {
            // successfully added
            Com_Printf(trap_TranslateString("Added favorite server %s\n"),
                       addr);
          }
        }
      }
      return;
    }
    if (Q_stricmp(name, "deleteFavorite") == 0) {
      if (ui_netSource.integer == AS_FAVORITES) {
        char addr[MAX_NAME_LENGTH];
        trap_LAN_GetServerInfo(
            ui_netSource.integer,
            uiInfo.serverStatus
                .displayServers[uiInfo.serverStatus.currentServer],
            buff, MAX_STRING_CHARS);
        addr[0] = '\0';
        Q_strncpyz(addr, Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
        if (strlen(addr) > 0) {
          trap_LAN_RemoveServer(AS_FAVORITES, addr);
        }
      }
      return;
    }
    if (Q_stricmp(name, "createFavorite") == 0) {
      if (ui_netSource.integer == AS_FAVORITES) {
        char serverName[MAX_NAME_LENGTH];
        char addr[MAX_NAME_LENGTH];
        int res;

        serverName[0] = addr[0] = '\0';
        Q_strncpyz(serverName, UI_Cvar_VariableString("ui_favoriteName"),
                   MAX_NAME_LENGTH);
        Q_strncpyz(addr, UI_Cvar_VariableString("ui_favoriteAddress"),
                   MAX_NAME_LENGTH);
        if (strlen(serverName) > 0 && strlen(addr) > 0) {
          res = trap_LAN_AddServer(AS_FAVORITES, serverName, addr);
          if (res == 0) {
            // server already in the list
            Com_Printf(trap_TranslateString("Favorite already in list\n"));
          } else if (res == -1) {
            // list full
            Com_Printf(trap_TranslateString("Favorite list full\n"));
          } else {
            // successfully added
            Com_Printf(trap_TranslateString("Added favorite server %s\n"),
                       addr);
          }
        }
      }
      return;
    }
    if (Q_stricmp(name, "createFavoriteIngame") == 0) {
      uiClientState_t cstate;
      char serverName[MAX_NAME_LENGTH];
      char addr[MAX_NAME_LENGTH];
      int res;

      trap_GetClientState(&cstate);

      addr[0] = '\0';
      serverName[0] = '\0';
      Q_strncpyz(addr, cstate.servername, MAX_NAME_LENGTH);
      Q_strncpyz(serverName, cstate.servername, MAX_NAME_LENGTH);
      if (*serverName && *addr && Q_stricmp(addr, "localhost")) {
        res = trap_LAN_AddServer(AS_FAVORITES, serverName, addr);
        if (res == 0) {
          // server already in the list
          Com_Printf(trap_TranslateString("Favorite already in list\n"));
        } else if (res == -1) {
          // list full
          Com_Printf(trap_TranslateString("Favorite list full\n"));
        } else {
          // successfully added
          Com_Printf(trap_TranslateString("Added favorite server %s\n"), addr);
        }
      }
      return;
    }
    if (Q_stricmp(name, "orders") == 0) {
      const char *orders;
      if (String_Parse(args, &orders)) {
        int selectedPlayer =
            static_cast<int>(trap_Cvar_VariableValue("cg_selectedPlayer"));
        if (selectedPlayer < uiInfo.myTeamCount) {
          Q_strncpyz(buff, orders, sizeof(buff));
          trap_Cmd_ExecuteText(EXEC_APPEND,
                               va(buff, uiInfo.teamClientNums[selectedPlayer]));
          trap_Cmd_ExecuteText(EXEC_APPEND, "\n");
        } else {
          int i;
          for (i = 0; i < uiInfo.myTeamCount; i++) {
            if (Q_stricmp(UI_Cvar_VariableString("name"),
                          uiInfo.teamNames[i]) == 0) {
              continue;
            }
            Q_strncpyz(buff, orders, sizeof(buff));
            trap_Cmd_ExecuteText(EXEC_APPEND, va(buff, uiInfo.teamNames[i]));
            trap_Cmd_ExecuteText(EXEC_APPEND, "\n");
          }
        }
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
        trap_Key_ClearStates();
        trap_Cvar_Set("cl_paused", "0");
        Menus_CloseAll();
      }
      return;
    }
    if (Q_stricmp(name, "voiceOrdersTeam") == 0) {
      const char *orders;
      if (String_Parse(args, &orders)) {
        int selectedPlayer =
            static_cast<int>(trap_Cvar_VariableValue("cg_selectedPlayer"));
        if (selectedPlayer == uiInfo.myTeamCount) {
          trap_Cmd_ExecuteText(EXEC_APPEND, orders);
          trap_Cmd_ExecuteText(EXEC_APPEND, "\n");
        }
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
        trap_Key_ClearStates();
        trap_Cvar_Set("cl_paused", "0");
        Menus_CloseAll();
      }
      return;
    }
    if (Q_stricmp(name, "voiceOrders") == 0) {
      const char *orders;
      if (String_Parse(args, &orders)) {
        int selectedPlayer =
            static_cast<int>(trap_Cvar_VariableValue("cg_selectedPlayer"));
        if (selectedPlayer < uiInfo.myTeamCount) {
          Q_strncpyz(buff, orders, sizeof(buff));
          trap_Cmd_ExecuteText(EXEC_APPEND,
                               va(buff, uiInfo.teamClientNums[selectedPlayer]));
          trap_Cmd_ExecuteText(EXEC_APPEND, "\n");
        }
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
        trap_Key_ClearStates();
        trap_Cvar_Set("cl_paused", "0");
        Menus_CloseAll();
      }
      return;
    }
    if (Q_stricmp(name, "glCustom") == 0) {
      trap_Cvar_Set("ui_glCustom", "4");
      return;
    }
    if (Q_stricmp(name, "update") == 0) {
      if (String_Parse(args, &name2)) {
        UI_Update(name2);
      }
      // NERVE - SMF
      return;
    }
    if (Q_stricmp(name, "showSpecScores") == 0) {
      if (Q_atoi(UI_Cvar_VariableString("ui_isSpectator"))) {
        trap_Cmd_ExecuteText(EXEC_APPEND, "+scores\n");
      }
      return;
    }
    if (Q_stricmp(name, "setPbClStatus") == 0) {
      int stat;

      if (Int_Parse(args, &stat)) {
        trap_SetPbClStatus(stat);
      }
      // DHM - Nerve
      return;
    }
    if (Q_stricmp(name, "rconGame") == 0) {
      if (ui_netGameType.integer >= 0 &&
          ui_netGameType.integer < uiInfo.numGameTypes) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND, va("rcon g_gametype %i\n", ui_netGameType.integer));
      }
      return;
    }
    if (Q_stricmp(name, "rconMap") == 0) {
      if (ui_currentNetMap.integer >= 0 &&
          ui_currentNetMap.integer < uiInfo.mapCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("rcon map %s\n",
               uiInfo.mapList[ui_currentNetMap.integer].mapLoadName));
      }
      return;
    }
    if (Q_stricmp(name, "refMap") == 0) {
      if (ui_currentNetMap.integer >= 0 &&
          ui_currentNetMap.integer < uiInfo.mapCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("ref map %s\n",
               uiInfo.mapList[ui_currentNetMap.integer].mapLoadName));
      }
      return;
    }
    if (Q_stricmp(name, "rconKick") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("rcon kick \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "refKick") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("ref kick \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "rconBan") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("rcon ban \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "refMute") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("ref mute \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "refUnMute") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("ref unmute \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "refMakeAxis") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("ref putaxis \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "refMakeAllied") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(EXEC_APPEND,
                             va("ref putallies \"%s\"\n",
                                uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "refMakeSpec") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("ref remove \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "refUnReferee") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(EXEC_APPEND,
                             va("ref unreferee \"%s\"\n",
                                uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "refMakeReferee") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("ref referee \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "rconMakeReferee") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(EXEC_APPEND,
                             va("rcon makeReferee \"%s\"\n",
                                uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "rconRemoveReferee") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(EXEC_APPEND,
                             va("rcon removeReferee \"%s\"\n",
                                uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "rconMute") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("rcon mute \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "rconUnMute") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("rcon unmute \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "refWarning") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        char buffer[128];
        trap_Cvar_VariableStringBuffer("ui_warnreason", buffer, 128);

        trap_Cmd_ExecuteText(
            EXEC_APPEND, va("ref warn \"%s\" \"%s\"\n",
                            uiInfo.playerNames[uiInfo.playerIndex], buffer));
      }
      return;
    }
    if (Q_stricmp(name, "refWarmup") == 0) {
      char buffer[128];
      trap_Cvar_VariableStringBuffer("ui_warmup", buffer, 128);

      trap_Cmd_ExecuteText(EXEC_APPEND, va("ref warmup \"%s\"\n", buffer));
      return;
    }
    if (Q_stricmp(name, "ignorePlayer") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("ignore \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "unIgnorePlayer") == 0) {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) {
        trap_Cmd_ExecuteText(
            EXEC_APPEND,
            va("unignore \"%s\"\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
      return;
    }
    if (Q_stricmp(name, "loadCachedServers") == 0) {
      trap_LAN_LoadCachedServers(); // load servercache.dat
      return;
    }
    if (Q_stricmp(name, "loadProfiles") == 0) {
      UI_LoadProfiles();
      return;
    }
    if (Q_stricmp(name, "createProfile") == 0) {
      fileHandle_t f;
      char profileName[MAX_CVAR_VALUE_STRING];
      Q_strncpyz(profileName, ui_profile.string, sizeof(profileName));
      Q_CleanStr(profileName);
      Q_CleanDirName(profileName);

      if (trap_FS_FOpenFile(va("profiles/%s/profile.dat", profileName), &f,
                            FS_WRITE) >= 0) {
        trap_FS_Write(va("\"%s\"", ui_profile.string),
                      static_cast<int>(strlen(ui_profile.string)) + 2, f);
        trap_FS_FCloseFile(f);
      }
      trap_Cvar_Set("name", ui_profile.string);
      return;
    }
    if (Q_stricmp(name, "clearPID") == 0) {
      fileHandle_t f;

      // delete profile.pid from current profile
      if (trap_FS_FOpenFile(va("profiles/%s/profile.pid", cl_profile.string),
                            &f, FS_READ) >= 0) {
        trap_FS_FCloseFile(f);
        trap_FS_Delete(va("profiles/%s/profile.pid", cl_profile.string));
      }
      return;
    }
    if (Q_stricmp(name, "applyProfile") == 0) {
      Q_strncpyz(cl_profile.string, ui_profile.string,
                 sizeof(cl_profile.string));
      Q_CleanStr(cl_profile.string);
      Q_CleanDirName(cl_profile.string);
      trap_Cvar_Set("cl_profile", cl_profile.string);
      return;
    }
    if (Q_stricmp(name, "setDefaultProfile") == 0) {
      fileHandle_t f;

      Q_strncpyz(cl_defaultProfile.string, ui_profile.string,
                 sizeof(cl_profile.string));
      Q_CleanStr(cl_defaultProfile.string);
      Q_CleanDirName(cl_defaultProfile.string);
      trap_Cvar_Set("cl_defaultProfile", cl_defaultProfile.string);

      if (trap_FS_FOpenFile("profiles/defaultprofile.dat", &f, FS_WRITE) >= 0) {
        trap_FS_Write(va("\"%s\"", cl_defaultProfile.string),
                      static_cast<int>(strlen(cl_defaultProfile.string)) + 2,
                      f);
        trap_FS_FCloseFile(f);
      }
      return;
    }
    if (Q_stricmp(name, "deleteProfile") == 0) {
      char profileName[MAX_CVAR_VALUE_STRING];

      Q_strncpyz(profileName, ui_profile.string, sizeof(profileName));
      Q_CleanStr(profileName);
      Q_CleanDirName(profileName);

      // can't delete active profile
      if (Q_stricmp(profileName, cl_profile.string)) {
        if (!Q_stricmp(profileName, cl_defaultProfile.string)) {
          // if deleting the default profile,
          // set the default to the current active profile
          fileHandle_t f;

          trap_Cvar_Set("cl_defaultProfile", cl_profile.string);
          if (trap_FS_FOpenFile("profiles/defaultprofile.dat", &f, FS_WRITE) >=
              0) {
            trap_FS_Write(va("\"%s\"", cl_profile.string),
                          static_cast<int>(strlen(cl_profile.string)) + 2, f);
            trap_FS_FCloseFile(f);
          }
        }

        trap_FS_Delete(va("profiles/%s", profileName));
      }
      return;
    }
    if (Q_stricmp(name, "renameProfileInit") == 0) {
      trap_Cvar_Set("ui_profile_renameto", ui_profile.string);
      return;
    }
    if (Q_stricmp(name, "renameProfile") == 0) {
      fileHandle_t f, f2;
      int len;
      char profileName[MAX_CVAR_VALUE_STRING];
      char ui_renameprofileto[MAX_CVAR_VALUE_STRING];
      char uiprofile[MAX_CVAR_VALUE_STRING];

      trap_Cvar_VariableStringBuffer("ui_profile_renameto", ui_renameprofileto,
                                     sizeof(ui_renameprofileto));
      Q_strncpyz(profileName, ui_renameprofileto, sizeof(profileName));
      Q_CleanStr(profileName);
      Q_CleanDirName(profileName);

      Q_strncpyz(uiprofile, ui_profile.string, sizeof(uiprofile));
      Q_CleanStr(uiprofile);
      Q_CleanDirName(uiprofile);

      if (trap_FS_FOpenFile(va("profiles/%s/profile.dat", profileName), &f,
                            FS_WRITE) >= 0) {
        trap_FS_Write(va("\"%s\"", ui_renameprofileto),
                      static_cast<int>(strlen(ui_renameprofileto)) + 2, f);
        trap_FS_FCloseFile(f);
      }

      // FIXME: make this copying handle all files in the profiles directory
      if (Q_stricmp(uiprofile, profileName)) {
        if (trap_FS_FOpenFile(va("profiles/%s/%s", profileName, CONFIG_NAME),
                              &f, FS_WRITE) >= 0) {
          if ((len = trap_FS_FOpenFile(
                   va("profiles/%s/%s", uiprofile, CONFIG_NAME), &f2,
                   FS_READ)) >= 0) {
            int i;

            for (i = 0; i < len; i++) {
              byte b;

              trap_FS_Read(&b, 1, f2);
              trap_FS_Write(&b, 1, f);
            }
            trap_FS_FCloseFile(f2);
          }
          trap_FS_FCloseFile(f);
        }

        if (trap_FS_FOpenFile(va("profiles/%s/servercache.dat", profileName),
                              &f, FS_WRITE) >= 0) {
          if ((len = trap_FS_FOpenFile(
                   va("profiles/%s/servercache.dat", cl_profile.string), &f2,
                   FS_READ)) >= 0) {
            int i;

            for (i = 0; i < len; i++) {
              byte b;

              trap_FS_Read(&b, 1, f2);
              trap_FS_Write(&b, 1, f);
            }
            trap_FS_FCloseFile(f2);
          }
          trap_FS_FCloseFile(f);
        }

        if (!Q_stricmp(uiprofile, cl_defaultProfile.string)) {
          // if renaming the default profile, set the default to the new profile
          trap_Cvar_Set("cl_defaultProfile", profileName);
          if (trap_FS_FOpenFile("profiles/defaultprofile.dat", &f, FS_WRITE) >=
              0) {
            trap_FS_Write(va("\"%s\"", profileName),
                          static_cast<int>(strlen(profileName)) + 2, f);
            trap_FS_FCloseFile(f);
          }
        }

        // if renaming the active profile, set active to new name
        if (!Q_stricmp(uiprofile, cl_profile.string)) {
          trap_Cvar_Set("cl_profile", profileName);
        }

        // delete the old profile
        trap_FS_Delete(va("profiles/%s", uiprofile));
      }

      trap_Cvar_Set("ui_profile", ui_renameprofileto);
      trap_Cvar_Set("ui_profile_renameto", "");
      return;
    }
    if (Q_stricmp(name, "togglePbSvStatus") == 0) {
      // TTimo
      int sv_pb = static_cast<int>(trap_Cvar_VariableValue("sv_punkbuster"));
      if (sv_pb) {
        trap_SetPbSvStatus(0);
      } else {
        trap_SetPbSvStatus(1);
      }
      return;
    }
    if (Q_stricmp(name, "initHostGameFeatures") == 0) {
      int cvar = static_cast<int>(trap_Cvar_VariableValue("g_maxlives"));
      if (cvar) {
        trap_Cvar_Set("ui_maxlives", "1");
      } else {
        trap_Cvar_Set("ui_maxlives", "0");
      }

      return;
    }
    if (Q_stricmp(name, "toggleMaxLives") == 0) {
      int ui_ml = static_cast<int>(trap_Cvar_VariableValue("ui_maxlives"));
      if (ui_ml) {
        trap_Cvar_Set("g_maxlives", "5");
      } else {
        trap_Cvar_Set("g_maxlives", "0");
      }
      return;
    }
    if (Q_stricmp(name, "openModURL") == 0) {
      trap_Cvar_Set("ui_finalURL", UI_Cvar_VariableString("ui_modURL"));
      return;
    }
    if (Q_stricmp(name, "openServerURL") == 0) {
      trap_Cvar_Set("ui_finalURL", UI_Cvar_VariableString("ui_URL"));
      return;
    }
    if (Q_stricmp(name, "validate_openURL") == 0) {
      // this is the only one that effectively triggers the URL,
      // after the disclaimers are done with we use ui_finalURL as
      // an auxiliary variable to gather URLs from various sources
      trap_openURL(UI_Cvar_VariableString("ui_finalURL"));
      return;
    }
    if (Q_stricmp(name, "clientCheckVote") == 0) {
      int flags = static_cast<int>(trap_Cvar_VariableValue("cg_ui_voteFlags"));

      if (flags == VOTING_DISABLED) {
        trap_Cvar_SetValue("cg_ui_novote", 1);
      } else {
        trap_Cvar_SetValue("cg_ui_novote", 0);
      }
      return;
    }
    if (Q_stricmp(name, "reconnect") == 0) {
      // TODO: if dumped because of cl_allowdownload problem,
      //  toggle on first (we don't have appropriate support for this yet)
      trap_Cmd_ExecuteText(EXEC_APPEND, "reconnect\n");
      return;
    }
    if (Q_stricmp(name, "redirect") == 0) // fretn
    {
      char buf[MAX_STRING_CHARS];
      trap_Cvar_VariableStringBuffer("com_errorMessage", buf, sizeof(buf));
      trap_Cmd_ExecuteText(EXEC_APPEND, va("connect %s\n", buf));
      trap_Cvar_Set("com_errorMessage", "");
      return;
    }
    if (Q_stricmp(name, "updateGameType") == 0) {
      trap_Cvar_Update(&ui_netGameType);
      if (ui_mapIndex.integer >= UI_MapCountByGameType()) {
        Menu_SetFeederSelection(nullptr, FEEDER_MAPS, 0, nullptr);
        Menu_SetFeederSelection(nullptr, FEEDER_ALLMAPS, 0, nullptr);
      }
      return;
    }
    // ydnar

    if (Q_stricmp(name, "vidSave") == 0) {
      int mode;

      // get mode
      mode = static_cast<int>(trap_Cvar_VariableValue("r_mode"));

      // save mode to old mode
      trap_Cvar_SetValue("r_oldMode", static_cast<float>(mode));
      return;
    }
    if (Q_stricmp(name, "vidReset") == 0) {
      int oldMode;

      // get old mode
      oldMode = static_cast<int>(trap_Cvar_VariableValue("r_oldMode"));
      if (oldMode == 0) {
        oldMode = 3;
      }

      // reset mode to old mode
      trap_Cvar_SetValue("r_mode", static_cast<float>(oldMode));
      trap_Cvar_Set("r_oldMode", "");
      return;
    }
    if (Q_stricmp(name, "vidConfirm") == 0) {
      trap_Cvar_Set("r_oldMode", "");
      return;
    }
    if (Q_stricmp(name, "systemCvarsGet") == 0) {
      int ui_r_mode = trap_Cvar_VariableValue("r_mode");
      float ui_r_gamma = trap_Cvar_VariableValue("r_gamma");
      int ui_rate = trap_Cvar_VariableValue("rate");
      int ui_cl_maxpackets = trap_Cvar_VariableValue("cl_maxpackets");
      int ui_cl_packetdup = trap_Cvar_VariableValue("cl_packetdup");
      float ui_sensitivity = trap_Cvar_VariableValue("sensitivity");
      int ui_r_colorbits = trap_Cvar_VariableValue("r_colorbits");
      int ui_r_fullscreen = trap_Cvar_VariableValue("r_fullscreen");
      int ui_r_lodbias = trap_Cvar_VariableValue("r_lodbias");
      int ui_r_subdivisions = trap_Cvar_VariableValue("r_subdivisions");
      int ui_r_picmip = trap_Cvar_VariableValue("r_picmip");
      int ui_r_texturebits = trap_Cvar_VariableValue("r_texturebits");
      int ui_r_depthbits = trap_Cvar_VariableValue("r_depthbits");
      int ui_r_ext_compressed_textures =
          trap_Cvar_VariableValue("r_ext_compressed_textures");
      int ui_r_finish = trap_Cvar_VariableValue("r_finish");
      int ui_r_dynamiclight = trap_Cvar_VariableValue("r_dynamiclight");
      int ui_r_allowextensions = trap_Cvar_VariableValue("r_allowextensions");
      int ui_m_filter = trap_Cvar_VariableValue("m_filter");
      int ui_s_khz = trap_Cvar_VariableValue("s_khz");
      int ui_r_detailtextures = trap_Cvar_VariableValue("r_detailtextures");
      char ui_r_texturemode[MAX_CVAR_VALUE_STRING];
      trap_Cvar_VariableStringBuffer("r_texturemode", ui_r_texturemode,
                                     sizeof(ui_r_texturemode));

      trap_Cvar_Set("ui_r_mode", va("%i", ui_r_mode));
      trap_Cvar_Set("ui_r_gamma", va("%f", ui_r_gamma));
      trap_Cvar_Set("ui_rate", va("%i", ui_rate));
      trap_Cvar_Set("ui_cl_maxpackets", va("%i", ui_cl_maxpackets));
      trap_Cvar_Set("ui_cl_packetdup", va("%i", ui_cl_packetdup));
      trap_Cvar_Set("ui_sensitivity", va("%f", ui_sensitivity));
      trap_Cvar_Set("ui_r_colorbits", va("%i", ui_r_colorbits));
      trap_Cvar_Set("ui_r_fullscreen", va("%i", ui_r_fullscreen));
      trap_Cvar_Set("ui_r_lodbias", va("%i", ui_r_lodbias));
      trap_Cvar_Set("ui_r_subdivisions", va("%i", ui_r_subdivisions));
      trap_Cvar_Set("ui_r_picmip", va("%i", ui_r_picmip));
      trap_Cvar_Set("ui_r_texturebits", va("%i", ui_r_texturebits));
      trap_Cvar_Set("ui_r_depthbits", va("%i", ui_r_depthbits));
      trap_Cvar_Set("ui_r_ext_compressed_textures",
                    va("%i", ui_r_ext_compressed_textures));
      trap_Cvar_Set("ui_r_finish", va("%i", ui_r_finish));
      trap_Cvar_Set("ui_r_dynamiclight", va("%i", ui_r_dynamiclight));
      trap_Cvar_Set("ui_r_allowextensions", va("%i", ui_r_allowextensions));
      trap_Cvar_Set("ui_m_filter", va("%i", ui_m_filter));
      trap_Cvar_Set("ui_s_khz", va("%i", ui_s_khz));
      trap_Cvar_Set("ui_r_detailtextures", va("%i", ui_r_detailtextures));
      trap_Cvar_Set("ui_r_texturemode", ui_r_texturemode);
      return;
    }
    if (Q_stricmp(name, "systemCvarsReset") == 0) {
      trap_Cvar_Set("ui_r_mode", "");
      trap_Cvar_Set("ui_r_gamma", "");
      trap_Cvar_Set("ui_rate", "");
      trap_Cvar_Set("ui_cl_maxpackets", "");
      trap_Cvar_Set("ui_cl_packetdup", "");
      trap_Cvar_Set("ui_sensitivity", "");
      trap_Cvar_Set("ui_r_colorbits", "");
      trap_Cvar_Set("ui_r_fullscreen", "");
      trap_Cvar_Set("ui_r_lodbias", "");
      trap_Cvar_Set("ui_r_subdivisions", "");
      trap_Cvar_Set("ui_r_picmip", "");
      trap_Cvar_Set("ui_r_texturebits", "");
      trap_Cvar_Set("ui_r_depthbits", "");
      trap_Cvar_Set("ui_r_ext_compressed_textures", "");
      trap_Cvar_Set("ui_r_finish", "");
      trap_Cvar_Set("ui_r_dynamiclight", "");
      trap_Cvar_Set("ui_r_allowextensions", "");
      trap_Cvar_Set("ui_m_filter", "");
      trap_Cvar_Set("ui_s_khz", "");
      trap_Cvar_Set("ui_r_detailtextures", "");
      trap_Cvar_Set("ui_r_texturemode", "");
      return;
    }
    if (Q_stricmp(name, "systemCvarsApply") == 0) {
      int ui_r_mode = static_cast<int>(trap_Cvar_VariableValue("ui_r_mode"));
      float ui_r_gamma = trap_Cvar_VariableValue("ui_r_gamma");
      int ui_rate = static_cast<int>(trap_Cvar_VariableValue("ui_rate"));
      int ui_cl_maxpackets =
          static_cast<int>(trap_Cvar_VariableValue("ui_cl_maxpackets"));
      int ui_cl_packetdup =
          static_cast<int>(trap_Cvar_VariableValue("ui_cl_packetdup"));
      float ui_sensitivity = trap_Cvar_VariableValue("ui_sensitivity");
      int ui_r_colorbits =
          static_cast<int>(trap_Cvar_VariableValue("ui_r_colorbits"));
      int ui_r_fullscreen =
          static_cast<int>(trap_Cvar_VariableValue("ui_r_fullscreen"));
      int ui_r_lodbias =
          static_cast<int>(trap_Cvar_VariableValue("ui_r_lodbias"));
      int ui_r_subdivisions =
          static_cast<int>(trap_Cvar_VariableValue("ui_r_subdivisions"));
      int ui_r_picmip =
          static_cast<int>(trap_Cvar_VariableValue("ui_r_picmip"));
      int ui_r_texturebits =
          static_cast<int>(trap_Cvar_VariableValue("ui_r_texturebits"));
      int ui_r_depthbits =
          static_cast<int>(trap_Cvar_VariableValue("ui_r_depthbits"));
      int ui_r_ext_compressed_textures = static_cast<int>(
          trap_Cvar_VariableValue("ui_r_ext_compressed_textures"));
      int ui_r_finish =
          static_cast<int>(trap_Cvar_VariableValue("ui_r_finish"));
      int ui_r_dynamiclight =
          static_cast<int>(trap_Cvar_VariableValue("ui_r_dynamiclight"));
      int ui_r_allowextensions =
          static_cast<int>(trap_Cvar_VariableValue("ui_r_allowextensions"));
      int ui_m_filter =
          static_cast<int>(trap_Cvar_VariableValue("ui_m_filter"));
      int ui_s_khz = static_cast<int>(trap_Cvar_VariableValue("ui_s_khz"));
      int ui_r_detailtextures =
          static_cast<int>(trap_Cvar_VariableValue("ui_r_detailtextures"));
      char ui_r_texturemode[MAX_CVAR_VALUE_STRING];
      trap_Cvar_VariableStringBuffer("ui_r_texturemode", ui_r_texturemode,
                                     sizeof(ui_r_texturemode));

      // failsafe
      if (ui_rate == 0) {
        ui_rate = 5000;
        ui_cl_maxpackets = 30;
        ui_cl_packetdup = 1;
      }

      trap_Cvar_Set("r_mode", va("%i", ui_r_mode));
      trap_Cvar_Set("r_gamma", va("%f", ui_r_gamma));
      trap_Cvar_Set("rate", va("%i", ui_rate));
      trap_Cvar_Set("cl_maxpackets", va("%i", ui_cl_maxpackets));
      trap_Cvar_Set("cl_packetdup", va("%i", ui_cl_packetdup));
      trap_Cvar_Set("sensitivity", va("%f", ui_sensitivity));
      trap_Cvar_Set("r_colorbits", va("%i", ui_r_colorbits));
      trap_Cvar_Set("r_fullscreen", va("%i", ui_r_fullscreen));
      trap_Cvar_Set("r_lodbias", va("%i", ui_r_lodbias));
      trap_Cvar_Set("r_subdivisions", va("%i", ui_r_subdivisions));
      trap_Cvar_Set("r_picmip", va("%i", ui_r_picmip));
      trap_Cvar_Set("r_texturebits", va("%i", ui_r_texturebits));
      trap_Cvar_Set("r_depthbits", va("%i", ui_r_depthbits));
      trap_Cvar_Set("r_ext_compressed_textures",
                    va("%i", ui_r_ext_compressed_textures));
      trap_Cvar_Set("r_finish", va("%i", ui_r_finish));
      trap_Cvar_Set("r_dynamiclight", va("%i", ui_r_dynamiclight));
      trap_Cvar_Set("r_allowextensions", va("%i", ui_r_allowextensions));
      trap_Cvar_Set("m_filter", va("%i", ui_m_filter));
      trap_Cvar_Set("s_khz", va("%i", ui_s_khz));
      trap_Cvar_Set("r_detailtextures", va("%i", ui_r_detailtextures));
      trap_Cvar_Set("r_texturemode", ui_r_texturemode);

      trap_Cvar_Set("ui_r_mode", "");
      trap_Cvar_Set("ui_r_gamma", "");
      trap_Cvar_Set("ui_rate", "");
      trap_Cvar_Set("ui_cl_maxpackets", "");
      trap_Cvar_Set("ui_cl_packetdup", "");
      trap_Cvar_Set("ui_sensitivity", "");
      trap_Cvar_Set("ui_r_colorbits", "");
      trap_Cvar_Set("ui_r_fullscreen", "");
      trap_Cvar_Set("ui_r_lodbias", "");
      trap_Cvar_Set("ui_r_subdivisions", "");
      trap_Cvar_Set("ui_r_picmip", "");
      trap_Cvar_Set("ui_r_texturebits", "");
      trap_Cvar_Set("ui_r_depthbits", "");
      trap_Cvar_Set("ui_r_ext_compressed_textures", "");
      trap_Cvar_Set("ui_r_finish", "");
      trap_Cvar_Set("ui_r_dynamiclight", "");
      trap_Cvar_Set("ui_r_allowextensions", "");
      trap_Cvar_Set("ui_m_filter", "");
      trap_Cvar_Set("ui_s_khz", "");
      trap_Cvar_Set("ui_r_detailtextures", "");
      trap_Cvar_Set("ui_r_texturemode", "");
      return;
    }
    if (Q_stricmp(name, "profileCvarsGet") == 0) {
      int ui_profile_mousePitch =
          static_cast<int>(trap_Cvar_VariableValue("ui_mousePitch"));

      trap_Cvar_Set("ui_profile_mousePitch", va("%i", ui_profile_mousePitch));
      return;
    }
    if (Q_stricmp(name, "profileCvarsApply") == 0) {
      int ui_handedness =
          static_cast<int>(trap_Cvar_VariableValue("ui_handedness"));
      int ui_profile_mousePitch =
          static_cast<int>(trap_Cvar_VariableValue("ui_profile_mousePitch"));

      trap_Cvar_Set("ui_mousePitch", va("%i", ui_profile_mousePitch));

      if (ui_profile_mousePitch == 0) {
        trap_Cvar_SetValue("m_pitch", 0.022f);
      } else {
        trap_Cvar_SetValue("m_pitch", -0.022f);
      }

      if (ui_handedness == 0) {
        // exec default.cfg
        trap_Cmd_ExecuteText(EXEC_APPEND, "exec default.cfg\n");
        Controls_SetDefaults(qfalse);
      } else {
        // exec default_left.cfg
        trap_Cmd_ExecuteText(EXEC_APPEND, "exec default_left.cfg\n");
        Controls_SetDefaults(qtrue);
      }

      trap_Cvar_Set("ui_handedness", "");
      trap_Cvar_Set("ui_profile_mousePitch", "");
      return;
    }
    if (Q_stricmp(name, "profileCvarsReset") == 0) {
      trap_Cvar_Set("ui_handedness", "");
      trap_Cvar_Set("ui_profile_mousePitch", "");
      return;
    }
    if (Q_stricmp(name, "defaultControls") == 0) {
      int ui_handedness =
          static_cast<int>(trap_Cvar_VariableValue("ui_handedness"));

      if (ui_handedness == 0) {
        // exec default.cfg
        trap_Cmd_ExecuteText(EXEC_APPEND, "exec default.cfg\n");
        Controls_SetDefaults(qfalse);
      } else {
        // exec default_left.cfg
        trap_Cmd_ExecuteText(EXEC_APPEND, "exec default_left.cfg\n");
        Controls_SetDefaults(qtrue);
      }
      return;
    }
    if (!Q_stricmp(name, "voteAutoRtv")) {
      trap_Cmd_ExecuteText(
          EXEC_APPEND,
          va("callvote autoRtv %i\n",
             static_cast<int>(trap_Cvar_VariableValue("ui_voteAutoRtv"))));
      return;
    }

    if (!Q_stricmp(name, "uiPreviousMenu")) {
      if (String_Parse(args, &name2)) {
        if (!Q_stricmp(name2, "restore")) {
          Menus_OpenByName(uiPreviousMenu);
        } else {
          Q_strncpyz(uiPreviousMenu, name2, 256);
        }
      } else {
        uiPreviousMenu[0] = '\0';
        Com_Printf(S_COLOR_YELLOW
                   "WARNING: uiScript '%s' called with no arguments\n",
                   name);
      }

      return;
    }

    if (!Q_stricmp(name, "writeConfig")) {
      char buf[MAX_CVAR_VALUE_STRING];

      trap_Cvar_VariableStringBuffer("ui_writeconfig_name", buf, sizeof(buf));
      trap_Cmd_ExecuteText(EXEC_NOW, va("writeconfig %s", buf));

      return;
    }

    if (!Q_stricmp(name, "resetColorPickerState")) {
      DC->resetColorPickerState();
      return;
    }

    if (!Q_stricmp(name, "toggleColorPickerRGBSliders")) {
      DC->toggleRGBSliderValues();
      return;
    }

    if (!Q_stricmp(name, "loadCustomvotes")) {
      // don't re-request custom votes if we already have everything processed
      if (static_cast<int>(uiInfo.customVotes.size()) !=
          uiInfo.numCustomvotes) {
        uiInfo.customVotes.clear();
        // cgame handles this as a console command and sends request to qagame
        trap_Cmd_ExecuteText(EXEC_APPEND, "uiRequestCustomvotes");
      }

      Menu_SetFeederSelection(nullptr, FEEDER_CUSTOMVOTES, 0, nullptr);
      return;
    }

    if (!Q_stricmp(name, "voteCustomvote")) {
      if (uiInfo.customvoteIndex < 0 ||
          uiInfo.customvoteIndex > uiInfo.numCustomvotes) {
        return;
      }

      const std::string list = uiInfo.customVotes[uiInfo.customvoteIndex].type;
      trap_Cmd_ExecuteText(EXEC_APPEND,
                           va("callvote %s %s\n",
                              ui_voteCustomRTV.integer ? "rtv" : "randommap",
                              list.c_str()));
      return;
    }

    if (!Q_stricmp(name, "resetCustomvoteDetailsIndex")) {
      Menu_SetFeederSelection(nullptr, FEEDER_CUSTOMVOTES_MAPS_ONSERVER, 0,
                              nullptr);
      Menu_SetFeederSelection(nullptr, FEEDER_CUSTOMVOTES_MAPS_UNAVAILABLE, 0,
                              nullptr);
      return;
    }

    Com_Printf("^3WARNING: unknown UI script %s\n", name);
  }
}

static void UI_GetTeamColor(vec4_t *color) {}

/*
==================
UI_MapCountByGameType
==================
*/
static int UI_MapCountByGameType() {
  int i, c, game;
  c = 0;
  game = ui_netGameType.integer;

  for (i = 0; i < uiInfo.mapCount; i++) {
    uiInfo.mapList[i].active = qfalse;
    if (uiInfo.mapList[i].typeBits & (1 << game)) {
      c++;
      uiInfo.mapList[i].active = qtrue;
    }
  }
  return c;
}

/*
==================
UI_InsertServerIntoDisplayList
==================
*/
static void UI_InsertServerIntoDisplayList(int num, int position) {
  int i;

  if (position < 0 || position > uiInfo.serverStatus.numDisplayServers) {
    return;
  }
  //
  uiInfo.serverStatus.numDisplayServers++;
  for (i = uiInfo.serverStatus.numDisplayServers; i > position; i--) {
    uiInfo.serverStatus.displayServers[i] =
        uiInfo.serverStatus.displayServers[i - 1];
  }
  uiInfo.serverStatus.displayServers[position] = num;
}

/*
==================
UI_RemoveServerFromDisplayList
==================
*/
static void UI_RemoveServerFromDisplayList(int num) {
  int i, j;

  for (i = 0; i < uiInfo.serverStatus.numDisplayServers; i++) {
    if (uiInfo.serverStatus.displayServers[i] == num) {
      uiInfo.serverStatus.numDisplayServers--;
      for (j = i; j < uiInfo.serverStatus.numDisplayServers; j++) {
        uiInfo.serverStatus.displayServers[j] =
            uiInfo.serverStatus.displayServers[j + 1];
      }
      return;
    }
  }
}

/*
==================
UI_BinaryServerInsertion
==================
*/
static void UI_BinaryServerInsertion(int num) {
  int mid, offset, res, len;

  // use binary search to insert server
  len = uiInfo.serverStatus.numDisplayServers;
  mid = len;
  offset = 0;
  res = 0;
  while (mid > 0) {
    mid = len >> 1;
    //
    res = trap_LAN_CompareServers(
        ui_netSource.integer, uiInfo.serverStatus.sortKey,
        uiInfo.serverStatus.sortDir, num,
        uiInfo.serverStatus.displayServers[offset + mid]);
    // if equal
    if (res == 0) {
      UI_InsertServerIntoDisplayList(num, offset + mid);
      return;
    }
    // if larger
    else if (res == 1) {
      offset += mid;
      len -= mid;
    }
    // if smaller
    else {
      len -= mid;
    }
  }
  if (res == 1) {
    offset++;
  }
  UI_InsertServerIntoDisplayList(num, offset);
}

/*
==================
UI_BuildServerDisplayList
==================
*/
static void UI_BuildServerDisplayList(int force) {
  int i, count, clients, maxClients, ping, game, len, friendlyFire, maxlives,
      punkbuster, antilag, password, weaponrestricted, balancedteams;
  char info[MAX_STRING_CHARS];
  // qboolean startRefresh = qtrue; // TTimo: unused

  game = 0; // NERVE - SMF - shut up compiler warning

  if (!(force ||
        uiInfo.uiDC.realTime > uiInfo.serverStatus.nextDisplayRefresh)) {
    return;
  }

  // if we shouldn't reset
  if (force == 2) {
    force = qfalse;
  }

  // do motd updates here too
  trap_Cvar_VariableStringBuffer("cl_motdString", uiInfo.serverStatus.motd,
                                 sizeof(uiInfo.serverStatus.motd));
  len = static_cast<int>(strlen(uiInfo.serverStatus.motd));
  if (len == 0) {
    Q_strncpyz(uiInfo.serverStatus.motd,
               va("Enemy Territory - Version: %s", Q3_VERSION),
               sizeof(uiInfo.serverStatus.motd));
    len = static_cast<int>(strlen(uiInfo.serverStatus.motd));
  }
  if (len != uiInfo.serverStatus.motdLen) {
    uiInfo.serverStatus.motdLen = len;
    uiInfo.serverStatus.motdWidth = -1;
  }

  if (force) {
    // clear number of displayed servers
    uiInfo.serverStatus.numDisplayServers = 0;
    uiInfo.serverStatus.numPlayersOnServers = 0;
    // set list box index to zero
    Menu_SetFeederSelection(nullptr, FEEDER_SERVERS, 0, nullptr);
    // mark all servers as visible, so we store ping updates for them
    trap_LAN_MarkServerVisible(ui_netSource.integer, -1, qtrue);
  }

  // get the server count (comes from the master)
  count = trap_LAN_GetServerCount(ui_netSource.integer);
  if (count == -1 || (ui_netSource.integer == AS_LOCAL && count == 0)) {
    // still waiting on a response from the master
    uiInfo.serverStatus.numDisplayServers = 0;
    uiInfo.serverStatus.numPlayersOnServers = 0;
    uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 500;
    uiInfo.serverStatus.currentServerPreview = 0;
    return;
  }

  if (!uiInfo.serverStatus.numDisplayServers) {
    uiInfo.serverStatus.currentServerPreview = 0;
  }

  for (i = 0; i < count; i++) {
    // if we already got info for this server
    if (!trap_LAN_ServerIsVisible(ui_netSource.integer, i)) {
      continue;
    }
    // get the ping for this server
    ping = trap_LAN_GetServerPing(ui_netSource.integer, i);
    if (ping > 0 || ui_netSource.integer == AS_FAVORITES) {

      trap_LAN_GetServerInfo(ui_netSource.integer, i, info, MAX_STRING_CHARS);

      clients = Q_atoi(Info_ValueForKey(info, "clients"));
      uiInfo.serverStatus.numPlayersOnServers += clients;

      trap_Cvar_Update(&ui_browserShowEmptyOrFull);
      if (ui_browserShowEmptyOrFull.integer) {
        maxClients = Q_atoi(Info_ValueForKey(info, "sv_maxclients"));

        if (clients != maxClients &&
            ((!clients && ui_browserShowEmptyOrFull.integer == 2) ||
             (clients && ui_browserShowEmptyOrFull.integer == 1))) {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }

        if (clients && ((clients == maxClients &&
                         ui_browserShowEmptyOrFull.integer == 2) ||
                        (clients != maxClients &&
                         ui_browserShowEmptyOrFull.integer == 1))) {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      trap_Cvar_Update(&ui_browserShowPasswordProtected);
      if (ui_browserShowPasswordProtected.integer) {
        password = Q_atoi(Info_ValueForKey(info, "needpass"));
        if ((password && ui_browserShowPasswordProtected.integer == 2) ||
            (!password && ui_browserShowPasswordProtected.integer == 1)) {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      trap_Cvar_Update(&ui_browserShowFriendlyFire);
      if (ui_browserShowFriendlyFire.integer) {
        friendlyFire = Q_atoi(Info_ValueForKey(info, "friendlyFire"));

        if ((friendlyFire && ui_browserShowFriendlyFire.integer == 2) ||
            (!friendlyFire && ui_browserShowFriendlyFire.integer == 1)) {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      trap_Cvar_Update(&ui_browserShowMaxlives);
      if (ui_browserShowMaxlives.integer) {
        maxlives = Q_atoi(Info_ValueForKey(info, "maxlives"));
        if ((maxlives && ui_browserShowMaxlives.integer == 2) ||
            (!maxlives && ui_browserShowMaxlives.integer == 1)) {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      trap_Cvar_Update(&ui_browserShowPunkBuster);
      if (ui_browserShowPunkBuster.integer) {
        punkbuster = Q_atoi(Info_ValueForKey(info, "punkbuster"));

        if ((punkbuster && ui_browserShowPunkBuster.integer == 2) ||
            (!punkbuster && ui_browserShowPunkBuster.integer == 1)) {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      trap_Cvar_Update(&ui_browserShowAntilag);
      if (ui_browserShowAntilag.integer) {
        antilag = Q_atoi(Info_ValueForKey(info, "g_antilag"));

        if ((antilag && ui_browserShowAntilag.integer == 2) ||
            (!antilag && ui_browserShowAntilag.integer == 1)) {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      trap_Cvar_Update(&ui_browserShowWeaponsRestricted);
      if (ui_browserShowWeaponsRestricted.integer) {
        weaponrestricted = Q_atoi(Info_ValueForKey(info, "weaprestrict"));

        if ((weaponrestricted != 100 &&
             ui_browserShowWeaponsRestricted.integer == 2) ||
            (weaponrestricted == 100 &&
             ui_browserShowWeaponsRestricted.integer == 1)) {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      trap_Cvar_Update(&ui_browserShowTeamBalanced);
      if (ui_browserShowTeamBalanced.integer) {
        balancedteams = Q_atoi(Info_ValueForKey(info, "balancedteams"));

        if ((balancedteams && ui_browserShowTeamBalanced.integer == 2) ||
            (!balancedteams && ui_browserShowTeamBalanced.integer == 1)) {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      trap_Cvar_Update(&ui_joinGameType);
      if (ui_joinGameType.integer != -1) {
        game = Q_atoi(Info_ValueForKey(info, "gametype"));
        if (game != ui_joinGameType.integer) {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      trap_Cvar_Update(&ui_browserShowETJump);
      if (ui_browserShowETJump.integer > 0) {
        bool isETJump =
            Q_stricmpn(Info_ValueForKey(info, "game"), GAME_NAME, 6) == 0;
        if ((isETJump && ui_browserShowETJump.integer == 2) ||
            (!isETJump && ui_browserShowETJump.integer == 1)) {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      // make sure we never add a favorite server twice
      if (ui_netSource.integer == AS_FAVORITES) {
        UI_RemoveServerFromDisplayList(i);
      }
      // insert the server into the list
      if (uiInfo.serverStatus.numDisplayServers == 0) {
        char *s = Info_ValueForKey(info, "mapname");

        if (s && *s) {
          uiInfo.serverStatus.currentServerPreview = trap_R_RegisterShaderNoMip(
              va("levelshots/%s", Info_ValueForKey(info, "mapname")));
        } else {
          uiInfo.serverStatus.currentServerPreview =
              trap_R_RegisterShaderNoMip("levelshots/unknownmap");
        }
      }

      UI_BinaryServerInsertion(i);
      // done with this server
      if (ping > 0) {
        trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
      }
    }
  }

  uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime;
}

typedef struct {
  const char *name, *altName;
} serverStatusCvar_t;

serverStatusCvar_t serverStatusCvars[] = {
    {"sv_hostname", "Name"},   {"Address", ""},
    {"gamename", "Game name"}, {"g_gametype", "Game type"},
    {"mapname", "Map"},        {"version", ""},
    {"protocol", ""},          {"timelimit", ""},
    {"fraglimit", ""},         {NULL, NULL}};

/*
==================
UI_SortServerStatusInfo
==================
*/
static void UI_SortServerStatusInfo(serverStatusInfo_t *info) {
  int i, j, index;
  const char *tmp1, *tmp2;

  // FIXME: if "gamename" == "baseq3" or "missionpack" then
  // replace the gametype number by FFA, CTF etc.
  //
  index = 0;
  for (i = 0; serverStatusCvars[i].name; i++) {
    for (j = 0; j < info->numLines; j++) {
      if (!info->lines[j][1] || info->lines[j][1][0]) {
        continue;
      }
      if (!Q_stricmp(serverStatusCvars[i].name, info->lines[j][0])) {
        // swap lines
        tmp1 = info->lines[index][0];
        tmp2 = info->lines[index][3];
        info->lines[index][0] = info->lines[j][0];
        info->lines[index][3] = info->lines[j][3];
        info->lines[j][0] = tmp1;
        info->lines[j][3] = tmp2;
        //
        if (strlen(serverStatusCvars[i].altName)) {
          info->lines[index][0] = serverStatusCvars[i].altName;
        }
        index++;
      }
    }
  }
}

/*
==================
UI_GetServerStatusInfo
==================
*/
static int UI_GetServerStatusInfo(const char *serverAddress,
                                  serverStatusInfo_t *info) {
  char *p, *score, *ping, *name, *p_val = nullptr, *p_name = nullptr;
  menuDef_t *menu, *menu2; // we use the URL buttons in several menus
  int i, len;

  if (!info) {
    trap_LAN_ServerStatus(serverAddress, nullptr, 0);
    return qfalse;
  }
  memset(info, 0, sizeof(*info));
  if (trap_LAN_ServerStatus(serverAddress, info->text, sizeof(info->text))) {

    menu = Menus_FindByName("serverinfo_popmenu");
    menu2 = Menus_FindByName("popupError");

    Q_strncpyz(info->address, serverAddress, sizeof(info->address));
    p = info->text;
    info->numLines = 0;
    info->lines[info->numLines][0] = "Address";
    info->lines[info->numLines][1] = "";
    info->lines[info->numLines][2] = "";
    info->lines[info->numLines][3] = info->address;
    info->numLines++;
    // cleanup of the URL cvars
    trap_Cvar_Set("ui_URL", "");
    trap_Cvar_Set("ui_modURL", "");
    // get the cvars
    while (p && *p) {
      p = strchr(p, '\\');
      *p++ = '\0';
      if (p_name) {
        if (!Q_stricmp(p_name, "url")) {
          trap_Cvar_Set("ui_URL", p_val);
          if (menu) {
            Menu_ShowItemByName(menu, "serverURL", qtrue);
          }
          if (menu2) {
            Menu_ShowItemByName(menu2, "serverURL", qtrue);
          }
        } else if (!Q_stricmp(p_name, "mod_url")) {
          trap_Cvar_Set("ui_modURL", p_val);
          if (menu) {
            Menu_ShowItemByName(menu, "modURL", qtrue);
          }
          if (menu2) {
            Menu_ShowItemByName(menu2, "modURL", qtrue);
          }
        }
      }
      if (*p == '\\') {
        break;
      }
      p_name = p;
      info->lines[info->numLines][0] = p;
      info->lines[info->numLines][1] = "";
      info->lines[info->numLines][2] = "";
      p = strchr(p, '\\');
      if (!p) {
        break;
      }
      *p++ = '\0';
      p_val = p;
      info->lines[info->numLines][3] = p;

      info->numLines++;
      if (info->numLines >= MAX_SERVERSTATUS_LINES) {
        break;
      }
    }
    // get the player list
    if (info->numLines < MAX_SERVERSTATUS_LINES - 3) {
      // empty line
      info->lines[info->numLines][0] = "";
      info->lines[info->numLines][1] = "";
      info->lines[info->numLines][2] = "";
      info->lines[info->numLines][3] = "";
      info->numLines++;
      // header
      info->lines[info->numLines][0] = "num";
      info->lines[info->numLines][1] = "score";
      info->lines[info->numLines][2] = "ping";
      info->lines[info->numLines][3] = "name";
      info->numLines++;
      // parse players
      i = 0;
      len = 0;
      while (p && *p) {
        if (*p == '\\') {
          *p++ = '\0';
        }
        if (!p) {
          break;
        }
        score = p;
        p = strchr(p, ' ');
        if (!p) {
          break;
        }
        *p++ = '\0';
        ping = p;
        p = strchr(p, ' ');
        if (!p) {
          break;
        }
        *p++ = '\0';
        name = p;
        Com_sprintf(&info->pings[len],
                    static_cast<int>(sizeof(info->pings)) - len, "%d", i);
        info->lines[info->numLines][0] = &info->pings[len];
        len += static_cast<int>(strlen(&info->pings[len])) + 1;
        info->lines[info->numLines][1] = score;
        info->lines[info->numLines][2] = ping;
        info->lines[info->numLines][3] = name;
        info->numLines++;
        if (info->numLines >= MAX_SERVERSTATUS_LINES) {
          break;
        }
        p = strchr(p, '\\');
        if (!p) {
          break;
        }
        *p++ = '\0';
        //
        i++;
      }
    }
    UI_SortServerStatusInfo(info);
    return qtrue;
  }
  return qfalse;
}

/*
==================
stristr
==================
*/
static char *stristr(char *str, char *charset) {
  int i;

  while (*str) {
    for (i = 0; charset[i] && str[i]; i++) {
      if (toupper(charset[i]) != toupper(str[i])) {
        break;
      }
    }
    if (!charset[i]) {
      return str;
    }
    str++;
  }
  return NULL;
}

/*
==================
UI_BuildFindPlayerList
==================
*/
static void UI_BuildFindPlayerList(qboolean force) {
  static int numFound;
  int i, j, resend;
  serverStatusInfo_t info;
  char name[MAX_NAME_LENGTH + 2];
  char infoString[MAX_STRING_CHARS];

  if (!force) {
    if (!uiInfo.nextFindPlayerRefresh ||
        uiInfo.nextFindPlayerRefresh > uiInfo.uiDC.realTime) {
      return;
    }
  } else {
    memset(&uiInfo.pendingServerStatus, 0, sizeof(uiInfo.pendingServerStatus));
    uiInfo.numFoundPlayerServers = 0;
    uiInfo.currentFoundPlayerServer = 0;
    trap_Cvar_VariableStringBuffer("ui_findPlayer", uiInfo.findPlayerName,
                                   sizeof(uiInfo.findPlayerName));
    Q_CleanStr(uiInfo.findPlayerName);
    // should have a string of some length
    if (!strlen(uiInfo.findPlayerName)) {
      uiInfo.nextFindPlayerRefresh = 0;
      return;
    }
    // set resend time
    resend = ui_serverStatusTimeOut.integer / 2 - 10;
    if (resend < 50) {
      resend = 50;
    }
    trap_Cvar_Set("cl_serverStatusResendTime", va("%d", resend));
    // reset all server status requests
    trap_LAN_ServerStatus(NULL, NULL, 0);
    //
    uiInfo.numFoundPlayerServers = 1;
    Com_sprintf(
        uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
        sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1]),
        "searching %d...", uiInfo.pendingServerStatus.num);
    numFound = 0;
  }
  for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
    // if this pending server is valid
    if (uiInfo.pendingServerStatus.server[i].valid) {
      // try to get the server status for this server
      if (UI_GetServerStatusInfo(uiInfo.pendingServerStatus.server[i].adrstr,
                                 &info)) {
        //
        numFound++;
        // parse through the server status lines
        for (j = 0; j < info.numLines; j++) {
          // should have ping info
          if (!info.lines[j][2] || !info.lines[j][2][0]) {
            continue;
          }
          // clean string first
          Q_strncpyz(name, info.lines[j][3], sizeof(name));
          Q_CleanStr(name);
          // if the player name is a
          // substring
          if (stristr(name, uiInfo.findPlayerName)) {
            // add to found
            // server list if we
            // have space
            // (always leave
            // space for a line
            // with the number
            // found)
            if (uiInfo.numFoundPlayerServers < MAX_FOUNDPLAYER_SERVERS - 1) {
              //
              Q_strncpyz(
                  uiInfo
                      .foundPlayerServerAddresses[uiInfo.numFoundPlayerServers -
                                                  1],
                  uiInfo.pendingServerStatus.server[i].adrstr,
                  sizeof(uiInfo.foundPlayerServerAddresses[0]));
              Q_strncpyz(
                  uiInfo
                      .foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
                  uiInfo.pendingServerStatus.server[i].name,
                  sizeof(uiInfo.foundPlayerServerNames[0]));
              uiInfo.numFoundPlayerServers++;
            } else {
              // can't
              // add any
              // more so
              // we're
              // done
              uiInfo.pendingServerStatus.num =
                  uiInfo.serverStatus.numDisplayServers;
            }
          }
        }
        Com_sprintf(
            uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
            sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers -
                                                 1]),
            "searching %d/%d...", uiInfo.pendingServerStatus.num, numFound);
        // retrieved the server status so reuse
        // this spot
        uiInfo.pendingServerStatus.server[i].valid = qfalse;
      }
    }
    // if empty pending slot or timed out
    if (!uiInfo.pendingServerStatus.server[i].valid ||
        uiInfo.pendingServerStatus.server[i].startTime <
            uiInfo.uiDC.realTime - ui_serverStatusTimeOut.integer) {
      // reset server status request for this address
      UI_GetServerStatusInfo(uiInfo.pendingServerStatus.server[i].adrstr, NULL);
      // reuse pending slot
      uiInfo.pendingServerStatus.server[i].valid = qfalse;
      // if we didn't try to get the status of all
      // servers in the main browser yet
      if (uiInfo.pendingServerStatus.num <
          uiInfo.serverStatus.numDisplayServers) {
        uiInfo.pendingServerStatus.server[i].startTime = uiInfo.uiDC.realTime;
        trap_LAN_GetServerAddressString(
            ui_netSource.integer,
            uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num],
            uiInfo.pendingServerStatus.server[i].adrstr,
            sizeof(uiInfo.pendingServerStatus.server[i].adrstr));
        trap_LAN_GetServerInfo(
            ui_netSource.integer,
            uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num],
            infoString, sizeof(infoString));
        Q_strncpyz(uiInfo.pendingServerStatus.server[i].name,
                   Info_ValueForKey(infoString, "hostname"),
                   sizeof(uiInfo.pendingServerStatus.server[0].name));
        uiInfo.pendingServerStatus.server[i].valid = qtrue;
        uiInfo.pendingServerStatus.num++;
        Com_sprintf(
            uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
            sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers -
                                                 1]),
            "searching %d/%d...", uiInfo.pendingServerStatus.num, numFound);
      }
    }
  }
  for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
    if (uiInfo.pendingServerStatus.server[i].valid) {
      break;
    }
  }
  // if still trying to retrieve server status info
  if (i < MAX_SERVERSTATUSREQUESTS) {
    uiInfo.nextFindPlayerRefresh = uiInfo.uiDC.realTime + 25;
  } else {
    // add a line that shows the number of servers found
    if (!uiInfo.numFoundPlayerServers) {
      Com_sprintf(
          uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
          sizeof(uiInfo.foundPlayerServerAddresses[0]), "no servers found");
    } else {
      Com_sprintf(
          uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
          sizeof(uiInfo.foundPlayerServerAddresses[0]),
          "%d server%s found with player %s", uiInfo.numFoundPlayerServers - 1,
          uiInfo.numFoundPlayerServers == 2 ? "" : "s", uiInfo.findPlayerName);
    }
    uiInfo.nextFindPlayerRefresh = 0;
    // show the server status info for the selected server
    UI_FeederSelection(FEEDER_FINDPLAYER, uiInfo.currentFoundPlayerServer);
  }
}

/*
==================
UI_BuildServerStatus
==================
*/
static void UI_BuildServerStatus(qboolean force) {
  menuDef_t *menu;
  uiClientState_t cstate;
  trap_GetClientState(&cstate);

  if (uiInfo.nextFindPlayerRefresh) {
    return;
  }
  if (!force) {
    if (!uiInfo.nextServerStatusRefresh ||
        uiInfo.nextServerStatusRefresh > uiInfo.uiDC.realTime) {
      return;
    }
  } else {
    Menu_SetFeederSelection(NULL, FEEDER_SERVERSTATUS, 0, NULL);
    uiInfo.serverStatusInfo.numLines = 0;
    // TTimo - reset the server URL / mod URL till we get the
    // new ones the URL buttons are used in the two menus,
    // serverinfo_popmenu and error_popmenu_diagnose
    menu = Menus_FindByName("serverinfo_popmenu");
    if (menu) {
      Menu_ShowItemByName(menu, "serverURL", qfalse);
      Menu_ShowItemByName(menu, "modURL", qfalse);
    }
    menu = Menus_FindByName("popupError");
    if (menu) {
      Menu_ShowItemByName(menu, "serverURL", qfalse);
      Menu_ShowItemByName(menu, "modURL", qfalse);
    }
    // reset all server status requests
    trap_LAN_ServerStatus(NULL, NULL, 0);
  }
  if (cstate.connState < CA_CONNECTED) {
    if (uiInfo.serverStatus.currentServer < 0 ||
        uiInfo.serverStatus.currentServer >
            uiInfo.serverStatus.numDisplayServers ||
        uiInfo.serverStatus.numDisplayServers == 0) {
      return;
    }
  }
  if (UI_GetServerStatusInfo(uiInfo.serverStatusAddress,
                             &uiInfo.serverStatusInfo)) {
    uiInfo.nextServerStatusRefresh = 0;
    UI_GetServerStatusInfo(uiInfo.serverStatusAddress, NULL);
  } else {
    uiInfo.nextServerStatusRefresh = uiInfo.uiDC.realTime + 500;
  }
}

/*
==================
UI_FeederCount
==================
*/
static int UI_FeederCount(float feederID) {
  if (feederID == FEEDER_HEADS) {
    return uiInfo.characterCount;
  } else if (feederID == FEEDER_Q3HEADS) {
    return uiInfo.q3HeadCount;
  } else if (feederID == FEEDER_CINEMATICS) {
    return uiInfo.movieCount;
  } else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS) {
    return UI_MapCountByGameType();
  } else if (feederID == FEEDER_GLINFO) {
    return uiInfo.numGlInfoLines;
  } else if (feederID == FEEDER_PROFILES) {
    return uiInfo.profileCount;
  } else if (feederID == FEEDER_SERVERS) {
    return uiInfo.serverStatus.numDisplayServers;
  } else if (feederID == FEEDER_SERVERSTATUS) {
    return uiInfo.serverStatusInfo.numLines;
  } else if (feederID == FEEDER_FINDPLAYER) {
    return uiInfo.numFoundPlayerServers;
  } else if (feederID == FEEDER_PLAYER_LIST) {
    if (uiInfo.uiDC.realTime > uiInfo.playerRefresh) {
      uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
      UI_BuildPlayerList();
    }
    return uiInfo.playerCount;
  } else if (feederID == FEEDER_TEAM_LIST) {
    if (uiInfo.uiDC.realTime > uiInfo.playerRefresh) {
      uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
      UI_BuildPlayerList();
    }
    return uiInfo.myTeamCount;
  } else if (feederID == FEEDER_MODS) {
    return uiInfo.modCount;
  } else if (feederID == FEEDER_DEMOS) {
    return static_cast<int>(uiInfo.demoObjects.size());
  } else if (feederID == FEEDER_CUSTOMVOTES) {
    if (static_cast<int>(uiInfo.customVotes.size()) != uiInfo.numCustomvotes) {
      return 1;
    } else {
      return uiInfo.numCustomvotes;
    }
  } else if (feederID == FEEDER_CUSTOMVOTES_MAPS_ONSERVER) {
    return static_cast<int>(
        uiInfo.customVotes[uiInfo.customvoteIndex].mapsOnServer.size());
  } else if (feederID == FEEDER_CUSTOMVOTES_MAPS_UNAVAILABLE) {
    return static_cast<int>(
        uiInfo.customVotes[uiInfo.customvoteIndex].otherMaps.size());
  }
  return 0;
}

static const char *UI_SelectedMap(int index, int *actual) {
  int i, c;
  c = 0;
  *actual = 0;

  for (i = 0; i < uiInfo.mapCount; i++) {
    if (uiInfo.mapList[i].active) {
      if (c == index) {
        *actual = i;
        return uiInfo.mapList[i].mapName;
      } else {
        c++;
      }
    }
  }

  return "";
}

static void UI_UpdatePendingPings() {
  trap_LAN_ResetPings(ui_netSource.integer);
  uiInfo.serverStatus.refreshActive = qtrue;
  uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
}

// NERVE - SMF
static void UI_FeederAddItem(float feederID, const char *name, int index) {}
// -NERVE - SMF

//----(SA)	added (whoops, this got nuked in a check-in...)
static const char *UI_FileText(char *fileName) {
  int len;
  fileHandle_t f;
  static char buf[MAX_MENUDEFFILE];

  //	return "flubber";

  len = trap_FS_FOpenFile(fileName, &f, FS_READ);
  if (!f) {
    return NULL;
  }

  trap_FS_Read(buf, len, f);
  buf[len] = 0;
  trap_FS_FCloseFile(f);
  return &buf[0];
}
//----(SA)	end

static char *trimLeadingWiteSpace(char *str) {
  while (isspace((unsigned char)*str)) str++;

  return str;
}

const char *UI_FeederItemText(float feederID, int index, int column,
                              qhandle_t *handles, int *numhandles) {
  static char info[MAX_STRING_CHARS];
  static char hostname[1024];
  static char clientBuff[32];
  static char pingstr[10];
  static int lastColumn = -1;
  static int lastTime = 0;
  *numhandles = 0;
  if (feederID == FEEDER_HEADS) {
    if (index >= 0 && index < uiInfo.characterCount) {
      return uiInfo.characterList[index].name;
    }
  } else if (feederID == FEEDER_Q3HEADS) {
    if (index >= 0 && index < uiInfo.q3HeadCount) {
      return uiInfo.q3HeadNames[index];
    }
  } else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS) {
    int actual;
    return UI_SelectedMap(index, &actual);
  } else if (feederID == FEEDER_GLINFO) {
    if (index == 0) {
      return (va("Vendor: %s", uiInfo.uiDC.glconfig.vendor_string));
    } else if (index == 1) {
      return va("Version: %s: %s", uiInfo.uiDC.glconfig.version_string,
                uiInfo.uiDC.glconfig.renderer_string);
    } else if (index == 2) {
      return va("Pixelformat: color(%d-bits) "
                "Z(%d-bits) stencil(%d-bits)",
                uiInfo.uiDC.glconfig.colorBits, uiInfo.uiDC.glconfig.depthBits,
                uiInfo.uiDC.glconfig.stencilBits);
    } else if (index >= 4 && index < uiInfo.numGlInfoLines) {
      return uiInfo.glInfoLines[index - 4];
    } else {
      return "";
    }
  } else if (feederID == FEEDER_SERVERS) {
    if (index >= 0 && index < uiInfo.serverStatus.numDisplayServers) {
      int ping, game, antilag, needpass, friendlyfire, maxlives, punkbuster,
          weaponrestrictions, balancedteams, serverload;
      if (lastColumn != column || lastTime > uiInfo.uiDC.realTime + 5000) {
        trap_LAN_GetServerInfo(ui_netSource.integer,
                               uiInfo.serverStatus.displayServers[index], info,
                               MAX_STRING_CHARS);
        lastColumn = column;
        lastTime = uiInfo.uiDC.realTime;
      }
      ping = Q_atoi(Info_ValueForKey(info, "ping"));
      if (ping == -1) {
        // if we ever see a ping that is out of
        // date, do a server refresh
        // UI_UpdatePendingPings();
      }
      switch (column) {
        case SORT_HOST:
          // if( ping < 0 ) {
          if (ping <= 0) {
            return Info_ValueForKey(info, "addr");
          } else {
            if (ui_netSource.integer == AS_LOCAL) {
              Com_sprintf(hostname, sizeof(hostname),
                          "%s "
                          "[%"
                          "s]",
                          Info_ValueForKey(info, "hostname"),
                          netnames[Q_atoi(Info_ValueForKey(info, "nettype"))]);
              return hostname;
            } else {
              return trimLeadingWiteSpace(Info_ValueForKey(info, "hostname"));
            }
          }
        case SORT_MAP:
          return Info_ValueForKey(info, "mapname");
        case SORT_CLIENTS:
          Com_sprintf(clientBuff, sizeof(clientBuff), "%s/%s",
                      Info_ValueForKey(info, "clients"),
                      Info_ValueForKey(info, "sv_maxclients"));
          return clientBuff;
        case SORT_GAME:
          game = Q_atoi(Info_ValueForKey(info, "gametype"));
          if (ping > /*=*/0 && game >= 0 && game < uiInfo.numGameTypes) {
            int i;

            // return
            // ETGameTypes[game];
            // return
            // shortETGameTypes[game];

            for (i = 0; i < uiInfo.numGameTypes; i++) {
              if (uiInfo.gameTypes[i].gtEnum == game) {
                return uiInfo.gameTypes[i].gameTypeShort;
              }
            }
          }
          return "???";
        case SORT_PING:
          // if (ping < 0) {
          if (ping <= 0) {
            return "...";
          } else {
            /*antilag =
            Q_atoi(Info_ValueForKey(info,
            "g_antilag")); if
            (!antilag)
                Com_sprintf(
            pingstr,
            sizeof(pingstr),
            "^3%3i", ping );
            else*/

            serverload = Q_atoi(Info_ValueForKey(info, "serverload"));
            if (serverload == -1) {
              Com_sprintf(pingstr, sizeof(pingstr),
                          " %"
                          "3i",
                          ping);
            } else if (serverload > 75) {
              Com_sprintf(pingstr, sizeof(pingstr),
                          "^1 "
                          "%3i",
                          ping);
            } else if (serverload > 40) {
              Com_sprintf(pingstr, sizeof(pingstr),
                          "^3 "
                          "%3i",
                          ping);
            } else {
              Com_sprintf(pingstr, sizeof(pingstr),
                          "^2 "
                          "%3i",
                          ping);
            }
            return pingstr;
          }
        case SORT_FILTERS:
          if (ping <= 0) {
            *numhandles = 0;
            return "";
          } else {
            *numhandles = 7;
            needpass = Q_atoi(Info_ValueForKey(info, "needpass"));
            friendlyfire = Q_atoi(Info_ValueForKey(info, "friendlyFire"));
            maxlives = Q_atoi(Info_ValueForKey(info, "maxlives"));
            punkbuster = Q_atoi(Info_ValueForKey(info, "punkbuster"));
            weaponrestrictions = Q_atoi(Info_ValueForKey(info, "weaprestrict"));
            antilag = Q_atoi(Info_ValueForKey(info, "g_antilag"));
            balancedteams = Q_atoi(Info_ValueForKey(info, "balancedteams"));

            if (needpass) {
              handles[0] = uiInfo.passwordFilter;
            } else {
              handles[0] = -1;
            }
            if (friendlyfire) {
              handles[1] = uiInfo.friendlyFireFilter;
            } else {
              handles[1] = -1;
            }
            if (maxlives) {
              handles[2] = uiInfo.maxLivesFilter;
            } else {
              handles[2] = -1;
            }
            if (punkbuster) {
              handles[3] = uiInfo.punkBusterFilter;
            } else {
              handles[3] = -1;
            }
            if (weaponrestrictions < 100) {
              handles[4] = uiInfo.weaponRestrictionsFilter;
            } else {
              handles[4] = -1;
            }
            if (antilag) {
              handles[5] = uiInfo.antiLagFilter;
            } else {
              handles[5] = -1;
            }
            if (balancedteams) {
              handles[6] = uiInfo.teamBalanceFilter;
            } else {
              handles[6] = -1;
            }

            return "";
          }
        case SORT_FAVOURITES:
          *numhandles = 1;

          if (trap_LAN_ServerIsInFavoriteList(
                  ui_netSource.integer,
                  uiInfo.serverStatus.displayServers[index])) {
            handles[0] = uiInfo.uiDC.Assets.checkboxCheck;
          } else {
            handles[0] = uiInfo.uiDC.Assets.checkboxCheckNot;
          }
          return "";
      }
    }
  } else if (feederID == FEEDER_SERVERSTATUS) {
    if (index >= 0 && index < uiInfo.serverStatusInfo.numLines) {
      if (column >= 0 && column < 4) {
        return uiInfo.serverStatusInfo.lines[index][column];
      }
    }
  } else if (feederID == FEEDER_FINDPLAYER) {
    if (index >= 0 && index < uiInfo.numFoundPlayerServers) {
      // return
      // uiInfo.foundPlayerServerAddresses[index];
      return uiInfo.foundPlayerServerNames[index];
    }
  } else if (feederID == FEEDER_PLAYER_LIST) {
    if (index >= 0 && index < uiInfo.playerCount) {
      return uiInfo.playerNames[index];
    }
  } else if (feederID == FEEDER_TEAM_LIST) {
    if (index >= 0 && index < uiInfo.myTeamCount) {
      return uiInfo.teamNames[index];
    }
  } else if (feederID == FEEDER_MODS) {
    if (index >= 0 && index < uiInfo.modCount) {
      if (uiInfo.modList[index].modDescr && *uiInfo.modList[index].modDescr) {
        return uiInfo.modList[index].modDescr;
      } else {
        return uiInfo.modList[index].modName;
      }
    }
  } else if (feederID == FEEDER_CINEMATICS) {
    if (index >= 0 && index < uiInfo.movieCount) {
      return uiInfo.movieList[index];
    }
  } else if (feederID == FEEDER_DEMOS) {
    if (index >= 0 && index < static_cast<int>(uiInfo.demoObjects.size())) {
      if (uiInfo.demoObjects[index].type == FileSystemObjectType::Folder) {
        *numhandles = 1;

        if (uiInfo.demoObjects[index].name == ".") {
          handles[0] = uiInfo.uiDC.Assets.replayUp;
        } else if (uiInfo.demoObjects[index].name == "..") {
          handles[0] = uiInfo.uiDC.Assets.replayHome;
        } else {
          handles[0] = uiInfo.uiDC.Assets.replayDirectory;
        }
      }

      return uiInfo.demoObjects[index].displayName.c_str();
    }
  } else if (feederID == FEEDER_PROFILES) {
    if (index >= 0 && index < uiInfo.profileCount) {
      char buff[MAX_CVAR_VALUE_STRING];

      Q_strncpyz(buff, uiInfo.profileList[index].name, sizeof(buff));
      Q_CleanStr(buff);
      Q_CleanDirName(buff);

      if (!Q_stricmp(buff, cl_profile.string)) {
        if (!Q_stricmp(buff, cl_defaultProfile.string)) {
          return (va("^7(Default) %s", uiInfo.profileList[index].name));
        } else {
          return (va("^7%s", uiInfo.profileList[index].name));
        }
      } else if (!Q_stricmp(buff, cl_defaultProfile.string)) {
        return (va("(Default) %s", uiInfo.profileList[index].name));
      } else {
        return uiInfo.profileList[index].name;
      }
    }
  } else if (feederID == FEEDER_CUSTOMVOTES) {
    if (static_cast<int>(uiInfo.customVotes.size()) != uiInfo.numCustomvotes) {
      return "Loading...";
    } else {
      return uiInfo.customVotes[index].callvoteText.c_str();
    }
  } else if (feederID == FEEDER_CUSTOMVOTES_MAPS_ONSERVER) {
    if (static_cast<int>(uiInfo.customVotes.size()) != uiInfo.numCustomvotes) {
      return "Loading...";
    } else {
      return uiInfo.customVotes[uiInfo.customvoteIndex]
          .mapsOnServer[index]
          .c_str();
    }
  } else if (feederID == FEEDER_CUSTOMVOTES_MAPS_UNAVAILABLE) {
    if (static_cast<int>(uiInfo.customVotes.size()) != uiInfo.numCustomvotes) {
      return "Loading...";
    } else {
      return uiInfo.customVotes[uiInfo.customvoteIndex]
          .otherMaps[index]
          .c_str();
    }
  }
  // -NERVE - SMF
  return "";
}

static qhandle_t UI_FeederItemImage(float feederID, int index) {
  if (feederID == FEEDER_HEADS) {
    if (index >= 0 && index < uiInfo.characterCount) {
      if (uiInfo.characterList[index].headImage == -1) {
        uiInfo.characterList[index].headImage =
            trap_R_RegisterShaderNoMip(uiInfo.characterList[index].imageName);
      }
      return uiInfo.characterList[index].headImage;
    }
  } else if (feederID == FEEDER_Q3HEADS) {
    if (index >= 0 && index < uiInfo.q3HeadCount) {
      return uiInfo.q3HeadIcons[index];
    }
  } else if (feederID == FEEDER_ALLMAPS || feederID == FEEDER_MAPS) {
    int actual;

    UI_SelectedMap(index, &actual);
    index = actual;

    if (index >= 0 && index < uiInfo.mapCount) {
      if (uiInfo.mapList[index].levelShot == -1) {
        uiInfo.mapList[index].levelShot =
            trap_R_RegisterShaderNoMip(uiInfo.mapList[index].imageName);
      }
      return uiInfo.mapList[index].levelShot;
    }
  }

  return 0;
}

void UI_FeederSelection(float feederID, int index) {
  static char info[MAX_STRING_CHARS];
  if (feederID == FEEDER_HEADS) {
    if (index >= 0 && index < uiInfo.characterCount) {
      trap_Cvar_Set("team_model",
                    uiInfo.characterList[index].female ? "janet" : "james");
      trap_Cvar_Set("team_headmodel",
                    va("*%s", uiInfo.characterList[index].name));
    }
  } else if (feederID == FEEDER_Q3HEADS) {
    if (index >= 0 && index < uiInfo.q3HeadCount) {
      trap_Cvar_Set("model", uiInfo.q3HeadNames[index]);
      trap_Cvar_Set("headmodel", uiInfo.q3HeadNames[index]);
    }
  } else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS) {
    int actual;

    UI_SelectedMap(index, &actual);
    trap_Cvar_Set("ui_mapIndex", va("%d", index));
    ui_mapIndex.integer = index;

    // NERVE - SMF - setup advanced server vars
    if (feederID == FEEDER_ALLMAPS) {
      ui_currentMap.integer = actual;
      trap_Cvar_Set("ui_currentMap", va("%d", actual));
    }
    // -NERVE - SMF

    if (feederID == FEEDER_MAPS) {
      ui_currentMap.integer = actual;
      trap_Cvar_Set("ui_currentMap", va("%d", actual));
    } else {
      ui_currentNetMap.integer = actual;
      trap_Cvar_Set("ui_currentNetMap", va("%d", actual));
    }
  } else if (feederID == FEEDER_SERVERS) {
    const char *mapName = NULL;

    uiInfo.serverStatus.currentServer = index;
    trap_LAN_GetServerInfo(ui_netSource.integer,
                           uiInfo.serverStatus.displayServers[index], info,
                           MAX_STRING_CHARS);

    mapName = Info_ValueForKey(info, "mapname");

    if (mapName && *mapName) {
      uiInfo.serverStatus.currentServerPreview = trap_R_RegisterShaderNoMip(
          va("levelshots/%s", Info_ValueForKey(info, "mapname")));
    } else {
      uiInfo.serverStatus.currentServerPreview =
          trap_R_RegisterShaderNoMip("levelshots/unknownmap");
    }
  } else if (feederID == FEEDER_SERVERSTATUS) {
    //
  } else if (feederID == FEEDER_FINDPLAYER) {
    uiInfo.currentFoundPlayerServer = index;
    //
    if (index < uiInfo.numFoundPlayerServers - 1) {
      // build a new server status for this server
      Q_strncpyz(
          uiInfo.serverStatusAddress,
          uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer],
          sizeof(uiInfo.serverStatusAddress));
      Menu_SetFeederSelection(NULL, FEEDER_SERVERSTATUS, 0, NULL);
      UI_BuildServerStatus(qtrue);
    }
  } else if (feederID == FEEDER_PLAYER_LIST) {
    uiInfo.playerIndex = index;
  } else if (feederID == FEEDER_TEAM_LIST) {
    uiInfo.teamIndex = index;
  } else if (feederID == FEEDER_MODS) {
    uiInfo.modIndex = index;
  } else if (feederID == FEEDER_CINEMATICS) {
    uiInfo.movieIndex = index;
    if (uiInfo.previewMovie >= 0) {
      trap_CIN_StopCinematic(uiInfo.previewMovie);
    }
    uiInfo.previewMovie = -1;
  } else if (feederID == FEEDER_DEMOS) {
    uiInfo.demoIndex = index;
  } else if (feederID == FEEDER_PROFILES) {
    uiInfo.profileIndex = index;
    trap_Cvar_Set("ui_profile", uiInfo.profileList[index].name);
  } else if (feederID == FEEDER_CUSTOMVOTES) {
    uiInfo.customvoteIndex = index;
  } else if (feederID == FEEDER_CUSTOMVOTES_MAPS_ONSERVER) {
    uiInfo.customvoteMapsOnServerIndex = index;
  } else if (feederID == FEEDER_CUSTOMVOTES_MAPS_UNAVAILABLE) {
    uiInfo.customvoteOtherMapsIndex = index;
  }
}

extern void Item_ListBox_MouseEnter(itemDef_t *item, float x, float y,
                                    qboolean click);
qboolean UI_FeederSelectionClick(itemDef_t *item) {
  listBoxDef_t *listPtr = (listBoxDef_t *)item->typeData;

  if (item->special == FEEDER_SERVERS && !Menus_CaptureFuncActive()) {
    // ugly hack for favourites handling
    rectDef_t rect;

    Item_ListBox_MouseEnter(item, DC->cursorx, DC->cursory, qtrue);

    rect.x = item->window.rect.x + listPtr->columnInfo[SORT_FAVOURITES].pos;
    rect.y = item->window.rect.y +
             (listPtr->cursorPos - listPtr->startPos) * listPtr->elementHeight;
    rect.w = listPtr->columnInfo[SORT_FAVOURITES].width;
    rect.h = listPtr->columnInfo[SORT_FAVOURITES].width;

    if (BG_CursorInRect(&rect)) {
      char buff[MAX_STRING_CHARS];
      char addr[MAX_NAME_LENGTH];

      if (trap_LAN_ServerIsInFavoriteList(
              ui_netSource.integer,
              uiInfo.serverStatus.displayServers[listPtr->cursorPos])) {
        trap_LAN_GetServerInfo(
            ui_netSource.integer,
            uiInfo.serverStatus
                .displayServers[uiInfo.serverStatus.currentServer],
            buff, MAX_STRING_CHARS);
        addr[0] = '\0';
        Q_strncpyz(addr, Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
        if (strlen(addr) > 0) {
          trap_LAN_RemoveServer(AS_FAVORITES, addr);
          if (ui_netSource.integer == AS_FAVORITES) {
            UI_BuildServerDisplayList(qtrue);
            UI_FeederSelection(FEEDER_SERVERS, 0);
          }
        }
      } else {
        char name[MAX_NAME_LENGTH];

        trap_LAN_GetServerInfo(
            ui_netSource.integer,
            uiInfo.serverStatus
                .displayServers[uiInfo.serverStatus.currentServer],
            buff, MAX_STRING_CHARS);
        addr[0] = '\0';
        name[0] = '\0';
        Q_strncpyz(addr, Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
        Q_strncpyz(name, Info_ValueForKey(buff, "hostname"), MAX_NAME_LENGTH);
        if (*name && *addr) {
          trap_LAN_AddServer(AS_FAVORITES, name, addr);
        }
      }

      return qtrue;
    }
  }

  return qfalse;
}

/*
// TTimo: unused
static qboolean Team_Parse(char **p) {
  char *token;
  const char *tempStr;
    int i;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{') {
    return qfalse;
  }

  while ( 1 ) {

    token = COM_ParseExt(p, qtrue);

    if (Q_stricmp(token, "}") == 0) {
      return qtrue;
    }

    if ( !token || token[0] == 0 ) {
      return qfalse;
    }

    if (token[0] == '{') {
      // seven tokens per line, team name and icon, and 5 team member names
      if (!String_Parse(p, &uiInfo.teamList[uiInfo.teamCount].teamName) ||
!String_Parse(p, &tempStr)) { return qfalse;
      }


            uiInfo.teamList[uiInfo.teamCount].imageName = tempStr;
        uiInfo.teamList[uiInfo.teamCount].teamIcon =
trap_R_RegisterShaderNoMip(uiInfo.teamList[uiInfo.teamCount].imageName);
          uiInfo.teamList[uiInfo.teamCount].teamIcon_Metal =
trap_R_RegisterShaderNoMip(va("%s_metal",uiInfo.teamList[uiInfo.teamCount].imageName));
            uiInfo.teamList[uiInfo.teamCount].teamIcon_Name =
trap_R_RegisterShaderNoMip(va("%s_name",
uiInfo.teamList[uiInfo.teamCount].imageName));

            uiInfo.teamList[uiInfo.teamCount].cinematic = -1;

            for (i = 0; i < TEAM_MEMBERS; i++) {
                uiInfo.teamList[uiInfo.teamCount].teamMembers[i] = NULL;
                if (!String_Parse(p,
&uiInfo.teamList[uiInfo.teamCount].teamMembers[i])) { return qfalse;
                }
            }

      Com_Printf("Loaded team %s with team icon %s.\n",
uiInfo.teamList[uiInfo.teamCount].teamName, tempStr); if (uiInfo.teamCount <
MAX_TEAMS) { uiInfo.teamCount++; } else { Com_Printf("Too many teams, last team
replaced!\n");
      }
      token = COM_ParseExt(p, qtrue);
      if (token[0] != '}') {
        return qfalse;
      }
    }
  }

  return qfalse;
}
*/

/*
// TTimo: unused
static qboolean Character_Parse(char **p) {
  char *token;
  const char *tempStr;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{') {
    return qfalse;
  }


  while ( 1 ) {
    token = COM_ParseExt(p, qtrue);

    if (Q_stricmp(token, "}") == 0) {
      return qtrue;
    }

    if ( !token || token[0] == 0 ) {
      return qfalse;
    }

    if (token[0] == '{') {
      // two tokens per line, character name and sex
      if (!String_Parse(p, &uiInfo.characterList[uiInfo.characterCount].name) ||
!String_Parse(p, &tempStr)) { return qfalse;
      }

      uiInfo.characterList[uiInfo.characterCount].headImage = -1;
            uiInfo.characterList[uiInfo.characterCount].imageName =
String_Alloc(va("models/players/heads/%s/icon_default.tga",
uiInfo.characterList[uiInfo.characterCount].name));

      if (tempStr && (tempStr[0] == 'f' || tempStr[0] == 'F')) {
        uiInfo.characterList[uiInfo.characterCount].female = qtrue;
      } else {
        uiInfo.characterList[uiInfo.characterCount].female = qfalse;
      }

      Com_Printf("Loaded %s character %s.\n", tempStr,
uiInfo.characterList[uiInfo.characterCount].name); if (uiInfo.characterCount <
MAX_HEADS) { uiInfo.characterCount++; } else { Com_Printf("Too many characters,
last character replaced!\n");
      }

      token = COM_ParseExt(p, qtrue);
      if (token[0] != '}') {
        return qfalse;
      }
    }
  }

  return qfalse;
}
*/

/*
// TTimo: unused
static qboolean Alias_Parse(char **p) {
  char *token;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{') {
    return qfalse;
  }

  while ( 1 ) {
    token = COM_ParseExt(p, qtrue);

    if (Q_stricmp(token, "}") == 0) {
      return qtrue;
    }

    if ( !token || token[0] == 0 ) {
      return qfalse;
    }

    if (token[0] == '{') {
      // three tokens per line, character name, bot alias, and preferred action
a - all purpose, d - defense, o - offense if (!String_Parse(p,
&uiInfo.aliasList[uiInfo.aliasCount].name) || !String_Parse(p,
&uiInfo.aliasList[uiInfo.aliasCount].ai) || !String_Parse(p,
&uiInfo.aliasList[uiInfo.aliasCount].action)) { return qfalse;
      }

      Com_Printf("Loaded character alias %s using character ai %s.\n",
uiInfo.aliasList[uiInfo.aliasCount].name,
uiInfo.aliasList[uiInfo.aliasCount].ai); if (uiInfo.aliasCount < MAX_ALIASES) {
        uiInfo.aliasCount++;
      } else {
        Com_Printf("Too many aliases, last alias replaced!\n");
      }

      token = COM_ParseExt(p, qtrue);
      if (token[0] != '}') {
        return qfalse;
      }
    }
  }

  return qfalse;
}
*/

// mode
// 0 - high level parsing
// 1 - team parsing
// 2 - character parsing
/*
// TTimo: unused
static void UI_ParseTeamInfo(const char *teamFile) {
    char	*token;
  char *p;
  char *buff = NULL;
  //int mode = 0; // TTimo: unused

  buff = GetMenuBuffer(teamFile);
  if (!buff) {
    return;
  }

  p = buff;

    while ( 1 ) {
        token = COM_ParseExt( &p, qtrue );
        if( !token || token[0] == 0 || token[0] == '}') {
            break;
        }

        if ( Q_stricmp( token, "}" ) == 0 ) {
      break;
    }

    if (Q_stricmp(token, "teams") == 0) {

      if (Team_Parse(&p)) {
        continue;
      } else {
        break;
      }
    }

    if (Q_stricmp(token, "characters") == 0) {
      Character_Parse(&p);
    }

    if (Q_stricmp(token, "aliases") == 0) {
      Alias_Parse(&p);
    }
  }
}
*/

/*
==============
GameType_Parse
==============
*/
static qboolean GameType_Parse(const char **p, qboolean join) {
  const char *token;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{') {
    return qfalse;
  }

  if (join) {
    uiInfo.numJoinGameTypes = 0;
  } else {
    uiInfo.numGameTypes = 0;
  }

  while (1) {
    token = COM_ParseExt(p, qtrue);

    if (Q_stricmp(token, "}") == 0) {
      return qtrue;
    }

    if (!token || token[0] == 0) {
      return qfalse;
    }

    if (token[0] == '{') {
      if (join) {
        if (!String_Parse(
                p, &uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gameType) ||
            !Int_Parse(p,
                       &uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gtEnum)) {
          return qfalse;
        }
      } else {
        if (!Int_Parse(p, &uiInfo.gameTypes[uiInfo.numGameTypes].gtEnum)) {
          return qfalse;
        }

        if (!String_Parse(p, &uiInfo.gameTypes[uiInfo.numGameTypes].gameType)) {
          return qfalse;
        }

        if (!String_Parse(
                p, &uiInfo.gameTypes[uiInfo.numGameTypes].gameTypeShort)) {
          return qfalse;
        }

        if (!String_Parse(
                p,
                &uiInfo.gameTypes[uiInfo.numGameTypes].gameTypeDescription)) {
          return qfalse;
        }
      }

      if (join) {
        if (uiInfo.numJoinGameTypes < MAX_GAMETYPES) {
          uiInfo.numJoinGameTypes++;
        } else {
          Com_Printf("Too many net "
                     "game types, last "
                     "one replace!\n");
        }
      } else {
        if (uiInfo.numGameTypes < MAX_GAMETYPES) {
          uiInfo.numGameTypes++;
        } else {
          Com_Printf("Too many game types, "
                     "last one replace!\n");
        }
      }

      token = COM_ParseExt(p, qtrue);
      if (token[0] != '}') {
        return qfalse;
      }
    }
  }
  return qfalse;
}

#if 0
static qboolean MapList_Parse(char **p)
{
	char *token;

	token = COM_ParseExt(p, qtrue);

	if (token[0] != '{')
	{
		return qfalse;
	}

	uiInfo.mapCount = 0;

	while (1)
	{
		token = COM_ParseExt(p, qtrue);

		if (Q_stricmp(token, "}") == 0)
		{
			return qtrue;
		}

		if (!token || token[0] == 0)
		{
			return qfalse;
		}

		if (token[0] == '{')
		{
			if (!String_Parse(p, &uiInfo.mapList[uiInfo.mapCount].mapName) || !String_Parse(p, &uiInfo.mapList[uiInfo.mapCount].mapLoadName)
			    || !Int_Parse(p, &uiInfo.mapList[uiInfo.mapCount].teamMembers))
			{
				return qfalse;
			}

			if (!String_Parse(p, &uiInfo.mapList[uiInfo.mapCount].opponentName))
			{
				return qfalse;
			}

			uiInfo.mapList[uiInfo.mapCount].typeBits = 0;

			while (1)
			{
				token = COM_ParseExt(p, qtrue);
				if (token[0] >= '0' && token[0] <= '9')
				{
					uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << (token[0] - 0x030));
					if (!Int_Parse(p, &uiInfo.mapList[uiInfo.mapCount].timeToBeat[token[0] - 0x30]))
					{
						return qfalse;
					}
				}
				else
				{
					break;
				}
			}

			//mapList[mapCount].imageName = String_Alloc(va("levelshots/%s", mapList[mapCount].mapLoadName));
			//if (uiInfo.mapCount == 0) {
			// only load the first cinematic, selection loads the others
			//  uiInfo.mapList[uiInfo.mapCount].cinematic = trap_CIN_PlayCinematic(va("%s.roq",uiInfo.mapList[uiInfo.mapCount].mapLoadName), qfalse, qfalse, qtrue, 0, 0, 0, 0);
			//}
			uiInfo.mapList[uiInfo.mapCount].cinematic = -1;
			uiInfo.mapList[uiInfo.mapCount].levelShot = trap_R_RegisterShaderNoMip(va("levelshots/%s_small", uiInfo.mapList[uiInfo.mapCount].mapLoadName));

			if (uiInfo.mapCount < MAX_MAPS)
			{
				uiInfo.mapCount++;
			}
			else
			{
				Com_Printf("Too many maps, last one replaced!\n");
			}
		}
	}
	return qfalse;
}
#endif

static void UI_ParseGameInfo(const char *teamFile) {
  const char *token;
  const char *p;
  char *buff = NULL;
  // int mode = 0; // TTimo: unused

  buff = GetMenuBuffer(teamFile);
  if (!buff) {
    return;
  }

  p = buff;

  while (1) {
    token = COM_ParseExt(&p, qtrue);
    if (!token || token[0] == 0 || token[0] == '}') {
      break;
    }

    if (Q_stricmp(token, "}") == 0) {
      break;
    }

    if (Q_stricmp(token, "gametypes") == 0) {

      if (GameType_Parse(&p, qfalse)) {
        continue;
      } else {
        break;
      }
    }

    //		if (Q_stricmp(token, "maps") == 0) {
    // start a new menu
    //			MapList_Parse(&p);
    //		}
  }
}

static void UI_Pause(qboolean b) {
  if (b) {
    // pause the game and set the ui keycatcher
    trap_Cvar_Set("cl_paused", "1");
    trap_Key_SetCatcher(KEYCATCH_UI);
  } else {
    // unpause the game and clear the ui keycatcher
    trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
    trap_Key_ClearStates();
    trap_Cvar_Set("cl_paused", "0");
  }
}

/*
// TTimo: unused
static int UI_OwnerDraw_Width(int ownerDraw) {
  return 0;
}
*/

static int UI_PlayCinematic(const char *name, float x, float y, float w,
                            float h) {
  return trap_CIN_PlayCinematic(name, x, y, w, h, (CIN_loop | CIN_silent));
}

static void UI_StopCinematic(int handle) {
  if (handle >= 0) {
    trap_CIN_StopCinematic(handle);
  } else {
    handle = abs(handle);
    if (handle == UI_MAPCINEMATIC) {
      if (uiInfo.mapList[ui_currentMap.integer].cinematic >= 0) {
        trap_CIN_StopCinematic(uiInfo.mapList[ui_currentMap.integer].cinematic);
        uiInfo.mapList[ui_currentMap.integer].cinematic = -1;
      }
    } else if (handle == UI_NETMAPCINEMATIC) {
      if (uiInfo.serverStatus.currentServerCinematic >= 0) {
        trap_CIN_StopCinematic(uiInfo.serverStatus.currentServerCinematic);
        uiInfo.serverStatus.currentServerCinematic = -1;
      }
    } else if (handle == UI_CLANCINEMATIC) {
      int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
      if (i >= 0 && i < uiInfo.teamCount) {
        if (uiInfo.teamList[i].cinematic >= 0) {
          trap_CIN_StopCinematic(uiInfo.teamList[i].cinematic);
          uiInfo.teamList[i].cinematic = -1;
        }
      }
    }
  }
}

static void UI_DrawCinematic(int handle, float x, float y, float w, float h) {
  trap_CIN_SetExtents(handle, x, y, w, h);
  trap_CIN_DrawCinematic(handle);
}

static void UI_RunCinematicFrame(int handle) { trap_CIN_RunCinematic(handle); }

namespace ETJump {
/*
 * TL;DR - every client is shit and pretends to be something it's not.
 *
 * ET: Legacy sends 3 args to UI_INIT, but 2.60b only sends 1.
 * The idea is that ET: Legacy wants to nag 2.60b users to download ET: Legacy
 * client if legacy mod is loaded with the vanilla client.
 * arg1 is a boolean (is this ET: Legacy client?) and arg2 is an integer
 * representation of current version (for various compatibility checks).
 *
 * ETe also sends 3 args, and fakes being an ET: Legacy client (though for UI,
 * I'm not sure what good this does as it doesn't get around the nagging).
 * What this means for us is that we can't rely on checking the arg1/2
 * to detect if a client is ET: Legacy or not, because ETe pretends
 * that it's ET: Legacy too.
 *
 * Solution? Check the 'version' string that each client sends, right?
 * Right...? Yeah lol, no.
 *
 * We CAN differentiate ETe from 2.60b using the version string, vanilla client
 * is either 'ET 2.60b' or 'ET 2.60d', and ETe is 'ET 2.60e'.
 * So this means we can just check for the version string first,
 * try to match that to ETe, and then fallback to arg1/arg2 for ET: Legacy,
 * and if neither match = vanilla client, right? Nope.
 *
 * The VM_Call argument parsing function in vanilla is not ideal.
 * For every VM_Call, it reads memory as if you're sending the max amount of
 * supported varags. This means that it will *very* likely read and send
 * garbage memory as arg1 and arg2 to UI_Init, which means it can accidentally
 * identify as an ET: Legacy client, because arg1 is treated as boolean.
 *
 * Okay, so just read the version string then and forget arg1/2 completely?
 * Yeah nope, ET: Legacy needs to be special.
 *
 * For some reason it sends a faked 'version' string, pretending
 * to be 2.60b client, presumably to maintain compatibility with some mods.
 * Which is a bit odd as ETPro doesn't run anyway, and no other mod has a
 * client version check that prevents you from playing them on custom clients.
 *
 * There is a saving grace though: ET: Legacy also sends a special 'etVersion'
 * string, along with the regular 'version' string. So this means, we can
 * identify the client by doing the following:
 *
 * 1. parse 'etVersion' string
 * 2. if 'etVersion string is empty, we can parse the 'version' string safely
 * to differentiate between vanilla client and ETe
 * 3. if 'etVersion' string is not empty, we can parse arg1/2 to grab
 * ET: Legacy client version.
 */

static void detectClientEngine(int legacyClient, int clientVersion) {
  char etVersionStr[MAX_CVAR_VALUE_STRING]; // ET: Legacy exclusive
  trap_Cvar_VariableStringBuffer("etVersion", etVersionStr,
                                 sizeof(etVersionStr));

  if (etVersionStr[0] == '\0') {
    char versionStr[MAX_CVAR_VALUE_STRING];
    trap_Cvar_VariableStringBuffer("version", versionStr, sizeof(versionStr));

    // we can use this length for every detection
    const auto len = static_cast<int>(strlen("ET 2.60b"));

    if (!Q_stricmpn(versionStr, "ET 2.60b", len) ||
        !Q_stricmpn(versionStr, "ET 2.60d", len)) {
      uiInfo.vetClient = true;
    } else if (!Q_stricmpn(versionStr, "ET 2.60e", len)) {
      uiInfo.eteClient = true;
    }
  } else {
    MOD_CHECK_ETLEGACY(legacyClient, clientVersion, uiInfo.etLegacyClient);
  }
}
} // namespace ETJump

/*
=================
UI_Init
=================
*/
void _UI_Init(int legacyClient, int clientVersion) {
  int x;

  // uiInfo.inGameLoad = inGameLoad;
  Com_Printf(S_COLOR_LTGREY GAME_HEADER);
  Com_Printf(S_COLOR_LTGREY "____________________________\n");
  Com_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                      " " S_COLOR_LTGREY GAME_BINARY_NAME
                                      " init...\n");

  UI_RegisterCvars();
  UI_InitMemory();
  trap_PC_RemoveAllGlobalDefines();

  trap_Cvar_Set("ui_menuFiles", DEFAULT_MENU_FILE);

  // cache redundant calulations
  trap_GetGlconfig(&uiInfo.uiDC.glconfig);

  UI_ParseGLConfig();

  // for 640x480 virtualized screen
  int width = static_cast<int>(ceil(uiInfo.uiDC.glconfig.vidWidth * 480.0 /
                                    uiInfo.uiDC.glconfig.vidHeight));
  uiInfo.uiDC.screenWidth = width > 640 ? width : 640;
  uiInfo.uiDC.screenHeight = 480;

  uiInfo.uiDC.yscale = uiInfo.uiDC.glconfig.vidHeight / 480.0;
  uiInfo.uiDC.xscale =
      uiInfo.uiDC.glconfig.vidWidth / (float)uiInfo.uiDC.screenWidth;

  if (uiInfo.uiDC.glconfig.vidWidth * 480 >
      uiInfo.uiDC.glconfig.vidHeight * 640) {
    // wide screen
    uiInfo.uiDC.bias =
        0.5 * (uiInfo.uiDC.glconfig.vidWidth -
               (uiInfo.uiDC.glconfig.vidHeight * (640.0 / 480.0)));
  } else {
    // no wide screen
    uiInfo.uiDC.bias = 0;
  }

  ETJump::detectClientEngine(legacyClient, clientVersion);

  // UI_Load();
  uiInfo.uiDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
  uiInfo.uiDC.setColor = &UI_SetColor;
  uiInfo.uiDC.drawHandlePic = &UI_DrawHandlePic;
  uiInfo.uiDC.drawStretchPic = &trap_R_DrawStretchPic;
  uiInfo.uiDC.drawText = &Text_Paint;
  uiInfo.uiDC.drawTextExt = &Text_Paint_Ext;
  uiInfo.uiDC.textWidth = &Text_Width;
  uiInfo.uiDC.textWidthExt = &Text_Width_Ext;
  uiInfo.uiDC.multiLineTextWidth = &Multiline_Text_Width;
  uiInfo.uiDC.textHeight = &Text_Height;
  uiInfo.uiDC.textHeightExt = &Text_Height_Ext;
  uiInfo.uiDC.multiLineTextHeight = &Multiline_Text_Height;
  uiInfo.uiDC.textFont = &Text_SetActiveFont;
  uiInfo.uiDC.registerModel = &trap_R_RegisterModel;
  uiInfo.uiDC.modelBounds = &trap_R_ModelBounds;
  uiInfo.uiDC.fillRect = &UI_FillRect;
  uiInfo.uiDC.drawRect = &_UI_DrawRect;
  uiInfo.uiDC.drawRectFixed = &_UI_DrawRect_DrawRect_FixedBorder;
  uiInfo.uiDC.drawSides = &_UI_DrawSides;
  uiInfo.uiDC.drawTopBottom = &_UI_DrawTopBottom;
  uiInfo.uiDC.drawSidesNoScale = &_UI_DrawSides_NoScale;
  uiInfo.uiDC.drawTopBottomNoScale = &_UI_DrawTopBottom_NoScale;
  uiInfo.uiDC.clearScene = &trap_R_ClearScene;
  uiInfo.uiDC.drawSides = &_UI_DrawSides;
  uiInfo.uiDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
  uiInfo.uiDC.renderScene = &trap_R_RenderScene;
  uiInfo.uiDC.registerFont = &trap_R_RegisterFont;
  uiInfo.uiDC.ownerDrawItem = &UI_OwnerDraw;
  uiInfo.uiDC.getValue = &UI_GetValue;
  uiInfo.uiDC.ownerDrawVisible = &UI_OwnerDrawVisible;
  uiInfo.uiDC.runScript = &UI_RunMenuScript;
  uiInfo.uiDC.getTeamColor = &UI_GetTeamColor;
  uiInfo.uiDC.setCVar = trap_Cvar_Set;
  uiInfo.uiDC.getCVarString = trap_Cvar_VariableStringBuffer;
  uiInfo.uiDC.getCVarValue = trap_Cvar_VariableValue;
  uiInfo.uiDC.drawTextWithCursor = &Text_PaintWithCursor;
  uiInfo.uiDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
  uiInfo.uiDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
  uiInfo.uiDC.startLocalSound = &trap_S_StartLocalSound;
  uiInfo.uiDC.ownerDrawHandleKey = &UI_OwnerDrawHandleKey;
  uiInfo.uiDC.feederCount = &UI_FeederCount;
  uiInfo.uiDC.feederItemImage = &UI_FeederItemImage;
  uiInfo.uiDC.feederItemText = &UI_FeederItemText;
  uiInfo.uiDC.fileText = &UI_FileText; //----(SA)	re-added
  uiInfo.uiDC.feederSelection = &UI_FeederSelection;
  uiInfo.uiDC.feederSelectionClick = &UI_FeederSelectionClick;
  uiInfo.uiDC.feederAddItem = &UI_FeederAddItem; // NERVE - SMF
  uiInfo.uiDC.setBinding = &trap_Key_SetBinding;
  uiInfo.uiDC.getBindingBuf = &trap_Key_GetBindingBuf;
  uiInfo.uiDC.getKeysForBinding = &trap_Key_KeysForBinding;
  uiInfo.uiDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
  uiInfo.uiDC.keyIsDown = &trap_Key_IsDown;
  uiInfo.uiDC.executeText = &trap_Cmd_ExecuteText;
  uiInfo.uiDC.Error = &Com_Error;
  uiInfo.uiDC.Print = &Com_Printf;
  uiInfo.uiDC.Pause = &UI_Pause;
  uiInfo.uiDC.ownerDrawWidth = &UI_OwnerDrawWidth;
  uiInfo.uiDC.registerSound = &trap_S_RegisterSound;
  uiInfo.uiDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
  uiInfo.uiDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
  uiInfo.uiDC.playCinematic = &UI_PlayCinematic;
  uiInfo.uiDC.stopCinematic = &UI_StopCinematic;
  uiInfo.uiDC.drawCinematic = &UI_DrawCinematic;
  uiInfo.uiDC.runCinematicFrame = &UI_RunCinematicFrame;
  uiInfo.uiDC.translateString = &trap_TranslateString; // NERVE - SMF
  uiInfo.uiDC.checkAutoUpdate = &trap_CheckAutoUpdate; // DHM - Nerve
  uiInfo.uiDC.getAutoUpdate = &trap_GetAutoUpdate;     // DHM - Nerve

  uiInfo.uiDC.add2dPolys = &trap_R_Add2dPolys;
  uiInfo.uiDC.updateScreen = &trap_UpdateScreen;
  uiInfo.uiDC.getHunkData = &trap_GetHunkData;
  uiInfo.uiDC.getConfigString = &trap_GetConfigString;
  uiInfo.uiDC.getActiveFont = &GetActiveFont;

  ETJump::initColorPicker();

  Init_Display(&uiInfo.uiDC);

  String_Init();

  uiInfo.uiDC.whiteShader = trap_R_RegisterShaderNoMip("white");

  AssetCache();

  uiInfo.passwordFilter =
      trap_R_RegisterShaderNoMip("ui/assets/filter_pass.tga");
  uiInfo.friendlyFireFilter =
      trap_R_RegisterShaderNoMip("ui/assets/filter_ff.tga");
  uiInfo.maxLivesFilter =
      trap_R_RegisterShaderNoMip("ui/assets/filter_lives.tga");
  uiInfo.punkBusterFilter =
      trap_R_RegisterShaderNoMip("ui/assets/filter_pb.tga");
  uiInfo.weaponRestrictionsFilter =
      trap_R_RegisterShaderNoMip("ui/assets/filter_weap.tga");
  uiInfo.antiLagFilter =
      trap_R_RegisterShaderNoMip("ui/assets/filter_antilag.tga");
  uiInfo.teamBalanceFilter =
      trap_R_RegisterShaderNoMip("ui/assets/filter_balance.tga");

  uiInfo.campaignMap = trap_R_RegisterShaderNoMip("gfx/loading/camp_map.tga");

  trap_R_RegisterFont("ariblk", 27, &uiInfo.loadscreenfont1);
  trap_R_RegisterFont("courbd", 30, &uiInfo.loadscreenfont2);

  uiInfo.teamCount = 0;
  uiInfo.characterCount = 0;
  uiInfo.aliasCount = 0;

  uiInfo.numCustomvotes = -1;

  UI_LoadPanel_Init();

  UI_ParseGameInfo("gameinfo.txt");

  UI_LoadMenus(DEFAULT_MENU_FILE, qfalse);

  Menus_CloseAll();

  trap_LAN_LoadCachedServers();
  UI_LoadBestScores(
      uiInfo.mapList[0].mapLoadName,
      uiInfo
          .gameTypes[Numeric::clamp(ui_gameType.integer, 0, MAX_GAMETYPES - 1)]
          .gtEnum);

  // sets defaults for ui temp cvars
  // rain - bounds check array index, although I'm pretty sure this
  // stuff isn't used anymore...
  x = (int)trap_Cvar_VariableValue("color") - 1;
  if (x < 0 ||
      x >= static_cast<int>(sizeof(gamecodetoui) / sizeof(gamecodetoui[0]))) {
    x = 0;
  }
  uiInfo.effectsColor = gamecodetoui[x];
  uiInfo.currentCrosshair = (int)trap_Cvar_VariableValue("cg_drawCrosshair");
  trap_Cvar_Set("ui_mousePitch",
                (trap_Cvar_VariableValue("m_pitch") >= 0) ? "0" : "1");

  uiInfo.serverStatus.currentServerCinematic = -1;
  uiInfo.previewMovie = -1;

  if (trap_Cvar_VariableValue("ui_TeamArenaFirstRun") == 0) {
    trap_Cvar_Set("s_volume", "0.8");
    trap_Cvar_Set("s_musicvolume", "0.5");
    trap_Cvar_Set("ui_TeamArenaFirstRun", "1");
  }

  trap_Cvar_Register(NULL, "debug_protocol", "", 0);

  // ETJump: default to objective
  trap_Cvar_Set("ui_netGameType", "2");
  trap_Cvar_Update(&ui_netGameType);

  // init Yes/No once for cl_language -> server browser (punkbuster)
  Q_strncpyz(translated_yes, DC->translateString("Yes"),
             sizeof(translated_yes));
  Q_strncpyz(translated_no, DC->translateString("NO"), sizeof(translated_no));

  trap_AddCommand("ui_report");

  Com_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                      " " S_COLOR_LTGREY GAME_BINARY_NAME
                                      " init... " S_COLOR_GREEN "DONE\n");
}

/*
=================
UI_KeyEvent
=================
*/
void _UI_KeyEvent(int key, qboolean down) {
  static qboolean bypassKeyClear = qfalse;

  if (Menu_Count() > 0) {
    menuDef_t *menu = Menu_GetFocused();
    if (menu) {
      if (trap_Cvar_VariableValue("cl_bypassMouseInput")) {
        bypassKeyClear = qtrue;
      }

      //			if (key == K_ESCAPE &&
      // down &&
      //! Menus_AnyFullScreenVisible()) {
      //! Menus_CloseAll(); 			} else {
      //				Menu_HandleKey(menu,
      // key, down );
      //			}
      // always have the menus do the proper handling
      Menu_HandleKey(menu, key, down);
    } else {
      trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);

      // NERVE - SMF - we don't want to clear key states
      // if bypassing input
      if (!bypassKeyClear) {
        trap_Key_ClearStates();
      }

      if (cl_bypassMouseInput.integer) {
        if (!trap_Key_GetCatcher()) {
          trap_Cvar_Set("cl_bypassMouseInput", 0);
        }
      }

      bypassKeyClear = qfalse;

      trap_Cvar_Set("cl_paused", "0");
    }
  }

  // if ((s > 0) && (s != menu_null_sound)) {
  //	trap_S_StartLocalSound( s, CHAN_LOCAL_SOUND );
  // }
}

/*
=================
UI_MouseEvent
=================
*/
void _UI_MouseEvent(int dx, int dy) {
  float mdx, mdy;
  ETJump::scaleMenuSensitivity(dx, dy, &mdx, &mdy);
  dx = static_cast<int>(mdx);
  dy = static_cast<int>(mdy);

  // update mouse screen position
  uiInfo.uiDC.cursorx += dx;
  if (uiInfo.uiDC.cursorx < 0) {
    uiInfo.uiDC.cursorx = 0;
  } else if (uiInfo.uiDC.cursorx > SCREEN_WIDTH) {
    uiInfo.uiDC.cursorx = SCREEN_WIDTH;
  }

  uiInfo.uiDC.cursory += dy;
  if (uiInfo.uiDC.cursory < 0) {
    uiInfo.uiDC.cursory = 0;
  } else if (uiInfo.uiDC.cursory > SCREEN_HEIGHT) {
    uiInfo.uiDC.cursory = SCREEN_HEIGHT;
  }

  if (Menu_Count() > 0) {
    Display_MouseMove(nullptr, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory);
  }
}

//----(SA)	added
static uiMenuCommand_t menutype = UIMENU_NONE;

uiMenuCommand_t _UI_GetActiveMenu(void) { return menutype; }
//----(SA)	end

#define MISSING_FILES_MSG "The following packs are missing:"

/*
 *	We handle here illegal redirects, that happen upon serverlist loading
 *	due to incorrect response recieve for a getstatus request.
 */
namespace ETJump {
void markAllServersVisible() {
  int count = trap_LAN_GetServerCount(ui_netSource.integer);
  for (int i = 0; i < count; i++) {
    trap_LAN_MarkServerVisible(ui_netSource.integer, i, qtrue);
  }
}

void keepServerListUpdating() {
  uiInfo.serverStatus.refreshActive = qtrue;
  uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
}

void openPlayOnlineMenu() {
  Menus_CloseByName("main"); // opened by main_opener after ui reload
  Menus_ActivateByName("playonline", qtrue);
  const char *str = "clearError"; // clears com_errorMessage
  UI_RunMenuScript(&str);
}

void handleIllegalRedirect() {
  Com_Printf("^1ETJump: illegal redirect was detected, reacting...\n");
  markAllServersVisible();
  keepServerListUpdating();
  openPlayOnlineMenu();
}

void parseMaplist() {
  char arg[MAX_QPATH];

  // start iterating from 1 to skip the command string
  for (int i = 1, len = trap_Argc(); i < len; i++) {
    trap_Argv(i, arg, sizeof(arg));
    uiInfo.serverMaplist.emplace_back(arg);
  }

  ETJump::StringUtil::sortStrings(uiInfo.serverMaplist, true);
}

void parseNumCustomvotes() {
  if (trap_Argc() < 2) {
    Com_Printf(va(S_COLOR_YELLOW
                  "%s: unable to parse customvote count: no arguments given.\n",
                  __func__));
    return;
  }

  char arg[MAX_TOKEN_CHARS];
  trap_Argv(1, arg, sizeof(arg));
  uiInfo.numCustomvotes = Q_atoi(arg);
}

void parseCustomvote() {
  // if a mapsOnServer or otherMaps is empty, we only have 3 args
  static constexpr int minArgs = 3;
  const int numArgs = trap_Argc();

  if (numArgs < minArgs) {
    Com_Printf(va(S_COLOR_YELLOW "%s: unable to parse customvote: malformed "
                                 "command - too few arguments (%i < %i).\n",
                  __func__, numArgs, minArgs));
    return;
  }

  char arg[MAX_TOKEN_CHARS];
  CustomMapVotes::MapType *mapType = nullptr;

  trap_Argv(1, arg, sizeof(arg));

  // grab an existing list if we've already parsed this list before
  for (auto &customVote : uiInfo.customVotes) {
    if (customVote.type == arg) {
      mapType = &customVote;
      break;
    }
  }

  // create a new entry if we didn't find an existing list
  if (mapType == nullptr) {
    uiInfo.customVotes.emplace_back();
    mapType = &uiInfo.customVotes.back();
    mapType->type = arg;
  }

  trap_Argv(2, arg, sizeof(arg));
  const std::string field = arg;

  if (field == CUSTOMVOTE_TYPE) {
    trap_Argv(3, arg, sizeof(arg));
    mapType->type = arg;
  } else if (field == CUSTOMVOTE_CVTEXT) {
    // this can potentially be multiple args
    std::string cvtext;

    for (int i = 3; i < numArgs; i++) {
      trap_Argv(i, arg, sizeof(arg));
      cvtext += std::string(arg) + " ";
    }

    cvtext.pop_back();
    mapType->callvoteText = std::move(cvtext);
  } else if (field == CUSTOMVOTE_SERVERMAPS) {
    for (int i = 3; i < numArgs; i++) {
      trap_Argv(i, arg, sizeof(arg));
      mapType->mapsOnServer.emplace_back(arg);
    }
  } else if (field == CUSTOMVOTE_OTHERMAPS) {
    for (int i = 3; i < numArgs; i++) {
      trap_Argv(i, arg, sizeof(arg));
      mapType->otherMaps.emplace_back(arg);
    }
  }
}

void resetCustomvotes() {
  uiInfo.customVotes.clear();
  uiInfo.numCustomvotes = -1;
  uiInfo.customvoteIndex = 0;
  uiInfo.customvoteMapsOnServerIndex = 0;
  uiInfo.customvoteOtherMapsIndex = 0;
}
} // namespace ETJump

void _UI_SetActiveMenu(uiMenuCommand_t menu) {
  char buf[4096]; // com_errorMessage can go up to 4096
  char *missing_files;

  // this should be the ONLY way the menu system is brought up
  // enusure minumum menu data is cached
  if (Menu_Count() > 0) {
    vec3_t v;
    v[0] = v[1] = v[2] = 0;

    menutype = menu; //----(SA)	added

    switch (menu) {
      case UIMENU_NONE:
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
        trap_Key_ClearStates();
        trap_Cvar_Set("cl_paused", "0");
        Menus_CloseAll();

        return;
      case UIMENU_MAIN:
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_CloseAll();
        // Menus_ActivateByName( "background_1",
        // qtrue );
        Menus_ActivateByName("backgroundmusic",
                             qtrue); // Arnout: not nice, but
                                     // best way to do it -
                                     // putting the music in it's
                                     // own menudef makes sure it
                                     // doesn't get restarted
                                     // every time you reach the
                                     // main menu
        if (!cl_profile.string[0]) {
          // Menus_ActivateByName(
          // "profilelogin", qtrue );
          //  FIXME: initial profile
          //  popup FIXED: handled in
          //  opener now
          Menus_ActivateByName("main_opener", qtrue);
        } else {
          Menus_ActivateByName("main_opener", qtrue);
        }

        trap_Cvar_VariableStringBuffer("com_errorMessage", buf, sizeof(buf));

        // JPW NERVE stricmp() is silly but
        // works, take a look at error.menu to
        // see why.  I think this is bustified
        // in q3ta NOTE TTimo - I'm not sure
        // Q_stricmp is useful to anything
        // anymore
        // https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=507
        // TTimo - improved and tweaked that
        // area a whole bunch
        if ((*buf) && (Q_stricmp(buf, ";"))) {
          trap_Cvar_Set("ui_connecting", "0");

          if (!Q_stricmpn(buf, "Invalid password", 16)) {
            trap_Cvar_Set("com_"
                          "errorMessage",
                          trap_TranslateString(buf)); // NERVE
                                                      // -
                                                      // SMF
            Menus_ActivateByName("popupPassword", qtrue);
          } else if (strlen(buf) > 5 && !Q_stricmpn(buf, "ET://",
                                                    5)) // fretn
          {
            // legal redirects
            // contain source ip
            // in this variable
            if (!UI_Cvar_VariableString("com_"
                                        "errorDiagn"
                                        "oseIP")[0]) {
              ETJump::handleIllegalRedirect();
              return;
            }

            Q_strncpyz(buf, buf + 5, sizeof(buf));
            Com_Printf("Server is "
                       "full, "
                       "redirect to: "
                       "%s\n",
                       buf);
            switch (ui_autoredirect.integer) {
              // auto-redirect
              case 1:
                trap_Cvar_Set("com_errorMessage", "");
                trap_Cmd_ExecuteText(EXEC_APPEND, va("connect %s\n", buf));
                break;
              // prompt
              // (default)
              default:
                trap_Cvar_Set("com_errorMessage", buf);
                Menus_ActivateByName("popupServerRedirect", qtrue);
                break;
            }
          } else {
            qboolean pb_enable = qfalse;

            if (strstr(buf, "must be "
                            "Enable"
                            "d")) {
              pb_enable = qtrue;
            }

            trap_Cvar_Set("com_"
                          "errorMessage",
                          trap_TranslateString(buf)); // NERVE
                                                      // -
                                                      // SMF
            // hacky, wanted to
            // have the printout
            // of missing files
            // text printing
            // limitations force
            // us to keep it all
            // in a single
            // message NOTE:
            // this works thanks
            // to flip flop in
            // UI_Cvar_VariableString
            if (UI_Cvar_VariableString("com_"
                                       "errorDiagn"
                                       "oseIP")[0]) {
              missing_files = UI_Cvar_VariableString("com_"
                                                     "miss"
                                                     "ingF"
                                                     "ile"
                                                     "s");
              if (missing_files[0]) {
                trap_Cvar_Set("com_errorMessage",
                              va("%s\n\n%s\n%s",
                                 UI_Cvar_VariableString("com_errorMessage"),
                                 trap_TranslateString(MISSING_FILES_MSG),
                                 missing_files));
              }
            }
            if (pb_enable) {
              Menus_ActivateByName("popu"
                                   "pErr"
                                   "or_"
                                   "pben"
                                   "abl"
                                   "e",
                                   qtrue);
            } else {
              Menus_ActivateByName("popu"
                                   "pErr"
                                   "or",
                                   qtrue);
            }
          }
        }

        trap_S_FadeAllSound(1.0f, 1000,
                            qfalse); // make sure sound fades up
        return;

      case UIMENU_TEAM:
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_ActivateByName("team", qtrue);
        return;

      case UIMENU_NEED_CD:
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_ActivateByName("needcd", qtrue);
        return;

      case UIMENU_BAD_CD_KEY:
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_ActivateByName("badcd", qtrue);
        return;

      case UIMENU_INGAME:
        trap_Key_SetCatcher(KEYCATCH_UI);
        UI_BuildPlayerList();
        Menu_SetFeederSelection(NULL, FEEDER_PLAYER_LIST, 0, NULL);
        Menus_CloseAll();
        Menus_ActivateByName("ingame_main", qtrue);
        return;

      // NERVE - SMF
      case UIMENU_WM_QUICKMESSAGE:
        uiInfo.uiDC.cursorx = 640;
        uiInfo.uiDC.cursory = 480;
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_CloseAll();
        Menus_OpenByName("wm_quickmessage");
        return;

      case UIMENU_WM_QUICKMESSAGEALT:
        uiInfo.uiDC.cursorx = 640;
        uiInfo.uiDC.cursory = 480;
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_CloseAll();
        Menus_OpenByName("wm_quickmessageAlt");
        return;

      case UIMENU_WM_FTQUICKMESSAGE:
        uiInfo.uiDC.cursorx = 640;
        uiInfo.uiDC.cursory = 480;
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_CloseAll();
        Menus_OpenByName("wm_ftquickmessage");
        return;

      case UIMENU_WM_FTQUICKMESSAGEALT:
        uiInfo.uiDC.cursorx = 640;
        uiInfo.uiDC.cursory = 480;
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_CloseAll();
        Menus_OpenByName("wm_ftquickmessageAlt");
        return;

      case UIMENU_WM_TAPOUT:
        uiInfo.uiDC.cursorx = 640;
        uiInfo.uiDC.cursory = 480;
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_CloseAll();
        Menus_OpenByName("tapoutmsg");
        return;

      case UIMENU_WM_TAPOUT_LMS:
        uiInfo.uiDC.cursorx = 640;
        uiInfo.uiDC.cursory = 480;
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_CloseAll();
        Menus_OpenByName("tapoutmsglms");
        return;

      case UIMENU_WM_AUTOUPDATE:
        // TTimo - changing the auto-update
        // strategy to a modal prompt
        Menus_OpenByName("wm_autoupdate_modal");
        return;
      // -NERVE - SMF

      // ydnar: say, team say, etc
      case UIMENU_INGAME_MESSAGEMODE:
        // trap_Cvar_Set( "cl_paused", "1" );
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_OpenByName("ingame_messagemode");
        return;

      case UIMENU_INGAME_FT_SAVELIMIT:
        trap_Key_SetCatcher(KEYCATCH_UI);
        Menus_OpenByName("ingame_ft_savelimit");
        return;

      default:
        return; // TTimo: a lot of not handled
    }
  }
}

qboolean _UI_IsFullscreen(void) { return Menus_AnyFullScreenVisible(); }

// static connstate_t	lastConnState;
// static char			lastLoadingText[MAX_INFO_VALUE];

void UI_ReadableSize(char *buf, int bufsize, int value) {
  if (value > 1024 * 1024 * 1024) // gigs
  {
    Com_sprintf(buf, bufsize, "%d", value / (1024 * 1024 * 1024));
    Com_sprintf(buf + strlen(buf), bufsize - strlen(buf), ".%02d GB",
                (value % (1024 * 1024 * 1024)) * 100 / (1024 * 1024 * 1024));
  } else if (value > 1024 * 1024) // megs
  {
    Com_sprintf(buf, bufsize, "%d", value / (1024 * 1024));
    Com_sprintf(buf + strlen(buf), bufsize - strlen(buf), ".%02d MB",
                (value % (1024 * 1024)) * 100 / (1024 * 1024));
  } else if (value > 1024) // kilos
  {
    Com_sprintf(buf, bufsize, "%d KB", value / 1024);
  } else // bytes
  {
    Com_sprintf(buf, bufsize, "%d bytes", value);
  }
}

// Assumes time is in sec
void UI_PrintTime(char *buf, int bufsize, int time) {
  // time /= 1000;  // change to seconds

  if (time > 3600) // in the hours range
  {
    Com_sprintf(buf, bufsize, "%d hr %d min", time / 3600, (time % 3600) / 60);
  } else if (time > 60) // mins
  {
    Com_sprintf(buf, bufsize, "%d min %d sec", time / 60, time % 60);
  } else // secs
  {
    Com_sprintf(buf, bufsize, "%d sec", time);
  }
}

void Text_PaintCenter(float x, float y, float scale, vec4_t color,
                      const char *text, float adjust) {
  int len = Text_Width(text, scale, 0);
  Text_Paint(x - len / 2, y, scale, color, text, 0, 0, ITEM_TEXTSTYLE_SHADOWED);
}

#if 0 // rain - unused
  #define ESTIMATES 80
static void UI_DisplayDownloadInfo(const char *downloadName, float centerPoint, float yStart, float scale)
{
	static char dlText[]                = "Downloading:";
	static char etaText[]               = "Estimated time left:";
	static char xferText[]              = "Transfer rate:";
	static int  tleEstimates[ESTIMATES] = { 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
		                                    60,  60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
		                                    60,  60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
		                                    60,  60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60 };
	static int  tleIndex = 0;

	int        downloadSize, downloadCount, downloadTime;
	char       dlSizeBuf[64], totalSizeBuf[64], xferRateBuf[64], dlTimeBuf[64];
	int        xferRate;
	const char *s;

	vec4_t bg_color = { 0.3f, 0.3f, 0.3f, 0.8f };

	downloadSize  = trap_Cvar_VariableValue("cl_downloadSize");
	downloadCount = trap_Cvar_VariableValue("cl_downloadCount");
	downloadTime  = trap_Cvar_VariableValue("cl_downloadTime");

	// Background
	UI_FillRect(0, yStart + 185, 640, 83, bg_color);

	UI_SetColor(colorYellow);
	Text_Paint(92, yStart + 210, scale, colorYellow, dlText, 0, 64, ITEM_TEXTSTYLE_SHADOWEDMORE);
	Text_Paint(35, yStart + 235, scale, colorYellow, etaText, 0, 64, ITEM_TEXTSTYLE_SHADOWEDMORE);
	Text_Paint(86, yStart + 260, scale, colorYellow, xferText, 0, 64, ITEM_TEXTSTYLE_SHADOWEDMORE);

	if (downloadSize > 0)
	{
		s = va("%s (%d%%)", downloadName, downloadCount * 100 / downloadSize);
	}
	else
	{
		s = downloadName;
	}

	Text_Paint(260, yStart + 210, scale, colorYellow, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);

	UI_ReadableSize(dlSizeBuf, sizeof dlSizeBuf, downloadCount);
	UI_ReadableSize(totalSizeBuf, sizeof totalSizeBuf, downloadSize);

	if (downloadCount < 4096 || !downloadTime)
	{
		Text_PaintCenter(centerPoint, yStart + 235, scale, colorYellow, "estimating", 0);
		Text_PaintCenter(centerPoint, yStart + 340, scale, colorYellow, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
	}
	else
	{
		if ((uiInfo.uiDC.realTime - downloadTime) / 1000)
		{
			xferRate = downloadCount / ((uiInfo.uiDC.realTime - downloadTime) / 1000);
		}
		else
		{
			xferRate = 0;
		}
		UI_ReadableSize(xferRateBuf, sizeof xferRateBuf, xferRate);

		// Extrapolate estimated completion time
		if (downloadSize && xferRate)
		{
			int n        = downloadSize / xferRate; // estimated time for entire d/l in secs
			int timeleft = 0, i;

			// We do it in K (/1024) because we'd overflow around 4MB
			tleEstimates[tleIndex] = (n - (((downloadCount / 1024) * n) / (downloadSize / 1024)));
			tleIndex++;
			if (tleIndex >= ESTIMATES)
			{
				tleIndex = 0;
			}

			for (i = 0; i < ESTIMATES; i++)
				timeleft += tleEstimates[i];

			timeleft /= ESTIMATES;

			UI_PrintTime(dlTimeBuf, sizeof dlTimeBuf, timeleft);

			Text_Paint(260, yStart + 235, scale, colorYellow, dlTimeBuf, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
			Text_PaintCenter(centerPoint, yStart + 340, scale, colorYellow, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
		}
		else
		{
			Text_PaintCenter(centerPoint, yStart + 235, scale, colorYellow, "estimating", 0);
			if (downloadSize)
			{
				Text_PaintCenter(centerPoint, yStart + 340, scale, colorYellow, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
			}
			else
			{
				Text_PaintCenter(centerPoint, yStart + 340, scale, colorYellow, va("(%s copied)", dlSizeBuf), 0);
			}
		}

		if (xferRate)
		{
			Text_Paint(260, yStart + 260, scale, colorYellow, va("%s/Sec", xferRateBuf), 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
		}
	}
}
#endif

/*
========================
UI_DrawConnectScreen

This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
========================
*/
#define CP_LINEWIDTH 50

void UI_DrawConnectScreen(qboolean overlay) {
  //	static qboolean playingMusic = qfalse;

  if (!overlay) {
    UI_DrawLoadPanel(qfalse, qfalse, qfalse);
  } else {
    //		if( !playingMusic ) {
    //			trap_S_StartBackgroundTrack(
    //"sound/music/level_load.wav",
    //"", 1000
    //); 			playingMusic = qtrue;
    //		}
  }

  /*	if( !overlay ) {
          BG_DrawConnectScreen( qfalse );
      }*/
  /*
      char			*s;
      uiClientState_t	cstate;
      char			info[MAX_INFO_VALUE];
      char text[256];
      float centerPoint, yStart, scale;
      vec4_t color = { 0.3f, 0.3f, 0.3f, 0.8f };
  //	static qboolean playingMusic = qfalse;

      char downloadName[MAX_INFO_VALUE];

      menuDef_t *menu = Menus_FindByName("Connect");

      if ( !overlay && menu ) {
          Menu_Paint(menu, qtrue);
      }

      if (!overlay) {
          centerPoint = 320;
          yStart = 130;
          scale = 0.4f;
      } else {
          centerPoint = 320;
          yStart = 32;
          scale = 0.6f;

          // see what information we should display
          trap_GetClientState( &cstate );


          return;
      }

  //	playingMusic = qfalse;

      // see what information we should display
      trap_GetClientState( &cstate );

      info[0] = '\0';

      if (!Q_stricmp(cstate.servername,"localhost")) {
          Text_PaintCenter(centerPoint, yStart + 48, scale,
  colorWhite,va( "Enemy Territory - Version: %s", Q3_VERSION ),
  ITEM_TEXTSTYLE_SHADOWEDMORE); } else { strcpy(text, va(
  trap_TranslateString( "Connecting to %s" ), cstate.servername));
  Text_PaintCenter(centerPoint, yStart + 48, scale, colorWhite,text ,
  ITEM_TEXTSTYLE_SHADOWEDMORE);
      }

      // display global MOTD at bottom (don't draw during download, the
  space is already used)
      // moved downloadName query up, this is used in CA_CONNECTED
      trap_Cvar_VariableStringBuffer( "cl_downloadName", downloadName,
  sizeof(downloadName) );

      if (!*downloadName) {
          Text_PaintCenter(centerPoint, 475, scale, colorWhite,
  Info_ValueForKey( cstate.updateInfoString, "motd" ), 0);
      }

      // print any server info (server full, bad version, etc)
      // DHM - Nerve :: This now accepts strings up to 256 chars long,
  and will break them up into multiple lines.
      //					They are also now printed
  in Yellow for readability. if ( cstate.connState < CA_CONNECTED ) {
  char	*s; char ps[60]; int		i, len, index = 0, yPrint =
  yStart + 210; qboolean neednewline = qfalse;

          s = trap_TranslateString( cstate.messageString );
          len = strlen( s );

          for ( i = 0; i < len; i++, index++ ) {

              // copy to temp buffer
              ps[index] = s[i];

              if ( index > (CP_LINEWIDTH - 10) && i > 0 )
                  neednewline = qtrue;

              // if out of temp buffer room OR end of string OR it is
  time to linebreak & we've found a space if ( (index >= 58) || (i ==
  (len-1)) || (neednewline && s[i] == ' ') ) { ps[index+1] = '\0';

                  DC->fillRect(0, yPrint - 17, 640, 22, color);
                  Text_PaintCenter(centerPoint, yPrint, scale,
  colorYellow, ps, 0);

                  neednewline = qfalse;
                  yPrint += 22;		// next line
                  index = -1;			// sigh, for loop will
  increment to 0
              }
          }

      }

      if ( lastConnState > cstate.connState ) {
          lastLoadingText[0] = '\0';
      }
      lastConnState = cstate.connState;

      switch ( cstate.connState ) {
      case CA_CONNECTING:
          s = va( trap_TranslateString( "Awaiting connection...%i" ),
  cstate.connectPacketCount); break; case CA_CHALLENGING: s = va(
  trap_TranslateString( "Awaiting challenge...%i" ),
  cstate.connectPacketCount); break; case CA_CONNECTED: if
  (*downloadName) { UI_DisplayDownloadInfo( downloadName, centerPoint,
  yStart, scale ); return;
              }
          s = trap_TranslateString( "Awaiting gamestate..." );
          break;
      case CA_LOADING:
          return;
      case CA_PRIMED:
          return;
      default:
          return;
      }


      if (Q_stricmp(cstate.servername,"localhost")) {
          Text_PaintCenter(centerPoint, yStart + 80, scale, colorWhite,
  s, 0);
      }

      // password required / connection rejected information goes here
  */
}

/*
================
cvars
================
*/

typedef struct {
  vmCvar_t *vmCvar;
  const char *cvarName;
  const char *defaultString;
  int cvarFlags;
  int modificationCount; // for tracking changes
} cvarTable_t;

vmCvar_t ui_ffa_fraglimit;
vmCvar_t ui_ffa_timelimit;

vmCvar_t ui_team_fraglimit;
vmCvar_t ui_team_timelimit;
vmCvar_t ui_team_friendly;

vmCvar_t ui_ctf_capturelimit;
vmCvar_t ui_ctf_timelimit;
vmCvar_t ui_ctf_friendly;

vmCvar_t ui_arenasFile;
vmCvar_t ui_botsFile;
vmCvar_t ui_spScores1;
vmCvar_t ui_spScores2;
vmCvar_t ui_spScores3;
vmCvar_t ui_spScores4;
vmCvar_t ui_spScores5;
vmCvar_t ui_spAwards;
vmCvar_t ui_spVideos;
vmCvar_t ui_spSkill;

vmCvar_t ui_spSelection;
vmCvar_t ui_master;

vmCvar_t ui_brassTime;
vmCvar_t ui_drawCrosshair;
vmCvar_t ui_drawCrosshairNames;
vmCvar_t ui_drawCrosshairPickups; //----(SA) added
vmCvar_t ui_marks;
// JOSEPH 12-3-99
vmCvar_t ui_autoactivate;
// END JOSEPH

vmCvar_t ui_server1;
vmCvar_t ui_server2;
vmCvar_t ui_server3;
vmCvar_t ui_server4;
vmCvar_t ui_server5;
vmCvar_t ui_server6;
vmCvar_t ui_server7;
vmCvar_t ui_server8;
vmCvar_t ui_server9;
vmCvar_t ui_server10;
vmCvar_t ui_server11;
vmCvar_t ui_server12;
vmCvar_t ui_server13;
vmCvar_t ui_server14;
vmCvar_t ui_server15;
vmCvar_t ui_server16;

vmCvar_t ui_cdkeychecked;

vmCvar_t ui_selectedPlayer;
vmCvar_t ui_selectedPlayerName;
vmCvar_t ui_netSource;
vmCvar_t ui_menuFiles;
vmCvar_t ui_gameType;
vmCvar_t ui_netGameType;
vmCvar_t ui_joinGameType;
vmCvar_t ui_dedicated;

vmCvar_t ui_clipboardName; // the name of the group for the current clipboard
                           // item //----(SA)	added

vmCvar_t ui_notebookCurrentPage; //----(SA)	added

// NERVE - SMF - cvars for multiplayer
vmCvar_t ui_currentNetMap;
vmCvar_t ui_currentMap;
vmCvar_t ui_mapIndex;

vmCvar_t ui_browserMaster;
vmCvar_t ui_browserGameType;
vmCvar_t ui_browserSortKey;
vmCvar_t ui_browserShowEmptyOrFull;
vmCvar_t ui_browserShowPasswordProtected;
vmCvar_t ui_browserShowFriendlyFire; // NERVE - SMF
vmCvar_t ui_browserShowMaxlives;     // NERVE - SMF
vmCvar_t ui_browserShowPunkBuster;   // DHM - Nerve
vmCvar_t ui_browserShowAntilag;      // TTimo
vmCvar_t ui_browserShowWeaponsRestricted;
vmCvar_t ui_browserShowTeamBalanced;
vmCvar_t ui_browserShowETJump;

vmCvar_t ui_serverStatusTimeOut;

vmCvar_t ui_Q3Model;
vmCvar_t ui_headModel;
vmCvar_t ui_model;

vmCvar_t ui_limboOptions;
vmCvar_t ui_limboPrevOptions;
vmCvar_t ui_limboObjective;

vmCvar_t ui_cmd;

vmCvar_t ui_prevTeam;
vmCvar_t ui_prevClass;
vmCvar_t ui_prevWeapon;

vmCvar_t ui_limboMode;
vmCvar_t ui_objective;

vmCvar_t ui_team;
vmCvar_t ui_class;
vmCvar_t ui_weapon;

vmCvar_t ui_isSpectator;

vmCvar_t ui_friendlyFire;

vmCvar_t ui_userTimeLimit;
vmCvar_t ui_userAlliedRespawnTime;
vmCvar_t ui_userAxisRespawnTime;
vmCvar_t ui_glCustom; // JPW NERVE missing from q3ta
// -NERVE - SMF

vmCvar_t g_gameType;

vmCvar_t cl_profile;
vmCvar_t cl_defaultProfile;
vmCvar_t ui_profile;
// OSP
// cgame mappings
vmCvar_t ui_blackout; // For speclock
vmCvar_t cg_crosshairColor;
vmCvar_t cg_crosshairColorAlt;
vmCvar_t cg_crosshairAlpha;
vmCvar_t cg_crosshairAlphaAlt;
vmCvar_t cg_crosshairSize;
// OSP

vmCvar_t cl_bypassMouseInput;

// bani
vmCvar_t ui_autoredirect;

vmCvar_t ui_voteCheats;
vmCvar_t ui_voteCustomRTV;

vmCvar_t etj_menuSensitivity;

cvarTable_t cvarTable[] = {

    {&ui_glCustom, "ui_glCustom", "4",
     CVAR_ARCHIVE}, // JPW NERVE missing from q3ta
    {&ui_ffa_fraglimit, "ui_ffa_fraglimit", "20", CVAR_ARCHIVE},
    {&ui_ffa_timelimit, "ui_ffa_timelimit", "0", CVAR_ARCHIVE},

    {&ui_team_fraglimit, "ui_team_fraglimit", "0", CVAR_ARCHIVE},
    {&ui_team_timelimit, "ui_team_timelimit", "20", CVAR_ARCHIVE},
    {&ui_team_friendly, "ui_team_friendly", "1", CVAR_ARCHIVE},

    {&ui_ctf_capturelimit, "ui_ctf_capturelimit", "8", CVAR_ARCHIVE},
    {&ui_ctf_timelimit, "ui_ctf_timelimit", "30", CVAR_ARCHIVE},
    {&ui_ctf_friendly, "ui_ctf_friendly", "0", CVAR_ARCHIVE},

    {&ui_arenasFile, "g_arenasFile", "", CVAR_INIT | CVAR_ROM},
    {&ui_botsFile, "g_botsFile", "", CVAR_INIT | CVAR_ROM},
    {&ui_spScores1, "g_spScores1", "", CVAR_ARCHIVE | CVAR_ROM},
    {&ui_spScores2, "g_spScores2", "", CVAR_ARCHIVE | CVAR_ROM},
    {&ui_spScores3, "g_spScores3", "", CVAR_ARCHIVE | CVAR_ROM},
    {&ui_spScores4, "g_spScores4", "", CVAR_ARCHIVE | CVAR_ROM},
    {&ui_spScores5, "g_spScores5", "", CVAR_ARCHIVE | CVAR_ROM},
    {&ui_spAwards, "g_spAwards", "", CVAR_ARCHIVE | CVAR_ROM},
    {&ui_spVideos, "g_spVideos", "", CVAR_ARCHIVE | CVAR_ROM},
    {&ui_spSkill, "g_spSkill", "2", CVAR_ARCHIVE | CVAR_LATCH},

    // NERVE - SMF
    {&ui_friendlyFire, "g_friendlyFire", "1", CVAR_ARCHIVE},

    {&ui_userTimeLimit, "ui_userTimeLimit", "0", 0},
    {&ui_userAlliedRespawnTime, "ui_userAlliedRespawnTime", "0", 0},
    {&ui_userAxisRespawnTime, "ui_userAxisRespawnTime", "0", 0},
    // -NERVE - SMF

    // JPW NERVE
    {&ui_teamArenaFirstRun, "ui_teamArenaFirstRun", "0",
     CVAR_ARCHIVE}, // so sound stuff latches, strange as that seems
                    // jpw

    {&ui_spSelection, "ui_spSelection", "", CVAR_ROM},
    {&ui_master, "ui_master", "0", CVAR_ARCHIVE},

    {&ui_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE}, // JPW NERVE
    {&ui_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE},
    {&ui_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE},
    {&ui_drawCrosshairPickups, "cg_drawCrosshairPickups", "1",
     CVAR_ARCHIVE}, //----(SA) added
    {&ui_marks, "cg_marktime", "20000", CVAR_ARCHIVE},
    // JOSEPH 12-2-99
    {&ui_autoactivate, "cg_autoactivate", "1", CVAR_ARCHIVE},
    // END JOSEPH

    {&ui_server1, "server1", "", CVAR_ARCHIVE},
    {&ui_server2, "server2", "", CVAR_ARCHIVE},
    {&ui_server3, "server3", "", CVAR_ARCHIVE},
    {&ui_server4, "server4", "", CVAR_ARCHIVE},
    {&ui_server5, "server5", "", CVAR_ARCHIVE},
    {&ui_server6, "server6", "", CVAR_ARCHIVE},
    {&ui_server7, "server7", "", CVAR_ARCHIVE},
    {&ui_server8, "server8", "", CVAR_ARCHIVE},
    {&ui_server9, "server9", "", CVAR_ARCHIVE},
    {&ui_server10, "server10", "", CVAR_ARCHIVE},
    {&ui_server11, "server11", "", CVAR_ARCHIVE},
    {&ui_server12, "server12", "", CVAR_ARCHIVE},
    {&ui_server13, "server13", "", CVAR_ARCHIVE},
    {&ui_server14, "server14", "", CVAR_ARCHIVE},
    {&ui_server15, "server15", "", CVAR_ARCHIVE},
    {&ui_server16, "server16", "", CVAR_ARCHIVE},

    {&ui_dedicated, "ui_dedicated", "0", CVAR_ARCHIVE},
    {&ui_cdkeychecked, "ui_cdkeychecked", "0", CVAR_ROM},
    {&ui_selectedPlayer, "cg_selectedPlayer", "0", CVAR_ARCHIVE},
    {&ui_selectedPlayerName, "cg_selectedPlayerName", "", CVAR_ARCHIVE},
    {&ui_netSource, "ui_netSource", "1", CVAR_ARCHIVE},
    {&ui_menuFiles, "ui_menuFiles", DEFAULT_MENU_FILE, CVAR_ARCHIVE},
    {&ui_gameType, "ui_gametype", "2", CVAR_ARCHIVE},
    {&ui_joinGameType, "ui_joinGametype", "-1", CVAR_ARCHIVE},
    {&ui_netGameType, "ui_netGametype", "2", CVAR_ARCHIVE},

    {&ui_notebookCurrentPage, "ui_notebookCurrentPage", "1", CVAR_ROM},
    {&ui_clipboardName, "cg_clipboardName", "", CVAR_ROM},

    // NERVE - SMF - multiplayer cvars
    {&ui_mapIndex, "ui_mapIndex", "0", CVAR_ARCHIVE},
    {&ui_currentMap, "ui_currentMap", "0", CVAR_ARCHIVE},
    {&ui_currentNetMap, "ui_currentNetMap", "0", CVAR_ARCHIVE},

    {&ui_browserMaster, "ui_browserMaster", "0", CVAR_ARCHIVE},
    {&ui_browserGameType, "ui_browserGameType", "0", CVAR_ARCHIVE},
    {&ui_browserSortKey, "ui_browserSortKey", "4", CVAR_ARCHIVE},
    {&ui_browserShowEmptyOrFull, "ui_browserShowEmptyOrFull", "0",
     CVAR_ARCHIVE},
    {&ui_browserShowPasswordProtected, "ui_browserShowPasswordProtected", "0",
     CVAR_ARCHIVE},
    {&ui_browserShowFriendlyFire, "ui_browserShowFriendlyFire", "0",
     CVAR_ARCHIVE},
    {&ui_browserShowMaxlives, "ui_browserShowMaxlives", "0", CVAR_ARCHIVE},
    {&ui_browserShowPunkBuster, "ui_browserShowPunkBuster", "0", CVAR_ARCHIVE},
    {&ui_browserShowAntilag, "ui_browserShowAntilag", "0", CVAR_ARCHIVE},
    {&ui_browserShowWeaponsRestricted, "ui_browserShowWeaponsRestricted", "0",
     CVAR_ARCHIVE},
    {&ui_browserShowTeamBalanced, "ui_browserShowTeamBalanced", "0",
     CVAR_ARCHIVE},
    {&ui_browserShowETJump, "ui_browserShowETJump", "0", CVAR_ARCHIVE},

    {&ui_serverStatusTimeOut, "ui_serverStatusTimeOut", "7000", CVAR_ARCHIVE},

    {&ui_Q3Model, "ui_Q3Model", "1", 0},
    {&ui_headModel, "headModel", "", 0},

    {&ui_limboOptions, "ui_limboOptions", "0", 0},
    {&ui_limboPrevOptions, "ui_limboPrevOptions", "0", 0},
    {&ui_limboObjective, "ui_limboObjective", "0", 0},
    {&ui_cmd, "ui_cmd", "", 0},

    {&ui_prevTeam, "ui_prevTeam", "-1", 0},
    {&ui_prevClass, "ui_prevClass", "-1", 0},
    {&ui_prevWeapon, "ui_prevWeapon", "-1", 0},

    {&ui_limboMode, "ui_limboMode", "0", 0},
    {&ui_objective, "ui_objective", "", 0},

    {&ui_team, "ui_team", "Axis", 0},
    {&ui_class, "ui_class", "Soldier", 0},
    {&ui_weapon, "ui_weapon", "MP 40", 0},

    {&ui_isSpectator, "ui_isSpectator", "1", 0},
    // -NERVE - SMF

    {&g_gameType, "g_gameType", "2", CVAR_SERVERINFO | CVAR_LATCH},
    {NULL, "cg_drawBuddies", "1", CVAR_ARCHIVE},
    {NULL, "cg_drawRoundTimer", "0", CVAR_ARCHIVE},
    {NULL, "cg_showblood", "1", CVAR_ARCHIVE},
    {NULL, "cg_bloodFlash", "1.0", CVAR_ARCHIVE},
    {NULL, "cg_autoReload", "1", CVAR_ARCHIVE},
    {NULL, "cg_noAmmoAutoSwitch", "1", CVAR_ARCHIVE},
    {NULL, "cg_useWeapsForZoom", "1", CVAR_ARCHIVE},
    {NULL, "cg_zoomDefaultSniper", "20", CVAR_ARCHIVE},
    {NULL, "cg_zoomstepsniper", "2", CVAR_ARCHIVE},
    {NULL, "cg_voicespritetime", "6000", CVAR_ARCHIVE},
    {NULL, "cg_complaintPopUp", "1", CVAR_ARCHIVE},
    {NULL, "cg_announcer", "1", CVAR_ARCHIVE},
    {NULL, "cg_printObjectiveInfo", "1", CVAR_ARCHIVE},
    {NULL, "cg_useScreenshotJPEG", "1", CVAR_ARCHIVE},
    {NULL, "cg_drawGun", "1", CVAR_ARCHIVE},
    {NULL, "cg_drawCompass", "1", CVAR_ARCHIVE},
    {NULL, "cg_drawRoundTimer", "0", CVAR_ARCHIVE},
    {NULL, "cg_drawReinforcementTime", "0", CVAR_ARCHIVE},
    {NULL, "cg_cursorHints", "1", CVAR_ARCHIVE},
    {NULL, "cg_crosshairPulse", "1", CVAR_ARCHIVE},
    {NULL, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE},
    {NULL, "cg_crosshairColor", "White", CVAR_ARCHIVE},
    {NULL, "cg_crosshairAlpha", "1.0", CVAR_ARCHIVE},
    {NULL, "cg_crosshairColorAlt", "White", CVAR_ARCHIVE},
    {NULL, "cg_coronafardist", "1536", CVAR_ARCHIVE},
    {NULL, "cg_wolfparticles", "1", CVAR_ARCHIVE},
    {NULL, "g_password", "none", CVAR_TEMP},
    {NULL, "g_antilag", "1", CVAR_SERVERINFO | CVAR_ARCHIVE},
    {NULL, "g_warmup", "60", CVAR_ARCHIVE},
    {&cl_profile, "cl_profile", "", CVAR_ROM},
    {&cl_defaultProfile, "cl_defaultProfile", "", CVAR_ROM},
    {&ui_profile, "ui_profile", "", CVAR_ROM},

    // START - TAT 9/16/2002
    // cvar used to implement context sensitive bot menu
    //		if this is set to "engineer", for instance, then only engineer
    // commands will show up
    {NULL, "cg_botMenuType", "0", CVAR_TEMP},
    // END - TAT 9/15/2002

    // OSP
    // cgame mappings
    {&ui_blackout, "ui_blackout", "0", CVAR_ROM},
    {&cg_crosshairAlpha, "cg_crosshairAlpha", "1.0", CVAR_ARCHIVE},
    {&cg_crosshairAlphaAlt, "cg_crosshairAlphaAlt", "1.0", CVAR_ARCHIVE},
    {&cg_crosshairColor, "cg_crosshairColor", "White", CVAR_ARCHIVE},
    {&cg_crosshairColorAlt, "cg_crosshairColorAlt", "White", CVAR_ARCHIVE},
    {&cg_crosshairSize, "cg_crosshairSize", "48", CVAR_ARCHIVE},
    // game mappings (for create server option)
    {NULL, "bot_minplayers", "0", CVAR_ARCHIVE},
    {NULL, "g_ipcomplaintlimit", "3", CVAR_ARCHIVE},
    {NULL, "g_complaintlimit", "6", CVAR_ARCHIVE},
    {NULL, "g_doWarmup", "0", CVAR_ARCHIVE},
    {NULL, "g_inactivity", "180", CVAR_ARCHIVE},
    {NULL, "g_maxLives", "0", CVAR_ARCHIVE},
    {NULL, "refereePassword", "none", CVAR_ARCHIVE},
    {NULL, "g_teamForceBalance", "0", CVAR_ARCHIVE},
    {NULL, "sv_maxRate", "0", CVAR_ARCHIVE},
    {NULL, "g_spectatorInactivity", "0", CVAR_ARCHIVE},
    {NULL, "match_latejoin", "1", CVAR_ARCHIVE},
    {NULL, "match_minplayers", MATCH_MINPLAYERS, CVAR_ARCHIVE},
    {NULL, "match_readypercent", "100", CVAR_ARCHIVE},
    {NULL, "match_timeoutcount", "3", CVAR_ARCHIVE},
    {NULL, "match_timeoutlength", "180", CVAR_ARCHIVE},
    {NULL, "match_warmupDamage", "1", CVAR_ARCHIVE},
    {NULL, "server_motd0", " ^NEnemy Territory ^7MOTD ", CVAR_ARCHIVE},
    {NULL, "server_motd1", "", CVAR_ARCHIVE},
    {NULL, "server_motd2", "", CVAR_ARCHIVE},
    {NULL, "server_motd3", "", CVAR_ARCHIVE},
    {NULL, "server_motd4", "", CVAR_ARCHIVE},
    {NULL, "server_motd5", "", CVAR_ARCHIVE},
    {NULL, "team_maxPanzers", "-1", CVAR_ARCHIVE},
    {NULL, "team_maxplayers", "0", CVAR_ARCHIVE},
    {NULL, "team_nocontrols", "0", CVAR_ARCHIVE},
    {NULL, "vote_allow_map", "1", CVAR_ARCHIVE},
    {NULL, "vote_allow_matchreset", "1", CVAR_ARCHIVE},
    {NULL, "vote_allow_randommap", "1", CVAR_ARCHIVE},
    {NULL, "vote_allow_rtv", "1", CVAR_ARCHIVE},
    {NULL, "vote_allow_autoRtv", "1", CVAR_ARCHIVE},
    {NULL, "vote_limit", "5", CVAR_ARCHIVE},
    {NULL, "vote_percent", "50", CVAR_ARCHIVE},
    // OSP

    //{ NULL, "ui_creatingprofile", "", CVAR_ARCHIVE },
    {NULL, "ui_r_mode", "", CVAR_ARCHIVE},
    {NULL, "ui_r_gamma", "", CVAR_ARCHIVE},
    {NULL, "ui_rate", "", CVAR_ARCHIVE},
    {NULL, "ui_handedness", "", CVAR_ARCHIVE},
    {NULL, "ui_sensitivity", "", CVAR_ARCHIVE},
    {NULL, "ui_profile_mousePitch", "", CVAR_ARCHIVE},

    {&cl_bypassMouseInput, "cl_bypassMouseInput", "0", CVAR_TEMP},

    {NULL, "ui_showtooltips", "1", CVAR_ARCHIVE},

    // bani
    {&ui_autoredirect, "ui_autoredirect", "0", CVAR_ARCHIVE},

    {&ui_voteCheats, "ui_voteCheats", "0", CVAR_ARCHIVE},
    {&ui_voteCustomRTV, "ui_voteCustomRTV", "0", CVAR_ARCHIVE},

    {&etj_menuSensitivity, "etj_menuSensitivity", "1.0", CVAR_ARCHIVE},
};

int cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);

/*
=================
UI_RegisterCvars
=================
*/
void UI_RegisterCvars(void) {
  int i;
  cvarTable_t *cv;

  for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
    trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString,
                       cv->cvarFlags);
    if (cv->vmCvar != NULL) {
      cv->modificationCount = cv->vmCvar->modificationCount;
    }
  }

  // OSP - Always force this to 0 on init
  trap_Cvar_Set("ui_blackout", "0");
}

/*
=================
UI_UpdateCvars
=================
*/
void UI_UpdateCvars(void) {
  int i;
  cvarTable_t *cv;

  for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
    if (cv->vmCvar) {
      trap_Cvar_Update(cv->vmCvar);
      if (cv->modificationCount != cv->vmCvar->modificationCount) {
        cv->modificationCount = cv->vmCvar->modificationCount;
      }
    }
  }
}

// NERVE - SMF
/*
=================
ArenaServers_StopRefresh
=================
*/
static void UI_StopServerRefresh(void) {
  int count;

  if (!uiInfo.serverStatus.refreshActive) {
    // not currently refreshing
    return;
  }
  uiInfo.serverStatus.refreshActive = qfalse;
  Com_Printf("%d servers listed in browser with %d players.\n",
             uiInfo.serverStatus.numDisplayServers,
             uiInfo.serverStatus.numPlayersOnServers);
  count = trap_LAN_GetServerCount(ui_netSource.integer);
  if (count - uiInfo.serverStatus.numDisplayServers > 0) {
    // TTimo - used to be about cl_maxping filtering, that was
    // Q3 legacy, RTCW browser has much more filtering options
    Com_Printf("%d servers not listed (filtered out by game "
               "browser settings)\n",
               count - uiInfo.serverStatus.numDisplayServers);
  }
}

/*
=================
UI_DoServerRefresh
=================
*/
static void UI_DoServerRefresh(void) {
  qboolean wait = qfalse;

  if (!uiInfo.serverStatus.refreshActive) {
    return;
  }
  if (ui_netSource.integer != AS_FAVORITES) {
    if (ui_netSource.integer == AS_LOCAL) {
      if (!trap_LAN_GetServerCount(ui_netSource.integer)) {
        wait = qtrue;
      }
    } else {
      if (trap_LAN_GetServerCount(ui_netSource.integer) < 0) {
        wait = qtrue;
      }
    }
  }

  if (uiInfo.uiDC.realTime < uiInfo.serverStatus.refreshtime) {
    if (wait) {
      return;
    }
  }

  // if still trying to retrieve pings
  if (trap_LAN_UpdateVisiblePings(ui_netSource.integer)) {
    uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
  } else if (!wait) {
    // get the last servers in the list
    UI_BuildServerDisplayList(2);
    // stop the refresh
    UI_StopServerRefresh();
  }
  //
  UI_BuildServerDisplayList(qfalse);
}

/*
=================
UI_StartServerRefresh
=================
*/
static void UI_StartServerRefresh(qboolean full) {
  char *ptr;
  char buff[64];

  qtime_t q;
  trap_RealTime(&q);
  Com_sprintf(buff, sizeof(buff), "%s-%i, %i at %s:%s", MonthAbbrev[q.tm_mon],
              q.tm_mday, 1900 + q.tm_year,
              q.tm_hour < 10 ? va("0%i", q.tm_hour) : va("%i", q.tm_hour),
              q.tm_min < 10 ? va("0%i", q.tm_min) : va("%i", q.tm_min));
  trap_Cvar_Set(va("ui_lastServerRefresh_%i", ui_netSource.integer), buff);

  if (!full) {
    UI_UpdatePendingPings();
    return;
  }

  uiInfo.serverStatus.refreshActive = qtrue;
  uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 1000;
  // clear number of displayed servers
  uiInfo.serverStatus.numDisplayServers = 0;
  uiInfo.serverStatus.numPlayersOnServers = 0;
  // mark all servers as visible so we store ping updates for them
  trap_LAN_MarkServerVisible(ui_netSource.integer, -1, qtrue);
  // reset all the pings
  trap_LAN_ResetPings(ui_netSource.integer);
  //
  if (ui_netSource.integer == AS_LOCAL) {
    trap_Cmd_ExecuteText(EXEC_APPEND, "localservers\n");
    uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
    return;
  }

  uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 5000;
  if (ui_netSource.integer == AS_GLOBAL) {
    ptr = UI_Cvar_VariableString("debug_protocol");
    if (*ptr) {
      trap_Cmd_ExecuteText(EXEC_APPEND, va("globalservers %d %s\n", 0, ptr));
    } else {
      trap_Cmd_ExecuteText(EXEC_APPEND,
                           va("globalservers %d %d\n", 0,
                              (int)trap_Cvar_VariableValue("protocol")));
    }
  }
}
// -NERVE - SMF
