extern "C" {
#include "sha1.h"
}
#ifdef CGAMEDLL
#include "../cgame/cg_local.h"
#else
#include "../game/g_local.h"
#endif

const char *G_SHA1(const char *string)
{
	SHA1Context sha;

	SHA1Reset(&sha);
	SHA1Input(&sha, (const unsigned char *)string, strlen(string));

	if (!SHA1Result(&sha))
	{
		return "";
	}
	else
	{
		return va("%08X%08X%08X%08X%08X",
			sha.Message_Digest[0],
			sha.Message_Digest[1],
			sha.Message_Digest[2],
			sha.Message_Digest[3],
			sha.Message_Digest[4]);
	}
}
