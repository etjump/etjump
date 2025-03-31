/*
 * name:		cg_spawn.c
 *
 * desc:		Client sided only map entities
 */

#include <algorithm>

#include "cg_local.h"

qboolean CG_SpawnString(const char *key, const char *defaultString,
                        char **out) {
  int i;

  if (!cg.spawning) {
    CG_Error("CG_SpawnString() called while not spawning");
  }

  for (i = 0; i < cg.numSpawnVars; i++) {
    if (!strcmp(key, cg.spawnVars[i][0])) {
      *out = cg.spawnVars[i][1];
      return qtrue;
    }
  }

  // cast is fairly reasonable here, as nobody spawns a string just to overwrite
  // the default arg
  *out = const_cast<char *>(defaultString);
  return qfalse;
}

qboolean CG_SpawnFloat(const char *key, const char *defaultString, float *out) {
  char *s;
  qboolean present;

  present = CG_SpawnString(key, defaultString, &s);
  *out = Q_atof(s);
  return present;
}

qboolean CG_SpawnInt(const char *key, const char *defaultString, int *out) {
  char *s;
  qboolean present;

  present = CG_SpawnString(key, defaultString, &s);
  *out = Q_atoi(s);
  return present;
}

qboolean CG_SpawnVector(const char *key, const char *defaultString,
                        float *out) {
  char *s;
  qboolean present;

  present = CG_SpawnString(key, defaultString, &s);
  sscanf(s, "%f %f %f", &out[0], &out[1], &out[2]);
  return present;
}

qboolean CG_SpawnVector2D(const char *key, const char *defaultString,
                          float *out) {
  char *s;
  qboolean present;

  present = CG_SpawnString(key, defaultString, &s);
  sscanf(s, "%f %f", &out[0], &out[1]);
  return present;
}

void SP_path_corner_2(void) {
  char *targetname;
  vec3_t origin;

  CG_SpawnString("targetname", "", &targetname);
  CG_SpawnVector("origin", "0 0 0", origin);

  if (!*targetname) {
    CG_Error("path_corner_2 with no targetname at %s\n", vtos(origin));
    return;
  }

  if (numPathCorners >= MAX_PATH_CORNERS) {
    CG_Error("Maximum path_corners hit\n");
    return;
  }

  BG_AddPathCorner(targetname, origin);
}

void SP_info_train_spline_main(void) {
  char *targetname;
  char *target;
  char *control;
  vec3_t origin;
  int i;
  char *end;
  splinePath_t *spline;

  if (!CG_SpawnVector("origin", "0 0 0", origin)) {
    CG_Error("info_train_spline_main with no origin\n");
  }

  if (!CG_SpawnString("targetname", "", &targetname)) {
    CG_Error("info_train_spline_main with no targetname at %s\n", vtos(origin));
  }

  CG_SpawnString("target", "", &target);

  spline = BG_AddSplinePath(targetname, target, origin);

  if (CG_SpawnString("end", "", &end)) {
    spline->isEnd = qtrue;
  } else if (CG_SpawnString("start", "", &end)) {
    spline->isStart = qtrue;
  }

  for (i = 1;; i++) {
    if (!CG_SpawnString(i == 1 ? va("control") : va("control%i", i), "",
                        &control)) {
      break;
    }

    BG_AddSplineControl(spline, control);
  }
}

void SP_misc_gamemodel(void) {
  char *model;
  vec_t angle;
  vec3_t angles;

  vec_t scale;
  vec3_t vScale;

  vec3_t org;

  cg_gamemodel_t *gamemodel;

  int i;

  if (CG_SpawnString("targetname", "", &model) ||
      CG_SpawnString("scriptname", "", &model) ||
      CG_SpawnString("spawnflags", "", &model)) {
    // Gordon: this model may not be static, so let the server
    // handle it
    return;
  }

  if (cg.numMiscGameModels >= MAX_STATIC_GAMEMODELS) {
    CG_Error("^1MAX_STATIC_GAMEMODELS(%i) hit", MAX_STATIC_GAMEMODELS);
  }

  CG_SpawnString("model", "", &model);

  CG_SpawnVector("origin", "0 0 0", org);

  if (!CG_SpawnVector("angles", "0 0 0", angles)) {
    if (CG_SpawnFloat("angle", "0", &angle)) {
      angles[YAW] = angle;
    }
  }

  if (!CG_SpawnVector("modelscale_vec", "1 1 1", vScale)) {
    if (CG_SpawnFloat("modelscale", "1", &scale)) {
      VectorSet(vScale, scale, scale, scale);
    }
  }

  gamemodel = &cgs.miscGameModels[cg.numMiscGameModels++];
  gamemodel->model = trap_R_RegisterModel(model);
  AnglesToAxis(angles, gamemodel->axes);
  for (i = 0; i < 3; i++) {
    VectorScale(gamemodel->axes[i], vScale[i], gamemodel->axes[i]);
  }
  VectorCopy(org, gamemodel->org);

  if (gamemodel->model) {
    vec3_t mins, maxs;

    trap_R_ModelBounds(gamemodel->model, mins, maxs);

    for (i = 0; i < 3; i++) {
      mins[i] *= vScale[i];
      maxs[i] *= vScale[i];
    }

    gamemodel->radius = RadiusFromBounds(mins, maxs);
  } else {
    gamemodel->radius = 0;
    CG_Printf(S_COLOR_YELLOW "WARNING: failed to register misc_gamemodel %s\n",
              model);
  }
}

void SP_corona() {
  char *tmp;

  // server-side coronas
  if (CG_SpawnString("targetname", "", &tmp) ||
      CG_SpawnString("spawnflags", "", &tmp)) {
    return;
  }

  if (cg.numCoronas >= MAX_STATIC_CORONAS) {
    CG_Error("SP_Corona: MAX_STATIC_CORONAS (%i) exceeded", MAX_STATIC_CORONAS);
  }

  centity_t *corona = &cgs.coronas[cg.numCoronas];

  // this does not need to be the actual entity number,
  // it just needs to be unique per corona for portal camera drawing
  // realistically this might break if static and non-static coronas are
  // in the same portal scene but that is a minor inconvenience
  // as opposed to the benefit of freeing these from the server
  corona->currentState.number = cg.numCoronas;
  cg.numCoronas++;

  corona->currentState.eType = ET_CORONA;

  vec3_t origin{};
  CG_SpawnVector("origin", "", origin);
  VectorCopy(origin, corona->lerpOrigin);

  vec3_t color = {1.0f, 1.0f, 1.0f}; // default to white

  // both 'color' and '_color' are valid keys
  if (!CG_SpawnVector("color", "", color)) {
    CG_SpawnVector("_color", "", color);
  }

  VectorScale(color, 255, color);

  corona->currentState.dl_intensity =
      ((static_cast<int>(color[0])) | (static_cast<int>(color[1]) << 8) |
       (static_cast<int>(color[2]) << 16));

  float scale;
  CG_SpawnFloat("scale", "1", &scale);
  corona->currentState.density = static_cast<int>(scale * 255);
}

void SP_trigger_objective_info(void) {
  char *temp;

  CG_SpawnString("infoAllied", "^1No Text Supplied", &temp);
  Q_strncpyz(cg.oidTriggerInfoAllies[cg.numOIDtriggers2], temp, 256);

  CG_SpawnString("infoAxis", "^1No Text Supplied", &temp);
  Q_strncpyz(cg.oidTriggerInfoAxis[cg.numOIDtriggers2], temp, 256);

  cg.numOIDtriggers2++;
}

void SP_dlight() {
  char *s{};

  // server-side dlight
  // TODO: see if we could make sound work?
  //  pretty sure it requires a proper entitynum but need to investigate
  if (CG_SpawnString("targetname", "", &s) ||
      CG_SpawnString("scriptname", "", &s) ||
      CG_SpawnString("spawnflags", "", &s)) {
    return;
  }

  if (cg.numDlights >= MAX_STATIC_DLIGHTS) {
    CG_Error("%s: MAX_STATIC_DLIGHTS (%i) exceeded\n", __func__,
             MAX_STATIC_DLIGHTS);
  }

  centity_t *dlight = &cgs.dlights[cg.numDlights];
  cg.numDlights++;

  CG_SpawnVector("origin", "0 0 0", dlight->currentState.origin);
  CG_SpawnVector("angles", "0 0 0", dlight->currentState.angles);

  VectorCopy(dlight->currentState.origin, dlight->lerpOrigin);
  VectorCopy(dlight->currentState.angles, dlight->lerpAngles);

  if (CG_SpawnString("sound", "", &s)) {
    // normally this would be an array index set by the server,
    // which corresponds to cgs.gameSounds index as determined by CS_SOUNDS,
    // but because we don't store the sound for local dlights anywhere,
    // just set this to the file handle itself
    dlight->dl_sound = trap_S_RegisterSound(s, qfalse);
  }

  int style = 0;
  CG_SpawnInt("style", "0", &style);

  // 'stylestring' overrides pre-defined style strings
  if (CG_SpawnString("stylestring", "", &s)) {
    std::string styleString = s;

    // spawnvars are 2048 chars, but dlights are capped to 64
    if (styleString.length() >= MAX_DLIGHT_STYLESTRING) {
      styleString = styleString.substr(0, MAX_DLIGHT_STYLESTRING - 1);
    }

    Q_strncpyz(dlight->dl_stylestring, styleString.c_str(),
               sizeof(dlight->dl_stylestring));
  } else if (style) {
    style = std::clamp(style, 1, NUM_PREDEF_DLIGHT_STRINGS);
    Q_strncpyz(dlight->dl_stylestring, predef_lightstyles[style - 1],
               sizeof(dlight->dl_stylestring));
  } else {
    // default to a strobe to call attention to this not being set
    Q_strncpyz(dlight->dl_stylestring, "mmmaaa",
               sizeof(dlight->dl_stylestring));
  }

  const auto styleStringLen = static_cast<int>(strlen(dlight->dl_stylestring));

  if (CG_SpawnInt("atten", "0", &dlight->dl_atten)) {
    for (int i = 0; i < styleStringLen; i++) {
      // FIXME: this will wrap if 'atten' is too big or small and causes
      //  the clamping to behave unexpectedly, but that's VET behavior
      //  https://github.com/etjump/etjump/issues/1667
      dlight->dl_stylestring[i] += dlight->dl_atten;
      dlight->dl_stylestring[i] =
          std::clamp(dlight->dl_stylestring[i], 'a', 'z');
    }
  }

  CG_SpawnInt("offset", "0", &dlight->dl_frame);
  dlight->dl_frame = dlight->dl_frame % styleStringLen;
  dlight->dl_oldframe = dlight->dl_frame - 1;

  // wrap negative offset
  if (dlight->dl_oldframe < 0) {
    dlight->dl_oldframe = styleStringLen;
  }

  vec3_t color;
  CG_SpawnVector("color", "", color);

  // if the color is black or isn't set, default to white
  if (color[0] <= 0 && color[1] <= 0 && color[2] <= 0) {
    VectorSet(color, 1.0f, 1.0f, 1.0f);
  }

  VectorScale(color, 255, color);

  auto intensity =
      static_cast<float>(dlight->dl_stylestring[dlight->dl_frame] - 'a');
  intensity *= 1000.0f / 24.0f;
  intensity = std::clamp(intensity / 4, 0.0f, 255.0f);

  dlight->currentState.constantLight =
      static_cast<int>(color[0]) | static_cast<int>(color[1]) << 8 |
      static_cast<int>(color[2]) << 16 | static_cast<int>(intensity) << 24;

  dlight->dl_backlerp = 0;
  dlight->dl_time = cg.time;

  dlight->dl_clientOnly = true;
}

typedef struct {
  const char *name;
  void (*spawn)(void);
} spawn_t;

spawn_t spawns[] = {
    {0, 0},
    {"path_corner_2", SP_path_corner_2},
    {"info_train_spline_main", SP_info_train_spline_main},
    {"info_train_spline_control", SP_path_corner_2},

    {"trigger_objective_info", SP_trigger_objective_info},
    {"misc_gamemodel", SP_misc_gamemodel},
    {"corona", SP_corona},
    {"dlight", SP_dlight},
};

inline constexpr int NUMSPAWNS = sizeof(spawns) / sizeof(spawn_t);

/*
===================
CG_ParseEntityFromSpawnVar

Spawn an entity and fill in all of the level fields from
cg.spawnVars[], then call the class specfic spawn function
===================
*/
void CG_ParseEntityFromSpawnVars(void) {
  int i;
  char *classname;

  // check for "notteam" / "notfree" flags
  CG_SpawnInt("notteam", "0", &i);
  if (i) {
    return;
  }

  if (CG_SpawnString("classname", "", &classname)) {
    for (i = 0; i < NUMSPAWNS; i++) {
      if (!Q_stricmp(spawns[i].name, classname)) {
        spawns[i].spawn();
        break;
      }
    }
  }
}

/*
====================
CG_AddSpawnVarToken
====================
*/
char *CG_AddSpawnVarToken(const char *string) {
  int l;
  char *dest;

  l = static_cast<int>(strlen(string));
  if (cg.numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS) {
    CG_Error("CG_AddSpawnVarToken: MAX_SPAWN_VARS_CHARS");
  }

  dest = cg.spawnVarChars + cg.numSpawnVarChars;
  memcpy(dest, string, l + 1);

  cg.numSpawnVarChars += l + 1;

  return dest;
}

/*
====================
CG_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into cg.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean CG_ParseSpawnVars(void) {
  char keyname[MAX_TOKEN_CHARS];
  char com_token[MAX_TOKEN_CHARS];

  cg.numSpawnVars = 0;
  cg.numSpawnVarChars = 0;

  // parse the opening brace
  if (!trap_GetEntityToken(com_token, sizeof(com_token))) {
    // end of spawn string
    return qfalse;
  }
  if (com_token[0] != '{') {
    CG_Error("CG_ParseSpawnVars: found %s when expecting {", com_token);
  }

  // go through all the key / value pairs
  while (1) {
    // parse key
    if (!trap_GetEntityToken(keyname, sizeof(keyname))) {
      CG_Error("CG_ParseSpawnVars: EOF without closing "
               "brace");
    }

    if (keyname[0] == '}') {
      break;
    }

    // parse value
    if (!trap_GetEntityToken(com_token, sizeof(com_token))) {
      CG_Error("CG_ParseSpawnVars: EOF without closing "
               "brace");
    }

    if (com_token[0] == '}') {
      CG_Error("CG_ParseSpawnVars: closing brace "
               "without data");
    }
    if (cg.numSpawnVars == MAX_SPAWN_VARS) {
      CG_Error("CG_ParseSpawnVars: MAX_SPAWN_VARS");
    }
    cg.spawnVars[cg.numSpawnVars][0] = CG_AddSpawnVarToken(keyname);
    cg.spawnVars[cg.numSpawnVars][1] = CG_AddSpawnVarToken(com_token);
    cg.numSpawnVars++;
  }

  return qtrue;
}

void SP_worldspawn(void) {
  char *s;
  int i;

  CG_SpawnString("classname", "", &s);
  if (Q_stricmp(s, "worldspawn")) {
    CG_Error("SP_worldspawn: The first entity isn't 'worldspawn'");
  }

  cgs.ccLayers = 0;

  if (CG_SpawnVector2D("mapcoordsmins", "-128 128",
                       cg.mapcoordsMins) && // top left
      CG_SpawnVector2D("mapcoordsmaxs", "128 -128",
                       cg.mapcoordsMaxs)) // bottom right
  {
    cg.mapcoordsValid = qtrue;
  } else {
    cg.mapcoordsValid = qfalse;
  }

  CG_ParseSpawns();

  CG_SpawnString("cclayers", "0", &s);
  cgs.ccLayers = Q_atoi(s);

  for (i = 0; i < cgs.ccLayers; i++) {
    CG_SpawnString(va("cclayerceil%i", i), "0", &s);
    cgs.ccLayerCeils[i] = Q_atoi(s);
  }

  cg.mapcoordsScale[0] = 1 / (cg.mapcoordsMaxs[0] - cg.mapcoordsMins[0]);
  cg.mapcoordsScale[1] = 1 / (cg.mapcoordsMaxs[1] - cg.mapcoordsMins[1]);

  CG_SpawnString("atmosphere", "", &s);
  CG_EffectParse(s);

  cg.fiveMinuteSound_g[0] = cg.fiveMinuteSound_a[0] = cg.twoMinuteSound_g[0] =
      cg.twoMinuteSound_a[0] = cg.thirtySecondSound_g[0] =
          cg.thirtySecondSound_a[0] = '\0';

  CG_SpawnString("twoMinuteSound_axis", "axis_hq_5minutes", &s);
  Q_strncpyz(cg.fiveMinuteSound_g, s, sizeof(cg.fiveMinuteSound_g));
  CG_SpawnString("twoMinuteSound_allied", "allies_hq_5minutes", &s);
  Q_strncpyz(cg.fiveMinuteSound_a, s, sizeof(cg.fiveMinuteSound_a));

  CG_SpawnString("twoMinuteSound_axis", "axis_hq_2minutes", &s);
  Q_strncpyz(cg.twoMinuteSound_g, s, sizeof(cg.twoMinuteSound_g));
  CG_SpawnString("twoMinuteSound_allied", "allies_hq_2minutes", &s);
  Q_strncpyz(cg.twoMinuteSound_a, s, sizeof(cg.twoMinuteSound_a));

  CG_SpawnString("thirtySecondSound_axis", "axis_hq_30seconds", &s);
  Q_strncpyz(cg.thirtySecondSound_g, s, sizeof(cg.thirtySecondSound_g));
  CG_SpawnString("thirtySecondSound_allied", "allies_hq_30seconds", &s);
  Q_strncpyz(cg.thirtySecondSound_a, s, sizeof(cg.thirtySecondSound_a));

  // 5 minute axis
  if (!*cg.fiveMinuteSound_g) {
    cgs.media.fiveMinuteSound_g = 0;
  } else if (strstr(cg.fiveMinuteSound_g, ".wav")) {
    cgs.media.fiveMinuteSound_g =
        trap_S_RegisterSound(cg.fiveMinuteSound_g, qtrue);
  } else {
    cgs.media.fiveMinuteSound_g = -1;
  }

  // 5 minute allied
  if (!*cg.fiveMinuteSound_a) {
    cgs.media.fiveMinuteSound_a = 0;
  } else if (strstr(cg.fiveMinuteSound_a, ".wav")) {
    cgs.media.fiveMinuteSound_a =
        trap_S_RegisterSound(cg.fiveMinuteSound_a, qtrue);
  } else {
    cgs.media.fiveMinuteSound_a = -1;
  }

  // 2 minute axis
  if (!*cg.twoMinuteSound_g) {
    cgs.media.twoMinuteSound_g = 0;
  } else if (strstr(cg.twoMinuteSound_g, ".wav")) {
    cgs.media.twoMinuteSound_g =
        trap_S_RegisterSound(cg.twoMinuteSound_g, qtrue);
  } else {
    cgs.media.twoMinuteSound_g = -1;
  }

  // 2 minute allied
  if (!*cg.twoMinuteSound_a) {
    cgs.media.twoMinuteSound_a = 0;
  } else if (strstr(cg.twoMinuteSound_a, ".wav")) {
    cgs.media.twoMinuteSound_a =
        trap_S_RegisterSound(cg.twoMinuteSound_a, qtrue);
  } else {
    cgs.media.twoMinuteSound_a = -1;
  }

  // 30 seconds axis
  if (!*cg.thirtySecondSound_g) {
    cgs.media.thirtySecondSound_g = 0;
  } else if (strstr(cg.thirtySecondSound_g, ".wav")) {
    cgs.media.thirtySecondSound_g =
        trap_S_RegisterSound(cg.thirtySecondSound_g, qtrue);
  } else {
    cgs.media.thirtySecondSound_g = -1;
  }

  // 30 seconds allied
  if (!*cg.thirtySecondSound_a) {
    cgs.media.thirtySecondSound_a = 0;
  } else if (strstr(cg.thirtySecondSound_a, ".wav")) {
    cgs.media.thirtySecondSound_a =
        trap_S_RegisterSound(cg.thirtySecondSound_a, qtrue);
  } else {
    cgs.media.thirtySecondSound_a = -1;
  }

  for (int i = 0; i < MAX_RENDER_STRINGS; i++) {
    CG_SpawnString(va("text%i", i), "", &s);
    Q_strncpyz(cg.deformText[i], s, sizeof(cg.deformText[0]));
  }
}

/*
==============
CG_ParseEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void CG_ParseEntitiesFromString(void) {
  // allow calls to CG_Spawn*()
  cg.spawning = qtrue;
  cg.numSpawnVars = 0;
  cg.numMiscGameModels = 0;
  cg.numCoronas = 0;
  cg.numDlights = 0;

  // the worldspawn is not an actual entity, but it still
  // has a "spawn" function to perform any global setup
  // needed by a level (setting configstrings or cvars, etc)
  if (!CG_ParseSpawnVars()) {
    CG_Error("ParseEntities: no entities");
  }
  SP_worldspawn();

  // parse ents
  while (CG_ParseSpawnVars()) {
    CG_ParseEntityFromSpawnVars();
  }

  cg.spawning = qfalse; // any future calls to CG_Spawn*() will be errors
}
