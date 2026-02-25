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

#pragma once

#include <memory>
#include <vector>

namespace ETJump {
class CvarUpdateHandler;

class LeavesRemapper {
  const char *shaderName{"__etjump-leaves-shader__"};
  std::vector<const char *> leavesShaders{
      "models/mapobjects/trees_sd/winterbranch01",
      "models/mapobjects/tree_temperate_sd/leaves_temperate1",
      "models/mapobjects/tree_temperate_sd/leaves_temperate2",
      "models/mapobjects/tree_temperate_sd/leaves_temperate3",
      "models/mapobjects/tree_desert_sd/palm_leaf1",
      "models/mapobjects/plants_sd/bush_desert1",
      "models/mapobjects/plants_sd/bush_desert2",
      "models/mapobjects/plants_sd/bush_snow1",
      "models/mapobjects/plants_sd/catail1",
      "models/mapobjects/plants_sd/catail2",
      "models/mapobjects/plants_sd/deadbranch1",
      "models/mapobjects/plants_sd/deadbranch1_damp",
      "models/mapobjects/plants_sd/deadbranch2",
      "models/mapobjects/plants_sd/deadbranch3",
      "models/mapobjects/plants_sd/grassfoliage1",
      "models/mapobjects/plants_sd/grass_dry3",
      "models/mapobjects/plants_sd/grass_green1",
  };

  std::shared_ptr<CvarUpdateHandler> cvarUpdate;

  void turnOnLeaves();
  void turnOffLeaves();

public:
  explicit LeavesRemapper(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~LeavesRemapper();
};
} // namespace ETJump
