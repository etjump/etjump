#ifndef DATABASEOPERATION_HH
#define DATABASEOPERATION_HH

#include <boost/shared_ptr.hpp>
#include <sqlite3.h>

// This is a class that defines a queued database operation
// for g_instantDatabaseSync 0
class DatabaseOperation
{
public:
    DatabaseOperation(sqlite3 *db);
    virtual ~DatabaseOperation();
    virtual void Execute() = 0;
protected:
    sqlite3 *db_;    
};

struct Ban_s;
typedef boost::shared_ptr<Ban_s> Ban;
class BanUserOperation : public DatabaseOperation
{
public:
    BanUserOperation(sqlite3 *db, Ban ban);
    void Execute();
private:
    Ban ban_;
};
typedef boost::shared_ptr<BanUserOperation> BanUserOperationPtr;

class UpdateLastSeenOperation : public DatabaseOperation
{
public:
    UpdateLastSeenOperation(sqlite3 *db, unsigned id, unsigned lastSeen);
    void Execute();
private:

    unsigned id_;
    unsigned lastSeen_;
};
typedef boost::shared_ptr<UpdateLastSeenOperation> UpdateLastSeenOperationPtr;

struct User_s;
typedef boost::shared_ptr<User_s> User;
class AddUserOperation : public DatabaseOperation
{
public:
    AddUserOperation(sqlite3 *db, User user);
    void Execute();
private:
    User user_;
};
typedef boost::shared_ptr<AddUserOperation> AddUserOperationPtr;

class SaveUserOperation : public DatabaseOperation
{
public:
    SaveUserOperation(sqlite3 *db, User user);
    void Execute();
private:
    User user_;
};
typedef boost::shared_ptr<SaveUserOperation> SaveUserOperationPtr;

class UnbanOperation : public DatabaseOperation
{
public:
    UnbanOperation(sqlite3 *db, unsigned id);
    void Execute();
private:
    unsigned id_;
};
typedef boost::shared_ptr<UnbanOperation> UnbanOperationPtr;

class AddNewHWIDOperation : public DatabaseOperation
{
public: 
    AddNewHWIDOperation(sqlite3 *db, User user);
    void Execute();
private:
    User user_;
};
typedef boost::shared_ptr<AddNewHWIDOperation> AddNewHWIDOperationPtr;

#endif





