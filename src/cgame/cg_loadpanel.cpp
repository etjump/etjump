#include <vector>

#include "cg_local.h"
#include "../ui/ui_shared.h"

extern displayContextDef_t *DC;

qboolean bg_loadscreeninited;
qboolean bg_loadscreeninteractive;
fontInfo_t bg_loadscreenfont1;
fontInfo_t bg_loadscreenfont2;
qhandle_t bg_loadscreenbg;
qhandle_t bg_axispin;
qhandle_t bg_alliedpin;
qhandle_t bg_neutralpin;
qhandle_t bg_pin;

qhandle_t bg_filter_pb;
qhandle_t bg_filter_ff;
qhandle_t bg_filter_hw;
qhandle_t bg_filter_lv;
qhandle_t bg_filter_al;
qhandle_t bg_filter_bt;

qhandle_t bg_mappic;

// panel_button_text_t FONTNAME = { SCALEX, SCALEY, COLOUR, STYLE, FONT };

panel_button_text_t missiondescriptionTxt = {
    0.2f, 0.2f, {0.0f, 0.0f, 0.0f, 1.f}, 0, 0, &bg_loadscreenfont2,
};

panel_button_text_t missiondescriptionHeaderTxt = {
    0.2f,
    0.2f,
    {0.0f, 0.0f, 0.0f, 0.8f},
    0,
    ITEM_ALIGN_CENTER,
    &bg_loadscreenfont2,
};

panel_button_text_t campaignpheaderTxt = {
    0.2f, 0.2f, {1.0f, 1.0f, 1.0f, 0.6f}, 0, 0, &bg_loadscreenfont2,
};

panel_button_text_t campaignpTxt = {
    0.30f, 0.30f, {1.0f, 1.0f, 1.0f, 0.6f}, 0, 0, &bg_loadscreenfont2,
};

panel_button_text_t loadScreenMeterBackTxt = {
    0.22f,
    0.22f,
    {0.1f, 0.1f, 0.1f, 0.8f},
    0,
    ITEM_ALIGN_CENTER,
    &bg_loadscreenfont2,
};

panel_button_t loadScreenMap = {
    "gfx/loading/camp_map",
    "",
    {0, 0, 440, 480}, // shouldn't this be square?? // Gordon: no, the map is
                      // actually WIDER that tall, which makes it even worse...
    {0, 0, 0, 0, 0, 0, 0, 0},
    NULL, /* font		*/
    NULL, /* keyDown	*/
    NULL, /* keyUp	*/
    BG_PanelButtonsRender_Img,
    NULL,
};

panel_button_t loadScreenBack = {
    "gfx/loading/camp_side",
    "",
    {440, 0, 200, 480},
    {0, 0, 0, 0, 0, 0, 0, 0},
    NULL, /* font		*/
    NULL, /* keyDown	*/
    NULL, /* keyUp	*/
    BG_PanelButtonsRender_Img,
    NULL,
};

panel_button_t loadScreenPins = {
    NULL,
    "",
    {0, 0, 640, 480},
    {0, 0, 0, 0, 0, 0, 0, 0},
    NULL, /* font		*/
    NULL, /* keyDown	*/
    NULL, /* keyUp	*/
    CG_LoadPanel_RenderCampaignPins,
    NULL,
};

panel_button_t missiondescriptionPanelHeaderText = {
    NULL,
    "***TOP SECRET***",
    {440, 72, 200, 32},
    {0, 0, 0, 0, 0, 0, 0, 0},
    &missiondescriptionHeaderTxt, /* font		*/
    NULL,                         /* keyDown	*/
    NULL,                         /* keyUp	*/
    BG_PanelButtonsRender_Text,
    NULL,
};

panel_button_t missiondescriptionPanelText = {
    NULL,
    "",
    {460, 84, 160, 232},
    {0, 0, 0, 0, 0, 0, 0, 0},
    &missiondescriptionTxt, /* font		*/
    NULL,                   /* keyDown	*/
    NULL,                   /* keyUp	*/
    CG_LoadPanel_RenderMissionDescriptionText,
    NULL,
};

panel_button_t campaignPanelText = {
    NULL,
    "",
    {451, 11, 178, 35},
    {0, 0, 0, 0, 0, 0, 0, 0},
    &campaignpTxt, /* font		*/
    NULL,          /* keyDown	*/
    NULL,          /* keyUp	*/
    CG_LoadPanel_RenderCampaignNameText,
    NULL,
};

panel_button_t loadScreenMeterBack = {
    "gfx/loading/progressbar_back",
    "",
    {440 + 26, 480 - 30 + 1, 200 - 56, 20},
    {0, 0, 0, 0, 0, 0, 0, 0},
    NULL, /* font		*/
    NULL, /* keyDown	*/
    NULL, /* keyUp	*/
    BG_PanelButtonsRender_Img,
    NULL,
};

panel_button_t loadScreenMeterBack2 = {
    "gfx/loading/progressbar",
    "",
    {440 + 26, 480 - 30 + 1, 200 - 56, 20},
    {1, 255, 0, 0, 255, 0, 0, 0},
    NULL, /* font		*/
    NULL, /* keyDown	*/
    NULL, /* keyUp	*/
    CG_LoadPanel_RenderLoadingBar,
    NULL,
};

panel_button_t loadScreenMeterBackText = {
    NULL,
    "LOADING",
    {440 + 28, 480 - 28 + 12 + 1, 200 - 56 - 2, 16},
    {0, 0, 0, 0, 0, 0, 0, 0},
    &loadScreenMeterBackTxt, /* font		*/
    NULL,                    /* keyDown	*/
    NULL,                    /* keyUp	*/
    BG_PanelButtonsRender_Text,
    NULL,
};

panel_button_t *loadpanelButtons[] = {
    &loadScreenMap,
    &loadScreenBack,

    &missiondescriptionPanelText,
    &missiondescriptionPanelHeaderText,

    &campaignPanelText,

    &loadScreenMeterBack,
    &loadScreenMeterBack2,
    &loadScreenMeterBackText,

    &loadScreenPins,

    NULL,
};

std::vector<panel_button_t> loadpanelButtonsLayout;

/*
================
CG_DrawConnectScreen
================
*/

void CG_LoadPanel_Init() {
  bg_mappic = 0;
  bg_loadscreeninited = qfalse;
  trap_Cvar_Set("ui_connecting", "0");

  DC->registerFont("ariblk", 27, &bg_loadscreenfont1);
  DC->registerFont("courbd", 30, &bg_loadscreenfont2);

  bg_loadscreenbg = DC->registerShaderNoMip("white");

  bg_axispin = DC->registerShaderNoMip("gfx/loading/pin_axis");
  bg_alliedpin = DC->registerShaderNoMip("gfx/loading/pin_allied");
  bg_neutralpin = DC->registerShaderNoMip("gfx/loading/pin_neutral");
  bg_pin = DC->registerShaderNoMip("gfx/loading/pin_shot");

  bg_filter_pb = DC->registerShaderNoMip("ui/assets/filter_pb");
  bg_filter_ff = DC->registerShaderNoMip("ui/assets/filter_ff");
  bg_filter_hw = DC->registerShaderNoMip("ui/assets/filter_weap");
  bg_filter_lv = DC->registerShaderNoMip("ui/assets/filter_lives");
  bg_filter_al = DC->registerShaderNoMip("ui/assets/filter_antilag");
  bg_filter_bt = DC->registerShaderNoMip("ui/assets/filter_balance");

  loadpanelButtonsLayout.clear();

  for (auto panelBtnPtr : loadpanelButtons) {
    if (panelBtnPtr) {
      loadpanelButtonsLayout.push_back(*panelBtnPtr);
    }
  }

  BG_PanelButtonsSetupWide(loadpanelButtonsLayout);

  bg_loadscreeninited = qtrue;

  CG_DrawConnectScreen(qfalse, qtrue);
}

void CG_DrawConnectScreen(qboolean interactive, qboolean forcerefresh) {
  static qboolean inside = qfalse;
  char buffer[1024];

  bg_loadscreeninteractive = interactive;

  if (!DC) {
    return;
  }

  if (inside || !bg_loadscreeninited) {
    return;
  }

  inside = qtrue;

  vec4_t sideColor = {0.145f, 0.172f, 0.145f, 1.f};
  DC->setColor(sideColor);
  DC->drawHandlePic(0, 0, SCREEN_WIDTH, 480, bg_loadscreenbg);
  DC->setColor(nullptr);

  BG_PanelButtonsRender(loadpanelButtonsLayout);

  if (interactive) {
    DC->drawHandlePic(static_cast<float>(DC->cursorx),
                      static_cast<float>(DC->cursory), 32, 32,
                      DC->Assets.cursor);
  }

  DC->getConfigString(CS_SERVERINFO, buffer, sizeof(buffer));
  if (*buffer) {
    const char *str;
    float x, y;
    int i;
    vec4_t clr3 = {1.f, 1.f, 1.f, .6f};

    y = 322;
    CG_Text_Paint_Centred_Ext(SCREEN_OFFSET_X + 540, y, 0.22f, 0.22f, clr3,
                              GAME_TAG, 0, 0, 0, &bg_loadscreenfont1);

    y = 340;
    str = Info_ValueForKey(buffer, "sv_hostname");
    CG_Text_Paint_Centred_Ext(SCREEN_OFFSET_X + 540, y, 0.2f, 0.2f, colorWhite,
                              str && *str ? str : "ETHost", 0, 26, 0,
                              &bg_loadscreenfont2);

    y += 14;
    for (i = 0; i < MAX_MOTDLINES; i++) {
      str = CG_ConfigString(CS_CUSTMOTD + i);
      if (!str || !*str) {
        break;
      }

      CG_Text_Paint_Centred_Ext(SCREEN_OFFSET_X + 540, y, 0.2f, 0.2f,
                                colorWhite, str, 0, 26, 0, &bg_loadscreenfont2);

      y += 10;
    }

    y = 417;

    str = Info_ValueForKey(buffer, "sv_punkbuster");
    if (str && *str && Q_atoi(str)) {
      x = SCREEN_OFFSET_X + 518;
      CG_DrawPic(x, y, 16, 16, bg_filter_pb);
    }

    str = Info_ValueForKey(buffer, "g_antilag");
    if (str && *str && Q_atoi(str)) {
      x = SCREEN_OFFSET_X + 575;
      CG_DrawPic(x, y, 16, 16, bg_filter_al);
    }
  }

  if (*cgs.rawmapname) {
    if (!bg_mappic) {
      bg_mappic = DC->registerShaderNoMip(va("levelshots/%s", cgs.rawmapname));

      if (!bg_mappic) {
        bg_mappic = DC->registerShaderNoMip("levelshots/unknownmap");
      }
    }

    trap_R_SetColor(colorBlack);
    CG_DrawPic(SCREEN_OFFSET_X + 16 + 1, 2 + 1, 192, 144, bg_mappic);

    trap_R_SetColor(nullptr);
    CG_DrawPic(SCREEN_OFFSET_X + 16, 2, 192, 144, bg_mappic);

    CG_DrawPic(SCREEN_OFFSET_X + 16 + 80, 2 + 6, 20, 20, bg_pin);
  }

  if (forcerefresh) {
    DC->updateScreen();
  }

  inside = qfalse;
}

void CG_LoadPanel_RenderLoadingBar(panel_button_t *button) {
  int hunkused, hunkexpected;
  float frac;

  trap_GetHunkData(&hunkused, &hunkexpected);

  if (hunkexpected <= 0) {
    return;
  }

  frac = hunkused / (float)hunkexpected;
  if (frac < 0.f) {
    frac = 0.f;
  }
  if (frac > 1.f) {
    frac = 1.f;
  }

  CG_DrawPicST(button->rect.x, button->rect.y, button->rect.w * frac,
               button->rect.h, 0, 0, frac, 1, button->hShaderNormal);
}

void CG_LoadPanel_RenderCampaignNameText(panel_button_t *button) {
  const int MAX_LEN = 18; // the box fits roughly 18 chars on default text scale
  float textScale = button->font->scalex;
  int len = Q_PrintStrlen(cgs.arenaData.longname);

  if (!cgs.arenaInfoLoaded) {
    return;
  }

  if (len > MAX_LEN) {
    // theoretically longname can be up to 128 chars, but let's
    // not scale beyond 32 chars to keep it at readable size
    textScale /= (static_cast<float>(len > 32 ? 32 : len) / MAX_LEN);
  }

  auto w = static_cast<float>(CG_Text_Width_Ext(
      cgs.arenaData.longname, textScale, 0, button->font->font));
  auto h = static_cast<float>(CG_Text_Height_Ext(
      cgs.arenaData.longname, textScale, 0, button->font->font));
  CG_Text_Paint_Ext(button->rect.x + (button->rect.w - w) * 0.5f,
                    button->rect.y + (button->rect.h + h) * 0.5f, textScale,
                    textScale, button->font->colour, cgs.arenaData.longname, 0,
                    0, 0, button->font->font);
}

void CG_LoadPanel_RenderMissionDescriptionText(panel_button_t *button) {
  const char *cs;
  char *s, *p;
  char buffer[1024];
  float y;

  if (!cgs.arenaInfoLoaded) {
    return;
  }

  cs = cgs.arenaData.description;

  Q_strncpyz(buffer, cs, sizeof(buffer));
  for (s = strchr(buffer, '*'); s; s = strchr(buffer, '*')) {
    *s = '\n';
  }

  BG_FitTextToWidth_Ext(buffer, button->font->scalex, button->rect.w - 16,
                        sizeof(buffer), button->font->font);

  y = button->rect.y + 12;

  s = p = buffer;
  while (*p) {
    if (*p == '\n') {
      *p++ = '\0';
      DC->drawTextExt(button->rect.x + 4, y, button->font->scalex,
                      button->font->scaley, button->font->colour, s, 0, 0, 0,
                      button->font->font);
      y += 8;
      s = p;
    } else {
      p++;
    }
  }
}

void CG_LoadPanel_KeyHandling(int key, qboolean down) {
  if (BG_PanelButtonsKeyEvent(key, down, loadpanelButtonsLayout)) {
    return;
  }
}

qboolean CG_LoadPanel_ContinueButtonKeyDown(panel_button_t *button, int key) {
  if (key == K_MOUSE1) {
    CG_EventHandling(CGAME_EVENT_GAMEVIEW, qfalse);
    return qtrue;
  }

  return qfalse;
}

void CG_LoadPanel_DrawPin(const char *text, float px, float py, float sx,
                          float sy, qhandle_t shader, float pinsize,
                          float backheight) {
  vec4_t colourFadedBlack = {0.f, 0.f, 0.f, 0.4f};
  const auto textWidth = DC->textWidthExt(text, sx, 0, &bg_loadscreenfont2);

  if (px + 30 + textWidth > 440) {
    DC->fillRect(SCREEN_OFFSET_X + px - textWidth - 28 + 2,
                 py - (backheight / 2.f) + 2, 28 + textWidth, backheight,
                 colourFadedBlack);
    DC->fillRect(SCREEN_OFFSET_X + px - textWidth - 28, py - (backheight / 2.f),
                 28 + textWidth, backheight, colorBlack);
  } else {
    DC->fillRect(SCREEN_OFFSET_X + px + 2, py - (backheight / 2.f) + 2,
                 28 + textWidth, backheight, colourFadedBlack);
    DC->fillRect(SCREEN_OFFSET_X + px, py - (backheight / 2.f), 28 + textWidth,
                 backheight, colorBlack);
  }

  DC->drawHandlePic(SCREEN_OFFSET_X + px - pinsize, py - pinsize, pinsize * 2.f,
                    pinsize * 2.f, shader);

  if (px + 30 + textWidth > 440) {
    DC->drawTextExt(SCREEN_OFFSET_X + px - 20 - textWidth, py + 4, sx, sy,
                    colorWhite, text, 0, 0, 0, &bg_loadscreenfont2);
  } else {
    DC->drawTextExt(SCREEN_OFFSET_X + px + 16, py + 4, sx, sy, colorWhite, text,
                    0, 0, 0, &bg_loadscreenfont2);
  }
}

void CG_LoadPanel_RenderCampaignPins(panel_button_t *button) {
  float px, py;

  if (!cgs.arenaInfoLoaded) {
    return;
  }

  px = (cgs.arenaData.mappos[0] / 1024.f) * 440.f;
  py = (cgs.arenaData.mappos[1] / 1024.f) * 480.f;

  CG_LoadPanel_DrawPin(cgs.arenaData.longname, px, py, 0.22f, 0.25f,
                       bg_neutralpin, 16.f, 16.f);
}
