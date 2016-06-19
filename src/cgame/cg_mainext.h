#ifndef CG_MAINEXT_HH
#define CG_MAINEXT_HH

#include "cg_local.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

/**
* Initializes the CPP side of client
*/
void InitGame();

/**
* Extended CG_ServerCommand function. Checks whether server
* sent command matches to any defined here. If no match is found
* returns false
* @return qboolean Whether a match was found or not
*/
qboolean CG_ServerCommandExt(const char *cmd);


#endif