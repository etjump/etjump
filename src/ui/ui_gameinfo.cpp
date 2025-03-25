// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// gameinfo.c
//

#include <memory>
#include <algorithm>

#include "../game/etj_filesystem.h"
#include "../game/etj_string_utilities.h"

#include "ui_local.h"

//
// arena and bot info
//

int ui_numBots;
// static char *ui_botInfos[MAX_BOTS];

/*
===============
UI_LoadArenasFromFile
===============
*/
static void UI_LoadArenasFromFile(const int handle, char *filename) {
  pc_token_t token;

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
        trap_Print(
            va(S_COLOR_RED "unexpected end of file inside: %s\n", filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "longname")) {
      if (!PC_String_Parse(handle, &uiInfo.mapList[uiInfo.mapCount].mapName)) {
        trap_Print(
            va(S_COLOR_RED "unexpected end of file inside: %s\n", filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "briefing")) {
      if (!PC_String_Parse(handle, &uiInfo.mapList[uiInfo.mapCount].briefing)) {
        trap_Print(
            va(S_COLOR_RED "unexpected end of file inside: %s\n", filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "lmsbriefing")) {
      if (!PC_String_Parse(handle,
                           &uiInfo.mapList[uiInfo.mapCount].lmsbriefing)) {
        trap_Print(
            va(S_COLOR_RED "unexpected end of file inside: %s\n", filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "timelimit")) {
      if (!PC_Int_Parse(handle, &uiInfo.mapList[uiInfo.mapCount].Timelimit)) {
        trap_Print(
            va(S_COLOR_RED "unexpected end of file inside: %s\n", filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "axisrespawntime")) {
      if (!PC_Int_Parse(handle,
                        &uiInfo.mapList[uiInfo.mapCount].AxisRespawnTime)) {
        trap_Print(
            va(S_COLOR_RED "unexpected end of file inside: %s\n", filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "alliedrespawntime")) {
      if (!PC_Int_Parse(handle,
                        &uiInfo.mapList[uiInfo.mapCount].AlliedRespawnTime)) {
        trap_Print(
            va(S_COLOR_RED "unexpected end of file inside: %s\n", filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "type")) {
      if (!trap_PC_ReadToken(handle, &token)) {
        trap_Print(
            va(S_COLOR_RED "unexpected end of file inside: %s\n", filename));
        trap_PC_FreeSource(handle);
        return;
      }

      if (strstr(token.string, "wolfsp") || strstr(token.string, "wolflms") ||
          strstr(token.string, "wolfmp") || strstr(token.string, "wolfsw")) {
        uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << ETJUMP_GAMETYPE);
      }
    } else if (!Q_stricmp(token.string, "mapposition_x")) {
      if (!PC_Float_Parse(handle, &uiInfo.mapList[uiInfo.mapCount].mappos[0])) {
        trap_Print(
            va(S_COLOR_RED "unexpected end of file inside: %s\n", filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "mapposition_y")) {
      if (!PC_Float_Parse(handle, &uiInfo.mapList[uiInfo.mapCount].mappos[1])) {
        trap_Print(
            va(S_COLOR_RED "unexpected end of file inside: %s\n", filename));
        trap_PC_FreeSource(handle);
        return;
      }
    } else if (!Q_stricmp(token.string, "author")) {
      if (!PC_String_Parse(handle, &uiInfo.mapList[uiInfo.mapCount].author)) {
        trap_Print(
            va(S_COLOR_RED "unexpected end of file inside: %s\n", filename));
        trap_PC_FreeSource(handle);
        return;
      }
    }
  }

  trap_PC_FreeSource(handle);
}

/*
===============
UI_LoadArenas
===============
*/
void UI_LoadArenas() {
  char filename[128];
  int handle;

  uiInfo.mapCount = 0;
  uiClientState_t cstate;
  trap_GetClientState(&cstate);

  // we're in the menus, parse local files
  if (cstate.connState == CA_DISCONNECTED) {
    const std::vector<std::string> files =
        ETJump::FileSystem::getFileList("scripts", ".arena", true);

    for (const auto &file : files) {
      Q_strncpyz(filename, "scripts/", sizeof(filename));
      Q_strcat(filename, sizeof(filename), file.c_str());

      handle = trap_PC_LoadSource(filename);
      UI_LoadArenasFromFile(handle, filename);
    }
  } else {
    for (const auto &map : uiInfo.serverMaplist) {
      const std::string arenaFile = "scripts/" + map + ".arena";
      Q_strncpyz(filename, arenaFile.c_str(), sizeof(filename));

      handle = trap_PC_LoadSource(filename);
      if (!handle) {
        Com_LocalPrintf(va(S_COLOR_RED "file not found: %s\n", filename));

        // FIXME: remove up this typeBits field entirely, it's not necessary
        //  as we only have one valid gametype
        if (!uiInfo.mapList[uiInfo.mapCount].typeBits) {
          uiInfo.mapList[uiInfo.mapCount].typeBits = (1 << ETJUMP_GAMETYPE);
        }

        uiInfo.mapList[uiInfo.mapCount].cinematic = -1;
        uiInfo.mapList[uiInfo.mapCount].levelShot = -1;

        uiInfo.mapList[uiInfo.mapCount].mapLoadName = String_Alloc(map.c_str());
        uiInfo.mapList[uiInfo.mapCount].mapName = String_Alloc(map.c_str());

        uiInfo.mapCount++;
      } else {
        UI_LoadArenasFromFile(handle, filename);
      }
    }
  }

  // cap here rather than in the parser, so we get proper map counts
  if (uiInfo.mapCount >= MAX_MAPS) {
    Com_Printf(S_COLOR_YELLOW "WARNING: reached maximum maps for UI display "
                              "(%i > %i), not all maps are displayed.\n",
               uiInfo.mapCount, MAX_MAPS - 1);
    uiInfo.mapCount = MAX_MAPS - 1;
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
