using Microsoft.Data.Sqlite;
using Newtonsoft.Json;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Globalization;
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
            public string UserName { get; set; }
            public string ServerNameShort { get; set; }
            public DateTime Date { get; set; }
            public long TimeMs { get; set; }
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
        ///     "date": 1610596223836 (int, Unix time ms)
        ///     "time_ms": 234934 (int, ms)
        ///     "pmove_time_ms": 223320 (int, ms, -1 means no time)
        /// }
        /// </param>
        /// <param name="status"></param>
        /// <param name="data"></param>
        static private void HandleCommandAddTime(IDbConnection connection, int serverId, dynamic args,
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
            DateTime dateTime = new DateTime(1970, 1, 1) + TimeSpan.FromMilliseconds(date.Value);
            string dateStr = dateTime.ToString(DateTimeFormat);

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
        /// <param name="serverId"></param>
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
        /// <param name="serverId"></param>
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
        /// <param name="serverId"></param>
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

        // Cache the server LoginToken -> ServerId
        static private Dictionary<string, int> _serverLogins = new Dictionary<string, int>();
    }
}
