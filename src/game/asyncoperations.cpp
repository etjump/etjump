#include "asyncoperations.hh"
#include "g_local.hpp"
#include "g_utilities.hpp"
#include <sqlite3.h>
#include "../json/json.h"
#include "loadrace.hpp"

void AsyncSaveRace::Execute()
{
    Json::StyledWriter writer;
    Json::Value root;
    root["start"] = data_.start;
    root["end"] = data_.end;
    root["checkpoints"] = Json::Value();
    for (unsigned i = 0; i < data_.checkpoints.size(); i++)
    {
        root["checkpoints"][i] = data_.checkpoints[i];
    }

    std::string race = writer.write(root);

    if (!OpenDatabase(g_raceDatabase.string))
    {
        G_LogPrintf("ERROR: couldn't open %s: %s\n", g_raceDatabase, GetMessage().c_str());
        return;
    }

    if (!PrepareStatement("INSERT INTO races (name, creator, map, date, route) VALUES (?, ?, ?, ?, ?);"))
    {
        ChatPrintTo(ent_, va("^1ERROR: ^7couldn't prepare SQL-statement. Inform server administrator. %s", GetMessage().c_str()));
        return;
    }

    if (!BindString(1, data_.name) ||
        !BindString(2, data_.creator) ||
        !BindString(3, data_.map) ||
        !BindInt(4, data_.date) ||
        !BindString(5, race)) {
        ChatPrintTo(ent_, va("^1ERROR: ^7couldn't bind value to SQL-statement: %s", GetMessage().c_str()));
        return;
    }

    if (!ExecuteStatement())
    {
        ChatPrintTo(ent_, va("^1ERROR: ^7couldn't execute SQL-statement: %s", GetMessage().c_str()));
        return;
    }

    ChatPrintTo(ent_, va("^3race: ^7successfully saved race (%s).", data_.name.c_str()));
}

void AsyncLoadRace::Execute()
{
    if (!OpenDatabase(g_raceDatabase.string))
    {
        G_LogPrintf("ERROR: couldn't open %s: %s\n", g_raceDatabase, GetMessage().c_str());
        return;
    }

    if (!PrepareStatement("SELECT creator, date, route FROM races WHERE map=? AND name=?;"))
    {
        ChatPrintTo(ent_, va("^1ERROR: ^7couldn't prepare SQL-statement. Inform server administrator. %s", GetMessage().c_str()));
        return;
    }

    if (!BindString(1, level.rawmapname) ||
        !BindString(2, name_)) {
        ChatPrintTo(ent_, va("^1ERROR: ^7couldn't bind value to SQL-statement: %s", GetMessage().c_str()));
        return;
    }

    sqlite3_stmt *stmt = GetStatement();
    if (!stmt)
    {
        ChatPrintTo(ent_, "^1ERROR:^7 stmt == NULL.");
        return;
    }

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
        ChatPrintTo(ent_, va("^1ERROR: ^7failed to load the race %s", name_.c_str()));
        return;
    }
    const char *val = NULL;
    Races::Race race;
    race.name = name_;
    race.map = level.rawmapname;
    val = (const char*)(sqlite3_column_text(stmt, 0));
    race.creator = val ? val : "";
    race.date = sqlite3_column_int(stmt, 1);
    val = (const char*)(sqlite3_column_text(stmt, 2));
    
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(val, root))
    {
        ChatPrintTo(ent_, va("^1ERROR: ^7couldn't parse JSON."));
        return;
    }

    race.start = root["start"].asString();
    race.end = root["end"].asString();
    Json::Value cps = root["checkpoints"];
    for (int i = 0; i < cps.size(); i++)
    {
        race.checkpoints.push_back(cps[i].asString());
    }
    
    boost::shared_ptr<OperationQueue::Operation> op(new LoadRace(races_, race, ent_));
    queue_->AddNewQueuedOperation(op);
}