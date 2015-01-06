#include "asyncoperation.hpp"
#include <pthread.h>
#include "g_utilities.hpp"

void* AsyncOperation::StartThread(void* data)
{
    AsyncOperation *object = static_cast<AsyncOperation*>(data);
    object->Execute();
    delete object;
    pthread_exit(NULL);
    return NULL;
}

bool AsyncOperation::OpenDatabase(std::string const& database)
{
    int rc = sqlite3_open(GetPath(database.c_str()).c_str(), &db_);
    if (rc != SQLITE_OK)
    {
        errorMessage_ = sqlite3_errmsg(db_);
        return false;
    }
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL;",
        NULL, NULL, NULL);
    return true;
}

bool AsyncOperation::PrepareStatement(std::string const& statement)
{
    int rc = sqlite3_prepare_v2(db_, statement.c_str(), -1, &stmt_, 0);
    if (rc != SQLITE_OK)
    {
        errorMessage_ = sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

bool AsyncOperation::BindInt(int index, int value)
{
    int rc = sqlite3_bind_int(stmt_, index, value);
    if (rc != SQLITE_OK)
    {
        errorMessage_ = sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

sqlite3_stmt* AsyncOperation::GetStatement()
{
    return stmt_;
}

void AsyncOperation::PrintOpenError(std::string const& operation)
{
    G_LogPrintf("ERROR: failed to open database on %s %s\n", operation.c_str(),
        GetMessage().c_str());
}

void AsyncOperation::PrintPrepareError(std::string const& operation)
{
    G_LogPrintf("ERROR: failed to prepare %s statement. %s\n",
        operation.c_str(), GetMessage().c_str());
}

void AsyncOperation::PrintBindError(std::string const& operation)
{
    G_LogPrintf("ERROR: failed to bind value to %s statement. %s\n",
        operation.c_str(), GetMessage().c_str());
}

void AsyncOperation::PrintExecuteError(std::string const& operation)
{
    G_LogPrintf("ERROR: failed to execute %s. %s\n",
        operation.c_str(), GetMessage().c_str());
}

std::string AsyncOperation::GetMessage() const
{
    return errorMessage_;
}

bool AsyncOperation::ExecuteStatement()
{
    int rc = sqlite3_step(stmt_);
    if (rc != SQLITE_DONE)
    {
        errorMessage_ = sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

int AsyncOperation::GetParameterIndex(std::string const& param)
{
    return sqlite3_bind_parameter_index(stmt_, param.c_str());
}

bool AsyncOperation::BindString(int index, std::string const& value)
{
    int rc = sqlite3_bind_text(stmt_, index, value.c_str(), value.length(), SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        errorMessage_ = sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

void AsyncOperation::RunAndDeleteObject()
{
    pthread_create(&thread_, NULL, &AsyncOperation::StartThread, this);
    pthread_detach(thread_);
}