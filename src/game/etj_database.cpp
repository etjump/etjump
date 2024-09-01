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

#include "etj_database.h"
#include "utilities.hpp"
#include "etj_string_utilities.h"
#include "etj_printer.h"
#include "etj_session.h"
#include <iostream>

Database::Database() {}

Database::~Database() {}

Database::Users::iterator Database::GetUser(unsigned id) {
  return users_.find(id);
}

Database::Users::const_iterator Database::GetUserConst(unsigned id) const {
  return users_.find(id);
}

bool Database::PrepareStatement(const char *query, sqlite3_stmt **stmt) {
  unsigned rc = sqlite3_prepare_v2(db_, query, -1, stmt, 0);

  if (rc != SQLITE_OK) {
    message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
    return false;
  }

  return true;
}

bool Database::BindInt(sqlite3_stmt *stmt, int index, int val) {
  int rc = sqlite3_bind_int(stmt, index, val);
  if (rc != SQLITE_OK) {
    message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
    return false;
  }
  return true;
}

bool Database::BindString(sqlite3_stmt *stmt, int index,
                          const std::string &val) {
  int rc =
      sqlite3_bind_text(stmt, index, val.c_str(), val.length(), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
    return false;
  }
  return true;
}

bool Database::AddBanToSQLite(Ban ban) {
  AddBanOperation *addBan = new AddBanOperation(ban);
  addBan->RunAndDeleteObject();
  return true;
  //    int rc = 0;
  //    sqlite3_stmt *stmt = NULL;
  //    // TODO: do this once and finalize in game shutdown
  //    if (!PrepareStatement("INSERT INTO bans (name, guid, hwid, ip,
  //    banned_by, ban_date, expires, reason) VALUES (?, ?, ?, ?, ?, ?,
  //    ?, ?);", &stmt))
  //    {
  //        return false;
  //    }
  //
  //
  //    if (!BindString(stmt, 1, ban->name) ||
  //        !BindString(stmt, 2, ban->guid) ||
  //        !BindString(stmt, 3, ban->hwid) ||
  //        !BindString(stmt, 4, ban->ip) ||
  //        !BindString(stmt, 5, ban->bannedBy) ||
  //        !BindString(stmt, 6, ban->banDate) ||
  //        !BindInt(stmt, 7, ban->expires) ||
  //        !BindString(stmt, 8, ban->reason))
  //    {
  //        sqlite3_finalize(stmt);
  //        return false;
  //    }
  //
  //    rc = sqlite3_step(stmt);
  //    if (rc != SQLITE_DONE)
  //    {
  //        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
  //        sqlite3_finalize(stmt);
  //        return false;
  //    }
  //
  //    sqlite3_finalize(stmt);
  //    return true;
}

bool Database::AddUserToSQLite(User user) {
  InsertUserOperation *insert = new InsertUserOperation(user);
  insert->RunAndDeleteObject();
  return true;
  //    int rc = 0;
  //    sqlite3_stmt *stmt = NULL;
  //    // TODO: do this once and finalize in game shutdown
  //    if (!PrepareStatement("INSERT INTO users (id, guid, level,
  //    lastSeen, name, hwid, title, commands, greeting) VALUES (?, ?,
  //    ?, ?, ?, ?, ?, ?,
  //    ?);", &stmt))
  //    {
  //        return false;
  //    }
  //
  //
  //    G_DPrintf("HWIDS: %s\n", boost::algorithm::join(user->hwids,
  //    ",").c_str()); std::string hwids =
  //    boost::algorithm::join(user->hwids,
  //    ",");
  //
  //    if (!BindInt(stmt, 1, user->id) ||
  //        !BindString(stmt, 2, user->guid) ||
  //        !BindInt(stmt, 3, user->level) ||
  //        !BindInt(stmt, 4, user->lastSeen) ||
  //        !BindString(stmt, 5, user->name) ||
  //        !BindString(stmt, 6, hwids) ||
  //        !BindString(stmt, 7, user->title) ||
  //        !BindString(stmt, 8, user->commands) ||
  //        !BindString(stmt, 9, user->greeting)
  //        )
  //    {
  //        return false;
  //    }
  //
  //    rc = sqlite3_step(stmt);
  //    if (rc != SQLITE_DONE)
  //    {
  //        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
  //        return false;
  //    }
  //
  //    sqlite3_finalize(stmt);
  //
  //    return true;
}

unsigned Database::GetHighestFreeId() const {
  // if it's empty, let's start from 1
  if (users_.empty()) {
    return 1;
  }
  return users_.rbegin()->second->id + 1;
}

bool Database::UserExists(std::string const &guid) {
  auto user = std::find_if(users_.begin(), users_.end(),
                           [&guid](const std::pair<int, User> &u) -> bool {
                             return u.second->guid == guid;
                           });
  return user != users_.end();
}

// bool Database::ExecuteQueuedOperations()
//{
//     G_DPrintf("Executing %d queued database operations.\n",
//     databaseOperations_.size());
//     std::vector<boost::shared_ptr<DatabaseOperation> >::iterator it =
//         databaseOperations_.begin();
//     std::vector<boost::shared_ptr<DatabaseOperation> >::iterator end =
//         databaseOperations_.end();
//
//     sqlite3_exec(db_, "BEGIN TRANSACTION;", NULL, NULL, NULL);
//
//     while (it != end)
//     {
//         it->get()->Execute();
//         it++;
//     }
//
//     int rc = sqlite3_exec(db_, "END TRANSACTION;", NULL, NULL, NULL);
//     if (rc != SQLITE_OK)
//     {
//         G_LogPrintf("ERROR: failed to execute %d queued database operations:
//         %s\n",
//             databaseOperations_.size(), sqlite3_errmsg(db_));
//     }
//
//     databaseOperations_.clear();
//
//     return true;
// }

bool Database::UserInfo(gentity_t *ent, int id) {
  auto user = GetUserConst(id);
  const int clientNum = ClientNum(ent);

  if (user == users_.end()) {
    Printer::SendChatMessage(clientNum, "^3userinfo: ^7no user found with id " +
                                            std::to_string(id));
    return false;
  }

  Printer::SendChatMessage(clientNum,
                           "^3userinfo: ^7check console for more information.");

  std::string ip = "N/A";
  gentity_t *other = ETJump::session->gentityFromId(id);

  if (other) {
    std::string ipport = ValueForKey(other, "ip");
    std::string::size_type pos = ipport.find(':');
    ip = ipport.substr(0, pos);
  }

  std::string msg = ETJump::stringFormat(
      "^5ID: ^7%d\n"
      "^5GUID: ^7%s\n"
      "^5IP: ^7%s\n"
      "^5Level: ^7%d\n"
      "^5Last seen:^7 %s\n"
      "^5Name: ^7%s\n"
      "^5Title: ^7%s\n"
      "^5Commands: ^7%s\n"
      "^5Greeting: ^7%s\n",
      user->second->id, user->second->guid, ip, user->second->level,
      TimeStampToString(user->second->lastSeen), user->second->name,
      user->second->title, user->second->commands, user->second->greeting);

  Printer::SendConsoleMessage(clientNum, std::move(msg));
  return true;
}

bool Database::ListUsers(gentity_t *ent, int page) {
  const int USERS_PER_PAGE = 20;
  int size = users_.size();
  int pages = (size / USERS_PER_PAGE) + 1;
  int i = (page - 1) * USERS_PER_PAGE;

  if (page > pages) {
    ChatPrintTo(ent, "^3listusers: ^7no page #" + std::to_string(page));
    return false;
  }

  auto it = users_.begin();
  auto end = users_.end();

  ChatPrintTo(ent, "^3listusers: ^7check console for more information.");
  BeginBufferPrint();

  BufferPrint(ent, va("Listing page %d/%d\n", page, pages));
  int curr = 0;
  time_t t;
  time(&t);
  BufferPrint(ent, va("^7%-5s %-10s %-15s %-36s\n", "ID", "Level", "Last seen",
                      "Name"));
  while (it != end) {
    if (curr >= i && curr < i + USERS_PER_PAGE) {
      BufferPrint(ent,
                  va("^7%-5d %-10d %-15s %-36s\n", it->second->id,
                     it->second->level,
                     (TimeStampDifferenceToString(static_cast<unsigned>(t) -
                                                  it->second->lastSeen) +
                      " ago")
                         .c_str(),
                     it->second->name.c_str()));
    }

    curr++;
    it++;
  }
  FinishBufferPrint(ent, false);

  return true;
}

bool Database::RemoveBanFromSQLite(unsigned id) {
  RemoveBanOperation *remove = new RemoveBanOperation(id);
  remove->RunAndDeleteObject();
  return true;
  //    sqlite3_stmt *stmt = NULL;
  //    if (!PrepareStatement("DELETE FROM bans WHERE id=?;", &stmt))
  //    {
  //        return false;
  //    }
  //
  //    if (!BindInt(stmt, 1, id))
  //    {
  //        return false;
  //    }
  //
  //    int rc = sqlite3_step(stmt);
  //    if (rc != SQLITE_DONE)
  //    {
  //        message_ = sqlite3_errmsg(db_);
  //        return false;
  //    }
  //    return true;
}

bool Database::Unban(gentity_t *ent, int id) {
  for (unsigned i = 0, len = bans_.size(); i < len; i++) {
    if (bans_[i]->id == (unsigned)id) {
      bans_.erase(bans_.begin() + i);

      RemoveBanFromSQLite(id);

      //            if (!InstantSync())
      //            {
      //                databaseOperations_.push_back(UnbanOperationPtr(new
      //                UnbanOperation(this, id)));
      //            }
      //            else
      //            {
      //                RemoveBanFromSQLite(id);
      //            }

      return true;
    }
  }
  message_ = "no ban with id " + std::to_string(id);
  return false;
}

bool Database::ListBans(gentity_t *ent, int page) {
  const int BANS_PER_PAGE = 10;
  // 0-19, 20-39
  int i = (page - 1) * BANS_PER_PAGE;
  int printed = 0;
  int size = bans_.size();
  int pages = (size / BANS_PER_PAGE) + 1;

  if (page > pages) {
    ChatPrintTo(ent, "^3listbans: ^7no page #" + std::to_string(page));
  }

  ChatPrintTo(ent, "^3listbans: ^7check console for more information.");
  BeginBufferPrint();
  BufferPrint(ent, va("^7Showing page %d/%d\n", page, pages));
  for (; i < size; i++, printed++) {
    if (printed == BANS_PER_PAGE) {
      break;
    }
    BufferPrint(ent, va("%d %s ^7%s %s ^7%s %s\n", bans_[i]->id,
                        bans_[i]->name.c_str(), bans_[i]->banDate.c_str(),
                        bans_[i]->bannedBy.c_str(),
                        bans_[i]->expires != 0
                            ? TimeStampToString(bans_[i]->expires).c_str()
                            : "PERMANENTLY",
                        bans_[i]->reason.c_str()));
  }
  FinishBufferPrint(ent, false);
  return true;
}

bool Database::IsBanned(std::string const &guid, std::string const &hwid) {
  for (unsigned i = 0; i < bans_.size(); i++) {
    if ((bans_[i]->guid.length() > 0 && bans_[i]->guid == guid) ||
        (bans_[i]->hwid.length() > 0 && bans_[i]->hwid == hwid)) {
      return true;
    }
  }
  return false;
}

bool Database::IsIpBanned(std::string const &ip) {
  for (unsigned i = 0; i < bans_.size(); i++) {
    if (bans_[i]->ip.length() > 0 && bans_[i]->ip == ip) {
      return true;
    }
  }
  return false;
}

bool Database::BanUser(std::string const &name, std::string const &guid,
                       std::string const &hwid, std::string const &ip,
                       std::string const &bannedBy, std::string const &banDate,
                       unsigned expires, std::string const &reason) {
  Ban newBan(new Ban_s);

  newBan->name = name;
  newBan->guid = guid;
  newBan->hwid = hwid;
  newBan->ip = ip;
  newBan->bannedBy = bannedBy;
  newBan->banDate = banDate;
  newBan->expires = expires;
  newBan->reason = reason;

  AddBanToSQLite(newBan);

  //    // If this is set to 0, we put the bans into a queue and
  //    // add those bans to database once the server is empty or
  //    // the map changes
  //    if (!InstantSync())
  //    {
  //        databaseOperations_.push_back(BanUserOperationPtr(new
  //        BanUserOperation(this, newBan)));
  //    }
  //    else
  //    {
  //        if (!AddBanToSQLite(newBan))
  //        {
  //            return false;
  //        }
  //    }

  bans_.push_back(newBan);

  return true;
}

bool Database::UserExists(unsigned id) {
  auto user = GetUserConst(id);
  if (user != users_.end()) {
    return true;
  }
  return false;
}

void Database::NewName(int id, std::string const &name) {
  SaveNameOperation *op = new SaveNameOperation(name, id);
  op->RunAndDeleteObject();
  return;
}

bool Database::UpdateUser(gentity_t *ent, int id, std::string const &commands,
                          std::string const &greeting, std::string const &title,
                          int updated) {
  auto user = GetUser(id);
  if (user != users_.end()) {
    if (updated & Updated::COMMANDS) {
      user->second->commands = commands;
    }
    if (updated & Updated::GREETING) {
      user->second->greeting = greeting;
    }
    if (updated & Updated::TITLE) {
      user->second->title = title;
    }

    return Save(user->second, updated);
  }

  message_ = "Couldn't find user with id " + std::to_string(id);
  return false;
}

void Database::ListUserNames(gentity_t *ent, int id) {
  ListUserNamesOperation *listUserNamesOperation =
      new ListUserNamesOperation(ent, id);
  listUserNamesOperation->RunAndDeleteObject();
}

void Database::FindUser(gentity_t *ent, std::string const &user) {
  FindUserOperation *findUserOperation = new FindUserOperation(ent, user);
  findUserOperation->RunAndDeleteObject();
}

bool Database::UpdateLastSeenToSQLite(User user) {
  UpdateLastSeenOperation *updateLastSeenOperation =
      new UpdateLastSeenOperation(user);
  updateLastSeenOperation->RunAndDeleteObject();
  return true;
  //    sqlite3_stmt *stmt = NULL;
  //    if (!PrepareStatement("UPDATE users SET lastSeen=? WHERE id=?;",
  //    &stmt)
  //    ||
  //        !BindInt(stmt, 1, user->lastSeen) ||
  //        !BindInt(stmt, 2, user->id))
  //    {
  //        return false;
  //    }
  //
  //    int rc = sqlite3_step(stmt);
  //    if (rc != SQLITE_DONE)
  //    {
  //        message_ = "Failed to update user's last seen property.";
  //        return false;
  //    }
  //
  //    sqlite3_finalize(stmt);
  //    return true;
}

bool Database::UpdateLastSeen(int id, int lastSeen) {
  auto user = GetUser(id);
  if (user != users_.end()) {
    user->second->lastSeen = lastSeen;

    UpdateLastSeenToSQLite(user->second);

    //        if (!InstantSync())
    //        {
    //            databaseOperations_.push_back(UpdateLastSeenOperationPtr(new
    //            UpdateLastSeenOperation(this, *user)));
    //        }
    //        else
    //        {
    //            UpdateLastSeenToSQLite(*user);
    //        }

    return true;
  }
  message_ = "Couldn't find user with id " + std::to_string(id);
  return false;
}

bool Database::SetLevel(int id, int level) {
  auto user = GetUser(id);
  if (user != users_.end()) {
    user->second->level = level;

    //        if (!InstantSync())
    //        {
    //            databaseOperations_.push_back(SaveUserOperationPtr(new
    //            SaveUserOperation(this, *user,
    //            Updated::LEVEL))); return true;
    //        }
    return Save(user->second, Updated::LEVEL);
  }

  message_ = "Couldn't find user with id " + std::to_string(id);
  return false;
}

bool Database::Save(User user, unsigned updated) {
  AsyncSaveUserOperation *op = new AsyncSaveUserOperation(user, updated);
  op->RunAndDeleteObject();
  return true;
  //    std::vector<std::string> queryOptions;
  //    if (updated & Updated::COMMANDS)
  //    {
  //        queryOptions.push_back("commands=:commands");
  //    }
  //
  //    if (updated & Updated::GREETING)
  //    {
  //        queryOptions.push_back("greeting=:greeting");
  //    }
  //
  //    if (updated & Updated::LAST_SEEN)
  //    {
  //        queryOptions.push_back("lastSeen=:lastSeen");
  //    }
  //
  //    if (updated & Updated::LEVEL)
  //    {
  //        queryOptions.push_back("level=:level");
  //    }
  //
  //    if (updated & Updated::NAME)
  //    {
  //        queryOptions.push_back("name=:name");
  //    }
  //
  //    if (updated & Updated::TITLE)
  //    {
  //        queryOptions.push_back("title=:title");
  //    }
  //
  //    std::string query = "UPDATE users SET " +
  //    boost::join(queryOptions, ",
  //    ") + " WHERE id=:id;";
  //
  //    sqlite3_stmt *stmt = NULL;
  //    if (!PrepareStatement(query.c_str(), &stmt))
  //    {
  //        message_ = sqlite3_errmsg(db_);
  //        sqlite3_finalize(stmt);
  //        return false;
  //    }
  //
  //    if (updated & Updated::COMMANDS)
  //    {
  //        if (!BindString(stmt, sqlite3_bind_parameter_index(stmt,
  //        ":commands"), user->commands.c_str()))
  //        {
  //            sqlite3_finalize(stmt);
  //            return false;
  //        }
  //    }
  //
  //    if (updated & Updated::GREETING)
  //    {
  //        if (!BindString(stmt, sqlite3_bind_parameter_index(stmt,
  //        ":greeting"), user->greeting.c_str()))
  //        {
  //            sqlite3_finalize(stmt);
  //            return false;
  //        }
  //    }
  //
  //    if (updated & Updated::LAST_SEEN)
  //    {
  //        if (!BindInt(stmt, sqlite3_bind_parameter_index(stmt,
  //        ":lastSeen"), user->lastSeen))
  //        {
  //            sqlite3_finalize(stmt);
  //            return false;
  //        }
  //    }
  //
  //    if (updated & Updated::LEVEL)
  //    {
  //        if (!BindInt(stmt, sqlite3_bind_parameter_index(stmt,
  //        ":level"), user->level))
  //        {
  //            sqlite3_finalize(stmt);
  //            return false;
  //        }
  //    }
  //
  //    if (updated & Updated::NAME)
  //    {
  //        if (!BindString(stmt, sqlite3_bind_parameter_index(stmt,
  //        ":name"), user->name.c_str()))
  //        {
  //            sqlite3_finalize(stmt);
  //            return false;
  //        }
  //    }
  //
  //    if (updated & Updated::TITLE)
  //    {
  //        if (!BindString(stmt, sqlite3_bind_parameter_index(stmt,
  //        ":title"), user->title.c_str()))
  //        {
  //            sqlite3_finalize(stmt);
  //            return false;
  //        }
  //    }
  //
  //    if (!BindInt(stmt, sqlite3_bind_parameter_index(stmt, ":id"),
  //    user->id))
  //    {
  //        sqlite3_finalize(stmt);
  //        return false;
  //    }
  //
  //    int rc = 0;
  //    if (rc = sqlite3_step(stmt) != SQLITE_DONE)
  //    {
  //        G_LogPrintf("SQL ERROR: stepping failed when saving user
  //        (%d) %s\n", rc, sqlite3_errmsg(db_));
  //    }
  //
  //    sqlite3_finalize(stmt);
  //    return true;
}

bool Database::AddNewHWIDToDatabase(User user) {
  sqlite3_stmt *stmt = NULL;
  int rc = 0;

  if (!PrepareStatement("UPDATE users SET hwid=? WHERE id=?;", &stmt)) {
    return false;
  }

  std::string hwids = ETJump::StringUtil::join(user->hwids, ",");

  if (!BindString(stmt, 1, hwids) || !BindInt(stmt, 2, user->id)) {
    sqlite3_finalize(stmt);
    return false;
  }

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
    sqlite3_finalize(stmt);
    return false;
  }
  sqlite3_finalize(stmt);
  return true;
}

bool Database::AddNewHardwareId(int id, std::string const &hwid) {
  auto user = GetUser(id);

  if (user != users_.end()) {
    user->second->hwids.push_back(hwid);

    InsertNewHardwareIdOperation *op =
        new InsertNewHardwareIdOperation(user->second);
    op->RunAndDeleteObject();

    return true;
  }
  message_ = "Couldn't find user with id " + std::to_string(id);
  return false;
}

bool Database::AddUser(std::string const &guid, std::string const &hwid,
                       std::string const &name) {
  unsigned id = GetHighestFreeId();

  if (UserExists(guid)) {
    message_ = "User guid is not unique.";
    return false;
  }

  auto res = users_.emplace(id, std::make_shared<User_s>(id, guid, name, hwid));

  // already exists, no insertion
  if (!res.second) {
    return true;
  }

  if (!AddUserToSQLite(res.first->second)) {
    return false;
  }
  //
  //    if (!InstantSync())
  //    {
  //        databaseOperations_.push_back(AddUserOperationPtr(new
  //        AddUserOperation(this, newUser)));
  //    }
  //    else
  //    {
  //        if (!AddUserToSQLite(newUser))
  //        {
  //            return false;
  //        }
  //    }

  return true;
}

bool Database::CloseDatabase() {
  users_.clear();
  bans_.clear();
  return true;
}

User_s const *Database::GetUserData(unsigned id) const {
  auto user = users_.find(id);

  if (user == users_.end()) {
    return NULL;
  }

  return user->second.get();
}

bool Database::CreateNamesTable() {
  int rc = 0;
  char *errMsg = NULL;

  rc = sqlite3_exec(db_,
                    "CREATE TABLE IF NOT EXISTS name (id INTEGER PRIMARY KEY "
                    "AUTOINCREMENT, clean_name TEXT UNIQUE, name TEXT UNIQUE, "
                    "user_id INT, FOREIGN KEY (user_id) REFERENCES users(id));",
                    NULL, NULL, &errMsg);

  if (rc != SQLITE_OK) {
    message_ = std::string("SQL error: ") + errMsg;
    sqlite3_free(errMsg);
    sqlite3_close(db_);
    return false;
  }
  return true;
}

bool Database::LoadBans() {
  int rc = 0;
  sqlite3_stmt *stmt = NULL;

  // keep bans but ignore expired ones
  void ClearExpiredBans();
  if (!PrepareStatement("SELECT id, name, guid, hwid, ip, banned_by, ban_date, "
                        "expires, reason FROM bans WHERE expires = 0 OR "
                        "expires > STRFTIME('%s','now');",
                        &stmt)) {
    return false;
  }

  const char *val = NULL;
  rc = sqlite3_step(stmt);
  while (rc != SQLITE_DONE) {
    Ban newBan(new Ban_s());
    switch (rc) {
      case SQLITE_ROW:
        newBan->id = sqlite3_column_int(stmt, 0);
        val = (const char *)(sqlite3_column_text(stmt, 1));
        newBan->name = val ? val : "";
        val = (const char *)(sqlite3_column_text(stmt, 2));
        newBan->guid = val ? val : "";
        val = (const char *)(sqlite3_column_text(stmt, 3));
        newBan->hwid = val ? val : "";
        val = (const char *)(sqlite3_column_text(stmt, 4));
        newBan->ip = val ? val : "";
        val = (const char *)(sqlite3_column_text(stmt, 5));
        newBan->bannedBy = val ? val : "";
        val = (const char *)(sqlite3_column_text(stmt, 6));
        newBan->banDate = val ? val : "";
        newBan->expires = sqlite3_column_int(stmt, 7);
        val = (const char *)(sqlite3_column_text(stmt, 8));
        newBan->reason = val ? val : "";
        bans_.push_back(newBan);
        G_DPrintf("Ban: %s\n", newBan->ToChar());
        break;
      case SQLITE_BUSY:
      case SQLITE_ERROR:
      case SQLITE_MISUSE:
      default:
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        sqlite3_finalize(stmt);
        return false;
    }
    rc = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return true;
}

bool Database::LoadUsers() {
  int rc = 0;
  sqlite3_stmt *stmt = NULL;

  if (!PrepareStatement("SELECT id, guid, level, lastSeen, name, hwid, title, "
                        "commands, greeting FROM users;",
                        &stmt)) {
    return false;
  }

  const char *val = NULL;
  rc = sqlite3_step(stmt);
  while (rc != SQLITE_DONE) {
    User_s newUser;

    switch (rc) {
      case SQLITE_ROW:
        newUser.id = sqlite3_column_int(stmt, 0);
        val = (const char *)(sqlite3_column_text(stmt, 1));
        newUser.guid = val ? val : "";
        newUser.level = sqlite3_column_int(stmt, 2);
        newUser.lastSeen = sqlite3_column_int(stmt, 3);
        val = (const char *)(sqlite3_column_text(stmt, 4));
        newUser.name = val ? val : "";
        val = (const char *)(sqlite3_column_text(stmt, 5));
        if (val) {
          newUser.hwids = ETJump::StringUtil::split(val, ",");
        }
        val = (const char *)(sqlite3_column_text(stmt, 6));
        newUser.title = val ? val : "";
        val = (const char *)(sqlite3_column_text(stmt, 7));
        newUser.commands = val ? val : "";
        val = (const char *)(sqlite3_column_text(stmt, 8));
        newUser.greeting = val ? val : "";
        users_.emplace(newUser.id, std::make_shared<User_s>(newUser));
        G_DPrintf("User: %s\n", newUser.ToChar());
        break;
      case SQLITE_BUSY:
      case SQLITE_ERROR:
      case SQLITE_MISUSE:
      default:
        message_ = std::string("SQL error: ") + sqlite3_errmsg(db_);
        sqlite3_finalize(stmt);
        return false;
    }
    rc = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return true;
}

bool Database::CreateBansTable() {
  int rc = 0;
  char *errMsg = NULL;

  rc = sqlite3_exec(db_,
                    "CREATE TABLE IF NOT EXISTS bans (id INTEGER "
                    "PRIMARY KEY AUTOINCREMENT, "
                    "name TEXT, guid TEXT NOT NULL, hwid TEXT, ip "
                    "TEXT, banned_by TEXT, "
                    "ban_date TEXT, expires INT, reason TEXT);",
                    NULL, NULL, &errMsg);

  if (rc != SQLITE_OK) {
    message_ = std::string("SQL error: ") + errMsg;
    sqlite3_free(errMsg);
    sqlite3_close(db_);
    return false;
  }
  return true;
}

bool Database::CreateUsersTable() {
  int rc = 0;
  char *errMsg = NULL;

  rc =
      sqlite3_exec(db_,
                   "CREATE TABLE IF NOT EXISTS users (id INT PRIMARY KEY, guid "
                   "TEXT UNIQUE NOT NULL, level INT, lastSeen INT, name TEXT, "
                   "hwid TEXT, title TEXT, commands TEXT, greeting TEXT);",
                   NULL, NULL, &errMsg);

  if (rc != SQLITE_OK) {
    message_ = std::string("SQL error: ") + errMsg;
    sqlite3_free(errMsg);
    sqlite3_close(db_);
    return false;
  }
  return true;
}

std::string const Database::GetMessage() const { return message_; }

User_s const *Database::GetUserData(int id) const {
  auto user = GetUserConst(static_cast<unsigned>(id));
  if (user != users_.end()) {
    return user->second.get();
  }
  return NULL;
}

User_s const *Database::GetUserData(std::string const &guid) const {
  auto user = std::find_if(users_.begin(), users_.end(),
                           [&guid](const std::pair<int, User> &u) -> bool {
                             return u.second->guid == guid;
                           });
  return user->second.get();
}

bool Database::InitDatabase(char const *config) {
  int rc = sqlite3_open(GetPath(config).c_str(), &db_);

  users_.clear();
  bans_.clear();

  if (rc) {
    message_ = std::string("Can't open database: ") + sqlite3_errmsg(db_);
    sqlite3_close(db_);
    return false;
  }

  sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);

  if (!CreateUsersTable() || !CreateBansTable() || !CreateNamesTable()) {
    return false;
  }

  if (!LoadUsers() || !LoadBans()) {
    return false;
  }

  sqlite3_close(db_);
  db_ = NULL;

  return true;
}

Database::InsertUserOperation::InsertUserOperation(User user) : user_(user) {}

Database::InsertUserOperation::~InsertUserOperation() {}

void Database::InsertUserOperation::Execute() {
  if (!OpenDatabase(g_userConfig.string)) {
    G_LogPrintf("ERROR: failed to open user database for "
                "insert user operation. %s\n",
                GetMessage().c_str());
    return;
  }

  if (!PrepareStatement(
          "INSERT INTO users (id, guid, level, lastSeen, name, hwid, "
          "title, "
          "commands, greeting) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);")) {
    G_LogPrintf("ERROR: failed to prepare insert user "
                "operation statement. %s\n",
                GetMessage().c_str());
    return;
  }

  std::string hardwareIds = ETJump::StringUtil::join(user_->hwids, ",");

  if (!BindInt(1, user_->id) || !BindString(2, user_->guid) ||
      !BindInt(3, user_->level) || !BindInt(4, user_->lastSeen) ||
      !BindString(5, user_->name) || !BindString(6, hardwareIds) ||
      !BindString(7, user_->title) || !BindString(8, user_->commands) ||
      !BindString(9, user_->greeting)) {
    G_LogPrintf("ERROR: failed to bind value to insert user "
                "operation statement. %s\n",
                GetMessage().c_str());
    return;
  }

  if (!ExecuteStatement()) {
    G_LogPrintf("ERROR: failed to execute insert user operation. %s\n",
                GetMessage().c_str());
    return;
  }
}

Database::InsertNewHardwareIdOperation::InsertNewHardwareIdOperation(User user)
    : user_(user) {}

Database::InsertNewHardwareIdOperation::~InsertNewHardwareIdOperation() {}

void Database::InsertNewHardwareIdOperation::Execute() {
  if (!OpenDatabase(g_userConfig.string)) {
    G_LogPrintf("ERROR: couldn't open database for hardware id "
                "update operation. %s\n",
                GetMessage().c_str());
    return;
  }

  if (!PrepareStatement("UPDATE users SET hwid=? WHERE id=?;")) {
    G_LogPrintf("ERROR: failed to update user's hardware id. %s\n",
                GetMessage().c_str());
    return;
  }

  std::string hwids = ETJump::StringUtil::join(user_->hwids, ",");

  if (!BindString(1, hwids) || !BindInt(2, user_->id)) {
    G_LogPrintf("ERROR: failed to bind value to update user "
                "operation statement. %s\n",
                GetMessage().c_str());
    return;
  }

  if (!ExecuteStatement()) {
    G_LogPrintf("ERROR: failed to execute update user hardware "
                "id operation. %s\n",
                GetMessage().c_str());
    return;
  }
}

Database::AsyncSaveUserOperation::AsyncSaveUserOperation(User user, int updated)
    : user_(user), updated_(updated) {}

Database::AsyncSaveUserOperation::~AsyncSaveUserOperation() {}

void Database::AsyncSaveUserOperation::Execute() {
  if (updated_ == Updated::NONE) {
    return;
  }

  std::vector<std::string> queryOptions;
  if (updated_ & Updated::COMMANDS) {
    queryOptions.push_back("commands=:commands");
  }

  if (updated_ & Updated::GREETING) {
    queryOptions.push_back("greeting=:greeting");
  }

  if (updated_ & Updated::LAST_SEEN) {
    queryOptions.push_back("lastSeen=:lastSeen");
  }

  if (updated_ & Updated::LEVEL) {
    queryOptions.push_back("level=:level");
  }

  if (updated_ & Updated::NAME) {
    queryOptions.push_back("name=:name");
  }

  if (updated_ & Updated::TITLE) {
    queryOptions.push_back("title=:title");
  }

  std::string query = "UPDATE users SET " +
                      ETJump::StringUtil::join(queryOptions, ", ") +
                      " WHERE id=:id;";

  if (!OpenDatabase(g_userConfig.string)) {
    G_LogPrintf("ERROR: failed to open database on save user "
                "operation. %s\n",
                GetMessage().c_str());
    return;
  }

  if (!PrepareStatement(query)) {
    G_LogPrintf("ERROR: failed to prepare statement on save "
                "user operation. %s\n",
                GetMessage().c_str());
    return;
  }

  if (updated_ & Updated::COMMANDS) {
    if (!BindString(GetParameterIndex(":commands"), user_->commands)) {
      G_LogPrintf("ERROR: failed to bind value to save "
                  "user statement. %s\n",
                  GetMessage().c_str());
      return;
    }
  }

  if (updated_ & Updated::GREETING) {
    if (!BindString(GetParameterIndex(":greeting"), user_->greeting)) {
      G_LogPrintf("ERROR: failed to bind value to save "
                  "user statement. %s\n",
                  GetMessage().c_str());
      return;
    }
  }

  if (updated_ & Updated::LAST_SEEN) {
    if (!BindInt(GetParameterIndex(":lastSeen"), user_->lastSeen)) {
      G_LogPrintf("ERROR: failed to bind value to save "
                  "user statement. %s\n",
                  GetMessage().c_str());
      return;
    }
  }

  if (updated_ & Updated::LEVEL) {
    if (!BindInt(GetParameterIndex(":level"), user_->level)) {
      G_LogPrintf("ERROR: failed to bind value to save "
                  "user statement. %s\n",
                  GetMessage().c_str());
      return;
    }
  }

  if (updated_ & Updated::NAME) {
    if (!BindString(GetParameterIndex(":name"), user_->name)) {
      G_LogPrintf("ERROR: failed to bind value to save "
                  "user statement. %s\n",
                  GetMessage().c_str());
      return;
    }
  }

  if (updated_ & Updated::TITLE) {
    if (!BindString(GetParameterIndex(":title"), user_->title)) {
      G_LogPrintf("ERROR: failed to bind value to save "
                  "user statement. %s\n",
                  GetMessage().c_str());
      return;
    }
  }

  if (!BindInt(GetParameterIndex(":id"), user_->id)) {
    G_LogPrintf("ERROR: failed to bind value to save user "
                "statement. %s\n",
                GetMessage().c_str());
    return;
  }

  if (!ExecuteStatement()) {
    G_LogPrintf("ERROR: failed to execute save user statement. %s\n",
                GetMessage().c_str());
    return;
  }

  return;
}

Database::AddBanOperation::AddBanOperation(Ban ban) : ban_(ban) {}

Database::AddBanOperation::~AddBanOperation() {}

void Database::AddBanOperation::Execute() {
  if (!OpenDatabase(g_userConfig.string)) {
    G_LogPrintf("ERROR: failed to open database on add ban "
                "operation. %s\n",
                GetMessage().c_str());
    return;
  }

  if (!PrepareStatement("INSERT INTO bans (name, guid, hwid, ip, banned_by, "
                        "ban_date, "
                        "expires, reason) VALUES (?, ?, ?, ?, ?, ?, ?, ?);")) {
    G_LogPrintf("ERROR: failed to prepare add ban operation "
                "statement. %s\n",
                GetMessage().c_str());
    return;
  }

  if (!BindString(1, ban_->name) || !BindString(2, ban_->guid) ||
      !BindString(3, ban_->hwid) || !BindString(4, ban_->ip) ||
      !BindString(5, ban_->bannedBy) || !BindString(6, ban_->banDate) ||
      !BindInt(7, ban_->expires) || !BindString(8, ban_->reason)) {
    G_LogPrintf("ERROR: failed to bind value to add ban "
                "operation statement. %s\n",
                GetMessage().c_str());
    return;
  }

  if (!ExecuteStatement()) {
    G_LogPrintf("ERROR: failed to execute add ban operation. %s\n",
                GetMessage().c_str());
    return;
  }
}

Database::RemoveBanOperation::RemoveBanOperation(int id) : id_(id) {}

Database::RemoveBanOperation::~RemoveBanOperation() {}

void Database::RemoveBanOperation::Execute() {
  std::string op = "remove ban operation";
  if (!OpenDatabase(g_userConfig.string)) {
    PrintOpenError(op);
    return;
  }

  if (!PrepareStatement("DELETE FROM bans WHERE id=?;")) {
    PrintPrepareError(op);
    return;
  }

  if (!BindInt(1, id_)) {
    PrintBindError(op);
    return;
  }

  if (!ExecuteStatement()) {
    PrintExecuteError(op);
    return;
  }
}

Database::UpdateLastSeenOperation::UpdateLastSeenOperation(User user)
    : user_(user) {}

Database::UpdateLastSeenOperation::~UpdateLastSeenOperation() {}

void Database::UpdateLastSeenOperation::Execute() {
  std::string op = "update last seen operation";
  if (!OpenDatabase(g_userConfig.string)) {
    PrintOpenError(op);
    return;
  }

  if (!PrepareStatement("UPDATE users SET lastSeen=? WHERE id=?;")) {
    PrintPrepareError(op);
    return;
  }

  if (!BindInt(1, user_->lastSeen) || !BindInt(2, user_->id)) {
    PrintBindError(op);
    return;
  }

  if (!ExecuteStatement()) {
    PrintExecuteError(op);
    return;
  }
}

Database::FindUserOperation::FindUserOperation(gentity_t *ent,
                                               std::string const &user)
    : ent_(ent), user_(user) {}

Database::FindUserOperation::~FindUserOperation() {}

void Database::FindUserOperation::Execute() {
  std::string op = "find user operation";
  if (!OpenDatabase(g_userConfig.string)) {
    PrintOpenError(op);
    return;
  }

  if (!PrepareStatement("SELECT user_id, name FROM name WHERE clean_name LIKE "
                        "'%' || ? || '%' LIMIT(20);")) {
    PrintPrepareError(op);
    return;
  }

  if (!BindString(1, user_)) {
    PrintBindError(op);
    return;
  }

  sqlite3_stmt *stmt = GetStatement();
  int rc = SQLITE_OK;
  std::vector<std::pair<int, std::string>> users;
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    std::pair<int, std::string> user;
    user.first = sqlite3_column_int(stmt, 0);
    const char *val = (const char *)sqlite3_column_text(stmt, 1);
    user.second = val ? val : "";
    users.push_back(user);
  }

  if (users.size() == 0) {
    ChatPrintTo(ent_, "^3finduser: ^7no users found.");
    return;
  }

  ChatPrintTo(ent_, "^3finduser: ^7check console for more information.");
  BufferPrinter printer(ent_);
  printer.Begin();
  printer.Print("ID       Name\n");
  for (unsigned i = 0; i < users.size(); i++) {
    printer.Print(ETJump::stringFormat("%-8d %-36s^7\n", users[i].first,
                                       users[i].second));
  }
  printer.Finish(false);
}

Database::SaveNameOperation::SaveNameOperation(std::string const &name, int id)
    : name_(name), id_(id) {}

Database::SaveNameOperation::~SaveNameOperation() {}

void Database::SaveNameOperation::Execute() {
  std::string op = "save name operation";
  if (!OpenDatabase(g_userConfig.string)) {
    PrintOpenError(op);
    return;
  }

  if (!PrepareStatement("INSERT INTO name(clean_name, name, user_id) "
                        "VALUES(? , ? , ? );")) {
    PrintPrepareError(op);
    return;
  }

  std::string sanitizedName = ETJump::sanitize(name_, true);

  if (!BindString(1, sanitizedName) || !BindString(2, name_) ||
      !BindInt(3, id_)) {
    PrintBindError(op);
    return;
  }

  if (!ExecuteStatement()) {
    return;
  }
}

Database::ListUserNamesOperation::ListUserNamesOperation(gentity_t *ent, int id)
    : ent_(ent), id_(id) {}

Database::ListUserNamesOperation::~ListUserNamesOperation() {}

void Database::ListUserNamesOperation::Execute() {
  const std::string op = "list user names operation";
  if (!OpenDatabase(g_userConfig.string)) {
    PrintOpenError(op);
    return;
  }

  if (!PrepareStatement("SELECT name FROM name WHERE user_id=?;")) {
    PrintPrepareError(op);
    return;
  }

  if (!BindInt(1, id_)) {
    PrintBindError(op);
    return;
  }

  sqlite3_stmt *stmt = GetStatement();
  int rc = 0;
  std::vector<std::string> names;
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    const char *name = NULL;
    name = (const char *)sqlite3_column_text(stmt, 0);
    names.push_back(name ? name : "");
  }

  if (names.size() == 0) {
    ChatPrintTo(ent_, "^3listusernames: ^7couldn't find any names with id " +
                          std::to_string(id_));
  } else {
    ConsolePrintTo(ent_, "^3listusernames: ^7check console for "
                         "more information.");
    BufferPrinter printer(ent_);
    printer.Begin();
    printer.Print(ETJump::stringFormat("Found %d names with id: %d\n",
                                       names.size(), id_));
    for (unsigned i = 0; i < names.size(); i++) {
      printer.Print(names[i] + "\n");
    }
    printer.Finish(false);
  }
}

int Database::ResetUsersWithLevel(int level) {
  auto it = users_.begin();
  auto end = users_.end();

  int resetedUsersCount = 0;

  while (it != end) {
    if (it->second->level == level) {
      it->second->level = 0;
      resetedUsersCount++;
    }
    it++;
  }

  return resetedUsersCount;
}

Database::ResetUsersWithLevelOperation::ResetUsersWithLevelOperation(int level)
    : level_(level) {}

Database::ResetUsersWithLevelOperation::~ResetUsersWithLevelOperation() {}

void Database::ResetUsersWithLevelOperation::Execute() {
  std::string op = "Reset users with level -operation";
  if (!OpenDatabase(g_userConfig.string)) {
    PrintOpenError(op);
    return;
  }

  if (!PrepareStatement("UPDATE users SET level=0 WHERE level=?;")) {
    PrintPrepareError(op);
    return;
  }

  if (!BindInt(1, level_)) {
    PrintBindError(op);
    return;
  }

  if (!ExecuteStatement()) {
    return;
  }
}
