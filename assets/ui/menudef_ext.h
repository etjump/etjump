// ETJump settings menu definitions

// Main settings window
#define WINDOW_X 16
#define WINDOW_Y 16
#define WINDOW_WIDTH 608
#define WINDOW_HEIGHT 448
#define MAIN_ELEMENT_MARGIN 6

// Main buttons
#define NUM_MAIN_BTNS 7
#define BTN_MAIN_X MAIN_ELEMENT_MARGIN
#define BTN_MAIN_W ((WINDOW_WIDTH - ((MAIN_ELEMENT_MARGIN * NUM_MAIN_BTNS) + MAIN_ELEMENT_MARGIN)) / NUM_MAIN_BTNS)
#define BTN_MAIN_H 18
#define BTN_MAIN_Y 24 + MAIN_ELEMENT_MARGIN // 24: settings title bar height
#define BTN_MAIN_TEXT_ALIGN 13

#define MAINBUTTON(pos, text, cmd) BUTTON(BTN_MAIN_X + ((BTN_MAIN_W + MAIN_ELEMENT_MARGIN) * (pos - 1)), BTN_MAIN_Y, BTN_MAIN_W, BTN_MAIN_H, text, 0.25, BTN_MAIN_TEXT_ALIGN, cmd)
#define MAINBUTTON_ACTIVE(pos, text, cmd) BUTTONACTIVE(BTN_MAIN_X + ((BTN_MAIN_W + MAIN_ELEMENT_MARGIN) * (pos - 1)), BTN_MAIN_Y, BTN_MAIN_W, BTN_MAIN_H, text, 0.25, BTN_MAIN_TEXT_ALIGN, cmd)

// Side bar / buttons
#define SIDEBAR_X 8
#define SIDEBAR_Y (BTN_MAIN_Y + BTN_MAIN_H + MAIN_ELEMENT_MARGIN)
#define SIDEBAR_W 128
#define SIDEBAR_H (WINDOW_HEIGHT - (SIDEBAR_Y + BTN_MAIN_H + (MAIN_ELEMENT_MARGIN * 2)))

#define SIDEBAR_BTN_X (SIDEBAR_X + MAIN_ELEMENT_MARGIN)
#define SIDEBAR_BTN_W (SIDEBAR_W - (MAIN_ELEMENT_MARGIN * 2))
#define SIDEBAR_BTN_H 16
#define SIDEBAR_BTN_TEXT_ALIGN 11

#define SIDEBUTTON(pos, text, cmd) BUTTON(SIDEBAR_BTN_X, SIDEBAR_BTN_Y + ((SIDEBAR_BTN_H + MAIN_ELEMENT_MARGIN) * (pos - 1)), SIDEBAR_BTN_W, SIDEBAR_BTN_H, text, 0.2, SIDEBAR_BTN_TEXT_ALIGN, cmd)
#define SIDEBUTTON_ACTIVE(pos, text, cmd) BUTTONACTIVE(SIDEBAR_BTN_X, SIDEBAR_BTN_Y + ((SIDEBAR_BTN_H + MAIN_ELEMENT_MARGIN) * (pos - 1)), SIDEBAR_BTN_W, SIDEBAR_BTN_H, text, 0.2, SIDEBAR_BTN_TEXT_ALIGN, cmd)

// Subwindows
#define SETTINGS_SUBW_X (SIDEBAR_X + SIDEBAR_W + MAIN_ELEMENT_MARGIN)
#define SETTINGS_SUBW_Y SIDEBAR_Y
#define SETTINGS_SUBW_W (WINDOW_WIDTH - SIDEBAR_W - 3 - (MAIN_ELEMENT_MARGIN * 3)) // - 3 to account for rect borders
#define SETTINGS_SUBW_H SIDEBAR_H

// Menu items
#define SETTINGS_ITEM_X SETTINGS_SUBW_X + 8
#define SETTINGS_ITEM_Y (SETTINGS_SUBW_Y + 12 + MAIN_ELEMENT_MARGIN) // 12: subwindow title bar height
#define SETTINGS_ITEM_W (SETTINGS_SUBW_W - (MAIN_ELEMENT_MARGIN * 2))
#define SETTINGS_ITEM_H 8
#define SETTINGS_ITEM_MARGIN 3

#define SETTINGS_EF_H 10
#define SETTINGS_EF_MAXCHARS 255
#define SETTINGS_EF_MAXPAINTCHARS 32

#define SETTINGS_ITEM_POS(pos) SETTINGS_ITEM_X, SETTINGS_ITEM_Y + ((SETTINGS_ITEM_H + SETTINGS_ITEM_MARGIN) * (pos - 1)), SETTINGS_ITEM_W, SETTINGS_ITEM_H
#define SETTINGS_ITEM_POS_REVERSE(pos) SETTINGS_ITEM_X, SETTINGS_SUBW_Y + SETTINGS_SUBW_H - MAIN_ELEMENT_MARGIN + SETTINGS_ITEM_MARGIN - ((SETTINGS_ITEM_MARGIN + SETTINGS_ITEM_H) * (pos + 1)), SETTINGS_ITEM_W, SETTINGS_ITEM_H
#define SETTINGS_EF_POS(pos) SETTINGS_ITEM_X, SETTINGS_ITEM_Y + ((SETTINGS_ITEM_H + SETTINGS_ITEM_MARGIN) * (pos - 1)), SETTINGS_ITEM_W, SETTINGS_EF_H, SETTINGS_ITEM_W - MAIN_ELEMENT_MARGIN

// Bottom buttons
#ifdef FUI
  #define NUM_LOWER_BTNS 2
#else
  #define NUM_LOWER_BTNS 3
#endif

#define BTN_LOWER_W ((WINDOW_WIDTH - ((MAIN_ELEMENT_MARGIN * NUM_LOWER_BTNS) + MAIN_ELEMENT_MARGIN)) / NUM_LOWER_BTNS)
#define BTN_LOWER_H BTN_MAIN_H
#define BTN_LOWER_X BTN_MAIN_X
#define BTN_LOWER_Y (WINDOW_HEIGHT - MAIN_ELEMENT_MARGIN - BTN_LOWER_H)
#define BTN_LOWER_TEXT_ALIGN BTN_MAIN_TEXT_ALIGN

#define LOWERBUTTON(pos, text, cmd) BUTTON(BTN_LOWER_X + ((BTN_LOWER_W + MAIN_ELEMENT_MARGIN) * (pos - 1)), BTN_LOWER_Y, BTN_LOWER_W, BTN_LOWER_H, text, 0.25, BTN_LOWER_TEXT_ALIGN, cmd)

// Helper macros
#define SLIDER_LABEL_X $evalfloat(SETTINGS_ITEM_W - MAIN_ELEMENT_MARGIN)
#define COLOR_LIST cvarStrList { "White"; "white"; "Yellow"; "yellow"; "Red"; "red"; "Green"; "green"; "Blue"; "blue"; "Magenta"; "magenta"; "Cyan"; "cyan"; "Orange"; "orange"; "Light Blue"; "0xa0c0ff"; "Medium Blue"; "mdblue"; "Light Red"; "0xffc0a0"; "Medium Red"; "mdred"; "Light Green"; "0xa0ffc0"; "Medium Green"; "mdgreen"; "Dark Green"; "dkgreen"; "Medium Cyan"; "mdcyan"; "Medium Yellow"; "mdyellow"; "Medium Orange"; "mdorange"; "Light Grey"; "ltgrey"; "Medium Grey"; "mdgrey"; "Dark Grey"; "dkgrey"; "Black"; "black" }
#define CHS_LIST cvarFloatList { "Off" 0 "Speed" 1 "Health" 2 "Ammo" 4 "Distance XY" 10 "Distance Z" 11 "Distance XYZ" 12 "Distance ViewXYZ" 13 "Distance XY Z XYZ" 14 "Distance XY Z ViewXYZ" 15 "Look XYZ" 16 "Speed X" 20 "Speed Y" 21 "Speed Z" 22 "Speed XY" 23 "Speed XYZ" 24 "Speed forward" 25 "Speed sideways" 26 "Speed forward sideways" 27 "Speed XY forward sideways" 28 "Pitch" 30 "Yaw" 31 "Roll" 32 "Position X" 33 "Position Y" 34 "Position Z" 35 "View position X" 36 "View position Y" 37 "View position Z" 38 "Pitch yaw" 40 "Player XYZ" 41 "Player XYZ pitch yaw" 42 "View position XYZ pitch yaw" 43 "Position XYZ" 44 "View position XYZ" 45 "Angles XYZ" 46 "Velocity XYZ" 47 "Jump XYZ" 50 "Plane Angle Z" 53 "Last Jump Speed" 55 }

// 'subwindow*' will greedily match 'subwindowtitle' so we need to explicitly set those colors to keep the subwindow titles intact
#define BACKGROUND_OFF "setitemcolor window backcolor 0 0 0 0; setitemcolor subwindow* backcolor 0 0 0 0; setitemcolor subwindowtitle* backcolor .16 .2 .17 .8"
#define BACKGROUND_ON "setitemcolor window backcolor 0 0 0 .6; setitemcolor subwindow* backcolor 0 0 0 .2; setitemcolor subwindowtitle* backcolor .16 .2 .17 .8"

// Old stuff
// TODO: clean these up and use the new defs in all the menus

// Subwindows
#define SUBW_RECT_LEFT_X 8
#define SUBW_RECT_RIGHT_X 308
#define SUBW_Y 32
#define SUBW_HEADER_HEIGHT 16
#define SUBW_SPACING_Y 4
#define SUBW_WIDTH 292
#define SUBW_ITEM_WIDTH 276
#define SUBW_ITEM_HEIGHT 8
#define SUBW_EDITFIELD_HEIGHT 10
#define SUBW_EDITFIELD_MAXCHARS 128
#define SUBW_EDITFIELD_MAXPAINTCHARS 32
#define SUBW_BIND_HEIGHT 10
#define SUBW_ITEM_SPACING_Y 12
#define SUBW_ITEM_LEFT_X SUBW_RECT_LEFT_X + 8
#define SUBW_ITEM_RIGHT_X SUBW_RECT_RIGHT_X + 8

// Buttons
#define ETJ_BUTTON_X 7
#define ETJ_BUTTON_Y 424
#define ETJ_BUTTON_WIDTH 78
#define ETJ_BUTTON_SPACING_X 86
#define ETJ_BUTTON_HEIGHT 18
#define ETJ_BUTTON_ITEM_HEIGHT 14
