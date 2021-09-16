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

#include "etj_async_operation.h"
#include <thread>
#include "utilities.hpp"

void *AsyncOperation::StartThread(void *data)
{
	AsyncOperation *object = static_cast<AsyncOperation *>(data);
	object->Execute();
	delete object;
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

	sqlite3_busy_timeout(db_, 5000);

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

sqlite3_stmt *AsyncOperation::GetStatement()
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
	auto taskRunner = std::thread{ AsyncOperation::StartThread, this };
	taskRunner.detach();
}
