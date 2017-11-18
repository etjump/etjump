#include "g_local.h"

const char *systemMessages[SM_NUM_SYS_MSGS] =
{
	"SYS_NeedMedic",
	"SYS_NeedEngineer",
	"SYS_NeedLT",
	"SYS_NeedCovertOps",
	"SYS_MenDown",
	"SYS_ObjCaptured",
	"SYS_ObjLost",
	"SYS_ObjDestroyed",
	"SYS_ConstructComplete",
	"SYS_ConstructFailed",
	"SYS_Destroyed",
};

int G_GetSysMessageNumber(const char *sysMsg)
{
	int i;

	for (i = 0; i < SM_NUM_SYS_MSGS; i++)
	{
		if (!Q_stricmp(systemMessages[i], sysMsg))
		{
			return i;
		}
	}

	return -1;
}

void G_SendSystemMessage(sysMsg_t message, int team)
{
	gentity_t *other;
	int *time;
	int j;

	time = team == TEAM_AXIS ? &level.lastSystemMsgTime[0] : &level.lastSystemMsgTime[1];

	if (*time && (level.time - *time) < 15000)
	{
		return;
	}

	*time = level.time;

	for (j = 0; j < level.maxclients; j++)
	{
		other = &g_entities[j];

		if (!other->client || !other->inuse)
		{
			continue;
		}

		if (other->client->sess.sessionTeam != team)
		{
			continue;
		}

		trap_SendServerCommand(other - g_entities, va("vschat 0 %d 3 %s 0 0 0", other - g_entities, systemMessages[message]));
	}
}