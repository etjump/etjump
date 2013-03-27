#ifndef g_local_hpp__
#define g_local_hpp__

// Local C++ definitions for game module

extern "C" {
#include "g_local.h"
}

#ifdef max
#undef max
#endif

#include "g_users.h"
#include "g_save.h"
#include "g_levels.h"

extern class UserDatabase     users;
extern class SaveSystem       positions;
extern class LevelDatabase    levels;


#endif // g_local_hpp__
