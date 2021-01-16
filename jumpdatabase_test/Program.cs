using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Text;
using Newtonsoft.Json;

namespace jumpdatabase_test
{
    class Program
    {
        public class UserLoginCommand
        {
            public string login_token { get; set; }
            public string command => "userlogin";

            public class UserLoginCommandArgs
            {
                public string username { get; set; }
                public string password { get; set; }
            }
            public UserLoginCommandArgs command_args;
        }

        public class AddTimeCommand
        {
            public string login_token { get; set; }
            public string command => "addtime";
            public class AddTimeCommandArgs
            {
                public string mapname { get; set; }
                public string username { get; set; }
                public long date { get; set; }
                public int time_ms { get; set; }
                public int pmove_time_ms { get; set; }
            }
            public AddTimeCommandArgs command_args;
        }

        public class AddUserPrivateCommand
        {
            public string login_token { get; set; }
            public string command => "adduserprivate";

            public class AddUserPrivateCommandArgs
            {
                public int userid { get; set; }
                public string username { get; set; }
                public string password { get; set; }
            }
            public AddUserPrivateCommandArgs command_args;
        }

        public class AddMapCommand
        {
            public string login_token { get; set; }
            public string command => "addmap";

            public class AddMapCommandArgs
            {
                public string mapname { get; set; }
            }
            public AddMapCommandArgs command_args;
        }

        public class TimeRecord
        {
            public string mapname { get; set; }
            public string username { get; set; }
            public int time_ms { get; set; }
            public int pmove_time_ms { get; set; }
            public long date { get; set; }
        }

        const string ServiceUrl = "http://localhost:57540/";

        static void Main(string[] args)
        {
            Console.WriteLine("Starting tests...");
            HttpClient client = new HttpClient();

            //const string DateTimeFormat = "yyyy-MM-dd HH:mm:ss";
            //string testStr = "27/02/12";
            //DateTime date = DateTime.ParseExact(testStr, "dd/MM/yy", CultureInfo.InvariantCulture);
            //DateTime date2 = DateTime.SpecifyKind(date, DateTimeKind.Utc);
            //long unixms = ((DateTimeOffset)date2).ToUnixTimeSeconds();
            //DateTime dateTime = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc) + TimeSpan.FromSeconds(unixms);
            //string dateStr = dateTime.ToString(DateTimeFormat);


            var users = LoadUsersFromOldServerFiles();
            //int count = 0;
            //foreach (var user in users)
            //{
            //    AddUserPrivateCommand addUserPrivateCommand = new AddUserPrivateCommand();
            //    addUserPrivateCommand.command_args = new AddUserPrivateCommand.AddUserPrivateCommandArgs();
            //    addUserPrivateCommand.login_token = "123456";
            //    addUserPrivateCommand.command_args.userid = user.userid;
            //    addUserPrivateCommand.command_args.username = user.username;
            //    addUserPrivateCommand.command_args.password = user.password;
            //    string jsonStr = JsonConvert.SerializeObject(addUserPrivateCommand);
            //    var data = new StringContent(jsonStr, Encoding.ASCII, "application/json");
            //    var response = client.PostAsync(ServiceUrl, data).Result;
            //    if (!response.IsSuccessStatusCode)
            //    {
            //        Console.WriteLine($"Could not write user {user.userid}, {user.username}");
            //    }
            //    count++;
            //    if (count % 100 == 0)
            //    {
            //        Console.WriteLine($"Wrote {count} users");
            //    }
            //}

            var maps = LoadMaplistFromOldServerFiles();
            //int count = 0;
            //foreach (var map in maps)
            //{
            //    AddMapCommand addMapCommand = new AddMapCommand();
            //    addMapCommand.command_args = new AddMapCommand.AddMapCommandArgs();
            //    addMapCommand.login_token = "123456";
            //    addMapCommand.command_args.mapname = map;
            //    string jsonStr = JsonConvert.SerializeObject(addMapCommand);
            //    var data = new StringContent(jsonStr, Encoding.ASCII, "application/json");
            //    var response = client.PostAsync(ServiceUrl, data).Result;
            //    if (!response.IsSuccessStatusCode)
            //    {
            //        Console.WriteLine($"Could not write map {map}");
            //    }
            //    count++;
            //    if (count % 100 == 0)
            //    {
            //        Console.WriteLine($"Wrote {count} maps");
            //    }
            //}

            var maptimes = LoadMaptimesFromOldServerFiles(maps, users);
            int count = 0;
            foreach (var maptime in maptimes)
            {
                try
                {
                    AddTimeCommand addTimeCommand = new AddTimeCommand();
                    addTimeCommand.command_args = new AddTimeCommand.AddTimeCommandArgs();
                    addTimeCommand.login_token = "123456";
                    addTimeCommand.command_args.username = maptime.username;
                    addTimeCommand.command_args.mapname = maptime.mapname;
                    addTimeCommand.command_args.date = maptime.date;
                    addTimeCommand.command_args.time_ms = maptime.time_ms;
                    addTimeCommand.command_args.pmove_time_ms = maptime.pmove_time_ms;
                    string jsonStr = JsonConvert.SerializeObject(addTimeCommand);
                    var data = new StringContent(jsonStr, Encoding.ASCII, "application/json");
                    var response = client.PostAsync(ServiceUrl, data).Result;
                    count++;
                    if (count % 100 == 0)
                    {
                        Console.WriteLine($"Wrote maptimes {count}/{maptimes.Count}");
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                }
            }

            //try
            //{
            //    UserLoginCommand userLoginCommand = new UserLoginCommand();
            //    userLoginCommand.command_args = new UserLoginCommand.UserLoginCommandArgs();
            //    userLoginCommand.login_token = "123456";
            //    userLoginCommand.command_args.username = "Slip";
            //    userLoginCommand.command_args.password = "123456";
            //    string jsonStr = JsonConvert.SerializeObject(userLoginCommand);
            //    var data = new StringContent(jsonStr, Encoding.ASCII, "application/json");
            //    var response = client.PostAsync(ServiceUrl, data).Result;
            //}
            //catch (Exception e)
            //{
            //    Console.WriteLine(e);
            //}

            //try
            //{
            //    AddTimeCommand addTimeCommand = new AddTimeCommand();
            //    addTimeCommand.command_args = new AddTimeCommand.AddTimeCommandArgs();
            //    addTimeCommand.login_token = "123456";
            //    addTimeCommand.command_args.username = "Slip2";
            //    addTimeCommand.command_args.mapname = "ddrace";
            //    addTimeCommand.command_args.date = 1610767963;
            //    addTimeCommand.command_args.time_ms = 8000;
            //    addTimeCommand.command_args.pmove_time_ms = -1;
            //    string jsonStr = JsonConvert.SerializeObject(addTimeCommand);
            //    var data = new StringContent(jsonStr, Encoding.ASCII, "application/json");
            //    var response = client.PostAsync(ServiceUrl, data).Result;
            //}
            //catch (Exception e)
            //{
            //    Console.WriteLine(e);
            //}
        }

        public class User
        {
            public int userid { get; set; }
            public string username { get; set; }
            public string password { get; set; }
        }

        private static List<User> LoadUsersFromOldServerFiles()
        {
            List<User> users = new List<User>();

            string userfile = "E:/Quake2Dev/jumprefresh/27910/old/users.t";
            string line = File.ReadAllText(userfile);
            var tokens = line.Split(' ', StringSplitOptions.RemoveEmptyEntries);
            for (int i = 0; i < tokens.Length; i += 4)
            {
                User user = new User();
                user.userid = Int32.Parse(tokens[i]);
                user.username = tokens[i + 3];
                user.password = RandomString(8);
                users.Add(user);
            }
            return users;
        }

        private static List<string> LoadMaplistFromOldServerFiles()
        {
            List<string> mapnames = new List<string>();

            string maplist = "E:/Quake2Dev/jumprefresh/27910/old/maplist.ini";
            string text = File.ReadAllText(maplist);
            var lines = text.Split('\n', StringSplitOptions.RemoveEmptyEntries);
            for (int i = 0; i < lines.Length; ++i)
            {
                if (lines[i].StartsWith('[') || lines[i].StartsWith('#') || string.IsNullOrEmpty(lines[i]))
                {
                    continue;
                }
                mapnames.Add(lines[i].Trim());
            }
            return mapnames;
        }

        private static List<TimeRecord> LoadMaptimesFromOldServerFiles(List<string> mapnames, List<User> users)
        {
            List<TimeRecord> maptimes = new List<TimeRecord>();
            string timedir = "E:/Quake2Dev/jumprefresh/27910/old/";
            int count = 0;
            foreach (var file in Directory.EnumerateFiles(timedir))
            {
                if (Path.GetExtension(file) != ".t")
                {
                    continue;
                }
                string mapname = Path.GetFileNameWithoutExtension(file);
                if (!mapnames.Contains(mapname))
                {
                    Console.WriteLine($"Map not in maplist {mapname}");
                    continue;
                }
                foreach (string line in File.ReadLines(file))
                {
                    var tokens = line.Split(' ', StringSplitOptions.RemoveEmptyEntries);
                    if (tokens.Length != 4)
                    {
                        Console.WriteLine($"Map not in correct format {mapname}");
                        continue;
                    }
                    DateTime dateTime = DateTime.ParseExact(tokens[0], "dd/MM/yy", CultureInfo.InvariantCulture);
                    dateTime = DateTime.SpecifyKind(dateTime, DateTimeKind.Utc);
                    int time_ms = (int)(float.Parse(tokens[1]) * 1000);
                    int pmove_time_ms = -1;
                    int userid = int.Parse(tokens[2]);
                    string username = users.FirstOrDefault(x => x.userid == userid)?.username;
                    if (string.IsNullOrEmpty(username))
                    {
                        Console.WriteLine($"Unknown user {userid}");
                        continue;
                    }

                    TimeRecord timeRecord = new TimeRecord();
                    timeRecord.mapname = mapname;
                    timeRecord.date = ((DateTimeOffset)dateTime).ToUnixTimeSeconds();
                    timeRecord.time_ms = time_ms;
                    timeRecord.pmove_time_ms = pmove_time_ms;
                    timeRecord.username = username;
                    maptimes.Add(timeRecord);
                }
                count++;
                if (count % 100 == 0)
                {
                    Console.WriteLine($"Loaded {count} maps");
                }
            }
            return maptimes;
        }

        private static Random random = new Random();
        public static string RandomString(int length)
        {
            const string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
            return new string(Enumerable.Repeat(chars, length)
              .Select(s => s[random.Next(s.Length)]).ToArray());
        }
    }
}
