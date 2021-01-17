// cg_scoreboard -- draw the scoreboard on top of the game screen
#include "cg_local.h"
#include "../game/etj_string_utilities.h"

#define SCOREBOARD_WIDTH    (31 * BIGCHAR_WIDTH)

vec4_t clrUiBack = { 0.f, 0.f, 0.f, .6f };
vec4_t clrUiBackDark = { 0.f, 0.f, 0.f, .8f };
vec4_t clrUiBar = { .16f, .2f, .17f, .8f };
vec4_t clrUiBorder = { 0.f, 0.f, 0.f, 1.f };

vec4_t compactUiBack = { 0.f, 0.f, 0.f, .5f };
vec4_t compactUiBackLight = { 0.f, 0.f, 0.f, .25f };
vec4_t compactUiBorder = { .75f, .75f, .75f, .75f };

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
	CG_Text_Paint_Ext(SCREEN_CENTER_X - CG_Text_Width_Ext(header, 0.25f, 0, font) / 2,
	                  y, 0.25f, 0.25f, textColor, header, 0, 0, 0, font);

	y     += ALT_SCOREBOARD_VERTICAL_DELTA;
	header = va(CG_TranslateString(va("^7%s", cgs.rawmapname)));
	CG_Text_Paint_Ext(SCREEN_CENTER_X - CG_Text_Width_Ext(header, 0.25f, 0, font) / 2, y, 0.25f, 0.25f, textColor, header, 0, 0, 0, font);
}

void CG_AltScoreboardDrawClientScore(float x, float y, score_t *score, vec4_t color, float fade)
{
	clientInfo_t *ci = &cgs.clientinfo[score->client];
	vec4_t textColor;
	float tempX = x;
	float offsetX = 0.0f;
	int nameMaxWidth = 12;
	Vector4Copy(color, textColor);
	textColor[3] *= fade;

	// draw indicators
	if (ci->timerunActive)
	{
		CG_DrawPic(tempX, y + 2, 8, 8, cgs.media.stopwatchIcon);
		offsetX += 8 + 2;
		nameMaxWidth -= 1;
	}
	if (etj_drawScoreboardInactivity.integer && ci->clientIsInactive)
	{
		CG_DrawPic(tempX + offsetX, y + 2, 8, 8, cgs.media.idleIcon);
		offsetX += 8 + 2;
		nameMaxWidth -= 1;
	}

	CG_DrawStringExt(tempX + offsetX, y, ci->name, textColor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, nameMaxWidth);
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
		team_t team = cgs.clientinfo[cg.scores[i].client].team;
		if ( team != TEAM_ALLIES && team != TEAM_AXIS)
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
	float  x = 20 + SCREEN_OFFSET_X;
	float  y = 10;
	vec4_t currentClrUiBack;
	vec4_t currentClrUiBorder;
	Vector4Copy(clrUiBack, currentClrUiBack);
	Vector4Copy(clrUiBorder, currentClrUiBorder);
	currentClrUiBack[3]   *= fade;
	currentClrUiBorder[3] *= fade;
	CG_FillRect(x, y, 640 - 2 * 20, 480 - 2 * y, currentClrUiBack);
	CG_DrawRect_FixedBorder(x, y, 640 - 2 * 20, 480 - 2 * y, 1, currentClrUiBorder);
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
	std::string ipAddress = ETJump::stringFormat("^7%s", cg.ipAddr[0] ? cg.ipAddr : "localhost");
	header = va(CG_TranslateString(va("^7%s", Info_ValueForKey(configString, "sv_hostname"))));
	CG_Text_Paint_Ext(tempX, tempY, 0.25f, 0.25f, textColor, header, 0, 0, 0, font);
	CG_Text_Paint_Ext(tempX + 1, tempY + 10, 0.15f, 0.15f, textColor, ipAddress, 0, 0, 0, font);


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

void CG_ThirdScoreboardDrawClientScore(float x, float y, score_t *score, vec4_t color, float fade)
{
	clientInfo_t *ci = &cgs.clientinfo[score->client];
	vec4_t textColor;
	float tempX = x;
	float offsetX = 0.0f;
	int nameMaxWidth = 22;
	Vector4Copy(color, textColor);
	textColor[3] *= fade;

	if (ci->team == TEAM_SPECTATOR)
	{
		tempX += 2;
	}

	// draw indicators
	if (ci->timerunActive)
	{
		CG_DrawPic(tempX, y + 2, 8, 8, cgs.media.stopwatchIcon);
		offsetX += 8 + 2;
		nameMaxWidth -= 2;
	}
	if (etj_drawScoreboardInactivity.integer && ci->clientIsInactive)
	{
		CG_DrawPic(tempX + offsetX, y + 2, 8, 8, cgs.media.idleIcon);
		offsetX += 8 + 2;
		nameMaxWidth -= 2;
	}

	CG_DrawStringExt(tempX + offsetX, y, ci->name, textColor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, nameMaxWidth);
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
		team_t team = cgs.clientinfo[cg.scores[i].client].team;
		if (team != TEAM_ALLIES && team != TEAM_AXIS)
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
	float  x               = 20 + SCREEN_OFFSET_X;
	float  tempX           = x;
	float  y               = 10;
	float  tempY           = y;
	float  width           = ALT_SCOREBOARD_WIDTH;
	float  height          = THIRD_SCOREBOARD_HEADER_HEIGHT;
	float  distanceFromTop = y;
	vec4_t currentClrUiBack;
	vec4_t currentClrUiBackLight;
	vec4_t currentClrUiBorder;
	Vector4Copy(clrUiBack, currentClrUiBack);
	Vector4Copy(clrUiBack, currentClrUiBackLight);
	Vector4Copy(clrUiBorder, currentClrUiBorder);
	currentClrUiBackLight[3] = 0.3f;
	currentClrUiBack[3]      *= fade;
	currentClrUiBackLight[3] *= fade;
	currentClrUiBorder[3]    *= fade;

	// Draw server info bar
	CG_FillRect(tempX, tempY, width, height, currentClrUiBack);
	CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, currentClrUiBorder);

	distanceFromTop += height;

	tempY += THIRD_SCOREBOARD_DIVIDER_WIDTH + height;
	height = THIRD_SCOREBOARD_SUBHEADER_HEIGHT;
	width = ALT_SCOREBOARD_WIDTH / 2 - 2;

	// Draw the jumping header bar
	CG_FillRect(tempX, tempY, width, height, currentClrUiBack);
	CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, currentClrUiBorder);

	tempX += width + THIRD_SCOREBOARD_DIVIDER_WIDTH;

	// Draw the spectating header bar
	CG_FillRect(tempX, tempY, width, height, currentClrUiBack);
	CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, currentClrUiBorder);

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
	CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, currentClrUiBorder);
	// Draws the inner box
	tempX += THIRD_SCOREBOARD_DIVIDER_WIDTH;
	tempY += THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT;
	CG_FillRect(tempX, tempY, width - 2 * THIRD_SCOREBOARD_DIVIDER_WIDTH, height - THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT - THIRD_SCOREBOARD_DIVIDER_WIDTH, currentClrUiBackLight);
	CG_DrawRect_FixedBorder(tempX, tempY, width - 2 * THIRD_SCOREBOARD_DIVIDER_WIDTH, height - THIRD_SCOREBOARD_DIVIDER_WIDTH - THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT, 1, currentClrUiBorder);

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
	CG_DrawRect_FixedBorder(tempX, tempY, width, height, 1, currentClrUiBorder);
	// Draws the inner box
	tempX += THIRD_SCOREBOARD_DIVIDER_WIDTH;
	tempY += THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT;
	CG_FillRect(tempX, tempY, width - 2 * THIRD_SCOREBOARD_DIVIDER_WIDTH, height - THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT - THIRD_SCOREBOARD_DIVIDER_WIDTH, currentClrUiBackLight);
	CG_DrawRect_FixedBorder(tempX, tempY, width - 2 * THIRD_SCOREBOARD_DIVIDER_WIDTH, height - THIRD_SCOREBOARD_DIVIDER_WIDTH - THIRD_SCOREBOARD_SUBSUBHEADER_HEIGHT, 1, currentClrUiBorder);



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
Alt scoreboard 3
=================
*/

#define ALT_SCOREBOARD_3_HEADER_WIDTH			200
#define ALT_SCOREBOARD_3_HEADER_HEIGHT			28

#define ALT_SCOREBOARD_3_ROW_WIDTH				300
#define ALT_SCOREBOARD_3_ROW_HEIGHT				12

#define ALT_SCOREBOARD_3_DIVIDER				14

#define ALT_SCOREBOARD_3_PLAYER_WIDTH			192
#define ALT_SCOREBOARD_3_FPS_WIDTH				23
#define ALT_SCOREBOARD_3_INFO_WIDTH				52
#define ALT_SCOREBOARD_3_PING_WIDTH				23

#define ALT_SCOREBOARD_3_PLAYER_X				SCREEN_CENTER_X - ALT_SCOREBOARD_3_ROW_WIDTH / 2 + 5;
#define ALT_SCOREBOARD_3_FPS_X					ALT_SCOREBOARD_3_PLAYER_X + ALT_SCOREBOARD_3_PLAYER_WIDTH
#define ALT_SCOREBOARD_3_INFO_X					ALT_SCOREBOARD_3_FPS_X + ALT_SCOREBOARD_3_FPS_WIDTH
#define ALT_SCOREBOARD_3_PING_X					ALT_SCOREBOARD_3_INFO_X + ALT_SCOREBOARD_3_INFO_WIDTH

void CG_DrawHeader3(float x, float y, float fade, vec4_t textColor, fontInfo_t *font)
{
	const char *configString = CG_ConfigString(CS_SERVERINFO);
	float headerTextX = x + 5;
	float headerTextY = y + ALT_SCOREBOARD_3_HEADER_HEIGHT / 2;

	// Draw server name & IP address (left side)
	std::string hostName = ETJump::stringFormat("^7%s", Info_ValueForKey(configString, "sv_hostname"));
	std::string ipAddress = ETJump::stringFormat("^7%s", cg.ipAddr[0] ? cg.ipAddr : "localhost");
	float leftTextX = headerTextX;
	float leftTextY = headerTextY;

	CG_Text_Paint_Ext(leftTextX, leftTextY - 4, 0.15f, 0.15f, textColor, hostName, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
	CG_Text_Paint_Ext(leftTextX, leftTextY + 10, 0.12f, 0.12f, textColor, ipAddress, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);

	// Map name and mod version (right side)

	std::string mapName = ETJump::stringFormat("^7%s", cgs.rawmapname);
	std::string modVersion = ETJump::stringFormat("^7%s", GAME_TAG);
	float rightTextX = SCREEN_WIDTH - leftTextX;
	float rightTextY = headerTextY;
	float mapNameOffsetX = CG_Text_Width_Ext(mapName, 0.15f, 0, font);
	float modVersionOffsetX = CG_Text_Width_Ext(modVersion, 0.12f, 0, font);

	CG_Text_Paint_Ext(rightTextX - mapNameOffsetX, rightTextY - 4, 0.15f, 0.15f, textColor, mapName, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
	CG_Text_Paint_Ext(rightTextX - modVersionOffsetX, rightTextY + 10, 0.12f, 0.12f, textColor, modVersion, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
}

void CG_DrawScoreboardRows3(float x, float y, int j, score_t *score, float fade)
{
	vec4_t currentCompactUiBack;
	vec4_t currentCompactUiBackLight;
	vec4_t currentCompactUiBorder;
	Vector4Copy(compactUiBack, currentCompactUiBack);
	Vector4Copy(compactUiBack, currentCompactUiBackLight);
	Vector4Copy(compactUiBorder, currentCompactUiBorder);
	currentCompactUiBackLight[3] = 0.25f;
	currentCompactUiBack[3] *= fade;
	currentCompactUiBackLight[3] *= fade;
	currentCompactUiBorder[3] *= fade;

	// Highlight yourself or followed client
	if (score->client == cg.snap->ps.clientNum)
	{
		Vector4Set(currentCompactUiBack, 0.5f, 0.5f, 0.2f, fade * 0.33);
		Vector4Set(currentCompactUiBackLight, 0.5f, 0.5f, 0.2f, fade * 0.33);
	}

	/* Draw light or dark fill depending on player number.
	This drawing needs to be done here instead of CG_DrawAltScoreboard3,
	because there's a small delay in client fetching cg.numScores
	resulting in inaccurate measurement of player/spectator count,
	so the amount of rows to draw is inaccurate. */

	if (j % 2 == 0)
	{
		CG_FillRect(x, y + ALT_SCOREBOARD_3_ROW_HEIGHT, ALT_SCOREBOARD_3_ROW_WIDTH, ALT_SCOREBOARD_3_ROW_HEIGHT, currentCompactUiBackLight);
	}
	else
	{
		CG_FillRect(x, y + ALT_SCOREBOARD_3_ROW_HEIGHT, ALT_SCOREBOARD_3_ROW_WIDTH, ALT_SCOREBOARD_3_ROW_HEIGHT, currentCompactUiBack);
	}
}


void CG_DrawPlayerHeader3(float x, float y, int playerCount, float fade, vec4_t textColor, fontInfo_t *font)
{
	std::string playerHeader = ETJump::stringFormat("^7Players (%d)", playerCount);

	CG_Text_Paint_Centred_Ext(x, y + 9, 0.15f, 0.15f, textColor, playerHeader, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
}

void CG_AddPlayerToList3(float x, float y, float fpsCenterX, float infoX, float pingCenterX, score_t *score, vec4_t textColor, fontInfo_t *font)
{
	clientInfo_t *ci = &cgs.clientinfo[score->client];

	// Draw indicator if player is idle
	if (etj_drawScoreboardInactivity.integer && ci->clientIsInactive)
	{
		CG_DrawPic(x, y - 6, 8, 8, cgs.media.idleIcon);
		x += 10;
	}

	// Draw player
	CG_Text_Paint_Ext(x, y, 0.12f, 0.12f, textColor, ci->name, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);

	// Draw FPS
	std::string maxFPS = ETJump::stringFormat("%i", ci->maxFPS);
	CG_Text_Paint_Centred_Ext(fpsCenterX, y, 0.12f, 0.12f, textColor, maxFPS, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);

	// Draw client info
	const char *clientInfoPmove = ci->pmoveFixed ? "^2P" : "";
	const char *clientInfoCGaz = ci->CGaz ? "^8C" : "";
	const char* clientInfoSnapHUD = ci->snaphud ? "^5V" : "";
	const char *clientInfoHideMe = ci->hideMe ? "^bH" : "";
	const char *clientInfoSpecLock = ci->specLocked ? "^9S" : "";
	const char *clientInfoTimerun = ci->timerunActive ? "^3T" : "";
	const char *clientInfoTeam = ci->team == TEAM_ALLIES ? "^4B" : "^1R";

	CG_Text_Paint_Centred_Ext(infoX + 5, y, 0.12f, 0.12f, textColor, clientInfoPmove, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
	CG_Text_Paint_Centred_Ext(infoX + 12, y, 0.12f, 0.12f, textColor, clientInfoCGaz, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
	CG_Text_Paint_Centred_Ext(infoX + 19, y, 0.12f, 0.12f, textColor, clientInfoSnapHUD, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
	CG_Text_Paint_Centred_Ext(infoX + 26, y, 0.12f, 0.12f, textColor, clientInfoHideMe, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
	CG_Text_Paint_Centred_Ext(infoX + 33, y, 0.12f, 0.12f, textColor, clientInfoSpecLock, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
	CG_Text_Paint_Centred_Ext(infoX + 40, y, 0.12f, 0.12f, textColor, clientInfoTimerun, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
	CG_Text_Paint_Centred_Ext(infoX + 47, y, 0.12f, 0.12f, textColor, clientInfoTeam, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);

	// Draw client ping
	std::string ping = ETJump::stringFormat("%i", score->ping);
	CG_Text_Paint_Centred_Ext(pingCenterX, y, 0.12f, 0.12f, textColor, ping, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
}

void CG_DrawPlayerList3(float x, float y, float fade, vec4_t textColor, fontInfo_t *font)
{
	float playerCenterY = y + 8;

	// Draw info text for player list

	// Player
	std::string playerHeader = "^7Player";
	float playerX = ALT_SCOREBOARD_3_PLAYER_X;

	CG_Text_Paint_Ext(playerX, playerCenterY, 0.12f, 0.12f, textColor, playerHeader, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);

	// FPS
	std::string fpsHeader = "^7FPS";
	float fpsX = playerX + ALT_SCOREBOARD_3_PLAYER_WIDTH;
	float fpsCenterX = fpsX + ALT_SCOREBOARD_3_FPS_WIDTH / 2;

	CG_Text_Paint_Centred_Ext(fpsCenterX, playerCenterY, 0.12f, 0.12f, textColor, fpsHeader, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);

	// Info
	std::string infoHeader = "^7Info";
	float infoX = fpsX + ALT_SCOREBOARD_3_FPS_WIDTH;
	float infoCenterX = infoX + ALT_SCOREBOARD_3_INFO_WIDTH / 2;

	CG_Text_Paint_Centred_Ext(infoCenterX, playerCenterY, 0.12f, 0.12f, textColor, infoHeader, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);

	// Ping
	std::string pingHeader = "^7Ping";
	float pingX = infoX + ALT_SCOREBOARD_3_INFO_WIDTH;
	float pingCenterX = pingX + ALT_SCOREBOARD_3_PING_WIDTH / 2;

	CG_Text_Paint_Centred_Ext(pingCenterX, playerCenterY, 0.12f, 0.12f, textColor, pingHeader, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);

	// Get number of players and draw names + fill boxes
	for (int i = 0, j = 0; i < cg.numScores; i++)
	{
		team_t team = cgs.clientinfo[cg.scores[i].client].team;
		if (team != TEAM_AXIS && team != TEAM_ALLIES)
		{
			continue;
		}
		
		CG_DrawScoreboardRows3(x, y, j++, &cg.scores[i], fade);
		y += ALT_SCOREBOARD_3_ROW_HEIGHT;

		CG_AddPlayerToList3(playerX, playerCenterY + ALT_SCOREBOARD_3_ROW_HEIGHT, fpsCenterX, infoX, pingCenterX, &cg.scores[i], textColor, font);
		playerCenterY += ALT_SCOREBOARD_3_ROW_HEIGHT;
	}
}

void CG_DrawSpectatorHeader3(float x, float y, int spectatorCount, float fade, vec4_t textColor, fontInfo_t *font)
{
	std::string playerHeader = ETJump::stringFormat("^7Spectators (%d)", spectatorCount);

	CG_Text_Paint_Centred_Ext(x, y + 9, 0.15f, 0.15f, textColor, playerHeader, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
}

void CG_AddSpectatorToList3(float x, float y, float pingCenterX, score_t *score, vec4_t textColor, fontInfo_t *font)
{
	clientInfo_t *ci = &cgs.clientinfo[score->client];
	std::string connecting = "^3CONNECTING";
	std::string spectator = "^3SPECTATOR";
	std::string following = "^3>";
	std::string followedClient = cgs.clientinfo[score->followedClient].name;
	std::string ping = ETJump::stringFormat("%i", score->ping);
	float playerX = ALT_SCOREBOARD_3_PLAYER_X;
	float rightTextX = SCREEN_WIDTH - playerX;
	float connectingTextOffsetX = CG_Text_Width_Ext(connecting, 0.12f, 0, font);
	float spectatorTextOffsetX = CG_Text_Width_Ext(spectator, 0.12f, 0, font);
	float followedClientTextOffsetX = CG_Text_Width_Ext(followedClient, 0.12f, 0, font);

	// Draw indicator if player is idle
	if (etj_drawScoreboardInactivity.integer && ci->clientIsInactive)
	{
		CG_DrawPic(x, y - 6, 8, 8, cgs.media.idleIcon);
		playerX += 10;
	}

	// Draw player
	CG_Text_Paint_Ext(playerX, y, 0.12f, 0.12f, textColor, ci->name, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);

	// Draw connecting text, spectator text or followed client
	if (score->ping == -1)
	{
		CG_Text_Paint_Ext(rightTextX - connectingTextOffsetX - 25, y, 0.12f, 0.12f, textColor, connecting, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
	}
	else if (ci->clientNum == score->followedClient)
	{
		CG_Text_Paint_Ext(rightTextX - spectatorTextOffsetX - 25, y, 0.12f, 0.12f, textColor, spectator, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
	}
	else
	{
		CG_Text_Paint_Ext(SCREEN_CENTER_X, y, 0.12f, 0.12f, textColor, following, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
		CG_Text_Paint_Ext(rightTextX - followedClientTextOffsetX - 25, y, 0.12f, 0.12f, textColor, followedClient, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
	}

	// Draw ping
	CG_Text_Paint_Centred_Ext(pingCenterX, y, 0.12f, 0.12f, textColor, ping, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);
}

void CG_DrawSpectatorList3(float x, float y, float fade, vec4_t textColor, fontInfo_t *font)
{
	float playerCenterY = y + 8;
	int i, j;

	// Draw info text for spectator list

	// Player
	std::string playerHeader = "^7Player";
	float playerX = ALT_SCOREBOARD_3_PLAYER_X;

	CG_Text_Paint_Ext(playerX, playerCenterY, 0.12f, 0.12f, textColor, playerHeader, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);

	// Ping
	std::string pingHeader = "^7Ping";
	float pingX = playerX + ALT_SCOREBOARD_3_PLAYER_WIDTH + ALT_SCOREBOARD_3_FPS_WIDTH + ALT_SCOREBOARD_3_INFO_WIDTH;
	float pingCenterX = pingX + ALT_SCOREBOARD_3_PING_WIDTH / 2;

	CG_Text_Paint_Centred_Ext(pingCenterX, playerCenterY, 0.12f, 0.12f, textColor, pingHeader, 0, 0, ITEM_TEXTSTYLE_SHADOWED, font);

	// Get number of spectators and draw names + fill boxes
	for (i = 0, j = 0; i < cg.numScores; i++)
	{
		if (cgs.clientinfo[cg.scores[i].client].team != TEAM_SPECTATOR)
		{
			continue;
		}
		
		CG_DrawScoreboardRows3(x, y, j++, &cg.scores[i], fade);
		y += ALT_SCOREBOARD_3_ROW_HEIGHT;

		CG_AddSpectatorToList3(playerX, playerCenterY + ALT_SCOREBOARD_3_ROW_HEIGHT, pingCenterX, &cg.scores[i], textColor, font);
		playerCenterY += ALT_SCOREBOARD_3_ROW_HEIGHT;
	}
}


void CG_DrawAltScoreboard3(float fade)
{
	fontInfo_t *font = &cgs.media.limboFont1;
	vec4_t textColor = { 1.0f, 1.0f, 1.0f, 0 };
	textColor[3] = fade;
	vec4_t currentCompactUiBack;
	vec4_t currentCompactUiBackLight;
	vec4_t currentCompactUiBorder;
	Vector4Copy(compactUiBack, currentCompactUiBack);
	Vector4Copy(compactUiBack, currentCompactUiBackLight);
	Vector4Copy(compactUiBorder, currentCompactUiBorder);
	currentCompactUiBackLight[3] = 0.25f;
	currentCompactUiBack[3] *= fade;
	currentCompactUiBackLight[3] *= fade;
	currentCompactUiBorder[3] *= fade;
	float baseHeight = ALT_SCOREBOARD_3_HEADER_HEIGHT + (ALT_SCOREBOARD_3_DIVIDER * 2) + (ALT_SCOREBOARD_3_ROW_HEIGHT * 2);
	float currentHeight = baseHeight + (ALT_SCOREBOARD_3_ROW_HEIGHT * cg.numScores);
	float baseY = SCREEN_CENTER_Y - currentHeight / 2;
	int i;
	int spectatorCount = 0;
	int playerCount = 0;

	// Add each client to either playerCount or spectatorCount for size adjustments
	for (i = 0; i < cg.numScores; i++)
	{
		team_t team = cgs.clientinfo[cg.scores[i].client].team;
		switch (team)
		{
		case TEAM_SPECTATOR:
			spectatorCount++;
			break;
		case TEAM_ALLIES:
		case TEAM_AXIS:
			playerCount++;
			break;
		default:
			break;
		}
	}

	// Draw header
	float headerX = SCREEN_CENTER_X - ALT_SCOREBOARD_3_HEADER_WIDTH / 2;
	float headerY = baseY;

	CG_FillRect(headerX, headerY, ALT_SCOREBOARD_3_HEADER_WIDTH, ALT_SCOREBOARD_3_HEADER_HEIGHT, currentCompactUiBack);
	CG_DrawRect_FixedBorder(headerX, headerY, ALT_SCOREBOARD_3_HEADER_WIDTH, ALT_SCOREBOARD_3_HEADER_HEIGHT, 1, currentCompactUiBorder);
	CG_DrawHeader3(headerX, headerY, fade, textColor, font);
    
	// Draw player list header
	float playerHeaderX = SCREEN_CENTER_X;
	float playerHeaderY = headerY + ALT_SCOREBOARD_3_HEADER_HEIGHT;

	CG_DrawPlayerHeader3(playerHeaderX, playerHeaderY, playerCount, fade, textColor, font);

	// Draw player list
	float playerListX = SCREEN_CENTER_X - ALT_SCOREBOARD_3_ROW_WIDTH / 2;
	float playerListY = playerHeaderY + ALT_SCOREBOARD_3_DIVIDER;
	float playerListHeight = ALT_SCOREBOARD_3_ROW_HEIGHT + playerCount * ALT_SCOREBOARD_3_ROW_HEIGHT;

	CG_FillRect(playerListX, playerListY, ALT_SCOREBOARD_3_ROW_WIDTH, ALT_SCOREBOARD_3_ROW_HEIGHT, currentCompactUiBack);
	CG_DrawRect_FixedBorder(playerListX, playerListY, ALT_SCOREBOARD_3_ROW_WIDTH, ALT_SCOREBOARD_3_ROW_HEIGHT, 1, currentCompactUiBorder);

	CG_DrawPlayerList3(playerListX, playerListY, fade, textColor, font);

	CG_DrawRect_FixedBorder(playerListX, playerListY, ALT_SCOREBOARD_3_ROW_WIDTH, playerListHeight, 1, currentCompactUiBorder);

	// Draw spectator list header
	float spectatorHeaderX = SCREEN_CENTER_X;
	float spectatorHeaderY = playerListY + playerListHeight;

	CG_DrawSpectatorHeader3(spectatorHeaderX, spectatorHeaderY, spectatorCount, fade, textColor, font);

	// Draw spectator list
	float spectatorListX = SCREEN_CENTER_X - ALT_SCOREBOARD_3_ROW_WIDTH / 2;
	float spectatorListY = spectatorHeaderY + ALT_SCOREBOARD_3_DIVIDER;
	float spectatorListHeight = ALT_SCOREBOARD_3_ROW_HEIGHT + spectatorCount * ALT_SCOREBOARD_3_ROW_HEIGHT;

	CG_FillRect(spectatorListX, spectatorListY, ALT_SCOREBOARD_3_ROW_WIDTH, ALT_SCOREBOARD_3_ROW_HEIGHT, currentCompactUiBack);
	CG_DrawRect_FixedBorder(spectatorListX, spectatorListY, ALT_SCOREBOARD_3_ROW_WIDTH, ALT_SCOREBOARD_3_ROW_HEIGHT, 1, currentCompactUiBorder);

	CG_DrawSpectatorList3(spectatorListX, spectatorListY, fade, textColor, font);

	CG_DrawRect_FixedBorder(spectatorListX, spectatorListY, ALT_SCOREBOARD_3_ROW_WIDTH, spectatorListHeight, 1, currentCompactUiBorder);
}


/*
=================
WM_DrawObjectives
=================
*/

namespace INFO
{
	static const int TOTAL_WIDTH =   280;
	static const int PLAYER_WIDTH =  0.54 * TOTAL_WIDTH;
	static const int FPS_WIDTH =     0.09 * TOTAL_WIDTH;
	static const int PMOVE_WIDTH =   0.14 * TOTAL_WIDTH;
	static const int CGAZ_WIDTH =    0.12 * TOTAL_WIDTH;
	static const int LATENCY_WIDTH = 0.11 * TOTAL_WIDTH;
	static const int TEAM_HEIGHT =   20;
	static const int LINE_HEIGHT =   16;
	static const int MAX_LINES =     22;
}

int WM_DrawObjectives(int x, int y, int width, float fade)
{
	const char *s;
	auto       height = 26;
	float      textWidth;
	auto       font = &cgs.media.limboFont2;
	vec4_t     color1, color2;
	vec4_t currentClrUiBar, currentClrUiBack, currentClrUiBorder;
	Vector4Copy(clrUiBar, currentClrUiBar);
	Vector4Copy(clrUiBack, currentClrUiBack);
	Vector4Copy(clrUiBorder, currentClrUiBorder);
	currentClrUiBar[3]    *= fade;
	currentClrUiBack[3]   *= fade;
	currentClrUiBorder[3] *= fade;

	VectorCopy(colorMdGreen, color1);
	VectorCopy(colorWhite, color2);
	color1[3] = fade;
	color2[3] = fade;

	CG_FillRect(x, y, width, height, currentClrUiBack);
	CG_FillRect(x, y, width, height, currentClrUiBar);
	CG_DrawRect_FixedBorder(x, y, width, height, 1, currentClrUiBorder);

	// ETJump: ETJump version
	s = GAME_TAG;
	CG_Text_Paint_Ext(
		x + 5,
		y + 17,
		0.25f, 0.25f, color1, s, 0, 0, 0, font);

	// ETJump: map name on the center
	s = cgs.rawmapname;
	textWidth = CG_Text_Width_Ext(s, 0.25f, 0, font);
	CG_Text_Paint_Ext(
		x + width / 2 - textWidth / 2,
		y + 17, 
		0.25f, 0.25f, color2, s, 0, 0, 0, font);
	
	// ETJump: ETJump website link
	s = GAME_URL;
	textWidth = CG_Text_Width_Ext(s, 0.25f, 0, font);
	CG_Text_Paint_Ext(
		x + width - 5 - textWidth, 
		y + 17, 
		0.25f, 0.25f, color1, s, 0, 0, 0, font);

	return y + height;
}

static void WM_DrawClientScore(int x, int y, score_t *score, float fade)
{
	vec4_t       hcolor;
	clientInfo_t *ci;

	if (y + SMALLCHAR_HEIGHT >= 470)
	{
		return;
	}

	ci = &cgs.clientinfo[score->client];

	if (score->client == cg.snap->ps.clientNum)
	{
		Vector4Set(hcolor, .5f, .5f, .2f, fade * 0.3);
		CG_FillRect(x, y, INFO::TOTAL_WIDTH, INFO::LINE_HEIGHT, hcolor);
	}

	{
		const char *s;
		int textWidth;
		float offsetX = 0.0f;
		int nameMaxWidth = 18;
		// DHM - Nerve
		Vector4Set(hcolor, 1, 1, 1, fade);
		
		// draw class icons
		if (ci->team != TEAM_SPECTATOR)
		{
			int playerType;
			playerType = ci->cls;

			if (playerType == PC_MEDIC)
			{
			CG_DrawPic(x + 4, y + 2, 12, 12, cgs.media.skillPics[SK_FIRST_AID]);
			}
			else if (playerType == PC_ENGINEER)
			{
			CG_DrawPic(x + 4, y + 2, 12, 12, cgs.media.skillPics[SK_EXPLOSIVES_AND_CONSTRUCTION]);
			}
			else if (playerType == PC_FIELDOPS)
			{
			CG_DrawPic(x + 4, y + 2, 12, 12, cgs.media.skillPics[SK_SIGNALS]);
			}
			else if (playerType == PC_COVERTOPS)
			{
			CG_DrawPic(x + 4, y + 2, 12, 12, cgs.media.skillPics[SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS]);
			}
			else
			{
			CG_DrawPic(x + 4, y + 2, 12, 12, cgs.media.skillPics[SK_HEAVY_WEAPONS]);
			}

			offsetX = 12 + 2;
		}

		// draw indicators
		if (ci->timerunActive)
		{
			CG_DrawPic(x + 4 + offsetX, y + 2, 12, 12, cgs.media.stopwatchIcon);
			offsetX += 12 + 2;
			nameMaxWidth -= 2;
		}
		if (etj_drawScoreboardInactivity.integer && ci->clientIsInactive)
		{
			CG_DrawPic(x + 4 + offsetX, y + 2, 11, 11, cgs.media.idleIcon);
			offsetX += 11 + 2;
			nameMaxWidth -= 2;
		}

		if (ci->team == TEAM_SPECTATOR)
		{
			ETJump::DrawString(
				x + 4 + offsetX,
				y + 12,
				0.23f, 0.25f, hcolor, qfalse, ci->name, nameMaxWidth, 0);
			x += INFO::PLAYER_WIDTH;

			if (score->ping == -1)
			{
				s = CG_TranslateString("^3CONNECTING");
				ETJump::DrawSmallString(
					x + 13, 
					y + 12, 
					s, fade);
			}
			else if (ci->clientNum == score->followedClient)
			{
				s = CG_TranslateString("^3SPECTATOR");
				ETJump::DrawSmallString(
					x + 13, 
					y + 12, 
					s, fade);
			}
			else
			{
				s = va("%s%s", "^3> ^7", cgs.clientinfo[score->followedClient].name);
				ETJump::DrawString(x - 12, y + 12, 0.23f, 0.25f, hcolor, qfalse, s, 14, 0);
			}

			x += INFO::FPS_WIDTH + INFO::PMOVE_WIDTH + INFO::CGAZ_WIDTH;

			s = va("%i", score->ping);
			textWidth = ETJump::DrawStringWidth(s, 0.23f);
			ETJump::DrawSmallString(
				x + INFO::LATENCY_WIDTH / 2 - textWidth / 2 - 1,
				y + 12,
				s, fade);
		} 
		else
		{
			ETJump::DrawString(
				x + 4 + offsetX,
				y + 12,
				0.23f, 0.25f, hcolor, qfalse, ci->name, nameMaxWidth, 0);
			x += INFO::PLAYER_WIDTH;

			s = va("%i", ci->maxFPS);
			textWidth = ETJump::DrawStringWidth(s, 0.23f);
			ETJump::DrawSmallString(
				x + INFO::FPS_WIDTH / 2 - textWidth / 2 - 1,
				y + 12,
				s, fade);
			x += INFO::FPS_WIDTH;

			s = va("%i", ci->pmoveFixed);
			textWidth = ETJump::DrawStringWidth(s, 0.23f);
			ETJump::DrawSmallString(
				x + INFO::PMOVE_WIDTH / 2 - textWidth / 2 - 1,
				y + 12,
				s, fade);
			x += INFO::PMOVE_WIDTH;

			s = va("%s", ci->CGaz ? "Yes" : "No");
			textWidth = ETJump::DrawStringWidth(s, 0.23f);
			ETJump::DrawSmallString(
				x + INFO::CGAZ_WIDTH / 2 - textWidth / 2 - 1,
				y + 12,
				s, fade);
			x += INFO::CGAZ_WIDTH;

			s = va("%i", score->ping);
			textWidth = ETJump::DrawStringWidth(s, 0.23f);
			ETJump::DrawSmallString(
				x + INFO::LATENCY_WIDTH / 2 - textWidth / 2 - 1,
				y + 12,
				s, fade);
		}
	}
}

static int WM_TeamScoreboard(int x, int y, team_t team, float fade, int maxrows)
{
	vec4_t hcolor;
	int    width;
	vec4_t tclr  = { 0.6f, 0.6f, 0.6f, fade };
	auto   boldFont = &cgs.media.limboFont1;
	int    textWidth, tempy;
	const  char* text;
	vec4_t borderColor = { 0.0f, 0.0f, 0.0f, 0.6f * fade };
	vec4_t borderColor2 = { 0.0f, 0.0f, 0.0f, fade };
	vec4_t currentClrUiBar, currentClrUiBack;
	Vector4Copy(clrUiBack, currentClrUiBack);
	Vector4Copy(clrUiBar, currentClrUiBar);
	currentClrUiBack[3] *= fade;
	currentClrUiBar[3] *= fade;

	width = INFO::TOTAL_WIDTH;

	cg.teamPlayers[team] = 0; // JPW NERVE
	for (auto i = 0; i < cg.numScores; i++)
	{
		if (team != cgs.clientinfo[cg.scores[i].client].team)
		{
			continue;
		}

		cg.teamPlayers[team]++;
	}

	// team main header
	{
		CG_FillRect(x, y, width, INFO::TEAM_HEIGHT, currentClrUiBack);
		CG_FillRect(x, y, width, INFO::TEAM_HEIGHT, currentClrUiBar);

		Vector4Set(hcolor, 0, 0, 0, fade);
		CG_DrawRect_FixedBorder(x, y, width, INFO::TEAM_HEIGHT, 1, hcolor);

		if (team == TEAM_AXIS)
		{
			text = va("%s (%d %s)", CG_TranslateString("AXIS"), cg.teamPlayers[team], cg.teamPlayers[team] == 1 ? CG_TranslateString("PLAYER") : CG_TranslateString("PLAYERS"));
		}
		else
		{
			text = va("%s (%d %s)", CG_TranslateString("ALLIES"), cg.teamPlayers[team], cg.teamPlayers[team] == 1 ? CG_TranslateString("PLAYER") : CG_TranslateString("PLAYERS"));
		}

		textWidth = CG_Text_Width_Ext(text, 0.22f, 0, boldFont);
		CG_Text_Paint_Ext(
			x + width / 2 - textWidth / 2,
			y + 14,
			0.22f, 0.22f, tclr, text, 0, 0, 0, boldFont);

		y += INFO::TEAM_HEIGHT;
	}

	// team table header
	{
		CG_FillRect(x, y, width, INFO::LINE_HEIGHT, currentClrUiBack);
		CG_FillRect(x, y + INFO::LINE_HEIGHT - 1, width, 1, borderColor2);
		int tempx = x;

		// draw player info headings
		ETJump::DrawSmallString(tempx + 5, y + 12, CG_TranslateString("Name"), fade);
		tempx += INFO::PLAYER_WIDTH;

		ETJump::DrawSmallString(tempx, y + 12, CG_TranslateString("FPS"), fade);
		tempx += INFO::FPS_WIDTH;

		ETJump::DrawSmallString(tempx, y + 12, CG_TranslateString("Pmove"), fade);
		tempx += INFO::PMOVE_WIDTH;

		ETJump::DrawSmallString(tempx, y + 12, CG_TranslateString("CGaz"), fade);
		tempx += INFO::CGAZ_WIDTH;

		ETJump::DrawSmallString(tempx, y + 12, CG_TranslateString("Ping"), fade);

		y += INFO::LINE_HEIGHT;
	}

	// draw color bands
	tempy = y;
	for (auto i = 0; i < maxrows; i++)
	{
		if (i % 2 == 0)
		{
			Vector4Set(hcolor, 0.3f, 0.3f, 0.3f, fade * 0.3);
		}
		else
		{
			Vector4Set(hcolor, 0.0f, 0.0f, 0.0f, fade * 0.3);
		}

		// ETJump: background
		CG_FillRect(x, tempy, width, INFO::LINE_HEIGHT, hcolor);
		// ETJump: bottom border
		CG_FillRect(x, tempy + INFO::LINE_HEIGHT, width, 1, borderColor);

		tempy += INFO::LINE_HEIGHT;
	}

	// draw active players
	for (auto i = 0; i < cg.numScores && i < maxrows; i++)
	{
		if (team == cgs.clientinfo[cg.scores[i].client].team)
		{
			WM_DrawClientScore(x, y, &cg.scores[i], fade);
			y += INFO::LINE_HEIGHT;
		}
	}

	// draw spectators
	for (auto i = 0; i < cg.numScores && i < maxrows; i++)
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

		y += INFO::LINE_HEIGHT;
		WM_DrawClientScore(x, y, &cg.scores[i], fade);
	}

	return tempy;
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
	float fade;
	float *fadeColor;

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

	if (etj_altScoreboard.integer == 1)
	{
		CG_DrawAltScoreboard(fade);
		return qtrue;
	}
	
	if (etj_altScoreboard.integer == 2)
	{
		CG_DrawAltScoreboard2(fade);
		return qtrue;
	}

	if (etj_altScoreboard.integer == 3)
	{
		CG_DrawAltScoreboard3(fade);
		return qtrue;
	}

	auto x = SCREEN_OFFSET_X + 20;
	auto y = 10;

	y = WM_DrawObjectives(x, y, INFO::TOTAL_WIDTH * 2 + 40, fade);
	WM_TeamScoreboard(x, y + 5, TEAM_AXIS, fade, INFO::MAX_LINES);
	WM_TeamScoreboard(x + INFO::TOTAL_WIDTH + 40, y + 5, TEAM_ALLIES, fade, INFO::MAX_LINES);
	
	return qtrue;
}
