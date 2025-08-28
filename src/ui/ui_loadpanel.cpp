#include <vector>

#include "ui_local.h"
#include "ui_shared.h"

#include "../cgame/etj_utilities.h"

extern displayContextDef_t *DC;

fontInfo_t bg_loadscreenfont1;
fontInfo_t bg_loadscreenfont2;

void UI_LoadPanel_RenderHeaderText(panel_button_t *button);
void UI_LoadPanel_RenderLoadingText(panel_button_t *button);

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

panel_button_text_t mapTxt = {
    0.35f, 0.35f, {1.0f, 1.0f, 1.0f, 0.6f}, 0, 0, &bg_loadscreenfont2,
};

panel_button_t loadScreenMap = {
    "gfx/loading/camp_map",
    "",
    {0, 0, 440, 480}, // shouldn't this be square??
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

panel_button_t loadingPanelText = {
    NULL,
    "",
    {460, 72, 160, 244},
    {0, 0, 0, 0, 0, 0, 0, 0},
    &missiondescriptionTxt, /* font		*/
    NULL,                   /* keyDown	*/
    NULL,                   /* keyUp	*/
    UI_LoadPanel_RenderLoadingText,
    NULL,
};

panel_button_t mapPanelText = {
    NULL,
    "", //"CONNECTING...",
    {451, 11, 178, 35},
    {0, 0, 0, 0, 0, 0, 0, 0},
    &mapTxt, /* font		*/
    NULL,    /* keyDown	*/
    NULL,    /* keyUp	*/
    UI_LoadPanel_RenderHeaderText,
    NULL,
};

panel_button_t *loadpanelButtons[] = {
    &loadScreenMap, &loadScreenBack, &loadingPanelText, &mapPanelText, nullptr,
};

std::vector<panel_button_t> loadpanelButtonsLayout;

void UI_LoadPanel_Init() {
  trap_R_RegisterFont("ariblk", 27, &bg_loadscreenfont1);
  trap_R_RegisterFont("courbd", 30, &bg_loadscreenfont2);

  loadpanelButtonsLayout.clear();

  for (auto panelBtnPtr : loadpanelButtons) {
    if (panelBtnPtr) {
      loadpanelButtonsLayout.push_back(*panelBtnPtr);
    }
  }

  BG_PanelButtonsSetupWide(loadpanelButtonsLayout);
}

/*
================
CG_DrawConnectScreen
================
*/
static qboolean connect_ownerdraw;
void UI_DrawLoadPanel(qboolean forcerefresh, qboolean ownerdraw,
                      qboolean uihack) {
  static qboolean inside = qfalse;

  if (inside) {
    if (!uihack && trap_Cvar_VariableValue("ui_connecting")) {
      trap_Cvar_Set("ui_connecting", "0");
    }
    return;
  }

  connect_ownerdraw = ownerdraw;

  inside = qtrue;

  // side frames to block view
  vec4_t sideColor = {0.145f, 0.172f, 0.145f, 1.f};
  uiInfo.uiDC.fillRect(0, 0, SCREEN_OFFSET_X, 480, sideColor);
  uiInfo.uiDC.fillRect(SCREEN_OFFSET_X + 640, 0, SCREEN_OFFSET_X, 480,
                       sideColor);

  BG_PanelButtonsRender(loadpanelButtonsLayout);

  if (forcerefresh) {
    // trap_UpdateScreen();
  }

  if (!uihack && trap_Cvar_VariableValue("ui_connecting")) {
    trap_Cvar_Set("ui_connecting", "0");
  }

  inside = qfalse;
}

void MiniAngleToAxis(vec_t angle, vec2_t axes[2]) {
  axes[0][0] = (vec_t)sin(-angle);
  axes[0][1] = -(vec_t)cos(-angle);

  axes[1][0] = -axes[0][1];
  axes[1][1] = axes[0][0];
}

void SetupRotatedThing(polyVert_t *verts, vec2_t org, float w, float h,
                       vec_t angle) {
  vec2_t axes[2];

  MiniAngleToAxis(angle, axes);

  verts[0].xyz[0] = org[0] - (w * 0.5f) * axes[0][0];
  verts[0].xyz[1] = org[1] - (w * 0.5f) * axes[0][1];
  verts[0].xyz[2] = 0;
  verts[0].st[0] = 0;
  verts[0].st[1] = 1;
  verts[0].modulate[0] = 255;
  verts[0].modulate[1] = 255;
  verts[0].modulate[2] = 255;
  verts[0].modulate[3] = 255;

  verts[1].xyz[0] = verts[0].xyz[0] + w * axes[0][0];
  verts[1].xyz[1] = verts[0].xyz[1] + w * axes[0][1];
  verts[1].xyz[2] = 0;
  verts[1].st[0] = 1;
  verts[1].st[1] = 1;
  verts[1].modulate[0] = 255;
  verts[1].modulate[1] = 255;
  verts[1].modulate[2] = 255;
  verts[1].modulate[3] = 255;

  verts[2].xyz[0] = verts[1].xyz[0] + h * axes[1][0];
  verts[2].xyz[1] = verts[1].xyz[1] + h * axes[1][1];
  verts[2].xyz[2] = 0;
  verts[2].st[0] = 1;
  verts[2].st[1] = 0;
  verts[2].modulate[0] = 255;
  verts[2].modulate[1] = 255;
  verts[2].modulate[2] = 255;
  verts[2].modulate[3] = 255;

  verts[3].xyz[0] = verts[2].xyz[0] - w * axes[0][0];
  verts[3].xyz[1] = verts[2].xyz[1] - w * axes[0][1];
  verts[3].xyz[2] = 0;
  verts[3].st[0] = 0;
  verts[3].st[1] = 0;
  verts[3].modulate[0] = 255;
  verts[3].modulate[1] = 255;
  verts[3].modulate[2] = 255;
  verts[3].modulate[3] = 255;
}

void UI_LoadPanel_RenderHeaderText(panel_button_t *button) {
  uiClientState_t cstate;
  char downloadName[MAX_INFO_VALUE];

  trap_GetClientState(&cstate);

  trap_Cvar_VariableStringBuffer("cl_downloadName", downloadName,
                                 sizeof(downloadName));

  if ((cstate.connState == CA_DISCONNECTED ||
       cstate.connState == CA_CONNECTED) &&
      *downloadName) {
    button->text = "DOWNLOADING...";
  } else {
    button->text = "CONNECTING...";
  }

  // we can't really do this in the rendering function
  // since it's used to render other things too

  // restore default values first
  button->rect.x = 451 + SCREEN_OFFSET_X;
  button->rect.y = 11;

  auto w = static_cast<float>(DC->textWidthExt(
      button->text.c_str(), button->font->scalex, 0, button->font->font));
  auto h = static_cast<float>(DC->textHeightExt(
      button->text.c_str(), button->font->scaley, 0, button->font->font));
  button->rect.x = button->rect.x + (button->rect.w - w) * 0.5f;
  button->rect.y = button->rect.y + (button->rect.h + h) * 0.5f;

  BG_PanelButtonsRender_Text(button);
}

inline constexpr int ESTIMATES = 80;

const char *UI_DownloadInfo(const char *downloadName) {
  static char dlText[] = "Downloading:";
  static char etaText[] = "Estimated time left:";
  static char xferText[] = "Transfer rate:";
  static int tleEstimates[ESTIMATES] = {
      60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
      60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
      60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
      60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
      60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60};
  static int tleIndex = 0;

  char dlSizeBuf[64], totalSizeBuf[64], xferRateBuf[64], dlTimeBuf[64];
  int downloadSize, downloadCount, downloadTime;
  int xferRate;
  const char *s, *ds;

  downloadSize = trap_Cvar_VariableValue("cl_downloadSize");
  downloadCount = trap_Cvar_VariableValue("cl_downloadCount");
  downloadTime = trap_Cvar_VariableValue("cl_downloadTime");

  if (downloadSize > 0) {
    ds = va("%s (%d%%)", downloadName,
            (int)((float)downloadCount * 100.0f / (float)downloadSize));
  } else {
    ds = downloadName;
  }

  UI_ReadableSize(dlSizeBuf, sizeof dlSizeBuf, downloadCount);
  UI_ReadableSize(totalSizeBuf, sizeof totalSizeBuf, downloadSize);

  if (downloadCount < 4096 || !downloadTime) {
    s = va("%s\n %s\n%s\n\n%s\n estimating...\n\n%s\n\n%s copied", dlText, ds,
           totalSizeBuf, etaText, xferText, dlSizeBuf);
    return s;
  } else {
    if ((uiInfo.uiDC.realTime - downloadTime) / 1000) {
      xferRate = downloadCount / ((uiInfo.uiDC.realTime - downloadTime) / 1000);
    } else {
      xferRate = 0;
    }
    UI_ReadableSize(xferRateBuf, sizeof xferRateBuf, xferRate);

    // Extrapolate estimated completion time
    if (downloadSize && xferRate) {
      int n = downloadSize / xferRate; // estimated time for
                                       // entire d/l in secs
      int timeleft = 0, i;

      // We do it in K (/1024) because we'd overflow
      // around 4MB
      tleEstimates[tleIndex] =
          (n - (((downloadCount / 1024) * n) / (downloadSize / 1024)));
      tleIndex++;
      if (tleIndex >= ESTIMATES) {
        tleIndex = 0;
      }

      for (i = 0; i < ESTIMATES; i++) timeleft += tleEstimates[i];

      timeleft /= ESTIMATES;

      UI_PrintTime(dlTimeBuf, sizeof dlTimeBuf, timeleft);
    } else {
      dlTimeBuf[0] = '\0';
    }

    if (xferRate) {
      s = va("%s\n %s\n%s\n\n%s\n %s\n\n%s\n "
             "%s/sec\n\n%s copied",
             dlText, ds, totalSizeBuf, etaText, dlTimeBuf, xferText,
             xferRateBuf, dlSizeBuf);
    } else {
      if (downloadSize) {
        s = va("%s\n %s\n%s\n\n%s\n "
               "estimating...\n\n%s\n\n%s copied",
               dlText, ds, totalSizeBuf, etaText, xferText, dlSizeBuf);
      } else {
        s = va("%s\n %s\n\n%s\n "
               "estimating...\n\n%s\n\n%s copied",
               dlText, ds, etaText, xferText, dlSizeBuf);
      }
    }

    return s;
  }

  return "";
}

void UI_LoadPanel_RenderLoadingText(panel_button_t *button) {
  uiClientState_t cstate;
  char downloadName[MAX_INFO_VALUE];
  char buff[2560];
  char *p;
  const char *s = "";
  float y;

  trap_GetClientState(&cstate);

  Com_sprintf(buff, sizeof(buff), "Connecting to:\n %s^*\n\n%s",
              cstate.servername,
              Info_ValueForKey(cstate.updateInfoString, "motd"));

  // Com_sprintf( buff, sizeof(buff), "%s^*", cstate.servername,
  // Info_ValueForKey( cstate.updateInfoString, "motd" ) );

  trap_Cvar_VariableStringBuffer("cl_downloadName", downloadName,
                                 sizeof(downloadName));

  if (!connect_ownerdraw) {
    if (!trap_Cvar_VariableValue("ui_connecting")) {
      switch (cstate.connState) {
        case CA_CONNECTING:
          s = va("Awaiting connection...%i", cstate.connectPacketCount);
          break;
        case CA_CHALLENGING:
          s = va("Awaiting challenge...%i", cstate.connectPacketCount);
          break;
        case CA_DISCONNECTED:
        case CA_CONNECTED:
          if (*downloadName || cstate.connState == CA_DISCONNECTED) {
            s = UI_DownloadInfo(downloadName);
          } else {
            s = "Awaiting gamestate...";
          }
          break;
        case CA_LOADING:
        case CA_PRIMED:
        default:
          break;
      }
    } else if (trap_Cvar_VariableValue("ui_dl_running")) {
      // only toggle during a disconnected download
      s = UI_DownloadInfo(downloadName);
    }

    Q_strcat(buff, sizeof(buff), va("\n\n%s^*", s));

    if (cstate.connState < CA_CONNECTED && *cstate.messageString) {
      Q_strcat(buff, sizeof(buff), va("\n\n%s^*", cstate.messageString));
    }
  }

  BG_FitTextToWidth_Ext(buff, button->font->scalex, button->rect.w,
                        sizeof(buff), button->font->font);

  // UI_DrawRect( button->rect.x, button->rect.y, button->rect.w,
  // button->rect.h, colorRed );

  y = button->rect.y + 12;

  s = p = buff;

  while (*p) {
    if (*p == '\n') {
      *p++ = '\0';
      Text_Paint_Ext(button->rect.x + 4, y, button->font->scalex,
                     button->font->scaley, button->font->colour, s, 0, 0, 0,
                     button->font->font);
      y += 8;
      s = p;
    } else {
      p++;
    }
  }
}
