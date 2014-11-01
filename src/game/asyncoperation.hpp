#ifndef ASYNC_OPERATION_HH
#define ASYNC_OPERATION_HH

#ifdef _WIN32
#define PTW32_STATIC_LIB
#endif

#include <pthread.h>
#include <sqlite3.h>
#include <string>
#include "g_local.hpp"

// All async operation objects need to be dynamically allocated
class AsyncOperation {
public:
    AsyncOperation() : db_(NULL), stmt_(NULL)
    {
        
    }
    virtual ~AsyncOperation()
    {
        sqlite3_close(db_);
        db_ = NULL;
        sqlite3_finalize(stmt_);
        stmt_ = NULL;
    }

    // This is called to execute an async operation
    // Object is deleted on execute
    void RunAndDeleteObject();

    bool OpenDatabase(const std::string& database);
    bool PrepareStatement(const std::string& statement);
    bool BindInt(int index, int value);
    bool BindString(int index, const std::string& value);
    int GetParameterIndex(const std::string& param);
    bool ExecuteStatement();
    std::string GetMessage() const;

    void PrintOpenError(const std::string& operation);
    void PrintPrepareError(const std::string& operation);
    void PrintBindError(const std::string& operation);
    void PrintExecuteError(const std::string& operation);

protected:
    sqlite3_stmt *GetStatement();

private:
    pthread_t thread_;

    // This is the actual operation
    virtual void Execute() = 0;

    // This is for pthread
    static void *StartThread(void *data);

    sqlite3 *db_;
    sqlite3_stmt *stmt_;
    std::string errorMessage_;
};

#endif 