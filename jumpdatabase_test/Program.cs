using System;
using System.Collections.Generic;
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

        const string ServiceUrl = "http://localhost:57540/";

        static void Main(string[] args)
        {
            Console.WriteLine("Starting tests...");
            HttpClient client = new HttpClient();

            //var users = LoadUsersFromOldServerFiles();
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

            try
            {
                UserLoginCommand userLoginCommand = new UserLoginCommand();
                userLoginCommand.command_args = new UserLoginCommand.UserLoginCommandArgs();
                userLoginCommand.login_token = "123456";
                userLoginCommand.command_args.username = "Slip";
                userLoginCommand.command_args.password = "123456";
                string jsonStr = JsonConvert.SerializeObject(userLoginCommand);
                var data = new StringContent(jsonStr, Encoding.ASCII, "application/json");
                var response = client.PostAsync(ServiceUrl, data).Result;
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }

            try
            {
                AddTimeCommand addTimeCommand = new AddTimeCommand();
                addTimeCommand.command_args = new AddTimeCommand.AddTimeCommandArgs();
                addTimeCommand.login_token = "123456";
                addTimeCommand.command_args.username = "Slip2";
                addTimeCommand.command_args.mapname = "ddrace";
                addTimeCommand.command_args.date = 1610767963;
                addTimeCommand.command_args.time_ms = 8000;
                addTimeCommand.command_args.pmove_time_ms = -1;
                string jsonStr = JsonConvert.SerializeObject(addTimeCommand);
                var data = new StringContent(jsonStr, Encoding.ASCII, "application/json");
                var response = client.PostAsync(ServiceUrl, data).Result;
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
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

        private static Random random = new Random();
        public static string RandomString(int length)
        {
            const string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
            return new string(Enumerable.Repeat(chars, length)
              .Select(s => s[random.Next(s.Length)]).ToArray());
        }
    }
}
