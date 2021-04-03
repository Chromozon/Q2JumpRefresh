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

typedef struct LastSeenEntry
{
    std::string username;
    std::string lastSeen;
} LastSeenEntry;


class LocalDatabase
{
public:
    static LocalDatabase& Instance();

    void Init();
    void Close();

    void AddMap(const std::string& mapname);
    void AddMapList(const std::vector<std::string>& maps);
    void AddMapTime(const std::string& mapname, const std::string& username, int timeMs, int pmoveTimeMs,
        const std::vector<replay_frame_t>& replay);

    void AddUser(const std::string& username);
    void UpdateLastSeen(int userid);

    void CalculateAllStatistics(const std::vector<std::string>& maplist);

    void GetMapTimes(std::vector<MapTimesEntry>& results, const std::string& mapname, int limit = -1, int offset = 0);
    void GetLastSeen(std::vector<LastSeenEntry>& results, int limit = -1, int offset = 0);
    int GetMapTime(const std::string& mapname, const std::string& username);
    bool GetReplayByUser(const std::string& mapname, const std::string& username, std::vector<replay_frame_t>& replay);
    bool GetReplayByPosition(const std::string& mapname, int position, std::vector<replay_frame_t>& replay);
    int GetUserId(const std::string& username);
    int GetMapId(const std::string& mapname);
    

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
    bool GetReplay(int mapId, int userId, std::vector<replay_frame_t>& replay);

    // Migrate from old highscores
    void ClearAllTables();
    void MigrateUsers(const std::string& userFile);
    void MigrateMaplist(const std::string& maplistFile);
    void MigrateMapTimes(const std::string& folder);
    void MigrateReplays(const std::string& folder);
    bool MigrateReplay(const std::string& mapname, int userid, const std::vector<replay_frame_t>& replay);
    bool ConvertOldReplay(const std::string& demoFile, std::vector<replay_frame_t>& newReplay);
    

    sqlite3* m_db = nullptr;

    // TODO: when a user first logs in or when they change their name,
    // look up their user id and store it to their client ent
};

}