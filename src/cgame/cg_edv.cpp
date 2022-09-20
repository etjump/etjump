#include "cg_local.h"

void CG_RunBinding(int key, qboolean down) {
  char buf[MAX_STRING_TOKENS];

  // we don't want these duplicate keypresses
  if (key & K_CHAR_FLAG) {
    return;
  }

  trap_Key_GetBindingBuf(key, buf, sizeof(buf));

  if (!buf[0]) {
    return;
  }

  CG_RunBindingBuf(key, down, buf);
}

void CG_RunBindingBuf(int key, qboolean down, char *buf) {
  if (!buf[0]) {
    return;
  }

  // +commands need to be handled specially
  if (buf[0] == '+') {
    if (!down) {
      buf[0] = '-';
    }

    // pass the key number so that the engine can handle
    // keyups on its own if neccessary

    // the engine also tacks on the time so that sub-frame
    // key timing can be achieved
    trap_SendConsoleCommand(va("%s %d %d\n", buf, key, trap_Milliseconds()));

    // for cg_runinput so we can override client engine
    // hopefully this is ugly enough to make someone want to
    // make it teh bettar
    if (cgs.demoCam.renderingFreeCam) {
      if (!Q_stricmp(buf, "+moveright")) {
        cgs.demoCam.move |= 0x04;
      } else if (!Q_stricmp(buf, "-moveright")) {
        cgs.demoCam.move &= ~0x04;
      } else if (!Q_stricmp(buf, "+moveleft")) {
        cgs.demoCam.move |= 0x08;
      } else if (!Q_stricmp(buf, "-moveleft")) {
        cgs.demoCam.move &= ~0x08;
      } else if (!Q_stricmp(buf, "+forward")) {
        cgs.demoCam.move |= 0x01;
      } else if (!Q_stricmp(buf, "-forward")) {
        cgs.demoCam.move &= ~0x01;
      } else if (!Q_stricmp(buf, "+back")) {
        cgs.demoCam.move |= 0x02;
      } else if (!Q_stricmp(buf, "-back")) {
        cgs.demoCam.move &= ~0x02;
      } else if (!Q_stricmp(buf, "+moveup")) {
        cgs.demoCam.move |= 0x10;
      } else if (!Q_stricmp(buf, "-moveup")) {
        cgs.demoCam.move &= ~0x10;
      } else if (!Q_stricmp(buf, "+movedown")) {
        cgs.demoCam.move |= 0x20;
      } else if (!Q_stricmp(buf, "-movedown")) {
        cgs.demoCam.move &= ~0x20;
      } else if (!Q_stricmp(buf, "+sprint")) {
        cgs.demoCam.move |= 0x40;
      } else if (!Q_stricmp(buf, "-sprint")) {
        cgs.demoCam.move &= ~0x40;
      }
    } else {
      cgs.demoCam.move = 0;
    }

    return;
  } else if (down == qfalse) {
    return; // we don't care about keyups otherwise
  }
  trap_SendConsoleCommand(va("%s\n", buf));
}

extern pmove_t cg_pmove; // cg_predict.c
extern void CG_TraceCapsule_World(trace_t *result, const vec3_t start,
                                  const vec3_t mins, const vec3_t maxs,
                                  const vec3_t end, int skipNumber,
                                  int mask); // cg_predict.c

void CG_EDV_RunInput(void) {
  playerState_t edv_ps;
  pmoveExt_t edv_pmext;
  static int lasttime = 0;
  int i, delta;
  vec_t frametime;

  static vec3_t mins = {-6, -6, -6};
  static vec3_t maxs = {6, 6, 6};

  trap_GetUserCmd(trap_GetCurrentCmdNumber(), &cg_pmove.cmd);

  // anti cl_freezedemo
  cg_pmove.cmd.serverTime = trap_Milliseconds();

  // calculate time so we can hack in angles
  delta = cg.time - lasttime;
  lasttime = cg.time;

  // semi-arbitrary - matches min/max values in pmove
  if (delta < 1) {
    delta = 1;
  } else if (delta > 200) {
    delta = 200;
  }

  frametime = delta / 1000.0f;

  // client engine insists on "helping" cgame if PERS_HWEAPON_USE is
  // set. so we substitute saved cmd from etpro_RunBindingBuf() and tell
  // client to FOAD
  if (cg.snap->ps.persistant[PERS_HWEAPON_USE] &&
      cgs.demoCam.renderingFreeCam) {
    //		CG_Printf( "%d %d %d\n", edv_rightmove,
    // edv_forwardmove,
    // edv_upmove
    //);
    // I still don't like this
    cg_pmove.cmd.forwardmove = (cgs.demoCam.move & 0x01) ? 127 : 0;
    cg_pmove.cmd.forwardmove += (cgs.demoCam.move & 0x02) ? -127 : 0;
    cg_pmove.cmd.rightmove = (cgs.demoCam.move & 0x04) ? 127 : 0;
    cg_pmove.cmd.rightmove += (cgs.demoCam.move & 0x08) ? -127 : 0;
    cg_pmove.cmd.upmove = (cgs.demoCam.move & 0x10) ? 127 : 0;
    cg_pmove.cmd.upmove += (cgs.demoCam.move & 0x20) ? -127 : 0;
    // additional buttons
    if (cgs.demoCam.move & 0x40) {
      cg_pmove.cmd.buttons |= BUTTON_SPRINT;
    } else {
      cg_pmove.cmd.buttons &= ~BUTTON_SPRINT;
    }
  }

  // run turns, I still don't like this
  cg.refdefViewAngles[YAW] +=
      (cgs.demoCam.turn & 0x01) ? etj_demo_yawturnspeed.value * frametime : 0;
  cg.refdefViewAngles[YAW] +=
      (cgs.demoCam.turn & 0x02) ? -etj_demo_yawturnspeed.value * frametime : 0;
  cg.refdefViewAngles[PITCH] +=
      (cgs.demoCam.turn & 0x04) ? etj_demo_pitchturnspeed.value * frametime : 0;
  cg.refdefViewAngles[PITCH] += (cgs.demoCam.turn & 0x08)
                                    ? -etj_demo_pitchturnspeed.value * frametime
                                    : 0;
  cg.refdefViewAngles[ROLL] +=
      (cgs.demoCam.turn & 0x10) ? etj_demo_rollspeed.value * frametime : 0;
  cg.refdefViewAngles[ROLL] +=
      (cgs.demoCam.turn & 0x20) ? -etj_demo_rollspeed.value * frametime : 0;

  /* zinx - Use current viewangles instead of the command angles;
  looking is handled elsewhere (where, i do not know) */
  for (i = 0; i < 3; i++) {
    cg_pmove.cmd.angles[i] = ANGLE2SHORT(cg.refdefViewAngles[i]);
  }

  cg_pmove.cmd.buttons &=
      ~BUTTON_TALK; // zinx - FIXME: Why is the engine talking?

  /* Create a playerState for cam movement */
  memset(&edv_ps, 0, sizeof(edv_ps));
  edv_ps.commandTime = cgs.demoCam.commandTime;
  if (cgs.demoCam.noclip) {
    edv_ps.pm_type = PM_NOCLIP;
  } else {
    edv_ps.pm_type = PM_SPECTATOR;
  }
  edv_ps.pm_flags = 0;
  edv_ps.gravity = 0;
  edv_ps.friction = 5.0f;
  edv_ps.groundEntityNum = ENTITYNUM_NONE;
  edv_ps.clientNum = cg.predictedPlayerState.clientNum;
  edv_ps.eFlags = 0;
  VectorCopy(mins, edv_ps.mins);
  VectorCopy(maxs, edv_ps.maxs);

  // added speed cvar
  edv_ps.speed = etj_demo_freecamspeed.integer;
  edv_ps.runSpeedScale = 0.8;
  edv_ps.sprintSpeedScale = 1.1;
  edv_ps.crouchSpeedScale = 0.25;

  VectorSet(edv_ps.delta_angles, 0, 0, 0);
  VectorCopy(cg.refdefViewAngles, edv_ps.viewangles);
  VectorCopy(cgs.demoCam.camOrigin, edv_ps.origin);
  VectorCopy(cgs.demoCam.velocity, edv_ps.velocity);

  edv_ps.crouchMaxZ =
      edv_ps.maxs[2] - (edv_ps.standViewHeight - edv_ps.crouchViewHeight);

  /* Create pmext for cam movement */
  memset(&edv_pmext, 0, sizeof(edv_pmext));
  edv_pmext.sprintTime = SPRINTTIME;
  edv_pmext.noclipScale = etj_noclipScale.value;

  /* Fill in pmove stuff */
  cg_pmove.ps = &edv_ps;
  cg_pmove.pmext = &edv_pmext;
  cg_pmove.character =
      CG_CharacterForClientinfo(&cgs.clientinfo[cg.snap->ps.clientNum],
                                &cg_entities[cg.snap->ps.clientNum]);
  cg_pmove.skill = cgs.clientinfo[cg.snap->ps.clientNum].skill;

  cg_pmove.trace = CG_TraceCapsule_World;
  cg_pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
  cg_pmove.pointcontents = CG_PointContents;
  cg_pmove.noFootsteps = qtrue;
  cg_pmove.noWeapClips = qtrue;

  /* Do the move */
  Pmove(&cg_pmove);

  /* Update cam */
  cgs.demoCam.commandTime = edv_ps.commandTime;
  VectorCopy(edv_ps.origin, cgs.demoCam.camOrigin);
  VectorCopy(edv_ps.velocity, cgs.demoCam.velocity);
  // angles, too
  VectorCopy(edv_ps.viewangles, cg.refdefViewAngles);
}