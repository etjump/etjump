// cg_scoreboard -- draw the scoreboard on top of the game screen
#include "cg_local.h"

#define SCOREBOARD_WIDTH    (31 * BIGCHAR_WIDTH)

vec4_t clrUiBack = { 0.f, 0.f, 0.f, .6f };
vec4_t clrUiBackDark = { 0.f, 0.f, 0.f, .8f };
vec4_t clrUiBar = { .16f, .2f, .17f, .8f };

// Begin of alt scoreboard 1

#define ALT_SCOREBOARD_WIDTH 600
#define ALT_SCOREBOARD_HEIGHT 460

#define ALT_SCOREBOARD_HORIZONTAL_DELTA 20
#define ALT_SCOREBOARD_VERTICAL_DELTA 13

#define ALT_SCOREBOARD_PLAYER_WIDTH 105
#define ALT_SCOREBOARD_INFO_WIDTH 40
#define ALT_SCOREBOARD_FPS_WIDTH 36
#define ALT_SCOREBOARD_PING_WIDTH 40

void CG_DrawHeader(float x, float y, float fade)
{
	fontInfo_t *font         = &cgs.media.limboFont1;
	const char *header       = NULL;
	const char *configString = CG_ConfigString(CS_SERVERINFO);
	vec4_t     textColor     = { 0.6f, 0.6f, 0.6f, 0 };
	textColor[3] = fade;

	header = va(CG_TranslateString(va("^7%s", Info_ValueForKey(configString, "sv_hostname"))));

	y += ALT_SCOREBOARD_VERTICAL_DELTA;
	CG_Text_Paint_Ext(ALT_SCOREBOARD_WIDTH / 2 - CG_Text_Width_Ext(header, 0.25f, 0, font) / 2,
	                  y, 0.25f, 0.25f, textColor, header, 0, 0, 0, font);

	y     += ALT_SCOREBOARD_VERTICAL_DELTA;
	header = va(CG_TranslateString(va("^7%s", cgs.rawmapname)));
	CG_Text_Paint_Ext(ALT_SCOREBOARD_WIDTH / 2 - CG_Text_Width_Ext(header, 0.25f, 0, font) / 2, y, 0.25f, 0.25f, textColor, header, 0, 0, 0, font);
}

void CG_AltScoreboardDrawClientScore(float x, float y, score_t *score, vec4_t textColor, float fade)
{
	clientInfo_t *ci = &cgs.clientinfo[score->client];

	float tempX = x;
	textColor[4] *= fade;

	CG_DrawStringExt(tempX, y, ci->name, textColor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, 12);
	tempX += ALT_SCOREBOARD_PLAYER_WIDTH;

	if (ci->team == TEAM_SPECTATOR)
	{
		if (score->ping == -1)
		{
			CG_DrawMiniString(tempX, y, CG_TranslateString("^3CONNECTING"), fade);
		}
		else if (ci->clientNum == score->followedClient)
		{
			CG_DrawMiniString(tempX, y, CG_TranslateString("^3SPECTATOR"), fade);
		}
		else
		{
			CG_DrawMiniString(tempX, y, CG_TranslateString("^3>"), fade);
			tempX += 19;
			CG_DrawStringExt(tempX, y, cgs.clientinfo[score->followedClient].name, textColor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, 12);
		}
	}
	else
	{
		CG_DrawMiniString(tempX, y, va("%3i", ci->maxFPS), fade);
		tempX += ALT_SCOREBOARD_FPS_WIDTH;

		CG_DrawMiniString(tempX, y, va("%4i", score->ping), fade);
		tempX += ALT_SCOREBOARD_PING_WIDTH;

		CG_DrawMiniString(tempX, y, va("%1s%1s %1s", ci->pmoveFixed ? "^2P" : "", ci->CGaz ? "^8C" : "", ci->team == TEAM_ALLIES ? "^4B" : "^1R"), fade);
	}
}

void CG_DrawPlayers(float x, float y, float fade)
{
	float tempX      = x;
	float tempY      = y;
	float specStartX = 0;
	int   i          = 0;

	tempX     += ALT_SCOREBOARD_HORIZONTAL_DELTA;
	tempY     += 3 * ALT_SCOREBOARD_VERTICAL_DELTA;
	specStartX = tempX + 320;

	CG_DrawSmallString(tempX, tempY, CG_TranslateString("PLAYING"), fade);
	tempX = specStartX;
	CG_DrawSmallString(tempX, tempY, CG_TranslateString("SPECTATING"), fade);
	tempX  = x + ALT_SCOREBOARD_HORIZONTAL_DELTA;
	tempY += ALT_SCOREBOARD_VERTICAL_DELTA + 5;
	CG_DrawMiniString(tempX, tempY, CG_TranslateString("Player"), fade);
	CG_DrawMiniString(specStartX, tempY, CG_TranslateString("Player"), fade);
	tempX += ALT_SCOREBOARD_PLAYER_WIDTH;
	CG_DrawMiniString(tempX, tempY, CG_TranslateString("FPS"), fade);
	tempX += ALT_SCOREBOARD_FPS_WIDTH;
	CG_DrawMiniString(tempX, tempY, CG_TranslateString("Ping"), fade);
	tempX += ALT_SCOREBOARD_PING_WIDTH;
	CG_DrawMiniString(tempX, tempY, CG_TranslateString("Info"), fade);
	tempX += ALT_SCOREBOARD_INFO_WIDTH;

	// Spec info
	tempY = y + 5 * ALT_SCOREBOARD_VERTICAL_DELTA + 5;

	tempX = x + ALT_SCOREBOARD_HORIZONTAL_DELTA;

	for (i = 0; i < cg.numScores; i++)
	{
		if (cgs.clientinfo[cg.scores[i].client].team != TEAM_ALLIES)
		{
			continue;
		}

		CG_AltScoreboardDrawClientScore(tempX, tempY, &cg.scores[i], colorWhite, fade);
		tempY += ALT_SCOREBOARD_VERTICAL_DELTA;
	}

	for (i = 0; i < cg.numScores; i++)
	{
		if (cgs.clientinfo[cg.scores[i].client].team != TEAM_AXIS)
		{
			continue;
		}

		CG_AltScoreboardDrawClientScore(tempX, tempY, &cg.scores[i], colorWhite, fade);
		tempY += ALT_SCOREBOARD_VERTICAL_DELTA;
	}

	tempX = specStartX;
	tempY = y + 5 * ALT_SCOREBOARD_VERTICAL_DELTA + 5;
	for (i = 0; i < cg.numScores; i++)
	{
		if (cgs.clientinfo[cg.scores[i].client].team != TEAM_SPECTATOR)
		{
			continue;
		}

		CG_AltScoreboardDrawClientScore(tempX, tempY, &cg.scores[i], colorWhite, fade);
		tempY += ALT_SCOREBOARD_VERTICAL_DELTA;
	}
}

void CG_DrawAltScoreboard(float fade)
{
	float  x = 20;
	float  y = 10;
	vec4_t currentClrUiBack;
	Vector4Copy(clrUiBack, currentClrUiBack);
	currentClrUiBack[3] = currentClrUiBack[3] * fade;
	CG_FillRect(x, y, 640 - 2 * x, 480 - 2 * y, currentClrUiBack);
	CG_DrawRect_FixedBorder(x, y, 640 - 2 * x, 480 - 2 * y, 1, colorBlack);
	y += 10;
	CG_DrawHeader(x, y, fade);
	CG_DrawPlayers(x, y, fade);
}


// End of alt scoreboard 1

/*
=================
Alt scoreboard 2
=================
*/

#define THIRD_SCOREBOARD_HEADER_HEIGHT 50
#define THIRD_SCOREBOARD_SUBHEADER_HEIGHT 30
#define THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT 20 // TODO: rename pls :D
#define THIRD_SCOREBOARD_DIVIDER_WIDTH 4
#define THIRD_SCOREBOARD_PLAYER_WIDTH 180
#define THIRD_SCOREBOARD_FPS_WIDTH 36
#define THIRD_SCOREBOARD_PING_WIDTH 40
#define THIRD_SCOREBOARD_INFO_WIDTH 40


void CG_DrawHeader2(float x, float y, float fade)
{
	fontInfo_t *font               = &cgs.media.limboFont2;
	const char *header             = NULL;
	const char *configString       = CG_ConfigString(CS_SERVERINFO);
	vec4_t     textColor           = { 0.6f, 0.6f, 0.6f, 0 };
	float      tempX               = x + 20;
	float      tempY               = y + (THIRD_SCOREBOARD_HEADER_HEIGHT / 2) + 4;
	float      mapNameWidth        = 0;
	float      scoreboardWidth     = ALT_SCOREBOARD_WIDTH;
	float      teamScoreboardWidth = scoreboardWidth / 2;

	textColor[3] = fade;

	// Draw the server hostname
	header = va(CG_TranslateString(va("^7%s", Info_ValueForKey(configString, "sv_hostname"))));
	CG_Text_Paint_Ext(tempX, tempY, 0.25f, 0.25f, textColor, header, 0, 0, 0, font);
	CG_Text_Paint_Ext(tempX + 1, tempY + 10, 0.15f, 0.15f, textColor, va("^7%s", cg.ipAddr), 0, 0, 0, font);


	// Draw the current map name
	header       = va(CG_TranslateString(va("^7%s", cgs.rawmapname)));
	mapNameWidth = CG_Text_Width_Ext(header, 0.25f, 0, font);
	tempX        = x + ALT_SCOREBOARD_WIDTH - 20 - mapNameWidth;
	CG_Text_Paint_Ext(tempX, tempY, 0.25f, 0.25f, textColor, header, 0, 0, 0, font);

	// Draw the "jumping" text
	header = va(CG_TranslateString("^7Jumping"));
	tempX  = x + (teamScoreboardWidth / 2) - (CG_Text_Width_Ext(header, 0.25, 0, font) / 2);
	tempY  = y + THIRD_SCOREBOARD_HEADER_HEIGHT
	         + THIRD_SCOREBOARD_DIVIDER_WIDTH
	         + (THIRD_SCOREBOARD_SUBHEADER_HEIGHT / 2) + 4;
	CG_Text_Paint_Ext(tempX, tempY, 0.25f, 0.25f, textColor, header, 0, 0, 0, font);

	// Draw the "spectating" text
	header = CG_TranslateString("^7Spectating");
	tempX += teamScoreboardWidth;
	tempY  = y + THIRD_SCOREBOARD_HEADER_HEIGHT
	         + THIRD_SCOREBOARD_DIVIDER_WIDTH
	         + (THIRD_SCOREBOARD_SUBHEADER_HEIGHT / 2) + 4;
	CG_Text_Paint_Ext(tempX, tempY, 0.25f, 0.25f, textColor, header, 0, 0, 0, font);

	// Draw the header for the player list
	header = CG_TranslateString("^7Player");
	tempX  = x + THIRD_SCOREBOARD_DIVIDER_WIDTH;
	tempY  = y + THIRD_SCOREBOARD_HEADER_HEIGHT // Servername
	         + THIRD_SCOREBOARD_DIVIDER_WIDTH // divider
	         + THIRD_SCOREBOARD_SUBHEADER_HEIGHT // Jumping, spectating
	         + THIRD_SCOREBOARD_DIVIDER_WIDTH // divider
	         + 4; // Random value to make this be in the middle of the bar
	CG_DrawMiniString(tempX, tempY, header, fade);

	tempX += THIRD_SCOREBOARD_PLAYER_WIDTH;
	CG_DrawMiniString(tempX, tempY, CG_TranslateString("FPS"), fade);
	tempX += THIRD_SCOREBOARD_FPS_WIDTH;
	CG_DrawMiniString(tempX, tempY, CG_TranslateString("Ping"), fade);
	tempX += THIRD_SCOREBOARD_PING_WIDTH;
	CG_DrawMiniString(tempX, tempY, CG_TranslateString("Info"), fade);
	tempX += THIRD_SCOREBOARD_INFO_WIDTH;

	tempX = x + ALT_SCOREBOARD_WIDTH / 2 + THIRD_SCOREBOARD_DIVIDER_WIDTH + 2;
	tempY = y + THIRD_SCOREBOARD_HEADER_HEIGHT // Servername
	        + THIRD_SCOREBOARD_DIVIDER_WIDTH // divider
	        + THIRD_SCOREBOARD_SUBHEADER_HEIGHT // Jumping, spectating
	        + THIRD_SCOREBOARD_DIVIDER_WIDTH // divider
	        + 4; // Random value to make this be in the middle of the bar
	CG_DrawMiniString(tempX, tempY, CG_TranslateString("Player"), fade);

}

void CG_ThirdScoreboardDrawClientScore(float x, float y, score_t *score, vec4_t textColor, float fade)
{
	clientInfo_t *ci = &cgs.clientinfo[score->client];

	float tempX = x;
	textColor[4] *= fade;

	if (ci->team == TEAM_SPECTATOR)
	{
		tempX += 2;
	}

	CG_DrawStringExt(tempX, y, ci->name, textColor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, 22);
	tempX += THIRD_SCOREBOARD_PLAYER_WIDTH - 2;

	if (ci->team == TEAM_SPECTATOR)
	{
		if (score->ping == -1)
		{
			CG_DrawMiniString(tempX, y, CG_TranslateString("^3CONNECTING"), fade);
		}
		else if (ci->clientNum == score->followedClient)
		{
			CG_DrawMiniString(tempX, y, CG_TranslateString("^3SPECTATOR"), fade);
		}
		else
		{
			CG_DrawMiniString(tempX, y, CG_TranslateString("^3>"), fade);
			tempX += 14;
			CG_DrawStringExt(tempX, y, cgs.clientinfo[score->followedClient].name, textColor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, 12);
		}
	}
	else
	{
		CG_DrawMiniString(tempX, y, va("%3i", ci->maxFPS), fade);
		tempX += ALT_SCOREBOARD_FPS_WIDTH;

		CG_DrawMiniString(tempX, y, va("%4i", score->ping), fade);
		tempX += ALT_SCOREBOARD_PING_WIDTH;

		CG_DrawMiniString(tempX, y, va("%1s%1s %1s", ci->pmoveFixed ? "^2P" : "", ci->CGaz ? "^8C" : "", ci->team == TEAM_ALLIES ? "^4B" : "^1R"), fade);
	}
}

void CG_DrawPlayers2(float x, float y, float fade)
{
	float tempX      = x + THIRD_SCOREBOARD_DIVIDER_WIDTH + 2;
	float tempY      = y + THIRD_SCOREBOARD_HEADER_HEIGHT + THIRD_SCOREBOARD_DIVIDER_WIDTH + THIRD_SCOREBOARD_SUBHEADER_HEIGHT + THIRD_SCOREBOARD_DIVIDER_WIDTH + THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT + THIRD_SCOREBOARD_DIVIDER_WIDTH;
	float specStartX = x + ALT_SCOREBOARD_WIDTH / 2 + THIRD_SCOREBOARD_DIVIDER_WIDTH + 2;
	int   i          = 0;

	for (i = 0; i < cg.numScores; i++)
	{
		if (cgs.clientinfo[cg.scores[i].client].team != TEAM_ALLIES)
		{
			continue;
		}

		CG_ThirdScoreboardDrawClientScore(tempX, tempY, &cg.scores[i], colorWhite, fade);
		tempY += ALT_SCOREBOARD_VERTICAL_DELTA;
	}

	for (i = 0; i < cg.numScores; i++)
	{
		if (cgs.clientinfo[cg.scores[i].client].team != TEAM_AXIS)
		{
			continue;
		}

		CG_ThirdScoreboardDrawClientScore(tempX, tempY, &cg.scores[i], colorWhite, fade);
		tempY += ALT_SCOREBOARD_VERTICAL_DELTA;
	}

	tempX = specStartX;
	tempY = y + THIRD_SCOREBOARD_HEADER_HEIGHT + THIRD_SCOREBOARD_DIVIDER_WIDTH + THIRD_SCOREBOARD_SUBHEADER_HEIGHT + THIRD_SCOREBOARD_DIVIDER_WIDTH + THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT + THIRD_SCOREBOARD_DIVIDER_WIDTH;
	for (i = 0; i < cg.numScores; i++)
	{
		if (cgs.clientinfo[cg.scores[i].client].team != TEAM_SPECTATOR)
		{
			continue;
		}

		CG_ThirdScoreboardDrawClientScore(tempX, tempY, &cg.scores[i], colorWhite, fade);
		tempY += ALT_SCOREBOARD_VERTICAL_DELTA;
	}
}

void CG_DrawAltScoreboard2(float fade)
{
	float  x               = 20;
	float  tempX           = x;
	float  y               = 10;
	float  tempY           = y;
	float  width           = ALT_SCOREBOARD_WIDTH;
	float  height          = THIRD_SCOREBOARD_HEADER_HEIGHT;
	float  distanceFromTop = y;
	vec4_t currentClrUiBack;
	vec4_t currentClrUiBackLight;
	Vector4Copy(clrUiBack, currentClrUiBack);
	Vector4Copy(clrUiBack, currentClrUiBackLight);
	currentClrUiBackLight[3] = 0.3f;
	currentClrUiBack[3]      = currentClrUiBack[3] * fade;
	currentClrUiBackLight[3] = currentClrUiBackLight[3] * fade;


	// Draw server info bar
	CG_FillRect(tempX, tempY, width, height, currentClrUiBack);
	CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, colorBlack);

	distanceFromTop += height;

	tempY += THIRD_SCOREBOARD_DIVIDER_WIDTH + height;
	height = THIRD_SCOREBOARD_SUBHEADER_HEIGHT;
	width  = ALT_SCOREBOARD_WIDTH / 2 - 2;

	// Draw the jumping header bar
	CG_FillRect(tempX, tempY, width, height, currentClrUiBack);
	CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, colorBlack);

	tempX += width + THIRD_SCOREBOARD_DIVIDER_WIDTH;

	// Draw the spectating header bar
	CG_FillRect(tempX, tempY, width, height, currentClrUiBack);
	CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, colorBlack);

	// Go back to start
	tempX            = x;
	distanceFromTop += height;

	tempY += THIRD_SCOREBOARD_DIVIDER_WIDTH + height;
	height = ALT_SCOREBOARD_HEIGHT - distanceFromTop;

	// Draw the jumping bar
	// Draws the left side of the outer box
	CG_FillRect(tempX, tempY, THIRD_SCOREBOARD_DIVIDER_WIDTH, height, currentClrUiBack);
	// Draws the top side of the outer box
	CG_FillRect(tempX + THIRD_SCOREBOARD_DIVIDER_WIDTH, tempY, width - THIRD_SCOREBOARD_DIVIDER_WIDTH * 2, THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT, currentClrUiBack);
	// Draws the right side of the outer box
	CG_FillRect(tempX + width - THIRD_SCOREBOARD_DIVIDER_WIDTH, tempY, THIRD_SCOREBOARD_DIVIDER_WIDTH, height, currentClrUiBack);
	// Draws the bottom side of the outer box
	CG_FillRect(tempX + THIRD_SCOREBOARD_DIVIDER_WIDTH, tempY + height - THIRD_SCOREBOARD_DIVIDER_WIDTH, width - THIRD_SCOREBOARD_DIVIDER_WIDTH * 2, THIRD_SCOREBOARD_DIVIDER_WIDTH, currentClrUiBack);
	// Draws the outline of the outer box
	CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, colorBlack);
	// Draws the inner box
	tempX += THIRD_SCOREBOARD_DIVIDER_WIDTH;
	tempY += THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT;
	CG_FillRect(tempX, tempY, width - 2 * THIRD_SCOREBOARD_DIVIDER_WIDTH, height - THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT - THIRD_SCOREBOARD_DIVIDER_WIDTH, currentClrUiBackLight);
	CG_DrawRect_FixedBorder(tempX, tempY, width - 2 * THIRD_SCOREBOARD_DIVIDER_WIDTH, height - THIRD_SCOREBOARD_DIVIDER_WIDTH - THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT, 1, colorBlack);

	tempX  = x + width + THIRD_SCOREBOARD_DIVIDER_WIDTH;
	tempY -= THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT;

	// Draw the spectating bar
	// Draws the left side of the outer box
	CG_FillRect(tempX, tempY, THIRD_SCOREBOARD_DIVIDER_WIDTH, height, currentClrUiBack);
	// Draws the top side of the outer box
	CG_FillRect(tempX + THIRD_SCOREBOARD_DIVIDER_WIDTH, tempY, width - THIRD_SCOREBOARD_DIVIDER_WIDTH * 2, THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT, currentClrUiBack);
	// Draws the right side of the outer box
	CG_FillRect(tempX + width - THIRD_SCOREBOARD_DIVIDER_WIDTH, tempY, THIRD_SCOREBOARD_DIVIDER_WIDTH, height, currentClrUiBack);
	// Draws the bottom side of the outer box
	CG_FillRect(tempX + THIRD_SCOREBOARD_DIVIDER_WIDTH, tempY + height - THIRD_SCOREBOARD_DIVIDER_WIDTH, width - THIRD_SCOREBOARD_DIVIDER_WIDTH * 2, THIRD_SCOREBOARD_DIVIDER_WIDTH, currentClrUiBack);
	// Draws the outline of the outer box
	CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, colorBlack);
	// Draws the inner box
	tempX += THIRD_SCOREBOARD_DIVIDER_WIDTH;
	tempY += THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT;
	CG_FillRect(tempX, tempY, width - 2 * THIRD_SCOREBOARD_DIVIDER_WIDTH, height - THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT - THIRD_SCOREBOARD_DIVIDER_WIDTH, currentClrUiBackLight);
	CG_DrawRect_FixedBorder(tempX, tempY, width - 2 * THIRD_SCOREBOARD_DIVIDER_WIDTH, height - THIRD_SCOREBOARD_DIVIDER_WIDTH - THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT, 1, colorBlack);



//    CG_FillRect(tempX, tempY, width, height, currentClrUiBack);
//    CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, colorBlack);
//
//    tempX += width + THIRD_SCOREBOARD_DIVIDER_WIDTH;
//
//    // Draw the spectating bar
//    CG_FillRect(tempX, tempY, width, height, currentClrUiBack);
//    CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, colorBlack);
//
//
//    // Draw the inner jumping bar
//    CG_FillRect(tempX + THIRD_SCOREBOARD_DIVIDER_WIDTH, tempY + 20, width - 2 * THIRD_SCOREBOARD_DIVIDER_WIDTH, height, currentClrUiBack);
//
//    // Draw the inner spectating bar

	CG_DrawHeader2(x, y, fade);
	CG_DrawPlayers2(x, y, fade);
}


/*
=================
WM_DrawObjectives
=================
*/

#define INFO_XP_WIDTH           36
#define INFO_LIVES_WIDTH        20
#define INFO_PLAYER_WIDTH       100
#define INFO_CGAZ_WIDTH         35
#define INFO_SCORE_WIDTH        56
#define INFO_FPS_WIDTH          36
#define INFO_CLASS_WIDTH        20
#define INFO_LATENCY_WIDTH      40
#define INFO_PMOVE_WIDTH        54
#define INFO_TEAM_HEIGHT        24
#define INFO_BORDER             0
#define INFO_LINE_HEIGHT        30
#define INFO_TOTAL_WIDTH        (INFO_PLAYER_WIDTH + INFO_CLASS_WIDTH + INFO_FPS_WIDTH + INFO_SCORE_WIDTH + INFO_PMOVE_WIDTH + INFO_LATENCY_WIDTH + INFO_CGAZ_WIDTH)

int WM_DrawObjectives(int x, int y, int width, float fade)
{
	const char *s, *str;
	int        tempy, rows;
	int        msec, mins, seconds, tens; // JPW NERVE
	vec4_t     tclr = { 0.6f, 0.6f, 0.6f, 1.0f };

	if (cg.snap->ps.pm_type == PM_INTERMISSION)
	{
		const char *s, *buf, *shader = NULL, *flagshader = NULL, *nameshader = NULL;

		// Moved to CG_DrawIntermission
/*		static int doScreenshot = 0, doDemostop = 0;

        // OSP - End-of-level autoactions
        if(!cg.demoPlayback) {
            if(!cg.latchVictorySound) {
                if(cg_autoAction.integer & AA_SCREENSHOT) {
                    doScreenshot = cg.time + 1000;
                }
                if(cg_autoAction.integer & AA_STATSDUMP) {
                    CG_dumpStats_f();
                }
                if((cg_autoAction.integer & AA_DEMORECORD) && (cgs.gametype == GT_WOLF_STOPWATCH && cgs.currentRound != 1)) {
                    doDemostop = cg.time + 5000;	// stats should show up within 5 seconds
                }
            }
            if(doScreenshot > 0 && doScreenshot < cg.time) {
                CG_autoScreenShot_f();
                doScreenshot = 0;
            }
            if(doDemostop > 0 && doDemostop < cg.time) {
                trap_SendConsoleCommand("stoprecord\n");
                doDemostop = 0;
            }
        }
*/
		rows = 8;
		y   += SMALLCHAR_HEIGHT * (rows - 1);

		s   = CG_ConfigString(CS_MULTI_MAPWINNER);
		buf = Info_ValueForKey(s, "winner");

		if (atoi(buf) == -1)
		{
			str = "ITS A TIE!";
		}
		else if (atoi(buf))
		{
			str = "ALLIES";
//			shader = "ui/assets/portraits/allies_win";
			flagshader = "ui/assets/portraits/allies_win_flag.tga";
			nameshader = "ui/assets/portraits/text_allies.tga";

/*			if ( !cg.latchVictorySound ) {
                cg.latchVictorySound = qtrue;
                trap_S_StartLocalSound( trap_S_RegisterSound( "sound/music/allies_win.wav", qtrue ), CHAN_LOCAL_SOUND );	// FIXME: stream
            }*/
		}
		else
		{
			str = "AXIS";
//			shader = "ui/assets/portraits/axis_win";
			flagshader = "ui/assets/portraits/axis_win_flag.tga";
			nameshader = "ui/assets/portraits/text_axis.tga";

/*			if ( !cg.latchVictorySound ) {
                cg.latchVictorySound = qtrue;
                trap_S_StartLocalSound( trap_S_RegisterSound( "sound/music/axis_win.wav", qtrue ), CHAN_LOCAL_SOUND );	// FIXME: stream
            }*/
		}

		y += SMALLCHAR_HEIGHT * ((rows - 2) / 2);

		if (flagshader)
		{
			CG_DrawPic(100, 10, 210, 136, trap_R_RegisterShaderNoMip(flagshader));
			CG_DrawPic(325, 10, 210, 136, trap_R_RegisterShaderNoMip(flagshader));
		}

		if (shader)
		{
			CG_DrawPic(229, 10, 182, 136, trap_R_RegisterShaderNoMip(shader));
		}
		if (nameshader)
		{
			CG_DrawPic(140, 50, 127, 64, trap_R_RegisterShaderNoMip(nameshader));
			CG_DrawPic(365, 50, 127, 64, trap_R_RegisterShaderNoMip("ui/assets/portraits/text_win.tga"));
		}
		return y;
	}
// JPW NERVE -- mission time & reinforce time
	else
	{
		tempy = y;
		rows  = 1;

		CG_FillRect(x - 5, y - 2, width + 5, 21, clrUiBack);
		CG_FillRect(x - 5, y - 2, width + 5, 21, clrUiBar);
		CG_DrawRect_FixedBorder(x - 5, y - 2, width + 5, 21, 1, colorBlack);

		y += SMALLCHAR_HEIGHT * (rows - 1);
		if (cgs.timelimit > 0.0f)
		{
			msec = (cgs.timelimit * 60.f * 1000.f) - (cg.time - cgs.levelStartTime);

			seconds  = msec / 1000;
			mins     = seconds / 60;
			seconds -= mins * 60;
			tens     = seconds / 10;
			seconds -= tens * 10;
		}
		else
		{
			msec = mins = tens = seconds = 0;
		}

		if (cgs.gamestate != GS_PLAYING)
		{
			s = va("%s %s", CG_TranslateString("MISSION TIME:"), CG_TranslateString("WARMUP"));
		}
		else if (msec < 0 && cgs.timelimit > 0.0f)
		{
			if (cgs.gamestate == GS_WAITING_FOR_PLAYERS)
			{
				s = va("%s %s", CG_TranslateString("MISSION TIME:"), CG_TranslateString("GAME STOPPED"));
			}
			else
			{
				s = va("%s %s", CG_TranslateString("MISSION TIME:"), CG_TranslateString("SUDDEN DEATH"));
			}
		}
		else
		{
			s = va("%s   %2.0f:%i%i", CG_TranslateString("MISSION TIME:"), (float)mins, tens, seconds);     // float cast to line up with reinforce time
		}

		//	CG_Text_Paint_Ext( x, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.limboFont1 );

		s = va(CG_TranslateString(va("^<%s", cgs.rawmapname)));
		CG_Text_Paint_Ext(640 / 2 - CG_Text_Width_Ext(s, 0.25f, 0, &cgs.media.limboFont2) / 2, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.limboFont2);
		s = CG_TranslateString("^<www.etjump.com");
		CG_Text_Paint_Ext(640 - 80 - CG_Text_Width_Ext(s, 0.25f, 0, &cgs.media.limboFont2) / 2, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.limboFont2);

		if (cgs.gametype != GT_WOLF_LMS)
		{
			if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS || cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_ALLIES)
			{
				msec = CG_CalculateReinfTime(qfalse) * 1000;
			}
			else // no team (spectator mode)
			{
				msec = 0;
			}

			if (msec)
			{
				seconds  = msec / 1000;
				mins     = seconds / 60;
				seconds -= mins * 60;
				tens     = seconds / 10;
				seconds -= tens * 10;

				s = va("%s %2.0f:%i%i", CG_TranslateString("REINFORCE TIME:"), (float)mins, tens, seconds);
				//	CG_Text_Paint_Ext( 640 - 20 - CG_Text_Width_Ext( s, 0.25f, 0, &cgs.media.limboFont1 ), y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.limboFont1 );
			}
		}

		// NERVE - SMF
		if (cgs.gametype == GT_WOLF_STOPWATCH)
		{
			int w;
			s = va("%s %i", CG_TranslateString("STOPWATCH ROUND"), cgs.currentRound + 1);

			w = CG_Text_Width_Ext(s, 0.25f, 0, &cgs.media.limboFont1);

			//CG_Text_Paint_Ext( x + 300 - w*0.5f, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.limboFont1 );
		}
		else if (cgs.gametype == GT_WOLF_LMS)
		{
			int w;
			s = va("%s %i  %s %i-%i", CG_TranslateString("ROUND"), cgs.currentRound + 1, CG_TranslateString("SCORE"), cg.teamWonRounds[1], cg.teamWonRounds[0]);
			w = CG_Text_Width_Ext(s, 0.25f, 0, &cgs.media.limboFont1);

			//CG_Text_Paint_Ext( x + 300 - w*0.5f, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.limboFont1 );
		}
		else if (cgs.gametype == GT_WOLF_CAMPAIGN)
		{
			int w;
			s = va("MAP %i of %i", cgs.currentCampaignMap + 1, cgs.campaignData.mapCount);
			w = CG_Text_Width_Ext(s, 0.25f, 0, &cgs.media.limboFont1);

			//CG_Text_Paint_Ext( x + 300 - w*0.5f, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.limboFont1 );
		}

		y += SMALLCHAR_HEIGHT * 2;
	}
// jpw

	return y;
}

static void WM_DrawClientScore(int x, int y, score_t *score, float *color, float fade)
{
	int          offset;
	float        tempx;
	vec4_t       hcolor;
	clientInfo_t *ci;
	int          w;
	char         *cgazOn;

	if (y + SMALLCHAR_HEIGHT >= 470)
	{
		return;
	}

	ci = &cgs.clientinfo[score->client];

	if (score->client == cg.snap->ps.clientNum)
	{
		tempx = x;

		hcolor[3] = fade * 0.3;
		VectorSet(hcolor, .5f, .5f, .2f);           // DARK-RED

		// FIXME Removed INFO_CGAZ_WIDTH x1
		CG_FillRect(tempx - 4, y + 1, INFO_PLAYER_WIDTH - INFO_BORDER + 4, SMALLCHAR_HEIGHT - 1, hcolor);
		tempx += INFO_PLAYER_WIDTH;

		if (ci->team == TEAM_SPECTATOR)
		{
			int width;
			width = INFO_CLASS_WIDTH + INFO_SCORE_WIDTH  + INFO_CGAZ_WIDTH + INFO_LATENCY_WIDTH * 2 - 6;

			CG_FillRect(tempx, y + 1, width - INFO_BORDER, SMALLCHAR_HEIGHT - 1, hcolor);
			tempx += width;
		}
		else
		{
			int width;
			width = INFO_CLASS_WIDTH + INFO_SCORE_WIDTH  + INFO_CGAZ_WIDTH + INFO_LATENCY_WIDTH * 2 - 6;

			CG_FillRect(tempx, y + 1, width - INFO_BORDER, SMALLCHAR_HEIGHT - 1, hcolor);
		}
	}

	tempx = x;

	// DHM - Nerve
	VectorSet(hcolor, 1, 1, 1);
	hcolor[3] = fade;

	offset = 0;

	// draw name, 12 chars if drawing cgaz users, else 16
	if (cg_drawCGazUsers.integer)
	{
		CG_DrawStringExt(tempx, y, ci->name, hcolor, qfalse, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 12);
	}
	else
	{
		CG_DrawStringExt(tempx, y, ci->name, hcolor, qfalse, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 16);
	}

	tempx += INFO_PLAYER_WIDTH - offset;

	if (ci->team == TEAM_SPECTATOR)
	{
		const char *s;
		int        totalwidth;

		totalwidth = INFO_CLASS_WIDTH + INFO_SCORE_WIDTH + INFO_CGAZ_WIDTH + INFO_LATENCY_WIDTH - 8 + INFO_CGAZ_WIDTH;

		if (!cg_drawCGazUsers.integer)
		{
			tempx += INFO_CGAZ_WIDTH;
		}

		tempx += 19;

		if (score->ping == -1)
		{
			s = CG_TranslateString("^3CONNECTING");
			w = CG_DrawStrlen(s) * SMALLCHAR_WIDTH;
			CG_DrawSmallString(tempx + 10, y, s, fade);
		}
		else if (ci->clientNum == score->followedClient)
		{
			s = CG_TranslateString("^3SPECTATOR");
			w = CG_DrawStrlen(s) * SMALLCHAR_WIDTH;
			CG_DrawSmallString(tempx + 12, y, s, fade);
		}
		else
		{
			s = cgs.clientinfo[score->followedClient].name;

			// Draws max 12 chars of the name
			CG_DrawStringExt(tempx + 3, y, s, hcolor, qfalse, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 12);

			CG_DrawSmallString(tempx - 16, y, CG_TranslateString("^3>"), fade);
		}

		if (!cg_drawCGazUsers.integer)
		{
			tempx -= 27;
		}
		CG_DrawSmallString(tempx + 10 + INFO_PMOVE_WIDTH + INFO_FPS_WIDTH, y, va("^z%5i", score->ping), fade);
		return;
	}
	else
	{
		int playerType;
		playerType = ci->cls;

		if (!cg_drawCGazUsers.integer)
		{
			tempx += INFO_CGAZ_WIDTH;
		}

		if (playerType == PC_MEDIC)
		{
			CG_DrawPic(tempx + 1, y + 2, 12, 12, cgs.media.skillPics[SK_FIRST_AID]);
		}
		else if (playerType == PC_ENGINEER)
		{
			CG_DrawPic(tempx + 1, y + 2, 12, 12, cgs.media.skillPics[SK_EXPLOSIVES_AND_CONSTRUCTION]);
		}
		else if (playerType == PC_FIELDOPS)
		{
			CG_DrawPic(tempx + 1, y + 2, 12, 12, cgs.media.skillPics[SK_SIGNALS]);
		}
		else if (playerType == PC_COVERTOPS)
		{
			CG_DrawPic(tempx + 1, y + 2, 12, 12, cgs.media.skillPics[SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS]);
		}
		else
		{
			CG_DrawPic(tempx + 1, y + 2, 12, 12, cgs.media.skillPics[SK_HEAVY_WEAPONS]);
		}
	}

	tempx += INFO_CLASS_WIDTH;


	// Dini, shows current FPS..
	CG_DrawSmallString(tempx, y, va("%3i", ci->maxFPS), fade);
	tempx += (INFO_FPS_WIDTH / 2) - 5;

	// Dini, shows current Pmove..
	CG_DrawSmallString(tempx + 15, y, va("%4i", ci->pmoveFixed), fade);
	tempx += (INFO_PMOVE_WIDTH / 2) - 5;

	if (cg_drawCGazUsers.integer)
	{

		if (ci->CGaz)
		{
			cgazOn = "Yes";
		}
		else
		{
			cgazOn = " No";
		}

		CG_DrawSmallString(tempx + 50, y, va("%s", cgazOn), fade);
		tempx += (INFO_CGAZ_WIDTH / 2) - 5;

	}
	else
	{
		tempx -= 13;
	}

	// Ping
	CG_DrawSmallString(tempx + 50, y, va("%5i", score->ping), fade);

}

const char *WM_TimeToString(float msec)
{
	int mins, seconds, tens;

	seconds  = msec / 1000;
	mins     = seconds / 60;
	seconds -= mins * 60;
	tens     = seconds / 10;
	seconds -= tens * 10;

	return va("%i:%i%i", mins, tens, seconds);
}

static int WM_DrawInfoLine(int x, int y, float fade)
{
	int        w, defender, winner;
	const char *s;
	vec4_t     tclr = { 0.6f, 0.6f, 0.6f, 1.0f };

	if (cg.snap->ps.pm_type != PM_INTERMISSION)
	{
		return y;
	}

	w = 360;
//	CG_DrawPic( 320 - w/2, y, w, INFO_LINE_HEIGHT, trap_R_RegisterShaderNoMip( "ui/assets/mp_line_strip.tga" ) );

	s        = CG_ConfigString(CS_MULTI_INFO);
	defender = atoi(Info_ValueForKey(s, "defender"));

	s      = CG_ConfigString(CS_MULTI_MAPWINNER);
	winner = atoi(Info_ValueForKey(s, "winner"));

	if (cgs.currentRound)
	{
		// first round
		s = va(CG_TranslateString("CLOCK IS NOW SET TO %s!"), WM_TimeToString(cgs.nextTimeLimit * 60.f * 1000.f));
	}
	else
	{
		// second round
		if (!defender)
		{
			if (winner != defender)
			{
				s = "ALLIES SUCCESSFULLY BEAT THE CLOCK!";
			}
			else
			{
				s = "ALLIES COULDN'T BEAT THE CLOCK!";
			}
		}
		else
		{
			if (winner != defender)
			{
				s = "AXIS SUCCESSFULLY BEAT THE CLOCK!";
			}
			else
			{
				s = "AXIS COULDN'T BEAT THE CLOCK!";
			}
		}

		s = CG_TranslateString(s);
	}

	CG_FillRect(320 - w / 2, y, w, 20, clrUiBar);
	CG_DrawRect_FixedBorder(320 - w / 2, y, w, 20, 1, colorBlack);

	w = CG_Text_Width_Ext(s, 0.25f, 0, &cgs.media.limboFont1);

	CG_Text_Paint_Ext(320 - w * 0.5f, y + 15, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.limboFont1);
//	CG_DrawSmallString( 320 - w/2, ( y + INFO_LINE_HEIGHT / 2 ) - SMALLCHAR_HEIGHT / 2, s, fade );
	return y + INFO_LINE_HEIGHT + 6;
}

static int WM_TeamScoreboard(int x, int y, team_t team, float fade, int maxrows)
{
	vec4_t hcolor;
	float  tempx, tempy;
	int    height, width;
	int    i;
	int    count = 0;
	vec4_t tclr  = { 0.6f, 0.6f, 0.6f, 1.0f };

	height = SMALLCHAR_HEIGHT * maxrows;

	width = INFO_PLAYER_WIDTH + INFO_CLASS_WIDTH + INFO_PMOVE_WIDTH + INFO_FPS_WIDTH + INFO_LATENCY_WIDTH + INFO_CGAZ_WIDTH;

	CG_FillRect(x - 5, y - 2, width + 5, 21, clrUiBack);
	CG_FillRect(x - 5, y - 2, width + 5, 21, clrUiBar);

	Vector4Set(hcolor, 0, 0, 0, fade);
	CG_DrawRect_FixedBorder(x - 5, y - 2, width + 5, 21, 1, colorBlack);

	// draw header
	if (cg_gameType.integer == GT_WOLF_LMS)
	{
		char *s;
		if (team == TEAM_AXIS)
		{
			s = va("%s [%d] (%d %s)", CG_TranslateString("AXIS"), cg.teamScores[0], cg.teamPlayers[team], CG_TranslateString("PLAYERS"));
			s = va("%s ^3%s", s, cg.teamFirstBlood == TEAM_AXIS ? CG_TranslateString("FIRST BLOOD") : "");

			CG_Text_Paint_Ext(x, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.limboFont1);
		}
		else if (team == TEAM_ALLIES)
		{
			s = va("%s [%d] (%d %s)", CG_TranslateString("ALLIES"), cg.teamScores[1], cg.teamPlayers[team], CG_TranslateString("PLAYERS"));
			s = va("%s ^3%s", s, cg.teamFirstBlood == TEAM_ALLIES ? CG_TranslateString("FIRST BLOOD") : "");

			CG_Text_Paint_Ext(x, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.limboFont1);
		}
	}
	else
	{
		if (team == TEAM_AXIS)
		{
			CG_Text_Paint_Ext(x, y + 13, 0.25f, 0.25f, tclr, va("%s [%d] (%d %s)", CG_TranslateString("AXIS"), cg.teamScores[0], cg.teamPlayers[team], CG_TranslateString("PLAYERS")), 0, 0, 0, &cgs.media.limboFont1);
		}
		else if (team == TEAM_ALLIES)
		{
			CG_Text_Paint_Ext(x, y + 13, 0.25f, 0.25f, tclr, va("%s [%d] (%d %s)", CG_TranslateString("ALLIES"), cg.teamScores[1], cg.teamPlayers[team], CG_TranslateString("PLAYERS")), 0, 0, 0, &cgs.media.limboFont1);
		}
	}


	y += SMALLCHAR_HEIGHT + 3;

	// save off y val
	tempy = y;

	// draw color bands
	for (i = 0; i <= maxrows; i++)
	{
		if (i % 2 == 0)
		{
			VectorSet(hcolor, (80.f / 255.f), (80.f / 255.f), (80.f / 255.f));          // LIGHT BLUE
		}
		else
		{
			VectorSet(hcolor, (0.f / 255.f), (0.f / 255.f), (0.f / 255.f));         // DARK BLUE
		}
		hcolor[3] = fade * 0.3;

		CG_FillRect(x - 5, y, width + 5, SMALLCHAR_HEIGHT + 1, hcolor);
		trap_R_SetColor(colorBlack);
		CG_DrawTopBottom(x - 5, y, width + 5, SMALLCHAR_HEIGHT + 1, 1);
		trap_R_SetColor(NULL);

		y += SMALLCHAR_HEIGHT;
	}
	hcolor[3] = 1;

	y = tempy;

	tempx = x;

	CG_FillRect(x - 5, y - 1, width + 5, 18, clrUiBack);
	//CG_FillRect( x-5, y-1, width+5, 18, clrUiBar );
	trap_R_SetColor(colorBlack);
	CG_DrawTopBottom(x - 5, y - 1, width + 5, 18, 1);
	trap_R_SetColor(NULL);

	// draw player info headings
	CG_DrawSmallString(tempx, y, CG_TranslateString("Name"), fade);
	if (cg_drawCGazUsers.integer)
	{
		tempx += INFO_PLAYER_WIDTH;
	}
	else
	{
		tempx += (INFO_PLAYER_WIDTH + 35);
	}

	CG_DrawSmallString(tempx + 2, y, CG_TranslateString("C"), fade);
	tempx += INFO_CLASS_WIDTH;

	CG_DrawSmallString(tempx, y, CG_TranslateString("FPS"), fade);
	tempx += INFO_FPS_WIDTH;

	CG_DrawSmallString(tempx, y, CG_TranslateString("Pmove"), fade);
	if (cg_drawCGazUsers.integer)
	{
		tempx += INFO_PMOVE_WIDTH - 10;
	}
	else
	{
		tempx += INFO_PMOVE_WIDTH;
	}

	if (cg_drawCGazUsers.integer)
	{
		CG_DrawSmallString(tempx, y, CG_TranslateString("CGaz"), fade);
		tempx += INFO_CGAZ_WIDTH;
	}

	CG_DrawSmallString(tempx, y, CG_TranslateString("Ping"), fade);
	tempx += INFO_LATENCY_WIDTH;


	y += SMALLCHAR_HEIGHT;

	// draw player info
	VectorSet(hcolor, 1, 1, 1);
	hcolor[3] = fade;

	cg.teamPlayers[team] = 0; // JPW NERVE
	for (i = 0; i < cg.numScores; i++)
	{
		if (team != cgs.clientinfo[cg.scores[i].client].team)
		{
			continue;
		}

		cg.teamPlayers[team]++;
	}

	count = 0;
	for (i = 0; i < cg.numScores && count < maxrows; i++)
	{
		if (team != cgs.clientinfo[cg.scores[i].client].team)
		{
			continue;
		}

		WM_DrawClientScore(x, y, &cg.scores[i], hcolor, fade);
		y += SMALLCHAR_HEIGHT;
		count++;
	}

	// draw spectators
	y += SMALLCHAR_HEIGHT;

	for (i = 0; i < cg.numScores; i++)
	{
		if (cgs.clientinfo[cg.scores[i].client].team != TEAM_SPECTATOR)
		{
			continue;
		}
		if (team == TEAM_AXIS && (i % 2))
		{
			continue;
		}
		if (team == TEAM_ALLIES && ((i + 1) % 2))
		{
			continue;
		}

		WM_DrawClientScore(x, y, &cg.scores[i], hcolor, fade);
		y += SMALLCHAR_HEIGHT;
	}

	return y;
}
// -NERVE - SMF

/*
=================
CG_DrawScoreboard

Draw the normal in-game scoreboard
=================
*/
qboolean CG_DrawScoreboard(void)
{
	int   x = 0, y = 0, x_right;
	float fade;
	float *fadeColor;

	x = 20;
	y = 10;

	x_right = 640 - x - (INFO_TOTAL_WIDTH - 5);

	// don't draw anything if the menu or console is up
	if (cg_paused.integer)
	{
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	// OSP - also for pesky scoreboards in demos
	if ((cg.demoPlayback && cg.snap->ps.pm_type != PM_INTERMISSION) && !cg.showScores)
	{
		return qfalse;
	}

	// don't draw if in cameramode
	if (cg.cameraMode)
	{
		return qtrue;
	}

	if (cg.showScores || cg.predictedPlayerState.pm_type == PM_INTERMISSION)
	{
		fade      = 1.0;
		fadeColor = colorWhite;
	}
	else
	{
		fadeColor = CG_FadeColor(cg.scoreFadeTime, FADE_TIME);

		if (!fadeColor)
		{
			// next time scoreboard comes up, don't print killer
			*cg.killerName = 0;
			return qfalse;
		}
		fade = fadeColor[3];
	}

	if (cg_altScoreboard.integer == 1)
	{
		CG_DrawAltScoreboard(fade);
		return qtrue;
	}
	else if (cg_altScoreboard.integer == 2)
	{
		CG_DrawAltScoreboard2(fade);
		return qtrue;
	}

	y = WM_DrawObjectives(x, y, 640 - 2 * x + 5, fade);

	// Dini, Note, positions \ draws the team container thingies etc
	if (cgs.gametype == GT_WOLF_STOPWATCH && (cg.snap->ps.pm_type == PM_INTERMISSION))
	{
		y = WM_DrawInfoLine(x, 155, fade);

		WM_TeamScoreboard(x, y, TEAM_AXIS, fade, 8);
		x = x_right;
		WM_TeamScoreboard(x, y, TEAM_ALLIES, fade, 8);
	}
	else
	{
		if (cg.snap->ps.pm_type == PM_INTERMISSION)
		{
			WM_TeamScoreboard(x, y, TEAM_AXIS, fade, 9);
			x = x_right;
			WM_TeamScoreboard(x, y, TEAM_ALLIES, fade, 9);
		}
		else
		{
			WM_TeamScoreboard(x, y, TEAM_AXIS, fade, 25);
			x = x_right;
			WM_TeamScoreboard(x + 56, y, TEAM_ALLIES, fade, 25);
		}
	}

	/*	if(!CG_IsSinglePlayer()) {
	        qtime_t ct;

	        G_showWindowMessages();
	        trap_RealTime(&ct);
	        s = va("^3%02d:%02d:%02d - %02d %s %d",
	                            ct.tm_hour, ct.tm_min, ct.tm_sec,
	                            ct.tm_mday, aMonths[ct.tm_mon], 1900 + ct.tm_year);
	        CG_DrawStringExt(444, 12, s, colorWhite, qfalse, qtrue, 8, 8, 0);
	    }
	*/
	return qtrue;
}
