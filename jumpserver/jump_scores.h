#pragma once

#include <string>
#include <vector>
#include <map>
#include "jump_types.h"
#include "jump_local_database.h"

namespace Jump
{

/// <summary>
/// This class manages the highscore statistics (playermaps, playertimes, playerscores, etc.).
/// Note that these are calculated every map change.  So if you use a function from this class,
/// it won't have any updates from the current map.
/// </summary>
class LocalScores
{
public:
    static void LoadMaplist();
    static std::string GetRandomMap();
    static bool IsMapInMaplist(const std::string& mapname);
    static void CalculateAllStatistics();
    static int GetTotalTimesForMap(const std::string& mapname);
    static std::string GetUserName(int userId);
    static int GetServerMapCount();

    static void PrintPlayerTimes(edict_t* ent);
    static void PrintPlayerScores(edict_t* ent);
    static void PrintPlayerMaps(edict_t* ent);
    static void PrintMapTimes(edict_t* ent);
    static void PrintMapsLeft(edict_t* ent);
    static void PrintMapsDone(edict_t* ent);

private:
    // Helper functions
    static int CalculateTotalScore(const std::array<int, 15>& highscores);
    static float CalculatePercentScore(int totalScore, int userMapCount);
    static bool GetMapsLeft(int userId, std::vector<std::string>& results);
    static bool GetMapsDone(int userId, std::vector<std::string>& results);

    static std::vector<std::string> _maplist; // <mapnames sorted alphabetically>

    // Statistics cache (reloaded between levels)
    static std::map<std::string, std::vector<MapTimesEntry>> _allMapTimes; // <mapname, times sorted best to worst>
    static std::map<int, std::string> _allUsers; // <userId, username>
    static std::map<int, UserHighscores> _allUserHighscores; // <userId, highscores>
    static std::map<int, std::vector<std::string>> _allUserMaps; // <userId, list of completed maps sorted alphabetically>
    static std::vector<std::pair<int, int>> _allTotalScores; // <userId, total score> sorted best to worst
    static std::vector<std::pair<int, float>> _allPercentScores; // <userId, percent score> sorted best to worst
    static std::vector<std::pair<int, int>> _allMapCounts; // <userId, mapcount> sorted best to worst
};

} // namespace Jump

