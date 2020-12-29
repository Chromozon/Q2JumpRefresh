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
#define TIME_FILE_EXTENSION "time"

#include <string>

namespace Jump
{
    typedef struct {
        int64_t time_ms;
        std::string date;
        int32_t completions;
    } user_time_file_record;

    void LoadTimesForMap(const std::string& mapname);
    void SaveTime(const std::string& mapname, const std::string& username, int64_t time_ms);
    void SaveTimeRecordToFile(const std::string& path, const user_time_file_record& record);

} // namespace Jump

