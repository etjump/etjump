/*
 * MIT License
 * 
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
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

#include <string>
#include "etj_sqlite_wrapper.h"
#include <cassert>

bool SQLiteWrapper::open(const std::string &database)
{
	assert(_db == nullptr);
	auto rc = sqlite3_open(database.c_str(), &_db);
	if (rc != SQLITE_OK)
	{
		_message   = sqlite3_errmsg(_db);
		_errorCode = rc;
		return false;
	}

	rc = sqlite3_exec(_db, "PRAGMA journal_mode=WAL;", 0, 0, 0);
	if (rc != SQLITE_OK)
	{
		_message   = sqlite3_errmsg(_db);
		_errorCode = rc;
		return false;
	}

	// Wait atleast 5000 ms for other threads to finish writing
	sqlite3_busy_timeout(_db, 5000);

	return true;
}

bool SQLiteWrapper::prepare(const std::string &sql)
{
	if (_stmt)
	{
		sqlite3_finalize(_stmt);
		_stmt = nullptr;
	}
	auto rc = sqlite3_prepare(_db, sql.c_str(), -1, &_stmt, NULL);
	if (rc != SQLITE_OK)
	{
		_message   = sqlite3_errmsg(_db);
		_errorCode = rc;
		return false;
	}
	return true;
}

bool SQLiteWrapper::bindText(int index, const std::string &text)
{
	auto rc = sqlite3_bind_text(_stmt, index, text.c_str(), text.length(), SQLITE_STATIC);
	if (rc != SQLITE_OK)
	{
		_message   = sqlite3_errmsg(_db);
		_errorCode = rc;
		return false;
	}
	return true;
}

bool SQLiteWrapper::bindInteger(int index, int number)
{
	auto rc = sqlite3_bind_int(_stmt, index, number);
	if (rc != SQLITE_OK)
	{
		_message   = sqlite3_errmsg(_db);
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

	if (rc != SQLITE_DONE)
	{
		_message   = sqlite3_errmsg(_db);
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
