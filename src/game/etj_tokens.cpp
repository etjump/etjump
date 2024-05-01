/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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

#include <memory>
#include "json/json.h"
#include "etj_utilities.h"
#include "etj_time_utilities.h"

namespace ETJump {
static std::array<Tokens::Token, MAX_TOKENS_PER_DIFFICULTY> easyTokens;
static std::array<Tokens::Token, MAX_TOKENS_PER_DIFFICULTY> mediumTokens;
static std::array<Tokens::Token, MAX_TOKENS_PER_DIFFICULTY> hardTokens;

std::string toString(Tokens::Difficulty difficulty) {
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
Tokens::findNearestToken(std::array<float, 3> coordinates) {
  auto idx = 0;
  auto tokenNum = 0;
  Token *token = nullptr;
  auto difficulty = Easy;
  float nearestDistance = 1 << 20;
  for (auto &t : easyTokens) {
    if (t.isActive) {
      auto newDistance =
          VectorDistance(t.coordinates.data(), coordinates.data());
      if (newDistance < nearestDistance) {
        nearestDistance = newDistance;
        token = &t;
        tokenNum = idx + 1;
        difficulty = Easy;
      }
    }
    ++idx;
  }

  idx = 0;
  for (auto &t : mediumTokens) {
    if (t.isActive) {
      auto newDistance =
          VectorDistance(t.coordinates.data(), coordinates.data());
      if (newDistance < nearestDistance) {
        nearestDistance = newDistance;
        token = &t;
        tokenNum = idx + 1;
        difficulty = Medium;
      }
    }
    ++idx;
  }

  idx = 0;
  for (auto &t : hardTokens) {
    if (t.isActive) {
      auto newDistance =
          VectorDistance(t.coordinates.data(), coordinates.data());
      if (newDistance < nearestDistance) {
        nearestDistance = newDistance;
        token = &t;
        tokenNum = idx + 1;
        difficulty = Hard;
      }
    }
    ++idx;
  }

  return NearestToken{tokenNum, token, nearestDistance, difficulty};
}

std::pair<bool, std::string> Tokens::deleteToken(Difficulty difficulty,
                                                 int index) {
  Token *token;
  try {
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
        throw std::runtime_error("deleteToken: undefined difficulty.\n");
    }
  } catch (const std::exception &e) {
    G_Error(e.what());
  }

  if (token->isActive) {
    token->isActive = false;
    G_FreeEntity(token->entity);
    token->entity = nullptr;
    saveTokens(_filepath);
    return std::make_pair(true, va("Successfully deleted %s token #%d",
                                   toString(difficulty).c_str(), index + 1));
  }
  return std::make_pair(false, va("%s token with number #%d does not exist.",
                                  toString(difficulty).c_str(), index + 1));
}

std::pair<bool, std::string>
Tokens::deleteNearestToken(std::array<float, 3> coordinates) {
  auto nearestToken = findNearestToken(coordinates);

  if (!nearestToken.token) {
    return std::make_pair(false, "no tokens in the map.");
  }

  nearestToken.token->isActive = false;
  saveTokens(_filepath);
  G_FreeEntity(nearestToken.token->entity);
  nearestToken.token->entity = nullptr;
  return std::make_pair(true, va("Deleted %s token #%d.",
                                 toString(nearestToken.difficulty).c_str(),
                                 nearestToken.number));
}

std::pair<bool, std::string>
Tokens::moveNearestToken(std::array<float, 3> coordinates) {
  auto nearestToken = findNearestToken(coordinates);

  if (!nearestToken.token) {
    return std::make_pair(false, "no tokens in the map.");
  }

  nearestToken.token->coordinates = coordinates;
  G_SetOrigin(nearestToken.token->entity, coordinates.data());

  saveTokens(_filepath);

  return std::make_pair(true, va("Moved %s #%d to new location.",
                                 toString(nearestToken.difficulty).c_str(),
                                 nearestToken.number));
}

std::pair<bool, std::string>
Tokens::createToken(Difficulty difficulty, std::array<float, 3> coordinates) {
  std::array<Token, MAX_TOKENS_PER_DIFFICULTY> *tokens;
  try {
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
        throw std::runtime_error("createToken: undefined difficulty.\n");
    }
  } catch (const std::exception &e) {
    G_Error(e.what());
  }

  Token *nextFreeToken = nullptr;
  auto idx = 0;
  for (auto &token : *tokens) {
    if (!token.isActive) {
      nextFreeToken = &token;
      break;
    }
    ++idx;
  }

  if (nextFreeToken == nullptr) {
    return std::make_pair(false, "no free tokens left for the difficulty.");
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

bool Tokens::loadTokens(const std::string &filepath) {
  _filepath = filepath;
  std::string content;
  try {
    content = Utilities::ReadFile(filepath);
  } catch (std::runtime_error &e) {
    Utilities::Logln(std::string("Tokens: Could not read file: ") + e.what());
    return false;
  }

  Json::Value root;
  Json::Reader reader;

  if (!reader.parse(content, root)) {
    Utilities::Logln("Tokens: Could not parse file \"" + filepath + "\".");
    Utilities::Logln("Tokens: " + reader.getFormattedErrorMessages());
    return false;
  }

  try {
    auto eTokens = root["easyTokens"];
    auto mTokens = root["mediumTokens"];
    auto hTokens = root["hardTokens"];

    auto idx = 0;
    for (const auto &token : eTokens) {
      easyTokens[idx].fromJson(token);
      easyTokens[idx].data->idx = idx;
      ++idx;
    }

    idx = 0;
    for (const auto &token : mTokens) {
      mediumTokens[idx].fromJson(token);
      mediumTokens[idx].data->idx = idx;
      ++idx;
    }

    idx = 0;
    for (const auto &token : hTokens) {
      hardTokens[idx].fromJson(token);
      hardTokens[idx].data->idx = idx;
      ++idx;
    }
  } catch (std::runtime_error &e) {
    Utilities::Logln(
        std::string("Tokens: Could not parse configuration from file \"" +
                    filepath + "\": ") +
        e.what());
    return false;
  }

  createEntities();

  Utilities::Logln("Tokens: Successfully loaded all tokens from \"" + filepath +
                   "\" for current map.");

  return true;
}

bool Tokens::allTokensCollected(gentity_t *ent) {
  auto tokenCounts = getTokenCounts();

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
  bool *collected;
  switch (self->tokenInformation->difficulty) {
    case Tokens::Difficulty::Easy:
      difficulty = "^2easy";
      collected =
          &other->client->pers.collectedEasyTokens[self->tokenInformation->idx];
      break;
    case Tokens::Difficulty::Medium:
      difficulty = "^3medium";
      collected = &other->client->pers
                       .collectedMediumTokens[self->tokenInformation->idx];
      break;
    case Tokens::Difficulty::Hard:
      difficulty = "^1hard";
      collected =
          &other->client->pers.collectedHardTokens[self->tokenInformation->idx];
      break;
    default:
      G_Error("tokenThink: undefined difficulty.\n");
      return;
  }

  if (*collected) {
    return;
  }

  *collected = qtrue;
  C_CPMTo(other, va("^7You collected %s ^7token ^5#%d", difficulty,
                    self->tokenInformation->idx + 1));

  if (allTokensCollected(other)) {
    const std::string &time = millisToString(
        level.time - other->client->pers.tokenCollectionStartTime);
    C_CPMAll(va("%s ^7collected all tokens in %s", other->client->pers.netname,
                time.c_str()));
  }
}

void Tokens::createEntity(Token &token, Difficulty difficulty) {
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
      G_Error("createEntity: unknown token difficulty.\n");
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

  Json::StyledWriter writer;
  const std::string &output = writer.write(root);
  try {
    Utilities::WriteFile(filepath, output);
  } catch (std::runtime_error &e) {
    Utilities::Logln(std::string("Tokens: Could not save tokens to file: ") +
                     e.what());
    return false;
  }

  Utilities::Logln("Tokens: Saved all tokens to \"" + filepath + "\"");

  return true;
}

void Tokens::Token::fromJson(const Json::Value &json) {
  if (json["coordinates"].size() != 3) {
    throw std::runtime_error("Coordinates array should have 3 items.");
  }
  coordinates[0] = json["coordinates"][0].asFloat();
  coordinates[1] = json["coordinates"][1].asFloat();
  coordinates[2] = json["coordinates"][2].asFloat();
  name = json["name"].asString();
  isActive = true;
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
