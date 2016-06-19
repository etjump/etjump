//
// Created by Jussi on 4.4.2015.
//

#ifndef ETJUMP_SQLITEWRAPPER_H
#define ETJUMP_SQLITEWRAPPER_H

#include <sqlite3.h>
#include <string>

class SQLiteWrapper {
public:

	SQLiteWrapper() : _db(nullptr), _stmt(nullptr)
	{

	}


	virtual ~SQLiteWrapper()
	{
		if (_stmt != nullptr)
		{
			sqlite3_finalize(_stmt);
		}

		if (_db != nullptr)
		{
			sqlite3_close(_db);
			_db = nullptr;
		}
	}

	/**
	 * Opens the database
	 * @param database The database file
	 * @return false if the open failed
	 */
	bool open(const std::string& database);

	/**
	 * Prepares the statement
	 * @param sql The statement to be prepared
	 * @return false if the prepare failed
	 */
	bool prepare(const std::string& sql);

	/**
	 * Binds a string to the statement in index
	 * @param index The index of the parameter to be bound
	 * @param text The string to be bound
	 * @return false if the bind failed
	 */
	bool bindText(int index, const std::string& text);

	/**
	 * Binds an integer to the statement in index
	 * @param index The index of the int to be bound
	 * @param number The int to be bound
	 * @return false if the bind failed
	 */
	bool bindInteger(int index, int number);

	/**
	 * Gets the named parameter index
	 * @param namedParameter The parameter name to look for
	 */
	int namedParameterIndex(const std::string& namedParameter);

	/**
	 * Executes the prepared statement
	 * @return true if SQLITE_DONE
	 */
	bool execute();

	/**
	 * returns the statement for cases where select is used and
	 * results need to be parsed.
	 * @return The statement
	 */
	sqlite3_stmt *getStatement()
	{
		return _stmt;
	};

	/**
	 * Gets the error code
	 * @return Error code
	 */
	int errorCode() const;

	/**
	 * Gets the error message
	 * @return Error message
	 */
	std::string errorMessage() const;

	/**
	 * Gets the error message straight from sqlite
	 * @return The sqlite err msg
	 */
	std::string getSQLiteErrorMessage();

private:
	int          _errorCode;
	std::string  _message;
	sqlite3      *_db;
	sqlite3_stmt *_stmt;
};


#endif //ETJUMP_SQLITEWRAPPER_H
