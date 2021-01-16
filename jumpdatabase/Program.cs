using Microsoft.Data.Sqlite;
using Newtonsoft.Json;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace jumpdatabase
{
    class Program
    {
        class TimeRecord
        {
            public string User { get; set; }
            public string Server { get; set; }
            public DateTime Date { get; set; }
            public long TimeMs { get; set; }
        }

        class AddTimeCommand
        {
            public string Map { get; set; }
            public string User { get; set; }
            public string Server { get; set; }
            public DateTime Date { get; set; }
            public long TimeMs { get; set; }
        }

        class GetTimesCommand
        {
            public string Map { get; set; }
            public int Page { get; set; }
        }

        const int ServicePort = 57540;
        const string DatabasePath = "./jumpdatabase.sqlite3";

        static void Main(string[] args)
        {
            // Initialize the database
            SqliteConnection dbConnection = new SqliteConnection($"Data Source={DatabasePath}");
            dbConnection.Open();
            Console.WriteLine($"Opened connection to database \"{DatabasePath}\"");
            Tables.CreateAllTables(dbConnection);

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
                    if (request.ContentType != "application/json")
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
                    int serverId = -1;
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
                            HandleCommandAddTime(dbConnection, serverId, commandArgs, out responseStatus, out responseData);
                            break;
                        case "gettimes":
                            HandleCommandGetTimes(dbConnection, serverId, commandArgs, out responseStatus, out responseData);
                            break;
                        case "userlogin":
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
        ///     "username": "username" (string)
        ///     "date": 1610596223836 (int, Unix time ms)
        ///     "time_ms": 234934 (int, ms)
        ///     "pmove_time_ms": 223320 (int, ms)
        /// }
        /// </param>
        /// <param name="status"></param>
        /// <param name="data"></param>
        static private void HandleCommandAddTime(IDbConnection connection, int serverId, dynamic args,
            out int status, out string data)
        {
            status = (int)HttpStatusCode.BadRequest;
            data = string.Empty;

            string mapname = args.mapname;
            string username = args.username;
            long? date = args.date;
            long? timeMs = args.time_ms;
            long? pmoveTimeMs = args.pmove_time_ms;
            if (mapname == null || username == null || date == null || timeMs == null || pmoveTimeMs == null)
            {
                return;
            }
            DateTime dateTime = new DateTime(1970, 1, 1) + TimeSpan.FromMilliseconds(date.Value);
            string dateStr = dateTime.ToString("yyyy-MM-dd HH:mm:ss");

            var command = connection.CreateCommand();
            command.CommandText = $@"
                INSERT OR IGNORE INTO MapTimes (MapId, UserId, ServerId, TimeMs, PMoveTimeMs, Date)
                VALUES (
                    (SELECT MapId FROM Maps WHERE MapName = @mapname),
                    (SELECT UsesrId FROM Users WHERE UserName = @username),
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
            command.ExecuteNonQuery();
        }

        /// <summary>
        /// Retrieves a set of times for the given map.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="serverId"></param>
        /// <param name="args">
        /// {
        ///     "mapname": "mapname" (string)
        ///     "page": 1 (int, 1-based)
        /// }
        /// </param>
        /// <param name="status"></param>
        /// <param name="data"></param>
        static private void HandleCommandGetTimes(IDbConnection connection, int serverId, dynamic args,
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
            int mapId = -1;
            if (!_maps.TryGetValue(mapName, out mapId))
            {
                return;
            }
            List<TimeRecord> times = new List<TimeRecord>();

            var command = connection.CreateCommand();
            command.CommandText = $@"
                SELECT Users.UserName, Servers.ServerName, MapTimes.TimeMs, MapTimes.Date FROM MapTimes
                INNER JOIN Users ON MapTimes.UserId = Users.UserId
                INNER JOIN Servers ON MapTimes.ServerId = Servers.ServerId
                WHERE MapId = {mapId}
                ORDER BY TimeMs
                LIMIT {ResultsPerQuery} OFFSET {offset}
            ";
            var reader = command.ExecuteReader();
            while (reader.Read())
            {
                // TODO
                // Read times into list
                // Serialize into json
            }
        }

        /// <summary>
        /// Load all map ids from the database so we don't have to do multiple database queries
        /// for each command.
        /// </summary>
        /// <param name="connection"></param>
        static private void LoadMapsCache(IDbConnection connection)
        {
            _maps.Clear();
            var command = connection.CreateCommand();
            command.CommandText = @"
                SELECT MapId, MapName FROM Maps
            ";
            var reader = command.ExecuteReader();
            while (reader.Read())
            {
                int mapId = (int)reader[0];
                string mapName = (string)reader[1];
                _maps.Add(mapName, mapId);
            }
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
                int serverId = (int)reader[0];
                string loginToken = (string)reader[1];
                _serverLogins.Add(loginToken, serverId);
            }
        }

        /// <summary>
        /// Load the list of users so we don't have to check this for each query.
        /// </summary>
        /// <param name="connection"></param>
        static private void LoadUsersCache(IDbConnection connection)
        {
            _users.Clear();
            var command = connection.CreateCommand();
            command.CommandText = @"
                SELECT UserId, UserName FROM Users
            ";
            var reader = command.ExecuteReader();
            while (reader.Read())
            {
                int userId = (int)reader[0];
                string userName = (string)reader[1];
                _users.Add(userName, userId);
            }
        }

        // Cache the server LoginToken -> ServerId
        static private Dictionary<string, int> _serverLogins = new Dictionary<string, int>();

        // Cache all of the UserName -> UserId
        static private Dictionary<string, int> _users = new Dictionary<string, int>();

        // Cache of all MapName -> MapId
        static private Dictionary<string, int> _maps = new Dictionary<string, int>();
    }
}
