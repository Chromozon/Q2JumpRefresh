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
    class Program
    {
        const int ServicePort = 57540;
        const string DatabasePath = "./jumpdatabase.sqlite3";
        const string DateTimeFormat = "yyyy-MM-dd HH:mm:ss"; // format supported by sqlite db
        const int StatisticsRefreshTimeMs = 1000 * 60 * 5; // 5 minutes
        const string ReplayDirectory = "./replays/";
        const string ReplayExtension = ".demo";

        static void Main(string[] args)
        {
            // Initialize the database
            SqliteConnection dbConnection = new SqliteConnection($"Data Source={DatabasePath}");
            dbConnection.Open();
            Console.WriteLine($"Opened connection to database \"{DatabasePath}\"");
            Tables.CreateAllTables(dbConnection);
            LoadServerLoginsCache(dbConnection);
            Statistics.LoadAllStatistics(dbConnection);
            _lastStatisticsRefresh = DateTime.UtcNow;
            Directory.CreateDirectory(ReplayDirectory);

            // Start listening for requests from the various servers
            HttpListener httpListener = new HttpListener();
            httpListener.Prefixes.Add($"http://localhost:{ServicePort}/");
            httpListener.Start();
            Console.WriteLine($"Q2 Jump Database service listening on port {ServicePort}");

            while (true)
            {
                try
                {
                    // Update the statistics if the timeout has expired
                    DateTime now = DateTime.UtcNow;
                    TimeSpan timeSpan = _lastStatisticsRefresh.Subtract(now);
                    if (timeSpan.TotalMilliseconds > StatisticsRefreshTimeMs)
                    {
                        Statistics.LoadAllStatistics(dbConnection);
                        _lastStatisticsRefresh = now;
                    }

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
                        case "playertimes":
                            HandleCommandPlayertimes(dbConnection, commandArgs, out responseStatus, out responseData);
                            break;
                        case "playerscores":
                            HandleCommandPlayerscores(dbConnection, commandArgs, out responseStatus, out responseData);
                            break;
                        case "playermaps":
                            HandleCommandPlayermaps(dbConnection, commandArgs, out responseStatus, out responseData);
                            break;
                        case "maptimes":
                            HandleCommandMaptimes(dbConnection, commandArgs, out responseStatus, out responseData);
                            break;
                            // TODO: get replay cmd
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

        private static void ListenerCallback(IAsyncResult result)
        {
            HttpListener httpListener = (HttpListener)result.AsyncState;
            httpListener?.EndGetContext(result);
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
        ///     "replay_data": "dGhpcyBpcyBhIHRlc3Q=" (string, base64 encoded)
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
            string replayDataB64 = args.replay_data;
            if (string.IsNullOrEmpty(mapname) || string.IsNullOrEmpty(username) || date == null || timeMs == null ||
                pmoveTimeMs == null || string.IsNullOrEmpty(replayDataB64))
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
                // Save the replay
                byte[] replayData = Convert.FromBase64String(replayDataB64);

                // TODO

                status = (int)HttpStatusCode.OK;
            }
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
                INSERT OR IGNORE INTO Maps (MapName, DateAdded)
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

        /// <summary>
        /// Get maptimes for a particular map.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="args">
        /// {
        ///     "mapname": "mapname" (string, no file extension)
        ///     "page": 123 (int, 1-based)
        ///     "count_per_page": 15 (int)
        /// }
        /// </param>
        /// <param name="status"></param>
        /// <param name="data"></param>
        static private void HandleCommandMaptimes(IDbConnection connection, dynamic args, out int status,
            out string data)
        {
            status = (int)HttpStatusCode.BadRequest;
            data = string.Empty;

            string mapname = args.mapname;
            int? page = args.page;
            int? limit = args.count_per_page;
            if (string.IsNullOrEmpty(mapname) || page == null || limit == null)
            {
                return;
            }
            data = Statistics.GetMapTimesJson(mapname, page.Value, limit.Value);
            status = (int)HttpStatusCode.OK;
        }

        /// <summary>
        /// Get playertimes.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="args">
        /// {
        ///     "page": 123 (int, 1-based)
        ///     "count_per_page": 20 (int)
        /// }
        /// </param>
        /// <param name="status"></param>
        /// <param name="data"></param>
        static private void HandleCommandPlayertimes(IDbConnection connection, dynamic args, out int status,
            out string data)
        {
            status = (int)HttpStatusCode.BadRequest;
            data = string.Empty;

            int? page = args.page;
            int? limit = args.count_per_page;
            if (page == null || limit == null)
            {
                return;
            }
            data = Statistics.GetPlayerTimesJson(page.Value, limit.Value);
            status = (int)HttpStatusCode.OK;
        }

        /// <summary>
        /// Get playerscores.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="args">
        /// {
        ///     "page": 123 (int, 1-based)
        ///     "count_per_page": 20 (int)
        /// }
        /// </param>
        /// <param name="status"></param>
        /// <param name="data"></param>
        static private void HandleCommandPlayerscores(IDbConnection connection, dynamic args, out int status,
            out string data)
        {
            status = (int)HttpStatusCode.BadRequest;
            data = string.Empty;

            int? page = args.page;
            int? limit = args.count_per_page;
            if (page == null || limit == null)
            {
                return;
            }
            data = Statistics.GetPlayerScoresJson(page.Value, limit.Value);
            status = (int)HttpStatusCode.OK;
        }

        /// <summary>
        /// Get playermaps.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="args">
        /// {
        ///     "page": 123 (int, 1-based)
        ///     "count_per_page": 20 (int)
        /// }
        /// </param>
        /// <param name="status"></param>
        /// <param name="data"></param>
        static private void HandleCommandPlayermaps(IDbConnection connection, dynamic args, out int status,
            out string data)
        {
            status = (int)HttpStatusCode.BadRequest;
            data = string.Empty;

            int? page = args.page;
            int? limit = args.count_per_page;
            if (page == null || limit == null)
            {
                return;
            }
            data = Statistics.GetPlayerMapsJson(page.Value, limit.Value);
            status = (int)HttpStatusCode.OK;
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

        /// <summary>
        /// Saves a replay to the filesystem.
        /// </summary>
        /// <param name="username"></param>
        /// <param name="mapname"></param>
        /// <param name="serverId"></param>
        /// <param name="timeMs"></param>
        /// <param name="pmoveTimeMs"></param>
        /// <param name="date"></param>
        /// <param name="data"></param>
        /// <returns>True if replay saved successfully.</returns>
        bool SaveReplayToFile(string username, string mapname, long serverId, int timeMs, int pmoveTimeMs,
            long date, byte[] data)
        {
            long? userId = Statistics.GetUserIdFromUserName(username);
            long? mapId = Statistics.GetMapIdFromMapName(mapname);
            string serverShortName = Statistics.GetShortServerNameFromId(serverId);
            if (userId == null || mapId == null || string.IsNullOrEmpty(serverShortName))
            {
                return false;
            }
            string mapDir = Path.Combine(ReplayDirectory, mapId.Value.ToString());
            Directory.CreateDirectory(mapDir);
            string replayFilename = userId.Value.ToString() + ReplayExtension;
            string replayPath = Path.Combine(mapDir, replayFilename);
            string backupPath = replayPath + "_backup";
            if (File.Exists(replayPath))
            {
                File.Copy(replayPath, backupPath);
            }
            using (BinaryWriter outfile = new BinaryWriter(File.Open(replayPath, FileMode.Create)))
            {
                outfile.Write($"server\t{serverShortName}\n");
                outfile.Write($"mapname\t{mapname}\n");
                outfile.Write($"username\t{username}\n");
                outfile.Write($"date\t{GetDateStrFromUnixTimestamp(date)}\n");
                outfile.Write($"time_ms\t{timeMs}\n");
                outfile.Write($"pmove_time_ms\t{pmoveTimeMs}\n");
                outfile.Write(data);
            }
            if (File.Exists(backupPath))
            {
                File.Delete(backupPath);
            }
            return true;
        }

        /// <summary>
        /// Gets a readable datetime string from a Unix timestamp (seconds).
        /// </summary>
        /// <param name="unixTimestampSeconds"></param>
        /// <returns>Formatted date string</returns>
        private static string GetDateStrFromUnixTimestamp(long unixTimestampSeconds)
        {
            DateTime dateTime = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc)
                + TimeSpan.FromSeconds(unixTimestampSeconds);
            string dateStr = dateTime.ToString(DateTimeFormat);
            return dateStr;
        }

        // Cache the server LoginToken -> ServerId
        static private Dictionary<string, long> _serverLogins = new Dictionary<string, long>();

        // Last time the statistics were updated
        static private DateTime _lastStatisticsRefresh = DateTime.MinValue;
    }
}
