/*
 * MIT License
 * 
 * Copyright (c) 2021 ETJump team <zero@etjump.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef DATABASE_HH
#define DATABASE_HH
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/shared_ptr.hpp>
#include "etj_local.h"
#include <sqlite3.h>
#include <vector>
#include "etj_iauthentication.h"
#include "etj_async_operation.h"

using namespace boost::multi_index;

// Loads the data from SQLite database to memory as
// some servers don't support threads making it impossible
// to just query everything from database.

namespace Updated
{
const int NONE      = 0;
const int LEVEL     = 0x00001;
const int LAST_SEEN = 0x00002;
const int NAME      = 0x00004;
const int TITLE     = 0x00008;
const int COMMANDS  = 0x00010;
const int GREETING  = 0x00020;
}



struct Ban_s
{
	unsigned id;
	std::string name;
	std::string guid;
	std::string hwid;
	std::string ip;
	std::string bannedBy;
	std::string banDate;
	std::string reason;
	unsigned expires;

	Ban_s() : id(0), expires(0)
	{

	}

	const char *ToChar()
	{
		return va("%d %s %s %s %s %s %s %d %s", id, name.c_str(), guid.c_str(), hwid.c_str(), ip.c_str(), bannedBy.c_str(), banDate.c_str(), expires, reason.c_str());
	}
};

class DatabaseOperation;
class Database : public IAuthentication
{
public:
	Database();
	~Database();
	typedef boost::shared_ptr<User_s> User;

	typedef multi_index_container<
	        User,
	        indexed_by<
	            ordered_unique<mem_fun<User_s, unsigned, &User_s::GetId> >,
	            ordered_unique<mem_fun<User_s, std::string, &User_s::GetGuid> >
	            >
	        > Users;

	typedef Users::nth_index<0>::type::iterator IdIterator;
	typedef Users::nth_index<0>::type::const_iterator ConstIdIterator;
	typedef Users::nth_index<1>::type::iterator GuidIterator;
	typedef Users::nth_index<1>::type::const_iterator ConstGuidIterator;

	typedef boost::shared_ptr<Ban_s> Ban;

	const User_s *GetUserData(unsigned id) const;

	/**
	 * IAuthentication
	 */

	const std::string GetMessage() const;
	bool AddUser(const std::string& guid, const std::string& hwid, const std::string& name);
	bool AddNewHardwareId(int id, const std::string& hwid);
	bool BanUser(const std::string& name, const std::string& guid,
	             const std::string& hwid, const std::string& ip, const std::string& bannedBy,
	             const std::string& banDate, unsigned expires, const std::string& reason);
	bool IsBanned(const std::string& guid, const std::string& hwid);
	bool IsIpBanned(const std::string& ip);
	bool UserExists(unsigned id);
	bool UserExists(const std::string& guid);
	const User_s *GetUserData(const std::string& guid) const;
	const User_s *GetUserData(int id) const;
	bool SetLevel(int id, int level);
	void NewName(int id, const std::string& name);
	bool UpdateLastSeen(int id, int lastSeen);

	/**
	 * End of IAuthentication
	 */

	bool CreateUsersTable();
	bool CreateBansTable();
	bool LoadUsers();
	bool LoadBans();

	bool CreateNamesTable();
	bool InitDatabase(const char *config);
	bool CloseDatabase();
	// When user is added, all we have is the guid, hwid, name, lastSeen and level
	// Adds user to database
	bool Save(User user, unsigned updated);
	bool Save(IdIterator user, unsigned updated);



	bool ListBans(gentity_t *ent, int page);
	bool Unban(gentity_t *ent, int id);
	bool ListUsers(gentity_t *ent, int page);
	bool UserInfo(gentity_t *ent, int id);
//    bool ExecuteQueuedOperations();
	bool AddUserToSQLite(User user);
	bool AddBanToSQLite(Ban ban);
	bool AddNewHWIDToDatabase(User user);
	bool RemoveBanFromSQLite(unsigned id);
	bool UpdateLastSeenToSQLite(User user);
	void FindUser(gentity_t *ent, const std::string& user);
	void ListUserNames(gentity_t *ent, int id);
	bool UpdateUser(gentity_t *ent, int id,
	                const std::string& commands, const std::string& greeting,
	                const std::string& title, int updated);
	int ResetUsersWithLevel(int level);
private:
	unsigned GetHighestFreeId() const;

	bool BindInt(sqlite3_stmt *stmt, int index, int val);
	bool BindString(sqlite3_stmt *stmt, int index, const std::string& val);
	IdIterator GetUser(unsigned id) const;
	ConstIdIterator GetUserConst(unsigned id) const;
	GuidIterator GetUser(const std::string& guid) const;
	bool PrepareStatement(char const *query, sqlite3_stmt **stmt);
	ConstGuidIterator GetUserConst(const std::string& guid) const;
	bool InstantSync() const;
	Users            users_;
	std::vector<Ban> bans_;
	sqlite3          *db_;
	std::string      message_;

	ConstIdIterator IdIterEnd() const;
	ConstGuidIterator GuidIterEnd() const;

	// If instant database sync is disabled, all the
	// database operations needed are added to this queue
	std::vector<boost::shared_ptr<DatabaseOperation> > databaseOperations_;

	class InsertUserOperation : public AsyncOperation
	{
public:
		InsertUserOperation(User user);
		~InsertUserOperation();
private:
		User user_;
		void Execute();
	};

	class InsertNewHardwareIdOperation : public AsyncOperation
	{
public:
		InsertNewHardwareIdOperation(User user);
		~InsertNewHardwareIdOperation();
private:
		User user_;
		void Execute();
	};

	class AsyncSaveUserOperation : public AsyncOperation
	{
public:
		AsyncSaveUserOperation(User user, int updated);
		~AsyncSaveUserOperation();
private:
		User user_;
		int  updated_;
		void Execute();
	};

	class AddBanOperation : public AsyncOperation
	{
public:
		AddBanOperation(Ban ban);
		~AddBanOperation();
private:
		Ban ban_;
		void Execute();
	};

	class RemoveBanOperation : public AsyncOperation
	{
public:
		RemoveBanOperation(int id);
		~RemoveBanOperation();
private:
		int id_;
		void Execute();
	};

	class UpdateLastSeenOperation : public AsyncOperation
	{
public:
		UpdateLastSeenOperation(User user);
		~UpdateLastSeenOperation();
private:
		User user_;
		void Execute();
	};

	class FindUserOperation : public AsyncOperation
	{
public:
		FindUserOperation(gentity_t *ent, const std::string& user);
		~FindUserOperation();
private:
		gentity_t   *ent_;
		std::string user_;
		void Execute();
	};

	class SaveNameOperation : public AsyncOperation
	{
public:
		SaveNameOperation(const std::string& name, int id);
		~SaveNameOperation();
private:
		std::string name_;
		int         id_;
		void Execute();
	};

	class ListUserNamesOperation : public AsyncOperation
	{
public:
		ListUserNamesOperation(gentity_t *ent, int id);
		~ListUserNamesOperation();
private:
		gentity_t *ent_;
		int       id_;
		void Execute();
	};

	class ResetUsersWithLevelOperation : public AsyncOperation
	{
public:
		ResetUsersWithLevelOperation(int level);
		~ResetUsersWithLevelOperation();
private:
		int level_;
		void Execute();
	};
};

#endif // DATABASE_HH
