// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"
#include "etj_demo_compatibility.h"
#include "etj_entity_events_handler.h"
#include <algorithm>
#include "etj_player_events_handler.h"
#include "../game/etj_string_utilities.h"

extern void CG_StartShakeCamera(float param, entityState_t *es);
extern void CG_Tracer(vec3_t source, vec3_t dest, int sparks);
//==========================================================================

// from cg_weapons.c
extern int CG_WeaponIndex(int weapnum, int *bank, int *cycle);

static bool CG_IsPrimaryWeapon(int weapon, const playerState_t *ps) {
  const auto team = static_cast<team_t>(ps->persistant[PERS_TEAM]);

  if (team == TEAM_AXIS) {
    if (weapon == WP_THOMPSON) {
      weapon = WP_MP40;
    } else if (weapon == WP_CARBINE) {
      weapon = WP_KAR98;
    } else if (weapon == WP_GARAND) {
      weapon = WP_K43;
    }

  } else if (team == TEAM_ALLIES) {
    if (weapon == WP_MP40) {
      weapon = WP_THOMPSON;
    } else if (weapon == WP_KAR98) {
      weapon = WP_CARBINE;
    } else if (weapon == WP_K43) {
      weapon = WP_GARAND;
    }
  }

  return BG_WeaponIsPrimaryForClassAndTeam(ps->stats[STAT_PLAYER_CLASS], team,
                                           static_cast<weapon_t>(weapon));
}

/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
static void CG_ItemPickup(int itemNum) {
  const int itemID = bg_itemlist[itemNum].giTag;
  const int autoSwitch = cg_autoswitch.integer;

  if (etj_itemPickupText.integer) {
    CG_AddPMItem(PM_MESSAGE, va("Picked up %s", CG_PickupItemText(itemNum)),
                 cgs.media.pmImages[PM_MESSAGE]);
  }

  if (autoSwitch == AutoSwitchFlags::Disabled ||
      bg_itemlist[itemNum].giType != IT_WEAPON) {
    return;
  }

  // never autoswitch to scopes (this shouldn't happen anyway) or to ammo pack
  if (itemID == WP_FG42SCOPE || itemID == WP_GARAND_SCOPE ||
      itemID == WP_K43_SCOPE || itemID == WP_AMMO) {
    return;
  }

  // don't allow bypassing reload animation with autoswitch
  if (cg.predictedPlayerState.weaponstate == WEAPON_RELOADING) {
    return;
  }

  // always switch if we have no weapon selected
  if (!cg.weaponSelect) {
    cg.weaponSelect = itemID;
    cg.weaponSelectTime = cg.time;
    return;
  }

  const bool isPrimary = CG_IsPrimaryWeapon(itemID, &cg.snap->ps);
  const bool primaryActive = CG_IsPrimaryWeapon(cg.weaponSelect, &cg.snap->ps);

  bool doSwitch = false;

  if (autoSwitch & AutoSwitchFlags::Enabled) {
    doSwitch = true;
  }

  if (autoSwitch & AutoSwitchFlags::IfReplacingPrimary) {
    if (isPrimary && !primaryActive) {
      doSwitch = false;
    }
  }

  if (autoSwitch & AutoSwitchFlags::IgnorePortalGun) {
    if (itemID == WP_PORTAL_GUN) {
      doSwitch = false;
    }
  }

  if (doSwitch) {
    cg.weaponSelect = itemID;
    cg.weaponSelectTime = cg.time;
  }
}

/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
typedef struct {
  char *tag;
  int refEntOfs;
  int anim;
} painAnimForTag_t;

#define PEFOFS(x) ((int)&(((playerEntity_t *)0)->x))

void CG_PainEvent(centity_t *cent, int health, qboolean crouching) {
  const char *snd;

  // don't do more than two pain sounds a second
  if (cg.time - cent->pe.painTime < 500) {
    return;
  }

  if (health < 25) {
    snd = "*pain25_1.wav";
  } else if (health < 50) {
    snd = "*pain50_1.wav";
  } else if (health < 75) {
    snd = "*pain75_1.wav";
  } else {
    snd = "*pain100_1.wav";
  }
  trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE,
                    CG_CustomSound(cent->currentState.number, snd));

  // save pain time for programitic twitch animation
  cent->pe.painTime = cg.time;
  cent->pe.painDirection ^= 1;
}

/*
==============
CG_Explode


    if (cent->currentState.angles2[0] || cent->currentState.angles2[1] ||
cent->currentState.angles2[2])

==============
*/

#define POSSIBLE_PIECES 6

typedef struct fxSound_s {
  int max;
  qhandle_t sound[3];
  const char *soundfile[3];
} fxSound_t;

static fxSound_t fxSounds[POSSIBLE_PIECES] = {
    // wood
    {1, {-1, -1, -1}, {"sound/world/boardbreak.wav", NULL, NULL}},
    // glass
    {3,
     {-1, -1, -1},
     {"sound/world/glassbreak1.wav", "sound/world/glassbreak2.wav",
      "sound/world/glassbreak3.wav"}},
    // metal
    {1, {-1, -1, -1}, {"sound/world/metalbreak.wav", NULL, NULL}},
    // gibs
    {1, {-1, -1, -1}, {"sound/world/gibsplit1.wav", NULL, NULL}},
    // brick
    {1, {-1, -1, -1}, {"sound/world/debris1.wav", NULL, NULL}},
    // stone
    {1, {-1, -1, -1}, {"sound/world/stonefall.wav", NULL, NULL}}};

void CG_PrecacheFXSounds(void) {
  int i, j;

  for (i = 0; i < POSSIBLE_PIECES; i++) {
    for (j = 0; j < fxSounds[i].max; j++) {
      fxSounds[i].sound[j] =
          trap_S_RegisterSound(fxSounds[i].soundfile[j], qfalse);
    }
  }
}

void CG_Explodef(vec3_t origin, vec3_t dir, int mass, int type, qhandle_t sound,
                 int forceLowGrav, qhandle_t shader);
void CG_RubbleFx(vec3_t origin, vec3_t dir, int mass, int type, qhandle_t sound,
                 int forceLowGrav, qhandle_t shader, float speedscale,
                 float sizescale);

/*
==============
CG_Explode
    the old cent-based explode calls will still work with this pass-through
==============
*/
void CG_Explode(centity_t *cent, vec3_t origin, vec3_t dir, qhandle_t shader) {

  qhandle_t inheritmodel = 0;

  // inherit shader
  // (SA) FIXME: do this at spawn time rather than explode time so any
  // new necessary shaders are created earlier
  if (cent->currentState.eFlags & EF_INHERITSHADER) {
    if (!shader) {
      //			inheritmodel =
      // cent->currentState.modelindex;
      inheritmodel =
          cgs.inlineDrawModel[cent->currentState.modelindex]; // okay, this
                                                              // should be
                                                              // better.
      if (inheritmodel) {
        shader = trap_R_GetShaderFromModel(inheritmodel, 0, 0);
      }
    }
  }

  if (!cent->currentState.dl_intensity) {
    sfxHandle_t sound;

    sound = random() * fxSounds[cent->currentState.frame].max;

    if (fxSounds[cent->currentState.frame].sound[sound] == -1) {
      fxSounds[cent->currentState.frame].sound[sound] = trap_S_RegisterSound(
          fxSounds[cent->currentState.frame].soundfile[sound], qfalse);
    }

    sound = fxSounds[cent->currentState.frame].sound[sound];

    CG_Explodef(origin, dir,
                cent->currentState.density, // mass
                cent->currentState.frame,   // type
                sound,                      // sound
                cent->currentState.weapon,  // forceLowGrav
                shader);
  } else {
    sfxHandle_t sound;

    if (cent->currentState.dl_intensity == -1) {
      sound = 0;
    } else {
      sound = cgs.gameSounds[cent->currentState.dl_intensity];
    }

    CG_Explodef(origin, dir,
                cent->currentState.density, // mass
                cent->currentState.frame,   // type
                sound,                      // sound
                cent->currentState.weapon,  // forceLowGrav
                shader);
  }
}

/*
==============
CG_Explode
    the old cent-based explode calls will still work with this pass-through
==============
*/
void CG_Rubble(centity_t *cent, vec3_t origin, vec3_t dir, qhandle_t shader) {

  qhandle_t inheritmodel = 0;

  // inherit shader
  // (SA) FIXME: do this at spawn time rather than explode time so any
  // new necessary shaders are created earlier
  if (cent->currentState.eFlags & EF_INHERITSHADER) {
    if (!shader) {
      //			inheritmodel =
      // cent->currentState.modelindex;
      inheritmodel =
          cgs.inlineDrawModel[cent->currentState.modelindex]; // okay, this
                                                              // should be
                                                              // better.
      if (inheritmodel) {
        shader = trap_R_GetShaderFromModel(inheritmodel, 0, 0);
      }
    }
  }

  if (!cent->currentState.dl_intensity) {
    sfxHandle_t sound;

    sound = random() * fxSounds[cent->currentState.frame].max;

    if (fxSounds[cent->currentState.frame].sound[sound] == -1) {
      fxSounds[cent->currentState.frame].sound[sound] = trap_S_RegisterSound(
          fxSounds[cent->currentState.frame].soundfile[sound], qfalse);
    }

    sound = fxSounds[cent->currentState.frame].sound[sound];

    CG_RubbleFx(origin, dir,
                cent->currentState.density, // mass
                cent->currentState.frame,   // type
                sound,                      // sound
                cent->currentState.weapon,  // forceLowGrav
                shader, cent->currentState.angles2[0],
                cent->currentState.angles2[1]);
  } else {
    sfxHandle_t sound;

    if (cent->currentState.dl_intensity == -1) {
      sound = 0;
    } else {
      sound = cgs.gameSounds[cent->currentState.dl_intensity];
    }

    CG_RubbleFx(origin, dir,
                cent->currentState.density, // mass
                cent->currentState.frame,   // type
                sound,                      // sound
                cent->currentState.weapon,  // forceLowGrav
                shader, cent->currentState.angles2[0],
                cent->currentState.angles2[1]);
  }
}

/*
==============
CG_RubbleFx
==============
*/
void CG_RubbleFx(vec3_t origin, vec3_t dir, int mass, int type,
                 sfxHandle_t sound, int forceLowGrav, qhandle_t shader,
                 float speedscale, float sizescale) {
  int i;
  localEntity_t *le;
  refEntity_t *re;
  int howmany, total, totalsounds;
  int pieces[6]; // how many of each piece
  qhandle_t modelshader = 0;
  float materialmul = 1; // multiplier for different types

  memset(&pieces, 0, sizeof(pieces));

  pieces[5] = (int)(mass / 250.0f);
  pieces[4] = (int)(mass / 76.0f);
  pieces[3] = (int)(mass / 37.0f); // so 2 per 75
  pieces[2] = (int)(mass / 15.0f);
  pieces[1] = (int)(mass / 10.0f);
  pieces[0] = (int)(mass / 5.0f);

  if (pieces[0] > 20) {
    pieces[0] = 20; // cap some of the smaller bits so they
                    // don't get out of control
  }
  if (pieces[1] > 15) {
    pieces[1] = 15;
  }
  if (pieces[2] > 10) {
    pieces[2] = 10;
  }

  if (type == 0) // cap wood even more since it's often grouped, and the
                 // small splinters can add up
  {
    if (pieces[0] > 10) {
      pieces[0] = 10;
    }
    if (pieces[1] > 10) {
      pieces[1] = 10;
    }
    if (pieces[2] > 10) {
      pieces[2] = 10;
    }
  }

  totalsounds = 0;
  total = pieces[5] + pieces[4] + pieces[3] + pieces[2] + pieces[1] + pieces[0];

  if (sound) {
    trap_S_StartSound(origin, -1, CHAN_AUTO, sound);
  }

  if (shader) // shader passed in to use
  {
    modelshader = shader;
  }

  for (i = 0; i < POSSIBLE_PIECES; i++) {
    leBounceSoundType_t snd = LEBS_NONE;
    int hmodel = 0;
    float scale;
    int endtime;
    for (howmany = 0; howmany < pieces[i]; howmany++) {

      scale = 1.0f;
      endtime = 0; // set endtime offset for
                   // faster/slower fadeouts

      switch (type) {
        case 0: // "wood"
          snd = LEBS_WOOD;
          hmodel = cgs.media.debWood[i];

          if (i == 0) {
            scale = 0.5f;
          } else if (i == 1) {
            scale = 0.6f;
          } else if (i == 2) {
            scale = 0.7f;
          } else if (i == 3) {
            scale = 0.5f;
          }
          //					else
          // goto pass;

          if (i < 3) {
            endtime = -3000; // small
                             // bits
                             // live 3
                             // sec
                             // shorter
                             // than
                             // normal
          }
          break;

        case 1: // "glass"
          snd = LEBS_NONE;
          if (i == 5) {
            hmodel = cgs.media.shardGlass1;
          } else if (i == 4) {
            hmodel = cgs.media.shardGlass2;
          } else if (i == 2) {
            hmodel = cgs.media.shardGlass2;
          } else if (i == 1) {
            hmodel = cgs.media.shardGlass2;
            scale = 0.5f;
          } else {
            goto pass;
          }
          break;

        case 2: // "metal"
          snd = LEBS_METAL;
          if (i == 5) {
            hmodel = cgs.media.shardMetal1;
          } else if (i == 4) {
            hmodel = cgs.media.shardMetal2;
          } else if (i == 2) {
            hmodel = cgs.media.shardMetal2;
          } else if (i == 1) {
            hmodel = cgs.media.shardMetal2;
            scale = 0.5f;
          } else {
            goto pass;
          }
          break;

        case 3: // "gibs"
          snd = LEBS_BLOOD;
          if (i == 5) {
            hmodel = cgs.media.gibIntestine;
          } else if (i == 4) {
            hmodel = cgs.media.gibLeg;
          } else if (i == 2) {
            hmodel = cgs.media.gibChest;
          } else {
            goto pass;
          }
          break;

        case 4: // "brick"
          snd = LEBS_ROCK;
          hmodel = cgs.media.debBlock[i];
          break;

        case 5: // "rock"
          snd = LEBS_ROCK;
          if (i == 5) {
            hmodel = cgs.media.debRock[2]; // temporarily
                                           // use
                                           // the
                                           // next
                                           // smallest
                                           // rock
                                           // piece
          } else if (i == 4) {
            hmodel = cgs.media.debRock[2];
          } else if (i == 3) {
            hmodel = cgs.media.debRock[1];
          } else if (i == 2) {
            hmodel = cgs.media.debRock[0];
          } else if (i == 1) {
            hmodel = cgs.media.debBlock[1]; // temporarily
                                            // use
                                            // the
                                            // small
                                            // block
                                            // pieces
          } else {
            hmodel = cgs.media.debBlock[0]; // temporarily
                                            // use
                                            // the
                                            // small
                                            // block
                                            // pieces
          }
          if (i <= 2) {
            endtime = -2000; // small
                             // bits
                             // live 2
                             // sec
                             // shorter
                             // than
                             // normal
          }
          break;

        case 6: // "fabric"
          if (i == 5) {
            hmodel = cgs.media.debFabric[0];
          } else if (i == 4) {
            hmodel = cgs.media.debFabric[1];
          } else if (i == 2) {
            hmodel = cgs.media.debFabric[2];
          } else if (i == 1) {
            hmodel = cgs.media.debFabric[2];
            scale = 0.5;
          } else {
            goto pass; // (only
                       // do 5,
                       // 4, 2
                       // and 1)
          }
          break;
      }

      le = CG_AllocLocalEntity();
      re = &le->refEntity;

      le->leType = LE_FRAGMENT;
      le->startTime = cg.time;

      le->endTime = (le->startTime + 5000 + random() * 5000) + endtime;

      // as it turns out, i'm not sure if setting the
      // re->axis here will actually do anything
      //			AxisClear(re->axis);
      //			re->axis[0][0] =
      //			re->axis[1][1] =
      //			re->axis[2][2] = scale;
      //
      //			if(scale != 1.0)
      //				re->nonNormalizedAxes
      //= qtrue;

      le->sizeScale = scale * sizescale;

      if (type == 1) // glass
      {              // Rafael added this because glass looks funky
                     // when it fades out
        // TBD: need to look into this so that
        // they fade out correctly
        re->fadeStartTime = le->endTime;
        re->fadeEndTime = le->endTime;
      } else {
        re->fadeStartTime = le->endTime - 4000;
        re->fadeEndTime = le->endTime;
      }

      if (total > 5) {
        if (totalsounds > 5 || (howmany % 8) != 0) {
          snd = LEBS_NONE;
        } else {
          totalsounds++;
        }
      }

      le->lifeRate = 1.0 / (le->endTime - le->startTime);
      le->leFlags = LEF_TUMBLE;
      le->leMarkType = LEMT_NONE;

      VectorCopy(origin, re->origin);
      AxisCopy(axisDefault, re->axis);

      le->leBounceSoundType = snd;
      re->hModel = hmodel;

      // inherit shader
      if (modelshader) {
        re->customShader = modelshader;
      }

      re->radius = 1000;

      // trying to make this a little more interesting
      if (type == 6) // "fabric"
      {
        le->pos.trType = TR_GRAVITY_FLOAT; // the fabric
                                           // stuff will
                                           // change to use
                                           // something that
                                           // looks better
      } else {
        if (!forceLowGrav && rand() & 1) // if low gravity is not
                                         // forced and die roll goes
                                         // our way use regular grav
        {
          le->pos.trType = TR_GRAVITY;
        } else {
          le->pos.trType = TR_GRAVITY_LOW;
        }
      }

      switch (type) {
        case 6: // fabric
          le->bounceFactor = 0.0;
          materialmul = 0.3; // rotation speed
          break;
        default:
          le->bounceFactor = 0.4;
          break;
      }

      // rotation
      le->angles.trType = TR_LINEAR;
      le->angles.trTime = cg.time;
      le->angles.trBase[0] = rand() & 31;
      le->angles.trBase[1] = rand() & 31;
      le->angles.trBase[2] = rand() & 31;
      le->angles.trDelta[0] = ((100 + (rand() & 500)) - 300) * materialmul;
      le->angles.trDelta[1] = ((100 + (rand() & 500)) - 300) * materialmul;
      le->angles.trDelta[2] = ((100 + (rand() & 500)) - 300) * materialmul;

      //			if(type == 6)	//
      // fabric 				materialmul = 1;
      // //
      // translation speed

      VectorCopy(origin, le->pos.trBase);
      VectorNormalize(dir);
      le->pos.trTime = cg.time;

      // (SA) hoping that was just intended to represent
      // randomness
      //			if
      //(cent->currentState.angles2[0] ||
      // cent->currentState.angles2[1] ||
      // cent->currentState.angles2[2])
      if (le->angles.trBase[0] == 1 || le->angles.trBase[1] == 1 ||
          le->angles.trBase[2] == 1) {
        le->pos.trType = TR_GRAVITY;
        VectorScale(dir, 10 * 8, le->pos.trDelta);
        le->pos.trDelta[0] += ((random() * 400) - 200) * speedscale;
        le->pos.trDelta[1] += ((random() * 400) - 200) * speedscale;
        le->pos.trDelta[2] = ((random() * 400) + 400) * speedscale;

      } else {
        // location
        VectorScale(dir, 200 + mass, le->pos.trDelta);
        le->pos.trDelta[0] += ((random() * 200) - 100);
        le->pos.trDelta[1] += ((random() * 200) - 100);

        if (dir[2]) {
          le->pos.trDelta[2] = random() * 200 * materialmul; // randomize
                                                             // sort of a
                                                             // lot so
                                                             // they don't
                                                             // all land
                                                             // together
        } else {
          le->pos.trDelta[2] = random() * 20;
        }
      }
    }
  pass:
    continue;
  }
}

/*
==============
CG_Explodef
    made this more generic for spawning hits and breaks without needing a *cent
==============
*/
void CG_Explodef(vec3_t origin, vec3_t dir, int mass, int type, qhandle_t sound,
                 int forceLowGrav, qhandle_t shader) {
  int i;
  localEntity_t *le;
  refEntity_t *re;
  int howmany, total, totalsounds;
  int pieces[6]; // how many of each piece
  qhandle_t modelshader = 0;
  float materialmul = 1; // multiplier for different types

  memset(&pieces, 0, sizeof(pieces));

  pieces[5] = (int)(mass / 250.0f);
  pieces[4] = (int)(mass / 76.0f);
  pieces[3] = (int)(mass / 37.0f); // so 2 per 75
  pieces[2] = (int)(mass / 15.0f);
  pieces[1] = (int)(mass / 10.0f);
  pieces[0] = (int)(mass / 5.0f);

  if (pieces[0] > 20) {
    pieces[0] = 20; // cap some of the smaller bits so they
                    // don't get out of control
  }
  if (pieces[1] > 15) {
    pieces[1] = 15;
  }
  if (pieces[2] > 10) {
    pieces[2] = 10;
  }

  if (type == 0) // cap wood even more since it's often grouped, and the
                 // small splinters can add up
  {
    if (pieces[0] > 10) {
      pieces[0] = 10;
    }
    if (pieces[1] > 10) {
      pieces[1] = 10;
    }
    if (pieces[2] > 10) {
      pieces[2] = 10;
    }
  }

  total = pieces[5] + pieces[4] + pieces[3] + pieces[2] + pieces[1] + pieces[0];
  totalsounds = 0;

  if (sound) {
    trap_S_StartSound(origin, -1, CHAN_AUTO, sound);
  }

  if (shader) // shader passed in to use
  {
    modelshader = shader;
  }

  for (i = 0; i < POSSIBLE_PIECES; i++) {
    leBounceSoundType_t snd = LEBS_NONE;
    int hmodel = 0;
    float scale;
    int endtime;
    for (howmany = 0; howmany < pieces[i]; howmany++) {

      scale = 1.0f;
      endtime = 0; // set endtime offset for
                   // faster/slower fadeouts

      switch (type) {
        case 0: // "wood"
          snd = LEBS_WOOD;
          hmodel = cgs.media.debWood[i];

          if (i == 0) {
            scale = 0.5f;
          } else if (i == 1) {
            scale = 0.6f;
          } else if (i == 2) {
            scale = 0.7f;
          } else if (i == 3) {
            scale = 0.5f;
          }
          //					else
          // goto pass;

          if (i < 3) {
            endtime = -3000; // small
                             // bits
                             // live 3
                             // sec
                             // shorter
                             // than
                             // normal
          }
          break;

        case 1: // "glass"
          snd = LEBS_NONE;
          if (i == 5) {
            hmodel = cgs.media.shardGlass1;
          } else if (i == 4) {
            hmodel = cgs.media.shardGlass2;
          } else if (i == 2) {
            hmodel = cgs.media.shardGlass2;
          } else if (i == 1) {
            hmodel = cgs.media.shardGlass2;
            scale = 0.5f;
          } else {
            goto pass;
          }
          break;

        case 2: // "metal"
          snd = LEBS_BRASS;
          if (i == 5) {
            hmodel = cgs.media.shardMetal1;
          } else if (i == 4) {
            hmodel = cgs.media.shardMetal2;
          } else if (i == 2) {
            hmodel = cgs.media.shardMetal2;
          } else if (i == 1) {
            hmodel = cgs.media.shardMetal2;
            scale = 0.5f;
          } else {
            goto pass;
          }
          break;

        case 3: // "gibs"
          snd = LEBS_BLOOD;
          if (i == 5) {
            hmodel = cgs.media.gibIntestine;
          } else if (i == 4) {
            hmodel = cgs.media.gibLeg;
          } else if (i == 2) {
            hmodel = cgs.media.gibChest;
          } else {
            goto pass;
          }
          break;

        case 4: // "brick"
          snd = LEBS_ROCK;
          hmodel = cgs.media.debBlock[i];
          break;

        case 5: // "rock"
          snd = LEBS_ROCK;
          if (i == 5) {
            hmodel = cgs.media.debRock[2]; // temporarily
                                           // use
                                           // the
                                           // next
                                           // smallest
                                           // rock
                                           // piece
          } else if (i == 4) {
            hmodel = cgs.media.debRock[2];
          } else if (i == 3) {
            hmodel = cgs.media.debRock[1];
          } else if (i == 2) {
            hmodel = cgs.media.debRock[0];
          } else if (i == 1) {
            hmodel = cgs.media.debBlock[1]; // temporarily
                                            // use
                                            // the
                                            // small
                                            // block
                                            // pieces
          } else {
            hmodel = cgs.media.debBlock[0]; // temporarily
                                            // use
                                            // the
                                            // small
                                            // block
                                            // pieces
          }
          if (i <= 2) {
            endtime = -2000; // small
                             // bits
                             // live 2
                             // sec
                             // shorter
                             // than
                             // normal
          }
          break;

        case 6: // "fabric"
          if (i == 5) {
            hmodel = cgs.media.debFabric[0];
          } else if (i == 4) {
            hmodel = cgs.media.debFabric[1];
          } else if (i == 2) {
            hmodel = cgs.media.debFabric[2];
          } else if (i == 1) {
            hmodel = cgs.media.debFabric[2];
            scale = 0.5;
          } else {
            goto pass; // (only
                       // do 5,
                       // 4, 2
                       // and 1)
          }
          break;
      }

      le = CG_AllocLocalEntity();
      re = &le->refEntity;

      le->leType = LE_FRAGMENT;
      le->startTime = cg.time;

      le->endTime = (le->startTime + 5000 + random() * 5000) + endtime;

      // as it turns out, i'm not sure if setting the
      // re->axis here will actually do anything
      //			AxisClear(re->axis);
      //			re->axis[0][0] =
      //			re->axis[1][1] =
      //			re->axis[2][2] = scale;
      //
      //			if(scale != 1.0)
      //				re->nonNormalizedAxes
      //= qtrue;

      le->sizeScale = scale;

      if (type == 1) // glass
      {              // Rafael added this because glass looks funky
                     // when it fades out
        // TBD: need to look into this so that
        // they fade out correctly
        re->fadeStartTime = le->endTime;
        re->fadeEndTime = le->endTime;
      } else {
        re->fadeStartTime = le->endTime - 4000;
        re->fadeEndTime = le->endTime;
      }

      if (total > 5) {
        if (totalsounds > 5 || (howmany % 8) != 0) {
          snd = LEBS_NONE;
        } else {
          totalsounds++;
        }
      }

      le->lifeRate = 1.0 / (le->endTime - le->startTime);
      le->leFlags = LEF_TUMBLE;
      le->leMarkType = LEMT_NONE;

      VectorCopy(origin, re->origin);
      AxisCopy(axisDefault, re->axis);

      le->leBounceSoundType = snd;
      re->hModel = hmodel;

      // inherit shader
      if (modelshader) {
        re->customShader = modelshader;
      }

      re->radius = 1000;

      // trying to make this a little more interesting
      if (type == 6) // "fabric"
      {
        le->pos.trType = TR_GRAVITY_FLOAT; // the fabric
                                           // stuff will
                                           // change to use
                                           // something that
                                           // looks better
      } else {
        if (!forceLowGrav && rand() & 1) // if low gravity is not
                                         // forced and die roll goes
                                         // our way use regular grav
        {
          le->pos.trType = TR_GRAVITY;
        } else {
          le->pos.trType = TR_GRAVITY_LOW;
        }
      }

      switch (type) {
        case 6: // fabric
          le->bounceFactor = 0.0;
          materialmul = 0.3; // rotation speed
          break;
        default:
          le->bounceFactor = 0.4;
          break;
      }

      // rotation
      le->angles.trType = TR_LINEAR;
      le->angles.trTime = cg.time;
      le->angles.trBase[0] = rand() & 31;
      le->angles.trBase[1] = rand() & 31;
      le->angles.trBase[2] = rand() & 31;
      le->angles.trDelta[0] = ((100 + (rand() & 500)) - 300) * materialmul;
      le->angles.trDelta[1] = ((100 + (rand() & 500)) - 300) * materialmul;
      le->angles.trDelta[2] = ((100 + (rand() & 500)) - 300) * materialmul;

      //			if(type == 6)	//
      // fabric 				materialmul = 1;
      // //
      // translation speed

      VectorCopy(origin, le->pos.trBase);
      VectorNormalize(dir);
      le->pos.trTime = cg.time;

      // (SA) hoping that was just intended to represent
      // randomness
      //			if
      //(cent->currentState.angles2[0] ||
      // cent->currentState.angles2[1] ||
      // cent->currentState.angles2[2])
      if (le->angles.trBase[0] == 1 || le->angles.trBase[1] == 1 ||
          le->angles.trBase[2] == 1) {
        le->pos.trType = TR_GRAVITY;
        VectorScale(dir, 10 * 8, le->pos.trDelta);
        le->pos.trDelta[0] += ((random() * 100) - 50);
        le->pos.trDelta[1] += ((random() * 100) - 50);
        le->pos.trDelta[2] = (random() * 200) + 200;

      } else {
        // location
        VectorScale(dir, 200 + mass, le->pos.trDelta);
        le->pos.trDelta[0] += ((random() * 100) - 50);
        le->pos.trDelta[1] += ((random() * 100) - 50);

        if (dir[2]) {
          le->pos.trDelta[2] = random() * 200 * materialmul; // randomize
                                                             // sort of a
                                                             // lot so
                                                             // they don't
                                                             // all land
                                                             // together
        } else {
          le->pos.trDelta[2] = random() * 20;
        }
      }
    }
  pass:
    continue;
  }
}

/*
==============
CG_Effect
    Quake ed -> target_effect (0 .5 .8) (-6 -6 -6) (6 6 6) fire explode smoke
debris gore lowgrav
==============
*/
void CG_Effect(centity_t *cent, vec3_t origin, vec3_t dir) {
  localEntity_t *le;
  refEntity_t *re;
  //	int				howmany;
  //	int				large, small;
  vec4_t projection, color;

  VectorSet(dir, 0, 0, 1); // straight up.

  //		1 large per 100, 1 small per 24
  //	large	= (int)(mass / 100);
  //	small	= (int)(mass / 24) + 1;

  if (cent->currentState.eventParm & 1) // fire
  {
    CG_MissileHitWall(WP_DYNAMITE, 0, origin, dir, 0);
    return;
  }

  // (SA) right now force smoke on any explosions
  //	if(cent->currentState.eventParm & 4)	// smoke
  if (cent->currentState.eventParm & 7) {
    int i, j;
    vec3_t sprVel, sprOrg;
    // explosion sprite animation
    VectorScale(dir, 16, sprVel);
    for (i = 0; i < 5; i++) {
      for (j = 0; j < 3; j++)
        sprOrg[j] = origin[j] + 64 * dir[j] + 24 * crandom();
      sprVel[2] += rand() % 50;
      //			CG_ParticleExplosion( 2,
      // sprOrg, sprVel,
      // 1000+rand()%250, 20, 40+rand()%60 );
      CG_ParticleExplosion("blacksmokeanim", sprOrg, sprVel,
                           3500 + rand() % 250, 10, 250 + rand() % 60,
                           qfalse); // JPW NERVE was smokeanimb
    }
  }

  if (cent->currentState.eventParm & 2) // explode
  {
    vec3_t sprVel, sprOrg;
    trap_S_StartSound(origin, -1, CHAN_AUTO, cgs.media.sfx_rockexp);

    // new explode	(from rl)
    VectorMA(origin, 16, dir, sprOrg);
    VectorScale(dir, 100, sprVel);
    CG_ParticleExplosion("explode1", sprOrg, sprVel, 500, 20, 160, qtrue);
    // CG_ParticleExplosion( "blueexp", sprOrg, sprVel, 1200, 9,
    // 300 );

    // (SA) this is done only if the level designer has it
    // marked in the entity.
    //		(see "cent->currentState.eventParm & 64" below)

    // RF, throw some debris
    //		CG_AddDebris( origin, dir,
    //						280,	// speed
    //						1400,	//
    // duration
    //						// 15 + rand()%5 );	//
    // count 						7 + rand()%2 );	// count

    //%	CG_ImpactMark( cgs.media.burnMarkShader, origin, dir,
    // random()*360,
    // 1,1,1,1, qfalse, 64, qfalse, 0xffffffff );
    VectorSet(projection, 0, 0, -1);
    projection[3] = 64.0f;
    Vector4Set(color, 1.0f, 1.0f, 1.0f, 1.0f);
    trap_R_ProjectDecal(cgs.media.burnMarkShader, 1, (vec3_t *)origin,
                        projection, color, cg_markTime.integer,
                        (cg_markTime.integer >> 4));
  }

  if (cent->currentState.eventParm & 8) // rubble
  { // share the cg_explode code with func_explosives
    const char *s;
    qhandle_t sh = 0; // shader handle

    vec3_t newdir = {0, 0, 0};

    if (cent->currentState.angles2[0] || cent->currentState.angles2[1] ||
        cent->currentState.angles2[2]) {
      VectorCopy(cent->currentState.angles2, newdir);
    }

    s = CG_ConfigString(CS_TARGETEFFECT); // see if ent has a shader specified
    if (s && strlen(s) > 0) {
      sh = trap_R_RegisterShader(
          va("textures/%s", s)); // FIXME: don't do this
                                 // here.  only for testing
    }
    cent->currentState.eFlags &=
        ~EF_INHERITSHADER;               // don't try to inherit shader
    cent->currentState.dl_intensity = 0; // no sound
    CG_Explode(cent, origin, newdir, sh);
  }

  if (cent->currentState.eventParm & 16) // gore
  {
    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    le->leType = LE_FRAGMENT;
    le->startTime = cg.time;
    le->endTime = le->startTime + 5000 + random() * 3000;
    //----(SA)	fading out
    re->fadeStartTime = le->endTime - 4000;
    re->fadeEndTime = le->endTime;
    //----(SA)	end

    VectorCopy(origin, re->origin);
    AxisCopy(axisDefault, re->axis);
    //	re->hModel = hModel;
    re->hModel = cgs.media.gibIntestine;
    le->pos.trType = TR_GRAVITY;
    VectorCopy(origin, le->pos.trBase);

    //	VectorCopy( velocity, le->pos.trDelta );
    VectorNormalize(dir);
    VectorMA(dir, 200, dir, le->pos.trDelta);

    le->pos.trTime = cg.time;

    le->bounceFactor = 0.3;

    le->leBounceSoundType = LEBS_BLOOD;
    le->leMarkType = LEMT_BLOOD;
  }

  if (cent->currentState.eventParm &
      64) // debris trails (the black strip that Ryan did)
  {
    CG_AddDebris(origin, dir,
                 280,  // speed
                 1400, // duration
                 // 15 + rand()%5 );	// count
                 7 + rand() % 2); // count
  }
}

/*
CG_Shard

    We should keep this separate since there will be considerable differences
    in the physical properties of shard vrs debris. not to mention the fact
    there is no way we can quantify what type of effects the designers will
    potentially desire. If it is still possible to merge the functionality of
    cg_shard into cg_explode at a latter time I would have no problem with that
    but for now I want to keep it separate
*/
void CG_Shard(centity_t *cent, vec3_t origin, vec3_t dir) {
  localEntity_t *le;
  refEntity_t *re;
  int type;
  int howmany;
  int i;
  int rval;

  qboolean isflyingdebris = qfalse;

  type = cent->currentState.density;
  howmany = cent->currentState.frame;

  for (i = 0; i < howmany; i++) {
    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    le->leType = LE_FRAGMENT;
    le->startTime = cg.time;
    le->endTime = le->startTime + 5000 + random() * 5000;

    //----(SA)	fading out
    re->fadeStartTime = le->endTime - 1000;
    re->fadeEndTime = le->endTime;
    //----(SA)	end

    if (type == 999) {
      le->startTime = cg.time;
      le->endTime = le->startTime + 100;
      re->fadeStartTime = le->endTime - 100;
      re->fadeEndTime = le->endTime;
      type = 1;

      isflyingdebris = qtrue;
    }

    le->lifeRate = 1.0 / (le->endTime - le->startTime);
    le->leFlags = LEF_TUMBLE;
    le->bounceFactor = 0.4;
    // le->leBounceSoundType	= LEBS_WOOD;
    le->leMarkType = LEMT_NONE;

    VectorCopy(origin, re->origin);
    AxisCopy(axisDefault, re->axis);

    if (type == FXTYPE_GLASS) // glass
    {
      rval = rand() % 2;

      if (rval) {
        re->hModel = cgs.media.shardGlass1;
      } else {
        re->hModel = cgs.media.shardGlass2;
      }
    } else if (type == FXTYPE_WOOD) // wood
    {
      rval = rand() % 2;

      if (rval) {
        re->hModel = cgs.media.shardWood1;
      } else {
        re->hModel = cgs.media.shardWood2;
      }
    } else if (type == FXTYPE_METAL) // metal
    {
      rval = rand() % 2;

      if (rval) {
        re->hModel = cgs.media.shardMetal1;
      } else {
        re->hModel = cgs.media.shardMetal2;
      }
    }
    /*else if (type == 3) // ceramic
    {
        rval = rand()%2;

        if (rval)
            re->hModel = cgs.media.shardCeramic1;
        else
            re->hModel = cgs.media.shardCeramic2;
    }*/
    else if (type == FXTYPE_BRICK || type == FXTYPE_STONE) // rubble
    {
      rval = rand() % 3;

      if (rval == 1) {
        re->hModel = cgs.media.shardRubble1;
      } else if (rval == 2) {
        re->hModel = cgs.media.shardRubble2;
      } else {
        re->hModel = cgs.media.shardRubble3;
      }

    } else {
      CG_Printf("CG_Debris has an unknown type\n");
    }

    // location
    if (isflyingdebris) {
      le->pos.trType = TR_GRAVITY_LOW;
    } else {
      le->pos.trType = TR_GRAVITY;
    }

    VectorCopy(origin, le->pos.trBase);
    VectorNormalize(dir);
    VectorScale(dir, 10 * howmany, le->pos.trDelta);
    le->pos.trTime = cg.time;
    le->pos.trDelta[0] += ((random() * 100) - 50);
    le->pos.trDelta[1] += ((random() * 100) - 50);
    if (type) {
      le->pos.trDelta[2] =
          (random() * 200) + 100; // randomize sort of a lot so they don't
                                  // all land together
    } else                        // glass
    {
      le->pos.trDelta[2] =
          (random() * 100) + 50; // randomize sort of a lot so they don't
                                 // all land together
    }
    // rotation
    le->angles.trType = TR_LINEAR;
    le->angles.trTime = cg.time;
    le->angles.trBase[0] = rand() & 31;
    le->angles.trBase[1] = rand() & 31;
    le->angles.trBase[2] = rand() & 31;
    le->angles.trDelta[0] = (100 + (rand() & 500)) - 300;
    le->angles.trDelta[1] = (100 + (rand() & 500)) - 300;
    le->angles.trDelta[2] = (100 + (rand() & 500)) - 300;
  }
}

void CG_ShardJunk(centity_t *cent, vec3_t origin, vec3_t dir) {
  localEntity_t *le;
  refEntity_t *re;

  le = CG_AllocLocalEntity();
  re = &le->refEntity;

  le->leType = LE_FRAGMENT;
  le->startTime = cg.time;
  le->endTime = le->startTime + 5000 + random() * 5000;

  re->fadeStartTime = le->endTime - 1000;
  re->fadeEndTime = le->endTime;

  le->lifeRate = 1.0 / (le->endTime - le->startTime);
  le->leFlags = LEF_TUMBLE;
  le->bounceFactor = 0.4;
  le->leMarkType = LEMT_NONE;

  VectorCopy(origin, re->origin);
  AxisCopy(axisDefault, re->axis);

  re->hModel = cgs.media.shardJunk[rand() % MAX_LOCKER_DEBRIS];

  le->pos.trType = TR_GRAVITY;

  VectorCopy(origin, le->pos.trBase);
  VectorNormalize(dir);
  VectorScale(dir, 10 * 8, le->pos.trDelta);
  le->pos.trTime = cg.time;
  le->pos.trDelta[0] += ((random() * 100) - 50);
  le->pos.trDelta[1] += ((random() * 100) - 50);

  le->pos.trDelta[2] =
      (random() * 100) +
      50; // randomize sort of a lot so they don't all land together

  // rotation
  le->angles.trType = TR_LINEAR;
  le->angles.trTime = cg.time;
  // le->angles.trBase[0] = rand()&31;
  // le->angles.trBase[1] = rand()&31;
  le->angles.trBase[2] = rand() & 31;

  // le->angles.trDelta[0] = (100 + (rand()&500)) - 300;
  // le->angles.trDelta[1] = (100 + (rand()&500)) - 300;
  le->angles.trDelta[2] = (100 + (rand() & 500)) - 300;
}

// Gordon: debris test
void CG_Debris(centity_t *cent, vec3_t origin, vec3_t dir) {
  localEntity_t *le;
  refEntity_t *re;

  le = CG_AllocLocalEntity();
  re = &le->refEntity;

  le->leType = LE_FRAGMENT;
  le->startTime = cg.time;
  le->endTime = le->startTime + 5000 + random() * 5000;

  re->fadeStartTime = le->endTime - 1000;
  re->fadeEndTime = le->endTime;

  le->lifeRate = 1.0 / (le->endTime - le->startTime);
  le->leFlags = LEF_TUMBLE | LEF_TUMBLE_SLOW;
  le->bounceFactor = 0.4;
  le->leMarkType = LEMT_NONE;
  le->breakCount = 1;
  le->sizeScale = 0.5;

  VectorCopy(origin, re->origin);
  AxisCopy(axisDefault, re->axis);

  re->hModel = cgs.inlineDrawModel[cent->currentState.modelindex];

  le->pos.trType = TR_GRAVITY;

  VectorCopy(origin, le->pos.trBase);
  VectorCopy(dir, le->pos.trDelta);
  le->pos.trTime = cg.time;

  // rotation
  le->angles.trType = TR_LINEAR;
  le->angles.trTime = cg.time;
  le->angles.trBase[2] = rand() & 31;

  le->angles.trDelta[2] = (100 + (rand() & 500)) - 300;
  le->angles.trDelta[2] = (50 + (rand() & 400)) - 100;
  le->angles.trDelta[2] = (50 + (rand() & 400)) - 100;
}
// ===================

// void CG_BatDeath( centity_t *cent )
//{
//	CG_ParticleExplosion( "blood", cent->lerpOrigin, vec3_origin, 400, 20,
// 30, qfalse );
// }

void CG_MortarImpact(centity_t *cent, vec3_t origin, int sfx, qboolean dist) {
  if (sfx >= 0) {
    trap_S_StartSound(origin, -1, CHAN_AUTO, cgs.media.sfx_mortarexp[sfx]);
  }

  if (dist) {
    vec3_t gorg, norm;
    float gdist;

    VectorSubtract(origin, cg.refdef_current->vieworg, norm);
    gdist = VectorNormalize(norm);
    if (gdist > 1200 && gdist < 8000) // 1200 is max cam shakey dist (2*600) use
                                      // gorg as the new sound origin
    {
      VectorMA(cg.refdef_current->vieworg, 800, norm,
               gorg); // non-distance falloff makes more
                      // sense; sfx2range was gdist*0.2
      // sfx2range is variable to give us minimum volume
      // control different explosion sizes (see mortar,
      // panzerfaust, and grenade)
      trap_S_StartSoundEx(gorg, -1, CHAN_WEAPON, cgs.media.sfx_mortarexpDist,
                          SND_NOCUT);
    }

    if (cent->currentState.clientNum == cg.snap->ps.clientNum &&
        cg.mortarImpactTime != -2) {
      VectorCopy(origin, cg.mortarImpactPos);
      cg.mortarImpactTime = cg.time;
      cg.mortarImpactOutOfMap = qfalse;
    }
  }
}

void CG_MortarMiss(centity_t *cent, vec3_t origin) {
  if (cent->currentState.clientNum == cg.snap->ps.clientNum &&
      cg.mortarImpactTime != -2) {
    VectorCopy(origin, cg.mortarImpactPos);
    cg.mortarImpactTime = cg.time;
    if (cent->currentState.density) {
      cg.mortarImpactOutOfMap = qtrue;
    } else {
      cg.mortarImpactOutOfMap = qfalse;
    }
  }
}

/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
extern void CG_AddBulletParticles(vec3_t origin, vec3_t dir, int speed,
                                  int duration, int count, float randScale);
// JPW NERVE
void CG_MachineGunEjectBrass(centity_t *cent);
void CG_MachineGunEjectBrassNew(centity_t *cent);
// jpw

void CG_EntityEvent(centity_t *cent, vec3_t position) {
  entityState_t *es;
  int event;
  vec3_t dir;
  const char *s;
  int clientNum;
  char tempStr[MAX_QPATH];

  // JPW NERVE copied here for mg42 SFX event
  vec3_t porg, gorg, norm; // player/gun origin
  float gdist;
  // jpw

  static int footstepcnt = 0;
  static int splashfootstepcnt = 0;

  es = &cent->currentState;
  event = es->event & ~EV_EVENT_BITS;

  const auto debugPrint = [&] {
    const std::string entityType =
        es->eType < ET_EVENTS
            ? ETJump::stringFormat("^z[%s]", entityTypeNames[es->eType])
            : "^z[N/A]";
    const std::string eventName =
        ETJump::stringFormat("^z[%s]", eventnames[event]);
    CG_Printf("time: %i ent: %3i %-22s ^7event: %3i %-28s ^7eventParm: %3i\n",
              cg.time, es->number, entityType.c_str(), event, eventName.c_str(),
              es->eventParm);
  };

  if (!event) {
    if (cg_debugEvents.integer) {
      debugPrint();
    }

    return;
  }

  clientNum = es->clientNum;
  if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
    clientNum = 0;
  }

  // shift event numbers for 'EV_GENERAL_CLIENT_SOUND_VOLUME' due to it
  // getting placed in the middle of entity_events_t enum in 2.3.0
  if (ETJump::demoCompatibility->flags.adjustEvGeneralClientSoundVolume) {
    if (event == EV_GENERAL_CLIENT_SOUND_VOLUME) {
      event = EV_GLOBAL_SOUND;
    } else if (event > EV_GENERAL_CLIENT_SOUND_VOLUME) {
      event++;
    }
  }

  // adjust freestanding events to account for ET_TOKEN_EASY/MEDIUM/HARD,
  // ET_VELOCITY_PUSH_TRIGGER, ET_FAKEBRUSH and ET_TELEPORT_TRIGGER_CLIENT
  // freestanding events always have an eType > ET_EVENTS
  if ((ETJump::demoCompatibility->flags.adjustEvVelocityPushTrigger ||
       ETJump::demoCompatibility->flags.adjustEvFakebrushAndClientTeleporter ||
       ETJump::demoCompatibility->flags.adjustEvTokens) &&
      es->eType > ET_EVENTS) {
    event = ETJump::demoCompatibility->adjustedEventNum(event);
  }

  if (event == EV_CUSHIONFALLSTEP) {
    if (!etj_fixedCushionSteps.integer) {
      event = EV_FOOTSTEP;
    }
  }

  if (cg_debugEvents.integer) {
    debugPrint();
  }

  switch (event) {
    //
    // movement generated events
    //
    case EV_UPHILLSTEP:
      if (!etj_uphillSteps.integer) {
        break;
      }
      // fall through
    case EV_FOOTSTEP:
      if (es->eventParm != FOOTSTEP_TOTAL && cg_footsteps.integer) {
        if (es->eventParm) {
          trap_S_StartSoundVControl(
              nullptr, es->number, CHAN_BODY,
              cgs.media.footsteps[es->eventParm][footstepcnt],
              static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
        } else {
          bg_character_t *character =
              CG_CharacterForClientinfo(&cgs.clientinfo[clientNum], cent);
          trap_S_StartSoundVControl(
              nullptr, es->number, CHAN_BODY,
              cgs.media
                  .footsteps[character->animModelInfo->footsteps][footstepcnt],
              static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
        }
      }
      break;
    case EV_FOOTSPLASH:
      if (cg_footsteps.integer)
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_BODY,
            cgs.media.footsteps[FOOTSTEP_SPLASH][splashfootstepcnt],
            static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
      break;
    case EV_FOOTWADE:
      if (cg_footsteps.integer)
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_BODY,
            cgs.media.footsteps[FOOTSTEP_SPLASH][splashfootstepcnt],
            static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
      break;
    case EV_SWIM:
      if (cg_footsteps.integer)
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_BODY,
            cgs.media.footsteps[FOOTSTEP_SPLASH][footstepcnt],
            static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
      break;

    case EV_FALL_SHORT:
    case EV_CUSHIONFALLSTEP:
      if (es->eventParm != FOOTSTEP_TOTAL) {
        if (es->eventParm) {
          trap_S_StartSoundVControl(
              nullptr, es->number, CHAN_AUTO,
              cgs.media.landSound[es->eventParm],
              static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
        } else {
          bg_character_t *character =
              CG_CharacterForClientinfo(&cgs.clientinfo[clientNum], cent);
          trap_S_StartSoundVControl(
              nullptr, es->number, CHAN_AUTO,
              cgs.media.landSound[character->animModelInfo->footsteps],
              static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
        }
      }
      if (clientNum == cg.predictedPlayerState.clientNum) {
        // smooth landing z changes
        cg.landChange = -8;
        cg.landTime = cg.time;
      }
      break;

    case EV_FALL_DMG_10:
      if (!(cg.thisFrameTeleport || cg.nextFrameTeleport)) {
        if (es->eventParm != FOOTSTEP_TOTAL) {
          if (es->eventParm) {
            trap_S_StartSoundVControl(
                nullptr, es->number, CHAN_AUTO,
                cgs.media.landSound[es->eventParm],
                static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
          } else {
            bg_character_t *character =
                CG_CharacterForClientinfo(&cgs.clientinfo[clientNum], cent);
            trap_S_StartSoundVControl(
                nullptr, es->number, CHAN_AUTO,
                cgs.media.landSound[character->animModelInfo->footsteps],
                static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
          }
        }
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_AUTO, cgs.media.landHurt,
            static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
        cent->pe.painTime = cg.time; // don't play a pain sound right after this
        if (clientNum == cg.predictedPlayerState.clientNum) {
          // smooth landing z changes
          cg.landChange = -16;
          cg.landTime = cg.time;
        }
      }

      break;
    case EV_FALL_DMG_15:
      if (!(cg.thisFrameTeleport || cg.nextFrameTeleport)) {
        if (es->eventParm != FOOTSTEP_TOTAL) {
          if (es->eventParm) {
            trap_S_StartSoundVControl(
                nullptr, es->number, CHAN_AUTO,
                cgs.media.landSound[es->eventParm],
                static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
          } else {
            bg_character_t *character =
                CG_CharacterForClientinfo(&cgs.clientinfo[clientNum], cent);
            trap_S_StartSoundVControl(
                nullptr, es->number, CHAN_AUTO,
                cgs.media.landSound[character->animModelInfo->footsteps],
                static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
          }
        }
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_AUTO, cgs.media.landHurt,
            static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
        cent->pe.painTime = cg.time; // don't play a pain sound right after this
        if (clientNum == cg.predictedPlayerState.clientNum) {
          // smooth landing z changes
          cg.landChange = -16;
          cg.landTime = cg.time;
        }
      }

      break;
    case EV_FALL_DMG_25:
      if (!(cg.thisFrameTeleport || cg.nextFrameTeleport)) {
        if (es->eventParm != FOOTSTEP_TOTAL) {
          if (es->eventParm) {
            trap_S_StartSoundVControl(
                nullptr, es->number, CHAN_AUTO,
                cgs.media.landSound[es->eventParm],
                static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
          } else {
            bg_character_t *character =
                CG_CharacterForClientinfo(&cgs.clientinfo[clientNum], cent);
            trap_S_StartSoundVControl(
                nullptr, es->number, CHAN_AUTO,
                cgs.media.landSound[character->animModelInfo->footsteps],
                static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
          }
        }
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_AUTO, cgs.media.landHurt,
            static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
        cent->pe.painTime = cg.time; // don't play a pain sound right after this
        if (clientNum == cg.predictedPlayerState.clientNum) {
          // smooth landing z changes
          cg.landChange = -24;
          cg.landTime = cg.time;
        }
      }

      break;
    case EV_FALL_DMG_50:
      if (!(cg.thisFrameTeleport || cg.nextFrameTeleport)) {
        if (es->eventParm != FOOTSTEP_TOTAL) {
          if (es->eventParm) {
            trap_S_StartSoundVControl(
                nullptr, es->number, CHAN_AUTO,
                cgs.media.landSound[es->eventParm],
                static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
          } else {
            bg_character_t *character =
                CG_CharacterForClientinfo(&cgs.clientinfo[clientNum], cent);
            trap_S_StartSoundVControl(
                nullptr, es->number, CHAN_AUTO,
                cgs.media.landSound[character->animModelInfo->footsteps],
                static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
          }
        }
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_AUTO, cgs.media.landHurt,
            static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
        cent->pe.painTime = cg.time; // don't play a pain sound right after this
        if (clientNum == cg.predictedPlayerState.clientNum) {
          // smooth landing z changes
          cg.landChange = -24;
          cg.landTime = cg.time;
        }
      }

      break;
    case EV_FALL_NDIE:
      if (!(cg.thisFrameTeleport || cg.nextFrameTeleport)) {
        if (es->eventParm != FOOTSTEP_TOTAL) {
          if (es->eventParm) {
            trap_S_StartSoundVControl(
                nullptr, es->number, CHAN_AUTO,
                cgs.media.landSound[es->eventParm],
                static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
          } else {
            bg_character_t *character =
                CG_CharacterForClientinfo(&cgs.clientinfo[clientNum], cent);
            trap_S_StartSoundVControl(
                nullptr, es->number, CHAN_AUTO,
                cgs.media.landSound[character->animModelInfo->footsteps],
                static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
          }
        }
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_AUTO, cgs.media.landHurt,
            static_cast<int>(DEFAULT_VOLUME * etj_footstepVolume.value));
        cent->pe.painTime = cg.time; // don't play a pain sound
                                     // right after this splat
      }

      break;

    case EV_EXERT1:
      trap_S_StartSound(nullptr, es->number, CHAN_VOICE,
                        CG_CustomSound(es->number, "*exert1.wav"));
      break;
    case EV_EXERT2:
      trap_S_StartSound(nullptr, es->number, CHAN_VOICE,
                        CG_CustomSound(es->number, "*exert2.wav"));
      break;
    case EV_EXERT3:
      trap_S_StartSound(nullptr, es->number, CHAN_VOICE,
                        CG_CustomSound(es->number, "*exert3.wav"));
      break;

    case EV_STEP_4:
    case EV_STEP_8:
    case EV_STEP_12:
    case EV_STEP_16: // smooth out step up transitions
    {
      float oldStep;
      int delta;
      int step;

      if (clientNum != cg.predictedPlayerState.clientNum) {
        break;
      }
      // if we are interpolating, we don't need to smooth steps
      if (cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
          cg_nopredict.integer
#ifdef ALLOW_GSYNC
          || cgs.synchronousClients
#endif // ALLOW_GSYNC
      ) {
        break;
      }
      // check for stepping up before a previous step is completed
      delta = cg.time - cg.stepTime;
      if (delta < STEP_TIME) {
        oldStep =
            cg.stepChange * (static_cast<float>(STEP_TIME - delta)) / STEP_TIME;
      } else {
        oldStep = 0;
      }

      // add this amount
      step = 4 * (event - EV_STEP_4 + 1);
      cg.stepChange = oldStep + static_cast<float>(step);
      if (cg.stepChange > MAX_STEP_CHANGE) {
        cg.stepChange = MAX_STEP_CHANGE;
      }
      cg.stepTime = cg.time;
      break;
    }

    case EV_JUMP:
      VectorCopy(cent->lerpOrigin, cg.etjLastJumpPos);
      trap_S_StartSound(nullptr, es->number, CHAN_VOICE,
                        CG_CustomSound(es->number, "*jump1.wav"));
      if (clientNum == cg.predictedPlayerState.clientNum) {
        ETJump::entityEventsHandler->check(EV_JUMP, cent);
      }
      break;
    case EV_TAUNT:
      trap_S_StartSound(nullptr, es->number, CHAN_VOICE,
                        CG_CustomSound(es->number, "*taunt.wav"));
      break;
    case EV_WATER_TOUCH:
      trap_S_StartSound(nullptr, es->number, CHAN_AUTO, cgs.media.watrInSound);
      break;
    case EV_WATER_LEAVE:
      trap_S_StartSound(nullptr, es->number, CHAN_AUTO, cgs.media.watrOutSound);
      break;
    case EV_WATER_UNDER:
      trap_S_StartSound(nullptr, es->number, CHAN_AUTO, cgs.media.watrUnSound);
      if (cg.clientNum == es->number) {
        cg.waterundertime = cg.time + HOLDBREATHTIME;
      }
      break;
    case EV_WATER_CLEAR:
      // trap_S_StartSound (NULL, es->number, CHAN_AUTO,
      // CG_CustomSound( es->number, "*gasp.wav" ) );
      trap_S_StartSound(nullptr, es->number, CHAN_AUTO, cgs.media.watrOutSound);
      if (es->eventParm) {
        trap_S_StartSound(nullptr, es->number, CHAN_AUTO,
                          cgs.media.watrGaspSound);
      }
      break;

    case EV_ITEM_PICKUP:
    case EV_ITEM_PICKUP_QUIET: {
      gitem_t *item;
      int index;

      index = es->eventParm; // player predicted

      if (index < 1 || index >= bg_numItems) {
        break;
      }
      item = &bg_itemlist[index];

      if (event == EV_ITEM_PICKUP) // not quiet
      {
        // powerups and team items will have a separate global sound,
        // this one will be played at prediction time
        if (item->giType == IT_TEAM) {
          trap_S_StartSound(
              nullptr, es->number, CHAN_AUTO,
              trap_S_RegisterSound("sound/misc/w_pkup.wav", qfalse));
        } else {
          trap_S_StartSound(nullptr, es->number, CHAN_AUTO,
                            trap_S_RegisterSound(item->pickup_sound, qfalse));
        }
      }

      // show icon and name on status bar
      if (es->number == cg.snap->ps.clientNum) {
        CG_ItemPickup(index);
      }
    } break;

    case EV_GLOBAL_ITEM_PICKUP: {
      gitem_t *item;
      int index;

      index = es->eventParm; // player predicted

      if (index < 1 || index >= bg_numItems) {
        break;
      }
      item = &bg_itemlist[index];
      if (*item->pickup_sound) {
        // powerup pickups are global
        trap_S_StartSound(nullptr, cg.snap->ps.clientNum, CHAN_AUTO,
                          trap_S_RegisterSound(item->pickup_sound,
                                               qfalse)); // FIXME: precache
      }

      // show icon and name on status bar
      if (es->number == cg.snap->ps.clientNum) {
        CG_ItemPickup(index);
      }
    } break;

    //
    // weapon events
    //
    case EV_VENOM:
      break;

    case EV_WEAP_OVERHEAT:
      // start weapon idle animation
      if (es->number == cg.snap->ps.clientNum) {
        cg.predictedPlayerState.weapAnim =
            ((cg.predictedPlayerState.weapAnim & ANIM_TOGGLEBIT) ^
             ANIM_TOGGLEBIT) |
            PM_IdleAnimForWeapon(cg.snap->ps.weapon);
        cent->overheatTime = cg.time; // used to make the barrels
                                      // smoke when overheated
      }

      if (BG_PlayerMounted(es->eFlags)) {
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_AUTO, cgs.media.hWeaponHeatSnd,
            static_cast<int>(255 * etj_weaponVolume.value));
      } else if (cg_weapons[es->weapon].overheatSound) {
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_AUTO,
            cg_weapons[es->weapon].overheatSound,
            static_cast<int>(DEFAULT_VOLUME * etj_weaponVolume.value));
      }
      break;

      // JPW NERVE
    case EV_SPINUP:
      trap_S_StartSoundVControl(
          nullptr, es->number, CHAN_AUTO, cg_weapons[es->weapon].spinupSound,
          static_cast<int>(DEFAULT_VOLUME * etj_weaponVolume.value));
      break;
      // jpw
    case EV_EMPTYCLIP:
      break;

    case EV_FILL_CLIP:
      if (cgs.clientinfo[cg.clientNum].skill[SK_LIGHT_WEAPONS] >= 2 &&
          BG_isLightWeaponSupportingFastReload(es->weapon) &&
          cg_weapons[es->weapon].reloadFastSound) {
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_WEAPON,
            cg_weapons[es->weapon].reloadFastSound,
            static_cast<int>(DEFAULT_VOLUME * etj_weaponVolume.value));
      } else if (cg_weapons[es->weapon].reloadSound) {
        // JPW NERVE following sherman's SP fix,
        // should allow killing reload sound when player dies
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_WEAPON,
            cg_weapons[es->weapon].reloadSound,
            static_cast<int>(DEFAULT_VOLUME * etj_weaponVolume.value));
      }
      break;

      // JPW NERVE play a sound when engineer fixes MG42
    case EV_MG42_FIXED:
      break;
      // jpw

    case EV_NOAMMO:
    case EV_WEAPONSWITCHED:
      if ((es->weapon != WP_GRENADE_LAUNCHER) &&
          (es->weapon != WP_GRENADE_PINEAPPLE) && (es->weapon != WP_DYNAMITE) &&
          (es->weapon != WP_LANDMINE) && (es->weapon != WP_SATCHEL) &&
          (es->weapon != WP_SATCHEL_DET) && (es->weapon != WP_TRIPMINE) &&
          (es->weapon != WP_SMOKE_BOMB) && (es->weapon != WP_AMMO) &&
          (es->weapon != WP_MEDKIT)) // Feen: PGM
      {
        trap_S_StartSoundVControl(
            nullptr, es->number, CHAN_AUTO, cgs.media.noAmmoSound,
            static_cast<int>(DEFAULT_VOLUME * etj_weaponVolume.value));
      }

      if (es->number == cg.snap->ps.clientNum &&
          ((cg_noAmmoAutoSwitch.integer > 0 &&
            !CG_WeaponSelectable(cg.weaponSelect)) ||
           es->weapon == WP_MORTAR_SET || es->weapon == WP_MOBILE_MG42_SET ||
           es->weapon == WP_GRENADE_LAUNCHER ||
           es->weapon == WP_GRENADE_PINEAPPLE || es->weapon == WP_DYNAMITE ||
           es->weapon == WP_SMOKE_MARKER || es->weapon == WP_PANZERFAUST ||
           es->weapon == WP_ARTY || es->weapon == WP_LANDMINE ||
           es->weapon == WP_SATCHEL || es->weapon == WP_SATCHEL_DET ||
           es->weapon == WP_TRIPMINE || es->weapon == WP_SMOKE_BOMB ||
           es->weapon == WP_AMMO || es->weapon == WP_MEDKIT)) {
        CG_OutOfAmmoChange(event == EV_WEAPONSWITCHED ? qfalse : qtrue);
      }
      break;
    case EV_CHANGE_WEAPON:
      trap_S_StartSoundVControl(
          nullptr, es->number, CHAN_AUTO, cgs.media.selectSound,
          static_cast<int>(DEFAULT_VOLUME * etj_weaponVolume.value));
      break;
    case EV_CHANGE_WEAPON_2:
      trap_S_StartSoundVControl(
          nullptr, es->number, CHAN_AUTO, cgs.media.selectSound,
          static_cast<int>(DEFAULT_VOLUME * etj_weaponVolume.value));

      if (es->number == cg.snap->ps.clientNum) {
        int newweap = 0;

        // client will get this message if reloading
        // while using an alternate weapon
        // client should voluntarily switch back to primary at that point
        switch (es->weapon) {
          case WP_FG42SCOPE:
            newweap = WP_FG42;
            break;
          case WP_GARAND_SCOPE:
            newweap = WP_GARAND;
            break;
          case WP_K43_SCOPE:
            newweap = WP_K43;
            break;
          default:
            break;
        }

        if (newweap) {
          CG_FinishWeaponChange(es->weapon, newweap);
        }
      }
      break;

    case EV_FIRE_WEAPON_MOUNTEDMG42:
    case EV_FIRE_WEAPON_MG42:
      VectorCopy(cent->currentState.pos.trBase, gorg);
      VectorCopy(cg.refdef_current->vieworg, porg);
      VectorSubtract(gorg, porg, norm);
      gdist = VectorNormalize(norm);
      if (gdist > 512 && gdist < 4096) {
        VectorMA(cg.refdef_current->vieworg, 64, norm, gorg);
        // should we use a browning?
        if (cg_entities
                [cg_entities[cg_entities[cent->currentState.number].tagParent]
                     .tankparent]
                    .currentState.density &
            8) {
          trap_S_StartSoundExVControl(
              gorg, cent->currentState.number, CHAN_WEAPON,
              cgs.media.hWeaponEchoSnd_2, SND_NOCUT,
              static_cast<int>(DEFAULT_VOLUME * etj_weaponVolume.value));
        } else {
          trap_S_StartSoundExVControl(
              gorg, cent->currentState.number, CHAN_WEAPON,
              cgs.media.hWeaponEchoSnd, SND_NOCUT,
              static_cast<int>(DEFAULT_VOLUME * etj_weaponVolume.value));
        }
      }
      CG_FireWeapon(cent);
      break;
    case EV_FIRE_WEAPON_AAGUN:
      CG_FireWeapon(cent);
      break;
    case EV_FIRE_WEAPON:
    case EV_FIRE_WEAPONB:
      if (cent->currentState.clientNum == cg.snap->ps.clientNum &&
          cg.snap->ps.eFlags & EF_ZOOMING) // to stop airstrike sfx
      {
        break;
      }
      CG_FireWeapon(cent);
      if (event == EV_FIRE_WEAPONB) // akimbo firing
      {
        cent->akimboFire = qtrue;
      } else {
        cent->akimboFire = qfalse;
      }
      break;
    case EV_FIRE_WEAPON_LASTSHOT:
      CG_FireWeapon(cent);
      break;

    case EV_NOFIRE_UNDERWATER:
      if (cgs.media.noFireUnderwater) {
        trap_S_StartSound(nullptr, es->number, CHAN_WEAPON,
                          cgs.media.noFireUnderwater);
      }
      break;

    case EV_PLAYER_TELEPORT_IN:
    case EV_PLAYER_TELEPORT_OUT:
    case EV_ITEM_POP:
    case EV_ITEM_RESPAWN:
      break;

    case EV_GRENADE_BOUNCE:
      // DYNAMITE
      // Gordon: or LANDMINE FIXME: change this? (mebe a metallic sound)
      if (es->weapon == WP_SATCHEL) {
        trap_S_StartSound(nullptr, es->number, CHAN_AUTO,
                          cgs.media.satchelbounce1);
      } else if (es->weapon == WP_DYNAMITE) {
        trap_S_StartSound(nullptr, es->number, CHAN_AUTO,
                          cgs.media.dynamitebounce1);
      } else if (es->weapon == WP_LANDMINE) {
        trap_S_StartSound(nullptr, es->number, CHAN_AUTO,
                          cgs.media.landminebounce1);
      } else {
        // GRENADES
        if (es->eventParm != FOOTSTEP_TOTAL) {
          if (rand() & 1) {
            trap_S_StartSound(nullptr, es->number, CHAN_AUTO,
                              cgs.media.grenadebounce[es->eventParm][0]);
          } else {
            trap_S_StartSound(nullptr, es->number, CHAN_AUTO,
                              cgs.media.grenadebounce[es->eventParm][1]);
          }
        }
      }
      break;
    case EV_RAILTRAIL: {
      vec3_t color = {es->angles[0], es->angles[1], es->angles[2]};

      // red is default if there is no color set
      if (color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f) {
        color[0] = 1.0f;
        color[1] = 0.0f;
        color[2] = 0.0f;
      }
      CG_RailTrail(&cgs.clientinfo[es->otherEntityNum2], es->origin2,
                   es->pos.trBase, es->dmgFlags,
                   color); //----(SA)	added'type' field
    } break;

    //
    // missile impacts
    //
    case EV_MISSILE_HIT:
      ByteToDir(es->eventParm, dir);
      CG_MissileHitPlayer(cent, es->weapon, position, dir, es->otherEntityNum);
      if (es->weapon == WP_MORTAR_SET) {
        if (!es->legsAnim) {
          CG_MortarImpact(cent, position, 3, qtrue);
        } else {
          CG_MortarImpact(cent, position, -1, qtrue);
        }
      }
      break;

    case EV_MISSILE_MISS_SMALL:
      ByteToDir(es->eventParm, dir);
      CG_MissileHitWallSmall(es->weapon, 0, position, dir);
      break;

    case EV_MISSILE_MISS:
      ByteToDir(es->eventParm, dir);
      CG_MissileHitWall(es->weapon, 0, position, dir,
                        0); // (SA) modified to send missilehitwall
                            // surface parameters
      if (es->weapon == WP_MORTAR_SET) {
        if (!es->legsAnim) {
          CG_MortarImpact(cent, position, 3, qtrue);
        } else {
          CG_MortarImpact(cent, position, -1, qtrue);
        }
      }
      break;

    case EV_MISSILE_MISS_LARGE:
      ByteToDir(es->eventParm, dir);
      if (es->weapon == WP_ARTY || es->weapon == WP_SMOKE_MARKER) {
        CG_MissileHitWall(es->weapon, 0, position, dir,
                          0); // (SA) modified to send
                              // missilehitwall surface
                              // parameters
      } else {
        CG_MissileHitWall(VERYBIGEXPLOSION, 0, position, dir,
                          0); // (SA) modified to send
                              // missilehitwall surface
                              // parameters
      }
      break;

    case EV_MORTAR_IMPACT:
      CG_MortarImpact(cent, position, rand() % 3, qfalse);
      break;
    case EV_MORTAR_MISS:
      CG_MortarMiss(cent, position);
      break;

    case EV_MG42BULLET_HIT_WALL:
      ByteToDir(es->eventParm, dir);
      CG_Bullet(es->pos.trBase, es->otherEntityNum, dir, qfalse,
                ENTITYNUM_WORLD, es->otherEntityNum2, es->origin2[0],
                es->effect1Time);
      break;

    case EV_MG42BULLET_HIT_FLESH:
      CG_Bullet(es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm,
                es->otherEntityNum2, 0, es->effect1Time);
      break;

    case EV_BULLET_HIT_WALL:
      ByteToDir(es->eventParm, dir);
      CG_Bullet(es->pos.trBase, es->otherEntityNum, dir, qfalse,
                ENTITYNUM_WORLD, es->otherEntityNum2, es->origin2[0], 0);
      break;

    case EV_BULLET_HIT_FLESH:
      CG_Bullet(es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm,
                es->otherEntityNum2, 0, 0);
      break;

    case EV_POPUPBOOK:
    case EV_POPUP:
    case EV_GIVEPAGE:
      break;
    case EV_GENERAL_SOUND:
      // Ridah, check for a sound script
      s = CG_ConfigString(CS_SOUNDS + es->eventParm);
      if (!strstr(s, ".wav")) {
        if (CG_SoundPlaySoundScript(s, nullptr, es->number, qfalse)) {
          break;
        }
        // try with .wav
        Q_strncpyz(tempStr, s, sizeof(tempStr));
        Q_strcat(tempStr, sizeof(tempStr), ".wav");
        s = tempStr;
      }

      // done.
      if (cgs.gameSounds[es->eventParm]) {
        // xkan, 10/31/2002 - crank up the volume
        trap_S_StartSoundVControl(nullptr, es->number, CHAN_VOICE,
                                  cgs.gameSounds[es->eventParm], 255);
      } else {
        s = CG_ConfigString(CS_SOUNDS + es->eventParm);
        // xkan, 10/31/2002 - crank up the volume
        trap_S_StartSoundVControl(nullptr, es->number, CHAN_VOICE,
                                  CG_CustomSound(es->number, s), 255);
      }
      break;

    case EV_FX_SOUND: {
      sfxHandle_t sound;

      sound = static_cast<int>(random()) * fxSounds[es->eventParm].max;

      if (fxSounds[es->eventParm].sound[sound] == -1) {
        fxSounds[es->eventParm].sound[sound] = trap_S_RegisterSound(
            fxSounds[es->eventParm].soundfile[sound], qfalse);
      }

      sound = fxSounds[es->eventParm].sound[sound];

      trap_S_StartSoundVControl(nullptr, es->number, CHAN_VOICE, sound, 255);
    } break;
    case EV_GENERAL_CLIENT_SOUND_VOLUME:
      if (cg.snap->ps.clientNum != es->teamNum) {
        break;
      }
      // fall through
    case EV_GENERAL_SOUND_VOLUME: {
      int sound = es->eventParm;
      int volume = es->onFireStart;

      // Ridah, check for a sound script
      s = CG_ConfigString(CS_SOUNDS + sound);
      if (!strstr(s, ".wav")) {
        if (CG_SoundPlaySoundScript(s, nullptr, es->number, qfalse)) {
          break;
        }
        // try with .wav
        Q_strncpyz(tempStr, s, sizeof(tempStr));
        Q_strcat(tempStr, sizeof(tempStr), ".wav");
        s = tempStr;
      }
      // done.
      if (cgs.gameSounds[sound]) {
        trap_S_StartSoundVControl(nullptr, es->number, CHAN_VOICE,
                                  cgs.gameSounds[sound], volume);
      } else {
        s = CG_ConfigString(CS_SOUNDS + sound);
        trap_S_StartSoundVControl(nullptr, es->number, CHAN_VOICE,
                                  CG_CustomSound(es->number, s), volume);
      }
    } break;

    case EV_GLOBAL_TEAM_SOUND:
      if (cgs.clientinfo[cg.snap->ps.clientNum].team != es->teamNum) {
        break;
      }
      // fall through
    case EV_GLOBAL_SOUND: // play from the player's head so it never diminishes
      // Ridah, check for a sound script
      s = CG_ConfigString(CS_SOUNDS + es->eventParm);
      if (!strstr(s, ".wav")) {
        if (CG_SoundPlaySoundScript(s, nullptr, -1, qtrue)) {
          break;
        }

        // try with .wav
        Q_strncpyz(tempStr, s, sizeof(tempStr));
        Q_strcat(tempStr, sizeof(tempStr), ".wav");
        s = tempStr;
      }

      if (cgs.gameSounds[es->eventParm]) {
        if (cgs.demoCam.renderingFreeCam) {
          trap_S_StartLocalSound(cgs.gameSounds[es->eventParm], CHAN_AUTO);
        } else {
          trap_S_StartSound(nullptr, cg.snap->ps.clientNum, CHAN_AUTO,
                            cgs.gameSounds[es->eventParm]);
        }
      } else {
        s = CG_ConfigString(CS_SOUNDS + es->eventParm);
        if (cgs.demoCam.renderingFreeCam) {
          trap_S_StartLocalSound(CG_CustomSound(es->number, s), CHAN_AUTO);
        } else {
          trap_S_StartSound(nullptr, cg.snap->ps.clientNum, CHAN_AUTO,
                            CG_CustomSound(es->number, s));
        }
      }
      break;

    // DHM - Nerve
    case EV_GLOBAL_CLIENT_SOUND:
      if (cg.snap->ps.clientNum == es->teamNum) {
        s = CG_ConfigString(CS_SOUNDS + es->eventParm);
        if (!strstr(s, ".wav")) {
          if (CG_SoundPlaySoundScript(s, nullptr, -1,
                                      (es->effect1Time ? qfalse : qtrue))) {
            break;
          }
          // try with .wav
          Q_strncpyz(tempStr, s, sizeof(tempStr));
          Q_strcat(tempStr, sizeof(tempStr), ".wav");
          s = tempStr;
        }
        // done.
        if (cgs.gameSounds[es->eventParm]) {
          trap_S_StartSound(nullptr, cg.snap->ps.clientNum, CHAN_AUTO,
                            cgs.gameSounds[es->eventParm]);
        } else {
          s = CG_ConfigString(CS_SOUNDS + es->eventParm);
          trap_S_StartSound(nullptr, cg.snap->ps.clientNum, CHAN_AUTO,
                            CG_CustomSound(es->number, s));
        }
      }

      break;
      // dhm - end

    case EV_PAIN:
      // local player sounds are triggered in CG_CheckLocalSounds,
      // so ignore events on the player
      if (cent->currentState.number != cg.snap->ps.clientNum) {
        CG_PainEvent(cent, es->eventParm, qfalse);
      }
      break;

    case EV_CROUCH_PAIN:
      // local player sounds are triggered in CG_CheckLocalSounds,
      // so ignore events on the player
      if (cent->currentState.number != cg.snap->ps.clientNum) {
        CG_PainEvent(cent, es->eventParm, qtrue);
      }
      break;

    case EV_DEATH1:
    case EV_DEATH2:
    case EV_DEATH3:
      trap_S_StartSound(nullptr, es->number, CHAN_VOICE,
                        CG_CustomSound(es->number, va("*death%i.wav",
                                                      event - EV_DEATH1 + 1)));
      break;

    case EV_OBITUARY:
      break;

    // JPW NERVE -- swiped from SP/Sherman
    case EV_STOPSTREAMINGSOUND:
      // kill weapon sound (could be reloading)
      trap_S_StartSoundEx(nullptr, es->number, CHAN_WEAPON, 0, SND_CUTOFF_ALL);
      break;

    case EV_LOSE_HAT:
      ByteToDir(es->eventParm, dir);
      CG_LoseHat(cent, dir);
      break;

    case EV_GIB_PLAYER:
      trap_S_StartSound(es->pos.trBase, -1, CHAN_AUTO, cgs.media.gibSound);
      ByteToDir(es->eventParm, dir);
      CG_GibPlayer(cent, cent->lerpOrigin, dir);
      break;

    case EV_STOPLOOPINGSOUND:
      es->loopSound = 0;
      break;

    case EV_DEBUG_LINE:
      CG_Beam(cent);
      break;

    // Rafael particles
    case EV_SMOKE:
      if (cent->currentState.density == 3) {
        CG_ParticleSmoke(cgs.media.smokePuffShaderdirty, cent);
      } else {
        CG_ParticleSmoke(cgs.media.smokePuffShader, cent);
      }
      break;

    case EV_FLAMETHROWER_EFFECT:
      CG_FireFlameChunks(cent, cent->currentState.origin,
                         cent->currentState.apos.trBase, 0.6, qtrue);
      break;

    case EV_DUST:
      CG_ParticleDust(cent, cent->currentState.origin,
                      cent->currentState.angles);
      break;

    case EV_RUMBLE_EFX: {
      float pitch, yaw;
      pitch = cent->currentState.angles[0];
      yaw = cent->currentState.angles[1];
      CG_RumbleEfx(pitch, yaw);
    } break;

    case EV_CONCUSSIVE:
      CG_Concussive(cent);
      break;

    case EV_EMITTER: {
      localEntity_t *le;
      le = CG_AllocLocalEntity();
      le->leType = LE_EMITTER;
      le->startTime = cg.time;
      le->endTime = le->startTime + 20000;
      le->pos.trType = TR_STATIONARY;
      VectorCopy(cent->currentState.origin, le->pos.trBase);
      VectorCopy(cent->currentState.origin2, le->angles.trBase);
      le->ownerNum = 0;
    } break;

    case EV_OILPARTICLES:
      CG_Particle_OilParticle(cgs.media.oilParticle, cent->currentState.origin,
                              cent->currentState.origin2,
                              cent->currentState.time,
                              cent->currentState.density);
      break;
    case EV_OILSLICK:
      CG_Particle_OilSlick(cgs.media.oilSlick, cent);
      break;
    case EV_OILSLICKREMOVE:
      CG_OilSlickRemove(cent);
      break;

    case EV_MG42EFX:
      CG_MG42EFX(cent);
      break;

    case EV_SPARKS_ELECTRIC:
    case EV_SPARKS: {
      int numsparks;
      int i;
      int duration;
      float x, y;
      float speed;
      vec3_t source, dest;

      if (!(cent->currentState.density)) {
        cent->currentState.density = 1;
      }
      numsparks = rand() % cent->currentState.density;
      duration = cent->currentState.frame;
      x = cent->currentState.angles2[0];
      y = cent->currentState.angles2[1];
      speed = cent->currentState.angles2[2];

      if (!numsparks) {
        numsparks = 1;
      }
      for (i = 0; i < numsparks; i++) {

        if (event == EV_SPARKS_ELECTRIC) {
          VectorCopy(cent->currentState.origin, source);

          VectorCopy(source, dest);
          dest[0] += static_cast<float>((rand() & 31) - 16);
          dest[1] += static_cast<float>((rand() & 31) - 16);
          dest[2] += static_cast<float>((rand() & 31) - 16);

          CG_Tracer(source, dest, 1);
        } else {
          CG_ParticleSparks(cent->currentState.origin,
                            cent->currentState.angles, duration, x, y, speed);
        }
      }

    } break;

    case EV_GUNSPARKS: {
      int numsparks;
      int speed;
      // int	count;

      numsparks = cent->currentState.density;
      speed = static_cast<int>(cent->currentState.angles2[2]);

      CG_AddBulletParticles(cent->currentState.origin,
                            cent->currentState.angles, speed, 800, numsparks,
                            1.0f);

    } break;

    // Rafael snow pvs check
    case EV_SNOW_ON:
      CG_SnowLink(cent, qtrue);
      break;

    case EV_SNOW_OFF:
      CG_SnowLink(cent, qfalse);
      break;

    case EV_SNOWFLURRY:
      CG_ParticleSnowFlurry(cgs.media.snowShader, cent);
      break;

    // for func_exploding
    case EV_EXPLODE:
      ByteToDir(es->eventParm, dir);
      CG_Explode(cent, position, dir, 0);
      break;

    case EV_RUBBLE:
      ByteToDir(es->eventParm, dir);
      CG_Rubble(cent, position, dir, 0);
      break;

    // for target_effect
    case EV_EFFECT:
      ByteToDir(es->eventParm, dir);
      CG_Effect(cent, position, dir);
      break;

    case EV_MORTAREFX: // mortar firing
      CG_MortarEFX(cent);
      break;

    case EV_SHARD:
      ByteToDir(es->eventParm, dir);
      CG_Shard(cent, position, dir);
      break;

    case EV_JUNK:
      ByteToDir(es->eventParm, dir);
      {
        int i;
        int rval;

        rval = rand() % 3 + 3;

        for (i = 0; i < rval; i++) CG_ShardJunk(cent, position, dir);
      }
      break;

    case EV_DISGUISE_SOUND:
      trap_S_StartSound(nullptr, cent->currentState.number, CHAN_WEAPON,
                        cgs.media.uniformPickup);
      break;
    case EV_BUILDDECAYED_SOUND:
      trap_S_StartSound(cent->lerpOrigin, cent->currentState.number, CHAN_AUTO,
                        cgs.media.buildDecayedSound);
      break;

    // Gordon: debris test
    case EV_DEBRIS:
      CG_Debris(cent, position, cent->currentState.origin2);
      break;
      // ===================

    case EV_SHAKE: {
      vec3_t v;
      float len;

      if (cgs.demoCam.renderingFreeCam) {
        break;
      }

      VectorSubtract(cg.snap->ps.origin, cent->lerpOrigin, v);
      len = VectorLength(v);

      if (len > static_cast<float>(cent->currentState.onFireStart)) {
        break;
      }

      if (cent->currentState.onFireStart != 0) {
        len = 1.0f - len / static_cast<float>(cent->currentState.onFireStart);
      }
      len = std::min(1.f, len);

      CG_StartShakeCamera(len, es);
    }

    break;

    case EV_ALERT_SPEAKER:
      switch (cent->currentState.otherEntityNum2) {
        case 1:
          CG_UnsetActiveOnScriptSpeaker(cent->currentState.otherEntityNum);
          break;
        case 2:
          CG_SetActiveOnScriptSpeaker(cent->currentState.otherEntityNum);
          break;
        case 0:
        default:
          CG_ToggleActiveOnScriptSpeaker(cent->currentState.otherEntityNum);
          break;
      }
      break;

    case EV_POPUPMESSAGE: {
      const char *str = CG_GetPMItemText(cent);
      qhandle_t shader = CG_GetPMItemIcon(cent);
      if (str) {
        CG_AddPMItem(
            static_cast<popupMessageType_t>(cent->currentState.effect1Time),
            str, shader);
      }
      CG_PlayPMItemSound(cent);
    } break;

    case EV_AIRSTRIKEMESSAGE: {
      const char *wav = nullptr;

      if (cgs.demoCam.renderingFreeCam) {
        break;
      }

      switch (cent->currentState.density) {
        case 0: // too many called
          if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS) {
            wav = "axis_hq_airstrike_denied";
          } else {
            wav = "allies_hq_airstrike_denied";
          }
          break;
        case 1: // aborting can't see target
          if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS) {
            wav = "axis_hq_airstrike_abort";
          } else {
            wav = "allies_hq_airstrike_abort";
          }
          break;
        case 2: // firing for effect
          if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS) {
            wav = "axis_hq_airstrike";
          } else {
            wav = "allies_hq_airstrike";
          }
          break;
      }

      if (wav) {
        CG_SoundPlaySoundScript(wav, nullptr, -1,
                                (es->effect1Time ? qfalse : qtrue));
      }
    } break;

    case EV_ARTYMESSAGE: {
      const char *wav = nullptr;

      if (cgs.demoCam.renderingFreeCam) {
        break;
      }

      switch (cent->currentState.density) {
        case 0: // too many called
          if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS) {
            wav = "axis_hq_ffe_denied";
          } else {
            wav = "allies_hq_ffe_denied";
          }
          break;
        case 1: // aborting can't see target
          if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS) {
            wav = "axis_hq_ffe_abort";
          } else {
            wav = "allies_hq_ffe_abort";
          }
          break;
        case 2: // firing for effect
          if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS) {
            wav = "axis_hq_ffe";
          } else {
            wav = "allies_hq_ffe";
          }
          break;
      }

      if (wav) {
        CG_SoundPlaySoundScript(wav, nullptr, -1,
                                (es->effect1Time ? qfalse : qtrue));
      }
    } break;

    case EV_MEDIC_CALL:
      switch (cgs.clientinfo[cent->currentState.number].team) {
        case TEAM_AXIS:
          trap_S_StartSound(nullptr, cent->currentState.number, CHAN_AUTO,
                            cgs.media.sndMedicCall[0]);
          break;
        case TEAM_ALLIES:
          trap_S_StartSound(nullptr, cent->currentState.number, CHAN_AUTO,
                            cgs.media.sndMedicCall[1]);
          break;
        default: // shouldn't happen
          break;
      }

      break;

    case EV_PORTAL_TELEPORT:
      break;

    case EV_LOAD_TELEPORT:
      if (es->clientNum != cg.snap->ps.clientNum) {
        return;
      }

      // should refactor users to playereventshandler
      ETJump::entityEventsHandler->check(EV_LOAD_TELEPORT, cent);
      ETJump::playerEventsHandler->check("load", {});
      trap_SendConsoleCommand("resetJumpSpeeds\n");
      trap_SendConsoleCommand("resetStrafeQuality\n");
      trap_SendConsoleCommand("resetUpmoveMeter\n");
      CG_ResetTransitionEffects();
      break;
    case EV_PORTAL_TRAIL:
      // not our portal trail
      if (!etj_viewPlayerPortals.integer &&
          es->otherEntityNum2 != cg.clientNum) {
        return;
      }
      // not our portal trail, not spectating
      if (etj_viewPlayerPortals.integer == 2 &&
          cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR &&
          es->otherEntityNum2 != cg.clientNum) {
        return;
      }

      CG_RailTrail(&cgs.clientinfo[es->otherEntityNum2], es->origin2,
                   es->pos.trBase, es->dmgFlags,
                   tv(es->angles[0], es->angles[1], es->angles[2]));
      break;
    default:
      break;
  }
  {
    int rval;

    rval = rand() & 3;

    if (splashfootstepcnt != rval) {
      splashfootstepcnt = rval;
    } else {
      splashfootstepcnt++;
    }

    if (splashfootstepcnt > 3) {
      splashfootstepcnt = 0;
    }

    if (footstepcnt != rval) {
      footstepcnt = rval;
    } else {
      footstepcnt++;
    }

    if (footstepcnt > 3) {
      footstepcnt = 0;
    }
  }
}

/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents(centity_t *cent) {
  int i, event;

  // calculate the position at exactly the frame time
  BG_EvaluateTrajectory(&cent->currentState.pos, cg.snap->serverTime,
                        cent->lerpOrigin, qfalse,
                        cent->currentState.effect2Time);
  CG_SetEntitySoundPosition(cent);

  // check for event-only entities
  if (cent->currentState.eType > ET_EVENTS) {
    if (cent->previousEvent) {
      // goto skipEvent;
      return; // already fired
    }
    // if this is a player event set the entity number of the
    // client entity number
    //(SA) note: EF_PLAYER_EVENT never set
    //		if ( cent->currentState.eFlags & EF_PLAYER_EVENT )
    //{ 			cent->currentState.number =
    // cent->currentState.otherEntityNum;
    //		}

    cent->previousEvent = 1;

    cent->currentState.event = cent->currentState.eType - ET_EVENTS;
  } else {

    // DHM - Nerve :: Entities that make it here are Not
    // TempEntities.
    //		As far as we could tell, for all non-TempEntities,
    // the 		circular 'events' list contains the valid events.  So
    // we 		skip processing the single 'event' field and go
    // straight 		to the circular list.

    goto skipEvent;
    /*
    // check for events riding with another entity
    if ( cent->currentState.event == cent->previousEvent ) {
        goto skipEvent;
        //return;
    }
    cent->previousEvent = cent->currentState.event;
    if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) {
        goto skipEvent;
        //return;
    }
    */
    // dhm - end
  }

  CG_EntityEvent(cent, cent->lerpOrigin);
  // DHM - Nerve :: Temp ents return after processing
  return;

skipEvent:

  // check the sequencial list
  // if we've added more events than can fit into the list, make sure we
  // only add them once
  if (cent->currentState.eventSequence < cent->previousEventSequence) {
    cent->previousEventSequence -= (1 << 8); // eventSequence is sent as an
                                             // 8-bit through network stream
  }
  if (cent->currentState.eventSequence - cent->previousEventSequence >
      MAX_EVENTS) {
    cent->previousEventSequence = cent->currentState.eventSequence - MAX_EVENTS;
  }
  for (i = cent->previousEventSequence; i != cent->currentState.eventSequence;
       i++) {
    event = cent->currentState.events[i & (MAX_EVENTS - 1)];

    cent->currentState.event = event;
    cent->currentState.eventParm =
        cent->currentState.eventParms[i & (MAX_EVENTS - 1)];
    CG_EntityEvent(cent, cent->lerpOrigin);
  }
  cent->previousEventSequence = cent->currentState.eventSequence;

  // set the event back so we don't think it's changed next frame
  // (unless it really has)
  cent->currentState.event = cent->previousEvent;
}
