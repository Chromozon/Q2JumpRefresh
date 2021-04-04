#pragma once

#include <string>
#include <vector>
#include <map>
#include "jump_types.h"
#include "jump_local_database.h"

namespace Jump
{

class LocalScores
{
public:
    static void LoadMaplist();
    static std::string GetRandomMap();
    static bool IsMapInMaplist(const std::string& mapname);
    static void CalculateAllStatistics();

    static void PrintPlayerTimes(edict_t* ent);
    static void PrintPlayerScores(edict_t* ent);
    static void PrintPlayerMaps(edict_t* ent);
    static void PrintMapTimes(edict_t* ent);

private:
    // Helper functions
    static int CalculateTotalScore(const std::array<int, 15>& highscores);
    static float CalculatePercentScore(int totalScore, int userMapCount);

    // Statistics cache (reloaded between levels)
    static std::vector<std::string> _maplist;
    static std::map<std::string, std::vector<MapTimesEntry>> _allMapTimes; // <mapname, times sorted best to worst>
    static std::map<int, std::string> _allUsers; // <local userId, username>
    static std::map<int, UserHighscores> _allUserHighscores; // <userId, highscores>
    static std::vector<std::pair<int, int>> _allTotalScores; // <userId, total score> sorted best to worst
    static std::vector<std::pair<int, float>> _allPercentScores; // <userId, percent score> sorted best to worst
    static std::vector<std::pair<int, int>> _allMapCounts; // <userId, mapcount> sorted best to worst
};

    void SaveMapCompletion(
        const std::string& mapname,
        const std::string& username,
        int64_t time_ms,
        const std::vector<replay_frame_t>& replay_buffer);
    void SaveTimeRecordToFile(const user_time_record& record);
    void LoadLocalMapList(std::unordered_set<std::string>& maplist);
    void LoadAllLocalMaptimes(const std::unordered_set<std::string>& maplist,
        std::unordered_map<std::string, std::vector<user_time_record>>& all_local_maptimes);
    bool LoadTimeRecordFromFile(const std::string& filepath, user_time_record& record);
    bool SortTimeRecordByTime(const user_time_record& left, const user_time_record& right);
    bool GetHighscoresForMap(
        const std::string& mapname,
        std::vector<user_time_record>& highscores,
        int& players,
        int& completions
    );
    bool HasUserCompletedMap(const std::string& mapname, const std::string& username);

    void SaveReplayToFile(
        const std::string& mapname,
        const std::string& username,
        int64_t time_ms,
        const std::vector<replay_frame_t>& replay_buffer);

    bool LoadReplayFromFile(
        const std::string& mapname,
        const std::string& username,
        std::vector<replay_frame_t>& replay_buffer);

    void CalculateAllLocalStatistics();

    bool SortUserHighscoresByScore(
        const std::pair<username_key, user_highscores_t>& left,
        const std::pair<username_key, user_highscores_t>& right);

    bool SortUserHighscoresByMapCount(
        const std::pair<username_key, int>& left,
        const std::pair<username_key, int>& right);

    bool SortUserHighscoresByPercentScore(
        const std::pair<username_key, user_highscores_t>& left,
        const std::pair<username_key, user_highscores_t>& right);

    int CalculateScore(const user_highscores_t& scores);
    float CalculatePercentScore(const user_highscores_t& scores);

    void ConvertOldHighscores();

    void LoadLastSeenTimes();
    void UpdateLastSeenTime(std::string username);

} // namespace Jump

