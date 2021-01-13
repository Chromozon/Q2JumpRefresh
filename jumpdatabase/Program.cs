using Microsoft.Data.Sqlite;
using Newtonsoft.Json;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;

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

        static void Main(string[] args)
        {
            TcpListener tcpListener = new TcpListener(IPAddress.Any, ServicePort);
            tcpListener.Start();
            Console.WriteLine($"Q2 Jump Database service listening on port {ServicePort}");

            byte[] buffer = new byte[1024];

            while (true)
            {
                try
                {
                    TcpClient client = tcpListener.AcceptTcpClient();
                    NetworkStream stream = client.GetStream();
                    do
                    {
                        int bytesRead = stream.Read(buffer, 0, buffer.Length);
                        string jsonStr = System.Text.Encoding.ASCII.GetString(buffer);
                        dynamic jsonObj = JsonConvert.DeserializeObject(jsonStr);

                        string token = jsonObj.token;
                        if (token == null)
                        {
                            Console.WriteLine("Invalid json: no token");
                            break;
                        }
                        bool validToken = _serverTokens.TryGetValue(token, out var serverName);
                        if (!validToken)
                        {
                            Console.WriteLine("Invalid token");
                            break;
                        }
                        string command = jsonObj.command;
                        if (command == null)
                        {
                            Console.WriteLine("Invalid json: no command");
                            break;
                        }
                        dynamic commandArgs = jsonObj.commandargs;
                        if (commandArgs == null)
                        {
                            Console.WriteLine("Invalid json: no command args");
                            return;
                        }
                        if (command == "addtime")
                        {
                            HandleCommandAddTime(commandArgs);
                        }
                        else if (command == "gettimes")
                        {
                            HandleCommandGetTimes(commandArgs);
                        }
                        else
                        {
                            Console.WriteLine("Invalid command");
                            break;
                        }
                    }
                    while (stream.DataAvailable);
                    stream.Close();
                    client.Close();
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Exception: {e}");
                }
            }
        }

        static private void HandleCommandAddTime(dynamic args)
        {
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

        static private List<TimeRecord> HandleCommandGetTimes(dynamic args)
        {
            List<TimeRecord> times = new List<TimeRecord>();
            string mapname = args.mapname;
            int? page = args.page;
            if (mapname == null)
            {
                Console.WriteLine("Invalid gettimes command: no mapname");
                return times;
            }
            if (page == null)
            {
                Console.WriteLine("Invalid gettimes command: no page");
                return times;
            }
            if (page.Value < 1)
            {
                page = 1;
            }
            // query database for maptimes at page n
            return times;
        }

        

        static private Dictionary<string, string> _serverTokens = new Dictionary<string, string>()
        {
            { "UyzyfBHakVoJM2sy", "German All" },
            { "N5WgTrdyd6TDmQby", "German Race" },
            { "DdBsAfhmpwEKHgnO", "US All" },
        };
    }
}
