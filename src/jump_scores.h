#pragma once

#include <string>
#include <vector>
#include "jump_types.h"

namespace Jump
{
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
    bool GetHighscoresForMap(const std::string& mapname, std::vector<user_time_record>& highscores, int& completions);
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
        const std::pair<username_key, float>& left,
        const std::pair<username_key, float>& right);

    int CalculateScore(const user_highscores_t& scores);
    float CalculatePercentScore(const user_highscores_t& scores);

} // namespace Jump

