#ifdef WIN32
#include <windows.h>
#include "cg_anticheat.h"
#include <stdio.h>

void InitAntiCheat(AntiCheat_t ac)
{
    
}
#else
void InitAntiCheat()
{
    // Do nothing
}
#endif