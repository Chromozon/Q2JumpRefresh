#include "jump_scores.h"
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include "jump_logger.h"
#include "jump_utils.h"
#include "jump.h"
#include <time.h>
#include <sstream>

namespace Jump
{
    void SaveMapCompletion(
        const std::string& mapname,
        const std::string& username,
        int64_t time_ms,
        const std::vector<replay_frame_t>& replay_buffer)
    {
        auto it = jump_server.all_local_maptimes.find(mapname);
        if (it == jump_server.all_local_maptimes.end())
        {
            Logger::Error(va("Cannot save maptime for map %s that is not in the maplist", mapname.c_str()));
            return;
        }

        std::vector<user_time_record>& maptimes = it->second;
        std::string username_lower = AsciiToLower(username);

        std::string path = GetModPortDir() + '/' + SCORES_DIR + '/' + mapname;
        std::filesystem::create_directories(path);
        path += '/' + username + TIME_FILE_EXTENSION;

        auto cached_record = maptimes.begin();
        for (; cached_record != maptimes.end(); ++cached_record)
        {
            if (username_lower == cached_record->username_key)
            {
                break;
            }
        }

        if (cached_record == maptimes.end())
        {
            // New user time for this map, need to create a new record in table
            user_time_record record;
            record.filepath = path;
            record.time_ms = time_ms;
            record.date = GetCurrentTimeUTC();
            record.completions = 1;
            record.username_key = username_lower;
            SaveTimeRecordToFile(record);
            SaveReplayToFile(mapname, username, time_ms, replay_buffer);

            auto iter = std::upper_bound(maptimes.begin(), maptimes.end(), record, SortTimeRecordByTime);
            //auto pos = std::distance(maptimes.begin(), iter);
            maptimes.insert(iter, record);
        }
        else
        {
            // User already has set a time for this map.  Update the cache.
            if (time_ms < cached_record->time_ms)
            {
                // Since we have a pointer to the record, update the time after figuring out the new position
                // so it doesn't mess up the algorithms.
                // Can replace with binary search (upper bound)
                auto new_pos_iter = maptimes.begin();
                for (; new_pos_iter != maptimes.end(); ++new_pos_iter)
                {
                    if (time_ms < new_pos_iter->time_ms)
                    {
                        break;
                    }
                }
                std::rotate(new_pos_iter, cached_record, cached_record + 1);
                new_pos_iter->time_ms = time_ms;
                new_pos_iter->date = GetCurrentTimeUTC();
                new_pos_iter->completions++;
                SaveReplayToFile(mapname, username, time_ms, replay_buffer);
                SaveTimeRecordToFile(*new_pos_iter);
            }
            else
            {
                // No better time, just increase the number of completions
                cached_record->completions++;
                SaveTimeRecordToFile(*cached_record);
            }
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

    // Reads the maplist.txt file and creates a list of all maps
    void LoadLocalMapList(std::unordered_set<std::string>& maplist)
    {
        maplist.clear();
        std::string path = GetModPortDir() + '/' + MAPLIST_FILENAME;
        std::ifstream file(path);
        if (!file.is_open())
        {
            Logger::Warning(va("Maplist file \"%s\" not found, scores will not be saved", path.c_str()));
            return;
        }
        std::string line;
        while (std::getline(file, line))
        {
            if (!line.empty())
            {
                maplist.insert(line);
            }
        }
        Logger::Info(va("Loaded %d maps from maplist \"%s\"", static_cast<int>(maplist.size()), path.c_str()));
    }

    void LoadAllLocalMaptimes(const std::unordered_set<std::string>& maplist,
        std::unordered_map<std::string, std::vector<user_time_record>>& all_local_maptimes)
    {
        all_local_maptimes.clear();
        //return;

        std::string scores_dir = GetModPortDir() + '/' + SCORES_DIR;
        std::filesystem::create_directories(scores_dir);

        for (const std::string& mapname : maplist)
        {
            std::string map_dir = scores_dir + '/' + mapname;
            std::filesystem::create_directories(map_dir);

            all_local_maptimes.insert({ mapname, std::vector<user_time_record>() });

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
                    all_local_maptimes[mapname].push_back(record);
                }
            }
            std::sort(
                all_local_maptimes[mapname].begin(),
                all_local_maptimes[mapname].end(),
                SortTimeRecordByTime);
        }
        Logger::Info("Loaded all local maptimes");
    }

    bool LoadTimeRecordFromFile(const std::string& filepath, user_time_record& record)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            Logger::Warning("Could not open file " + filepath);
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

            record.username_key = AsciiToLower(RemoveFileExtension(RemovePathFromFilename(filepath)));
            return true;
        }
    }

    bool SortTimeRecordByTime(const user_time_record& left, const user_time_record& right)
    {
        return left.time_ms < right.time_ms;
    }

    bool GetHighscoresForMap(
        const std::string& mapname,
        std::vector<user_time_record>& highscores,
        int& players,
        int& completions)
    {
        highscores.clear();
        players = 0;
        completions = 0;

        auto it = jump_server.all_local_maptimes.find(mapname);
        if (it == jump_server.all_local_maptimes.end())
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
            players = it->second.size();
            return true;
        }
    }

    bool HasUserCompletedMap(const std::string& mapname, const std::string& username)
    {
        auto it = jump_server.all_local_maptimes.find(mapname);
        if (it == jump_server.all_local_maptimes.end())
        {
            return false;
        }
        else
        {
            // TODO replace this with a call to the list of maps created for each user because this can be slow
            for (const auto& record : it->second)
            {
                std::string username_lower = AsciiToLower(username);
                if (username_lower == record.username_key)
                {
                    return true;
                }
            }
            return false;
        }
    }

    void SaveReplayToFile(
        const std::string& mapname,
        const std::string& username,
        int64_t time_ms,
        const std::vector<replay_frame_t>& replay_buffer)
    {
        std::string path = GetModPortDir() + '/' + SCORES_DIR + '/' + mapname;
        std::filesystem::create_directories(path);
        path += '/' + username + DEMO_FILE_EXTENSION;
        std::string path_old = path + ".old";

        // Rename the current demo file as backup in case writing the new one fails
        if (std::filesystem::exists(path))
        {
            std::filesystem::rename(path, path_old);
        }

        // Write the demo file
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            Logger::Error("Could not create demo file for user " + username + " on map " + mapname);
            std::filesystem::rename(path_old, path);
            return;
        }
        else
        {
            try
            {
                file << GetCompletionTimeDisplayString(time_ms) << std::endl;
                file << GetCurrentTimeUTC() << std::endl;
                file << replay_buffer.size() << std::endl;
                for (size_t i = 0; i < replay_buffer.size(); ++i)
                {
                    const replay_frame_t& frame = replay_buffer[i];
                    file.write(reinterpret_cast<const char*>(&frame.pos), sizeof(frame.pos));
                    file.write(reinterpret_cast<const char*>(&frame.angles), sizeof(frame.angles));
                    file.write(reinterpret_cast<const char*>(&frame.key_states), sizeof(frame.key_states));
                    file.write(reinterpret_cast<const char*>(&frame.fps), sizeof(frame.fps));
                    file.write(reinterpret_cast<const char*>(&frame.reserved1), sizeof(frame.reserved1));
                    file.write(reinterpret_cast<const char*>(&frame.reserved2), sizeof(frame.reserved2));
                }
                file.flush();
            }
            catch (...)
            {
                Logger::Error("Could not write demo file for user " + username + " on map " + mapname);
                std::filesystem::rename(path_old, path);
                return;
            }
        }
        file.close();
        std::filesystem::remove(path_old);
    }

    bool LoadReplayFromFile(
        const std::string& mapname,
        const std::string& username,
        std::vector<replay_frame_t>& replay_buffer)
    {
        replay_buffer.clear();

        std::string path = GetModPortDir() + '/' + SCORES_DIR + '/' + mapname + '/' + username + DEMO_FILE_EXTENSION;
        if (!std::filesystem::exists(path))
        {
            Logger::Warning("Replay file does not exist for user " + username + ", map " + mapname);
            return false;
        }

        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            Logger::Error("Could not open replay file for user " + username + ", map " + mapname);
            return false;
        }
        std::string line;
        std::getline(file, line);
        // std::string time = line; // unused
        std::getline(file, line);
        // std::string date = line; // unused
        std::getline(file, line);
        size_t frames = std::stoul(line);

        replay_buffer.resize(frames);
        for (size_t i = 0; i < frames; ++i)
        {
            replay_frame_t& frame = replay_buffer[i];
            file.read(reinterpret_cast<char*>(&frame.pos), sizeof(frame.pos));
            file.read(reinterpret_cast<char*>(&frame.angles), sizeof(frame.angles));
            file.read(reinterpret_cast<char*>(&frame.key_states), sizeof(frame.key_states));
            file.read(reinterpret_cast<char*>(&frame.fps), sizeof(frame.fps));
            file.read(reinterpret_cast<char*>(&frame.reserved1), sizeof(frame.reserved1));
            file.read(reinterpret_cast<char*>(&frame.reserved2), sizeof(frame.reserved2));
        }
        return true;
    }

    void CalculateAllLocalStatistics()
    {
        std::unordered_map<username_key, user_highscores_t> scores;

        // Calculate all the highscores for all maps
        for (const auto& map_record : jump_server.all_local_maptimes)
        {
            for (size_t pos = 0; pos < map_record.second.size(); ++pos)
            {
                auto it = scores.insert({ map_record.second[pos].username_key, {} });
                if (pos < MAX_HIGHSCORES)
                {
                    it.first->second.highscore_counts[pos]++;
                }
                it.first->second.map_count++;
            }
        }

        // Create a list of highscores sorted best to worst
        jump_server.all_local_highscores.clear();
        std::copy(scores.begin(), scores.end(), std::back_inserter(jump_server.all_local_highscores));
        std::sort(
            jump_server.all_local_highscores.begin(),
            jump_server.all_local_highscores.end(),
            SortUserHighscoresByScore);

        // Create a list of map counts sorted best to worst
        jump_server.all_local_mapcounts.clear();
        for (const auto& record : scores)
        {
            jump_server.all_local_mapcounts.push_back({ record.first, record.second.map_count });
        }
        std::sort(
            jump_server.all_local_mapcounts.begin(),
            jump_server.all_local_mapcounts.end(),
            SortUserHighscoresByMapCount);

        // Create a list of all playerscores sorted best to worst
        jump_server.all_local_mapscores = jump_server.all_local_highscores; // copy
        std::sort(
            jump_server.all_local_mapscores.begin(),
            jump_server.all_local_mapscores.end(),
            SortUserHighscoresByPercentScore);
    }

    bool SortUserHighscoresByScore(
        const std::pair<username_key, user_highscores_t>& left,
        const std::pair<username_key, user_highscores_t>& right)
    {
        return CalculateScore(left.second) > CalculateScore(right.second);
    }

    bool SortUserHighscoresByMapCount(
        const std::pair<username_key, int>& left,
        const std::pair<username_key, int>& right)
    {
        return left.second > right.second;
    }

    bool SortUserHighscoresByPercentScore(
        const std::pair<username_key, user_highscores_t>& left,
        const std::pair<username_key, user_highscores_t>& right)
    {
        return CalculatePercentScore(left.second) > CalculatePercentScore(right.second);
    }

    int CalculateScore(const user_highscores_t& scores)
    {
        return scores.highscore_counts[0] * 25 +
            scores.highscore_counts[1] * 20 +
            scores.highscore_counts[2] * 16 +
            scores.highscore_counts[3] * 13 +
            scores.highscore_counts[4] * 11 +
            scores.highscore_counts[5] * 10 +
            scores.highscore_counts[6] * 9 +
            scores.highscore_counts[7] * 8 +
            scores.highscore_counts[8] * 7 +
            scores.highscore_counts[9] * 6 +
            scores.highscore_counts[10] * 5 +
            scores.highscore_counts[11] * 4 +
            scores.highscore_counts[12] * 3 +
            scores.highscore_counts[13] * 2 +
            scores.highscore_counts[14] * 1;
    }

    float CalculatePercentScore(const user_highscores_t& scores)
    {
        // A user has to complete n number of maps before the percent score is calculated.
        // This avoids the situation where a user completes only 1 map with a first place and is first on the list forever.
        if (scores.map_count < 50)
        {
            return 0.0f;
        }
        return (static_cast<float>(CalculateScore(scores)) / (scores.map_count * 25)) * 100;
    }

    // Put all of the old time files (maplist.ini, users.t, and <mapname>.t) in "/jump/27910/old/"
    // This function will read those files and save them into the new format
    // It takes ~10 seconds to load the old times into memory
    // It takes ~5 minutes to save them into the new format
    // NOTE: this function will completely overwrite all current maptimes
    void ConvertOldHighscores()
    {
        std::string path = GetModPortDir() + '/' + "old";

        // Load the old maplist
        std::vector<std::string> old_maplist;
        std::string maplist_path = path + '/' + "maplist.ini";
        std::ifstream maplist_file(maplist_path);
        std::string line;
        while (std::getline(maplist_file, line))
        {
            if (line.empty() || line[0] == '[' || line[0] == '#')
            {
                continue;
            }
            else
            {
                old_maplist.push_back(line);
            }
        }
        maplist_file.close();

        // Load the list of users
        std::unordered_map<int, std::string> old_users;
        std::string userlist_path = path + '/' + "users.t";
        std::ifstream userlist_file(userlist_path);
        while (!userlist_file.eof())
        {
            int userid = 0;
            userlist_file >> userid;
            int ignore;
            userlist_file >> ignore;
            userlist_file >> ignore;
            std::string username;
            userlist_file >> username;
            old_users.insert({ userid, username });
        }
        userlist_file.close();

        // Load the maptimes
        std::unordered_map<std::string /*mapname*/, std::vector<user_time_record>> old_maptimes;
        for (size_t i = 0; i < old_maplist.size(); ++i)
        {
            std::string mapname = old_maplist[i];
            old_maptimes.insert({ mapname, {} });
            std::string map_path = path + '/' + mapname + ".t";
            std::ifstream maptime_file(map_path);
            while (std::getline(maptime_file, line))
            {
                std::vector<std::string> tokens = SplitString(line, ' ');
                if (tokens.size() == 4)
                {
                    std::string date = tokens[0];
                    double time = std::stod(tokens[1]);
                    int userid = std::stoi(tokens[2]);
                    int completions = std::stoi(tokens[3]);
                    
                    auto it = old_users.find(userid);
                    if (it != old_users.end())
                    {
                        std::string username = it->second;
                        if (!IsUsernameValid(username))
                        {
                            // TODO: right now we just ignore these times
                            // They aren't really anything notable
                            Logger::Warning("Invalid username found when converting old scores: " + username);
                            continue;
                        }

                        std::string username_key = AsciiToLower(username);
                        user_time_record record;

                        struct tm tm = { 0 };
                        std::istringstream ss(date);
                        ss >> std::get_time(&tm, "%d/%m/%y");
                        if (tm.tm_year < 22)
                        {
                            tm.tm_year += 100;
                        }
                        char buffer[128];
                        strftime(buffer, sizeof(buffer), "%F %T", &tm);

                        record.completions = completions;
                        record.date = buffer;
                        record.filepath = GetModPortDir() + '/' + SCORES_DIR + '/' + mapname + '/' + username + TIME_FILE_EXTENSION;
                        record.time_ms = time * 1000;
                        record.username_key = username_key;

                        old_maptimes[mapname].push_back(record);
                    }
                }
            }
        }

        // Save the times to the file system
        // NOTE!! This will completely clobber any old times if the usernames are the same
        int j = 0;
        for (auto it = old_maptimes.begin(); it != old_maptimes.end(); ++it)
        {
            std::string path = GetModPortDir() + '/' + SCORES_DIR + '/' + it->first;
            std::filesystem::create_directories(path);

            j++;
            for (size_t i = 0; i < it->second.size(); ++i)
            {
                SaveTimeRecordToFile(it->second[i]);
            }
            if (j % 100 == 0)
            {
                Logger::Info(va("Finished saving old maptimes %d/%d", j, static_cast<int>(old_maptimes.size())));
            }
        }
    }

    void LoadLastSeenTimes()
    {
        std::string path = GetModPortDir() + '/' + SEEN_DIR;
        std::filesystem::create_directories(path);
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }
            if (entry.path().extension().generic_string() != SEEN_FILE_EXTENSION)
            {
                continue;
            }
            std::ifstream seen_file(entry);
            std::string line;
            std::getline(seen_file, line);
            auto tokens = SplitString(line, '\t');
            if (tokens.size() != 2)
            {
                Logger::Warning("Invalid contents for seen file " + entry.path().generic_string());
                continue;
            }
            std::string username = tokens[0];
            int64_t seen_time = std::stoll(tokens[1]);
            jump_server.last_seen.push_back({ username, seen_time });
        }
        std::sort(jump_server.last_seen.begin(), jump_server.last_seen.end(),
            [](const std::pair<std::string, int64_t>& left, const std::pair<std::string, int64_t>& right)
            {
                return left.second > right.second;
            }
        );
    }

    void UpdateLastSeenTime(std::string username)
    {
        std::string path = GetModPortDir() + '/' + SEEN_DIR;
        std::filesystem::create_directories(path);
        path += '/' + username + SEEN_FILE_EXTENSION;
        std::ofstream seen_file(path, std::ios::trunc);
        if (!seen_file.is_open())
        {
            Logger::Warning("Could not update last seen file " + path);
            return;
        }
        int64_t seen_time = Sys_Milliseconds();
        seen_file << username << '\t' << seen_time;
        seen_file.close();

        auto it = jump_server.last_seen.begin();
        for (; it != jump_server.last_seen.end(); ++it)
        {
            if (StringCompareInsensitive(username, it->first))
            {
                it->second = seen_time;
                break;
            }
        }
        if (it == jump_server.last_seen.end())
        {
            jump_server.last_seen.push_back({ username, seen_time });
        }
        std::sort(jump_server.last_seen.begin(), jump_server.last_seen.end(),
            [](const std::pair<std::string, int64_t>& left, const std::pair<std::string, int64_t>& right)
            {
                return left.second > right.second;
            }
        );
    }

} // namespace Jump