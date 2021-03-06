#include "jump_scores.h"
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include "jump_logger.h"
#include "jump_utils.h"
#include "jump.h"
#include <time.h>
#include <sstream>
#include <chrono>
#include "jump_local_database.h"

namespace Jump
{

/// <summary>
/// Define the cache private variables.
/// </summary>
std::vector<std::string> LocalScores::_maplistByName;
std::vector<std::string> LocalScores::_maplistByAdded;
std::map<std::string, std::vector<MapTimesEntry>> LocalScores::_allMapTimes;
std::map<int, std::string> LocalScores::_allUsers;
std::map<int, UserHighscores> LocalScores::_allUserHighscores;
std::map<int, std::vector<std::string>> LocalScores::_allUserMaps;
std::vector<std::pair<int, int>> LocalScores::_allTotalScores;
std::vector<std::pair<int, float>> LocalScores::_allPercentScores;
std::vector<std::pair<int, int>> LocalScores::_allMapCounts;
std::vector<std::tuple<time_t, std::string, MapTimesEntry>> LocalScores::_firstPlacesToday;

/// <summary>
/// Loads the list of server maps from the maplist.ini file and update the local database.
/// Verifies that the server has all of the bsp files in the maplist.
/// </summary>
void LocalScores::LoadMaplist()
{
    _maplistByName.clear();
    _maplistByAdded.clear();
    std::string path = GetModPortDir() + "/maplist.ini";
    std::ifstream file(path);
    if (!file.is_open())
    {
        Logger::Error(va("Could not open maplist %s, times will not be saved.", path.c_str()));
        return;
    }
    std::vector<std::string> filesNotFound;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '[' || line[0] == '#')
        {
            continue;
        }
        TrimString(line);
        if (!line.empty())
        {
            std::string mapfile = GetModDir() + "/maps/" + line + ".bsp";
            if (std::filesystem::exists(mapfile))
            {
                if (std::find(_maplistByName.begin(), _maplistByName.end(), line) != _maplistByName.end())
                {
                    Logger::Warning(va("Duplicate map found in maplist: %s", line.c_str()));
                }
                else
                {
                    _maplistByName.push_back(line);
                    _maplistByAdded.push_back(line);
                }
            }
            else
            {
                filesNotFound.push_back(line);
            }
        }
    }

    // Sort the maplist so that it makes finding mapsleft and mapvote todos very easy
    std::sort(_maplistByName.begin(), _maplistByName.end(), std::less<std::string>());

    LocalDatabase::AddMapList(_maplistByName);
    Logger::Info(va("Loaded %d maps from maplist \"%s\"", static_cast<int>(_maplistByName.size()), path.c_str()));
    for (const std::string& mapname : filesNotFound)
    {
        Logger::Warning("Server does not have bsp file: " + mapname);
    }
}

/// <summary>
/// Picks a random map from the maplist.
/// </summary>
/// <returns>Mapname or emtpy string if invalid.</returns>
std::string LocalScores::GetRandomMap()
{
    std::string mapname;
    if (_maplistByName.empty())
    {
        return mapname;
    }
    static bool initialized = false;
    if (!initialized)
    {
        srand(time(0));
        initialized = true;
    }
    int n = rand() % _maplistByName.size();
    return _maplistByName[n];
}

/// <summary>
/// Checks to see if the given map is in the loaded maplist.
/// </summary>
/// <param name="mapname"></param>
/// <returns>True if in maplist</returns>
bool LocalScores::IsMapInMaplist(const std::string& mapname)
{
    auto it = std::find(_maplistByName.begin(), _maplistByName.end(), mapname);
    return it != _maplistByName.end();
}

/// <summary>
/// Calculate all local statistics (playertimes, playermaps, playerscores, maptimes).
/// </summary>
/// <param name="maplist"></param>
void LocalScores::CalculateAllStatistics()
{
    // Load the times for all maps sorted by best to worst time.
    _allMapTimes.clear();
    for (const std::string& mapname : _maplistByName)
    {
        std::vector<MapTimesEntry> results;
        LocalDatabase::GetMapTimes(results, mapname);
        _allMapTimes.insert(std::make_pair(mapname, results));
    }

    // Create the list of first places set in the last 24 hours.
    time_t timeNowUtcSeconds = std::time(nullptr);
    time_t timeYesterdayUtcSeconds = timeNowUtcSeconds - (24 * 60 * 60);
    _firstPlacesToday.clear();
    for (const auto& entry : _allMapTimes)
    {
        if (!entry.second.empty())
        {
            std::tm tm;
            std::istringstream ss(entry.second[0].date);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            time_t timeCompletionUtcSeconds = mktime(&tm);

            if (timeCompletionUtcSeconds > timeYesterdayUtcSeconds)
            {
                _firstPlacesToday.push_back(std::make_tuple(timeCompletionUtcSeconds, entry.first, entry.second[0]));
            }
        }
    }
    std::sort(_firstPlacesToday.begin(), _firstPlacesToday.end(),
        [](const std::tuple<time_t, std::string, MapTimesEntry>& left,
           const std::tuple<time_t, std::string, MapTimesEntry>& right) -> bool
    {
        return std::get<0>(left) < std::get<0>(right);
    });

    // Get a list of all local userIds and usernames.
    _allUsers.clear();
    LocalDatabase::GetAllUsers(_allUsers);

    // Initialize the list of maps completed by each user.
    _allUserMaps.clear();
    for (const auto& user : _allUsers)
    {
        int userId = user.first;
        _allUserMaps.insert(std::make_pair(userId, std::vector<std::string>{}));
    }

    // Calculate the top 15 counts and total maps completed for each user.
    _allUserHighscores.clear();
    for (const auto& user : _allUsers)
    {
        int userId = user.first;
        _allUserHighscores.insert(std::make_pair(userId, UserHighscores{}));
    }
    for (const auto& mapTimes : _allMapTimes)
    {
        for (size_t place = 0; place < mapTimes.second.size() && place < 15; ++place)
        {
            int userId = mapTimes.second[place].userId;
            if (_allUserHighscores.find(userId) == _allUserHighscores.end())
            {
                Logger::Warning(va("Maptime exists for a map %s, userId %d that is not in the Users list",
                    mapTimes.first.c_str(), userId));
            }
            else
            {
                _allUserHighscores[userId].highscores[place]++;
            }
        }
        for (auto i = 0; i < mapTimes.second.size(); ++i)
        {
            int userId = mapTimes.second[i].userId;
            _allUserHighscores[userId].mapcount++;
            _allUserMaps[userId].push_back(mapTimes.first);
        }
    }

    // Calculate total score and percentage score for all users and sort them best to worst.
    _allTotalScores.clear();
    _allPercentScores.clear();
    for (const auto& userHighscores : _allUserHighscores)
    {
        int totalScore = CalculateTotalScore(userHighscores.second.highscores);
        _allTotalScores.push_back(std::make_pair(userHighscores.first, totalScore));

        float percentScore = CalculatePercentScore(totalScore, userHighscores.second.mapcount);
        _allPercentScores.push_back(std::make_pair(userHighscores.first, percentScore));
    }
    std::sort(_allTotalScores.begin(), _allTotalScores.end(),
        [](const std::pair<int, int>& left, const std::pair<int, int>& right) -> bool
    {
        return left.second > right.second;
    });
    std::sort(_allPercentScores.begin(), _allPercentScores.end(),
        [](const std::pair<int, float>& left, const std::pair<int, float>& right) -> bool
    {
        return left.second > right.second;
    });

    // Create a sorted list of total map completions.
    _allMapCounts.clear();
    for (const auto& userHighscores : _allUserHighscores)
    {
        int userId = userHighscores.first;
        _allMapCounts.push_back(std::make_pair(userId, userHighscores.second.mapcount));
    }
    std::sort(_allMapCounts.begin(), _allMapCounts.end(),
        [](const std::pair<int, int>& left, const std::pair<int, int>& right) -> bool
    {
        return left.second > right.second;
    });

    // Sort the user maps alphabetically.
    for (auto& entry : _allUserMaps)
    {
        std::sort(entry.second.begin(), entry.second.end(), std::less<std::string>());
    }
}

/// <summary>
/// Calculates the total score for a user based on how many first places, second places, etc. that they have.
/// </summary>
/// <param name="highscores"></param>
/// <returns></returns>
int LocalScores::CalculateTotalScore(const std::array<int, 15>& highscores)
{
    int totalScore =
        highscores[0] * 25 +
        highscores[1] * 20 +
        highscores[2] * 16 +
        highscores[3] * 13 +
        highscores[4] * 11 +
        highscores[5] * 10 +
        highscores[6] * 9 +
        highscores[7] * 8 +
        highscores[8] * 7 +
        highscores[9] * 6 +
        highscores[10] * 5 +
        highscores[11] * 4 +
        highscores[12] * 3 +
        highscores[13] * 2 +
        highscores[14] * 1;
    return totalScore;
}

/// <summary>
/// Retrieves the username and time for the first place on a given map.
/// </summary>
/// <param name="mapname"></param>
/// <param name="username">Set on return</param>
/// <param name="timeMs">Set on return</param>
/// <returns>False if map does not exist or if there are no times set for the map.</returns>
bool LocalScores::GetFirstPlaceForMaplist(const std::string& mapname, std::string& username, int& timeMs)
{
    auto it = _allMapTimes.find(mapname);
    if (it == _allMapTimes.end())
    {
        // Map does not exist
        return false;
    }
    if (it->second.empty())
    {
        // No time set for this map
        return false;
    }
    username = GetUserName(it->second[0].userId);
    timeMs = it->second[0].timeMs;
    return true;
}

/// <summary>
/// Calculates the percent score for a user.  
/// This is how close a user is to being first in every map they have completed.
/// </summary>
/// <param name="totalScore"></param>
/// <param name="userMapCount"></param>
/// <returns></returns>
float LocalScores::CalculatePercentScore(int totalScore, int userMapCount)
{
    // A user has to complete n number of maps before the percent score is calculated.
    // This avoids the situation where a user completes only 1 map with a first place and is first on the list forever.
    if (userMapCount < 50)
    {
        return 0.0f;
    }
    float percentScore = (totalScore / (userMapCount * 25.0f)) * 100.0f;
    return percentScore;
}

/// <summary>
/// Gets how many players have completed a given map.
/// </summary>
/// <param name="mapname"></param>
/// <returns>Total players, 0 if no completions, -1 if map does not exist.</returns>
int LocalScores::GetTotalTimesForMap(const std::string& mapname)
{
    auto it = _allMapTimes.find(mapname);
    if (it == _allMapTimes.end())
    {
        return -1;
    }
    else
    {
        return static_cast<int>(it->second.size());
    }
}

/// <summary>
/// Gets the username given the userid.
/// </summary>
/// <param name="userId"></param>
/// <returns>Username or empty string if not found</returns>
std::string LocalScores::GetUserName(int userId)
{
    std::string username;
    auto it = _allUsers.find(userId);
    if (it != _allUsers.end())
    {
        username = it->second;
    }
    return username;
}

/// <summary>
/// Gets how many maps are in the loaded maplist.
/// </summary>
/// <returns></returns>
int LocalScores::GetServerMapCount()
{
    return static_cast<int>(_maplistByName.size());
}

/// <summary>
/// Gets the list of maps that the user has left to complete.
/// </summary>
/// <param name="userId"></param>
/// <param name="results"></param>
/// <returns>True if success, false if user not found.</returns>
bool LocalScores::GetMapsLeft(int userId, std::vector<std::string>& results)
{
    results.clear();
    auto it = _allUserMaps.find(userId);
    if (it == _allUserMaps.end())
    {
        return false;
    }
    std::set_difference(
        _maplistByName.begin(), _maplistByName.end(),
        it->second.begin(), it->second.end(), std::back_inserter(results));
    return true;
}

/// <summary>
/// Gets the list of maps that the user has completed.
/// </summary>
/// <param name="userId"></param>
/// <param name="results"></param>
/// <returns>True if success, false if user not found.</returns>
bool LocalScores::GetMapsDone(int userId, std::vector<std::string>& results)
{
    results.clear();
    auto it = _allUserMaps.find(userId);
    if (it == _allUserMaps.end())
    {
        return false;
    }
    results = it->second;
    return true;
}

/// <summary>
/// Print the playertimes to the client console.
/// </summary>
/// <param name="ent"></param>
void LocalScores::PrintPlayerTimes(edict_t* ent)
{
    int page = 1;
    if (gi.argc() > 1)
    {
        StringToIntMaybe(gi.argv(1), page);
    }
    if (page < 1)
    {
        page = 1;
    }
    size_t index_start = (page - 1) * CONSOLE_HIGHSCORES_COUNT_PER_PAGE;
    if (index_start >= _allUserHighscores.size())
    {
        gi.cprintf(ent, PRINT_HIGH, "There are no playertimes for this page.\n");
        return;
    }
    size_t index_end = std::min<size_t>(
        _allUserHighscores.size() - 1,
        (page * CONSOLE_HIGHSCORES_COUNT_PER_PAGE) - 1);

    // Point info
    gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------\n");
    gi.cprintf(ent, PRINT_HIGH, "Point Values: 1-15: 25,20,16,13,11,10,9,8,7,6,5,4,3,2,1\n");
    gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------\n");

    // Header row
    std::string header = GetGreenConsoleText(
        "No. Name            1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th Score");
    gi.cprintf(ent, PRINT_HIGH, "%s\n", header.c_str());

    for (size_t i = index_start; i <= index_end; ++i)
    {
        int userId = _allTotalScores[i].first;
        int totalScore = _allTotalScores[i].second;
        const char* username = _allUsers[userId].c_str();

        gi.cprintf(ent, PRINT_HIGH, "%-3d %-15s %3d %3d %3d %3d %3d %3d %3d %3d %3d %4d %4d %4d %4d %4d %4d %5d\n",
            static_cast<int>(i + 1),
            username,
            _allUserHighscores[userId].highscores[0],
            _allUserHighscores[userId].highscores[1],
            _allUserHighscores[userId].highscores[2],
            _allUserHighscores[userId].highscores[3],
            _allUserHighscores[userId].highscores[4],
            _allUserHighscores[userId].highscores[5],
            _allUserHighscores[userId].highscores[6],
            _allUserHighscores[userId].highscores[7],
            _allUserHighscores[userId].highscores[8],
            _allUserHighscores[userId].highscores[9],
            _allUserHighscores[userId].highscores[10],
            _allUserHighscores[userId].highscores[11],
            _allUserHighscores[userId].highscores[12],
            _allUserHighscores[userId].highscores[13],
            _allUserHighscores[userId].highscores[14],
            totalScore
        );
    }

    // Footer
    int totalPages = (_allUserHighscores.size() / CONSOLE_HIGHSCORES_COUNT_PER_PAGE) + 1;
    gi.cprintf(ent, PRINT_HIGH, "Page %d/%d (%d users). Use playertimes <page>\n",
        page, totalPages, static_cast<int>(_allUserHighscores.size()));
    gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------\n");
}

/// <summary>
/// Print the playerscores to the client console.
/// </summary>
/// <param name="ent"></param>
void LocalScores::PrintPlayerScores(edict_t* ent)
{
    int page = 1;
    if (gi.argc() > 1)
    {
        StringToIntMaybe(gi.argv(1), page);
    }
    if (page < 1)
    {
        page = 1;
    }
    size_t index_start = (page - 1) * CONSOLE_HIGHSCORES_COUNT_PER_PAGE;
    if (index_start >= _allUserHighscores.size())
    {
        gi.cprintf(ent, PRINT_HIGH, "There are no playerscores for this page.\n");
        return;
    }
    size_t index_end = std::min<size_t>(
        _allUserHighscores.size() - 1,
        (page * CONSOLE_HIGHSCORES_COUNT_PER_PAGE) - 1);

    // Point info
    gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------\n");
    gi.cprintf(ent, PRINT_HIGH, "Point Values: 1-15: 25,20,16,13,11,10,9,8,7,6,5,4,3,2,1\n");
    gi.cprintf(ent, PRINT_HIGH, "Score = (Your score) / (Potential score if 1st on all your completed maps\n");
    gi.cprintf(ent, PRINT_HIGH, "Ex: 5 maps completed || 3 1st's, 2 3rd's = 107 pts || 5 1st's = 125 pts || 107/125 = 85.6%%\n");
    gi.cprintf(ent, PRINT_HIGH, "NOTE: Playerscore are only calculated for users with at least 50 maps completed\n");
    gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------\n");

    // Header row
    std::string header = GetGreenConsoleText(
        "No. Name            1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th Score");
    gi.cprintf(ent, PRINT_HIGH, "%s\n", header.c_str());

    for (size_t i = index_start; i <= index_end; ++i)
    {
        int userId = _allTotalScores[i].first;
        float percentScore = _allPercentScores[i].second;
        const char* username = _allUsers[userId].c_str();

        gi.cprintf(ent, PRINT_HIGH, "%-3d %-15s %3d %3d %3d %3d %3d %3d %3d %3d %3d %4d %4d %4d %4d %4d %4d %2.1f%%\n",
            static_cast<int>(i + 1),
            username,
            _allUserHighscores[userId].highscores[0],
            _allUserHighscores[userId].highscores[1],
            _allUserHighscores[userId].highscores[2],
            _allUserHighscores[userId].highscores[3],
            _allUserHighscores[userId].highscores[4],
            _allUserHighscores[userId].highscores[5],
            _allUserHighscores[userId].highscores[6],
            _allUserHighscores[userId].highscores[7],
            _allUserHighscores[userId].highscores[8],
            _allUserHighscores[userId].highscores[9],
            _allUserHighscores[userId].highscores[10],
            _allUserHighscores[userId].highscores[11],
            _allUserHighscores[userId].highscores[12],
            _allUserHighscores[userId].highscores[13],
            _allUserHighscores[userId].highscores[14],
            percentScore
        );
    }

    // Footer
    int total_pages = (_allUserHighscores.size() / CONSOLE_HIGHSCORES_COUNT_PER_PAGE) + 1;
    gi.cprintf(ent, PRINT_HIGH, "Page %d/%d (%d users). Use playerscores <page>\n",
        page, total_pages, static_cast<int>(_allUserHighscores.size()));
    gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------\n");
}

/// <summary>
/// Print the playermaps to the client console.
/// </summary>
/// <param name="ent"></param>
void LocalScores::PrintPlayerMaps(edict_t* ent)
{
    int page = 1;
    if (gi.argc() > 1)
    {
        StringToIntMaybe(gi.argv(1), page);
    }
    if (page < 1)
    {
        page = 1;
    }
    size_t index_start = (page - 1) * CONSOLE_HIGHSCORES_COUNT_PER_PAGE;
    if (index_start >= _allMapCounts.size())
    {
        gi.cprintf(ent, PRINT_HIGH, "There are no playermaps for this page.\n");
        return;
    }
    size_t index_end = std::min<size_t>(
        _allMapCounts.size() - 1,
        (page * CONSOLE_HIGHSCORES_COUNT_PER_PAGE) - 1);

    // Header
    gi.cprintf(ent, PRINT_HIGH, "--------------------------------------\n");
    std::string header = GetGreenConsoleText("No. Name            Maps     %\n");
    gi.cprintf(ent, PRINT_HIGH, "%s\n", header.c_str());

    for (size_t i = index_start; i <= index_end; ++i)
    {
        int userId = _allMapCounts[i].first;
        int mapCount = _allMapCounts[i].second;
        float percentDone = (static_cast<float>(mapCount) / _maplistByName.size()) * 100.0f;
        const char* username = _allUsers[userId].c_str();
        gi.cprintf(ent, PRINT_HIGH, "%-3d %-15s %4d  %2.1f\n", static_cast<int>(i + 1), username, mapCount, percentDone);
    }

    // Footer
    int total_pages = (_allMapCounts.size() / CONSOLE_HIGHSCORES_COUNT_PER_PAGE) + 1;
    gi.cprintf(ent, PRINT_HIGH, "Page %d/%d (%d users). Use playermaps <page>\n",
        page, total_pages, static_cast<int>(_allMapCounts.size()));
    gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------\n");
}

/// <summary>
/// Print the maptimes to the client console.
/// </summary>
/// <param name="ent"></param>
void LocalScores::PrintMapTimes(edict_t* ent)
{
    std::string mapname = level.mapname;
    if (gi.argc() >= 2)
    {
        mapname = gi.argv(1);
    }

    const auto& entry = _allMapTimes.find(mapname);
    if (entry == _allMapTimes.end())
    {
        gi.cprintf(ent, PRINT_HIGH, "Invalid map.  Use maptimes <map> <page>.\n");
        return;
    }

    const auto& times = entry->second;
    if (times.empty())
    {
        gi.cprintf(ent, PRINT_HIGH, "No times for %s\n", mapname.c_str());
        return;
    }

    int page = 1;
    if (gi.argc() >= 3)
    {
        StringToIntMaybe(gi.argv(2), page);
    }
    if (page < 1)
    {
        page = 1;
    }
    size_t index_start = (page - 1) * MAX_HIGHSCORES;
    if (index_start >= times.size())
    {
        gi.cprintf(ent, PRINT_HIGH, "There are no maptimes for this page.\n");
        return;
    }
    size_t index_end = std::min<size_t>(
        times.size() - 1,
        (page * MAX_HIGHSCORES) - 1);

    // Header
    gi.cprintf(ent, PRINT_HIGH, "--------------------------------------------------------\n");
    gi.cprintf(ent, PRINT_HIGH, "Best Times for %s\n", mapname.c_str());
    std::string header = "No. Name             Date                           Time";
    header = GetGreenConsoleText(header);
    gi.cprintf(ent, PRINT_HIGH, "%s\n", header.c_str());

    int bestTime = times[0].timeMs;

    for (size_t i = index_start; i <= index_end; ++i)
    {
        int userId = times[i].userId;
        const char* username = _allUsers[userId].c_str();

        std::string date = times[i].date.substr(0, times[i].date.find_first_of(' '));

        std::string timeStr = GetCompletionTimeDisplayString(times[i].timeMs);

        int timeDiff = times[i].timeMs - bestTime;
        std::string timeDiffStr = "0.000";
        if (timeDiff > 0)
        {
            timeDiffStr = GetCompletionTimeDisplayString(timeDiff);
            timeDiffStr.insert(0, "-");   // This should be +, but it looks ugly
        }

        gi.cprintf(ent, PRINT_HIGH, "%-3d %-16s %s %12s %11s\n",
            i + 1, username, date.c_str(), timeDiffStr.c_str(), timeStr.c_str());
    }

    int userId = ent->client->jumpdata->localUserId;
    bool completed = false;
    for (const auto& time : times)
    {
        if (time.userId == userId)
        {
            completed = true;
            break;
        }
    }

    // Footer
    int total_pages = (times.size() / MAX_HIGHSCORES) + 1;
    gi.cprintf(ent, PRINT_HIGH, "Page %d/%d (%d users). Use maptimes <map> <page>\n",
        page, total_pages, static_cast<int>(times.size()));
    gi.cprintf(ent, PRINT_HIGH, "--------------------------------------------------------\n");
    if (completed)
    {
        gi.cprintf(ent, PRINT_HIGH, "You have completed this map\n");
    }
    else
    {
        gi.cprintf(ent, PRINT_HIGH, "You have NOT completed this map\n");
    }
    gi.cprintf(ent, PRINT_HIGH, "--------------------------------------------------------\n");
}

/// <summary>
/// Prints the mapsleft to the client console.
/// </summary>
/// <param name="ent"></param>
void LocalScores::PrintMapsLeft(edict_t* ent)
{
    if (gi.argc() < 3)
    {
        gi.cprintf(ent, PRINT_HIGH, "Invalid mapsleft command. Usage: \"mapsleft <username> <page>\" or \"mapsleft self <page>\"\n");
        gi.cprintf(ent, PRINT_HIGH, "Example: mapsleft self 1\n");
        return;
    }

    std::string username = gi.argv(1);
    int userId = -1;
    if (StringCompareInsensitive(username, "self"))
    {
        userId = ent->client->jumpdata->localUserId;
    }
    else
    {
        userId = LocalDatabase::GetUserId(username);
        if (userId == -1)
        {
            gi.cprintf(ent, PRINT_HIGH, "User not found.\n");
            return;
        }
    }

    int page = 1;
    StringToIntMaybe(gi.argv(2), page);
    if (page < 1)
    {
        page = 1;
    }

    std::vector<std::string> results;
    GetMapsLeft(userId, results);

    size_t index_start = (page - 1) * CONSOLE_HIGHSCORES_COUNT_PER_PAGE;
    if (index_start >= results.size())
    {
        gi.cprintf(ent, PRINT_HIGH, "There are no mapsleft for this page.\n");
        return;
    }
    size_t index_end = std::min<size_t>(
        results.size() - 1,
        (page * CONSOLE_HIGHSCORES_COUNT_PER_PAGE) - 1);

    // Header
    gi.cprintf(ent, PRINT_HIGH, "--------------------------------------\n");
    std::string header = GetGreenConsoleText("No.  Map\n");
    gi.cprintf(ent, PRINT_HIGH, "%s\n", header.c_str());

    for (size_t i = index_start; i <= index_end; ++i)
    {
        gi.cprintf(ent, PRINT_HIGH, "%-4d %-64s\n", static_cast<int>(i + 1), results[i].c_str());
    }

    // Footer
    int totalPages = std::ceil(results.size() / (float)CONSOLE_HIGHSCORES_COUNT_PER_PAGE);
    gi.cprintf(ent, PRINT_HIGH, "Page %d/%d (%d maps). Use mapsleft %s <page>\n",
        page, totalPages, static_cast<int>(results.size()), username.c_str());
    gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------\n");
}

/// <summary>
/// Prints the mapsdone to the client console.
/// </summary>
/// <param name="ent"></param>
void LocalScores::PrintMapsDone(edict_t* ent)
{
    if (gi.argc() < 3)
    {
        gi.cprintf(ent, PRINT_HIGH, "Invalid mapsdone command. Usage: \"mapsdone <username> <page>\" or \"mapsdone self <page>\"\n");
        gi.cprintf(ent, PRINT_HIGH, "Example: mapsdone self 1\n");
        return;
    }

    std::string username = gi.argv(1);
    int userId = -1;
    if (StringCompareInsensitive(username, "self"))
    {
        userId = ent->client->jumpdata->localUserId;
    }
    else
    {
        userId = LocalDatabase::GetUserId(username);
        if (userId == -1)
        {
            gi.cprintf(ent, PRINT_HIGH, "User not found.\n");
            return;
        }
    }

    int page = 1;
    StringToIntMaybe(gi.argv(2), page);
    if (page < 1)
    {
        page = 1;
    }

    auto it = _allUserMaps.find(userId);
    if (it == _allUserMaps.end())
    {
        gi.cprintf(ent, PRINT_HIGH, "Error retrieving mapsdone.\n");
        Logger::Error(va("Could not get mapsdone for user %s, %d", username.c_str(), userId));
        return;
    }
    const std::vector<std::string>& results = it->second;

    size_t index_start = (page - 1) * CONSOLE_HIGHSCORES_COUNT_PER_PAGE;
    if (index_start >= results.size())
    {
        gi.cprintf(ent, PRINT_HIGH, "There are no mapsdone for this page.\n");
        return;
    }
    size_t index_end = std::min<size_t>(
        results.size() - 1,
        (page * CONSOLE_HIGHSCORES_COUNT_PER_PAGE) - 1);

    // Header
    gi.cprintf(ent, PRINT_HIGH, "--------------------------------------\n");
    std::string header = GetGreenConsoleText("No.  Map\n");
    gi.cprintf(ent, PRINT_HIGH, "%s\n", header.c_str());

    for (size_t i = index_start; i <= index_end; ++i)
    {
        gi.cprintf(ent, PRINT_HIGH, "%-4d %-64s\n", static_cast<int>(i + 1), results[i].c_str());
    }

    // Footer
    int totalPages = std::ceil(results.size() / (float)CONSOLE_HIGHSCORES_COUNT_PER_PAGE);
    gi.cprintf(ent, PRINT_HIGH, "Page %d/%d (%d maps). Use mapsdone %s <page>\n",
        page, totalPages, static_cast<int>(results.size()), username.c_str());
    gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------\n");
}

/// <summary>
/// Prints the maplist alphabetically or by date added oldest to newest.
/// </summary>
/// <param name="ent"></param>
/// <param name="alphabetical"></param>
void LocalScores::PrintMaplist(edict_t* ent, bool alphabetical)
{
    const std::vector<std::string>* maplist = &_maplistByAdded;
    if (alphabetical)
    {
        maplist = &_maplistByName;
    }
    if (maplist->empty())
    {
        gi.cprintf(ent, PRINT_HIGH, "There are no maps in the maplist.\n");
        return;
    }

    int page = 1;
    if (gi.argc() > 1)
    {
        StringToIntMaybe(gi.argv(1), page);
    }
    if (page < 1)
    {
        page = 1;
    }
    size_t index_start = (page - 1) * CONSOLE_HIGHSCORES_COUNT_PER_PAGE;
    if (index_start >= maplist->size())
    {
        gi.cprintf(ent, PRINT_HIGH, "There are no maps for this page.\n");
        return;
    }
    size_t index_end = std::min<size_t>(
        maplist->size() - 1,
        (page * CONSOLE_HIGHSCORES_COUNT_PER_PAGE) - 1);

    // Header
    gi.cprintf(ent, PRINT_HIGH, "---------------------------------------------------------\n");

    for (size_t i = index_start; i <= index_end; ++i)
    {
        std::string mapname = (*maplist)[i];
        std::string username;
        int timeMs = 0;
        bool hasTime = GetFirstPlaceForMaplist(mapname, username, timeMs);
        if (mapname.size() > 16)
        {
            mapname = mapname.substr(0, 16);
            mapname += "..."; // trim long mapnames
        }
        if (hasTime)
        {
            std::string timeStr = GetCompletionTimeDisplayString(timeMs);
            gi.cprintf(ent, PRINT_HIGH, "%-4d  %-21s  %-16s  %10s\n",
                static_cast<int>(i + 1),
                mapname.c_str(),
                username.c_str(),
                timeStr.c_str()
            );
        }
        else
        {
            // No time for map
            gi.cprintf(ent, PRINT_HIGH, "%-4d  %-21s\n",
                static_cast<int>(i + 1),
                mapname.c_str()
            );
        }
    }

    // Footer
    std::string command = "maplistnew";
    if (alphabetical)
    {
        command = "maplist";
    }
    int totalPages = std::ceil(maplist->size() / (float)CONSOLE_HIGHSCORES_COUNT_PER_PAGE);
    gi.cprintf(ent, PRINT_HIGH, "Page %d/%d (%d maps). Use %s <page>.\n",
        page, totalPages, static_cast<int>(maplist->size()), command.c_str());
    gi.cprintf(ent, PRINT_HIGH, "---------------------------------------------------------\n");
}

} // namespace Jump