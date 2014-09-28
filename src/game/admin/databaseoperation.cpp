#include "databaseoperation.hpp"
#include "database.hpp"
#include <boost/algorithm/string.hpp>

DatabaseOperation::DatabaseOperation(Database *db) : db_(db)
{
}

Database* DatabaseOperation::GetDatabase()
{
    if (!db_)
    {
        throw "ERROR: no database defined in DatabaseOperation::GetDatabase()";
    }
    return db_;
}

DatabaseOperation::~DatabaseOperation()
{
}


void BanUserOperation::Execute()
{
    G_LogPrintf("Executing ban user operation: %s\n", ban_->name.c_str());
    if (!this->GetDatabase()->AddBanToSQLite(ban_))
    {
        G_LogPrintf("ERROR: %s\n", this->GetDatabase()->GetMessage().c_str());
    }
}

BanUserOperation::BanUserOperation(Database *db, Ban ban) : DatabaseOperation(db), ban_(ban)
{

}

UpdateLastSeenOperation::UpdateLastSeenOperation(Database *db, User user) :
DatabaseOperation(db), user_(user)
{
}

void UpdateLastSeenOperation::Execute()
{
    G_LogPrintf("Executing update last seen operation: %d %d\n", user_->id, user_->lastSeen);
    if (!this->GetDatabase()->UpdateLastSeenToSQLite(user_))
    {
        G_LogPrintf("ERROR: %s\n", this->GetDatabase()->GetMessage());
    }
}

UnbanOperation::UnbanOperation(Database *db, unsigned id) : DatabaseOperation(db), id_(id)
{
}

void UnbanOperation::Execute()
{
    G_LogPrintf("Executing unban operation: %d\n", id_);
    if (!this->GetDatabase()->RemoveBanFromSQLite(id_))
    {
        G_LogPrintf("ERROR: %s\n", this->GetDatabase()->GetMessage());
    }
}

SaveUserOperation::SaveUserOperation(Database *db, User user, unsigned updated) : DatabaseOperation(db), user_(user), updated_(updated)
{
}

void SaveUserOperation::Execute()
{
    G_LogPrintf("Executing Save user operation: id: %d name: %s\n", user_->id, user_->name.c_str());
    if (!this->GetDatabase()->Save(user_, updated_))
    {
        G_LogPrintf("ERROR: %s\n", this->GetDatabase()->GetMessage().c_str());
    }
}

AddUserOperation::AddUserOperation(Database *db, User user) : DatabaseOperation(db), user_(user)
{
}

void AddUserOperation::Execute()
{
    G_LogPrintf("Executing add user operation: level: %d name: %s\n", user_->level, user_->name.c_str());
    if (!this->GetDatabase()->AddUserToSQLite(user_))
    {
        G_LogPrintf("ERROR: %s\n", this->GetDatabase()->GetMessage().c_str());
    }
}

AddNewHWIDOperation::AddNewHWIDOperation(Database *db, User user) : DatabaseOperation(db), user_(user)
{
}

void AddNewHWIDOperation::Execute()
{
    G_LogPrintf("Executing add new hwid operation: %s\n", boost::algorithm::join(user_->hwids, ",").c_str());
    if (!this->GetDatabase()->AddNewHWIDToDatabase(user_))
    {
        G_LogPrintf("ERROR: %s\n", this->GetDatabase()->GetMessage().c_str());
    }
}