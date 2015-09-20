// G_referee.c: Referee handling
// -------------------------------
// 04 Apr 02
// rhea@OrangeSmoothie.org
//
#include "g_local.h"
#include "../../etjump/ui/menudef.h"


//
// UGH!  Clean me!!!!
//

// Parses for a referee command.
//	--> ref arg allows for the server console to utilize all referee commands (ent == NULL)
//
qboolean G_refCommandCheck(gentity_t *ent, char *cmd)
{
	if (!Q_stricmp(cmd, "help"))
	{
		G_refHelp_cmd(ent);
	}
	else if (!Q_stricmp(cmd, "putallies"))
	{
		G_refPlayerPut_cmd(ent, TEAM_ALLIES);
	}
	else if (!Q_stricmp(cmd, "putaxis"))
	{
		G_refPlayerPut_cmd(ent, TEAM_AXIS);
	}
	else if (!Q_stricmp(cmd, "remove"))
	{
		G_refRemove_cmd(ent);
	}
	else if (!Q_stricmp(cmd, "warn"))
	{
		G_refWarning_cmd(ent);
	}
	else if (!Q_stricmp(cmd, "mute"))
	{
		G_refMute_cmd(ent, qtrue);
	}
	else if (!Q_stricmp(cmd, "unmute"))
	{
		G_refMute_cmd(ent, qfalse);
	}
	else
	{
		return(qfalse);
	}

	return(qtrue);
}

// Lists ref commands.
void G_refHelp_cmd(gentity_t *ent)
{
	// List commands only for enabled refs.
	if (ent)
	{
		if (!ent->client->sess.referee)
		{
			CP("cpm \"Sorry, you are not a referee!\n");
			return;
		}

		CP("print \"\n^3Referee commands:^7\n\"");
		CP("print \"------------------------------------------\n\"");

		G_voteHelp(ent, qfalse);

		CP("print \"Usage: ^3\\ref <cmd> [params]\n\n\"");

		// Help for the console
	}
	else
	{
		G_Printf("Usage: <cmd> [params]\n\n");
	}
}
// Console ref kick
void G_refKick(char *name)
{
	int clientNum, timeout = 300;

	if (!*name)
	{
		G_Printf("usage: ref kick <name>\n");
		return;
	}

	if ((clientNum = ClientNumberFromString(NULL, name)) == -1)
	{
		G_Printf("Client not found!\n");
		return;
	}

	trap_DropClient(clientNum, "Player kicked!", timeout);
	return;
}
// Console cmd check
void G_ref_con()
{
	char arg[MAX_TOKEN_CHARS];

	trap_Argv(1, arg, sizeof(arg));

	if (!Q_stricmp(arg, "kick"))
	{
		trap_Argv(2, arg, sizeof(arg));
		G_refKick(arg);
		return;
	}

	if (G_refCommandCheck(NULL, arg))
	{
		return;
	}
	else
	{
		G_Printf("Invalid ref command.\n");
	}
}


// Request for ref status or lists ref commands.
void G_ref_cmd(gentity_t *ent, unsigned int dwCommand, qboolean fValue)
{
	char arg[MAX_TOKEN_CHARS];


	// Roll through ref commands if already a ref
	if (ent == NULL || ent->client->sess.referee)
	{
		voteInfo_t votedata;

		trap_Argv(1, arg, sizeof(arg));

		memcpy(&votedata, &level.voteInfo, sizeof(voteInfo_t));

		if (Cmd_CallVote_f(ent, 0, qtrue))
		{
			memcpy(&level.voteInfo, &votedata, sizeof(voteInfo_t));
			return;
		}
		else
		{
			memcpy(&level.voteInfo, &votedata, sizeof(voteInfo_t));

			if (G_refCommandCheck(ent, arg))
			{
				return;
			}
			else
			{
				G_refHelp_cmd(ent);
			}
		}
		return;
	}

	if (ent)
	{
		if (!Q_stricmp(refereePassword.string, "none") || !refereePassword.string[0])
		{
			CP("cpm \"Sorry, referee status disabled on this server.\n\"");
			return;
		}

		if (trap_Argc() < 2)
		{
			CP("cpm \"Usage: ref [password]\n\"");
			return;
		}

		trap_Argv(1, arg, sizeof(arg));

		if (Q_stricmp(arg, refereePassword.string))
		{
			CP("cpm \"Invalid referee password!\n\"");
			return;
		}

		ent->client->sess.referee     = 1;
		ent->client->sess.spec_invite = TEAM_AXIS | TEAM_ALLIES;
		AP(va("cp \"%s\n^3has become a referee\n\"", ent->client->pers.netname));
		ClientUserinfoChanged(ent - g_entities);
	}
}

// Puts a player on a team.
void G_refPlayerPut_cmd(gentity_t *ent, int team_id)
{
	int       pid;
	char      arg[MAX_TOKEN_CHARS];
	gentity_t *player;

	// Works for teamplayish matches
	if (g_gametype.integer < GT_WOLF)
	{
		G_refPrintf(ent, "\"put[allies|axis]\" only for team-based games!");
		return;
	}

	// Find the player to place.
	trap_Argv(2, arg, sizeof(arg));
	if ((pid = ClientNumberFromString(ent, arg)) == -1)
	{
		return;
	}

	player = g_entities + pid;

	// Can only move to other teams.
	if (player->client->sess.sessionTeam == team_id)
	{
		G_refPrintf(ent, "\"%s\" is already on team %s!\n", player->client->pers.netname, aTeams[team_id]);
		return;
	}

	if (team_maxplayers.integer && TeamCount(-1, team_id) >= team_maxplayers.integer)
	{
		G_refPrintf(ent, "Sorry, the %s team is already full!\n", aTeams[team_id]);
		return;
	}

	player->client->pers.invite = team_id;
	player->client->pers.ready  = qfalse;

	if (team_id == TEAM_AXIS)
	{
		SetTeam(player, "red", qtrue, -1, -1, qfalse);
	}
	else
	{
		SetTeam(player, "blue", qtrue, -1, -1, qfalse);
	}

	if (g_gamestate.integer == GS_WARMUP || g_gamestate.integer == GS_WARMUP_COUNTDOWN)
	{
		G_readyMatchState();
	}
}


// Removes a player from a team.
void G_refRemove_cmd(gentity_t *ent)
{
	int       pid;
	char      arg[MAX_TOKEN_CHARS];
	gentity_t *player;

	// Works for teamplayish matches
	if (g_gametype.integer < GT_WOLF)
	{
		G_refPrintf(ent, "\"remove\" only for team-based games!");
		return;
	}

	// Find the player to remove.
	trap_Argv(2, arg, sizeof(arg));
	if ((pid = ClientNumberFromString(ent, arg)) == -1)
	{
		return;
	}

	player = g_entities + pid;

	// Can only remove active players.
	if (player->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		G_refPrintf(ent, "You can only remove people in the game!");
		return;
	}

	// Announce the removal
	AP(va("cp \"%s\n^7removed from team %s\n\"", player->client->pers.netname, aTeams[player->client->sess.sessionTeam]));
	CPx(pid, va("print \"^5You've been removed from the %s team\n\"", aTeams[player->client->sess.sessionTeam]));

	SetTeam(player, "s", qtrue, -1, -1, qfalse);

	if (g_gamestate.integer == GS_WARMUP || g_gamestate.integer == GS_WARMUP_COUNTDOWN)
	{
		G_readyMatchState();
	}
}

void G_refWarning_cmd(gentity_t *ent)
{
	char cmd[MAX_TOKEN_CHARS];
	char reason[MAX_TOKEN_CHARS];
	int  kicknum;

	trap_Argv(2, cmd, sizeof(cmd));

	if (!*cmd)
	{
		G_refPrintf(ent, "usage: ref warn <clientname> [reason].");
		return;
	}

	trap_Argv(3, reason, sizeof(reason));

	kicknum = G_refClientnumForName(ent, cmd);

	if (kicknum != MAX_CLIENTS)
	{
		if (level.clients[kicknum].sess.referee == RL_NONE || ((!ent || ent->client->sess.referee == RL_RCON) && level.clients[kicknum].sess.referee <= RL_REFEREE))
		{
			trap_SendServerCommand(-1, va("cpm \"%s^7 was issued a ^1Warning^7 (%s)\n\"\n", level.clients[kicknum].pers.netname, *reason ? reason : "No Reason Supplied"));
		}
		else
		{
			G_refPrintf(ent, "Insufficient rights to issue client a warning.");
		}
	}
}

// (Un)Mutes a player
void G_refMute_cmd(gentity_t *ent, qboolean mute)
{
	int       pid;
	char      arg[MAX_TOKEN_CHARS];
	gentity_t *player;

	// Find the player to mute.
	trap_Argv(2, arg, sizeof(arg));
	if ((pid = ClientNumberFromString(ent, arg)) == -1)
	{
		return;
	}

	player = g_entities + pid;

	if (player->client->sess.referee != RL_NONE)
	{
		G_refPrintf(ent, "Cannot mute a referee.\n");
		return;
	}

	if (player->client->sess.muted == mute)
	{
		G_refPrintf(ent, "\"%s^*\" %s\n", player->client->pers.netname, mute ? "is already muted!" : "is not muted!");
		return;
	}

	if (mute)
	{
		char *ip;
		char userinfo[MAX_INFO_STRING];
		CPx(pid, "print \"^5You've been muted\n\"");
		player->client->sess.muted = qtrue;
		trap_GetUserinfo(player->client->ps.clientNum, userinfo, sizeof(userinfo));
		ip = Info_ValueForKey(userinfo, "ip");
		G_AddIpMute(ip);
		G_Printf("\"%s^*\" has been muted\n", player->client->pers.netname);
		ClientUserinfoChanged(pid);
	}
	else
	{
		char *ip;
		char userinfo[MAX_INFO_STRING];
		CPx(pid, "print \"^5You've been unmuted\n\"");
		player->client->sess.muted = qfalse;
		trap_GetUserinfo(player->client->ps.clientNum, userinfo, sizeof(userinfo));
		ip = Info_ValueForKey(userinfo, "ip");
		G_RemoveIPMute(ip);
		G_Printf("\"%s^*\" has been unmuted\n", player->client->pers.netname);
		ClientUserinfoChanged(pid);
	}
}

//////////////////////////////
//  Client authentication
//
void Cmd_AuthRcon_f(gentity_t *ent)
{
	char buf[MAX_TOKEN_CHARS], cmd[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer("rconPassword", buf, sizeof(buf));
	trap_Argv(1, cmd, sizeof(cmd));

	if (*buf && !strcmp(buf, cmd))
	{
		ent->client->sess.referee = RL_RCON;
	}
}


//////////////////////////////
//  Console admin commands
//
void G_PlayerBan()
{
	char cmd[MAX_TOKEN_CHARS];
	int  bannum;

	trap_Argv(1, cmd, sizeof(cmd));

	if (!*cmd)
	{
		G_Printf("usage: ban <clientname>.");
		return;
	}

	bannum = G_refClientnumForName(NULL, cmd);

	if (bannum != MAX_CLIENTS)
	{
//		if( level.clients[bannum].sess.referee != RL_RCON ) {
		const char *value;
		char       userinfo[MAX_INFO_STRING];

		trap_GetUserinfo(bannum, userinfo, sizeof(userinfo));
		value = Info_ValueForKey(userinfo, "ip");

		AddIPBan(value);
//		} else {
//			G_Printf( "^3*** Can't ban a superuser!\n" );
//		}
	}
}

void G_MakeReferee()
{
	char cmd[MAX_TOKEN_CHARS];
	int  cnum;

	trap_Argv(1, cmd, sizeof(cmd));

	if (!*cmd)
	{
		G_Printf("usage: MakeReferee <clientname>.");
		return;
	}

	cnum = G_refClientnumForName(NULL, cmd);

	if (cnum != MAX_CLIENTS)
	{
		if (level.clients[cnum].sess.referee == RL_NONE)
		{
			level.clients[cnum].sess.referee = RL_REFEREE;
			AP(va("cp \"%s\n^3has been made a referee\n\"", cmd));
			G_Printf("%s has been made a referee.\n", cmd);
		}
		else
		{
			G_Printf("User is already authed.\n");
		}
	}
}

void G_RemoveReferee()
{
	char cmd[MAX_TOKEN_CHARS];
	int  cnum;

	trap_Argv(1, cmd, sizeof(cmd));

	if (!*cmd)
	{
		G_Printf("usage: RemoveReferee <clientname>.");
		return;
	}

	cnum = G_refClientnumForName(NULL, cmd);

	if (cnum != MAX_CLIENTS)
	{
		if (level.clients[cnum].sess.referee == RL_REFEREE)
		{
			level.clients[cnum].sess.referee = RL_NONE;
			G_Printf("%s is no longer a referee.\n", cmd);
		}
		else
		{
			G_Printf("User is not a referee.\n");
		}
	}
}

void G_MuteClient()
{
	char cmd[MAX_TOKEN_CHARS];
	int  cnum;

	trap_Argv(1, cmd, sizeof(cmd));

	if (!*cmd)
	{
		G_Printf("usage: Mute <clientname>.");
		return;
	}

	cnum = G_refClientnumForName(NULL, cmd);

	if (cnum != MAX_CLIENTS)
	{
		if (level.clients[cnum].sess.referee != RL_RCON)
		{
			trap_SendServerCommand(cnum, va("cpm \"^3You have been muted\""));
			level.clients[cnum].sess.muted = qtrue;
			G_Printf("%s^* has been muted\n", cmd);
			ClientUserinfoChanged(cnum);
		}
		else
		{
			G_Printf("Cannot mute a referee.\n");
		}
	}
}

void G_UnMuteClient()
{
	char cmd[MAX_TOKEN_CHARS];
	int  cnum;

	trap_Argv(1, cmd, sizeof(cmd));

	if (!*cmd)
	{
		G_Printf("usage: Unmute <clientname>.\n");
		return;
	}

	cnum = G_refClientnumForName(NULL, cmd);

	if (cnum != MAX_CLIENTS)
	{
		if (level.clients[cnum].sess.muted)
		{
			trap_SendServerCommand(cnum, va("cpm \"^2You have been un-muted\""));
			level.clients[cnum].sess.muted = qfalse;
			G_Printf("%s has been un-muted\n", cmd);
			ClientUserinfoChanged(cnum);
		}
		else
		{
			G_Printf("User is not muted.\n");
		}
	}
}


/////////////////
//   Utility
//
int G_refClientnumForName(gentity_t *ent, const char *name)
{
	char cleanName[MAX_TOKEN_CHARS];
	int  i;

	if (!*name)
	{
		return(MAX_CLIENTS);
	}

	for (i = 0; i < level.numConnectedClients; i++)
	{
		Q_strncpyz(cleanName, level.clients[level.sortedClients[i]].pers.netname, sizeof(cleanName));
		Q_CleanStr(cleanName);
		if (!Q_stricmp(cleanName, name))
		{
			return(level.sortedClients[i]);
		}
	}

	G_refPrintf(ent, "Client not on server.");

	return(MAX_CLIENTS);
}

void G_refPrintf(gentity_t *ent, const char *fmt, ...)
{
	va_list argptr;
	char    text[1024];

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
