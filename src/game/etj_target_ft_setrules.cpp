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

#include "etj_target_ft_setrules.h"
#include "etj_printer.h"
#include "etj_string_utilities.h"

namespace ETJump {
void TargetFtSetRules::spawn(gentity_t *ent) {
  char keyNotSet[5]{};
  Com_sprintf(keyNotSet, sizeof(keyNotSet), "%i", KEY_NOT_SET);

  char *s;

  // this must be a string so we can support 'savelimit reset'
  G_SpawnString("savelimit", "", &s);
  ent->ftSavelimit = G_NewString(s);

  // this is a boolean but there's no G_SpawnBoolean so bleh
  ent->damage = KEY_NOT_SET;
  if (G_SpawnInt("noghost", keyNotSet, &ent->damage)) {
    ent->damage = std::clamp(ent->damage, 0, 1);
  }

  ent->health = KEY_NOT_SET;
  if (G_SpawnInt("teamjumpmode", keyNotSet, &ent->health)) {
    ent->health = std::clamp(ent->health, 0, 1);
  }

  G_SpawnString("leader_only_message", "", &s);
  ent->message = G_NewString(s);

  ent->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
}

void TargetFtSetRules::use(const gentity_t *self, gentity_t *activator) {
  if (!activator || !activator->client) {
    return;
  }

  fireteamData_t *ft = nullptr;
  const int clientNum = ClientNum(activator);

  if (!G_IsOnFireteam(clientNum, &ft)) {
    return;
  }

  if (self->spawnflags & LEADER_ONLY && !G_IsFireteamLeader(clientNum, &ft)) {
    if (self->message[0] != '\0') {
      std::string msg = self->message;

      // this will never be nullptr, otherwise we wouldn't have gotten this far
      const gentity_t *leader = getFireteamLeader(clientNum);
      StringUtil::replaceAll(msg, "%s",
                             std::string(leader->client->pers.netname) + "^7");
      Printer::center(activator, msg);
    }

    return;
  }

  // from here on, ft pointer will always be valid,
  // so we can dereference it safely to check current ft state

  bool rulesChanged = false;

  if (self->ftSavelimit[0] != '\0' && canSetFtSavelimit(clientNum, self)) {
    if (!Q_stricmp(self->ftSavelimit, "reset")) {
      // no need to set rulesChanged to true here as we don't have to update
      // configstrings for 'reset', see g_fireteams.cpp -> setFireTeamRules
      setSaveLimitForFTMembers(ft, ft->saveLimit);
    } else {
      const int limit = std::clamp(Q_atoi(self->ftSavelimit), -1, 100);
      setSaveLimitForFTMembers(ft, limit);
      rulesChanged = true;
    }
  }

  if (self->damage != KEY_NOT_SET && ft->noGhost != self->damage &&
      canEnableFtNoGhost(clientNum, ft, self)) {
    setFireTeamGhosting(ft, self->damage);
    rulesChanged = true;
  }

  // this 'canSetFtTeamjumpMode' is redundant here as it will always
  // succeed for non-player entities, but better to have it here
  // if other restrictions are added in the future
  if (self->health != KEY_NOT_SET && ft->teamJumpMode != self->health &&
      canSetFtTeamjumpMode(clientNum, self)) {
    setFireteamTeamjumpMode(ft, self->health);
    rulesChanged = true;
  }

  if (rulesChanged) {
    G_UpdateFireteamConfigString(ft);
  }
}
} // namespace ETJump
