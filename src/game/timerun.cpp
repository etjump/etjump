#include "timerun.hpp"
#include "g_local.hpp"
#include "g_utilities.hpp"
#include <sqlite3.h>

Timerun::Timerun()
{
}

Timerun::~Timerun()
{
}

void Timerun::Initialize()
{
    if (strlen(g_timerunsDatabase.string) == 0)
    {
        return;
    }

    std::string dbPath = GetPath(g_timerunsDatabase.string);
    sqlite3 *db = NULL;
    sqlite3_open(dbPath.c_str(), &db);

    char *err;
    int rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS record (id INT PRIMARY KEY AUTOINCREMENT, map VARCHAR(256), run VARCHAR(256), player INT NOT NULL);", 0, 0, &err);
    if (rc != SQLITE_OK)
    {
        G_LogPrintf("ERROR: couldn't create timerun database table. (%d): %s\n", rc, err);
        return;
    }

    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare_v2(db, va("SELECT id, run, player FROM record WHERE map=%s;", level.rawmapname), -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        G_LogPrintf("ERROR: couldn't prepare timerun query. (%d): %s\n", rc, sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_step(stmt);
    while (rc != SQLITE_DONE)
    {
        switch (rc)
        {
        case SQLITE_ROW:
            G_LogPrintf("Loaded a record %d %s %s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2));
            return;
        default:
            G_LogPrintf("ERROR: couldn't fetch a row from database. (%d): %s\n", rc, sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return;
        }
    }
    sqlite3_finalize(stmt);
    G_LogPrintf("Successfully initialized timerun records database.\n");
}

void Timerun::Shutdown()
{
}

void Timerun::StartTimer(const char* runName, gentity_t* ent)
{
    Player *player = &players_[ClientNum(ent)];

    if (!player->racing)
    {
        player->racing = true;
        player->raceStartTime = ent->client->ps.commandTime;
        player->runName = runName ? runName : "";
    }
}

void Timerun::StopTimer(const char* runName, gentity_t* ent)
{
    Player *player = &players_[ClientNum(ent)];

    if (player->racing)
    {
        int millis = ent->client->ps.commandTime - player->raceStartTime;
        int seconds = millis / 1000;
        millis = millis - seconds * 1000;
        int minutes = seconds / 60;
        seconds = seconds - minutes * 60;

        CPMTo(ent, boost::format("Finished %s in %02d:%02d:%03d") % player->runName % minutes % seconds % millis);

        player->racing = false;
        player->runName = "";
    }
}