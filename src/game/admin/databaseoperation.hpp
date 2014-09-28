#ifndef DATABASEOPERATION_HH
#define DATABASEOPERATION_HH

#include <boost/shared_ptr.hpp>
#include <string>
#include "database.hpp"

// This is a class that defines a queued database operation
// for g_instantDatabaseSync 0
class DatabaseOperation
{
public:
    DatabaseOperation(Database *db);
    virtual ~DatabaseOperation();
    virtual void Execute() = 0;
    Database *GetDatabase();
protected:
    Database *db_;   
    std::string message_;
};

struct Ban_s;
typedef boost::shared_ptr<Ban_s> Ban;
class BanUserOperation : public DatabaseOperation
{
public:
    BanUserOperation(Database *db, Ban ban);
    void Execute();
private:
    Ban ban_;
};
typedef boost::shared_ptr<BanUserOperation> BanUserOperationPtr;

struct User_s;
typedef boost::shared_ptr<User_s> User;
class UpdateLastSeenOperation : public DatabaseOperation
{
public:
    UpdateLastSeenOperation(Database *db, User user);
    void Execute();
private:

    User user_;
};
typedef boost::shared_ptr<UpdateLastSeenOperation> UpdateLastSeenOperationPtr;


class AddUserOperation : public DatabaseOperation
{
public:
    AddUserOperation(Database *db, User user);
    void Execute();
private:
    User user_;
};
typedef boost::shared_ptr<AddUserOperation> AddUserOperationPtr;

class SaveUserOperation : public DatabaseOperation
{
public:
    SaveUserOperation(Database *db, User user, unsigned updated);
    void Execute();
private:
    User user_;
    unsigned updated_;
};
typedef boost::shared_ptr<SaveUserOperation> SaveUserOperationPtr;

class UnbanOperation : public DatabaseOperation
{
public:
    UnbanOperation(Database *db, unsigned id);
    void Execute();
private:
    unsigned id_;
};
typedef boost::shared_ptr<UnbanOperation> UnbanOperationPtr;

class AddNewHWIDOperation : public DatabaseOperation
{
public: 
    AddNewHWIDOperation(Database *db, User user);
    void Execute();
private:
    User user_;
};
typedef boost::shared_ptr<AddNewHWIDOperation> AddNewHWIDOperationPtr;

#endif





