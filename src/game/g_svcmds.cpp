


// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"


/*
==============================================================================

PACKET FILTERING


You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and you can use '*' to match any value
so you can specify an entire class C network with "addip 192.246.40.*"

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.

TTimo NOTE: GUID functions are copied over from the model of IP banning,
used to enforce max lives independently from server reconnect and team changes (Xian)

TTimo NOTE: for persistence, bans are stored in g_banIPs cvar MAX_CVAR_VALUE_STRING
The size of the cvar string buffer is limiting the banning to around 20 masks
this could be improved by putting some g_banIPs2 g_banIps3 etc. maybe
still, you should rely on PB for banning instead

==============================================================================
*/

typedef struct ipGUID_s
{
	char compare[33];
} ipGUID_t;

#define MAX_IPFILTERS   1024

#define MAX_XPSTORAGEITEMS  MAX_CLIENTS
typedef struct ipXPStorageList_s
{
	ipXPStorage_t ipFilters[MAX_XPSTORAGEITEMS];
} ipXPStorageList_t;

typedef struct ipFilterList_s
{
	ipFilter_t ipFilters[MAX_IPFILTERS];
	int numIPFilters;
	char cvarIPList[32];
} ipFilterList_t;

static ipFilterList_t ipFilters;
#ifdef USEXPSTORAGE
static ipXPStorageList_t ipXPStorage;
#endif

/*
=================
StringToFilter
=================
*/
qboolean StringToFilter(const char *s, ipFilter_t *f)
{
	char num[128];
	int  i, j;
	byte b[4];
	byte m[4];

	for (i = 0 ; i < 4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}

	for (i = 0 ; i < 4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			if (*s == '*') // 'match any'
			{
				// b[i] and m[i] to 0
				s++;
				if (!*s)
				{
					break;
				}
				s++;
				continue;
			}
			G_Printf("Bad filter address: %s\n", s);
			return qfalse;
		}

		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i]   = atoi(num);
		m[i]   = 255;

		if (!*s)
		{
			break;
		}
		s++;
	}

	f->mask    = *(unsigned *)m;
	f->compare = *(unsigned *)b;

	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans(ipFilterList_t *ipFilterList)
{
	byte b[4];
	byte m[4];
	int  i, j;
	char iplist_final[MAX_CVAR_VALUE_STRING];
	char ip[64];

	*iplist_final = 0;
	for (i = 0 ; i < ipFilterList->numIPFilters ; i++)
	{
		if (ipFilterList->ipFilters[i].compare == 0xffffffff)
		{
			continue;
		}

		*(unsigned *)b = ipFilterList->ipFilters[i].compare;
		*(unsigned *)m = ipFilterList->ipFilters[i].mask;
		*ip            = 0;
		for (j = 0; j < 4 ; j++)
		{
			if (m[j] != 255)
			{
				Q_strcat(ip, sizeof(ip), "*");
			}
			else
			{
				Q_strcat(ip, sizeof(ip), va("%i", b[j]));
			}
			Q_strcat(ip, sizeof(ip), (j < 3) ? "." : " ");
		}
		if (strlen(iplist_final) + strlen(ip) < MAX_CVAR_VALUE_STRING)
		{
			Q_strcat(iplist_final, sizeof(iplist_final), ip);
		}
		else
		{
			Com_Printf("%s overflowed at MAX_CVAR_VALUE_STRING\n", ipFilterList->cvarIPList);
			break;
		}
	}

	trap_Cvar_Set(ipFilterList->cvarIPList, iplist_final);
}

/*
=================
G_FindIpData
=================
*/

ipXPStorage_t *G_FindIpData(ipXPStorageList_t *ipXPStorageList, char *from)
{
	int      i;
	unsigned in;
	byte     m[4];
	char     *p;

	i = 0;
	p = from;
	while (*p && i < 4)
	{
		m[i] = 0;
		while (*p >= '0' && *p <= '9')
		{
			m[i] = m[i] * 10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
		{
			break;
		}
		i++, p++;
	}

	in = *(unsigned *)m;

	for (i = 0; i < MAX_IPFILTERS; i++)
	{
		if (!ipXPStorageList->ipFilters[i].timeadded || level.time - ipXPStorageList->ipFilters[i].timeadded > (5 * 60000))
		{
			continue;
		}

		if ((in & ipXPStorageList->ipFilters[i].filter.mask) == ipXPStorageList->ipFilters[i].filter.compare)
		{
			return &ipXPStorageList->ipFilters[i];
		}
	}

	return NULL;
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket(ipFilterList_t *ipFilterList, const char *from)
{
	int      i;
	unsigned in;
	byte     m[4];
	const char     *p;

	i = 0;
	p = from;
	while (*p && i < 4)
	{
		m[i] = 0;
		while (*p >= '0' && *p <= '9')
		{
			m[i] = m[i] * 10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
		{
			break;
		}
		i++, p++;
	}

	in = *(unsigned *)m;

	for (i = 0; i < ipFilterList->numIPFilters; i++)
		if ((in & ipFilterList->ipFilters[i].mask) == ipFilterList->ipFilters[i].compare)
		{
			return g_filterBan.integer != 0 ? qtrue : qfalse;
		}

	return g_filterBan.integer == 0 ? qtrue : qfalse;
}

qboolean G_FilterIPBanPacket(const char *from)
{
	return(G_FilterPacket(&ipFilters, from));
}


/*
=================
AddIP
=================
*/
void AddIP(ipFilterList_t *ipFilterList, const char *str)
{
	int i;

	for (i = 0; i < ipFilterList->numIPFilters; i++)
	{
		if (ipFilterList->ipFilters[i].compare == 0xffffffff)
		{
			break;      // free spot
		}
	}

	if (i == ipFilterList->numIPFilters)
	{
		if (ipFilterList->numIPFilters == MAX_IPFILTERS)
		{
			G_Printf("IP filter list is full\n");
			return;
		}
		ipFilterList->numIPFilters++;
	}

	if (!StringToFilter(str, &ipFilterList->ipFilters[i]))
	{
		ipFilterList->ipFilters[i].compare = 0xffffffffu;
	}

	UpdateIPBans(ipFilterList);
}

void AddIPBan(const char *str)
{
	AddIP(&ipFilters, str);
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void)
{
	char *s, *t;
	char str[MAX_CVAR_VALUE_STRING];

	ipFilters.numIPFilters = 0;
	Q_strncpyz(ipFilters.cvarIPList, "g_banIPs", sizeof(ipFilters.cvarIPList));

	Q_strncpyz(str, g_banIPs.string, sizeof(str));

	for (t = s = g_banIPs.string; *t; /* */)
	{
		s = strchr(s, ' ');
		if (!s)
		{
			break;
		}
		while (*s == ' ')
			*s++ = 0;
		if (*t)
		{
			AddIP(&ipFilters, t);
		}
		t = s;
	}
}

/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f(void)
{
	char str[MAX_TOKEN_CHARS];

	if (trap_Argc() < 2)
	{
		G_Printf("Usage:  addip <ip-mask>\n");
		return;
	}

	trap_Argv(1, str, sizeof(str));

	AddIP(&ipFilters, str);

}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f(void)
{
	ipFilter_t f;
	int        i;
	char       str[MAX_TOKEN_CHARS];

	if (trap_Argc() < 2)
	{
		G_Printf("Usage:  removeip <ip-mask>\n");
		return;
	}

	trap_Argv(1, str, sizeof(str));

	if (!StringToFilter(str, &f))
	{
		return;
	}

	for (i = 0 ; i < ipFilters.numIPFilters ; i++)
	{
		if (ipFilters.ipFilters[i].mask == f.mask   &&
		    ipFilters.ipFilters[i].compare == f.compare)
		{
			ipFilters.ipFilters[i].compare = 0xffffffffu;
			G_Printf("Removed.\n");

			UpdateIPBans(&ipFilters);
			return;
		}
	}

	G_Printf("Didn't find %s.\n", str);
}

/*
===================
Svcmd_EntityList_f
===================
*/
void    Svcmd_EntityList_f(void)
{
	int       e;
	gentity_t *check;

	check = g_entities + 1;
	for (e = 1; e < level.num_entities ; e++, check++)
	{
		if (!check->inuse)
		{
			continue;
		}
		G_Printf("%3i:", e);
		switch (check->s.eType)
		{
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_CONCUSSIVE_TRIGGER:
			G_Printf("ET_CONCUSSIVE_TRIGGR");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
		case ET_EXPLOSIVE:
			G_Printf("ET_EXPLOSIVE        ");
			break;
		case ET_EF_SPOTLIGHT:
			G_Printf("ET_EF_SPOTLIGHT     ");
			break;
		case ET_ALARMBOX:
			G_Printf("ET_ALARMBOX          ");
			break;
		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if (check->classname)
		{
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

int refClientNumFromString(char *s)
{
	gclient_t *cl;
	int       idnum;
	char      s2[MAX_STRING_CHARS];
	char      n2[MAX_STRING_CHARS];
	qboolean  fIsNumber      = qtrue;
	int       partialMatchs  = 0;
	int       partialMatchId = -1;

	// See if its a number or string
	for (idnum = 0; idnum < static_cast<int>(strlen(s)) && s[idnum] != 0; idnum++)
	{
		if (s[idnum] < '0' || s[idnum] > '9')
		{
			fIsNumber = qfalse;
			break;
		}
	}

	// check for a name match
	SanitizeString(s, s2, qtrue);
	for (idnum = 0, cl = level.clients; idnum < level.maxclients; idnum++, cl++)
	{
		if (cl->pers.connected != CON_CONNECTED)
		{
			continue;
		}

		SanitizeString(cl->pers.netname, n2, qtrue);

		if (!strcmp(n2, s2))
		{
			return(idnum);
		}

		if (strstr(n2, s2) != NULL)
		{
			partialMatchs++;
			partialMatchId = idnum;
		}
	}

	// numeric values are just slot numbers
	if (fIsNumber)
	{
		idnum = atoi(s);
		if (idnum < 0 || idnum >= level.maxclients)
		{
			G_Printf("Bad client slot: ^3%i\n", idnum);
			return -1;
		}

		cl = &level.clients[idnum];
		if (cl->pers.connected != CON_CONNECTED)
		{
			G_Printf("Client ^3%i^7 is not active\n", idnum);
			return -1;
		}

		return(idnum);
	}

	if (partialMatchs > 1)
	{
		G_Printf("Several partial matches\n");
		return -1;
	}
	else if (partialMatchs == 1)
	{
		return partialMatchId;
	}

	G_Printf("User ^3%s^7 is not on the server\n", s);
	return(-1);
}

// fretn, note: if a player is called '3' and there are only 2 players
// on the server (clientnum 0 and 1)
// this function will say 'client 3 is not connected'
// solution: first check for usernames, if none is found, check for slotnumbers
gclient_t *ClientForString(const char *s)
{
	gclient_t *cl;
	int       i;
	int       idnum;

	// check for a name match
	for (i = 0 ; i < level.maxclients ; i++)
	{
		cl = &level.clients[i];
		if (cl->pers.connected == CON_DISCONNECTED)
		{
			continue;
		}
		if (!Q_stricmp(cl->pers.netname, s))
		{
			return cl;
		}
	}

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9')
	{
		idnum = atoi(s);
		if (idnum < 0 || idnum >= level.maxclients)
		{
			Com_Printf("Bad client slot: %i\n", idnum);
			return NULL;
		}

		cl = &level.clients[idnum];
		if (cl->pers.connected == CON_DISCONNECTED)
		{
			G_Printf("Client %i is not connected\n", idnum);
			return NULL;
		}
		return cl;
	}

	G_Printf("User %s is not on the server\n", s);

	return NULL;
}

// fretn

static qboolean G_Is_SV_Running(void)
{

	char cvar[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer("sv_running", cvar, sizeof(cvar));
	return (qboolean)atoi(cvar);
}

/*
==================
G_GetPlayerByNum
==================
*/
gclient_t *G_GetPlayerByNum(int clientNum)
{
	gclient_t *cl;


	// make sure server is running
	if (!G_Is_SV_Running())
	{
		return NULL;
	}

	if (trap_Argc() < 2)
	{
		G_Printf("No player specified.\n");
		return NULL;
	}

	if (clientNum < 0 || clientNum >= level.maxclients)
	{
		Com_Printf("Bad client slot: %i\n", clientNum);
		return NULL;
	}

	cl = &level.clients[clientNum];
	if (cl->pers.connected == CON_DISCONNECTED)
	{
		G_Printf("Client %i is not connected\n", clientNum);
		return NULL;
	}

	if (cl)
	{
		return cl;
	}


	G_Printf("User %d is not on the server\n", clientNum);

	return NULL;
}

/*
==================
G_GetPlayerByName
==================
*/
gclient_t *G_GetPlayerByName(char *name)
{

	int       i;
	gclient_t *cl;
	char      cleanName[64];

	// make sure server is running
	if (!G_Is_SV_Running())
	{
		return NULL;
	}

	if (trap_Argc() < 2)
	{
		G_Printf("No player specified.\n");
		return NULL;
	}

	for (i = 0; i < level.numConnectedClients; i++)
	{

		cl = &level.clients[i];

		if (!Q_stricmp(cl->pers.netname, name))
		{
			return cl;
		}

		Q_strncpyz(cleanName, cl->pers.netname, sizeof(cleanName));
		Q_CleanStr(cleanName);
		if (!Q_stricmp(cleanName, name))
		{
			return cl;
		}
	}

	G_Printf("Player %s is not on the server\n", name);

	return NULL;
}

// -fretn

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/

void Svcmd_ForceTeam_f(void)
{
	gclient_t *cl;
	char      str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv(1, str, sizeof(str));
	cl = ClientForString(str);
	if (!cl)
	{
		return;
	}

	// set the team
	trap_Argv(2, str, sizeof(str));
	SetTeam(&g_entities[cl - level.clients], str, qfalse, static_cast<weapon_t>(cl->sess.playerWeapon), static_cast<weapon_t>(cl->sess.playerWeapon2), qtrue);
}

/*
==================
Svcmd_ResetMatch_f

OSP - multiuse now for both map restarts and total match resets
==================
*/
void Svcmd_ResetMatch_f(qboolean fDoReset, qboolean fDoRestart)
{
	int i;

	for (i = 0; i < level.numConnectedClients; i++)
	{
		g_entities[level.sortedClients[i]].client->pers.ready = qfalse;
	}

	if (fDoRestart)
	{
		trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", ((g_gamestate.integer != GS_PLAYING) ? GS_RESET : GS_WARMUP)));
	}
}

// ydnar: modified from maddoc sp func
extern qboolean ReviveEntity(gentity_t *ent, gentity_t *traceEnt);
extern int FindClientByName(char *name);

void Svcmd_RevivePlayer(char *name)
{
	int       clientNum;
	gentity_t *player;


	if (!g_cheats.integer)
	{
		trap_SendServerCommand(-1, va("print \"Cheats are not enabled on this server.\n\""));
		return;
	}

	clientNum = FindClientByName(name);
	if (clientNum < 0)
	{
		return;
	}
	player = &g_entities[clientNum];

	ReviveEntity(player, player);

}

/*
==================
Svcmd_KickNum_f

Kick a user off of the server
==================
*/

static void Svcmd_KickNum_f(void)
{
	int  timeout = 300;
	int  clientNum;
	char name[MAX_TOKEN_CHARS], sTimeout[MAX_TOKEN_CHARS];

	if (!G_Is_SV_Running())
	{
		G_Printf("Server is not running.\n");
		return;
	}

	if (trap_Argc() < 2 || trap_Argc() > 3)
	{
		G_Printf("Usage: kick <client number> [timeout]\n");
		return;
	}

	if (trap_Argc() == 3)
	{
		trap_Argv(2, sTimeout, sizeof(sTimeout));
		timeout = atoi(sTimeout);
	}
	else
	{
		timeout = 300;
	}

	trap_Argv(1, name, sizeof(name));

	if ((clientNum = ClientNumberFromString(NULL, name)) == -1)
	{
		G_Printf("Client not found!\n");
		return;
	}

	trap_DropClient(clientNum, "player kicked", timeout);
}

void G_AddIpMute(char *ip)
{
	int i = 0;
	for (i = 0; i < MAX_IP_MUTES; i++)
	{
		if (!level.ipMutes[i].inuse)
		{
			break;
		}
	}

	if (i == MAX_IP_MUTES)
	{
		return; // No free mutes, start unmuting people!
	}

	Q_strncpyz(level.ipMutes[i].ip, ip, sizeof(level.ipMutes[i].ip));
	level.ipMutes[i].inuse = qtrue;
}

qboolean G_isIPMuted(const char *originalIp)
{
	char ip[32] = "\0";
	int i = 0; 
	Q_strncpyz(ip, originalIp, sizeof(ip));

	// Must remove port from ip
	for (i = 0; i < static_cast<int>(strlen(ip)); i++)
	{
		if (ip[i] == ':')
		{
			ip[i] = 0;
		}
	}

	for (i = 0; i < MAX_IP_MUTES; i++)
	{
		if (!level.ipMutes[i].inuse)
		{
			continue;
		}
		else
		{
			if (!Q_stricmp(level.ipMutes[i].ip, ip))
			{
				return qtrue;
			}
		}
	}
	return qfalse;
}

void G_RemoveIPMute(char *ip)
{
	int i = 0;
	for (i = 0; i < MAX_IP_MUTES; i++)
	{
		if (!level.ipMutes[i].inuse)
		{
			continue;
		}
		else
		{
			if (!Q_strncmp(level.ipMutes[i].ip, ip, strlen(level.ipMutes[i].ip)))
			{
				level.ipMutes[i].inuse = qfalse;
				return;
			}
		}
	}
}

void G_ClearIPMutes()
{
	int i;
	for (i = 0; i < MAX_IP_MUTES; i++)
		level.ipMutes[i].inuse = qfalse;
}

void AC_ListCheaters()
{
	int i = 0;
	for (; i < level.numConnectedClients; i++)
	{
		int       clientNum = level.sortedClients[i];
		gentity_t *target   = g_entities + clientNum;

		if (target->client->cheatDetected)
		{
			G_Printf("Cheater: %s\n", target->client->pers.netname);
		}

	}
}

void G_ListIPMutes()
{
	int i;
	G_Printf("IP Mutes:\n");
	for (i = 0; i < MAX_IP_MUTES; i++)
	{
		if (level.ipMutes[i].inuse)
		{
			G_Printf("%s\n", level.ipMutes[i].ip);
		}
	}
}

// -fretn

char *ConcatArgs(int start);

/*
=================
ConsoleCommand

=================
*/
qboolean    ConsoleCommand(void)
{
	char cmd[MAX_TOKEN_CHARS];

	trap_Argv(0, cmd, sizeof(cmd));

	if (Q_stricmp(cmd, "entitylist") == 0)
	{
		Svcmd_EntityList_f();
		return qtrue;
	}

	if (Q_stricmp(cmd, "forceteam") == 0)
	{
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if (Q_stricmp(cmd, "game_memory") == 0)
	{
		Svcmd_GameMem_f();
		return qtrue;
	}

	/*if (Q_stricmp (cmd, "addbot") == 0) {
	    Svcmd_AddBot_f();
	    return qtrue;
	}
	if (Q_stricmp (cmd, "removebot") == 0) {
	    Svcmd_AddBot_f();
	    return qtrue;
	}*/
	if (Q_stricmp(cmd, "addip") == 0)
	{
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp(cmd, "removeip") == 0)
	{
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp(cmd, "listip") == 0)
	{
		trap_SendConsoleCommand(EXEC_INSERT, "g_banIPs\n");
		return qtrue;
	}


// START - Mad Doc - TDF
	if (Q_stricmp(cmd, "revive") == 0)
	{
		trap_Argv(1, cmd, sizeof(cmd));
		Svcmd_RevivePlayer(cmd);
		return qtrue;
	}
// END - Mad Doc - TDF

	// fretn - moved from engine

	if (!Q_stricmp(cmd, "clientkick"))
	{
		Svcmd_KickNum_f();
		return qtrue;
	}

	if (!Q_stricmp(cmd, "m"))
	{
		Cmd_PrivateMessage_f(NULL);
		return qtrue;
	}

	if (!Q_stricmp(cmd, "ipmutes"))
	{
		G_ListIPMutes();
		return qtrue;
	}

	if (!Q_stricmp(cmd, "listcheaters"))
	{
		AC_ListCheaters();
		return qtrue;
	}

	if (!Q_stricmp(cmd, "mod_information"))
	{
		G_LogPrintf("%s %s %s\n", GAME_NAME, GAME_VERSION_DATED, __TIME__);
		return qtrue;
	}

	if (OnConsoleCommand())
	{
		return qtrue;
	}

	// -fretn

	if (g_dedicated.integer)
	{
		if (!Q_stricmp(cmd, "say"))
		{
			trap_SendServerCommand(-1, va("cpm \"server: %s\n\"", ConcatArgs(1)));
			return qtrue;
		}
		else if (!Q_stricmp(cmd, "qsay"))
		{
			trap_SendServerCommand(-1, va("chat \"%s\"", ConcatArgs(1)));
			return qtrue;
		}
		else if (!Q_stricmp(cmd, "enc_qsay"))
		{
			trap_SendServerCommand(-1, va("enc_chat \"%s\"", ConcatArgs(1)));
			return qtrue;
		}
		else if (!Q_stricmp(cmd, "cp"))
		{
			trap_SendServerCommand(-1, va("cp \"%s\n\"", ConcatArgs(1)));
			return qtrue;
		}

		// OSP - console also gets ref command

		// everything else will also be printed as a say command
//		trap_SendServerCommand( -1, va("cpm \"server: %s\n\"", ConcatArgs(0) ) );

		// prints to the console instead now
	}

	return qfalse;
}
