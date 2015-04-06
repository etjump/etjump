//
// Created by Jussi on 4.4.2015.
//

#include <string>
#include "SQLiteWrapper.h"
#include <cassert>

bool SQLiteWrapper::open(const std::string &database)
{
    assert(_db == nullptr);
    auto rc = sqlite3_open(database.c_str(), &_db);
    if (rc != SQLITE_OK) {
        _message = sqlite3_errmsg(_db);
        _errorCode = rc;
        return false;
    }
    return true;
}

bool SQLiteWrapper::prepare(const std::string &sql)
{
    if (_stmt) {
        sqlite3_finalize(_stmt);
        _stmt = nullptr;
    }
    auto rc = sqlite3_prepare(_db, sql.c_str(), -1, &_stmt, NULL);
    if (rc != SQLITE_OK) {
        _message = sqlite3_errmsg(_db);
        _errorCode = rc;
        return false;
    }
    return true;
}

bool SQLiteWrapper::bindText(int index, const std::string &text)
{
    auto rc = sqlite3_bind_text(_stmt, index, text.c_str(), text.length(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        _message = sqlite3_errmsg(_db);
        _errorCode = rc;
        return false;
    }
    return true;
}

bool SQLiteWrapper::bindInteger(int index, int number)
{
    auto rc = sqlite3_bind_int(_stmt, index, number);
    if (rc != SQLITE_OK) {
        _message = sqlite3_errmsg(_db);
        _errorCode = rc;
        return false;
    }
    return true;
}

int SQLiteWrapper::namedParameterIndex(const std::string &namedParameter)
{
    return sqlite3_bind_parameter_index(_stmt, namedParameter.c_str());
}

bool SQLiteWrapper::execute()
{
    auto rc = sqlite3_step(_stmt);

    if (rc != SQLITE_DONE) {
        _message = sqlite3_errmsg(_db);
        _errorCode = rc;
        return false;
    }
    return true;
}

int SQLiteWrapper::errorCode() const
{
    return _errorCode;
}

std::string SQLiteWrapper::errorMessage() const
{
    return _message;
}

std::string SQLiteWrapper::getSQLiteErrorMessage()
{
    const char *msg = sqlite3_errmsg(_db);
    return msg ? msg : "";
}
