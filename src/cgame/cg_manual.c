#include "cg_local.h"

void CG_Manual_f(void)
{
    int i = 0;
    int argc = trap_Argc();
    const char *cmd = NULL;

    if(argc != 2)
    {
        CG_Printf("Manual\n\n");
        CG_Printf("Usage:\n");
        CG_Printf("/man [command]\n\n");
        CG_Printf("Description:\n");
        CG_Printf("Explains how to use admin commands.\n");
        return;
    }

    cmd = CG_Argv(1);

    for(; i < sizeof(commandManuals)/sizeof(commandManuals[0]); i++)
    {
        if(!Q_stricmp(cmd, commandManuals[i].cmd))
        {
            CG_Printf("%s\n\nUsage:\n%s\n\nDescription:\n%s\n",
                cmd, commandManuals[i].usage, commandManuals[i].description);
            return;
        }
    }
    CG_Printf("Couldn't find command: %s\n", cmd);
}