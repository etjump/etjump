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

#include "etj_entity_utilities.h"
#include "etj_string_utilities.h"

extern field_t fields[];

namespace ETJump {
std::vector<std::string> EntityUtilities::parsedEntities;

bool EntityUtilities::isPlayer(gentity_t *ent) {
  auto cnum = ClientNum(ent);
  return cnum >= 0 && cnum < MAX_CLIENTS;
}

// vectors don't seem to like vec_t as their value,
// so using raw values for colors here instead of colorRed, colorGreen etc.
const std::vector<std::pair<std::string, std::vector<float>>> railBoxEnts{
    {"trigger_multiple_ext", {0.0f, 1.0f, 0.0f}},
    {"trigger_starttimer_ext", {0.0f, 0.0f, 1.0f}},
    {"trigger_stoptimer_ext", {0.0f, 0.0f, 1.0f}},
    {"trigger_checkpoint_ext", {1.0f, 0.0f, 1.0f}},
    {"func_fakebrush", {1.0f, 0.0f, 0.0f}}};

void EntityUtilities::checkForRailBox(gentity_t *ent) {
  for (const auto &railBoxEnt : railBoxEnts) {
    if (ent->classname == railBoxEnt.first) {
      drawRailBox(ent, railBoxEnt.second);
    }
  }
}

void EntityUtilities::drawRailBox(const gentity_t *ent,
                                  const std::vector<float> &color) {
  vec3_t b1;
  vec3_t b2;
  gentity_t *temp;

  VectorCopy(ent->r.currentOrigin, b1);
  VectorCopy(ent->r.currentOrigin, b2);
  VectorAdd(b1, ent->r.mins, b1);
  VectorAdd(b2, ent->r.maxs, b2);

  temp = G_TempEntity(b1, EV_RAILTRAIL);

  VectorCopy(b2, temp->s.origin2);
  VectorCopy(color, temp->s.angles);
  temp->s.dmgFlags = 1;

  temp->s.angles[0] = color[0] * 255;
  temp->s.angles[1] = color[1] * 255;
  temp->s.angles[2] = color[2] * 255;

  temp->s.effect1Time = ent->s.number + 1;
}

bool EntityUtilities::playerIsSolid(const int self, const int other) {
  fireteamData_t *ftSelf, *ftOther;
  const gclient_t *selfClient = g_entities[self].client;
  const gclient_t *otherClient = g_entities[other].client;

  if (self == other) {
    return false;
  }

  if (selfClient->sess.sessionTeam == TEAM_SPECTATOR ||
      otherClient->sess.sessionTeam == TEAM_SPECTATOR) {
    return false;
  }

  if (selfClient->ps.pm_type == PM_NOCLIP ||
      otherClient->ps.pm_type == PM_NOCLIP) {
    return false;
  }

  if (g_ghostPlayers.integer == 1) {
    if (!G_IsOnFireteam(self, &ftSelf)) {
      return false;
    }

    // we're on same fireteam, but noghost isn't enabled
    if (G_IsOnFireteam(other, &ftOther) && ftSelf == ftOther) {
      if (!ftSelf->noGhost) {
        return false;
      }
    }

    // we're not in the same fireteam
    if (!G_IsOnFireteam(other, &ftOther) || ftSelf != ftOther) {
      return false;
    }
  }

  return true;
}

bool EntityUtilities::entitiesFree(const int threshold) {
  int free = 0;

  for (int i = MAX_CLIENTS + BODY_QUEUE_SIZE; i < ENTITYNUM_MAX_NORMAL; i++) {
    const gentity_t *ent = &g_entities[i];

    if (!ent->inuse) {
      free++;
    }
  }

  return free > threshold;
}

void EntityUtilities::setCursorhintFromString(int &value,
                                              const std::string &hint) {
  for (int i = 0; i < HINT_NUM_HINTS; i++) {
    if (StringUtils::iEqual(hint, hintStrings[i])) {
      value = i;
    }
  }
}

void EntityUtilities::getOriginOrBmodelCenter(const gentity_t *ent,
                                              vec3_t origin) {
  if (!VectorCompare(ent->r.currentOrigin, vec3_origin)) {
    VectorCopy(ent->r.currentOrigin, origin);
  } else {
    origin[0] = (ent->r.absmax[0] + ent->r.absmin[0]) / 2;
    origin[1] = (ent->r.absmax[1] + ent->r.absmin[1]) / 2;
    origin[2] = (ent->r.absmax[2] + ent->r.absmin[2]) / 2;
  }
}

bool EntityUtilities::clearPortals(gentity_t *ent) {
  bool removed = false;

  // Clear portalgun portals
  if (ent->portalBlue) {
    G_FreeEntity(ent->portalBlue);
    ent->portalBlue = nullptr;
    removed = true;
  }

  if (ent->portalRed) {
    G_FreeEntity(ent->portalRed);
    ent->portalRed = nullptr;
    removed = true;
  }

  if (ent->client) {
    ent->client->numPortals = 0;
  }

  return removed;
}

void EntityUtilities::storeParsedEntity() {
  assert(level.spawning);

  std::string entity;

  for (int i = 0; i < level.numSpawnVars; i++) {
    entity += StringUtils::format("\"%s\" \"%s\"\n", level.spawnVars[i][0],
                                  level.spawnVars[i][1]);
  }

  parsedEntities.push_back("{\n" + entity + "}\n");
}

const std::vector<std::string> &EntityUtilities::getParsedEntities() {
  return parsedEntities;
}

FindEntitiesResult EntityUtilities::findEntitiesByField(
    const std::string &key, const std::string &value, const char *func) {
  FindEntitiesResult result;
  int32_t fieldIndex = 0;

  for (; fields[fieldIndex].name; fieldIndex++) {
    if (!Q_stricmp(fields[fieldIndex].name, key.c_str())) {
      break;
    }
  }

  if (!fields[fieldIndex].name) {
    G_Error("%s: non-existing key '%s'\n", func, key.c_str());
  }

  int32_t valueInt = 0;
  float valueFloat = 0.0f;
  vec3_t valueVec{};
  std::vector<std::string> args = StringUtils::split(value, " ");

  gentity_t *found = nullptr;

  const auto invalidArgCount = [&](const int expectedArgs,
                                   const size_t numArgs) {
    G_Printf("%s: Invalid number of arguments for ^3'%s'^7, expected ^3%i^7, "
             "got ^3%i\n",
             func, key.c_str(), expectedArgs, static_cast<int>(numArgs));
    result.valid = false;
  };

  const auto invalidArgType = [&](const std::string &expectedType,
                                  const std::string &type) {
    G_Printf("%s: Invalid argument for ^3'%s'^7, expected ^3%s^7, got ^3'%s'\n",
             func, key.c_str(), expectedType.c_str(), type.c_str());
    result.valid = false;
  };

  switch (fields[fieldIndex].type) {
    case F_INT:
      if (args.size() != 1) {
        invalidArgCount(1, args.size());
        break;
      }

      try {
        valueInt = std::stoi(value);
      } catch (const std::logic_error &) {
        invalidArgType("number", args[0]);
        break;
      }

      while ((found = G_FindInt(found, fields[fieldIndex].ofs, valueInt)) !=
             nullptr) {
        result.entities.emplace_back(found->s.number);
      }

      break;
    case F_FLOAT:
      if (args.size() != 1) {
        invalidArgCount(1, args.size());
        break;
      }

      try {
        valueFloat = std::stof(value);
      } catch (const std::logic_error &) {
        invalidArgType("number", args[0]);
        break;
      }

      while ((found = G_FindFloat(found, fields[fieldIndex].ofs, valueFloat)) !=
             nullptr) {
        result.entities.emplace_back(found->s.number);
      }

      break;
    case F_LSTRING:
    case F_GSTRING:
      while ((found = G_Find(found, fields[fieldIndex].ofs, value.c_str())) !=
             nullptr) {
        result.entities.emplace_back(found->s.number);
      }
      break;

    case F_VECTOR:
      if (args.size() != 3) {
        invalidArgCount(3, args.size());
        break;
      }

      int j;

      try {
        for (j = 0; j < 3; j++) {
          valueVec[j] = std::stof(args[j]);
        }
      } catch (const std::logic_error &) {
        invalidArgType("number", args[j]);
        break;
      }

      while ((found = G_FindVec(found, fields[fieldIndex].ofs, valueVec)) !=
             nullptr) {
        result.entities.emplace_back(found->s.number);
      }

      break;
    case F_ANGLEHACK:
      if (args.size() != 1) {
        invalidArgCount(1, args.size());
        break;
      }

      VectorClear(valueVec);

      try {
        valueVec[2] = std::stof(args[0]);
      } catch (const std::logic_error &) {
        invalidArgType("number", args[0]);
        break;
      }

      while ((found = G_FindVec(found, fields[fieldIndex].ofs, valueVec)) !=
             nullptr) {
        result.entities.emplace_back(found->s.number);
      }

      break;
    case F_CURSORHINT:
      if (args.size() != 1) {
        invalidArgCount(1, args.size());
        break;
      }

      // set to end cap initially, so we don't find all entities
      // with HINT_NONE if we don't find a match
      valueInt = HINT_NUM_HINTS;
      setCursorhintFromString(valueInt, value);

      while ((found = G_FindInt(found, fields[fieldIndex].ofs, valueInt)) !=
             nullptr) {
        result.entities.emplace_back(found->s.number);
      }

      break;
    default:
      G_Printf(S_COLOR_YELLOW "%s: invalid key '%s'\n", func, key.c_str());
      result.stopParsing = true;
      break;
  }

  return result;
}
} // namespace ETJump
