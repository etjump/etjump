/*
 * Crosshair stats HUD inspired by DeFRaG one.
 */

#include "cg_local.h"
#include "etj_utilities.h"
#include "../game/etj_numeric_utilities.h"

// TODO make font size configurable via cvars, see CG_DrawSpeed2()
#define CHSCHAR_SIZEX   0.2
#define CHSCHAR_SIZEY   0.2

static playerState_t *CG_CHS_GetPlayerState(void)
{
	if (cg.snap->ps.clientNum != cg.clientNum)
	{
		return &cg.snap->ps;
	}
	else
	{
		// return predictedPlayerState if not spectating
		return &cg.predictedPlayerState;
	}
}

static void CG_CHS_ViewTrace(trace_t *trace, int traceContents)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	vec3_t        start, end;

	VectorCopy(cg.refdef.vieworg, start);
	VectorMA(start, 131072, cg.refdef.viewaxis[0], end);
	CG_Trace(trace, start, vec3_origin, vec3_origin, end, ps->clientNum,
	         traceContents);
}

static void CG_CHS_Speed(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f", VectorLength(ps->velocity));
}

static void CG_CHS_Health(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%d", ps->stats[STAT_HEALTH]);
}

static void CG_CHS_Ammo(char *buf, int size)
{
	int ammo, clips, akimboammo;
	CG_PlayerAmmoValue(&ammo, &clips, &akimboammo);
	if (akimboammo >= 0)
	{
		Com_sprintf(buf, size, "%d|%d/%d", akimboammo, ammo, clips);
	}
	else if (clips >= 0)
	{
		Com_sprintf(buf, size, "%d/%d", ammo, clips);
	}
	else if (ammo >= 0)
	{
		Com_sprintf(buf, size, "%d", ammo);
	}
}

static void CG_CHS_Distance_XY(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	trace_t       trace;

	int traceContents = ETJump::checkExtraTrace(ETJump::CHS_10_11);

	CG_CHS_ViewTrace(&trace, traceContents);

	if (trace.fraction != 1.0)
	{
		Com_sprintf(buf, size, "%.0f",
		            sqrt(SQR(trace.endpos[0] - ps->origin[0])
		                 + SQR(trace.endpos[1] - ps->origin[1])));
	}
	else
	{
		Com_sprintf(buf, size, "-");
	}
}

static void CG_CHS_Distance_Z(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	trace_t trace;

	int traceContents = ETJump::checkExtraTrace(ETJump::CHS_10_11);

	CG_CHS_ViewTrace(&trace, traceContents);
	int offset = etj_CHSUseFeet.integer ? 24 : 0;

	if (trace.fraction != 1.0)
	{
		Com_sprintf(buf, size, "%.0f", trace.endpos[2] - (ps->origin[2] - offset));
	}
	else
	{
		Com_sprintf(buf, size, "-");
	}
}

static void CG_CHS_Distance_XYZ(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	trace_t trace;
	vec3_t origin;
	vec3_t originOffset{ 0, 0, 24 };

	int traceContents = ETJump::checkExtraTrace(ETJump::CHS_12);

	CG_CHS_ViewTrace(&trace, traceContents);

	VectorCopy(ps->origin, origin);
	if (trace.fraction != 1.0)
	{
		if (etj_CHSUseFeet.integer)
		{
			_VectorSubtract(origin, originOffset, origin);
		}
		Com_sprintf(buf, size, "%.0f", Distance(trace.endpos, origin));
	}
	else
	{
		Com_sprintf(buf, size, "-");
	}
}

static void CG_CHS_Distance_ViewXYZ(char *buf, int size)
{
	trace_t trace;

	int traceContents = ETJump::checkExtraTrace(ETJump::CHS_13_15);

	CG_CHS_ViewTrace(&trace, traceContents);

	if (trace.fraction != 1.0)
	{
		Com_sprintf(buf, size, "%.0f",
		            Distance(trace.endpos, cg.refdef.vieworg));
	}
	else
	{
		Com_sprintf(buf, size, "-");
	}
}

static void CG_CHS_Distance_XY_Z_XYZ(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	trace_t trace;
	vec3_t origin;
	vec3_t originOffset{ 0, 0, 24 };

	int traceContents = ETJump::checkExtraTrace(ETJump::CHS_13_15);

	CG_CHS_ViewTrace(&trace, traceContents);

	VectorCopy(ps->origin, origin);
	if (trace.fraction != 1.0)
	{
		if (etj_CHSUseFeet.integer)
		{
			_VectorSubtract(origin, originOffset, origin);
		}
		Com_sprintf(buf, size, "%.0f %.0f %.0f",
		            sqrt(SQR(trace.endpos[0] - origin[0])
		                 + SQR(trace.endpos[1] - origin[1])),
		            trace.endpos[2] - origin[2],
		            Distance(trace.endpos, origin));
	}
	else
	{
		Com_sprintf(buf, size, "- - -");
	}
}

static void CG_CHS_Distance_XY_Z_ViewXYZ(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	trace_t trace;
	vec3_t origin;
	vec3_t originOffset{ 0, 0, 24 };

	int traceContents = ETJump::checkExtraTrace(ETJump::CHS_13_15);

	CG_CHS_ViewTrace(&trace, traceContents);

	VectorCopy(ps->origin, origin);
	if (trace.fraction != 1.0)
	{
		if (etj_CHSUseFeet.integer)
		{
			_VectorSubtract(origin, originOffset, origin);
		}
		Com_sprintf(buf, size, "%.0f %.0f %.0f",
		            sqrt(SQR(trace.endpos[0] - origin[0])
		                 + SQR(trace.endpos[1] - origin[1])),
		            trace.endpos[2] - origin[2],
		            Distance(trace.endpos, cg.refdef.vieworg));
	}
	else
	{
		Com_sprintf(buf, size, "- - -");
	}
}

static void CG_CHS_Look_XYZ(char *buf, int size)
{
	trace_t trace;

	int traceContents = ETJump::checkExtraTrace(ETJump::CHS_16);

	CG_CHS_ViewTrace(&trace, traceContents);

	if (trace.fraction != 1.0)
	{
		Com_sprintf(buf, size, "%.0f %.0f %.0f",
		            trace.plane.dist * trace.plane.normal[0],
		            trace.plane.dist * trace.plane.normal[1],
		            trace.plane.dist * trace.plane.normal[2]);
	}
	else
	{
		Com_sprintf(buf, size, "- - -");
	}
}

static void CG_CHS_Speed_X(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f", ps->velocity[0]);
}

static void CG_CHS_Speed_Y(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f", ps->velocity[1]);
}

static void CG_CHS_Speed_Z(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f", ps->velocity[2]);
}

static void CG_CHS_Speed_XY(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f",
	            sqrt(SQR(ps->velocity[0]) + SQR(ps->velocity[1])));
}

static void CG_CHS_Speed_XYZ(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f", VectorLength(ps->velocity));
}

static void CG_CHS_Speed_Forward(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f",
	            DotProduct(ps->velocity, cg.refdef.viewaxis[0]));
}

static void CG_CHS_Speed_Sideways(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f",
	            DotProduct(ps->velocity, cg.refdef.viewaxis[1]));
}

static void CG_CHS_Speed_Forward_Sideways(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f %.0f",
	            DotProduct(ps->velocity, cg.refdef.viewaxis[0]),
	            DotProduct(ps->velocity, cg.refdef.viewaxis[1]));
}

static void CG_CHS_Speed_XY_Forward_Sideways(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f %.0f %.0f",
	            sqrt(SQR(ps->velocity[0]) + SQR(ps->velocity[1])),
	            DotProduct(ps->velocity, cg.refdef.viewaxis[0]),
	            DotProduct(ps->velocity, cg.refdef.viewaxis[1]));
}

static void CG_CHS_Pitch(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.2f", ps->viewangles[PITCH]);
}

static void CG_CHS_Yaw(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.2f", ps->viewangles[YAW]);
}

static void CG_CHS_Roll(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.2f", ps->viewangles[ROLL]);
}

static void CG_CHS_Position_X(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f", ps->origin[0]);
}

static void CG_CHS_Position_Y(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f", ps->origin[1]);
}

static void CG_CHS_Position_Z(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	int offset = etj_CHSUseFeet.integer ? 24 : 0;
	Com_sprintf(buf, size, "%.0f", (ps->origin[2] - offset));
}

static void CG_CHS_ViewPosition_X(char *buf, int size)
{
	Com_sprintf(buf, size, "%.0f", cg.refdef.vieworg[0]);
}

static void CG_CHS_ViewPosition_Y(char *buf, int size)
{
	Com_sprintf(buf, size, "%.0f", cg.refdef.vieworg[1]);
}

static void CG_CHS_ViewPosition_Z(char *buf, int size)
{
	Com_sprintf(buf, size, "%.0f", cg.refdef.vieworg[2]);
}

static void CG_CHS_Pitch_Yaw(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.2f %.2f",
	            ps->viewangles[PITCH], ps->viewangles[YAW]);
}

static void CG_CHS_Player_XYZ(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	int offset = etj_CHSUseFeet.integer ? 24 : 0;
	Com_sprintf(buf, size, "%.0f %.0f %.0f",
	            ps->origin[0], ps->origin[1], (ps->origin[2] - offset));
}

static void CG_CHS_Player_XYZ_Pitch_Yaw(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	int offset = etj_CHSUseFeet.integer ? 24 : 0;
	Com_sprintf(buf, size, "%.0f %.0f %.0f %.2f %.2f",
	            ps->origin[0], ps->origin[1], (ps->origin[2] - offset),
	            ps->viewangles[PITCH], ps->viewangles[YAW]);
}

static void CG_CHS_ViewPosition_XYZ_Pitch_Yaw(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f %.0f %.0f %.2f %.2f",
	            cg.refdef.vieworg[0], cg.refdef.vieworg[1], cg.refdef.vieworg[2],
	            ps->viewangles[PITCH], ps->viewangles[YAW]);
}

static void CG_CHS_Position_XYZ(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	int offset = etj_CHSUseFeet.integer ? 24 : 0;
	Com_sprintf(buf, size, "%.0f %.0f %.0f",
	            ps->origin[0], ps->origin[1], (ps->origin[2] - offset));
}

static void CG_CHS_ViewPosition_XYZ(char *buf, int size)
{
	Com_sprintf(buf, size, "%.0f %.0f %.0f",
	            cg.refdef.vieworg[0], cg.refdef.vieworg[1], cg.refdef.vieworg[2]);
}

static void CG_CHS_Angles_XYZ(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.2f %.2f %.2f",
	            ps->viewangles[PITCH], ps->viewangles[YAW], ps->viewangles[ROLL]);
}

static void CG_CHS_Velocity_XYZ(char *buf, int size)
{
	playerState_t *ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%.0f %.0f %.0f",
	            ps->velocity[0], ps->velocity[1], ps->velocity[2]);
}

static void CG_CHS_LastJumpPosition_XYZ(char *buf, int size)
{
	int offset = etj_CHSUseFeet.integer ? 24 : 0;
	Com_sprintf(buf, size, "%.0f %.0f %.0f", cg.etjLastJumpPos[0], cg.etjLastJumpPos[1], (cg.etjLastJumpPos[2] - offset));
}

static void CG_CHS_PlaneAngleZ(char *buf, int size)
{
	trace_t trace;

	int traceContents = ETJump::checkExtraTrace(ETJump::CHS_53);

	CG_CHS_ViewTrace(&trace, traceContents);

	float vec_x = trace.plane.normal[0];
	float vec_y = trace.plane.normal[1];
	float vec_z = trace.plane.normal[2];
	
	float angle_z = atan2(sqrt(pow(vec_x, 2) + pow(vec_y, 2)), vec_z) * 180 / M_PI;

	Com_sprintf(buf, size, "%.2f", angle_z);
}

static void CG_CHS_LastJumpSpeed(char* buf, int size)
{
	playerState_t* ps = CG_CHS_GetPlayerState();
	Com_sprintf(buf, size, "%d", ps->persistant[PERS_JUMP_SPEED]);
}

typedef struct
{
	void (*fun)(char *, int);
	const char *name;   // used as a CHS2 prefix
	const char *desc;   // used to display info about the stat
} stat_t;

/*
 * Arrays of all statistics.
 * Note: keep them synchronized to the DeFRaG CHS numbers (if possible).
 */
static stat_t stats[] =
{
/*   0 */ { NULL                                               },
/*   1 */ { CG_CHS_Speed, "Speed", "player speed"              },
/*   2 */ { CG_CHS_Health, "Health", "player health"           },
/*   3 */ { NULL                                               }, // armor
/*   4 */ { CG_CHS_Ammo, "Ammo", "player ammo for currently selected weapon"},
/*   5 */ { NULL                                               }, // health/armor
/*   6 */ { NULL                                               }, // health/armor/ammo
/*   7 */ { NULL                                               }, // empty
/*   8 */ { NULL                                               }, // empty
/*   9 */ { NULL                                               }, // empty

/*  10 */ { CG_CHS_Distance_XY, "Distance XY", "horizontal distance to plane"},
/*  11 */ { CG_CHS_Distance_Z, "Distance Z", "vertical distance to plane"},
/*  12 */ { CG_CHS_Distance_XYZ, "Distance XYZ", "true distance to plane"},
/*  13 */ { CG_CHS_Distance_ViewXYZ, "Distance ViewXYZ", "true distance to plane from view point"},
/*  14 */ { CG_CHS_Distance_XY_Z_XYZ, "Distance XY Z XYZ", "horizontal/vertical/true distance to plane"},
/*  15 */ { CG_CHS_Distance_XY_Z_ViewXYZ, "Distance XY Z ViewXYZ", "horizontal/vertical/true(view) distance to plane"},
/*  16 */ { CG_CHS_Look_XYZ, "Look XYZ", "world x y z location of plane"},
/*  17 */ { NULL                                               }, // empty
/*  18 */ { NULL                                               }, // empty
/*  19 */ { NULL                                               }, // empty

/*  20 */ { CG_CHS_Speed_X, "Speed X", "speed along world x axis"},
/*  21 */ { CG_CHS_Speed_Y, "Speed Y", "speed along world y axis"},
/*  22 */ { CG_CHS_Speed_Z, "Speed Z", "speed along world z axis"},
/*  23 */ { CG_CHS_Speed_XY, "Speed XY", "horizontal speed"    },
/*  24 */ { CG_CHS_Speed_XYZ, "Speed XYZ", "true speed"        },
/*  25 */ { CG_CHS_Speed_Forward, "Speed Forward", "speed relative to forward"},
/*  26 */ { CG_CHS_Speed_Sideways, "Speed Sideways", "speed relative to side"},
/*  27 */ { CG_CHS_Speed_Forward_Sideways, "Speed Forward Sideways", "speed relative to forward/side"},
/*  28 */ { CG_CHS_Speed_XY_Forward_Sideways, "Speed XY Forward Sideways", "horizontal speed/speed relative to forward/side"},
/*  29 */ { NULL                                               }, // empty

/*  30 */ { CG_CHS_Pitch, "Pitch", "player pitch"              },
/*  31 */ { CG_CHS_Yaw, "Yaw", "player yaw"                    },
/*  32 */ { CG_CHS_Roll, "Roll", "player roll"                 },
/*  33 */ { CG_CHS_Position_X, "Position X", "player X position"},
/*  34 */ { CG_CHS_Position_Y, "Position Y", "player Y position"},
/*  35 */ { CG_CHS_Position_Z, "Position Z", "player Z position"},
/*  36 */ { CG_CHS_ViewPosition_X, "View Position X", "view X position"},
/*  37 */ { CG_CHS_ViewPosition_Y, "View Position Y", "view Y position"},
/*  38 */ { CG_CHS_ViewPosition_Z, "View Position Z", "view Z position"},
/*  39 */ { NULL                                               }, // empty

/*  40 */ { CG_CHS_Pitch_Yaw, "Pitch Yaw", "player pitch/yaw"  },
/*  41 */ { CG_CHS_Player_XYZ, "Player XYZ", "player position in the world"},
/*  42 */ { CG_CHS_Player_XYZ_Pitch_Yaw, "Player XYZ Pitch Yaw", "player position in the world and pitch/yaw"},
/*  43 */ { CG_CHS_ViewPosition_XYZ_Pitch_Yaw, "View Position XYZ Pitch Yaw", "view position in the world and pitch/yaw"},
/*  44 */ { CG_CHS_Position_XYZ, "Position XYZ", "position x y z"},
/*  45 */ { CG_CHS_ViewPosition_XYZ, "View Position XYZ", "view position x y z"},
/*  46 */ { CG_CHS_Angles_XYZ, "Angles XYZ", "angles x y z"    },
/*  47 */ { CG_CHS_Velocity_XYZ, "Velocity XYZ", "velocity x y z"},
/*  48 */ { NULL                                               }, // empty
/*  49 */ { NULL                                               }, // empty

/*  50 */ { CG_CHS_LastJumpPosition_XYZ, "Jump XYZ", "jump x y z" },
/*  51 */ { NULL }, // Reserved for possible Plane Angle X
/*  52 */ { NULL }, // Reserved for possible Plane Angle Y
/*  53 */ { CG_CHS_PlaneAngleZ, "Plane Angle Z", "plane angle z" },
/*  54 */ { NULL }, // Reserved for possible Plane Angle XYZ
/*  55 */ { CG_CHS_LastJumpSpeed, "Last Jump Speed", "last jump speed" },


	{ NULL                                               }
};

static void CG_CHS_GetName(char *buf, int size, int statNum)
{
	if (!buf || size <= 0)
	{
		// no can do
		return;
	}

	// always terminate the buffer
	buf[0] = '\0';

	if (statNum < 0 || statNum >= sizeof(stats) / sizeof(stats[0])
	    || !stats[statNum].fun || !stats[statNum].name)
	{
		return;
	}

	// the stat is valid and has a name, fill the buffer
	Com_sprintf(buf, size, "%s: ", stats[statNum].name);
}

static void CG_CHS_GetValue(char *buf, int size, int statNum)
{
	if (!buf || size <= 0)
	{
		// no can do
		return;
	}

	// always terminate the buffer
	buf[0] = '\0';

	if (statNum < 0 || statNum >= sizeof(stats) / sizeof(stats[0])
	    || !stats[statNum].fun)
	{
		return;
	}

	// the stat is valid, fill the buffer
	stats[statNum].fun(buf, size);
}

// XXX isn't similar enum defined somewhere else already?
typedef enum
{
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT
} align_t;

static void CG_CHS_DrawSingleInfo(int x, int y, int stat, qboolean drawName, align_t align)
{
	char buf[1024];
	int  l = 0;
	int  x_off, y_off;

	// alpha, shadow and color stuff
	int textStyle = ITEM_TEXTSTYLE_NORMAL;
	float textAlpha = etj_CHSAlpha.value;
	vec4_t CHSColor = { 1.f, 1.f, 1.f, 1.f };

	if (etj_CHSShadow.integer > 0)
	{
		textStyle = ITEM_TEXTSTYLE_SHADOWED;
	}

	ETJump::parseColorString(etj_CHSColor.string, CHSColor);

	CHSColor[3] = Numeric::clamp(textAlpha, 0.0f, 1.0f);
	// end alpha, shadow and color stuff

	if (drawName)
	{
		CG_CHS_GetName(buf, sizeof(buf), stat);
		l = strlen(buf);
	}
	CG_CHS_GetValue(buf + l, sizeof(buf) - l, stat);

	x_off = CG_Text_Width_Ext(buf, CHSCHAR_SIZEX, 0, &cgs.media.limboFont1);
	y_off = CG_Text_Height_Ext(buf, CHSCHAR_SIZEY, 0, &cgs.media.limboFont1) / 2;
	switch (align)
	{
	case ALIGN_LEFT:
		x_off = 0;
		break;
	case ALIGN_CENTER:
		x_off /= 2;
		break;
	case ALIGN_RIGHT:
		break;
	}


	CG_Text_Paint_Ext(x - x_off, y + y_off, CHSCHAR_SIZEX, CHSCHAR_SIZEY,
		CHSColor, buf, 0, 0, textStyle, &cgs.media.limboFont1);
}

void CG_DrawCHS(void)
{
	int x, y;
	align_t CHS2Align = ALIGN_LEFT;

	// CHS1
	if (etj_drawCHS1.integer)
	{
		x = (SCREEN_WIDTH / 2) - 1;
		y = (SCREEN_HEIGHT / 2) - 1;
		/*      1
		 *   8     2
		 * 7    x    3
		 *   6     4
		 *      5
		 */
		CG_CHS_DrawSingleInfo(x, y - 20, etj_CHS1Info1.integer, qfalse, ALIGN_CENTER);
		CG_CHS_DrawSingleInfo(x + 10, y - 10, etj_CHS1Info2.integer, qfalse, ALIGN_LEFT);
		CG_CHS_DrawSingleInfo(x + 20, y, etj_CHS1Info3.integer, qfalse, ALIGN_LEFT);
		CG_CHS_DrawSingleInfo(x + 10, y + 10, etj_CHS1Info4.integer, qfalse, ALIGN_LEFT);
		CG_CHS_DrawSingleInfo(x, y + 20, etj_CHS1Info5.integer, qfalse, ALIGN_CENTER);
		CG_CHS_DrawSingleInfo(x - 10, y + 10, etj_CHS1Info6.integer, qfalse, ALIGN_RIGHT);
		CG_CHS_DrawSingleInfo(x - 20, y, etj_CHS1Info7.integer, qfalse, ALIGN_RIGHT);
		CG_CHS_DrawSingleInfo(x - 10, y - 10, etj_CHS1Info8.integer, qfalse, ALIGN_RIGHT);
	}

	// CHS2
	if (etj_drawCHS2.integer)
	{
		float chs2OffsetX = ETJump_AdjustPosition(etj_CHS2PosX.value);
		x = 30 + chs2OffsetX;
		y = (SCREEN_HEIGHT / 2) + 40 + etj_CHS2PosY.value;

		if (etj_drawCHS2.integer > 1) {
			CHS2Align = ALIGN_RIGHT;
			x = SCREEN_WIDTH - 30 + chs2OffsetX;
		}

		CG_CHS_DrawSingleInfo(x, y +  0, etj_CHS2Info1.integer, qtrue, CHS2Align);
		CG_CHS_DrawSingleInfo(x, y + 10, etj_CHS2Info2.integer, qtrue, CHS2Align);
		CG_CHS_DrawSingleInfo(x, y + 20, etj_CHS2Info3.integer, qtrue, CHS2Align);
		CG_CHS_DrawSingleInfo(x, y + 30, etj_CHS2Info4.integer, qtrue, CHS2Align);
		CG_CHS_DrawSingleInfo(x, y + 40, etj_CHS2Info5.integer, qtrue, CHS2Align);
		CG_CHS_DrawSingleInfo(x, y + 50, etj_CHS2Info6.integer, qtrue, CHS2Align);
		CG_CHS_DrawSingleInfo(x, y + 60, etj_CHS2Info7.integer, qtrue, CHS2Align);
		CG_CHS_DrawSingleInfo(x, y + 70, etj_CHS2Info8.integer, qtrue, CHS2Align);
	}
}

void CG_InfoCHS_f(void)
{
	int i;
	for (i = 0; i < sizeof(stats) / sizeof(stats[0]); i++)
	{
		if (!stats[i].fun || !stats[i].desc)
		{
			continue;
		}

		CG_Printf("% 3d: %s\n", i, stats[i].desc);
	}
}
