// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// gameinfo.c
//

#include "ui_local.h"

//
// arena and bot info
//

int ui_numBots;
// static char *ui_botInfos[MAX_BOTS];

static int ui_numArenas;

/*
===============
UI_LoadArenasFromFile
===============
*/
static void UI_LoadArenasFromFile(char *filename) {
  int handle;
  pc_token_t token;

  handle = trap_PC_LoadSource(filename);

  if (!handle) {
    trap_Print(va(S_COLOR_RED "file not found: %s\n", filename));
    return;
  }

  if (!trap_PC_ReadToken(handle, &token)) {
    trap_PC_FreeSource(handle);
    return;
  }

  if (*token.string != '{') {
    trap_PC_FreeSource(handle);
    return;
  }

  uiInfo.mapList[uiInfo.mapCount].cinematic = -1;
  uiInfo.mapList[uiInfo.mapCount].levelShot = -1;
  uiInfo.mapList[uiInfo.mapCount].typeBits = 0;

  while (trap_PC_ReadToken(handle, &token)) {
    if (*token.string == '}') {

      if (!uiInfo.mapList[uiInfo.mapCount].typeBits) {
        uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << ETJUMP_GAMETYPE);
      }

      uiInfo.mapCount++;
      if (uiInfo.mapCount >= MAX_MAPS) {
        break;
      }

      if (!trap_PC_ReadToken(handle, &token)) {
        // eof
        trap_PC_FreeSource(handle);
        return;
      }

      if (*token.string != '{') {
        trap_Print(va(S_COLOR_RED "unexpected token '%s' "
                                  "inside: %s\n",
                      token.string, filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "map")) {
      if (!PC_String_Parse(handle,
                           &uiInfo.mapList[uiInfo.mapCount].mapLoadName)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "longname")) {
      if (!PC_String_Parse(handle, &uiInfo.mapList[uiInfo.mapCount].mapName)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "briefing")) {
      if (!PC_String_Parse(handle, &uiInfo.mapList[uiInfo.mapCount].briefing)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "lmsbriefing")) {
      if (!PC_String_Parse(handle,
                           &uiInfo.mapList[uiInfo.mapCount].lmsbriefing)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return;
      }
      /*} else if( !Q_stricmp( token.string,
         "objectives" ) ) { if( !PC_String_Parse(
         handle,
         &uiInfo.mapList[uiInfo.mapCount].objectives ) )
         { trap_Print( va( S_COLOR_RED "unexpected end
         of file inside: %s\n", filename ) );
              trap_PC_FreeSource( handle );
              return;
          }*/
    } else if (!Q_stricmp(token.string, "timelimit")) {
      if (!PC_Int_Parse(handle, &uiInfo.mapList[uiInfo.mapCount].Timelimit)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "axisrespawntime")) {
      if (!PC_Int_Parse(handle,
                        &uiInfo.mapList[uiInfo.mapCount].AxisRespawnTime)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "alliedrespawntime")) {
      if (!PC_Int_Parse(handle,
                        &uiInfo.mapList[uiInfo.mapCount].AlliedRespawnTime)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "type")) {
      if (!trap_PC_ReadToken(handle, &token)) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return;
      } else {
        if (strstr(token.string, "wolfsp") || strstr(token.string, "wolflms") ||
            strstr(token.string, "wolfmp") || strstr(token.string, "wolfsw")) {
          uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << ETJUMP_GAMETYPE);
        }
      }
    } else if (!Q_stricmp(token.string, "mapposition_x")) {
      if (!PC_Float_Parse(handle, &uiInfo.mapList[uiInfo.mapCount].mappos[0])) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "mapposition_y")) {
      if (!PC_Float_Parse(handle, &uiInfo.mapList[uiInfo.mapCount].mappos[1])) {
        trap_Print(va(S_COLOR_RED "unexpected end of file "
                                  "inside: %s\n",
                      filename));
        trap_PC_FreeSource(handle);
        return;
      }
    }
  }

  trap_PC_FreeSource(handle);
  return;
}

/*
===============
UI_LoadArenas
===============
*/
void UI_LoadArenas() {
  int numdirs;
  char filename[128];
  char dirlist[16000];
  char *dirptr;
  int i;
  int dirlen;

  ui_numArenas = 0;
  uiInfo.mapCount = 0;

  // get all arenas from .arena files
  numdirs = trap_FS_GetFileList("scripts", ".arena", dirlist, 12000);
  dirptr = dirlist;
  for (i = 0; i < numdirs; i++, dirptr += dirlen + 1) {
    dirlen = static_cast<int>(strlen(dirptr));
    Q_strncpyz(filename, "scripts/", sizeof(filename));
    Q_strcat(filename, sizeof(filename), dirptr);
    UI_LoadArenasFromFile(filename);
  }
}

mapInfo *UI_FindMapInfoByMapname(const char *name) {
  int i;

  if (uiInfo.mapCount == 0) {
    UI_LoadArenas();
  }

  for (i = 0; i < uiInfo.mapCount; i++) {
    if (!Q_stricmp(uiInfo.mapList[i].mapLoadName, name)) {
      return &uiInfo.mapList[i];
    }
  }

  return NULL;
}
