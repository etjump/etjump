//
// Created by Jussi on 22.4.2015.
//

#include <thread>
#include <boost/format.hpp>
#include "User.h"
#include "SQLiteWrapper.h"
#include "Printer.h"

void User::Load(std::string database, std::string guid)
{
    if (guid.length() == 0) {
        return;
    }

    std::thread t([]() {
        SQLiteWrapper sqlite;
        if (!sqlite.open(database)) {
            Printer::LogPrintln((boost::format("ERROR: couldn't open database to load a user. error code: %d. error message: %s")
                                 % sqlite.errorCode() % sqlite.errorMessage()).str());
            return;
        }

        if (!sqlite.prepare("SELECT id, level, last_seen, "))
    });
    t.detach();


}

void User::Save(std::string database)
{

}