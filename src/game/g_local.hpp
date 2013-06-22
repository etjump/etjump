#ifndef g_local_hpp__
#define g_local_hpp__

// Local C++ definitions for game module

extern "C" {
#include "g_local.h"
}

#ifdef max
#undef max
#endif

#include "g_save.h"
#include "g_database.hpp"

extern class SaveSystem       positions;
extern class Database		  adminDB;
extern class SessionDB		  sessionDB;

#endif // g_local_hpp__

