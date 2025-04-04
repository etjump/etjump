// cg_syscalls.c -- this file is only included when building a dll
// cg_syscalls.asm is included instead when building a qvm
#include "cg_local.h"
#include "../game/etj_syscall_ext_shared.h"

static intptr_t(QDECL *syscall)(intptr_t arg,
                                ...) = (intptr_t(QDECL *)(intptr_t, ...))-1;

#if defined(__MACOS__)
  #ifndef __GNUC__
    #pragma export on
  #endif
#endif

extern "C" FN_PUBLIC void dllEntry(intptr_t(QDECL *syscallptr)(intptr_t arg,
                                                               ...)) {
  syscall = syscallptr;
}

#if defined(__MACOS__)
  #ifndef __GNUC__
    #pragma export off
  #endif
#endif

template <typename T, typename... Types>
static intptr_t ExpandSyscall(T syscallArg, Types... args) {
  // we have to do C-style casting here to support all types
  // of arguments passed onto syscalls
  return syscall((intptr_t)syscallArg, (intptr_t)args...);
}

inline int PASSFLOAT(const float &f) noexcept {
  floatint_t fi;
  fi.f = f;
  return fi.i;
}

void trap_PumpEventLoop(void) {
  if (!cgs.initing) {
    return;
  }
  SystemCall(CG_PUMPEVENTLOOP);
}

void trap_Print(const char *fmt) { SystemCall(CG_PRINT, fmt); }

// coverity[+kill]
[[noreturn]] void trap_Error(const char *fmt) {
  SystemCall(CG_ERROR, fmt);
  UNREACHABLE
}

int trap_Milliseconds(void) { return SystemCall(CG_MILLISECONDS); }

void trap_Cvar_Register(vmCvar_t *vmCvar, const char *varName,
                        const char *defaultValue, int flags) {
  SystemCall(CG_CVAR_REGISTER, vmCvar, varName, defaultValue, flags);
}

void trap_Cvar_Update(vmCvar_t *vmCvar) { SystemCall(CG_CVAR_UPDATE, vmCvar); }

void trap_Cvar_Set(const char *var_name, const char *value) {
  SystemCall(CG_CVAR_SET, var_name, value);
}

void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer,
                                    int bufsize) {
  SystemCall(CG_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize);
}

void trap_Cvar_LatchedVariableStringBuffer(const char *var_name, char *buffer,
                                           int bufsize) {
  SystemCall(CG_CVAR_LATCHEDVARIABLESTRINGBUFFER, var_name, buffer, bufsize);
}

int trap_Argc(void) { return SystemCall(CG_ARGC); }

void trap_Argv(int n, char *buffer, int bufferLength) {
  SystemCall(CG_ARGV, n, buffer, bufferLength);
}

void trap_Args(char *buffer, int bufferLength) {
  SystemCall(CG_ARGS, buffer, bufferLength);
}

int trap_FS_FOpenFile(const char *qpath, fileHandle_t *f, fsMode_t mode) {
  return SystemCall(CG_FS_FOPENFILE, qpath, f, mode);
}

void trap_FS_Read(void *buffer, int len, fileHandle_t f) {
  SystemCall(CG_FS_READ, buffer, len, f);
}

void trap_FS_Write(const void *buffer, int len, fileHandle_t f) {
  SystemCall(CG_FS_WRITE, buffer, len, f);
}

void trap_FS_FCloseFile(fileHandle_t f) { SystemCall(CG_FS_FCLOSEFILE, f); }

int trap_FS_GetFileList(const char *path, const char *extension, char *listbuf,
                        int bufsize) {
  return SystemCall(CG_FS_GETFILELIST, path, extension, listbuf, bufsize);
}

int trap_FS_Delete(const char *filename) {
  return SystemCall(CG_FS_DELETEFILE, filename);
}

void trap_SendConsoleCommand(const char *text) {
  SystemCall(CG_SENDCONSOLECOMMAND, text);
}

void trap_AddCommand(const char *cmdName) {
  SystemCall(CG_ADDCOMMAND, cmdName);
}

void trap_SendClientCommand(const char *s) {
  SystemCall(CG_SENDCLIENTCOMMAND, s);
}

void trap_UpdateScreen(void) { SystemCall(CG_UPDATESCREEN); }

/*void	trap_CM_LoadMap( const char *mapname ) {
    CG_DrawInformation();
    SystemCall( CG_CM_LOADMAP, mapname );
}*/

int trap_CM_NumInlineModels(void) { return SystemCall(CG_CM_NUMINLINEMODELS); }

clipHandle_t trap_CM_InlineModel(int index) {
  return SystemCall(CG_CM_INLINEMODEL, index);
}

clipHandle_t trap_CM_TempBoxModel(const vec3_t mins, const vec3_t maxs) {
  return SystemCall(CG_CM_TEMPBOXMODEL, mins, maxs);
}

clipHandle_t trap_CM_TempCapsuleModel(const vec3_t mins, const vec3_t maxs) {
  return SystemCall(CG_CM_TEMPCAPSULEMODEL, mins, maxs);
}

int trap_CM_PointContents(const vec3_t p, clipHandle_t model) {
  return SystemCall(CG_CM_POINTCONTENTS, p, model);
}

int trap_CM_TransformedPointContents(const vec3_t p, clipHandle_t model,
                                     const vec3_t origin, const vec3_t angles) {
  return SystemCall(CG_CM_TRANSFORMEDPOINTCONTENTS, p, model, origin, angles);
}

void trap_CM_BoxTrace(trace_t *results, const vec3_t start, const vec3_t end,
                      const vec3_t mins, const vec3_t maxs, clipHandle_t model,
                      int brushmask) {
  SystemCall(CG_CM_BOXTRACE, results, start, end, mins, maxs, model, brushmask);
}

void trap_CM_TransformedBoxTrace(trace_t *results, const vec3_t start,
                                 const vec3_t end, const vec3_t mins,
                                 const vec3_t maxs, clipHandle_t model,
                                 int brushmask, const vec3_t origin,
                                 const vec3_t angles) {
  SystemCall(CG_CM_TRANSFORMEDBOXTRACE, results, start, end, mins, maxs, model,
             brushmask, origin, angles);
}

void trap_CM_CapsuleTrace(trace_t *results, const vec3_t start,
                          const vec3_t end, const vec3_t mins,
                          const vec3_t maxs, clipHandle_t model,
                          int brushmask) {
  SystemCall(CG_CM_CAPSULETRACE, results, start, end, mins, maxs, model,
             brushmask);
}

void trap_CM_TransformedCapsuleTrace(trace_t *results, const vec3_t start,
                                     const vec3_t end, const vec3_t mins,
                                     const vec3_t maxs, clipHandle_t model,
                                     int brushmask, const vec3_t origin,
                                     const vec3_t angles) {
  SystemCall(CG_CM_TRANSFORMEDCAPSULETRACE, results, start, end, mins, maxs,
             model, brushmask, origin, angles);
}

int trap_CM_MarkFragments(int numPoints, const vec3_t *points,
                          const vec3_t projection, int maxPoints,
                          vec3_t pointBuffer, int maxFragments,
                          markFragment_t *fragmentBuffer) {
  return SystemCall(CG_CM_MARKFRAGMENTS, numPoints, points, projection,
                    maxPoints, pointBuffer, maxFragments, fragmentBuffer);
}

// ydnar
void trap_R_ProjectDecal(qhandle_t hShader, int numPoints, vec3_t *points,
                         vec4_t projection, vec4_t color, int lifeTime,
                         int fadeTime) {
  SystemCall(CG_R_PROJECTDECAL, hShader, numPoints, points, projection, color,
             lifeTime, fadeTime);
}

void trap_R_ClearDecals(void) { SystemCall(CG_R_CLEARDECALS); }

void trap_S_StartSound(vec3_t origin, int entityNum, int entchannel,
                       sfxHandle_t sfx) {
  // explicitly respatialize all local sounds in freecam
  if (cgs.demoCam.renderingFreeCam && entityNum == cg.snap->ps.clientNum) {
    SystemCall(CG_S_STARTSOUND, cg.snap->ps.origin, -1, entchannel, sfx,
               127 /* Gordon: default volume always for the moment*/);
  } else {

    SystemCall(CG_S_STARTSOUND, origin, entityNum, entchannel, sfx,
               127 /* Gordon: default volume always for the moment*/);
  }
}

void trap_S_StartSoundVControl(vec3_t origin, int entityNum, int entchannel,
                               sfxHandle_t sfx, int volume) {
  // explicitly respatialize all local sounds in freecam
  if (cgs.demoCam.renderingFreeCam && entityNum == cg.snap->ps.clientNum) {
    SystemCall(CG_S_STARTSOUND, cg.snap->ps.origin, -1, entchannel, sfx,
               127 /* Gordon: default volume always for the moment*/);
  } else {
    SystemCall(CG_S_STARTSOUND, origin, entityNum, entchannel, sfx, volume);
  }
}

//----(SA)	added
void trap_S_StartSoundEx(vec3_t origin, int entityNum, int entchannel,
                         sfxHandle_t sfx, int flags) {
  // explicitly respatialize all local sounds in freecam
  if (cgs.demoCam.renderingFreeCam && entityNum == cg.snap->ps.clientNum) {
    SystemCall(CG_S_STARTSOUND, cg.snap->ps.origin, -1, entchannel, sfx,
               127 /* Gordon: default volume always for the moment*/);
  } else {
    SystemCall(CG_S_STARTSOUNDEX, origin, entityNum, entchannel, sfx, flags,
               127 /* Gordon: default volume always for the moment*/);
  }
}
//----(SA)	end

void trap_S_StartSoundExVControl(vec3_t origin, int entityNum, int entchannel,
                                 sfxHandle_t sfx, int flags, int volume) {
  SystemCall(CG_S_STARTSOUNDEX, origin, entityNum, entchannel, sfx, flags,
             volume);
}

void trap_S_StartLocalSound(sfxHandle_t sfx, int channelNum) {
  SystemCall(CG_S_STARTLOCALSOUND, sfx, channelNum,
             127 /* Gordon: default volume always for the moment*/);
}

void trap_S_StartLocalSoundVControl(sfxHandle_t sfx, int channelNum,
                                    int volume) {
  SystemCall(CG_S_STARTLOCALSOUND, sfx, channelNum, volume);
}

void trap_S_ClearLoopingSounds(void) { SystemCall(CG_S_CLEARLOOPINGSOUNDS); }

void trap_S_ClearSounds(qboolean killmusic) {
  SystemCall(CG_S_CLEARSOUNDS, killmusic);
}

void trap_S_AddLoopingSound(const vec3_t origin, const vec3_t velocity,
                            sfxHandle_t sfx, int volume, int soundTime) {
  SystemCall(CG_S_ADDLOOPINGSOUND, origin, velocity, 1250, sfx, volume,
             soundTime); // volume was previously removed from
                         // CG_S_ADDLOOPINGSOUND.  I added 'range'
}

void trap_S_AddRealLoopingSound(const vec3_t origin, const vec3_t velocity,
                                sfxHandle_t sfx, int range, int volume,
                                int soundTime) {
  SystemCall(CG_S_ADDREALLOOPINGSOUND, origin, velocity, range, sfx, volume,
             soundTime);
}

void trap_S_StopStreamingSound(int entityNum) {
  SystemCall(CG_S_STOPSTREAMINGSOUND, entityNum);
}

void trap_S_UpdateEntityPosition(int entityNum, const vec3_t origin) {
  SystemCall(CG_S_UPDATEENTITYPOSITION, entityNum, origin);
}

// Ridah, talking animations
int trap_S_GetVoiceAmplitude(int entityNum) {
  return SystemCall(CG_S_GETVOICEAMPLITUDE, entityNum);
}
// done.

void trap_S_Respatialize(int entityNum, const vec3_t origin, vec3_t axis[3],
                         int inwater) {
  SystemCall(CG_S_RESPATIALIZE, entityNum, origin, axis, inwater);
}

/*sfxHandle_t	trap_S_RegisterSound( const char *sample, qboolean compressed )
{ CG_DrawInformation(); return SystemCall( CG_S_REGISTERSOUND, sample,
compressed );
}*/

int trap_S_GetSoundLength(sfxHandle_t sfx) {
  return SystemCall(CG_S_GETSOUNDLENGTH, sfx);
}

// ydnar: for timing looped sounds
int trap_S_GetCurrentSoundTime(void) {
  return SystemCall(CG_S_GETCURRENTSOUNDTIME);
}

void trap_S_StartBackgroundTrack(const char *intro, const char *loop,
                                 int fadeupTime) {
  SystemCall(CG_S_STARTBACKGROUNDTRACK, intro, loop, fadeupTime);
}

void trap_S_FadeBackgroundTrack(
    float targetvol, int time,
    int num) // yes, i know.  fadebackground coming in, fadestreaming going out.
             // will have to see where functionality leads...
{
  SystemCall(CG_S_FADESTREAMINGSOUND, PASSFLOAT(targetvol), time,
             num); // 'num' is '0' if it's music, '1' if it's "all
                   // streaming sounds"
}

void trap_S_FadeAllSound(float targetvol, int time, qboolean stopsounds) {
  SystemCall(CG_S_FADEALLSOUNDS, PASSFLOAT(targetvol), time, stopsounds);
}

int trap_S_StartStreamingSound(const char *intro, const char *loop, int entnum,
                               int channel, int attenuation) {
  return SystemCall(CG_S_STARTSTREAMINGSOUND, intro, loop, entnum, channel,
                    attenuation);
}

/*void	trap_R_LoadWorldMap( const char *mapname ) {
    CG_DrawInformation();
    SystemCall( CG_R_LOADWORLDMAP, mapname );
}

qhandle_t trap_R_RegisterModel( const char *name ) {
    CG_DrawInformation();
    return SystemCall( CG_R_REGISTERMODEL, name );
}*/

//----(SA)	added
qboolean trap_R_GetSkinModel(qhandle_t skinid, const char *type, char *name) {
  return SystemCall(CG_R_GETSKINMODEL, skinid, type, name) ? qtrue : qfalse;
}

qhandle_t trap_R_GetShaderFromModel(qhandle_t modelid, int surfnum,
                                    int withlightmap) {
  return SystemCall(CG_R_GETMODELSHADER, modelid, surfnum, withlightmap);
}
//----(SA)	end

/*qhandle_t trap_R_RegisterSkin( const char *name ) {
    CG_DrawInformation();
    return SystemCall( CG_R_REGISTERSKIN, name );
}

qhandle_t trap_R_RegisterShader( const char *name ) {
    CG_DrawInformation();
    return SystemCall( CG_R_REGISTERSHADER, name );
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
    CG_DrawInformation();
    return SystemCall( CG_R_REGISTERSHADERNOMIP, name );
}

void trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font)
{ SystemCall(CG_R_REGISTERFONT, fontName, pointSize, font );
}*/

void trap_R_ClearScene(void) { SystemCall(CG_R_CLEARSCENE); }

void trap_R_AddRefEntityToScene(const refEntity_t *re) {
  SystemCall(CG_R_ADDREFENTITYTOSCENE, re);
}

void trap_R_AddPolyToScene(qhandle_t hShader, int numVerts,
                           const polyVert_t *verts) {
  SystemCall(CG_R_ADDPOLYTOSCENE, hShader, numVerts, verts);
}

void trap_R_AddPolyBufferToScene(polyBuffer_t *pPolyBuffer) {
  SystemCall(CG_R_ADDPOLYBUFFERTOSCENE, pPolyBuffer);
}

// Ridah
void trap_R_AddPolysToScene(qhandle_t hShader, int numVerts,
                            const polyVert_t *verts, int numPolys) {
  SystemCall(CG_R_ADDPOLYSTOSCENE, hShader, numVerts, verts, numPolys);
}
// done.

// ydnar: new dlight system
//%	void	trap_R_AddLightToScene( const vec3_t org, float intensity, float
// r, float g, float b, int overdraw ) { %		SystemCall(
// CG_R_ADDLIGHTTOSCENE, org, PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g),
// PASSFLOAT(b), overdraw ); %	}
void trap_R_AddLightToScene(const vec3_t org, float radius, float intensity,
                            float r, float g, float b, qhandle_t hShader,
                            int flags) {
  SystemCall(CG_R_ADDLIGHTTOSCENE, org, PASSFLOAT(radius), PASSFLOAT(intensity),
             PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b), hShader, flags);
}

//----(SA)
void trap_R_AddCoronaToScene(const vec3_t org, float r, float g, float b,
                             float scale, int id, qboolean visible) {
  SystemCall(CG_R_ADDCORONATOSCENE, org, PASSFLOAT(r), PASSFLOAT(g),
             PASSFLOAT(b), PASSFLOAT(scale), id, visible);
}
//----(SA)

//----(SA)
void trap_R_SetFog(int fogvar, int var1, int var2, float r, float g, float b,
                   float density) {
  SystemCall(CG_R_SETFOG, fogvar, var1, var2, PASSFLOAT(r), PASSFLOAT(g),
             PASSFLOAT(b), PASSFLOAT(density));
}
//----(SA)

void trap_R_SetGlobalFog(qboolean restore, int duration, float r, float g,
                         float b, float depthForOpaque) {
  SystemCall(CG_R_SETGLOBALFOG, restore, duration, PASSFLOAT(r), PASSFLOAT(g),
             PASSFLOAT(b), PASSFLOAT(depthForOpaque));
}

void trap_R_RenderScene(const refdef_t *fd) {
  SystemCall(CG_R_RENDERSCENE, fd);
}

// Mad Doctor I, 11/4/2002.
void trap_R_SaveViewParms() { SystemCall(CG_R_SAVEVIEWPARMS); }

// Mad Doctor I, 11/4/2002.
void trap_R_RestoreViewParms() { SystemCall(CG_R_RESTOREVIEWPARMS); }

void trap_R_SetColor(const float *rgba) { SystemCall(CG_R_SETCOLOR, rgba); }

void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1,
                           float t1, float s2, float t2, qhandle_t hShader) {
  SystemCall(CG_R_DRAWSTRETCHPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w),
             PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2),
             PASSFLOAT(t2), hShader);
}

void trap_R_DrawRotatedPic(float x, float y, float w, float h, float s1,
                           float t1, float s2, float t2, qhandle_t hShader,
                           float angle) {
  SystemCall(CG_R_DRAWROTATEDPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w),
             PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2),
             PASSFLOAT(t2), hShader, PASSFLOAT(angle));
}

void trap_R_DrawStretchPicGradient(float x, float y, float w, float h, float s1,
                                   float t1, float s2, float t2,
                                   qhandle_t hShader,
                                   const float *gradientColor,
                                   int gradientType) {
  SystemCall(CG_R_DRAWSTRETCHPIC_GRADIENT, PASSFLOAT(x), PASSFLOAT(y),
             PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1),
             PASSFLOAT(s2), PASSFLOAT(t2), hShader, gradientColor,
             gradientType);
}

void trap_R_Add2dPolys(polyVert_t *verts, int numverts, qhandle_t hShader) {
  SystemCall(CG_R_DRAW2DPOLYS, verts, numverts, hShader);
}

void trap_R_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs) {
  SystemCall(CG_R_MODELBOUNDS, model, mins, maxs);
}

int trap_R_LerpTag(orientation_t *tag, const refEntity_t *refent,
                   const char *tagName, int startIndex) {
  return SystemCall(CG_R_LERPTAG, tag, refent, tagName, startIndex);
}

void trap_R_RemapShader(const char *oldShader, const char *newShader,
                        const char *timeOffset) {
  SystemCall(CG_R_REMAP_SHADER, oldShader, newShader, timeOffset);
}

void trap_GetGlconfig(glconfig_t *glconfig) {
  SystemCall(CG_GETGLCONFIG, glconfig);
}

void trap_GetGameState(gameState_t *gamestate) {
  SystemCall(CG_GETGAMESTATE, gamestate);
}

#ifdef _DEBUG
  // #define FAKELAG
  #ifdef FAKELAG
    #define MAX_SNAPSHOT_BACKUP 256
    #define MAX_SNAPSHOT_MASK (MAX_SNAPSHOT_BACKUP - 1)

static snapshot_t snaps[MAX_SNAPSHOT_BACKUP];
static int curSnapshotNumber;
int snapshotDelayTime;
static qboolean skiponeget;
  #endif // FAKELAG
#endif   // _DEBUG

void trap_GetCurrentSnapshotNumber(int *snapshotNumber, int *serverTime) {
  SystemCall(CG_GETCURRENTSNAPSHOTNUMBER, snapshotNumber, serverTime);

#ifdef FAKELAG
  {
    char s[MAX_STRING_CHARS];
    int fakeLag;

    trap_Cvar_VariableStringBuffer("g_fakelag", s, sizeof(s));
    fakeLag = Q_atoi(s);
    if (fakeLag < 0) {
      fakeLag = 0;
    }

    if (fakeLag) {
      if (curSnapshotNumber < cg.latestSnapshotNum) {
        *snapshotNumber = cg.latestSnapshotNum + 1;
        curSnapshotNumber =
            cg.latestSnapshotNum + 2; // skip one ahead and we're good
                                      // to go on the next frame
        skiponeget = qtrue;
      } else {
        *snapshotNumber = curSnapshotNumber;
      }
    }
  }
#endif // FAKELAG
}

qboolean trap_GetSnapshot(int snapshotNumber, snapshot_t *snapshot) {
#ifndef FAKELAG
  return SystemCall(CG_GETSNAPSHOT, snapshotNumber, snapshot) ? qtrue : qfalse;
#else
  {
    char s[MAX_STRING_CHARS];
    int fakeLag;

    if (skiponeget) {
      SystemCall(CG_GETSNAPSHOT, snapshotNumber, snapshot);
    }

    trap_Cvar_VariableStringBuffer("g_fakelag", s, sizeof(s));
    fakeLag = Q_atoi(s);
    if (fakeLag < 0) {
      fakeLag = 0;
    }

    if (fakeLag) {
      int i;
      int realsnaptime, thissnaptime;

      // store our newest usercmd
      curSnapshotNumber++;
      memcpy(&snaps[curSnapshotNumber & MAX_SNAPSHOT_MASK], snapshot,
             sizeof(snapshot_t));

      // find a usercmd that is fakeLag msec behind
      i = curSnapshotNumber & MAX_SNAPSHOT_MASK;
      realsnaptime = snaps[i].serverTime;
      i--;
      do {
        thissnaptime = snaps[i & MAX_SNAPSHOT_MASK].serverTime;

        if (realsnaptime - thissnaptime > fakeLag) {
          // found the right one
          snapshotDelayTime = realsnaptime - thissnaptime;
          snapshot = &snaps[i & MAX_SNAPSHOT_MASK];
          //*snapshotNumber = i &
          // MAX_SNAPSHOT_MASK;
          return qtrue;
        }

        i--;
      } while ((i & MAX_SNAPSHOT_MASK) !=
               (curSnapshotNumber & MAX_SNAPSHOT_MASK));

      // didn't find a proper one, just use the oldest
      // one we have
      snapshotDelayTime = realsnaptime - thissnaptime;
      snapshot = &snaps[(curSnapshotNumber - 1) & MAX_SNAPSHOT_MASK];
      //*snapshotNumber = (curSnapshotNumber - 1) &
      // MAX_SNAPSHOT_MASK;
      return qtrue;
    } else {
      return SystemCall(CG_GETSNAPSHOT, snapshotNumber, snapshot);
    }
  }
#endif // FAKELAG
}

qboolean trap_GetServerCommand(int serverCommandNumber) {
  return SystemCall(CG_GETSERVERCOMMAND, serverCommandNumber) ? qtrue : qfalse;
}

int trap_GetCurrentCmdNumber(void) {
  return SystemCall(CG_GETCURRENTCMDNUMBER);
}

qboolean trap_GetUserCmd(int cmdNumber, usercmd_t *ucmd) {
  return SystemCall(CG_GETUSERCMD, cmdNumber, ucmd) ? qtrue : qfalse;
}

void trap_SetUserCmdValue(int stateValue, int flags, float sensitivityScale,
                          int mpIdentClient) {
  SystemCall(CG_SETUSERCMDVALUE, stateValue, flags, PASSFLOAT(sensitivityScale),
             mpIdentClient);
}

void trap_SetClientLerpOrigin(float x, float y, float z) {
  SystemCall(CG_SETCLIENTLERPORIGIN, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(z));
}

void testPrintInt(char *string, int i) {
  SystemCall(CG_TESTPRINTINT, string, i);
}

void testPrintFloat(char *string, float f) {
  SystemCall(CG_TESTPRINTFLOAT, string, PASSFLOAT(f));
}

int trap_MemoryRemaining(void) { return SystemCall(CG_MEMORY_REMAINING); }

qboolean trap_loadCamera(int camNum, const char *name) {
  return SystemCall(CG_LOADCAMERA, camNum, name) ? qtrue : qfalse;
}

void trap_startCamera(int camNum, int time) {
  SystemCall(CG_STARTCAMERA, camNum, time);
}

void trap_stopCamera(int camNum) { SystemCall(CG_STOPCAMERA, camNum); }

qboolean trap_getCameraInfo(int camNum, int time, vec3_t *origin,
                            vec3_t *angles, float *fov) {
  return SystemCall(CG_GETCAMERAINFO, camNum, time, origin, angles, fov)
             ? qtrue
             : qfalse;
}

qboolean trap_Key_IsDown(int keynum) {
  return SystemCall(CG_KEY_ISDOWN, keynum) ? qtrue : qfalse;
}

int trap_Key_GetCatcher(void) { return SystemCall(CG_KEY_GETCATCHER); }

qboolean trap_Key_GetOverstrikeMode(void) {
  return SystemCall(CG_KEY_GETOVERSTRIKEMODE) ? qtrue : qfalse;
}

void trap_Key_SetOverstrikeMode(qboolean state) {
  SystemCall(CG_KEY_SETOVERSTRIKEMODE, state);
}

// binding MUST be lower case
void trap_Key_KeysForBinding(const char *binding, int *key1, int *key2) {
  SystemCall(CG_KEY_BINDINGTOKEYS, binding, key1, key2);
}

void trap_Key_SetCatcher(int catcher) {
  SystemCall(CG_KEY_SETCATCHER, catcher);
}

int trap_Key_GetKey(const char *binding) {
  return SystemCall(CG_KEY_GETKEY, binding);
}

int trap_PC_AddGlobalDefine(const char *define) {
  return SystemCall(CG_PC_ADD_GLOBAL_DEFINE, define);
}

int trap_PC_LoadSource(const char *filename) {
  return SystemCall(CG_PC_LOAD_SOURCE, filename);
}

int trap_PC_FreeSource(int handle) {
  return SystemCall(CG_PC_FREE_SOURCE, handle);
}

int trap_PC_ReadToken(int handle, pc_token_t *pc_token) {
  return SystemCall(CG_PC_READ_TOKEN, handle, pc_token);
}

int trap_PC_SourceFileAndLine(int handle, char *filename, int *line) {
  return SystemCall(CG_PC_SOURCE_FILE_AND_LINE, handle, filename, line);
}

int trap_PC_UnReadToken(int handle) {
  return SystemCall(CG_PC_UNREAD_TOKEN, handle);
}

void trap_S_StopBackgroundTrack(void) { SystemCall(CG_S_STOPBACKGROUNDTRACK); }

int trap_RealTime(qtime_t *qtime) { return SystemCall(CG_REAL_TIME, qtime); }

void trap_SnapVector(float *v) { SystemCall(CG_SNAPVECTOR, v); }

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1
// to NULL, alteredstates to qfalse (do not alter gamestate)
int trap_CIN_PlayCinematic(const char *arg0, int xpos, int ypos, int width,
                           int height, int bits) {
  return SystemCall(CG_CIN_PLAYCINEMATIC, arg0, xpos, ypos, width, height,
                    bits);
}

// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status trap_CIN_StopCinematic(int handle) {
  return (e_status)SystemCall(CG_CIN_STOPCINEMATIC, handle);
}

// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF
// if the end of the cinematic has been reached.
e_status trap_CIN_RunCinematic(int handle) {
  return (e_status)SystemCall(CG_CIN_RUNCINEMATIC, handle);
}

// draws the current frame
void trap_CIN_DrawCinematic(int handle) {
  SystemCall(CG_CIN_DRAWCINEMATIC, handle);
}

// allows you to resize the animation dynamically
void trap_CIN_SetExtents(int handle, int x, int y, int w, int h) {
  SystemCall(CG_CIN_SETEXTENTS, handle, x, y, w, h);
}

qboolean trap_GetEntityToken(char *buffer, int bufferSize) {
  return SystemCall(CG_GET_ENTITY_TOKEN, buffer, bufferSize) ? qtrue : qfalse;
}

//----(SA)	added
// bring up a popup menu
extern void Menus_OpenByName(const char *p);

// void trap_UI_Popup( const char *arg0) {
void trap_UI_Popup(int arg0) { SystemCall(CG_INGAME_POPUP, arg0); }

void trap_UI_ClosePopup(const char *arg0) {
  SystemCall(CG_INGAME_CLOSEPOPUP, arg0);
}

void trap_Key_GetBindingBuf(int keynum, char *buf, int buflen) {
  SystemCall(CG_KEY_GETBINDINGBUF, keynum, buf, buflen);
}

void trap_Key_SetBinding(int keynum, const char *binding) {
  SystemCall(CG_KEY_SETBINDING, keynum, binding);
}

void trap_Key_KeynumToStringBuf(int keynum, char *buf, int buflen) {
  SystemCall(CG_KEY_KEYNUMTOSTRINGBUF, keynum, buf, buflen);
}

void trap_TranslateString(const char *string, char *buf) {
  SystemCall(CG_TRANSLATE_STRING, string, buf);
}
// -NERVE - SMF

// Media register functions
#ifdef _DEBUG
  #define DEBUG_REGISTERPROFILE_INIT int dbgTime = trap_Milliseconds();
  #define DEBUG_REGISTERPROFILE_EXEC(f, n)                                     \
    if (developer.integer) {                                                   \
      CG_Printf("%s : loaded %s in %i msec\n", f, n,                           \
                trap_Milliseconds() - dbgTime);                                \
    }
sfxHandle_t trap_S_RegisterSound(const char *sample, qboolean compressed) {
  sfxHandle_t snd;
  DEBUG_REGISTERPROFILE_INIT
  CG_DrawInformation(qtrue);
  snd = SystemCall(CG_S_REGISTERSOUND, sample, qfalse /* compressed */);
  if (!*sample) {
    Com_Printf("^1Warning: Null Sample filename\n");
  }
  if (snd == 0) {
    Com_Printf("^1Warning: Failed to load sound: %s\n", sample);
  }
  DEBUG_REGISTERPROFILE_EXEC("trap_S_RegisterSound", sample)
  trap_PumpEventLoop();
  return snd;
}

qhandle_t trap_R_RegisterModel(const char *name) {
  qhandle_t handle;
  DEBUG_REGISTERPROFILE_INIT
  CG_DrawInformation(qtrue);
  handle = SystemCall(CG_R_REGISTERMODEL, name);
  DEBUG_REGISTERPROFILE_EXEC("trap_R_RegisterModel", name)
  trap_PumpEventLoop();
  return handle;
}

qhandle_t trap_R_RegisterSkin(const char *name) {
  qhandle_t handle;
  DEBUG_REGISTERPROFILE_INIT
  CG_DrawInformation(qtrue);
  handle = SystemCall(CG_R_REGISTERSKIN, name);
  DEBUG_REGISTERPROFILE_EXEC("trap_R_RegisterSkin", name)
  trap_PumpEventLoop();
  return handle;
}

qhandle_t trap_R_RegisterShader(const char *name) {
  qhandle_t handle;
  DEBUG_REGISTERPROFILE_INIT
  CG_DrawInformation(qtrue);
  handle = SystemCall(CG_R_REGISTERSHADER, name);
  DEBUG_REGISTERPROFILE_EXEC("trap_R_RegisterShader", name)
  trap_PumpEventLoop();
  return handle;
}

qhandle_t trap_R_RegisterShaderNoMip(const char *name) {
  qhandle_t handle;
  DEBUG_REGISTERPROFILE_INIT
  CG_DrawInformation(qtrue);
  handle = SystemCall(CG_R_REGISTERSHADERNOMIP, name);
  trap_PumpEventLoop();
  DEBUG_REGISTERPROFILE_EXEC("trap_R_RegisterShaderNpMip", name);
  return handle;
}

void trap_R_RegisterFont(const char *fontName, int pointSize,
                         fontInfo_t *font) {
  DEBUG_REGISTERPROFILE_INIT
  CG_DrawInformation(qtrue);
  SystemCall(CG_R_REGISTERFONT, fontName, pointSize, font);
  DEBUG_REGISTERPROFILE_EXEC("trap_R_RegisterFont", fontName)
  trap_PumpEventLoop();
}

void trap_CM_LoadMap(const char *mapname) {
  DEBUG_REGISTERPROFILE_INIT
  CG_DrawInformation(qtrue);
  SystemCall(CG_CM_LOADMAP, mapname);
  DEBUG_REGISTERPROFILE_EXEC("trap_CM_LoadMap", mapname)
  trap_PumpEventLoop();
}

void trap_R_LoadWorldMap(const char *mapname) {
  DEBUG_REGISTERPROFILE_INIT
  CG_DrawInformation(qtrue);
  SystemCall(CG_R_LOADWORLDMAP, mapname);
  DEBUG_REGISTERPROFILE_EXEC("trap_R_LoadWorldMap", mapname)
  trap_PumpEventLoop();
}
#else
sfxHandle_t trap_S_RegisterSound(const char *sample, qboolean compressed) {
  CG_DrawInformation(qtrue);
  trap_PumpEventLoop();
  return SystemCall(CG_S_REGISTERSOUND, sample, qfalse /* compressed */);
}

qhandle_t trap_R_RegisterModel(const char *name) {
  CG_DrawInformation(qtrue);
  trap_PumpEventLoop();
  return SystemCall(CG_R_REGISTERMODEL, name);
}

qhandle_t trap_R_RegisterSkin(const char *name) {
  CG_DrawInformation(qtrue);
  trap_PumpEventLoop();
  return SystemCall(CG_R_REGISTERSKIN, name);
}

qhandle_t trap_R_RegisterShader(const char *name) {
  CG_DrawInformation(qtrue);
  trap_PumpEventLoop();
  return SystemCall(CG_R_REGISTERSHADER, name);
}

qhandle_t trap_R_RegisterShaderNoMip(const char *name) {
  CG_DrawInformation(qtrue);
  trap_PumpEventLoop();
  return SystemCall(CG_R_REGISTERSHADERNOMIP, name);
}

void trap_R_RegisterFont(const char *fontName, int pointSize,
                         fontInfo_t *font) {
  CG_DrawInformation(qtrue);
  trap_PumpEventLoop();
  SystemCall(CG_R_REGISTERFONT, fontName, pointSize, font);
}

void trap_CM_LoadMap(const char *mapname) {
  CG_DrawInformation(qtrue);
  trap_PumpEventLoop();
  SystemCall(CG_CM_LOADMAP, mapname);
}

void trap_R_LoadWorldMap(const char *mapname) {
  CG_DrawInformation(qtrue);
  trap_PumpEventLoop();
  SystemCall(CG_R_LOADWORLDMAP, mapname);
}
#endif // _DEBUG

qboolean trap_R_inPVS(const vec3_t p1, const vec3_t p2) {
  return SystemCall(CG_R_INPVS, p1, p2) ? qtrue : qfalse;
}

void trap_GetHunkData(int *hunkused, int *hunkexpected) {
  SystemCall(CG_GETHUNKDATA, hunkused, hunkexpected);
}

// zinx - binary message channel
void trap_SendMessage(char *buf, int buflen) {
  SystemCall(CG_SENDMESSAGE, buf, buflen);
}

messageStatus_t trap_MessageStatus(void) {
  return (messageStatus_t)SystemCall(CG_MESSAGESTATUS);
}

// bani - dynamic shaders
qboolean trap_R_LoadDynamicShader(const char *shadername,
                                  const char *shadertext) {
  return SystemCall(CG_R_LOADDYNAMICSHADER, shadername, shadertext) ? qtrue
                                                                    : qfalse;
}

// fretn - render to texture
void trap_R_RenderToTexture(int textureid, int x, int y, int w, int h) {
  SystemCall(CG_R_RENDERTOTEXTURE, textureid, x, y, w, h);
}

int trap_R_GetTextureId(const char *name) {
  return SystemCall(CG_R_GETTEXTUREID, name);
}

// bani - sync rendering
void trap_R_Finish(void) { SystemCall(CG_R_FINISH); }

// ETJump: syscall extensions
namespace ETJump {
// entry point for additional system calls for other engines (ETe, ET: Legacy)
bool SyscallExt::trap_GetValue(char *value, const int size, const char *key) {
  return SystemCall(syscallExt->dll_com_trapGetValue, value, size, key);
}

// ET: Legacy - flash client window
void SyscallExt::trap_SysFlashWindowETLegacy(const FlashWindowState state) {
  if (syscallExt->cgameExtensions[syscallExt->flashWindowETLegacy]) {
    SystemCall(syscallExt->cgameExtensions[syscallExt->flashWindowETLegacy],
               static_cast<int>(state));
  }
}

void SyscallExt::trap_CmdBackup_Ext() {
  if (syscallExt->cgameExtensions[syscallExt->cmdBackupExt]) {
    cg.cmdBackup = CMD_BACKUP_EXT;
    cg.cmdMask = CMD_MASK_EXT;
    SystemCall(syscallExt->cgameExtensions[syscallExt->cmdBackupExt]);
  } else {
    cg.cmdBackup = CMD_BACKUP;
    cg.cmdMask = CMD_MASK;
  }
}

} // namespace ETJump
