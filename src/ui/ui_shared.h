#ifndef __UI_SHARED_H
#define __UI_SHARED_H

#include <vector>
#include <string>
#include <functional>
#include <memory>

#include "../game/q_shared.h"
#include "../cgame/tr_types.h"
#include "keycodes.h"

#include "../../assets/ui/menudef.h"
#include "../../assets/ui/menudef_ext.h"

inline constexpr int MAX_MENUDEFFILE = 4096;
inline constexpr int MAX_MENUFILE = 32768;
inline constexpr int MAX_MENUS = 256;     // was 128
inline constexpr int MAX_MENUITEMS = 128; // JPW NERVE q3ta was 96
inline constexpr int MAX_COLOR_RANGES = 10;
inline constexpr int MAX_MODAL_MENUS = 16;

// mouse is over it, non-exclusive
inline constexpr uint32_t WINDOW_MOUSEOVER = 1 << 0;
// has cursor focus, exclusive
inline constexpr uint32_t WINDOW_HASFOCUS = 1 << 1;
// is visible
inline constexpr uint32_t WINDOW_VISIBLE = 1 << 2;
// is visible but grey ( non-active )
inline constexpr uint32_t WINDOW_GREY = 1 << 3;
// for decoration only, no mouse, keyboard, etc..
inline constexpr uint32_t WINDOW_DECORATION = 1 << 4;
// fading out, non-active
inline constexpr uint32_t WINDOW_FADINGOUT = 1 << 5;
// fading in
inline constexpr uint32_t WINDOW_FADINGIN = 1 << 6;
// mouse is over it, non-exclusive
inline constexpr uint32_t WINDOW_MOUSEOVERTEXT = 1 << 7;
// window is in transition
inline constexpr uint32_t WINDOW_INTRANSITION = 1 << 8;
// forecolor was explicitly set ( used to color alpha images or not )
inline constexpr uint32_t WINDOW_FORECOLORSET = 1 << 9;
// for list boxes and sliders, vertical is default this is set of horizontal
inline constexpr uint32_t WINDOW_HORIZONTAL = 1 << 10;
// mouse is over left/up arrow
inline constexpr uint32_t WINDOW_LB_LEFTARROW = 1 << 11;
// mouse is over right/down arrow
inline constexpr uint32_t WINDOW_LB_RIGHTARROW = 1 << 12;
// mouse is over thumb
inline constexpr uint32_t WINDOW_LB_THUMB = 1 << 13;
// mouse is over page up
inline constexpr uint32_t WINDOW_LB_PGUP = 1 << 14;
// mouse is over page down
inline constexpr uint32_t WINDOW_LB_PGDN = 1 << 15;
// item is in orbit
inline constexpr uint32_t WINDOW_ORBITING = 1 << 16;
// close on out-of-bounds click
inline constexpr uint32_t WINDOW_OOB_CLICK = 1 << 17;
// manually wrap text
inline constexpr uint32_t WINDOW_WRAPPED = 1 << 18;
// auto wrap text
inline constexpr uint32_t WINDOW_AUTOWRAPPED = 1 << 19;
// forced open
inline constexpr uint32_t WINDOW_FORCED = 1 << 20;
// popup
inline constexpr uint32_t WINDOW_POPUP = 1 << 21;
// backcolor was explicitly set
inline constexpr uint32_t WINDOW_BACKCOLORSET = 1 << 22;
// visibility timing ( NOT implemented )
inline constexpr uint32_t WINDOW_TIMEDVISIBLE = 1 << 23;
// window will apply cg_hudAlpha value to colors unless this flag is set
inline constexpr uint32_t WINDOW_IGNORE_HUDALPHA = 1 << 24;
inline constexpr uint32_t WINDOW_DRAWALWAYSONTOP = 1 << 25;
// window is modal, the window to go back to is stored in a stack
inline constexpr uint32_t WINDOW_MODAL = 1 << 26;
inline constexpr uint32_t WINDOW_FOCUSPULSE = 1 << 27;
inline constexpr uint32_t WINDOW_TEXTASINT = 1 << 28;
inline constexpr uint32_t WINDOW_TEXTASFLOAT = 1 << 29;
inline constexpr uint32_t WINDOW_LB_SOMEWHERE = 1 << 30;
// horizontally centered
inline constexpr uint32_t WINDOW_CENTERED = 1 << 31;

#ifdef CGAME
inline constexpr int STRING_POOL_SIZE = 128 * 2048;
#else
inline constexpr int STRING_POOL_SIZE = 384 * 2048;
#endif

inline constexpr int MAX_EDITFIELD = 256;

inline constexpr char ART_FX_BASE[] = "menu/art/fx_base";
inline constexpr char ART_FX_BLUE[] = "menu/art/fx_blue";
inline constexpr char ART_FX_CYAN[] = "menu/art/fx_cyan";
inline constexpr char ART_FX_GREEN[] = "menu/art/fx_grn";
inline constexpr char ART_FX_RED[] = "menu/art/fx_red";
inline constexpr char ART_FX_TEAL[] = "menu/art/fx_teal";
inline constexpr char ART_FX_WHITE[] = "menu/art/fx_white";
inline constexpr char ART_FX_YELLOW[] = "menu/art/fx_yel";

inline constexpr char ASSET_GRADIENTBAR[] = "ui/assets/gradientbar2.tga";
inline constexpr char ASSET_SCROLLBAR[] = "ui/assets/scrollbar.tga";
inline constexpr char ASSET_SCROLLBAR_ARROWDOWN[] =
    "ui/assets/scrollbar_arrow_dwn_a.tga";
inline constexpr char ASSET_SCROLLBAR_ARROWUP[] =
    "ui/assets/scrollbar_arrow_up_a.tga";
inline constexpr char ASSET_SCROLLBAR_ARROWLEFT[] =
    "ui/assets/scrollbar_arrow_left.tga";
inline constexpr char ASSET_SCROLLBAR_ARROWRIGHT[] =
    "ui/assets/scrollbar_arrow_right.tga";
inline constexpr char ASSET_SCROLL_THUMB[] = "ui/assets/scrollbar_thumb.tga";
inline constexpr char ASSET_SLIDER_BAR[] = "ui/assets/slider2.tga";
inline constexpr char ASSET_SLIDER_THUMB[] = "ui/assets/sliderbutt_1.tga";
inline constexpr char ASSET_CHECKBOX_CHECK[] = "ui/assets/check.tga";
inline constexpr char ASSET_CHECKBOX_CHECK_NOT[] = "ui/assets/check_not.tga";
inline constexpr char ASSET_CHECKBOX_CHECK_NO[] = "ui/assets/check_no.tga";

inline constexpr char ASSET_REPLAY_DIRECTORY[] = "gfx/2d/directory";
inline constexpr char ASSET_REPLAY_HOME[] = "gfx/2d/home";
inline constexpr char ASSET_REPLAY_UP[] = "gfx/2d/up";

inline constexpr char ASSET_COLORPICKER_MASK[] = "gfx/2d/colorpicker_mask";

inline constexpr float SCROLLBAR_SIZE = 16.0f;
inline constexpr float SCROLLBAR_SIZE_COMBO = 10.0f;
inline constexpr float SLIDER_WIDTH = 96.0f;
inline constexpr float SLIDER_HEIGHT = 10.0f;
inline constexpr float SLIDER_THUMB_WIDTH = 12.0f;
inline constexpr float SLIDER_THUMB_HEIGHT = 12.0f;

inline constexpr int NUM_CROSSHAIRS = 17;

// y offset applied to each line of autowrapped text, along with text height
inline constexpr int AUTOWRAP_OFFSET = 5;

typedef struct rectDef_s {
  float x; // horiz position
  float y; // vert position
  float w; // width
  float h; // height;
} rectDef_t;

// FIXME: do something to separate text vs window stuff
typedef struct {
  rectDef_t rect;            // client coord rectangle
  rectDef_t rectClient;      // screen coord rectangle
  const char *name;          //
  const char *model;         //
  const char *group;         // if it belongs to a group
  const char *cinematicName; // cinematic name
  int cinematic;             // cinematic handle
  int style;                 //
  int border;                //
  int ownerDraw;             // ownerDraw style
  int ownerDrawFlags;        // show flags for ownerdraw items
  float borderSize;          //
  int borderFixedSize;
  uint32_t flags;         // visible, focus, mouseover, cursor
  rectDef_t rectEffects;  // for various effects
  rectDef_t rectEffects2; // for various effects
  int offsetTime;         // time based value for various effects
  int nextTime;           // time next effect should cycle
  vec4_t foreColor;       // text color
  vec4_t backColor;       // background color
  // alt background color, for alternating listbox entry background colors
  vec4_t backColorAlt;
  vec4_t borderColor;   // border color
  vec4_t outlineColor;  // border color
  qhandle_t background; // background asset
} windowDef_t;

typedef windowDef_t Window;

typedef struct {
  vec4_t color;
  int type;
  float low;
  float high;
} colorRangeDef_t;

inline constexpr int MAX_LB_COLUMNS = 16;

typedef struct columnInfo_s {
  int pos;
  int width;
  int maxChars;
} columnInfo_t;

typedef struct listBoxDef_s {
  int startPos;
  int endPos;
  int drawPadding;
  int cursorPos;
  float elementWidth;
  float elementHeight;
  int elementStyle;
  int numColumns;
  columnInfo_t columnInfo[MAX_LB_COLUMNS];
  const char *doubleClick;
  const char *contextMenu;
  qboolean notselectable;
} listBoxDef_t;

typedef struct editFieldDef_s {
  float minVal; //	edit field limits
  float maxVal; //
  float defVal; //
  float range;  //
  float step;
  int maxChars;      // for edit fields
  int maxPaintChars; // for edit fields
  int paintOffset;   //
} editFieldDef_t;

inline constexpr int MAX_MULTI_CVARS = 64;

typedef struct multiDef_s {
  const char *cvarList[MAX_MULTI_CVARS];
  const char *cvarStr[MAX_MULTI_CVARS];
  float cvarValue[MAX_MULTI_CVARS];
  int count;
  qboolean strDef;
  const char *undefinedStr;
} multiDef_t;

typedef struct modelDef_s {
  int angle;
  vec3_t origin;
  float fov_x;
  float fov_y;
  int rotationSpeed;

  int animated;
  int startframe;
  int numframes;
  int loopframes;
  int fps;

  int frame;
  int oldframe;
  float backlerp;
  int frameTime;
} modelDef_t;

struct comboDef_t {
  // note: rect height is NOT the dropdown height, it's merely the height of
  // the actual dropdown entry, comboData.height contains the dropdown height
  rectDef_t rect;
  int maxItems;
  bool bitflag;  // is this a bitflag selection dropdown?
  bool reversed; // should we draw this bottom to top?
  int startPos;
  bool scrollbar;
  float height; // height of the dropdown part of the menu
};

struct colorSliderDef_t {
  const char *colorVar;
  int colorType; // HSV, RGB, Alpha
};

struct textScroll_t {
  // the item which we're currently bound to, used to reset the state when
  // switching selection (e.g. selecting a new map to read briefing from)
  int scrollItem;
  // the text currently being scrolled, used to reset the state if
  // text updates mid-scroll
  char scrollText[MAX_TOKEN_CHARS];

  int scrollStartTime;
  int scrollEndTime;
  float scrollDeltaTime; // for framerate independent scroll speed

  float x;
  float y;

  int textOffset; // text index when scrolling horizontally off-screen

  bool scrolling;
};

inline constexpr int CVAR_ENABLE = 0x00000001;
inline constexpr int CVAR_DISABLE = 0x00000002;
inline constexpr int CVAR_SHOW = 0x00000004;
inline constexpr int CVAR_HIDE = 0x00000008;
inline constexpr int CVAR_NOTOGGLE = 0x00000010;

// OSP - "setting" flags for items
inline constexpr int SVS_DISABLED_SHOW = 0x01;
inline constexpr int SVS_ENABLED_SHOW = 0x02;

inline constexpr float DEFAULT_LINEHEIGHT = 11.0f;

typedef struct itemDef_s {
  Window window;      // common positional, border, style, layout info
  rectDef_t textRect; // rectangle the text ( if any ) consumes
  int type;          // text, button, radiobutton, checkbox, textfield, listbox,
                     // combo
  int alignment;     // left center right
  int textalignment; // ( optional ) alignment for text within rect
                     // based on text width
  float textalignx;  // ( optional ) text alignment x coord
  float textaligny;  // ( optional ) text alignment y coord
  float textscale;   // scale percentage from 72pts
  int font;          // (SA)
  int textStyle;     // ( optional ) style, normal and shadowed are it for
                     // now
  float lineHeight;
  const char *text;           // display text
  void *parent;               // menu owner
  qhandle_t asset;            // handle to asset
  const char *mouseEnterText; // mouse enter script
  const char *mouseExitText;  // mouse exit script
  const char *mouseEnter;     // mouse enter script
  const char *mouseExit;      // mouse exit script
  const char *action;         // select script
  const char *onAccept;       // NERVE - SMF - run when the users presses the
                              // enter key
  const char *onFocus;        // select script
  const char *leaveFocus;     // select script
  const char *cvar;           // associated cvar
  const char *cvarTest;       // associated cvar for enable actions
  const char *enableCvar;     // enable, disable, show, or hide based on
                              // value, this can contain a list
  int cvarFlags;              //	what type of action to take on cvarenables
  sfxHandle_t focusSound;
  int numColors; // number of color ranges
  colorRangeDef_t colorRanges[MAX_COLOR_RANGES];
  int colorRangeType; // either
  float special;      // used for feeder id's etc.. diff per type
  int cursorPos;      // cursor position in characters
  void *typeData;     // type specific data ptr's

  // START - TAT 9/16/2002
  //		For the bot menu, we have context sensitive menus
  //		the way it works, we could have multiple items in a menu
  // with
  // the same hotkey 		so in the mission pack, we search through
  // all the menu items to find the one that is applicable to this key
  // press 		so the item has to store both the hotkey and the
  // command to execute
  int hotkey;
  const char *onKey;
  // END - TAT 9/16/2002

  // OSP - on-the-fly enable/disable of items
  int settingTest;
  int settingFlags;
  int voteFlag;

  const char *onEsc;
  const char *onEnter;

  struct itemDef_s *toolTipData; // OSP - Tag an item to this item for
                                 // auto-help popups

  qboolean cvarLength;
  qboolean multiline;

  int cursorDir; // cursor vertical direction 1 down, -1 up
  vec4_t cursorColor;

  const char *hOffset;
  const char *yOffset;

  comboDef_t comboData;

  colorSliderDef_t colorSliderData;
  bool tooltipAbove;

  textScroll_t textScroll;

  bool cacheCvar; // update cvar value only when itemCapture ends
  const char *cacheCvarValue;
} itemDef_t;

typedef struct {
  Window window;
  const char *font;    // font
  qboolean fullScreen; // covers entire screen
  int itemCount;       // number of items;
  int fontIndex;       //
  int cursorItem;      // which item as the cursor
  int fadeCycle;       //
  float fadeClamp;     //
  float fadeAmount;    //
  const char *onOpen;  // run when the menu is first opened
  const char *onClose; // run when the menu is closed
  const char *onESC;   // run when the escape key is hit
  const char *onEnter; // run when the enter key is hit

  int timeout;           // ydnar: milliseconds until menu times out
  int openTime;          // ydnar: time menu opened
  const char *onTimeout; // ydnar: run when menu times out

  // NERVE - SMF - execs commands when a key is pressed
  const char *onKey[K_MAX_KEYS];
  const char *soundName; // background loop sound for menu

  vec4_t focusColor;               // focus color for items
  vec4_t disableColor;             // focus color for items
  itemDef_t *items[MAX_MENUITEMS]; // items this menu contains

  // START - TAT 9/16/2002
  // should we search through all the items to find the hotkey instead
  // of using the onKey array?
  //		The bot command menu needs to do this, see note above
  qboolean itemHotkeyMode;
  // END - TAT 9/16/2002
} menuDef_t;

typedef struct {
  const char *fontStr;
  const char *cursorStr;
  const char *gradientStr;
  fontInfo_t fonts[6];
  qhandle_t cursor;
  qhandle_t gradientBar;
  qhandle_t scrollBarArrowUp;
  qhandle_t scrollBarArrowDown;
  qhandle_t scrollBarArrowLeft;
  qhandle_t scrollBarArrowRight;
  qhandle_t scrollBar;
  qhandle_t scrollBarThumb;
  qhandle_t buttonMiddle;
  qhandle_t buttonInside;
  qhandle_t solidBox;
  qhandle_t sliderBar;
  qhandle_t sliderThumb;
  qhandle_t checkboxCheck;
  qhandle_t checkboxCheckNot;
  qhandle_t checkboxCheckNo;
  sfxHandle_t menuEnterSound;
  sfxHandle_t menuExitSound;
  sfxHandle_t menuBuzzSound;
  sfxHandle_t itemFocusSound;
  float fadeClamp;
  int fadeCycle;
  float fadeAmount;
  float shadowX;
  float shadowY;
  vec4_t shadowColor;
  float shadowFadeClamp;
  qboolean fontRegistered;

  // player settings
  qhandle_t fxBasePic;
  qhandle_t fxPic[7];
  qhandle_t crosshairShader[NUM_CROSSHAIRS];
  qhandle_t crosshairAltShader[NUM_CROSSHAIRS];

  qhandle_t replayDirectory;
  qhandle_t replayHome;
  qhandle_t replayUp;

  qhandle_t colorPickerMask;
} cachedAssets_t;

typedef struct {
  const char *name;
  void (*handler)(itemDef_t *item, qboolean *bAbort, const char **args);
} commandDef_t;

typedef struct {
  qhandle_t (*registerShaderNoMip)(const char *p);
  void (*setColor)(const vec4_t v);
  void (*drawHandlePic)(float x, float y, float w, float h, qhandle_t asset);
  void (*drawStretchPic)(float x, float y, float w, float h, float s1, float t1,
                         float s2, float t2, qhandle_t hShader);
  void (*drawText)(float x, float y, float scale, vec4_t color,
                   const char *text, float adjust, int limit, int style);
  void (*drawTextExt)(float x, float y, float scalex, float scaley,
                      vec4_t color, const char *text, float adjust, int limit,
                      int style, fontInfo_t *font);
  int (*textWidth)(const char *text, float scale, int limit);
  int (*textWidthExt)(const char *text, float scale, int limit,
                      fontInfo_t *font);
  int (*multiLineTextWidth)(const char *text, float scale, int limit);
  int (*textHeight)(const char *text, float scale, int limit);
  int (*textHeightExt)(const char *text, float scale, int limit,
                       fontInfo_t *font);
  int (*multiLineTextHeight)(const char *text, float scale, int limit);
  void (*textFont)(int font); // NERVE - SMF
  qhandle_t (*registerModel)(const char *p);
  void (*modelBounds)(qhandle_t model, vec3_t min, vec3_t max);
  void (*fillRect)(float x, float y, float w, float h, const vec4_t color);
  void (*drawRect)(float x, float y, float w, float h, float size,
                   const vec4_t color);
  void (*drawRectFixed)(float x, float y, float w, float h, float border,
                        const vec4_t color);
  void (*drawSides)(float x, float y, float w, float h, float size);
  void (*drawTopBottom)(float x, float y, float w, float h, float size);
  void (*drawSidesNoScale)(float x, float y, float w, float h, float size);
  void (*drawTopBottomNoScale)(float x, float y, float w, float h, float size);
  void (*clearScene)();
  void (*addRefEntityToScene)(const refEntity_t *re);
  void (*renderScene)(const refdef_t *fd);
  void (*registerFont)(const char *pFontname, int pointSize, fontInfo_t *font);
  void (*ownerDrawItem)(float x, float y, float w, float h, float text_x,
                        float text_y, int ownerDraw, int ownerDrawFlags,
                        int align, float special, float scale, vec4_t color,
                        qhandle_t shader, int textStyle);
  float (*getValue)(int ownerDraw, int type);
  qboolean (*ownerDrawVisible)(int flags);
  void (*runScript)(const char **p);
  void (*getTeamColor)(vec4_t *color);
  void (*getCVarString)(const char *cvar, char *buffer, int bufsize);
  float (*getCVarValue)(const char *cvar);
  void (*setCVar)(const char *cvar, const char *value);
  void (*drawTextWithCursor)(float x, float y, float scale, vec4_t color,
                             vec4_t cursorColor, const char *text,
                             int cursorPos, char cursor, int limit, int style);
  void (*setOverstrikeMode)(qboolean b);
  qboolean (*getOverstrikeMode)();
  void (*startLocalSound)(sfxHandle_t sfx, int channelNum);
  qboolean (*ownerDrawHandleKey)(int ownerDraw, int flags, float *special,
                                 int key);
  int (*feederCount)(float feederID);
  const char *(*feederItemText)(float feederID, int index, int column,
                                qhandle_t *handles, int *numhandles);
  const char *(*fileText)(char *flieName);
  qhandle_t (*feederItemImage)(float feederID, int index);
  void (*feederSelection)(float feederID, int index);
  qboolean (*feederSelectionClick)(itemDef_t *item);
  void (*feederAddItem)(float feederID, const char *name,
                        int index);             // NERVE - SMF
  char *(*translateString)(const char *string); // NERVE - SMF
  void (*checkAutoUpdate)();                    // DHM - Nerve
  void (*getAutoUpdate)();                      // DHM - Nerve

  void (*keynumToStringBuf)(int keynum, char *buf, int buflen);
  void (*getBindingBuf)(int keynum, char *buf, int buflen);
  void (*getKeysForBinding)(const char *binding, int *key1, int *key2);

  qboolean (*keyIsDown)(int keynum);

  void (*setBinding)(int keynum, const char *binding);
  void (*executeText)(int exec_when, const char *text);
  void (*Error)(int level, const char *error, ...);
  void (*Print)(const char *msg, ...);
  void (*Pause)(qboolean b);
  int (*ownerDrawWidth)(int ownerDraw, float scale);
  sfxHandle_t (*registerSound)(const char *name, qboolean compressed);
  void (*startBackgroundTrack)(const char *intro, const char *loop,
                               int fadeupTime);
  void (*stopBackgroundTrack)();
  int (*playCinematic)(const char *name, float x, float y, float w, float h);
  void (*stopCinematic)(int handle);
  void (*drawCinematic)(int handle, float x, float y, float w, float h);
  void (*runCinematicFrame)(int handle);

  void (*add2dPolys)(polyVert_t *verts, int numverts, qhandle_t hShader);
  void (*updateScreen)(void);
  void (*getHunkData)(int *hunkused, int *hunkexpected);
  int (*getConfigString)(int index, char *buff, int buffsize);
  fontInfo_t *(*getActiveFont)(void);

  float yscale;
  float xscale;
  float bias;
  int screenWidth;
  int screenHeight;

  int realTime;
  int frameTime;
  int cursorx;
  int cursory;
  qboolean debug;

  cachedAssets_t Assets;

  glconfig_t glconfig;
  qhandle_t whiteShader;
  qhandle_t gradientImage;
  qhandle_t cursor;
  float FPS;

  std::function<const char *(int)> getColorSliderString;
  std::function<void(itemDef_t *)> setColorSliderType;
  std::function<float(const std::string &)> getColorSliderValue;
  std::function<void(const std::string &, float)> setColorSliderValue;
  std::function<void(itemDef_t *)> updateSliderState;
  std::function<void(const std::string &)> cvarToColorPickerState;
  std::function<void()> resetColorPickerState;
  std::function<void(itemDef_t *, const float, const float, const int)>
      colorPickerDragFunc;
  std::function<void()> toggleRGBSliderValues;
  std::function<bool()> RGBSlidersAreNormalized;
} displayContextDef_t;

const char *String_Alloc(const char *p);
void String_Init();
void String_Report();
void Init_Display(displayContextDef_t *dc);
void Shutdown_Display();
void Display_ExpandMacros(char *buff);
void Menu_Init(menuDef_t *menu);
void Item_Init(itemDef_t *item);
void Menu_PostParse(menuDef_t *menu);
menuDef_t *Menu_GetFocused();
void Menu_HandleKey(menuDef_t *menu, int key, qboolean down);
void Menu_HandleMouseMove(menuDef_t *menu, float x, float y);
void Menu_ScrollFeeder(menuDef_t *menu, int feeder, qboolean down);
qboolean Float_Parse(const char **p, float *f);
qboolean Color_Parse(const char **p, vec4_t *c);
qboolean Int_Parse(const char **p, int *i);
qboolean Rect_Parse(const char **p, rectDef_t *r);
qboolean String_Parse(const char **p, const char **out);
void PC_SourceError(int handle, const char *format, ...);
void PC_SourceWarning(int handle, const char *format, ...);
qboolean PC_Float_Parse(int handle, float *f);
qboolean PC_Color_Parse(int handle, vec4_t *c);
qboolean PC_Int_Parse(int handle, int *i);
qboolean PC_Rect_Parse(int handle, rectDef_t *r);
qboolean PC_String_Parse(int handle, const char **out);
qboolean PC_String_ParseLower(int handle, const char **out);
qboolean PC_Script_Parse(int handle, const char **out);
qboolean PC_Char_Parse(int handle, char *out); // NERVE - SMF

namespace ETJump {
bool PC_hasFloat(int handle);
void scaleMenuSensitivity(int x, int y, float *mdx, float *mdy);
qhandle_t shaderForCrosshair(int crosshairNum, bool isAltShader);
} // namespace ETJump

int Menu_Count();
menuDef_t *Menu_Get(int handle);
void Menu_New(int handle);
void Menu_PaintAll();
menuDef_t *Menus_ActivateByName(const char *p, qboolean modalStack);
void Menu_Reset();
qboolean Menus_AnyFullScreenVisible();
void Menus_Activate(menuDef_t *menu);
qboolean Menus_CaptureFuncActive(void);

displayContextDef_t *Display_GetContext();
void *Display_CaptureItem(int x, int y);
qboolean Display_MouseMove(void *p, int x, int y);
qboolean Display_KeyBindPending();
void Menus_OpenByName(const char *p);
menuDef_t *Menus_FindByName(const char *p);
void Menus_ShowByName(const char *p);
void Menus_CloseByName(const char *p);
void Display_HandleKey(int key, qboolean down, int x, int y);
void LerpColor(vec4_t a, vec4_t b, vec4_t c, float t);
void Menus_CloseAll();
void Menu_Paint(menuDef_t *menu, qboolean forcePaint);
void Menu_SetFeederSelection(menuDef_t *menu, int feeder, int index,
                             const char *name);
void Display_CacheAll();

// TTimo
void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow);

void *UI_Alloc(int size);
void UI_InitMemory(void);
qboolean UI_OutOfMemory();

void Controls_GetConfig(void);
void Controls_SetConfig(qboolean restart);
void Controls_SetDefaults(qboolean lefthanded);

int trap_PC_AddGlobalDefine(const char *define);
int trap_PC_RemoveAllGlobalDefines(void);
int trap_PC_LoadSource(const char *filename);
int trap_PC_FreeSource(int handle);
int trap_PC_ReadToken(int handle, pc_token_t *pc_token);
int trap_PC_SourceFileAndLine(int handle, char *filename, int *line);
int trap_PC_UnReadToken(int handle);

//
// panelhandling
//

typedef struct panel_button_s panel_button_t;

typedef struct panel_button_text_s {
  float scalex, scaley;
  vec4_t colour;
  int style;
  int align;
  fontInfo_t *font;
} panel_button_text_t;

typedef qboolean (*panel_button_key_down)(panel_button_t *, int);
typedef qboolean (*panel_button_key_up)(panel_button_t *, int);
typedef void (*panel_button_render)(panel_button_t *);
typedef void (*panel_button_postprocess)(panel_button_t *);

// Button struct
struct panel_button_s {
  // compile time stuff
  // ======================
  const char *shaderNormal;

  // text
  std::string text;

  // rect
  rectDef_t rect;

  // data
  int data[8];

  // "font"
  panel_button_text_t *font;

  // functions
  panel_button_key_down onKeyDown;
  panel_button_key_up onKeyUp;
  panel_button_render onDraw;
  panel_button_postprocess onFinish;

  // run-time stuff
  // ======================
  qhandle_t hShaderNormal;
};

void BG_PanelButton_RenderEdit(panel_button_t *button);
qboolean BG_PanelButton_EditClick(panel_button_t *button, int key);
qboolean BG_PanelButtonsKeyEvent(int key, qboolean down,
                                 std::vector<panel_button_t> &buttons);
void BG_PanelButtonsSetup(std::vector<panel_button_t> &buttons);
void BG_PanelButtonsSetupWide(std::vector<panel_button_t> &buttons);
void BG_PanelButtonsRender(std::vector<panel_button_t> &buttons);
void BG_PanelButtonsRender_Text(panel_button_t *button);
void BG_PanelButtonsRender_TextExt(panel_button_t *button, const char *text);
void BG_PanelButtonsRender_Img(panel_button_t *button);
panel_button_t *
BG_PanelButtonsGetHighlightButton(std::vector<panel_button_t> &buttons);
void BG_PanelButtons_SetFocusButton(panel_button_t *button);
panel_button_t *BG_PanelButtons_GetFocusButton(void);

qboolean BG_RectContainsPoint(float x, float y, float w, float h, float px,
                              float py);
qboolean BG_CursorInRect(rectDef_t *rect);
qboolean BG_CursorInRectWide(rectDef_t *rect);

void BG_FitTextToWidth_Ext(char *instr, float scale, float w, int size,
                           fontInfo_t *font);
void BG_FitTextToWidth_Ext(std::string &instr, float scale, float w,
                           fontInfo_t *font);

void AdjustFrom640(float *x, float *y, float *w, float *h);
void SetupRotatedThing(polyVert_t *verts, vec2_t org, float w, float h,
                       vec_t angle);

void BG_HSVtoRGB(const vec4_t hsv, vec4_t rgb, bool normalize);
void BG_RGBtoHSV(const vec4_t rgb, vec4_t hsv);

#define SCREEN_WIDTH DC->screenWidth
#define SCREEN_HEIGHT DC->screenHeight
#define SCREEN_OFFSET_X (SCREEN_WIDTH - 640) / 2.f
#define SCREEN_CENTER_X SCREEN_WIDTH / 2.f
#define SCREEN_CENTER_Y SCREEN_HEIGHT / 2.f

#endif
