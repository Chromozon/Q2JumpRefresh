
#include "jump_local_database.h"
#include "jump_utils.h"
#include "jump_logger.h"
#include <string>
#include "q_shared.h"

namespace Jump
{

/// <summary>
/// Singleton instance.
/// </summary>
/// <returns></returns>
LocalDatabase& LocalDatabase::Instance()
{
    static LocalDatabase instance;
    return instance;
}

/// <summary>
/// Opens the connection to the database.  Creates the initial tables.
/// </summary>
void LocalDatabase::Init()
{
    std::string dbname = "local_db.sqlite3";
    std::string dbpath = GetModPortDir() + "/" + dbname;

    int error = sqlite3_open(dbpath.c_str(), &m_Handle);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Could not open local sqlite3 database, error: %d", error));
    }
    CreateTableUsers();
    CreateTableMaps();
    CreateTableMapTimes();
}

/// <summary>
/// Closes the connectin to the database.
/// </summary>
void LocalDatabase::Close()
{
    sqlite3_close(m_Handle);
}

/// <summary>
/// Creates the Maps table if it does not exist.  Does not update columns if they have changed.
/// </summary>
void LocalDatabase::CreateTableMaps()
{
    std::string sql = ""
        "CREATE TABLE IF NOT EXISTS Maps"
        "("
            "MapId INTEGER NOT NULL,"
            "MapName TEXT NOT NULL UNIQUE,"
            "DateAdded TEXT NOT NULL,"
            "MSet TEXT NULL,"
            "MSetDateUpdated TEXT NULL,"
            "MSetUpdatedBy TEXT NULL,"
            "Ents TEXT NULL,"
            "EntsDateUpdated TEXT NULL,"
            "EntsUpdatedBy TEXT NULL,"
            "PRIMARY KEY(MapId AUTOINCREMENT)"
        ")"
    ;
    int error = sqlite3_exec(m_Handle, sql.c_str(), nullptr, nullptr, nullptr);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Could not create Maps table, error: %d, %s", error, sqlite3_errmsg(m_Handle)));
    }
}

/// <summary>
/// Creates the Users table if it does not exist.  Does not update columns if they have changed.
/// </summary>
void LocalDatabase::CreateTableUsers()
{
    std::string sql = ""
        "CREATE TABLE IF NOT EXISTS Users"
        "("
            "UserId INTEGER NOT NULL,"
            "UserName TEXT NOT NULL COLLATE NOCASE UNIQUE,"
            "PRIMARY KEY(UserId AUTOINCREMENT)"
        ")"
    ;
    int error = sqlite3_exec(m_Handle, sql.c_str(), nullptr, nullptr, nullptr);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Could not create Users table, error: %d, %s", error, sqlite3_errmsg(m_Handle)));
    }
}

/// <summary>
/// Creates the MapTimes table if it does not exist.  Does not update columns if they have changed.
/// </summary>
void LocalDatabase::CreateTableMapTimes()
{
    std::string sql = ""
        "CREATE TABLE IF NOT EXISTS MapTimes"
        "("
            "MapId INTEGER NOT NULL,"
            "UserId INTEGER NOT NULL,"
            "TimeMs INTEGER NOT NULL,"
            "PMoveTimeMs INTEGER NULL," // old times are not recorded using pmove
            "Date TEXT NOT NULL,"
            "Replay BLOB NULL," // we might not always have replay data, especially for really old times
            "PRIMARY KEY(MapId, UserId)"
        ")"
    ;
    int error = sqlite3_exec(m_Handle, sql.c_str(), nullptr, nullptr, nullptr);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Could not create MapTimes table, error: %d, %s", error, sqlite3_errmsg(m_Handle)));
    }
}

/// <summary>
/// Adds a new map into the database.  Does not add if it already exists.
/// </summary>
/// <param name="mapname"></param>
void LocalDatabase::AddNewMap(const std::string& mapname)
{
    std::string sql = ""
        "INSERT OR IGNORE INTO Maps(MapName, DateAdded)"
        "VALUES(@mapname, @dateadded)"
    ;
    std::string dateadded = GetCurrentTimeUTC();
    sqlite3_stmt* prepared = nullptr;

    int error = sqlite3_prepare_v2(m_Handle, sql.c_str(), -1, &prepared, nullptr);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Error adding new map, error: %d, %s", error, sqlite3_errmsg(m_Handle)));
        sqlite3_finalize(prepared);
        return;
    }

    int index = sqlite3_bind_parameter_index(prepared, "@mapname");
    sqlite3_bind_text(prepared, index, mapname.c_str(), -1, SQLITE_STATIC);

    index = sqlite3_bind_parameter_index(prepared, "@dateadded");
    sqlite3_bind_text(prepared, index, dateadded.c_str(), -1, SQLITE_STATIC);

    sqlite3_step(prepared);

    error = sqlite3_finalize(prepared);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Error adding new map, error: %d, %s", error, sqlite3_errmsg(m_Handle)));
        return;
    }
}


}