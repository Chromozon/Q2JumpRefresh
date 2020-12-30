#pragma once

// Map changes to slipmap25
// Clear slipmap25 scores hash table
// Create directory scores/slipmap25/ if it does not already exist
// Enumerate all files in scores/slipmap25/ dir
//   foreach (file)
//     if (file.extension is .time)
//       add to hash table, key: filename (username), value: time, date, completions
//   ComputeTop15()
//     foreach (entry in hash table)
//       keep track of top 15 fastest times
//       do this by using push_heap()

#define SCORES_DIR "scores"
#define TIME_FILE_EXTENSION ".time"
#define MAX_HIGHSCORES 15    // The "top N" best times to keep track of for statistics (top 15, top 10, etc.)

#include <string>
#include <vector>

namespace Jump
{
    typedef std::string user_key; // The user key is the username in lowercase

    typedef struct {
        std::string filepath;
        int64_t time_ms;
        std::string date;
        int32_t completions;
    } user_time_record;

    typedef struct {
        std::string username;
        int32_t total_maps;
        std::vector<std::string> highscore_maps[MAX_HIGHSCORES]; // [0] is a list of all first places, [1] is all second places, etc.
    } user_overall_record;

    typedef std::string mapname_key; // The mapname key is just the mapname

    void LoadTimesForMap(const std::string& mapname);
    void SaveTime(const std::string& mapname, const std::string& username, int64_t time_ms);
    void SaveTimeRecordToFile(const user_time_record& record);
    void LoadAllStatistics();
    bool LoadTimeFileRecord(const std::string& filepath, user_time_record& record);
    bool SortUserTimeFileRecordByTime(const user_time_record& left, const user_time_record& right);
    bool GetHighscoresForMap(const std::string& mapname, std::vector<user_time_record>& highscores, int& completions);

} // namespace Jump

