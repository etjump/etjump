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

#include "etj_command_complete_ext.h"

namespace ETJump::CommandCompletions {
bool completeArgument() {
  // TODO: implement this - look for commands that want additional completions
  // in e.g. std::unordered_map<std::string, std::function>, where the key
  // is the command, and value is a function pointer to the completion
  // implementation. The implementation itself is responsible for providing
  // valid arguments, and calling 'trap_CommandComplete' for all the arguments
  // that are supported by the base command. The implementations may also call
  // additional completion requests for commands which take more than one
  // argument, and can do this contextually (e.g. if first argument is "foo",
  // provide a different set of completions for second argument).
  // The syscall itself does not give context on which command is requesting
  // autocomplete, we must call 'CG_Argv' here first to see which completion
  // we're looking for.

  return false;
}
} // namespace ETJump::CommandCompletions
