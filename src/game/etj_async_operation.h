/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#ifndef ASYNC_OPERATION_HH
#define ASYNC_OPERATION_HH

#include <sqlite3.h>
#include <string>
#include "etj_local.h"

// All async operation objects need to be dynamically allocated
class AsyncOperation {
public:
  AsyncOperation() : db_(NULL), stmt_(NULL) {}
  virtual ~AsyncOperation() {
    sqlite3_finalize(stmt_);
    stmt_ = NULL;
    int rc = sqlite3_close(db_);
    if (rc != SQLITE_OK) {
      G_LogPrintf("ERROR: COULDN'T CLOSE SQLITE FILE "
                  "HANDLE. CONTACT MOD "
                  "DEVELOPER! (%d): %s\n",
                  rc, sqlite3_errmsg(db_));
    }
    db_ = NULL;
  }

  // This is called to execute an async operation
  // Object is deleted on execute
  void RunAndDeleteObject();

  bool OpenDatabase(const std::string &database);
  bool PrepareStatement(const std::string &statement);
  bool BindInt(int index, int value);
  bool BindString(int index, const std::string &value);
  int GetParameterIndex(const std::string &param);
  bool ExecuteStatement();
  std::string GetMessage() const;

  void PrintOpenError(const std::string &operation);
  void PrintPrepareError(const std::string &operation);
  void PrintBindError(const std::string &operation);
  void PrintExecuteError(const std::string &operation);

protected:
  sqlite3_stmt *GetStatement();

private:
  // This is the actual operation
  virtual void Execute() = 0;

  // This is for pthread
  static void *StartThread(void *data);

  sqlite3 *db_;
  sqlite3_stmt *stmt_;
  std::string errorMessage_;
};

#endif
