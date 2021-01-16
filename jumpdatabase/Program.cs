﻿using Microsoft.Data.Sqlite;
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
    class Program
    {
        class TimeRecord
        {
            public string UserName { get; set; }
            public string ServerNameShort { get; set; }
            public DateTime Date { get; set; }
            public long TimeMs { get; set; }
        }

        class MapTimeModel
        {
            public long MapId { get; set; }
            public long UserId { get; set; }
            public long ServerId { get; set; }
            public long TimeMs { get; set; }
            public long PmoveTimeMs { get; set; }
            public string Date { get; set; }
        }

        const int ServicePort = 57540;
        const string DatabasePath = "./jumpdatabase.sqlite3";
        const string DateTimeFormat = "yyyy-MM-dd HH:mm:ss"; // format supported by sqlite db

        static void Main(string[] args)
        {
            // Initialize the database
            SqliteConnection dbConnection = new SqliteConnection($"Data Source={DatabasePath}");
            dbConnection.Open();
            Console.WriteLine($"Opened connection to database \"{DatabasePath}\"");
            Tables.CreateAllTables(dbConnection);
            LoadServerLoginsCache(dbConnection);
            LoadAllStatistics(dbConnection);
            return;

            // Start listening for requests from the various servers
            HttpListener httpListener = new HttpListener();
            httpListener.Prefixes.Add($"http://localhost:{ServicePort}/");
            httpListener.Start();
            Console.WriteLine($"Q2 Jump Database service listening on port {ServicePort}");

            while (true)
            {
                try
                {
                    // Receive a request from a server
                    var context = httpListener.GetContext();
                    var request = context.Request;
                    var response = context.Response;

                    // Parse the request
                    if (!request.ContentType.Contains("application/json"))
                    {
                        response.StatusCode = (int)HttpStatusCode.BadRequest;
                        response.Close();
                        continue;
                    }
                    MemoryStream memoryStream = new MemoryStream();
                    request.InputStream.CopyTo(memoryStream);
                    memoryStream.Position = 0;
                    dynamic json = JsonConvert.DeserializeObject(Encoding.ASCII.GetString(memoryStream.ToArray()));

                    string loginToken = json.login_token;
                    if (loginToken == null)
                    {
                        response.StatusCode = (int)HttpStatusCode.BadRequest;
                        response.Close();
                        continue;
                    }
                    long serverId = -1;
                    if (!_serverLogins.TryGetValue(loginToken, out serverId))
                    {
                        response.StatusCode = (int)HttpStatusCode.BadRequest;
                        response.Close();
                        continue;
                    }
                    string command = json.command;
                    if (string.IsNullOrEmpty(command))
                    {
                        response.StatusCode = (int)HttpStatusCode.BadRequest;
                        response.Close();
                        continue;
                    }
                    dynamic commandArgs = json.command_args;
                    if (commandArgs == null)
                    {
                        response.StatusCode = (int)HttpStatusCode.BadRequest;
                        response.Close();
                        continue;
                    }

                    // Execute the command
                    int responseStatus = (int)HttpStatusCode.BadRequest;
                    string responseData = string.Empty;
                    switch (command)
                    {
                        case "addtime":
                            // TODO add replay to file system
                            HandleCommandAddTime(dbConnection, serverId, commandArgs, out responseStatus);
                            break;
                        case "gettimes":
                            HandleCommandGetTimes(dbConnection, commandArgs, out responseStatus, out responseData);
                            break;
                        case "userlogin":
                            HandleCommandUserLogin(dbConnection, commandArgs, out responseStatus);
                            break;
                        case "changepassword":
                            HandleCommandChangePassword(dbConnection, commandArgs, out responseStatus);
                            break;
                        case "adduserprivate":
                            HandleCommandAddUserPrivate(dbConnection, commandArgs, out responseStatus);
                            break;
                        case "addmap":
                            HandleCommandAddMap(dbConnection, commandArgs, out responseStatus);
                            break;
                        default:
                            break;
                    }

                    // Send response back to the server
                    response.StatusCode = responseStatus;
                    var outputStream = response.OutputStream;
                    outputStream.Write(Encoding.ASCII.GetBytes(responseData));
                    outputStream.Close();
                    response.Close();
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Exception: {e}");
                }
            }
        }

        /// <summary>
        /// Adds a new best time for the given user and map.
        /// We ensure that the new time is faster than whatever is currently in the database.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="serverId"></param>
        /// <param name="args">
        /// {
        ///     "mapname": "mapname" (string, no file extension)
        ///     "username": "username" (string)
        ///     "date": 1610596223836 (int, Unix time s)
        ///     "time_ms": 234934 (int, ms)
        ///     "pmove_time_ms": 223320 (int, ms, -1 means no time)
        /// }
        /// </param>
        /// <param name="status"></param>
        static private void HandleCommandAddTime(IDbConnection connection, long serverId, dynamic args,
            out int status)
        {
            status = (int)HttpStatusCode.BadRequest;

            string mapname = args.mapname;
            string username = args.username;
            long? date = args.date;
            long? timeMs = args.time_ms;
            long? pmoveTimeMs = args.pmove_time_ms;
            if (mapname == null || username == null || date == null || timeMs == null || pmoveTimeMs == null)
            {
                return;
            }
            DateTime dateTime = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc) + TimeSpan.FromSeconds(date.Value);
            string dateStr = dateTime.ToString(DateTimeFormat);

            var command = connection.CreateCommand();
            command.CommandText = $@"
                INSERT OR IGNORE INTO MapTimes (MapId, UserId, ServerId, TimeMs, PMoveTimeMs, Date)
                VALUES (
                    (SELECT MapId FROM Maps WHERE MapName = @mapname),
                    (SELECT UserId FROM Users WHERE UserName = @username),
                    {serverId},
                    {timeMs.Value},
                    {pmoveTimeMs.Value},
                    @date
                );
                UPDATE MapTimes
                SET ServerId = {serverId}, TimeMs = {timeMs.Value}, PMoveTimeMs = {pmoveTimeMs.Value}, Date = @date
                WHERE
                    MapId = (SELECT MapId FROM Maps WHERE MapName = @mapname)
                    AND
                    UserId = (SELECT UserId FROM Users WHERE UserName = @username)
                    AND
                    TimeMs > {timeMs.Value}
            ";
            SqliteParameter paramMapName = new SqliteParameter("@mapname", SqliteType.Text);
            paramMapName.Value = mapname;
            SqliteParameter paramUserName = new SqliteParameter("@username", SqliteType.Text);
            paramUserName.Value = username;
            SqliteParameter paramDate = new SqliteParameter("@date", SqliteType.Text);
            paramDate.Value = dateStr;
            command.Parameters.Add(paramMapName);
            command.Parameters.Add(paramUserName);
            command.Parameters.Add(paramDate);
            command.Prepare();
            int rows = command.ExecuteNonQuery();
            if (rows == 1)
            {
                status = (int)HttpStatusCode.OK;
            }
        }

        /// <summary>
        /// Retrieves a set of times for the given map.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="args">
        /// {
        ///     "mapname": "mapname" (string)
        ///     "page": 123 (int, 1-based, any value < 1 is coerced to 1)
        /// }
        /// </param>
        /// <param name="status"></param>
        /// <param name="data"></param>
        static private void HandleCommandGetTimes(IDbConnection connection, dynamic args,
            out int status, out string data)
        {
            const int ResultsPerQuery = 15;

            status = (int)HttpStatusCode.BadRequest;
            data = string.Empty;

            string mapName = args.mapname;
            int? page = args.page;
            if (mapName == null || page == null)
            {
                return;
            }
            if (page.Value < 1)
            {
                page = 1;
            }
            int offset = (page.Value - 1) * ResultsPerQuery;
            List<TimeRecord> times = new List<TimeRecord>();

            var command = connection.CreateCommand();
            command.CommandText = $@"
                SELECT Users.UserName, Servers.ServerNameShort, MapTimes.TimeMs, MapTimes.Date FROM MapTimes
                INNER JOIN Users ON MapTimes.UserId = Users.UserId
                INNER JOIN Servers ON MapTimes.ServerId = Servers.ServerId
                WHERE MapId =
                    (SELECT MapId FROM Maps WHERE MapName = @mapname)
                ORDER BY TimeMs
                LIMIT {ResultsPerQuery} OFFSET {offset}
            ";
            SqliteParameter paramMapName = new SqliteParameter("@mapname", SqliteType.Text);
            paramMapName.Value = mapName;
            command.Parameters.Add(paramMapName);
            var reader = command.ExecuteReader();
            while (reader.Read())
            {
                TimeRecord timeRecord = new TimeRecord();
                timeRecord.UserName = (string)reader[0];
                timeRecord.ServerNameShort = (string)reader[1];
                timeRecord.TimeMs = (long)reader[2];
                timeRecord.Date = DateTime.ParseExact((string)reader[3], DateTimeFormat, CultureInfo.InvariantCulture);
                times.Add(timeRecord);
            }
            data = JsonConvert.SerializeObject(times);
            status = (int)HttpStatusCode.OK;
        }

        /// <summary>
        /// Tries to validate the usename and password.  If the user does not exist, adds a new user.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="args">
        /// {
        ///     "username": "username" (string)
        ///     "password": "password" (string)
        /// }
        /// </param>
        /// <param name="status"></param>
        static private void HandleCommandUserLogin(IDbConnection connection, dynamic args, out int status)
        {
            status = (int)HttpStatusCode.BadRequest;

            string userName = args.username;
            string password = args.password;
            if (string.IsNullOrEmpty(userName) || string.IsNullOrEmpty(password))
            {
                return;
            }

            var command = connection.CreateCommand();
            command.CommandText = $@"
                INSERT OR IGNORE INTO Users (UserName, Password)
                VALUES (@username, @password)
                ;
                SELECT UserId FROM Users WHERE UserName = @username AND Password = @password
            ";
            SqliteParameter paramUserName = new SqliteParameter("@username", SqliteType.Text);
            paramUserName.Value = userName;
            SqliteParameter paramPassword = new SqliteParameter("@password", SqliteType.Text);
            paramPassword.Value = password;
            command.Parameters.Add(paramUserName);
            command.Parameters.Add(paramPassword);
            var reader = command.ExecuteReader();
            bool correctPassword = false;
            while (reader.Read())
            {
                correctPassword = true;
                break;
            }
            if (correctPassword)
            {
                status = (int)HttpStatusCode.OK;
            }
        }

        /// <summary>
        /// Change a user password.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="args">
        /// {
        ///     "username": "username" (string)
        ///     "password_old": "password" (string)
        ///     "password_new": "password" (string)
        /// }
        /// </param>
        /// <param name="status"></param>
        static private void HandleCommandChangePassword(IDbConnection connection, dynamic args, out int status)
        {
            status = (int)HttpStatusCode.BadRequest;

            string userName = args.username;
            string passwordOld = args.password_old;
            string passwordNew = args.password_new;
            if (string.IsNullOrEmpty(userName) || string.IsNullOrEmpty(passwordOld) || string.IsNullOrEmpty(passwordNew))
            {
                return;
            }

            var command = connection.CreateCommand();
            command.CommandText = $@"
                UPDATE Users SET Password = @password_new
                WHERE UserName = @username AND Password = @password_old
            ";
            SqliteParameter paramUserName = new SqliteParameter("@username", SqliteType.Text);
            paramUserName.Value = userName;
            SqliteParameter paramPasswordOld = new SqliteParameter("@password_old", SqliteType.Text);
            paramPasswordOld.Value = passwordOld;
            SqliteParameter paramPasswordNew = new SqliteParameter("@password_new", SqliteType.Text);
            paramPasswordNew.Value = passwordNew;
            command.Parameters.Add(paramUserName);
            command.Parameters.Add(paramPasswordOld);
            command.Parameters.Add(paramPasswordNew);
            int rows = command.ExecuteNonQuery();
            if (rows == 1)
            {
                status = (int)HttpStatusCode.OK;
            }
        }

        /// <summary>
        /// Add a new map.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="args">
        /// {
        ///     "mapname": "mapname" (string, no extension)
        /// }
        /// </param>
        /// <param name="status"></param>
        static private void HandleCommandAddMap(IDbConnection connection, dynamic args, out int status)
        {
            status = (int)HttpStatusCode.BadRequest;

            string mapname = args.mapname;
            if (string.IsNullOrEmpty(mapname))
            {
                return;
            }
            string dateAdded = DateTime.UtcNow.ToString(DateTimeFormat);

            var command = connection.CreateCommand();
            command.CommandText = $@"
                INSERT INTO Maps (MapName, DateAdded)
                VALUES (@mapname, @dateadded)
            ";
            SqliteParameter paramMapName = new SqliteParameter("@mapname", SqliteType.Text);
            paramMapName.Value = mapname;
            SqliteParameter paramDateAdded = new SqliteParameter("@dateadded", SqliteType.Text);
            paramDateAdded.Value = dateAdded;
            command.Parameters.Add(paramMapName);
            command.Parameters.Add(paramDateAdded);
            int rows = command.ExecuteNonQuery();
            if (rows == 1)
            {
                status = (int)HttpStatusCode.OK;
            }
        }

        /// <summary>
        /// Add an existing user from the old jump server files.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="args">
        /// {
        ///     "userid": 123 (int)
        ///     "username": "username" (string)
        ///     "password": "password" (string)
        /// }
        /// </param>
        /// <param name="status"></param>
        static private void HandleCommandAddUserPrivate(IDbConnection connection, dynamic args, out int status)
        {
            status = (int)HttpStatusCode.BadRequest;

            int? userId = args.userid;
            string userName = args.username;
            string password = args.password;
            if (userId == null || string.IsNullOrEmpty(userName) || string.IsNullOrEmpty(password))
            {
                return;
            }

            var command = connection.CreateCommand();
            command.CommandText = $@"
                INSERT INTO Users (UserId, UserName, Password)
                VALUES ({userId.Value}, @username, @password)
            ";
            SqliteParameter paramUserName = new SqliteParameter("@username", SqliteType.Text);
            paramUserName.Value = userName;
            SqliteParameter paramPassword = new SqliteParameter("@password", SqliteType.Text);
            paramPassword.Value = password;
            command.Parameters.Add(paramUserName);
            command.Parameters.Add(paramPassword);
            int rows = command.ExecuteNonQuery();
            if (rows == 1)
            {
                status = (int)HttpStatusCode.OK;
            }
        }

        static private void LoadAllStatistics(IDbConnection connection)
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

            var sortedScores = totalScores.OrderByDescending(x => x.Value);
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

            // Calculate total map completions
            Dictionary<long, int> mapCompletions = new Dictionary<long, int>(); // <userId, maps completed>
            foreach (var userIdName in userIdsNames)
            {
                mapCompletions.Add(userIdName.Key, 0);
            }
            foreach (var maptime in maptimes)
            {
                foreach (var time in maptime.Value)
                {
                    mapCompletions[time.UserId]++;
                }
            }

            var sortedMapCompletions = mapCompletions.OrderBy(x => x.Value);
            //foreach (var record in sortedMapCompletions)
            //{
            //    string userName = userIdsNames[record.Key];
            //    int count = record.Value;
            //    double percent = (double)count / mapIdsNames.Count * 100;
            //    Console.WriteLine($"{userName} {count} ({percent.ToString("0.00")})");
            //}

            Console.WriteLine("nothing");
        }

        static int CalculateScore(int[] highscores)
        {
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
        /// Load the server login tokens so we don't have to check this for each query.
        /// </summary>
        /// <param name="connection"></param>
        static private void LoadServerLoginsCache(IDbConnection connection)
        {
            _serverLogins.Clear();
            var command = connection.CreateCommand();
            command.CommandText = @"
                SELECT ServerId, LoginToken FROM Servers
            ";
            var reader = command.ExecuteReader();
            while (reader.Read())
            {
                long serverId = (long)reader[0];
                string loginToken = (string)reader[1];
                _serverLogins.Add(loginToken, serverId);
            }
        }

        // Cache the server LoginToken -> ServerId
        static private Dictionary<string, long> _serverLogins = new Dictionary<string, long>();
    }
}
