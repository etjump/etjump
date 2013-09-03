#ifndef g_local_hpp__
#define g_local_hpp__

// Local C++ definitions for game module

extern "C" {
#include "g_local.h"
}

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include "g_database.hpp"

extern class Database         adminDB;

#endif // g_local_hpp__
