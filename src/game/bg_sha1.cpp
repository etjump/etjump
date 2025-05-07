extern "C" {
#include "sha1.h"
}
#ifdef CGAMEDLL
  #include "../cgame/cg_local.h"
#else
  #include "../game/g_local.h"
#endif

const char *G_SHA1(const char *str) {
  SHA1Context sha;

  SHA1Reset(&sha);
  SHA1Input(&sha, reinterpret_cast<const unsigned char *>(str), strlen(str));

  if (!SHA1Result(&sha)) {
    return "";
  }

  return va("%08X%08X%08X%08X%08X", sha.Message_Digest[0],
            sha.Message_Digest[1], sha.Message_Digest[2], sha.Message_Digest[3],
            sha.Message_Digest[4]);
}

const char *G_SHA1(const std::string &str) { return G_SHA1(str.c_str()); }
