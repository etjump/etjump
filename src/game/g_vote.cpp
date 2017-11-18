// g_vote.c: All callvote handling
// -------------------------------
//
#include "g_local.h"
#include "../../etjump/ui/menudef.h"    // For vote options



#define T_FFA   0x01
#define T_1V1   0x02
#define T_SP    0x04
#define T_TDM   0x08
#define T_CTF   0x10
#define T_WLF   0x20
#define T_WSW   0x40
#define T_WCP   0x80
#define T_WCH   0x100


static const char *ACTIVATED = "ACTIVATED";
static const char *DEACTIVATED = "DEACTIVATED";
static const char *ENABLED  = "ENABLED";
static const char *DISABLED = "DISABLED";

//
// Update info:
//	1. Add line to aVoteInfo w/appropriate info
//	2. Add implementation for attempt and success (see an existing command for an example)
//
typedef struct
{
	unsigned int dwGameTypes;
	const char *pszVoteName;
	int (*pVoteCommand)(gentity_t * ent, unsigned int dwVoteIndex, char *arg, char *arg2);
	const char *pszVoteMessage;
	const char *pszVoteHelp;
} vote_reference_t;

// VC optimizes for dup strings :)
static const vote_reference_t aVoteInfo[] =
{
	{ 0x1ff, "map",        G_Map_v,        "Change map to",
	  " <mapname>^7\n  Votes for a new map to be loaded" },
	{ 0x1ff, "maprestart", G_MapRestart_v, "Map Restart",
	  "^7\n  Restarts the current map in progress" },
	{ 0x1ff, "randommap",  G_RandomMap_v,  "Random Map",
	  " ^7\n Votes a new random map to be loaded" },
	{ 0,     0,            NULL,           0 }
};

void G_cpmPrintf(gentity_t *ent, const char *fmt, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, fmt);
	Q_vsnprintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	if (ent == NULL)
	{
		trap_Printf(text);
	}
	else
	{
		CP(va("cpm \"%s\n\"", text));
	}
}

// Checks for valid custom callvote requests from the client.
int G_voteCmdCheck(gentity_t *ent, char *arg, char *arg2)
{
	unsigned int i, cVoteCommands = sizeof(aVoteInfo) / sizeof(aVoteInfo[0]);

	for (i = 0; i < cVoteCommands; i++)
	{
		if (!Q_stricmp(arg, aVoteInfo[i].pszVoteName))
		{
			int hResult = aVoteInfo[i].pVoteCommand(ent, i, arg, arg2);

			if (hResult == G_OK)
			{
				Com_sprintf(arg, VOTE_MAXSTRING, aVoteInfo[i].pszVoteMessage);
				level.voteInfo.vote_fn = aVoteInfo[i].pVoteCommand;
			}
			else
			{
				level.voteInfo.vote_fn = NULL;
			}

			return(hResult);
		}
	}

	return(G_NOTFOUND);
}


// Voting help summary.
void G_voteHelp(gentity_t *ent, qboolean fShowVote)
{
	int i, rows = 0, num_cmds = sizeof(aVoteInfo) / sizeof(aVoteInfo[0]) - 1; // Remove terminator;
	int vi[100];            // Just make it large static.


	if (fShowVote)
	{
		CP("print \"\nValid ^3callvote^7 commands are:\n^3----------------------------\n\"");
	}

	for (i = 0; i < num_cmds; i++)
	{
		if (aVoteInfo[i].dwGameTypes & (1 << g_gametype.integer))
		{
			vi[rows++] = i;
		}
	}

	num_cmds = rows;
	rows = num_cmds / HELP_COLUMNS;

	if (num_cmds % HELP_COLUMNS)
	{
		rows++;
	}
	if (rows < 0)
	{
		return;
	}

	for (i = 0; i < rows; i++)
	{
		if (i + rows * 3 + 1 <= num_cmds)
		{
			G_cpmPrintf(ent, "^5%-17s%-17s%-17s%-17s", aVoteInfo[vi[i]].pszVoteName,
				aVoteInfo[vi[i + rows]].pszVoteName,
				aVoteInfo[vi[i + rows * 2]].pszVoteName,
				aVoteInfo[vi[i + rows * 3]].pszVoteName);
		}
		else if (i + rows * 2 + 1 <= num_cmds)
		{
			G_cpmPrintf(ent, "^5%-17s%-17s%-17s", aVoteInfo[vi[i]].pszVoteName,
				aVoteInfo[vi[i + rows]].pszVoteName,
				aVoteInfo[vi[i + rows * 2]].pszVoteName);
		}
		else if (i + rows + 1 <= num_cmds)
		{
			G_cpmPrintf(ent, "^5%-17s%-17s", aVoteInfo[vi[i]].pszVoteName,
				aVoteInfo[vi[i + rows]].pszVoteName);
		}
		else
		{
			G_cpmPrintf(ent, "^5%-17s", aVoteInfo[vi[i]].pszVoteName);
		}
	}

	if (fShowVote)
	{
		CP("print \"\nUsage: ^3\\callvote <command> <params>\n^7For current settings/help, use: ^3\\callvote <command> ?\n\n\"");
	}

	return;
}

// Set disabled votes for client UI
void G_voteFlags(void)
{
	int i, flags = 0;

	for (i = 0; i < numVotesAvailable; i++)
	{
		if (trap_Cvar_VariableIntegerValue(voteToggles[i].pszCvar) == 0)
		{
			flags |= voteToggles[i].flag;
		}
	}

	if (flags != voteFlags.integer)
	{
		trap_Cvar_Set("voteFlags", va("%d", flags));
	}
}

// Prints specific callvote command help description.
qboolean G_voteDescription(gentity_t *ent, int cmd)
{
	char arg[MAX_TOKEN_CHARS];

	if (!ent)
	{
		return(qfalse);
	}

	trap_Argv(2, arg, sizeof(arg));
	if (!Q_stricmp(arg, "?") || trap_Argc() == 2)
	{
		trap_Argv(1, arg, sizeof(arg));
		G_cpmPrintf(ent, "\nUsage: ^3\\vote %s%s\n", arg, aVoteInfo[cmd].pszVoteHelp);
		return(qtrue);
	}

	return(qfalse);
}


// Localize disable message info.
void G_voteDisableMessage(gentity_t *ent, const char *cmd)
{
	G_cpmPrintf(ent, "Sorry, [lof]^3%s^7 [lon]voting has been disabled", cmd);
}


// Player ID message stub.
void G_playersMessage(gentity_t *ent)
{
	G_cpmPrintf(ent, "Use the ^3players^7 command to find a valid player ID.");
}


// Localize current parameter setting.
void G_voteCurrentSetting(gentity_t *ent, const char *cmd, const char *setting)
{
	G_cpmPrintf(ent, "^2%s^7 is currently ^3%s\n", cmd, setting);
}


// Vote toggling
int G_voteProcessOnOff(gentity_t *ent, char *arg, char *arg2, int curr_setting, int vote_allow, int vote_type)
{
	if (!vote_allow && ent)
	{
		G_voteDisableMessage(ent, aVoteInfo[vote_type].pszVoteName);
		G_voteCurrentSetting(ent, aVoteInfo[vote_type].pszVoteName, ((curr_setting) ? ENABLED : DISABLED));
		return(G_INVALID);
	}
	if (G_voteDescription(ent, vote_type))
	{
		G_voteCurrentSetting(ent, aVoteInfo[vote_type].pszVoteName, ((curr_setting) ? ENABLED : DISABLED));
		return(G_INVALID);
	}

	if ((atoi(arg2) && curr_setting) || (!atoi(arg2) && !curr_setting))
	{
		G_cpmPrintf(ent, "^3%s^5 is already %s!", aVoteInfo[vote_type].pszVoteName, ((curr_setting) ? ENABLED : DISABLED));
		return(G_INVALID);
	}

	Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%s", arg2);
	Com_sprintf(arg2, VOTE_MAXSTRING, "%s", (atoi(arg2)) ? ACTIVATED : DEACTIVATED);

	return(G_OK);
}


//
// Several commands to help with cvar setting
//
void G_voteSetOnOff(const char *desc, const char *cvar)
{
	AP(va("cpm \"^3%s is: ^5%s\n\"", desc, (atoi(level.voteInfo.vote_value)) ? ENABLED : DISABLED));
	//trap_SendConsoleCommand(EXEC_APPEND, va("%s %s\n", cvar, level.voteInfo.vote_value));
	trap_Cvar_Set(cvar, level.voteInfo.vote_value);
}

void G_voteSetValue(const char *desc, const char *cvar)
{
	AP(va("cpm \"^3%s set to: ^5%s\n\"", desc, level.voteInfo.vote_value));
	//trap_SendConsoleCommand(EXEC_APPEND, va("%s %s\n", cvar, level.voteInfo.vote_value));
	trap_Cvar_Set(cvar, level.voteInfo.vote_value);
}

void G_voteSetVoteString(const char *desc)
{
	AP(va("print \"^3%s set to: ^5%s\n\"", desc, level.voteInfo.vote_value));
	trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", level.voteInfo.voteString));
}






////////////////////////////////////////////////////////
//
// Actual vote command implementation
//
////////////////////////////////////////////////////////

int G_RandomMap_v(gentity_t *ent, unsigned dwVoteIndex, char *arg,
                  char *arg2)
{
	const char *map = NULL;

	// We know that arg2 is a type that exists.
	if (arg)
	{
		if (strlen(arg2) > 0)
		{
			map = GetRandomMapByType(arg2);

			if (strlen(map) == 0)
			{
				C_ChatPrintTo(ent, "^3randommap: ^7no maps on the requested map list.");
				return G_INVALID;
			}
			else if (!G_MapExists(map))
			{
				C_ChatPrintTo(ent, va("^1ERROR: ^7map %s is not on the server. Please update customvotes.json.", map));
				return G_INVALID;
			}
		}
		else
		{
			map = GetRandomMap();
		}

		Q_strncpyz(level.voteInfo.vote_value,
			map, sizeof(level.voteInfo.vote_value));
	}
	else
	{
		trap_SendConsoleCommand(EXEC_APPEND,
			va("map %s\n", level.voteInfo.vote_value));
	}

	return G_OK;
}


// *** Map - simpleton: we dont verify map is allowed/exists ***
int G_Map_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2)
{
	// Vote request (vote is being initiated)
	if (arg)
	{
		char serverinfo[MAX_INFO_STRING];
		trap_GetServerinfo(serverinfo, sizeof(serverinfo));

		if (!vote_allow_map.integer && ent)
		{
			G_voteDisableMessage(ent, arg);
			G_voteCurrentSetting(ent, arg, Info_ValueForKey(serverinfo, "mapname"));
			return(G_INVALID);
		}
		if (G_voteDescription(ent, dwVoteIndex))
		{
			G_voteCurrentSetting(ent, arg, Info_ValueForKey(serverinfo, "mapname"));
			return(G_INVALID);
		}

		Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%s", arg2);

		G_increaseCallvoteCount(arg2);

		// Vote action (vote has passed)
	}
	else
	{
		char s[MAX_STRING_CHARS];

		G_increasePassedCount(level.voteInfo.vote_value);

		if (g_gametype.integer == GT_WOLF_CAMPAIGN)
		{
			trap_Cvar_VariableStringBuffer("nextcampaign", s, sizeof(s));
			trap_SendConsoleCommand(EXEC_APPEND, va("campaign %s%s\n", level.voteInfo.vote_value, ((*s) ? va("; set nextcampaign \"%s\"", s) : "")));
		}
		else
		{
			Svcmd_ResetMatch_f(qtrue, qfalse);
			trap_Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
			trap_SendConsoleCommand(EXEC_APPEND, va("map %s%s\n", level.voteInfo.vote_value, ((*s) ? va("; set nextmap \"%s\"", s) : "")));
		}
	}

	return(G_OK);
}

// *** Map Restart ***
int G_MapRestart_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2)
{
	// Vote request (vote is being initiated)
	if (arg)
	{
		if (trap_Argc() > 2)
		{
			if (!Q_stricmp(arg2, "?"))
			{
				G_cpmPrintf(ent, "Usage: ^3\\callvote %s%s\n", arg, aVoteInfo[dwVoteIndex].pszVoteHelp);
				return(G_INVALID);
			}
		}

		// Vote action (vote has passed)
	}
	else
	{
		// Restart the map back to warmup
		Svcmd_ResetMatch_f(qfalse, qtrue);
		AP("cp \"^1*** Level Restarted! ***\n\"");
	}

	return(G_OK);
}
