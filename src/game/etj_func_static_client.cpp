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

#include "etj_func_static_client.h"

namespace ETJump {
inline constexpr int32_t SF_START_INVIS = 1 << 0;
inline constexpr int32_t SF_GIB_INSIDE = 1 << 1;

/*
 * 'ent->s.effect1Time' and 'ent->s.effect2Time' are treated as boolean
 * bitsets by this entity. Clients 0-31 map to 'effect1Time',
 * clients 32-63 map to 'effect2Time'. If the bit corresponding to the
 * client's number is set in the entity, the entity is considered to be
 * in "off" state, and the client ignores collision with the entity.
 * Unless 'offShader' or 'offModel' are set, the entity will also be
 * completely invisible to the client.
 *
 * If 'model2' is set, the entity's brushmodel will be used for collision.
 */
void FuncStaticClient::spawn(gentity_t *ent) {
  trap_SetBrushModel(ent, ent->model);
  InitMover(ent);

  // re-link with correct origin
  VectorCopy(ent->s.origin, ent->s.pos.trBase);
  VectorCopy(ent->s.origin, ent->r.currentOrigin);
  trap_LinkEntity(ent);

  ent->use = use;
  ent->s.eType = ET_STATIC_CLIENT;

  // hide by default for all clients if spawnflag 1 is set
  if (ent->spawnflags & SF_START_INVIS) {
    ent->s.effect1Time = INT_MAX;
    ent->s.effect2Time = INT_MAX;
  }

  char *s = nullptr;

  // 'model2' support so we can also use models
  // NOTE: using 'ent->s.density' for both 'offModel' and 'offShader',
  // since they are mutually exclusive - client knows how to interpret this
  // by checking if 'ent->s.modelindex2' is set or not
  if (ent->s.modelindex2) {
    // alternative model if 'model2' is set, when the brush is in "off" state
    if (G_SpawnString("offModel", "", &s)) {
      ent->s.density = G_ModelIndex(s);
    }
  } else {
    // custom shader when the brush is in "off" state
    if (G_SpawnString("offShader", "", &s)) {
      ent->s.density = G_ShaderIndex(s);
    }
  }
}

void FuncStaticClient::use(gentity_t *self, [[maybe_unused]] gentity_t *other,
                           gentity_t *activator) {
  if (!activator || !activator->client) {
    G_Error("'%s' cannot be used by a non-client entity", self->classname);
  }

  const int32_t clientNum = ClientNum(activator);

  if (clientNum < MAX_CLIENTS / 2) {
    COM_BitCheck(&self->s.effect1Time, clientNum) ? turnOn(self, clientNum)
                                                  : turnOff(self, clientNum);
  } else {
    COM_BitCheck(&self->s.effect2Time, clientNum) ? turnOn(self, clientNum)
                                                  : turnOff(self, clientNum);
  }

  self->activator = activator;

  if (self->scriptName) {
    G_Script_ScriptEvent(
        self, "activate",
        activator->client->sess.sessionTeam == TEAM_AXIS ? "axis" : "allies");
  }

  G_UseTargets(self, self->activator);
}

void FuncStaticClient::turnOn(gentity_t *self, const int32_t clientNum) {
  if (clientNum < MAX_CLIENTS / 2) {
    COM_BitClear(&self->s.effect1Time, clientNum);
  } else {
    COM_BitClear(&self->s.effect2Time, clientNum);
  }

  if (self->spawnflags & SF_GIB_INSIDE &&
      activatorIsInsideEnt(self, clientNum)) {
    G_Damage(g_entities + clientNum, self, self, nullptr, nullptr, 9999,
             DAMAGE_NO_PROTECTION, MOD_CRUSH);
  }
}

void FuncStaticClient::turnOff(gentity_t *self, const int32_t clientNum) {
  if (clientNum < MAX_CLIENTS / 2) {
    COM_BitSet(&self->s.effect1Time, clientNum);
  } else {
    COM_BitSet(&self->s.effect2Time, clientNum);
  }
}

bool FuncStaticClient::activatorIsInsideEnt(const gentity_t *self,
                                            const int32_t clientNum) {
  std::array<int32_t, MAX_GENTITIES> touch{};
  const gentity_t *activator = g_entities + clientNum;
  vec3_t mins{};
  vec3_t maxs{};

  VectorAdd(activator->r.currentOrigin, activator->client->ps.mins, mins);
  VectorAdd(activator->r.currentOrigin, activator->client->ps.maxs, maxs);

  const int32_t count =
      trap_EntitiesInBox(mins, maxs, touch.data(), MAX_GENTITIES);

  for (int32_t i = 0; i < count; i++) {
    if (touch[i] == self->s.number) {
      return true;
    }
  }

  return false;
}
} // namespace ETJump
