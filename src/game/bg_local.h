// bg_local.h -- local definitions for the bg (both games) files

#pragma once

// can't walk on very steep slopes
inline constexpr float MIN_WALK_NORMAL = 0.7f;

inline constexpr int STEPSIZE = 18;

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
typedef struct {
  float frametime;

  int msec;

  float impactSpeed;

  vec3_t previous_origin;
  vec3_t previous_velocity;
  int previous_waterlevel;

  // Ridah, ladders
  qboolean ladder;

  // Feen: previous teleport (for crunch sound)
  qboolean previous_teleport;
  int previous_teleport_time;

} pml_t;

extern pmove_t *pm;
extern pml_t pml;

// movement parameters

inline constexpr float pm_stopspeed = 100;

inline constexpr float pm_waterSwimScale = 0.5;
inline constexpr float pm_waterWadeScale = 0.70;
inline constexpr float pm_slagSwimScale = 0.30;
inline constexpr float pm_slagWadeScale = 0.70;

// was: 0.18 (too slow) then: 0.24 (too fast)
inline constexpr float pm_proneSpeedScale = 0.21;

inline constexpr float pm_accelerate = 10;
inline constexpr float pm_airaccelerate = 1;
inline constexpr float pm_wateraccelerate = 4;
inline constexpr float pm_slagaccelerate = 2;
inline constexpr float pm_flyaccelerate = 8;

inline constexpr float pm_friction = 6;
inline constexpr float pm_waterfriction = 1;
inline constexpr float pm_slagfriction = 1;
inline constexpr float pm_flightfriction = 3;
inline constexpr float pm_ladderfriction = 14;
inline constexpr float pm_spectatorfriction = 5.0f;

extern int c_pmove;

void PM_AddTouchEnt(int entityNum);
void PM_AddEvent(int newEvent);

qboolean PM_SlideMove(qboolean gravity);
void PM_StepSlideMove(qboolean gravity);

qboolean PM_SlideMoveProne(qboolean gravity);
void PM_StepSlideMoveProne(qboolean gravity);

void PM_BeginWeaponChange(int oldweapon, int newweapon, qboolean reload);
