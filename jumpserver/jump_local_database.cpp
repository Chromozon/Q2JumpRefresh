
#include "jump_local_database.h"
#include "jump_utils.h"
#include "jump_logger.h"
#include <string>
#include "q_shared.h"
#include <fstream>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <map>
#include <array>

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

    int error = sqlite3_open(dbpath.c_str(), &m_db);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Could not open local sqlite3 database, error: %d", error));
    }

    // We bump up the page_size from the default of 4096 to 8192 because that offers the best performance
    // for working with large BLOBs.
    // The cache_size is the maximum amount (in KB) that the database can keep cached in memory.
    std::string sql = ""
        "PRAGMA page_size = 8192;"
        "VACUUM;"
        "PRAGMA cache_size = -100000;"
    ;
    error = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, nullptr);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Could not set database performance options, error: %d, %s", error, sqlite3_errmsg(m_db)));
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
    sqlite3_close(m_db);
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
    int error = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, nullptr);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Could not create Maps table, error: %d, %s", error, sqlite3_errmsg(m_db)));
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
            "LastSeen TEXT NULL,"   // we can have scores for users that have never logged in
            "PRIMARY KEY(UserId AUTOINCREMENT)"
        ")"
    ;
    int error = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, nullptr);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Could not create Users table, error: %d, %s", error, sqlite3_errmsg(m_db)));
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
            "Completions INTEGER NOT NULL,"
            "Replay BLOB NULL," // we might not always have replay data, especially for really old times
            "PRIMARY KEY(MapId, UserId)"
        ")"
    ;
    int error = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, nullptr);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Could not create MapTimes table, error: %d, %s", error, sqlite3_errmsg(m_db)));
    }
}

/// <summary>
/// Adds a new map into the database.  No-op if the map already exists.
/// </summary>
/// <param name="mapname"></param>
void LocalDatabase::AddMap(const std::string& mapname)
{
    std::string sql = ""
        "INSERT OR IGNORE INTO Maps(MapName, DateAdded) "
        "VALUES (@mapname, @dateadded)"
    ;
    std::string dateadded = GetCurrentTimeUTC();
    sqlite3_stmt* prepared = nullptr;

    int error = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &prepared, nullptr);
    if (error != SQLITE_OK)
    {
        Logger::Error(va("Error adding new map, error: %d, %s", error, sqlite3_errmsg(m_db)));
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
        Logger::Error(va("Error adding new map, error: %d, %s", error, sqlite3_errmsg(m_db)));
        return;
    }
}

/// <summary>
/// Adds the list of maps to the database.  No-ops for maps that already exist.
/// </summary>
/// <param name="maps"></param>
void LocalDatabase::AddMapList(const std::vector<std::string>& maps)
{
    for (const std::string& map : maps)
    {
        AddMap(map);
    }
}


// TODO

void LocalDatabase::AddMapTime(const std::string& mapname, const std::string& username, int timeMs, int pmoveTimeMs,
    std::vector<replay_frame_t>& replay)
{
    // I'm not sure if preparing a statement with a blob does a copy immediately or only when executed

    // See if the user already has a maptime for this map
    // If no time exists
    //     Push all data to maptime table, completion = 1
    // else if time exists
    //     if new time is worse
    //         increment completions
    //     else if new time is better
    //         push all data, completions++

    std::string sql = ""
        "INSERT OR IGNORE INTO MapTimes (MapId, UserId, TimeMs, Date, Completions) "
        "VALUES ("
            "(SELECT MapId FROM Maps WHERE MapName = @mapname),"
            "(SELECT UserId FROM Users WHERE UserName = @username),"
            "2147483647,"
            "none,"
            "0"
        ")"
    ;

    int rows = sqlite3_changes(m_db);

    std::string sql2 = ""
        "UPDATE MapTimes "
        "SET "
            "TimeMs = @timems,"
            "PMoveTimeMs = @pmovetimems,"
            "Date = @date,"
            "Completions = Completions + 1,"
            "Replay = @replay "
        "WHERE "
            "MapId = (SELECT MapId FROM Maps WHERE MapName = @mapname) "
            "AND "
            "UserId = (SELECT UserId FROM Users WHERE UserName = @username)"
    ;

    std::string sql3 = ""
        "UPDATE MapTimes SET Completions = Completions + 1 "
        "WHERE "
            "MapId = (SELECT MapId FROM Maps WHERE MapName = @mapname) "
            "AND "
            "UserId = (SELECT UserId FROM Users WHERE UserName = @username)"
    ;

}







void LocalDatabase::CalculateAllStatistics(const std::vector<std::string>& maplist)
{
    std::map<std::string, std::vector<MapTimesEntry>> allMapTimes; // map of <mapname, sorted list of best times>
    for (const std::string& mapname : maplist)
    {
        std::vector<MapTimesEntry> results;
        GetMapTimes(results, mapname);
        allMapTimes.insert(std::make_pair(mapname, results));
    }

    std::map<int, std::string> allUsers;
    GetAllUsers(allUsers);

    std::map<int, UserHighscores> allUserHighscores; // map of <userId, highscores>
    for (const auto& user : allUsers)
    {
        allUserHighscores.insert(std::make_pair(user.first, UserHighscores{}));
    }

    for (const auto& mapTimes : allMapTimes)
    {
        for (size_t place = 0; place < mapTimes.second.size() && place < 15; ++place)
        {
            int userId = mapTimes.second[place].userId;
            allUserHighscores[userId].highscores[place]++;
            allUserHighscores[userId].mapcount++;
        }
    }
}


/// <summary>
/// Gets a list of all userIds and userNames from the database.
/// </summary>
/// <param name="users"></param>
void LocalDatabase::GetAllUsers(std::map<int, std::string>& users)
{
    users.clear();
    const char* sql = "SELECT UserId, UserName FROM Users";
    sqlite3_stmt* prepared = nullptr;
    sqlite3_prepare_v2(m_db, sql, -1, &prepared, nullptr);
    int step = sqlite3_step(prepared);
    while (step == SQLITE_ROW)
    {
        std::pair<int, std::string> data;
        data.first = sqlite3_column_int(prepared, 0);
        data.second = reinterpret_cast<const char*>(sqlite3_column_text(prepared, 1));
        users.insert(data);
        step = sqlite3_step(prepared);
    }
    if (step != SQLITE_DONE)
    {
        Logger::Error(va("Error querying maptimes: %s", sqlite3_errmsg(m_db)));
    }
    sqlite3_finalize(prepared);
}

/// <summary>
/// Gets the sorted highscores list for a given map.
/// </summary>
/// <param name="results"></param>
/// <param name="mapname"></param>
/// <param name="limit"></param>
/// <param name="offset"></param>
void LocalDatabase::GetMapTimes(std::vector<MapTimesEntry>& results, const std::string& mapname, int limit, int offset)
{
    results.clear();
    const char* sql = va(""
        "SELECT UserId, TimeMs, PMoveTimeMs, Date, Completions FROM MapTimes "
        "WHERE MapId = (SELECT MapId FROM Maps WHERE MapName = \'%s\') "
        "ORDER BY TimeMs "
        "LIMIT %d OFFSET %d",
        mapname.c_str(), limit, offset
    );
    sqlite3_stmt* prepared = nullptr;
    sqlite3_prepare_v2(m_db, sql, -1, &prepared, nullptr);
    int step = sqlite3_step(prepared);
    while (step == SQLITE_ROW)
    {
        MapTimesEntry data;
        data.userId = sqlite3_column_int(prepared, 0);
        data.timeMs = sqlite3_column_int(prepared, 1);
        data.pmoveTimeMs = sqlite3_column_int(prepared, 2);
        data.date = reinterpret_cast<const char*>(sqlite3_column_text(prepared, 3));
        data.completions = sqlite3_column_int(prepared, 4);
        results.push_back(data);
        step = sqlite3_step(prepared);
    }
    if (step != SQLITE_DONE)
    {
        Logger::Error(va("Error querying maptimes: %s", sqlite3_errmsg(m_db)));
    }
    sqlite3_finalize(prepared);
}

/// <summary>
/// Deletes all data from all tables.
/// </summary>
void LocalDatabase::ClearAllTables()
{
    std::vector<std::string> tables = {
        "sqlite_sequence", // stores the autoincrement column values
        "Users",
        "Maps",
        "MapTimes"
    };
    for (const std::string& table : tables)
    {
        std::string sql = "DELETE FROM " + table;
        int error = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, nullptr);
        if (error != SQLITE_OK)
        {
            Logger::Error(va("Could not clear %s table, error: %d, %s", table.c_str(), error, sqlite3_errmsg(m_db)));
        }
    }
}

/// <summary>
/// Migrates the users.t file into the database.
/// </summary>
/// <param name="userFile"></param>
void LocalDatabase::MigrateUsers(const std::string& userFile)
{
    std::ifstream file(userFile);
    if (!file.is_open())
    {
        Logger::Error(va("Could not open file %s", userFile.c_str()));
        return;
    }
    int count = 0;
    while (!file.eof())
    {
        int userid;
        file >> userid;

        int ignored;
        file >> ignored;
        file >> ignored;

        std::string username;
        file >> username;

        std::string sql = ""
            "INSERT OR IGNORE INTO Users(UserId, UserName) "
            "VALUES (@userid, @username)";

        sqlite3_stmt* prepared = nullptr;
        int error = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &prepared, nullptr);
        if (error != SQLITE_OK)
        {
            Logger::Error(va("Error adding user %s, error: %d, %s", username.c_str(), error, sqlite3_errmsg(m_db)));
            sqlite3_finalize(prepared);
            continue;
        }

        int index = sqlite3_bind_parameter_index(prepared, "@username");
        sqlite3_bind_text(prepared, index, username.c_str(), -1, SQLITE_STATIC);

        index = sqlite3_bind_parameter_index(prepared, "@userid");
        sqlite3_bind_int(prepared, index, userid);

        sqlite3_step(prepared);

        error = sqlite3_finalize(prepared);
        if (error != SQLITE_OK)
        {
            Logger::Error(va("Error adding user %s, error: %d, %s", username.c_str(), error, sqlite3_errmsg(m_db)));
            continue;
        }
        count++;
    }
    Logger::Info(va("Migration: migrated %d users from %s", count, userFile.c_str()));
}

/// <summary>
/// Migrates the maplist.ini file into the database.
/// </summary>
/// <param name="maplistFile"></param>
void LocalDatabase::MigrateMaplist(const std::string& maplistFile)
{
    std::ifstream file(maplistFile);
    if (!file.is_open())
    {
        Logger::Error(va("Could not open file %s", maplistFile.c_str()));
        return;
    }
    int count = 0;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '[' || line[0] == '#')
        {
            continue;
        }
        TrimString(line);
        AddMap(line);
        count++;
    }
    Logger::Info(va("Migration: migrated %d maps from %s", count, maplistFile.c_str()));
}

/// <summary>
/// Migrate the maptimes files (.t) into the database.
/// </summary>
/// <param name="folder"></param>
void LocalDatabase::MigrateMapTimes(const std::string& folder)
{
    int count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(folder))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }
        if (std::filesystem::path(entry).extension() != ".t")
        {
            continue;
        }
        std::string mapname = RemoveFileExtension(std::filesystem::path(entry).filename().string());
        std::ifstream file(entry);
        if (!file.is_open())
        {
            Logger::Error(va("Migration: Could not open maptime file %s", entry.path().c_str()));
            continue;
        }
        std::string line;
        while (std::getline(file, line))
        {
            std::vector<std::string> tokens = SplitString(line, ' ');
            if (tokens.size() == 4)
            {
                std::string date = tokens[0];
                int time_ms = static_cast<int>(std::stod(tokens[1]) * 1000);
                int userid = std::stoi(tokens[2]);
                int completions = std::stoi(tokens[3]);

                struct tm tm = { 0 };
                std::istringstream ss(date);
                ss >> std::get_time(&tm, "%d/%m/%y");
                char dateBuffer[32];
                strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d %H:%M:%S", &tm);

                const char* sql = va(""
                    "INSERT OR IGNORE INTO MapTimes(MapId, UserId, TimeMs, Date, Completions) "
                    "VALUES ("
                        "(SELECT MapId FROM Maps WHERE MapName = \'%s\'),"
                        "%d,"
                        "%d,"
                        "\'%s\',"
                        "%d"
                    ")",
                    mapname.c_str(),
                    userid,
                    time_ms,
                    dateBuffer,
                    completions
                );
                int error = sqlite3_exec(m_db, sql, nullptr, nullptr, nullptr);
                if (error != SQLITE_OK)
                {
                    Logger::Error(va("Migration: could not add maptime, map %s, userid %d", mapname.c_str(), userid));
                    continue;
                }
                count++;
            }
        }
    }
    Logger::Info(va("Migration: migrated %d maptimes from %s", count, folder.c_str()));
}


void LocalDatabase::MigrateAll()
{
    std::string oldMaplistFile = "G:/Dropbox/Quake2/German_q2jump/german_times_dec_2020/27910/maplist.ini";
    std::string oldMapDir = "E:/Quake2/jump/maps/";
    std::string oldEntDir = "G:/Dropbox/Quake2/German_q2jump/german_map_ents/mapsent/";
    std::string oldMSetDir = "G:/Dropbox/Quake2/German_q2jump/german_msets/ent";
    std::string oldUserFile = "G:/Dropbox/Quake2/German_q2jump/german_times_dec_2020/27910/users.t";
    std::string oldMaptimesDir = "G:/Dropbox/Quake2/German_q2jump/german_times_dec_2020/27910";
    std::string oldReplayDir = "E:/Quake2/jump/jumpdemo/";

    auto start = std::chrono::high_resolution_clock::now();

    ClearAllTables();

    auto step = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(step - start);
    Logger::Info(va("Migration: cleared all tables (%d ms)", duration.count()));

    MigrateMaplist(oldMaplistFile);

    auto step2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(step2 - step);
    Logger::Info(va("Migration: added maplist (%d ms)", duration2.count()));

    MigrateUsers(oldUserFile);

    auto step3 = std::chrono::high_resolution_clock::now();
    auto duration3 = std::chrono::duration_cast<std::chrono::milliseconds>(step3 - step2);
    Logger::Info(va("Migration: added users (%d ms)", duration3.count()));

    MigrateMapTimes(oldMaptimesDir);

    auto step4 = std::chrono::high_resolution_clock::now();
    auto duration4 = std::chrono::duration_cast<std::chrono::milliseconds>(step4 - step3);
    Logger::Info(va("Migration: added maptimes (%d ms)", duration4.count()));
}


}