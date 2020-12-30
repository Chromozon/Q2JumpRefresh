#include "jump_scores.h"
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include "logger.h"
#include "jump_utils.h"

namespace Jump
{
    static std::unordered_map<user_key, user_time_record> current_map_time_records;

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

                // TODO: skip if this is a demo file

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
                        user_time_record record;
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
        path += '/' + username + TIME_FILE_EXTENSION;

        auto cached_record = current_map_time_records.find(username_lower);
        if (cached_record == current_map_time_records.end())
        {
            // New user time for this map, need to create a new record in table
            user_time_record record;
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

    void SaveTimeRecordToFile(const user_time_record& record)
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

    std::unordered_map<mapname_key, std::vector<user_time_record>> all_maptimes_cache;

    void LoadAllStatistics()
    {
        all_maptimes_cache.clear();

        std::string scores_dir = GetModDir() + '/' + SCORES_DIR;
        std::filesystem::create_directories(scores_dir);

        for (const auto& map_dir : std::filesystem::directory_iterator(scores_dir))
        {
            if (!map_dir.is_directory())
            {
                continue;
            }
            std::string mapname = map_dir.path().filename().generic_string();
            all_maptimes_cache.insert({ mapname, std::vector<user_time_record>() });

            for (const auto& entry : std::filesystem::directory_iterator(map_dir))
            {
                if (!entry.is_regular_file())
                {
                    continue;
                }
                if (entry.path().extension().generic_string() != TIME_FILE_EXTENSION)
                {
                    continue;
                }
                user_time_record record;
                if (LoadTimeRecordFromFile(entry.path().generic_string(), record))
                {
                    all_maptimes_cache[mapname].push_back(record);
                }
            }

            std::sort(
                all_maptimes_cache[mapname].begin(),
                all_maptimes_cache[mapname].end(),
                SortTimeRecordByTime);
        }
    }

    bool LoadTimeRecordFromFile(const std::string& filepath, user_time_record& record)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            Logger::Warning("Could no open file " + filepath);
            return false;
        }
        else
        {
            std::string line;

            std::getline(file, line);
            double time = std::stod(line);
            record.time_ms = static_cast<int64_t>(time * 1000);

            std::getline(file, line);
            record.date = line;

            std::getline(file, line);
            record.completions = std::stoi(line);

            record.filepath = filepath;
            return true;
        }
    }

    bool SortTimeRecordByTime(const user_time_record& left, const user_time_record& right)
    {
        return left.time_ms < right.time_ms;
    }

    bool GetHighscoresForMap(const std::string& mapname, std::vector<user_time_record>& highscores, int& completions)
    {
        highscores.clear();
        completions = 0;

        auto it = all_maptimes_cache.find(mapname);
        if (it == all_maptimes_cache.end())
        {
            return false;
        }
        else
        {
            size_t max_highscores = std::min<size_t>(MAX_HIGHSCORES, it->second.size());
            std::copy_n(it->second.begin(), max_highscores, std::back_inserter(highscores));
            for (const auto& record : it->second)
            {
                completions += record.completions;
            }
            return true;
        }
    }

    bool HasUserCompletedMap(const std::string& mapname, const std::string& username)
    {
        auto it = all_maptimes_cache.find(mapname);
        if (it == all_maptimes_cache.end())
        {
            return false;
        }
        else
        {
            // TODO replace this with a call to the list of maps created for each user because this can be slow
            for (const auto& record : it->second)
            {
                std::string username_lower = AsciiToLower(username);
                std::string test_lower = AsciiToLower(RemoveFileExtension(RemovePathFromFilename(record.filepath)));
                if (username_lower == test_lower)
                {
                    return true;
                }
            }
            return false;
        }
    }

    bool GetHighscoresForCurrentMap()
    {
        return false;
    }

} // namespace Jump