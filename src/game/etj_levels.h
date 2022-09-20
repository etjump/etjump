/*
 * MIT License
 *
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
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

#ifndef LEVELS_HPP
#define LEVELS_HPP

// This messes up mainext.cpp if its defined twice.
#ifndef g_local_hpp__
struct gentity_s;
typedef gentity_s gentity_t;
#endif

#include <string>
#include <vector>
#include <memory>

class Levels {
public:
  struct Level {
    Level(int level, const std::string &name, const std::string &greeting,
          const std::string &commands);

    int level;
    std::string name;
    std::string commands;
    std::string greeting;
  };

  Levels();
  ~Levels();

  bool Add(int level, const std::string name, const std::string commands,
           const std::string greeting);
  bool Edit(int level, std::string const &name, std::string const &commands,
            std::string const &greeting, int updated);
  bool Delete(int level);
  bool ReadFromConfig();
  static bool SortByLevel(const std::shared_ptr<Level> &lhs,
                          const std::shared_ptr<Level> &rhs);
  bool WriteToConfig();
  std::string ErrorMessage() const;
  void PrintLevels();
  const Level *GetLevel(int level);
  bool LevelExists(int level) const;
  void PrintLevelInfo(gentity_t *ent);
  void PrintLevelInfo(gentity_t *ent, int level);

private:
  typedef std::vector<std::shared_ptr<Level>>::const_iterator ConstIter;
  typedef std::vector<std::shared_ptr<Level>>::iterator Iter;
  bool CreateDefaultLevels();
  ConstIter FindConst(int level);
  Iter Find(int level);
  std::vector<std::shared_ptr<Level>> levels_;
  std::string errorMessage;
  std::shared_ptr<Level> dummyLevel_;
};

#endif
