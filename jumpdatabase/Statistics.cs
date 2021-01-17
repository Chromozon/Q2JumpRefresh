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

        public static void LoadAllStatistics(IDbConnection connection)
        {
            // Get all maps
            Dictionary<long, string> mapIdsNames = new Dictionary<long, string>(); // <mapId, mapName>
            var command = connection.CreateCommand();
            command.CommandText = $@"
                SELECT MapId, MapName FROM Maps
            ";
            var reader = command.ExecuteReader();
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
            _cacheUserIdsNames = userIdsNames;
            _cacheMapIdsNames = mapIdsNames;
            _cacheMaptimes = maptimes;
            _cacheUserHighscores = userHighscores;
            _cacheTotalScores = sortedScores;
            _cacheUserMapCompletions = userMapCompletions;
            _cacheCompletions = sortedMapCompletions;
            _cachePercentScores = sortedPlayerScores;
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
    }
}
