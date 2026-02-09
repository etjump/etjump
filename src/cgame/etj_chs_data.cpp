/*
 * MIT License
 *
 * Copyright (c) 2026 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "etj_chs_data.h"
#include "etj_utilities.h"

#include "../game/etj_string_utilities.h"

namespace ETJump {
CHSDataHandler::CHSDataHandler(
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdateHandler,
    const std::shared_ptr<ClientCommandsHandler> &consoleCommandsHandler)
    : cvarUpdateHandler(cvarUpdateHandler),
      consoleCommandsHandler(consoleCommandsHandler) {
  setupListeners();
  setupStats();
  setZOffset(&etj_CHSUseFeet);

  updateCHS1State();
  updateCHS2State();
}

CHSDataHandler::~CHSDataHandler() {
  consoleCommandsHandler->unsubscribe("chs");

  cvarUpdateHandler->unsubscribe(&etj_CHSUseFeet);
  cvarUpdateHandler->unsubscribe(&etj_extraTrace);

  for (const auto &CHSCvar : CHS1Cvars) {
    cvarUpdateHandler->unsubscribe(CHSCvar.cvar);
  }

  for (const auto &CHSCvar : CHS2Cvars) {
    cvarUpdateHandler->unsubscribe(CHSCvar.cvar);
  }
}

void CHSDataHandler::runFrame() {
  ps = getValidPlayerState();

  if (CHS1NeedsTrace || CHS2NeedsTrace) {
    viewTrace(&trace, CONTENTS_SOLID);
  }

  if (CHS1NeedsExtraTrace || CHS2NeedsExtraTrace) {
    viewTrace(&extraTrace, (CONTENTS_SOLID | CONTENTS_PLAYERCLIP));
  }
}

std::string CHSDataHandler::getStat(const vmCvar_t *cvar) const {
  const auto stat = static_cast<Stats>(cvar->integer);

  // this *should* be valid always, but let's be sure
  try {
    return stats.at(stat).fn();
  } catch (const std::out_of_range &) {
    CG_Printf(S_COLOR_RED "Invalid CHS value access! This is a bug,please "
                          "report this to the developers.\n");
    return "^1ERROR";
  }
}

std::string CHSDataHandler::getStatName(const vmCvar_t *cvar) const {
  const auto stat = static_cast<Stats>(cvar->integer);

  // this *should* be valid always, but let's be sure
  try {
    return stats.at(stat).name;
  } catch (const std::out_of_range &) {
    CG_Printf(S_COLOR_RED "Invalid CHS value access! This is a bug, please "
                          "report this to the developers.\n");
    return "^1ERROR";
  }
}

std::array<CHSDataHandler::CHSCvar, MAX_CHS_INFO> &
CHSDataHandler::getCHS1Cvars() {
  return CHS1Cvars;
}

std::array<CHSDataHandler::CHSCvar, MAX_CHS_INFO> &
CHSDataHandler::getCHS2Cvars() {
  return CHS2Cvars;
}

void CHSDataHandler::setupListeners() {
  consoleCommandsHandler->subscribe("chs",
                                    [this](const auto &) { printInfo(); });

  cvarUpdateHandler->subscribe(
      &etj_CHSUseFeet, [this](const vmCvar_t *cvar) { setZOffset(cvar); });

  cvarUpdateHandler->subscribe(&etj_extraTrace, [this](const vmCvar_t *cvar) {
    // there's no nice, clean way to map the extra trace values to the stats,
    // so rather than checking if any of these CHS infos are enabled,
    // just update the state for both of the CHS displays
    if (cvar->integer & (1 << CHS_10_11) || cvar->integer & (1 << CHS_12) ||
        cvar->integer & (1 << CHS_13_15) || cvar->integer & (1 << CHS_16) ||
        cvar->integer & (1 << CHS_53)) {
      updateCHS1State();
      updateCHS2State();
    }
  });

  for (const auto &CHSCvar : CHS1Cvars) {
    cvarUpdateHandler->subscribe(
        CHSCvar.cvar, [this](const vmCvar_t *) { updateCHS1State(); });
  }

  for (const auto &CHSCvar : CHS2Cvars) {
    cvarUpdateHandler->subscribe(
        CHSCvar.cvar, [this](const vmCvar_t *) { updateCHS2State(); });
  }
}

void CHSDataHandler::setZOffset(const vmCvar_t *cvar) {
  // we can't use 'ps->mins' here as it's nullptr
  // when we call this initially from the constructor
  ZOffset = cvar->integer ? playerMins[2] : 0;
}

void CHSDataHandler::updateCHS1State() {
  CHS1NeedsTrace = false;
  CHS1NeedsExtraTrace = false;

  for (auto &CHSCvar : CHS1Cvars) {
    const auto stat = static_cast<Stats>(CHSCvar.cvar->integer);

    if (stats.find(stat) == stats.cend()) {
      CHSCvar.valid = false;
      continue;
    }

    if (!CHS1NeedsTrace && stats[stat].opts & StatOpts::TRACE) {
      CHS1NeedsTrace = true;
    }

    if (!CHS1NeedsExtraTrace && stats[stat].opts & StatOpts::EXTRA_TRACE) {
      CHS1NeedsExtraTrace = statNeedsExtraTrace(stat);
    }

    CHSCvar.valid = true;
  }
}

void CHSDataHandler::updateCHS2State() {
  CHS2NeedsTrace = false;
  CHS2NeedsExtraTrace = false;

  for (auto &CHSCvar : CHS2Cvars) {
    const auto stat = static_cast<Stats>(CHSCvar.cvar->integer);

    if (stats.find(stat) == stats.cend()) {
      CHSCvar.valid = false;
      continue;
    }

    if (!CHS2NeedsTrace && stats[stat].opts & StatOpts::TRACE) {
      CHS2NeedsTrace = true;
    }

    if (!CHS2NeedsExtraTrace && stats[stat].opts & StatOpts::EXTRA_TRACE) {
      CHS2NeedsExtraTrace = statNeedsExtraTrace(stat);
    }

    CHSCvar.valid = true;
  }
}

bool CHSDataHandler::statNeedsExtraTrace(const Stats stat) {
  switch (stat) {
    case Stats::DISTANCE_XY:
    case Stats::DISTANCE_Z:
      return etj_extraTrace.integer & (1 << CHS_10_11);
    case Stats::DISTANCE_XYZ:
      return etj_extraTrace.integer & (1 << CHS_12);
    case Stats::DISTANCE_VIEW_XYZ:
    case Stats::DISTANCE_XY_Z_XYZ:
    case Stats::DISTANCE_XY_Z_VIEW_XYZ:
      return etj_extraTrace.integer & (1 << CHS_13_15);
    case Stats::LOOK_XYZ:
      return etj_extraTrace.integer & (1 << CHS_16);
    case Stats::PLANE_ANGLE_Z:
      return etj_extraTrace.integer & (1 << CHS_53);
    default:
      return false;
  }
}

void CHSDataHandler::viewTrace(trace_t *tr, const int32_t mask) {
  vec3_t start{};
  vec3_t end{};

  VectorCopy(cg.refdef.vieworg, start);
  VectorMA(start, MAX_MAP_SIZE * 2, cg.refdef.viewaxis[0], end);

  CG_Trace(tr, start, nullptr, nullptr, end, ps->clientNum, mask);
}

std::string CHSDataHandler::speed(const SpeedType type) const {
  switch (type) {
    case SpeedType::X:
      return stringFormat("%.0f", ps->velocity[0]);
    case SpeedType::Y:
      return stringFormat("%.0f", ps->velocity[1]);
    case SpeedType::Z:
      return stringFormat("%.0f", ps->velocity[2]);
    case SpeedType::XY:
      return stringFormat("%.0f", VectorLength2(ps->velocity));
    case SpeedType::XYZ:
      return stringFormat("%.0f", VectorLength(ps->velocity));
    case SpeedType::FORWARD:
      return stringFormat("%.0f",
                          DotProduct(ps->velocity, cg.refdef.viewaxis[0]));
    case SpeedType::SIDE:
      return stringFormat("%.0f",
                          DotProduct(ps->velocity, cg.refdef.viewaxis[1]));
    case SpeedType::FORWARD_SIDE:
      return stringFormat("%.0f %.0f",
                          DotProduct(ps->velocity, cg.refdef.viewaxis[0]),
                          DotProduct(ps->velocity, cg.refdef.viewaxis[1]));
    case SpeedType::XY_FORWARD_SIDE:
      return stringFormat("%.0f %.0f %.0f", VectorLength2(ps->velocity),
                          DotProduct(ps->velocity, cg.refdef.viewaxis[0]),
                          DotProduct(ps->velocity, cg.refdef.viewaxis[1]));
    default:
      return "-";
  }
}

std::string CHSDataHandler::health() const {
  return std::to_string(ps->stats[STAT_HEALTH]);
}

std::string CHSDataHandler::ammo() {
  int32_t ammo{};
  int32_t clips{};
  int32_t akimboAmmo{};

  CG_PlayerAmmoValue(&ammo, &clips, &akimboAmmo);

  if (akimboAmmo >= 0) {
    return stringFormat("%i|%i/%i", akimboAmmo, ammo, clips);
  }

  if (clips >= 0) {
    return stringFormat("%i/%i", ammo, clips);
  }

  if (ammo >= 0) {
    return std::to_string(ammo);
  }

  return "";
}

std::string CHSDataHandler::distance(const DistanceType type) {
  switch (type) {
    case DistanceType::XY: {
      const trace_t tr = getTraceResults(CHS_10_11);

      return tr.fraction != 1.0f
                 ? stringFormat("%.0f",
                                std::sqrt(SQR(tr.endpos[0] - ps->origin[0]) +
                                          SQR(tr.endpos[1] - ps->origin[1])))
                 : "-";
    } break;
    case DistanceType::Z: {
      const trace_t tr = getTraceResults(CHS_10_11);

      return tr.fraction != 1.0f
                 ? stringFormat("%.0f", tr.endpos[2] - ps->origin[2] - ZOffset)
                 : "-";
    } break;
    case DistanceType::XYZ: {
      const trace_t tr = getTraceResults(CHS_12);
      vec3_t origin;

      VectorCopy(ps->origin, origin);
      origin[2] += ZOffset;

      return tr.fraction != 1.0f
                 ? stringFormat("%.0f", Distance(tr.endpos, origin))
                 : "-";
    } break;
    case DistanceType::VIEW_XYZ: {
      const trace_t tr = getTraceResults(CHS_13_15);

      return tr.fraction != 1.0f
                 ? stringFormat("%.0f", Distance(tr.endpos, cg.refdef.vieworg))
                 : "-";
    } break;
    case DistanceType::XY_Z_XYZ: {
      const trace_t tr = getTraceResults(CHS_13_15);
      vec3_t origin;

      VectorCopy(ps->origin, origin);
      origin[2] += ZOffset;

      return tr.fraction != 1.0f
                 ? stringFormat("%.0f %.0f %.0f",
                                std::sqrt(SQR(tr.endpos[0] - origin[0]) +
                                          SQR(tr.endpos[1] - origin[1])),
                                tr.endpos[2] - origin[2],
                                Distance(tr.endpos, origin))
                 : "- - -";
    } break;
    case DistanceType::XY_Z_VIEW_XYZ: {
      const trace_t tr = getTraceResults(CHS_13_15);
      vec3_t origin;

      VectorCopy(ps->origin, origin);
      origin[2] += ZOffset;

      return tr.fraction != 1.0f
                 ? stringFormat("%.0f %.0f %.0f",
                                std::sqrt(SQR(tr.endpos[0] - origin[0]) +
                                          SQR(tr.endpos[1] - origin[1])),
                                tr.endpos[2] - origin[2],
                                Distance(tr.endpos, cg.refdef.vieworg))
                 : "- - -";
    } break;
    default:
      return "-";
  }
}

std::string CHSDataHandler::lookXYZ() {
  const auto tr = getTraceResults(CHS_16);

  if (tr.fraction != 1.0f) {
    return stringFormat("%.0f %.0f %.0f", tr.plane.dist * tr.plane.normal[0],
                        tr.plane.dist * tr.plane.normal[1],
                        tr.plane.dist * tr.plane.normal[2]);
  }

  return "- - -";
}

std::string CHSDataHandler::angle(const int32_t angle) const {
  switch (angle) {
    case PITCH:
      return stringFormat("%.2f", ps->viewangles[PITCH]);
    case YAW:
      return stringFormat("%.2f", ps->viewangles[YAW]);
    case ROLL:
      return stringFormat("%.2f", ps->viewangles[ROLL]);
    default:
      return "-";
  }
}

std::string CHSDataHandler::position(const PositionType type) const {
  switch (type) {
    case PositionType::X:
      return stringFormat("%.0f", ps->origin[0]);
    case PositionType::Y:
      return stringFormat("%.0f", ps->origin[1]);
    case PositionType::Z:
      return stringFormat("%.0f", ps->origin[2] + ZOffset);
    case PositionType::VIEW_X:
      return stringFormat("%.0f", cg.refdef.vieworg[0]);
    case PositionType::VIEW_Y:
      return stringFormat("%.0f", cg.refdef.vieworg[1]);
    case PositionType::VIEW_Z:
      return stringFormat("%.0f", cg.refdef.vieworg[2]);
    default:
      return "-";
  }
}

std::string CHSDataHandler::lastJumpPos() const {
  return stringFormat("%.0f %.0f %.0f", cg.etjLastJumpPos[0],
                      cg.etjLastJumpPos[1], cg.etjLastJumpPos[2] + ZOffset);
}

std::string CHSDataHandler::planeAngleZ() {
  const trace_t tr = getTraceResults(CHS_53);

  if (tr.fraction != 1.0f) {
    return stringFormat("%.2f",
                        std::atan2(std::sqrt(pow(tr.plane.normal[0], 2) +
                                             pow(tr.plane.normal[1], 2)),
                                   tr.plane.normal[2]) *
                            180 / M_PI);
  }

  return "-";
}

std::string CHSDataHandler::lastJumpSpeed() const {
  return std::to_string(ps->persistant[PERS_JUMP_SPEED]);
}

trace_t &CHSDataHandler::getTraceResults(const extraTraceOptions opt) {
  if (etj_extraTrace.integer & (1 << opt)) {
    return extraTrace;
  }

  return trace;
}

void CHSDataHandler::printInfo() const {
  for (const auto &[key, info] : stats) {
    CG_Printf("%3i: %s\n", info.name.c_str(), info.description.c_str());
  }
}

void CHSDataHandler::setupStats() {
  // TODO: in DeFRaG, this is XY speed only, make it XY here too?
  stats[Stats::SPEED] = {[this]() { return speed(SpeedType::XYZ); }, "Speed",
                         "player speed"};

  stats[Stats::HEALTH] = {[this]() { return health(); }, "Health",
                          "player health"};

  stats[Stats::AMMO] = {[]() { return ammo(); }, "Ammo",
                        "player ammo for currently selected weapon"};

  stats[Stats::DISTANCE_XY] = {[this]() { return distance(DistanceType::XY); },
                               "Distance XY",
                               "horizontal distance to plane",
                               {StatOpts::TRACE, StatOpts::EXTRA_TRACE}};

  stats[Stats::DISTANCE_Z] = {[this]() { return distance(DistanceType::Z); },
                              "Distance Z",
                              "vertical distance to plane",
                              {StatOpts::TRACE, StatOpts::EXTRA_TRACE}};

  stats[Stats::DISTANCE_XYZ] = {
      [this]() { return distance(DistanceType::XYZ); },
      "Distance XYZ",
      "true distance to plane",
      {StatOpts::TRACE, StatOpts::EXTRA_TRACE}};

  stats[Stats::DISTANCE_VIEW_XYZ] = {
      [this]() { return distance(DistanceType::VIEW_XYZ); },
      "Distance ViewXYZ",
      "true distance to plane from view point",
      {StatOpts::TRACE, StatOpts::EXTRA_TRACE}};

  stats[Stats::DISTANCE_XY_Z_XYZ] = {
      [this]() { return distance(DistanceType::XY_Z_XYZ); },
      "Distance XY Z XYZ",
      "horizontal/vertical/true distance to plane",
      {StatOpts::TRACE, StatOpts::EXTRA_TRACE}};

  stats[Stats::DISTANCE_XY_Z_VIEW_XYZ] = {
      [this]() { return distance(DistanceType::XY_Z_VIEW_XYZ); },
      "Distance XY Z ViewXYZ",
      "horizontal/vertical/true distance to plane from view point",
      {StatOpts::TRACE, StatOpts::EXTRA_TRACE}};

  stats[Stats::LOOK_XYZ] = {[this]() { return lookXYZ(); },
                            "Look XYZ",
                            "world x y z location of plane",
                            {StatOpts::TRACE, StatOpts::EXTRA_TRACE}};

  stats[Stats::SPEED_X] = {[this]() { return speed(SpeedType::X); }, "Speed X",
                           "speed along world x axis"};

  stats[Stats::SPEED_Y] = {[this]() { return speed(SpeedType::Y); }, "Speed Y",
                           "speed along world y axis"};

  stats[Stats::SPEED_Z] = {[this]() { return speed(SpeedType::Z); }, "Speed Z",
                           "speed along world z axis"};

  stats[Stats::SPEED_XY] = {[this]() { return speed(SpeedType::XY); },
                            "Speed XY", "horizontal speed"};

  stats[Stats::SPEED_XYZ] = {[this]() { return speed(SpeedType::XYZ); },
                             "Speed XYZ", "true speed"};

  stats[Stats::SPEED_FORWARD] = {[this]() { return speed(SpeedType::FORWARD); },
                                 "Speed Forward", "speed relative to forward"};

  stats[Stats::SPEED_SIDE] = {[this]() { return speed(SpeedType::SIDE); },
                              "Speed Sideways", "speed relative to side"};

  stats[Stats::SPEED_FORWARD_SIDE] = {
      [this]() { return speed(SpeedType::FORWARD_SIDE); },
      "Speed Forward Sideways", "speed relative to forward/side"};

  stats[Stats::SPEED_XY_FORWARD_SIDE] = {
      [this]() { return speed(SpeedType::XY_FORWARD_SIDE); },
      "Speed XY Forward Sideways",
      "horizontal speed/speed relative to forward/side"};

  stats[Stats::PITCH] = {[this]() { return angle(PITCH); }, "Pitch",
                         "player pitch"};

  stats[Stats::YAW] = {[this]() { return angle(YAW); }, "Yaw", "player yaw"};

  stats[Stats::ROLL] = {[this]() { return angle(ROLL); }, "Roll",
                        "player roll"};

  stats[Stats::POS_X] = {[this]() { return position(PositionType::X); },
                         "Position X", "player x position"};

  stats[Stats::POS_Y] = {[this]() { return position(PositionType::Y); },
                         "Position Y", "player y position"};

  stats[Stats::POS_Z] = {
      [this]() { return position(PositionType::Z); },
      "Position Z",
      "player z position",
  };

  stats[Stats::VIEW_POS_X] = {
      [this]() { return position(PositionType::VIEW_X); }, "View Position X",
      "view x position"};

  stats[Stats::VIEW_POS_Y] = {
      [this]() { return position(PositionType::VIEW_Y); }, "View Position Y",
      "view y position"};

  stats[Stats::VIEW_POS_Z] = {
      [this]() { return position(PositionType::VIEW_Z); }, "View Position Z",
      "view z position"};

  stats[Stats::PITCH_YAW] = {
      [this]() { return angle(PITCH) + " " + angle(YAW); }, "Pitch Yaw",
      "player pitch/yaw"};

  stats[Stats::PLAYER_POS_XYZ] = {
      [this]() {
        return position(PositionType::X) + " " + position(PositionType::Y) +
               " " + position(PositionType::Z);
      },
      "Player XYZ",
      "player position in the world",
  };

  stats[Stats::PLAYER_POS_XYZ_PITCH_YAW] = {
      [this]() {
        return position(PositionType::X) + " " + position(PositionType::Y) +
               " " + position(PositionType::Z) + " " + angle(PITCH) + " " +
               angle(YAW);
      },
      "Player XYZ Pitch Yaw",
      "player position in the world and pitch/yaw",
  };

  stats[Stats::VIEW_POS_XYZ_PITCH_YAW] = {
      [this]() {
        return position(PositionType::VIEW_X) + " " +
               position(PositionType::VIEW_Y) + " " +
               position(PositionType::VIEW_Z) + " " + angle(PITCH) + " " +
               angle(YAW);
      },
      "View Position XYZ Pitch Yaw",
      "view position in the world and pitch/yaw"};

  stats[Stats::POS_XYZ] = {
      [this]() {
        return position(PositionType::X) + " " + position(PositionType::Y) +
               " " + position(PositionType::Z);
      },
      "Position XYZ",
      "position x y z",
  };

  stats[Stats::ANGLES_XYZ] = {
      [this]() { return angle(PITCH) + " " + angle(YAW) + " " + angle(ROLL); },
      "Angles XYZ", "angles x y z"};

  stats[Stats::VELOCITY_XYZ] = {[this]() {
                                  return speed(SpeedType::X) + " " +
                                         speed(SpeedType::Y) + " " +
                                         speed(SpeedType::Z);
                                },
                                "Velocity XYZ", "velocity x y z"};

  stats[Stats::JUMP_XYZ] = {
      [this]() { return lastJumpPos(); },
      "Jump XYZ",
      "last jump x y z",
  };

  stats[Stats::PLANE_ANGLE_Z] = {[this]() { return planeAngleZ(); },
                                 "Plane Angle Z",
                                 "plane angle z",
                                 {StatOpts::TRACE, StatOpts::EXTRA_TRACE}};

  stats[Stats::LAST_JUMP_SPEED] = {[this]() { return lastJumpSpeed(); },
                                   "Jump speed", "last jump speed"};
}
} // namespace ETJump
