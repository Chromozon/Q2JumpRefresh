using Microsoft.Data.Sqlite;
using Newtonsoft.Json;
using System;
using System.Collections;
using System.Collections.Generic;
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
                    if (!_servers.TryGetValue(loginToken, out serverId))
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
                            HandleCommandAddTime(serverId, commandArgs, out responseStatus, out responseData);
                            break;
                        case "gettimes":
                            HandleCommandGetTimes(serverId, commandArgs, out responseStatus, out responseData);
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
        /// <param name="serverId"></param>
        /// <param name="args">
        /// {
        ///     "user": "username" (string)
        ///     "date": 1610596223836 (int, Unix time ms)
        ///     "time_ms": 234934 (int, ms)
        /// }
        /// </param>
        /// <param name="status"></param>
        /// <param name="data"></param>
        static private void HandleCommandAddTime(int serverId, dynamic args, out int status, out string data)
        {
            status = (int)HttpStatusCode.BadRequest;
            data = string.Empty;

            string user = args.user;
            long? date = args.date;
            long? time_ms = args.time_ms;
            if (user == null)
            {
                Console.WriteLine("Invalid addtime command: no user");
                return;
            }
            if (date == null)
            {
                Console.WriteLine("Invalid addtime command: no date");
                return;
            }
            if (time_ms == null)
            {
                Console.WriteLine("Invalid addtime command: no time_ms");
                return;
            }
            // TODO: update database, when adding, check if time is faster
        }

        /// <summary>
        /// Retrieves a set of times for the given map.
        /// </summary>
        /// <param name="serverId"></param>
        /// <param name="args">
        /// {
        ///     "mapname": "mapname" (string)
        ///     "page": 1 (int, 1-based)
        /// }
        /// </param>
        /// <param name="status"></param>
        /// <param name="data"></param>
        static private void HandleCommandGetTimes(int serverId, dynamic args, out int status, out string data)
        {
            status = (int)HttpStatusCode.BadRequest;
            data = string.Empty;

            string mapname = args.mapname;
            int? page = args.page;
            if (mapname == null || page == null)
            {
                return;
            }
            if (page.Value < 1)
            {
                page = 1;
            }
            List<TimeRecord> times = new List<TimeRecord>();
            // query database for maptimes at page n
        }

        // Cache the server login tokens -> serverid
        static private Dictionary<string, int> _servers = new Dictionary<string, int>()
        {
            { "UyzyfBHakVoJM2sy", 0 }, // German
            { "N5WgTrdyd6TDmQby", 1 }, // German 2
            { "DdBsAfhmpwEKHgnO", 2 }, // US
        };

        // Cache all of the usernames -> userid
        static private Dictionary<string, int> _users = new Dictionary<string, int>();

        // Cache of all mapnames -> mapid
        static private Dictionary<string, int> _maps = new Dictionary<string, int>();
    }
}
