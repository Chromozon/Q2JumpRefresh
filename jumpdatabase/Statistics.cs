using Microsoft.Data.Sqlite;
using Newtonsoft.Json;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace jumpdatabase
{
    public class Statistics
    {
        private class MapTimeModel
        {
            public long MapId { get; set; }
            public long UserId { get; set; }
            public long ServerId { get; set; }
            public long TimeMs { get; set; }
            public long PmoveTimeMs { get; set; }
            public string Date { get; set; }
        }

        private class PlayerTimesJsonObj
        {
            public int page { get; set; }
            public int max_pages { get; set; }
            public int count_per_page { get; set; }
            public int user_count { get; set; }
            public long last_updated { get; set; }
            public class PlayerTimesUserRecord
            {
                public int rank { get; set; } // overall rank, 1-based
                public string username { get; set; }
                public string highscore_counts { get; set; } // comma separated string of top15 counts
                public int total_score { get; set; }
            }
            public List<PlayerTimesUserRecord> user_records { get; set; }
        }

        private class PlayerScoresJsonObj
        {
            public int page { get; set; }
            public int max_pages { get; set; }
            public int count_per_page { get; set; }
            public int user_count { get; set; }
            public long last_updated { get; set; }
            public class PlayerScoresUserRecord
            {
                public int rank { get; set; } // overall rank, 1-based
                public string username { get; set; }
                public string highscore_counts { get; set; } // comma separated string of top15 counts
                public double percent_score { get; set; }
            }
            public List<PlayerScoresUserRecord> user_records { get; set; }
        }

        private class PlayerMapsJsonObj
        {
            public int page { get; set; }
            public int max_pages { get; set; }
            public int count_per_page { get; set; }
            public int user_count { get; set; }
            public long last_updated { get; set; }
            public class PlayerMapsUserRecord
            {
                public int rank { get; set; } // overall rank, 1-based
                public string username { get; set; }
                public int completions { get; set; }
                public double percent_complete { get; set; }
            }
            public List<PlayerMapsUserRecord> user_records { get; set; }
        }

        private class MapTimesJsonObj
        {
            public string mapname { get; set; }
            public int page { get; set; }
            public int max_pages { get; set; }
            public int count_per_page { get; set; }
            public int user_count { get; set; }
            public long last_updated { get; set; }
            public class MapTimesUserRecord
            {
                public int rank { get; set; } // overall rank, 1-based
                public string username { get; set; }
                public string server_name_short { get; set; }
                public long date { get; set; }
                public long time_ms { get; set; }
                public long pmove_time_ms { get; set; }
            }
            public List<MapTimesUserRecord> user_records { get; set; }
        }

        public static void LoadAllStatistics(IDbConnection connection)
        {
            // Get all servers
            Dictionary<long, string> serverIdsShortNames = new Dictionary<long, string>(); // <serverId, shortName>
            var command = connection.CreateCommand();
            command.CommandText = $@"
                SELECT ServerId, ServerNameShort FROM Servers
            ";
            var reader = command.ExecuteReader();
            while (reader.Read())
            {
                long serverId = (long)reader[0];
                string serverShortName = (string)reader[1];
                serverIdsShortNames.Add(serverId, serverShortName);
            }

            // Get all maps
            Dictionary<long, string> mapIdsNames = new Dictionary<long, string>(); // <mapId, mapName>
            command = connection.CreateCommand();
            command.CommandText = $@"
                SELECT MapId, MapName FROM Maps
            ";
            reader = command.ExecuteReader();
            while (reader.Read())
            {
                long mapId = (long)reader[0];
                string mapName = (string)reader[1];
                mapIdsNames.Add(mapId, mapName);
            }

            // Get all users
            Dictionary<long, string> userIdsNames = new Dictionary<long, string>(); // <userId, userName>
            command = connection.CreateCommand();
            command.CommandText = $@"
                SELECT UserId, UserName FROM Users
            ";
            reader = command.ExecuteReader();
            while (reader.Read())
            {
                long userId = (long)reader[0];
                string userName = (string)reader[1];
                userIdsNames.Add(userId, userName);
            }

            // Get highscores list for each map
            Dictionary<long, List<MapTimeModel>> maptimes = new Dictionary<long, List<MapTimeModel>>(); // <mapId, sorted times>
            foreach (var mapIdName in mapIdsNames)
            {
                maptimes.Add(mapIdName.Key, new List<MapTimeModel>());
                command = connection.CreateCommand();
                command.CommandText = $@"
                    SELECT UserId, ServerId, TimeMs, PMoveTimeMs, Date FROM MapTimes
                    WHERE MapId = {mapIdName.Key}
                    ORDER BY TimeMs
                ";
                reader = command.ExecuteReader();
                while (reader.Read())
                {
                    MapTimeModel time = new MapTimeModel();
                    time.MapId = mapIdName.Key;
                    time.UserId = (long)reader[0];
                    time.ServerId = (long)reader[1];
                    time.TimeMs = (long)reader[2];
                    time.PmoveTimeMs = (long)reader[3];
                    time.Date = (string)reader[4];
                    maptimes[mapIdName.Key].Add(time);
                }
            }

            // Calculate global playertimes (top 15)
            const int MaxHighscores = 15;
            Dictionary<long, int[]> userHighscores = new Dictionary<long, int[]>(); // <userId, highscores[15]>
            foreach (var userIdName in userIdsNames)
            {
                long userId = userIdName.Key;
                int[] highscores = new int[MaxHighscores];
                userHighscores.Add(userId, highscores);
            }
            foreach (var maptime in maptimes)
            {
                int maxIndex = MaxHighscores;
                if (maptime.Value.Count < maxIndex)
                {
                    maxIndex = maptime.Value.Count;
                }
                for (int i = 0; i < maxIndex; ++i)
                {
                    long userId = maptime.Value[i].UserId;
                    userHighscores[userId][i]++;
                }
            }

            // Calculate score for each user
            Dictionary<long, int> totalScores = new Dictionary<long, int>(); // <userId, score>
            foreach (var userHighscore in userHighscores)
            {
                long userId = userHighscore.Key;
                int score = CalculateScore(userHighscore.Value);
                totalScores.Add(userId, score);
            }
            var sortedScores = totalScores.OrderByDescending(x => x.Value).ToList();

            // Calculate the maps completed by each user
            Dictionary<long, List<MapTimeModel>> userMapCompletions = new Dictionary<long, List<MapTimeModel>>(); // <userId, all maps>
            foreach (var userIdName in userIdsNames)
            {
                userMapCompletions.Add(userIdName.Key, new List<MapTimeModel>());
            }
            foreach (var maptime in maptimes)
            {
                foreach (var record in maptime.Value)
                {
                    userMapCompletions[record.UserId].Add(record);
                }
            }
            var sortedMapCompletions = userMapCompletions.OrderByDescending(x => x.Value.Count).ToList();

            // Calculate playerscores
            Dictionary<long, double> playerScores = new Dictionary<long, double>(); // <userId, % score>
            foreach (var userIdName in userIdsNames)
            {
                playerScores.Add(userIdName.Key, 0.0);
            }
            foreach (var totalScore in totalScores)
            {
                long userId = totalScore.Key;
                int completions = userMapCompletions[userId].Count;
                int score = totalScore.Value;
                double percent = 0.0;
                if (completions >= 50)
                {
                    percent = (double)score / (completions * 25) * 100;
                }
                playerScores[userId] = percent;
            }
            var sortedPlayerScores = playerScores.OrderByDescending(x => x.Value).ToList();

            // Update the cached statistics
            _cacheServerIdsShortNames = serverIdsShortNames;
            _cacheUserIdsNames = userIdsNames;
            _cacheMapIdsNames = mapIdsNames;
            _cacheMaptimes = maptimes;
            _cacheUserHighscores = userHighscores;
            _cacheTotalScores = sortedScores;
            _cacheUserMapCompletions = userMapCompletions;
            _cacheCompletions = sortedMapCompletions;
            _cachePercentScores = sortedPlayerScores;
            _cacheLastUpdated = DateTime.UtcNow;
        }

        /// <summary>
        /// Get the playertimes json string.
        /// </summary>
        /// <param name="page">1-based</param>
        /// <param name="limit">How many results per page</param>
        /// <returns></returns>
        public static string GetPlayerTimesJson(int page, int limit = 20)
        {
            if (page < 1)
            {
                page = 1;
            }
            PlayerTimesJsonObj playerTimes = new PlayerTimesJsonObj();
            playerTimes.page = page;
            playerTimes.max_pages = (int)Math.Ceiling(_cacheTotalScores.Count / (double)limit);
            playerTimes.count_per_page = limit;
            playerTimes.user_count = _cacheTotalScores.Count;
            playerTimes.last_updated = GetUnixTimestamp(_cacheLastUpdated);
            playerTimes.user_records = new List<PlayerTimesJsonObj.PlayerTimesUserRecord>();

            int startIndex = (page - 1) * limit;
            int endIndex = startIndex + limit;
            for (int i = startIndex; i < endIndex; ++i)
            {
                if (i >= _cacheTotalScores.Count)
                {
                    break;
                }
                long userId = _cacheTotalScores[i].Key;
                int totalScore = _cacheTotalScores[i].Value;
                string userName = _cacheUserIdsNames[userId];
                string highscoreCounts = GetHighscoreCountString(_cacheUserHighscores[userId]);

                PlayerTimesJsonObj.PlayerTimesUserRecord record = new PlayerTimesJsonObj.PlayerTimesUserRecord();
                record.rank = i + 1;
                record.username = userName;
                record.highscore_counts = highscoreCounts;
                record.total_score = totalScore;
                playerTimes.user_records.Add(record);
            }

            string json = JsonConvert.SerializeObject(playerTimes);
            return json;
        }

        /// <summary>
        /// Get the playerscores json string.
        /// </summary>
        /// <param name="page">1-based</param>
        /// <param name="limit">How many results per page</param>
        /// <returns></returns>
        public static string GetPlayerScoresJson(int page, int limit = 20)
        {
            if (page < 1)
            {
                page = 1;
            }
            PlayerScoresJsonObj playerScores = new PlayerScoresJsonObj();
            playerScores.page = page;
            playerScores.max_pages = (int)Math.Ceiling(_cachePercentScores.Count / (double)limit);
            playerScores.count_per_page = limit;
            playerScores.user_count = _cachePercentScores.Count;
            playerScores.last_updated = GetUnixTimestamp(_cacheLastUpdated);
            playerScores.user_records = new List<PlayerScoresJsonObj.PlayerScoresUserRecord>();

            int startIndex = (page - 1) * limit;
            int endIndex = startIndex + limit;
            for (int i = startIndex; i < endIndex; ++i)
            {
                if (i >= _cachePercentScores.Count)
                {
                    break;
                }
                long userId = _cachePercentScores[i].Key;
                double percentScore = _cachePercentScores[i].Value;
                string userName = _cacheUserIdsNames[userId];
                string highscoreCounts = GetHighscoreCountString(_cacheUserHighscores[userId]);

                PlayerScoresJsonObj.PlayerScoresUserRecord record = new PlayerScoresJsonObj.PlayerScoresUserRecord();
                record.rank = i + 1;
                record.username = userName;
                record.highscore_counts = highscoreCounts;
                record.percent_score = percentScore;
                playerScores.user_records.Add(record);
            }

            string json = JsonConvert.SerializeObject(playerScores);
            return json;
        }

        /// <summary>
        /// Get the playermaps json string.
        /// </summary>
        /// <param name="page">1-based</param>
        /// <param name="limit">How many results per page</param>
        /// <returns></returns>
        public static string GetPlayerMapsJson(int page, int limit = 20)
        {
            if (page < 1)
            {
                page = 1;
            }
            PlayerMapsJsonObj playerMaps = new PlayerMapsJsonObj();
            playerMaps.page = page;
            playerMaps.max_pages = (int)Math.Ceiling(_cacheCompletions.Count / (double)limit);
            playerMaps.count_per_page = limit;
            playerMaps.user_count = _cacheCompletions.Count;
            playerMaps.last_updated = GetUnixTimestamp(_cacheLastUpdated);
            playerMaps.user_records = new List<PlayerMapsJsonObj.PlayerMapsUserRecord>();

            int startIndex = (page - 1) * limit;
            int endIndex = startIndex + limit;
            for (int i = startIndex; i < endIndex; ++i)
            {
                if (i >= _cacheCompletions.Count)
                {
                    break;
                }
                long userId = _cacheCompletions[i].Key;
                string userName = _cacheUserIdsNames[userId];
                int completions = _cacheCompletions[i].Value.Count;
                double percentComplete = (double)completions / _cacheMapIdsNames.Count * 100;

                PlayerMapsJsonObj.PlayerMapsUserRecord record = new PlayerMapsJsonObj.PlayerMapsUserRecord();
                record.rank = i + 1;
                record.username = userName;
                record.completions = completions;
                record.percent_complete = percentComplete;
                playerMaps.user_records.Add(record);
            }

            string json = JsonConvert.SerializeObject(playerMaps);
            return json;
        }

        /// <summary>
        /// Get the maptimes json string.
        /// </summary>
        /// <param name="mapname"></param>
        /// <param name="page">1-based</param>
        /// <param name="limit">How many results per page</param>
        /// <returns>If mapname is not found, returns json with empty mapname</returns>
        public static string GetMapTimesJson(string mapname, int page, int limit = 15)
        {
            if (page < 1)
            {
                page = 1;
            }
            MapTimesJsonObj mapTimes = new MapTimesJsonObj();

            // Default values in case the desired mapname does not exist
            mapTimes.mapname = string.Empty;
            mapTimes.user_records = new List<MapTimesJsonObj.MapTimesUserRecord>();

            var foundKey = _cacheMapIdsNames.Where(x => x.Value == mapname).FirstOrDefault();
            if (foundKey.Value == mapname)
            {
                long mapId = foundKey.Key;
                mapTimes.mapname = mapname;
                mapTimes.page = page;
                mapTimes.max_pages = (int)Math.Ceiling(_cacheMaptimes[mapId].Count / (double)limit);
                mapTimes.count_per_page = limit;
                mapTimes.user_count = _cacheMaptimes[mapId].Count;
                mapTimes.last_updated = GetUnixTimestamp(_cacheLastUpdated);

                int startIndex = (page - 1) * limit;
                int endIndex = startIndex + limit;
                for (int i = startIndex; i < endIndex; ++i)
                {
                    if (i >= _cacheMaptimes[mapId].Count)
                    {
                        break;
                    }
                    long userId = _cacheMaptimes[mapId][i].UserId;
                    string userName = _cacheUserIdsNames[userId];
                    string serverNameShort = _cacheServerIdsShortNames[_cacheMaptimes[mapId][i].ServerId];
                    DateTime dateTime = DateTime.ParseExact(
                        _cacheMaptimes[mapId][i].Date, DateTimeFormat, CultureInfo.InvariantCulture);
                    long date = GetUnixTimestamp(dateTime);
                    long timeMs = _cacheMaptimes[mapId][i].TimeMs;
                    long pmoveTimeMs = _cacheMaptimes[mapId][i].PmoveTimeMs;

                    MapTimesJsonObj.MapTimesUserRecord record = new MapTimesJsonObj.MapTimesUserRecord();
                    record.rank = i + 1;
                    record.server_name_short = serverNameShort;
                    record.username = userName;
                    record.date = date;
                    record.time_ms = timeMs;
                    record.pmove_time_ms = pmoveTimeMs;
                    mapTimes.user_records.Add(record);
                }
            }
            string json = JsonConvert.SerializeObject(mapTimes);
            return json;
        }

        private static void DebugPrintPlayerTimes()
        {
            //foreach (var sortedScore in sortedScores)
            //{
            //    long userId = sortedScore.Key;
            //    int score = sortedScore.Value;
            //    if (score == 0)
            //    {
            //        break;
            //    }
            //    string userName = userIdsNames[userId];
            //    var highscores = userHighscores[userId];
            //    Console.WriteLine(string.Format("{0} {1} {2} {3} {4} {5} {6} {7} {8} {9} {10} {11} {12} {13} {14} {15} {16}",
            //        userName, highscores[0], highscores[1], highscores[2], highscores[3], highscores[4],
            //        highscores[5], highscores[6], highscores[7], highscores[8], highscores[9], highscores[10],
            //        highscores[11], highscores[12], highscores[13], highscores[14], score));
            //}
        }

        private static void DebugPrintPlayerMaps()
        {
            //foreach (var record in sortedMapCompletions)
            //{
            //    string userName = userIdsNames[record.Key];
            //    int count = record.Value.Count;
            //    double percent = (double)count / mapIdsNames.Count * 100;
            //    Console.WriteLine($"{userName} {count} ({percent.ToString("0.00")})");
            //}
        }

        private static void DebugPrintPlayerScores()
        {
            //foreach (var record in sortedPlayerScores)
            //{
            //    string userName = userIdsNames[record.Key];
            //    double percent = record.Value;
            //    Console.WriteLine($"{userName} ({percent.ToString("0.00")})");
            //}
        }

        /// <summary>
        /// Calculate the player score based on how many top15 highscores they have.
        /// </summary>
        /// <param name="highscores"></param>
        /// <returns></returns>
        private static int CalculateScore(int[] highscores)
        {
            System.Diagnostics.Debug.Assert(highscores.Length == 15, "Highscores must be top15");
            return
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
        }

        /// <summary>
        /// Returns a comma separated string of all highscore counts.
        /// Ex: 12,10,8,4,5,0,0,0,2,0,0,0,1,0,0
        /// </summary>
        /// <param name="highscores"></param>
        /// <returns></returns>
        private static string GetHighscoreCountString(int[] highscores)
        {
            System.Diagnostics.Debug.Assert(highscores.Length == 15, "Highscores must be top15");
            return string.Join(',', highscores);
        }

        /// <summary>
        /// Given a DateTime, return the Unix timestamp (seconds since 1970)
        /// </summary>
        /// <param name="dateTime"></param>
        /// <returns></returns>
        private static long GetUnixTimestamp(DateTime dateTime)
        {
            DateTime utcTime = DateTime.SpecifyKind(dateTime, DateTimeKind.Utc);
            return ((DateTimeOffset)utcTime).ToUnixTimeSeconds();
        }

        private const string DateTimeFormat = "yyyy-MM-dd HH:mm:ss"; // format supported by sqlite db

        /// <summary>
        /// Table of [serverId, server short name]
        /// </summary>
        private static Dictionary<long, string> _cacheServerIdsShortNames =
            new Dictionary<long, string>();

        /// <summary>
        /// Table of [userId, userName]
        /// </summary>
        private static Dictionary<long, string> _cacheUserIdsNames =
            new Dictionary<long, string>();

        /// <summary>
        /// Table of [mapId, mapName]
        /// </summary>
        private static Dictionary<long, string> _cacheMapIdsNames =
            new Dictionary<long, string>();

        /// <summary>
        /// Table of [mapId, list of completion times sorted best to worst]
        /// </summary>
        private static Dictionary<long, List<MapTimeModel>> _cacheMaptimes =
            new Dictionary<long, List<MapTimeModel>>();

        /// <summary>
        /// Table of [userId, top15 counts]
        /// </summary>
        private static Dictionary<long, int[]> _cacheUserHighscores =
            new Dictionary<long, int[]>();

        /// <summary>
        /// List of [userId, score] sorted by score best to worst
        /// </summary>
        private static List<KeyValuePair<long, int>> _cacheTotalScores =
            new List<KeyValuePair<long, int>>();

        /// <summary>
        /// Table of [userId, map completion records]
        /// </summary>
        private static Dictionary<long, List<MapTimeModel>> _cacheUserMapCompletions =
            new Dictionary<long, List<MapTimeModel>>();

        /// <summary>
        /// List of [userId, map completion records] sorted by total map completions best to worst
        /// </summary>
        private static List<KeyValuePair<long, List<MapTimeModel>>> _cacheCompletions =
            new List<KeyValuePair<long, List<MapTimeModel>>>();

        /// <summary>
        /// List of [userId, percent score] sorted by percent score best to worst
        /// </summary>
        private static List<KeyValuePair<long, double>> _cachePercentScores =
            new List<KeyValuePair<long, double>>();

        /// <summary>
        /// UTC time when the cache was last updated
        /// </summary>
        private static DateTime _cacheLastUpdated = DateTime.MinValue;
    }
}
