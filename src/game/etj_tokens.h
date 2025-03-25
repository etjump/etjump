/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

#pragma once

#include <string>
#include <array>
#include "json/json-forwards.h"
#include <memory>
#include "etj_log.h"

typedef struct TokenInformation_s TokenInformation;

namespace ETJump {
class Tokens {
public:
  explicit Tokens(std::unique_ptr<Log> log);
  ~Tokens() = default;

  enum Difficulty { Easy, Medium, Hard };

  // the entity is drawn as 32x32, but this feels more natural with touching
  static constexpr float TOKEN_RADIUS = 8.0f;

  struct Token {
    Token()
        : coordinates{0.0f, 0.0f, 0.0f}, isActive(false), entity(nullptr),
          data(std::make_unique<TokenInformation>()) {}
    std::array<float, 3> coordinates;
    std::string name;
    bool isActive;
    gentity_t *entity;
    std::string errors;

    // Because we cannot capture values for the entity think lambda,
    // we must pass the data as a gentity pointer in gentity.
    // Because there is so much data stored, storing the same data for
    // every entity would be pretty pointless.
    // Only tokens have the data.
    std::unique_ptr<TokenInformation> data;
    Json::Value toJson() const;
    bool fromJson(const Json::Value &json);
  };

  std::pair<bool, std::string> createToken(Difficulty difficulty,
                                           std::array<float, 3> coordinates);
  struct NearestToken {
    int number;
    Token *token;
    float distance;
    Difficulty difficulty;
  };

  static NearestToken findNearestToken(std::array<float, 3> coordinates);
  std::pair<bool, std::string>
  moveNearestToken(std::array<float, 3> coordinates);
  std::pair<bool, std::string>
  deleteNearestToken(std::array<float, 3> coordinates);
  std::pair<bool, std::string> deleteToken(Difficulty difficulty, int index);

  void loadTokens(const std::string &filepath);
  bool saveTokens(const std::string &filepath);
  void createEntity(Token &token, Difficulty difficulty) const;
  static void tokenTouch(gentity_t *self, gentity_t *other, trace_t *trace);
  void createEntities();
  static void reset();
  static std::array<int, 3> getTokenCounts();
  static std::string tokenDifficultyToString(Tokens::Difficulty difficulty);

  static bool allTokensCollected(gentity_t *ent);

private:
  std::string _filepath;
  std::unique_ptr<Log> logger;
  std::string errors;
};
} // namespace ETJump
