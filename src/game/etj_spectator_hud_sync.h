#pragma once

#include <string>
#include <vector>

struct gentity_s;
typedef struct gentity_s gentity_t;

namespace ETJump::SpectatorHudSync {
using Arguments = const std::vector<std::string> *;

bool upload(gentity_t *ent, Arguments argv);
bool watch(gentity_t *ent, Arguments argv);

void clearWatchForTarget(int clientNum);
void resetWatchState(gentity_t *ent);
} // namespace ETJump::SpectatorHudSync
