#include "cg_local.h"

qboolean CG_FindArenaInfo(char *filename, char *mapname, arenaInfo_t *info) {
  int handle;
  pc_token_t token;
  const char *dummy;
  qboolean found = qfalse;

  handle = trap_PC_LoadSource(filename);

  if (!handle) {
    trap_Print(va(S_COLOR_RED "file not found: %s\n", filename));
    return qfalse;
  }

  if (!trap_PC_ReadToken(handle, &token)) {
    trap_PC_FreeSource(handle);
    return qfalse;
  }

  if (*token.string != '{') {
    trap_PC_FreeSource(handle);
    return qfalse;
  }

  while (trap_PC_ReadToken(handle, &token)) {
    if (*token.string == '}') {
      if (found) {
        //				info->image
        //=
        // trap_R_RegisterShaderNoMip(va("levelshots/%s",
        // mapname));
        trap_PC_FreeSource(handle);
        return qtrue;
      }
      found = qfalse;

      if (!trap_PC_ReadToken(handle, &token)) {
        // eof
        trap_PC_FreeSource(handle);
        return qfalse;
      }

      if (*token.string != '{') {
        trap_Print(va(S_COLOR_RED "unexpected token '%s' "
                                  "inside: %s\n",
                      token.string, filename));
        trap_PC_FreeSource(handle);
        return qfalse;
      }
    } else if (!Q_stricmp(token.string, "objectives") ||
               !Q_stricmp(token.string, "description") ||
               !Q_stricmp(token.string, "type")) {
      if (!PC_String_Parse(handle, &dummy)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return qfalse;
      }
    } else if (!Q_stricmp(token.string, "longname")) {
      if (!PC_String_Parse(handle, &dummy)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return qfalse;
      } else {
        // char* p = info->longname;

        Q_strncpyz(info->longname, dummy, 128);
        // Gordon: removing cuz, er, no-one
        // knows why it's here!...
        /*				while(*p)
           { *p = toupper(*p); p++;
                        }*/
      }
    } else if (!Q_stricmp(token.string, "map")) {
      if (!PC_String_Parse(handle, &dummy)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return qfalse;
      } else {
        if (!Q_stricmp(dummy, mapname)) {
          found = qtrue;
        }
      }
    } else if (!Q_stricmp(token.string, "Timelimit") ||
               !Q_stricmp(token.string, "AxisRespawnTime") ||
               !Q_stricmp(token.string, "AlliedRespawnTime")) {
      if (!PC_Int_Parse(handle, (int *)&dummy)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return qfalse;
      }
    } else if (!Q_stricmp(token.string, "lmsbriefing")) {
      if (!PC_String_Parse(handle, &dummy)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return qfalse;
      } else {
        Q_strncpyz(info->lmsdescription, dummy, sizeof(info->lmsdescription));
      }
    } else if (!Q_stricmp(token.string, "briefing")) {
      if (!PC_String_Parse(handle, &dummy)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return qfalse;
      } else {
        Q_strncpyz(info->description, dummy, sizeof(info->description));
      }
    } else if (!Q_stricmp(token.string, "alliedwintext")) {
      if (!PC_String_Parse(handle, &dummy)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return qfalse;
      } else {
        Q_strncpyz(info->alliedwintext, dummy, sizeof(info->description));
      }
    } else if (!Q_stricmp(token.string, "axiswintext")) {
      if (!PC_String_Parse(handle, &dummy)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return qfalse;
      } else {
        Q_strncpyz(info->axiswintext, dummy, sizeof(info->description));
      }
    } else if (!Q_stricmp(token.string, "mapposition_x")) {
      vec_t x;

      if (!trap_PC_ReadToken(handle, &token)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return qfalse;
      }

      x = token.floatvalue;

      info->mappos[0] = x;
    } else if (!Q_stricmp(token.string, "mapposition_y")) {
      vec_t y;

      if (!trap_PC_ReadToken(handle, &token)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return qfalse;
      }

      y = token.floatvalue;

      info->mappos[1] = y;
    }
  }

  trap_PC_FreeSource(handle);
  return qfalse;
}

void CG_LocateArena(void) {
  char filename[MAX_QPATH];

  Com_sprintf(filename, sizeof(filename), "scripts/%s.arena", cgs.rawmapname);

  if (/*!CG_FindArenaInfo( "scripts/wolfmp.arena", cgs.rawmapname,
     &cgs.arenaData ) && !CG_FindArenaInfo( "scripts/wolfxp.arena",
     cgs.rawmapname, &cgs.arenaData ) &&*/
      !CG_FindArenaInfo(filename, cgs.rawmapname, &cgs.arenaData)) {
    return;
  }

  cgs.arenaInfoLoaded = qtrue;
}
