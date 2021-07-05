#pragma once

#include <sqlite/sqlite3.h>
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

typedef struct LastSeenEntry
{
    std::string username;
    std::string lastSeen;
} LastSeenEntry;


class LocalDatabase
{
public:
    static void Init();
    static void Close();

    static void AddMap(const std::string& mapname);
    static void AddMapList(const std::vector<std::string>& maps);
    static void AddMapTime(const std::string& mapname, const std::string& username, int timeMs, int pmoveTimeMs,
        const std::vector<replay_frame_t>& replay);

    static int AddUser(const std::string& username);
    static void UpdateLastSeen(int userid);

    static void CalculateAllStatistics(const std::vector<std::string>& maplist);

    static void GetAllUsers(std::map<int, std::string>& users);
    static void GetMapTimes(std::vector<MapTimesEntry>& results, const std::string& mapname, int limit = -1, int offset = 0);
    static void GetLastSeen(std::vector<LastSeenEntry>& results, int limit = -1, int offset = 0);
    static int GetMapTime(const std::string& mapname, const std::string& username);
    static int GetPlayerCompletions(const std::string& mapname, const std::string& username);
    static int GetPlayerMapsCompletedCount(const std::string& username);
    static bool GetReplayByUser(const std::string& mapname, const std::string& username,
        std::vector<replay_frame_t>& replay, int& timeMs);
    static bool GetReplayByPosition(const std::string& mapname, int position,
        std::vector<replay_frame_t>& replay, int& timeMs, std::string& userName);
    static int GetUserId(const std::string& username);
    static int GetMapId(const std::string& mapname);
    static std::string GetUserName(int userId);
    static void GetTotalCompletions(const std::string& mapname, int& totalPlayers, int& totalCompletions);

    static void MigrateAll();

private:
    // Creation
    static void CreateTableMaps();
    static void CreateTableUsers();
    static void CreateTableMapTimes();

    // Misc helpers
    static bool GetReplay(int mapId, int userId, std::vector<replay_frame_t>& replay, int& timeMs);

    // Migrate from old highscores
    static void ClearAllTables();
    static void MigrateUsers(const std::string& userFile);
    static void MigrateMaplist(const std::string& maplistFile);
    static void MigrateMapTimes(const std::string& folder);
    static void MigrateReplays(const std::string& folder);
    static bool MigrateReplay(const std::string& mapname, int userid, const std::vector<replay_frame_t>& replay);
    static bool ConvertOldReplay(const std::string& demoFile, std::vector<replay_frame_t>& newReplay);
    
    static sqlite3* _db;
};

}