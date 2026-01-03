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

#include "g_local.h"
#include "etj_tokens.h"

#include "etj_filesystem.h"

#include <memory>
#include "json/json.h"
#include "etj_utilities.h"
#include "etj_time_utilities.h"
#include "etj_printer.h"
#include "etj_json_utilities.h"
#include "etj_log.h"

namespace ETJump {
static std::array<Tokens::Token, MAX_TOKENS_PER_DIFFICULTY> easyTokens;
static std::array<Tokens::Token, MAX_TOKENS_PER_DIFFICULTY> mediumTokens;
static std::array<Tokens::Token, MAX_TOKENS_PER_DIFFICULTY> hardTokens;

Tokens::Tokens(std::unique_ptr<Log> log) : logger(std::move(log)) {}

std::string Tokens::tokenDifficultyToString(const Difficulty difficulty) {
  switch (difficulty) {
    case Tokens::Easy:
      return "easy";
    case Tokens::Medium:
      return "medium";
    case Tokens::Hard:
      return "hard";
  }
  return "unknown";
}

Tokens::NearestToken
Tokens::findNearestToken(const std::array<float, 3> coordinates) {
  int tokenNum = 0;
  Token *token = nullptr;
  auto difficulty = Easy;
  float nearestDistance = 1 << 20;

  auto findNearest = [&](std::array<Token, MAX_TOKENS_PER_DIFFICULTY> &tokens,
                         const Difficulty diff) {
    int idx = 0;

    for (auto &t : tokens) {
      if (!t.isActive) {
        idx++;
        continue;
      }

      const float newDistance =
          VectorDistance(t.coordinates.data(), coordinates.data());
      if (newDistance < nearestDistance) {
        nearestDistance = newDistance;
        token = &t;
        tokenNum = idx + 1;
        difficulty = diff;
      }

      idx++;
    }
  };

  findNearest(easyTokens, Easy);
  findNearest(mediumTokens, Medium);
  findNearest(hardTokens, Hard);

  return NearestToken{tokenNum, token, nearestDistance, difficulty};
}

std::pair<bool, std::string> Tokens::deleteToken(const Difficulty difficulty,
                                                 const int index) {
  Token *token = nullptr;

  switch (difficulty) {
    case Easy:
      token = &easyTokens[index];
      break;
    case Medium:
      token = &mediumTokens[index];
      break;
    case Hard:
      token = &hardTokens[index];
      break;
    default:
      return std::make_pair(false, "deleteToken: undefined difficulty.");
  }

  if (token->isActive) {
    token->isActive = false;
    G_FreeEntity(token->entity);
    token->entity = nullptr;
    saveTokens(_filepath);
    return std::make_pair(
        true, stringFormat("Successfully deleted %s token #%d",
                           tokenDifficultyToString(difficulty), index + 1));
  }
  return std::make_pair(
      false, stringFormat("%s token with number #%d does not exist.",
                          tokenDifficultyToString(difficulty), index + 1));
}

std::pair<bool, std::string>
Tokens::deleteNearestToken(const std::array<float, 3> coordinates) {
  const auto nearestToken = findNearestToken(coordinates);

  if (!nearestToken.token) {
    return std::make_pair(false, "no tokens in the map.");
  }

  nearestToken.token->isActive = false;
  saveTokens(_filepath);
  G_FreeEntity(nearestToken.token->entity);
  nearestToken.token->entity = nullptr;
  return std::make_pair(
      true, stringFormat("Deleted %s token #%d.",
                         tokenDifficultyToString(nearestToken.difficulty),
                         nearestToken.number));
}

std::pair<bool, std::string>
Tokens::moveNearestToken(std::array<float, 3> coordinates) {
  const auto nearestToken = findNearestToken(coordinates);

  if (!nearestToken.token) {
    return std::make_pair(false, "no tokens in the map.");
  }

  nearestToken.token->coordinates = coordinates;
  G_SetOrigin(nearestToken.token->entity, coordinates.data());

  saveTokens(_filepath);

  return std::make_pair(
      true, stringFormat("Moved %s #%d to new location.",
                         tokenDifficultyToString(nearestToken.difficulty),
                         nearestToken.number));
}

std::pair<bool, std::string>
Tokens::createToken(const Difficulty difficulty,
                    const std::array<float, 3> coordinates) {
  std::array<Token, MAX_TOKENS_PER_DIFFICULTY> *tokens = nullptr;

  switch (difficulty) {
    case Easy:
      tokens = &easyTokens;
      break;
    case Medium:
      tokens = &mediumTokens;
      break;
    case Hard:
      tokens = &hardTokens;
      break;
    default:
      return std::make_pair(false, "createToken: undefined difficulty.");
  }

  Token *nextFreeToken = nullptr;

  int idx = 0;
  for (auto &token : *tokens) {
    if (!token.isActive) {
      nextFreeToken = &token;
      break;
    }
    ++idx;
  }

  if (nextFreeToken == nullptr) {
    return std::make_pair(
        false, stringFormat("No free tokens left for '%s' difficulty.",
                            tokenDifficultyToString(difficulty)));
  }

  nextFreeToken->isActive = true;
  nextFreeToken->name = "";
  nextFreeToken->coordinates = coordinates;
  nextFreeToken->data->idx = idx;

  createEntity(*nextFreeToken, difficulty);

  if (!saveTokens(_filepath)) {
    return std::make_pair(
        false,
        "Could not save tokens to a file. Check logs for more information.");
  }

  return std::make_pair(true, "");
}

void Tokens::loadTokens(const std::string &filepath) {
  _filepath = filepath;

  if (!FileSystem::exists(_filepath)) {
    logger->info("No tokens configured for the current map.");
    return;
  }

  Json::Value root;

  if (!JsonUtils::readFile(_filepath, root, &errors)) {
    logger->error(errors);
    return;
  }

  const Json::Value eTokens = root["easyTokens"];
  const Json::Value mTokens = root["mediumTokens"];
  const Json::Value hTokens = root["hardTokens"];

  const auto printError = [&](const std::string &error) {
    logger->error("Could not load configuration from file '%s':", _filepath);
    logger->error(error);
  };

  int idx = 0;

  for (const auto &token : eTokens) {
    if (!easyTokens[idx].fromJson(token)) {
      printError(easyTokens[idx].errors);
      return;
    }

    easyTokens[idx].data->idx = idx;
    ++idx;
  }

  idx = 0;
  for (const auto &token : mTokens) {
    if (!mediumTokens[idx].fromJson(token)) {
      printError(mediumTokens[idx].errors);
      return;
    }

    mediumTokens[idx].data->idx = idx;
    ++idx;
  }

  idx = 0;
  for (const auto &token : hTokens) {
    if (!hardTokens[idx].fromJson(token)) {
      printError(hardTokens[idx].errors);
      return;
    }

    hardTokens[idx].data->idx = idx;
    ++idx;
  }

  createEntities();
  logger->info(
      "Successfully loaded tokens all tokens from '%s' for the current map.",
      _filepath);
}

bool Tokens::allTokensCollected(gentity_t *ent) {
  const auto tokenCounts = getTokenCounts();

  auto easyCount = 0;
  auto mediumCount = 0;
  auto hardCount = 0;
  for (auto i = 0; i < ETJump::MAX_TOKENS_PER_DIFFICULTY; ++i) {
    if (ent->client->pers.collectedEasyTokens[i]) {
      ++easyCount;
    }

    if (ent->client->pers.collectedMediumTokens[i]) {
      ++mediumCount;
    }

    if (ent->client->pers.collectedHardTokens[i]) {
      ++hardCount;
    }
  }

  return tokenCounts[0] == easyCount && tokenCounts[1] == mediumCount &&
         tokenCounts[2] == hardCount;
}

void Tokens::tokenTouch(gentity_t *self, gentity_t *other, trace_t *trace) {
  if (!other->client || other->client->ps.pm_type == PM_NOCLIP) {
    return;
  }

  const char *difficulty;
  bool *collected = nullptr;

  switch (self->tokenInformation->difficulty) {
    case Easy:
      difficulty = "^2easy";
      collected =
          &other->client->pers.collectedEasyTokens[self->tokenInformation->idx];
      break;
    case Medium:
      difficulty = "^3medium";
      collected = &other->client->pers
                       .collectedMediumTokens[self->tokenInformation->idx];
      break;
    case Hard:
      difficulty = "^1hard";
      collected =
          &other->client->pers.collectedHardTokens[self->tokenInformation->idx];
      break;
    default:
      return;
  }

  if (*collected) {
    return;
  }

  *collected = qtrue;
  Printer::popup(other, va("^7You collected %s ^7token ^5#%d", difficulty,
                           self->tokenInformation->idx + 1));

  if (allTokensCollected(other)) {
    const std::string &time = millisToString(
        level.time - other->client->pers.tokenCollectionStartTime);
    Printer::popupAll(va("%s ^7collected all tokens in %s",
                         other->client->pers.netname, time.c_str()));
  }
}

void Tokens::createEntity(Token &token, const Difficulty difficulty) const {
  token.entity = G_Spawn();
  token.entity->tokenInformation = token.data.get();
  Q_strncpyz(token.data->name, token.name.c_str(), sizeof(token.data->name));
  token.data->difficulty = difficulty;
  token.entity->r.contents = CONTENTS_TRIGGER;

  switch (difficulty) {
    case Easy:
      token.entity->classname = "token_easy";
      token.entity->s.eType = ET_TOKEN_EASY;
      break;
    case Medium:
      token.entity->classname = "token_medium";
      token.entity->s.eType = ET_TOKEN_MEDIUM;
      break;
    case Hard:
      token.entity->classname = "token_hard";
      token.entity->s.eType = ET_TOKEN_HARD;
      break;
    default:
      logger->error("createEntity: unknown token difficulty.");
      G_FreeEntity(token.entity);
      return;
  }

  token.entity->touch = tokenTouch;

  G_SetOrigin(token.entity, token.coordinates.data());
  VectorSet(token.entity->r.mins, -TOKEN_RADIUS, -TOKEN_RADIUS, 0);
  VectorSet(token.entity->r.maxs, TOKEN_RADIUS, TOKEN_RADIUS, TOKEN_RADIUS);

  trap_LinkEntity(token.entity);
}

std::array<int, 3> Tokens::getTokenCounts() {
  std::array<int, 3> ret{};

  for (const auto &t : easyTokens) {
    if (t.isActive) {
      ++ret[0];
    }
  }
  for (const auto &t : mediumTokens) {
    if (t.isActive) {
      ++ret[1];
    }
  }
  for (const auto &t : hardTokens) {
    if (t.isActive) {
      ++ret[2];
    }
  }

  return ret;
}

void Tokens::reset() {
  for (auto &t : easyTokens) {
    t.isActive = false;
  }
  for (auto &t : mediumTokens) {
    t.isActive = false;
  }
  for (auto &t : hardTokens) {
    t.isActive = false;
  }
}

void Tokens::createEntities() {
  for (auto &t : easyTokens) {
    if (t.isActive) {
      createEntity(t, Easy);
    }
  }
  for (auto &t : mediumTokens) {
    if (t.isActive) {
      createEntity(t, Medium);
    }
  }
  for (auto &t : hardTokens) {
    if (t.isActive) {
      createEntity(t, Hard);
    }
  }
}

bool Tokens::saveTokens(const std::string &filepath) {
  Json::Value root;
  root["easyTokens"] = Json::arrayValue;
  root["mediumTokens"] = Json::arrayValue;
  root["hardTokens"] = Json::arrayValue;

  for (const auto &token : easyTokens) {
    if (token.isActive) {
      root["easyTokens"].append(token.toJson());
    }
  }

  for (const auto &token : mediumTokens) {
    if (token.isActive) {
      root["mediumTokens"].append(token.toJson());
    }
  }

  for (const auto &token : hardTokens) {
    if (token.isActive) {
      root["hardTokens"].append(token.toJson());
    }
  }

  if (!JsonUtils::writeFile(filepath, root, &errors)) {
    logger->error("Could not save tokens to file '%': ", errors);
    return false;
  }

  logger->info("Saved all tokens to '%s'.", filepath);
  return true;
}

bool Tokens::Token::fromJson(const Json::Value &json) {
  if (json["coordinates"].size() != 3) {
    errors = "Coordinates array should have 3 items.";
    return false;
  }

  for (int i = 0; i < 3; i++) {
    if (!JsonUtils::parseValue(coordinates[i], json["coordinates"][i], &errors,
                               "coordinates")) {
      return false;
    }
  }

  if (!JsonUtils::parseValue(name, json["name"], &errors, "name")) {
    return false;
  }

  return isActive = true;
}

Json::Value Tokens::Token::toJson() const {
  Json::Value jsonToken;
  jsonToken["coordinates"] = Json::arrayValue;
  jsonToken["coordinates"].append(coordinates[0]);
  jsonToken["coordinates"].append(coordinates[1]);
  jsonToken["coordinates"].append(coordinates[2]);

  jsonToken["name"] = name;

  return jsonToken;
}
} // namespace ETJump
