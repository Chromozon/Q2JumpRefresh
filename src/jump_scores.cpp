#include "jump_scores.h"
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include "logger.h"
#include "jump_utils.h"

namespace Jump
{
    static std::unordered_map<user_key, user_time_file_record> current_map_time_records;

    void LoadTimesForMap(const std::string& mapname)
    {
        current_map_time_records.clear();

        std::string path = GetModDir() + '/' + SCORES_DIR + '/' + mapname;
        std::filesystem::create_directories(path);

        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_regular_file())
            {
                std::string filepath = entry.path().generic_string();
                std::string username = RemoveFileExtension(RemovePathFromFilename(filepath));
                std::string username_lower = AsciiToLower(username);
                if (username == "")
                {
                    Jump::Logger::Warning("Invalid filename");
                }
                else
                {
                    std::ifstream file(filepath);
                    if (!file.is_open())
                    {
                        Jump::Logger::Warning("Could not open time file");
                    }
                    else
                    {
                        double time = 0.0;
                        file >> time;
                        int64_t time_ms = static_cast<int64_t>(time * 1000);
                        std::string date;
                        file >> date;
                        int32_t completions = 0;
                        file >> completions;
                        user_time_file_record record;
                        record.filepath = filepath;
                        record.time_ms = time_ms;
                        record.date = date;
                        record.completions = completions;
                        current_map_time_records.insert({ username_lower, record });
                    }
                }
            }
        }
    }

    void SaveTime(const std::string& mapname, const std::string& username, int64_t time_ms)
    {
        std::string username_lower = AsciiToLower(username);

        std::string path = GetModDir() + '/' + SCORES_DIR + '/' + mapname;
        std::filesystem::create_directories(path);
        path += '/' + username + '.' + TIME_FILE_EXTENSION;
        path += TIME_FILE_EXTENSION;

        auto cached_record = current_map_time_records.find(username_lower);
        if (cached_record == current_map_time_records.end())
        {
            // New user time for this map, need to create a new record in table
            user_time_file_record record;
            record.filepath = path;
            record.time_ms = time_ms;
            record.date = GetCurrentTimeUTC();
            record.completions = 1;
            current_map_time_records.insert({ username_lower, record });
            SaveTimeRecordToFile(record);
        }
        else
        {
            // User already has set a time for this map.  Update the cache.
            cached_record->second.completions++;
            if (time_ms < cached_record->second.time_ms)
            {
                cached_record->second.time_ms = time_ms;
                cached_record->second.date = GetCurrentTimeUTC();
            }
            SaveTimeRecordToFile(cached_record->second);
        }
    }

    void SaveTimeRecordToFile(const user_time_file_record& record)
    {
        std::ofstream file(record.filepath, std::ios::trunc);
        if (!file.is_open())
        {
            Logger::Error("Could not create time file \"" + record.filepath + "\"");
            return;
        }
        file << GetCompletionTimeDisplayString(record.time_ms) << '\n';
        file << record.date << '\n';
        file << record.completions;
        file.flush();
    }


} // namespace Jump