#include <vector>

#include "cg_local.h"
#include "../ui/ui_shared.h"

extern displayContextDef_t *DC;

qboolean   bg_loadscreeninited;
qboolean   bg_loadscreeninteractive;
fontInfo_t bg_loadscreenfont1;
fontInfo_t bg_loadscreenfont2;
qhandle_t  bg_loadscreenbg;
qhandle_t  bg_axispin;
qhandle_t  bg_alliedpin;
qhandle_t  bg_neutralpin;
qhandle_t  bg_pin;

qhandle_t bg_filter_pb;
qhandle_t bg_filter_ff;
qhandle_t bg_filter_hw;
qhandle_t bg_filter_lv;
qhandle_t bg_filter_al;
qhandle_t bg_filter_bt;

qhandle_t bg_mappic;

// panel_button_text_t FONTNAME = { SCALEX, SCALEY, COLOUR, STYLE, FONT };

panel_button_text_t missiondescriptionTxt =
{
	0.2f,                0.2f,
	{ 0.0f,              0.0f,0.0f,    1.f },
	0,                   0,
	&bg_loadscreenfont2,
};

panel_button_text_t missiondescriptionHeaderTxt =
{
	0.2f,                0.2f,
	{ 0.0f,              0.0f,             0.0f,    0.8f },
	0,                   ITEM_ALIGN_CENTER,
	&bg_loadscreenfont2,
};

panel_button_text_t campaignpheaderTxt =
{
	0.2f,                0.2f,
	{ 1.0f,              1.0f,1.0f,    0.6f },
	0,                   0,
	&bg_loadscreenfont2,
};

panel_button_text_t campaignpTxt =
{
	0.30f,               0.30f,
	{ 1.0f,              1.0f, 1.0f,  0.6f },
	0,                   0,
	&bg_loadscreenfont2,
};

panel_button_text_t loadScreenMeterBackTxt =
{
	0.22f,               0.22f,
	{ 0.1f,              0.1f,             0.1f,  0.8f },
	0,                   ITEM_ALIGN_CENTER,
	&bg_loadscreenfont2,
};

panel_button_t loadScreenMap =
{
	"gfx/loading/camp_map",
	NULL,
	{ 0,                      0,  440, 480 }, // shouldn't this be square?? // Gordon: no, the map is actually WIDER that tall, which makes it even worse...
	{ 0,                      0,  0,   0, 0, 0, 0, 0},
	NULL,                     /* font		*/
	NULL,                     /* keyDown	*/
	NULL,                     /* keyUp	*/
	BG_PanelButtonsRender_Img,
	NULL,
};

panel_button_t loadScreenBack =
{
	"gfx/loading/camp_side",
	NULL,
	{ 440,                    0,  200, 480 },
	{ 0,                      0,  0,   0, 0, 0, 0, 0},
	NULL,                     /* font		*/
	NULL,                     /* keyDown	*/
	NULL,                     /* keyUp	*/
	BG_PanelButtonsRender_Img,
	NULL,
};

panel_button_t loadScreenPins =
{
	NULL,
	NULL,
	{ 0,                            0,  640, 480 },
	{ 0,                            0,  0,   0, 0, 0, 0, 0},
	NULL,                           /* font		*/
	NULL,                           /* keyDown	*/
	NULL,                           /* keyUp	*/
	CG_LoadPanel_RenderCampaignPins,
	NULL,
};

panel_button_t missiondescriptionPanelHeaderText =
{
	NULL,
	"***TOP SECRET***",
	{ 440,                     72, 200, 32 },
	{ 0,                       0,  0,   0, 0, 0, 0, 0},
	&missiondescriptionHeaderTxt,/* font		*/
	NULL,                      /* keyDown	*/
	NULL,                      /* keyUp	*/
	BG_PanelButtonsRender_Text,
	NULL,
};

panel_button_t missiondescriptionPanelText =
{
	NULL,
	NULL,
	{ 460,                                    84,   160, 232 },
	{ 0,                                      0,    0,   0, 0, 0, 0, 0},
	&missiondescriptionTxt,                   /* font		*/
	NULL,                                     /* keyDown	*/
	NULL,                                     /* keyUp	*/
	CG_LoadPanel_RenderMissionDescriptionText,
	NULL,
};

panel_button_t campaignheaderPanelText =
{
	NULL,
	NULL,
	{ 456,                              24,   152, 232 },
	{ 0,                                0,    0,   0, 0, 0, 0, 0},
	&campaignpheaderTxt,                /* font		*/
	NULL,                               /* keyDown	*/
	NULL,                               /* keyUp	*/
	CG_LoadPanel_RenderCampaignTypeText,
	NULL,
};

panel_button_t campaignPanelText =
{
	NULL,
	NULL,
	{ 464,                              40,   152, 232 },
	{ 0,                                0,    0,   0, 0, 0, 0, 0},
	&campaignpTxt,                      /* font		*/
	NULL,                               /* keyDown	*/
	NULL,                               /* keyUp	*/
	CG_LoadPanel_RenderCampaignNameText,
	NULL,
};

panel_button_t loadScreenMeterBack =
{
	"gfx/loading/progressbar_back",
	NULL,
	{ 440 + 26,                    480 - 30 + 1,200 - 56, 20 },
	{ 0,                           0,  0,        0, 0, 0, 0, 0},
	NULL,                          /* font		*/
	NULL,                          /* keyDown	*/
	NULL,                          /* keyUp	*/
	BG_PanelButtonsRender_Img,
	NULL,
};

panel_button_t loadScreenMeterBack2 =
{
	"gfx/loading/progressbar",
	NULL,
	{ 440 + 26,                   480 - 30 + 1,200 - 56, 20 },
	{ 1,                          255,  0,        0, 255, 0, 0, 0},
	NULL,                         /* font		*/
	NULL,                         /* keyDown	*/
	NULL,                         /* keyUp	*/
	CG_LoadPanel_RenderLoadingBar,
	NULL,
};

panel_button_t loadScreenMeterBackText =
{
	NULL,
	"LOADING",
	{ 440 + 28,                480 - 28 + 12 + 1,   200 - 56 - 2, 16 },
	{ 0,                       0,                   0,            0, 0, 0, 0, 0},
	&loadScreenMeterBackTxt,   /* font		*/
	NULL,                      /* keyDown	*/
	NULL,                      /* keyUp	*/
	BG_PanelButtonsRender_Text,
	NULL,
};

panel_button_t *loadpanelButtons[] =
{
	&loadScreenMap,               &loadScreenBack,


	&missiondescriptionPanelText, &missiondescriptionPanelHeaderText,

	&campaignheaderPanelText,     &campaignPanelText,

	&loadScreenMeterBack,         &loadScreenMeterBack2,             &loadScreenMeterBackText,

	&loadScreenPins,

	NULL,
};

std::vector<panel_button_t> loadpanelButtonsLayout;

/*
================
CG_DrawConnectScreen
================
*/

const char *CG_LoadPanel_GameTypeName(gametype_t gt)
{
	switch (gt)
	{
	case GT_SINGLE_PLAYER:
		return "Single Player";
	case GT_COOP:
		return "Co-op";
	case GT_WOLF:
		return "Objective";
	case GT_WOLF_STOPWATCH:
		return "Stopwatch";
	case GT_WOLF_CAMPAIGN:
		return "Campaign";
	case GT_WOLF_LMS:
		return "Last Man Standing";
	default:
		break;
	}

	return "Invalid";
}

void CG_LoadPanel_Init()
{
	bg_mappic = 0;
	bg_loadscreeninited = qfalse;
	trap_Cvar_Set("ui_connecting", "0");

	DC->registerFont("ariblk", 27, &bg_loadscreenfont1);
	DC->registerFont("courbd", 30, &bg_loadscreenfont2);

	bg_loadscreenbg = DC->registerShaderNoMip("white");

	bg_axispin    = DC->registerShaderNoMip("gfx/loading/pin_axis");
	bg_alliedpin  = DC->registerShaderNoMip("gfx/loading/pin_allied");
	bg_neutralpin = DC->registerShaderNoMip("gfx/loading/pin_neutral");
	bg_pin        = DC->registerShaderNoMip("gfx/loading/pin_shot");


	bg_filter_pb = DC->registerShaderNoMip("ui/assets/filter_pb");
	bg_filter_ff = DC->registerShaderNoMip("ui/assets/filter_ff");
	bg_filter_hw = DC->registerShaderNoMip("ui/assets/filter_weap");
	bg_filter_lv = DC->registerShaderNoMip("ui/assets/filter_lives");
	bg_filter_al = DC->registerShaderNoMip("ui/assets/filter_antilag");
	bg_filter_bt = DC->registerShaderNoMip("ui/assets/filter_balance");

	loadpanelButtonsLayout.clear();

	for (auto panelBtnPtr : loadpanelButtons)
	{
		if (panelBtnPtr) 
		{
			loadpanelButtonsLayout.push_back(*panelBtnPtr);
		}
	}

	BG_PanelButtonsSetupWide(loadpanelButtonsLayout);

	bg_loadscreeninited = qtrue;

	CG_DrawConnectScreen(qfalse, qtrue);
}

void CG_DrawConnectScreen(qboolean interactive, qboolean forcerefresh)
{
	static qboolean inside = qfalse;
	char            buffer[1024];

	bg_loadscreeninteractive = interactive;

	if (!DC)
	{
		return;
	}

	if (inside || !bg_loadscreeninited)
	{
		return;
	}

	inside = qtrue;

	vec4_t sideColor = { 0.145f, 0.172f, 0.145f, 1.f };
	DC->setColor(sideColor);
	DC->drawHandlePic(0, 0, SCREEN_WIDTH, 480, bg_loadscreenbg);
	DC->setColor(NULL);

	BG_PanelButtonsRender(loadpanelButtonsLayout);

	if (interactive)
	{
		DC->drawHandlePic(DC->cursorx, DC->cursory, 32, 32, DC->Assets.cursor);
	}

	DC->getConfigString(CS_SERVERINFO, buffer, sizeof(buffer));
	if (*buffer)
	{
		const char *str;
		qboolean   enabled = qfalse;
		float      x, y;
		int        i;
//		vec4_t clr1 = { 41/255.f,	51/255.f,	43/255.f,	204/255.f };
//		vec4_t clr2 = { 0.f,		0.f,		0.f,		225/255.f };
		vec4_t clr3 = { 1.f, 1.f, 1.f, .6f };

/*		CG_FillRect( 8, 8, 230, 16, clr1 );
        CG_DrawRect_FixedBorder( 8, 8, 230, 16, 1, colorMdGrey );

        CG_FillRect( 8, 23, 230, 210, clr2 );
        CG_DrawRect_FixedBorder( 8, 23, 230, 216, 1, colorMdGrey );*/
		y = 322;
		CG_Text_Paint_Centred_Ext(SCREEN_OFFSET_X + 540, y, 0.22f, 0.22f, clr3, GAME_TAG, 0, 0, 0, &bg_loadscreenfont1);

		y   = 340;
		str = Info_ValueForKey(buffer, "sv_hostname");
		CG_Text_Paint_Centred_Ext(SCREEN_OFFSET_X + 540, y, 0.2f, 0.2f, colorWhite, str && *str ? str : "ETHost", 0, 26, 0, &bg_loadscreenfont2);


		y += 14;
		for (i = 0; i < MAX_MOTDLINES; i++)
		{
			str = CG_ConfigString(CS_CUSTMOTD + i);
			if (!str || !*str)
			{
				break;
			}

			CG_Text_Paint_Centred_Ext(SCREEN_OFFSET_X + 540, y, 0.2f, 0.2f, colorWhite, str, 0, 26, 0, &bg_loadscreenfont2);

			y += 10;
		}

		y = 417;

		if (enabled)
		{
			x = SCREEN_OFFSET_X + 489;
			CG_DrawPic(x, y, 16, 16, bg_filter_lv);
		}

		str = Info_ValueForKey(buffer, "sv_punkbuster");
		if (str && *str && atoi(str))
		{
			x = SCREEN_OFFSET_X + 518;
			CG_DrawPic(x, y, 16, 16, bg_filter_pb);
		}

		str = Info_ValueForKey(buffer, "g_antilag");
		if (str && *str && atoi(str))
		{
			x = SCREEN_OFFSET_X + 575;
			CG_DrawPic(x, y, 16, 16, bg_filter_al);
		}
	}

	if (*cgs.rawmapname)
	{
		if (!bg_mappic)
		{
			bg_mappic = DC->registerShaderNoMip(va("levelshots/%s", cgs.rawmapname));

			if (!bg_mappic)
			{
				bg_mappic = DC->registerShaderNoMip("levelshots/unknownmap");
			}
		}

		trap_R_SetColor(colorBlack);
		CG_DrawPic(SCREEN_OFFSET_X + 16 + 1, 2 + 1, 192, 144, bg_mappic);

		trap_R_SetColor(NULL);
		CG_DrawPic(SCREEN_OFFSET_X + 16, 2, 192, 144, bg_mappic);

		CG_DrawPic(SCREEN_OFFSET_X + 16 + 80, 2 + 6, 20, 20, bg_pin);
	}

	if (forcerefresh)
	{
		DC->updateScreen();
	}

	inside = qfalse;
}

void CG_LoadPanel_RenderLoadingBar(panel_button_t *button)
{
	int   hunkused, hunkexpected;
	float frac;

	trap_GetHunkData(&hunkused, &hunkexpected);

	if (hunkexpected <= 0)
	{
		return;
	}

	frac = hunkused / (float)hunkexpected;
	if (frac < 0.f)
	{
		frac = 0.f;
	}
	if (frac > 1.f)
	{
		frac = 1.f;
	}

	CG_DrawPicST(button->rect.x, button->rect.y, button->rect.w * frac, button->rect.h, 0, 0, frac, 1, button->hShaderNormal);
}

void CG_LoadPanel_RenderCampaignTypeText(panel_button_t *button)
{
/*	char buffer[1024];
    const char* str;
    DC->getConfigString( CS_SERVERINFO, buffer, sizeof( buffer ) );
    if( !*buffer ) {
        return;
    }

    str = Info_ValueForKey( buffer, "g_gametype" );
*/
	CG_Text_Paint_Ext(button->rect.x, button->rect.y, button->font->scalex, button->font->scaley, button->font->colour, va("%s:", CG_LoadPanel_GameTypeName(cgs.gametype)), 0, 0, button->font->style, button->font->font);
}


void CG_LoadPanel_RenderCampaignNameText(panel_button_t *button)
{
	const char *cs;
	float      w;

	//char buffer[1024];
	//int gametype;

	//DC->getConfigString( CS_SERVERINFO, buffer, sizeof( buffer ) );
	//cs = Info_ValueForKey( buffer, "g_gametype" );
	//gametype = atoi(cs);

	if (cgs.gametype == GT_WOLF_CAMPAIGN)
	{

		cs = DC->nameForCampaign();
		if (!cs)
		{
			return;
		}

		cs = va("%s %iof%i", cs, cgs.currentCampaignMap + 1, cgs.campaignData.mapCount);

		w = CG_Text_Width_Ext(cs, button->font->scalex, 0, button->font->font);
		CG_Text_Paint_Ext(button->rect.x + (button->rect.w - w) * 0.5f, button->rect.y, button->font->scalex, button->font->scaley, button->font->colour, cs, 0, 0, 0, button->font->font);

	}
	else
	{

		if (!cgs.arenaInfoLoaded)
		{
			return;
		}

		w = CG_Text_Width_Ext(cgs.arenaData.longname, button->font->scalex, 0, button->font->font);
		CG_Text_Paint_Ext(button->rect.x + (button->rect.w - w) * 0.5f, button->rect.y, button->font->scalex, button->font->scaley, button->font->colour, cgs.arenaData.longname, 0, 0, 0, button->font->font);
	}
}

void CG_LoadPanel_RenderMissionDescriptionText(panel_button_t *button)
{
	const char *cs;
	char       *s, *p;
	char       buffer[1024];
	float      y;

	//int gametype;

	//DC->getConfigString( CS_SERVERINFO, buffer, sizeof( buffer ) );
	//cs = Info_ValueForKey( buffer, "g_gametype" );
	//gametype = atoi(cs);

//	DC->fillRect( button->rect.x, button->rect.y, button->rect.w, button->rect.h, colorRed );

	if (cgs.gametype == GT_WOLF_CAMPAIGN)
	{

		cs = DC->descriptionForCampaign();
		if (!cs)
		{
			return;
		}

	}
	else if (cgs.gametype == GT_WOLF_LMS)
	{

		//cs = CG_ConfigString( CS_MULTI_MAPDESC3 );

		if (!cgs.arenaInfoLoaded)
		{
			return;
		}

		cs = cgs.arenaData.lmsdescription;

	}
	else
	{

		if (!cgs.arenaInfoLoaded)
		{
			return;
		}

		cs = cgs.arenaData.description;
	}

	Q_strncpyz(buffer, cs, sizeof(buffer));
	for (s = strchr(buffer, '*'); s; s = strchr(buffer, '*'))
	{
		*s = '\n';
	}

	BG_FitTextToWidth_Ext(buffer, button->font->scalex, button->rect.w - 16, sizeof(buffer), button->font->font);

	y = button->rect.y + 12;

	s = p = buffer;
	while (*p)
	{
		if (*p == '\n')
		{
			*p++ = '\0';
			DC->drawTextExt(button->rect.x + 4, y, button->font->scalex, button->font->scaley, button->font->colour, s, 0, 0, 0, button->font->font);
			y += 8;
			s  = p;
		}
		else
		{
			p++;
		}
	}
}

void CG_LoadPanel_KeyHandling(int key, qboolean down)
{
	if (BG_PanelButtonsKeyEvent(key, down, loadpanelButtonsLayout))
	{
		return;
	}
}

qboolean CG_LoadPanel_ContinueButtonKeyDown(panel_button_t *button, int key)
{
	if (key == K_MOUSE1)
	{
		CG_EventHandling(CGAME_EVENT_GAMEVIEW, qfalse);
		return qtrue;
	}

	return qfalse;
}


void CG_LoadPanel_DrawPin(const char *text, float px, float py, float sx, float sy, qhandle_t shader, float pinsize, float backheight)
{
	vec4_t colourFadedBlack = { 0.f, 0.f, 0.f, 0.4f };
	const auto textWidth = DC->textWidthExt(text, sx, 0, &bg_loadscreenfont2);

	if (px + 30 + textWidth > 440)
	{
		DC->fillRect(SCREEN_OFFSET_X + px - textWidth - 28 + 2, py - (backheight / 2.f) + 2, 28 + textWidth, backheight, colourFadedBlack);
		DC->fillRect(SCREEN_OFFSET_X + px - textWidth - 28, py - (backheight / 2.f), 28 + textWidth, backheight, colorBlack);
	}
	else
	{
		DC->fillRect(SCREEN_OFFSET_X + px + 2, py - (backheight / 2.f) + 2, 28 + textWidth, backheight, colourFadedBlack);
		DC->fillRect(SCREEN_OFFSET_X + px, py - (backheight / 2.f), 28 + textWidth, backheight, colorBlack);
	}

	DC->drawHandlePic(SCREEN_OFFSET_X + px - pinsize, py - pinsize, pinsize * 2.f, pinsize * 2.f, shader);

	if (px + 30 + textWidth > 440)
	{
		DC->drawTextExt(SCREEN_OFFSET_X + px - 20 - textWidth, py + 4, sx, sy, colorWhite, text, 0, 0, 0, &bg_loadscreenfont2);
	}
	else
	{
		DC->drawTextExt(SCREEN_OFFSET_X + px + 16, py + 4, sx, sy, colorWhite, text, 0, 0, 0, &bg_loadscreenfont2);
	}
}

void CG_LoadPanel_RenderCampaignPins(panel_button_t *button)
{
	int       i;
	qhandle_t shader;
	/*char buffer[1024];
	char *s;
	int gametype;

	DC->getConfigString( CS_SERVERINFO, buffer, sizeof( buffer ) );
	s = Info_ValueForKey( buffer, "g_gametype" );
	gametype = atoi(s);*/

	if (cgs.gametype == GT_WOLF_STOPWATCH || cgs.gametype == GT_WOLF_LMS || cgs.gametype == GT_WOLF)
	{
		float px, py;

		if (!cgs.arenaInfoLoaded)
		{
			return;
		}

		px = (cgs.arenaData.mappos[0] / 1024.f) * 440.f;
		py = (cgs.arenaData.mappos[1] / 1024.f) * 480.f;

		CG_LoadPanel_DrawPin(cgs.arenaData.longname, px, py, 0.22f, 0.25f, bg_neutralpin, 16.f, 16.f);
	}
	else
	{
		if (!cgs.campaignInfoLoaded)
		{
			return;
		}

		for (i = 0; i < cgs.campaignData.mapCount; i++)
		{
			float px, py;

			cg.teamWonRounds[1] = atoi(CG_ConfigString(CS_ROUNDSCORES1));
			cg.teamWonRounds[0] = atoi(CG_ConfigString(CS_ROUNDSCORES2));

			if (cg.teamWonRounds[1] & (1 << i))
			{
				shader = bg_axispin;
			}
			else if (cg.teamWonRounds[0] & (1 << i))
			{
				shader = bg_alliedpin;
			}
			else
			{
				shader = bg_neutralpin;
			}

			px = (cgs.campaignData.arenas[i].mappos[0] / 1024.f) * 440.f;
			py = (cgs.campaignData.arenas[i].mappos[1] / 1024.f) * 480.f;

			CG_LoadPanel_DrawPin(cgs.campaignData.arenas[i].longname, px, py, 0.22f, 0.25f, shader, 16.f, 16.f);
		}
	}
}
