#pragma once

#include "sqlite3.h"
#include <string>
#include <vector>
#include "jump_types.h"
#include <map>
#include <array>

namespace Jump
{


typedef struct UserHighscores
{
    std::array<int, 15> highscores = {};
    int mapcount = 0;
} UserHighscores;

typedef struct MapTimesEntry
{
    int userId = 0;
    int timeMs = 0;
    int pmoveTimeMs = 0;
    std::string date = {};
    int completions = 0;
} MapTimesEntry;


class LocalDatabase
{
public:
    static LocalDatabase& Instance();

    void Init();
    void Close();

    void AddMap(const std::string& mapname);
    void AddMapList(const std::vector<std::string>& maps);

    void AddUserOrUpdateSeen(const std::string& username);

    void AddMapTime(const std::string& mapname, const std::string& username, int timeMs, int pmoveTimeMs,
        std::vector<replay_frame_t>& replay);

    void CalculateAllStatistics(const std::vector<std::string>& maplist);

    void GetMapTimes(std::vector<MapTimesEntry>& results, const std::string& mapname, int limit = -1, int offset = 0);

    void MigrateAll();
private:
    // Disallow
    LocalDatabase() {}
    LocalDatabase(const LocalDatabase&) {}
    LocalDatabase& operator=(const LocalDatabase&) {}

    // Creation
    void CreateTableMaps();
    void CreateTableUsers();
    void CreateTableMapTimes();

    // Misc helpers
    void GetAllUsers(std::map<int, std::string>& users);

    // Migrate from old highscores
    void ClearAllTables();
    void MigrateUsers(const std::string& userFile);
    void MigrateMaplist(const std::string& maplistFile);
    void MigrateMapTimes(const std::string& folder);
    void MigrateMSets(const std::string& folder);
    void MigrateEnts(const std::string& folder);
    void MigrateReplays(const std::string& folder);

    sqlite3* m_db = nullptr;
};

}