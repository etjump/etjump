#include "databaseoperation.hpp"
#include "database.hpp"
#include <boost/algorithm/string.hpp>

DatabaseOperation::DatabaseOperation(sqlite3 *db) : db_(db)
{
}

DatabaseOperation::~DatabaseOperation()
{
}


void BanUserOperation::Execute()
{
    G_LogPrintf("Executing ban user operation: %s\n", ban_->name.c_str());
}

BanUserOperation::BanUserOperation(sqlite3 *db, Ban ban) : DatabaseOperation(db), ban_(ban)
{

}

void UpdateLastSeenOperation::Execute()
{
    G_LogPrintf("Executing update last seen operation: %d %d\n", id_, lastSeen_);
}

UpdateLastSeenOperation::UpdateLastSeenOperation(sqlite3 *db, unsigned id, unsigned lastSeen) :
DatabaseOperation(db), id_(id), lastSeen_(lastSeen)
{
}


UnbanOperation::UnbanOperation(sqlite3 *db, unsigned id) : DatabaseOperation(db), id_(id)
{
}

void UnbanOperation::Execute()
{
    G_LogPrintf("Executing unban operation: %d\n", id_);
}

SaveUserOperation::SaveUserOperation(sqlite3 *db, User user) : DatabaseOperation(db), user_(user)
{
}

void SaveUserOperation::Execute()
{
    G_LogPrintf("Executing Save user operation: id: %d name: %s\n", user_->id, user_->name.c_str());
}

AddUserOperation::AddUserOperation(sqlite3 *db, User user) : DatabaseOperation(db), user_(user)
{
}

void AddUserOperation::Execute()
{
    G_LogPrintf("Executing add user operation: level: %d name: %s\n", user_->level, user_->name.c_str());
}

AddNewHWIDOperation::AddNewHWIDOperation(sqlite3* db, User user) : DatabaseOperation(db), user_(user)
{
}

void AddNewHWIDOperation::Execute()
{
    G_LogPrintf("Executing add new hwid operation: %s\n", boost::algorithm::join(user_->hwids, ",").c_str());
}