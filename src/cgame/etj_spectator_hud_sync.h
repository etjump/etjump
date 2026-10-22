#pragma once

#include <cstdint>
#include <string>

#include "../game/q_shared.h"

namespace ETJump {
const char *spectatorHudSyncEffectiveHudCvarString(const vmCvar_t *cvar);
int32_t spectatorHudSyncEffectiveHudCvarInt(const vmCvar_t *cvar);
float spectatorHudSyncEffectiveHudCvarFloat(const vmCvar_t *cvar);

void spectatorHudSyncReset();
void spectatorHudSyncRunFrame();
void spectatorHudSyncOnServerCommand(int targetClientNum, bool fullUpdate,
                                     int32_t revision,
                                     const std::string &cgazPayload,
                                     const std::string &snaphudPayload);
} // namespace ETJump
