#include "etj_string_utilities.h"
extern "C" {
#include "../sha-1/sha1.h"
}


std::string ETJump::hash(const std::string& input)
{
	SHA1Context sha;

	SHA1Reset(&sha);
	SHA1Input(&sha, reinterpret_cast<const unsigned char *>(input.c_str()), input.length());

	if (!SHA1Result(&sha))
	{
		return "";
	}

	char buffer[60] = "";
	snprintf(buffer, sizeof(buffer), "%08X%08X%08X%08X%08X", sha.Message_Digest[0],
		sha.Message_Digest[1],
		sha.Message_Digest[2],
		sha.Message_Digest[3],
		sha.Message_Digest[4]);
	return buffer;
}
