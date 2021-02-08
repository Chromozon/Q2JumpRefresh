using log4net;
using Microsoft.Data.Sqlite;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Data;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;

namespace jumpdatabase
{
    internal class Migration
    {
        private static readonly ILog Log = LogManager.GetLogger("Migration");

        private const string DateTimeFormat = "yyyy-MM-dd HH:mm:ss"; // format supported by sqlite db

        private const string MapDirectory = "./maps/";
        private const string EntExtension = ".ent";
        private const string MSetExtension = ".mset";

        /// <summary>
        /// Performs all migration steps.  Completely replaces the database data with the old data, does not merge.
        /// </summary>
        /// <param name="connection"></param>
        public static void MigrateAll(IDbConnection connection)
        {
            DateTime start = DateTime.Now;
            Log.Info("Starting migration...");
            string oldMaplistFile = @"G:\Dropbox\Quake2\German_q2jump\german_times_dec_2020\27910\maplist.ini";
            string oldMapDir = @"E:\Quake2\jump\maps\";
            string oldEntDir = @"G:\Dropbox\Quake2\German_q2jump\german_map_ents\mapsent\";
            string oldMSetDir = @"G:\Dropbox\Quake2\German_q2jump\german_msets\ent";
            string oldUserFile = @"G:\Dropbox\Quake2\German_q2jump\german_times_dec_2020\27910\users.t";
            string oldMaptimesDir = @"G:\Dropbox\Quake2\German_q2jump\german_times_dec_2020\27910";
            string oldReplayDir = @"E:\Quake2\jump\jumpdemo\";
            int serverId = 1; // German

            // Wipe the database
            ClearDatabaseMapsUsersMapTimes(connection);

            // First, we need to load the maplist.
            LoadMaplistIntoDatabase(connection, oldMaplistFile);

            // Load the bsp, ent, and mset data.
            LoadMapsIntoDatabase(connection, oldMapDir);
            LoadEntsIntoDatabase(connection, oldEntDir);
            LoadMSetsIntoDatabase(connection, oldMSetDir);

            // Load the users.
            LoadUsersIntoDatabase(connection, oldUserFile);

            // Load the maptimes.
            LoadMaptimesIntoDatabase(connection, oldMaptimesDir, serverId);

            // Load the replays.
            ConvertReplays(connection, oldReplayDir);

            DateTime end = DateTime.Now;
            TimeSpan elapsed = end - start;
            Log.Info($"Migration finished! Elapsed time: {elapsed.Minutes} min, {elapsed.Seconds} s");
        }

        /// <summary>
        /// Migrates the old ent files to the new location.  Updates the database with ent file data.
        /// Ent files contain *all* of the entities for a given map.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="oldEntDir">Directory that contains all of the old ent files (.ent)</param>
        private static void LoadEntsIntoDatabase(IDbConnection connection, string oldEntDir)
        {
            HashSet<string> mapnames = new HashSet<string>();
            var command = connection.CreateCommand();
            command.CommandText = $@"
                SELECT MapName FROM Maps
            ";
            var reader = command.ExecuteReader();
            while (reader.Read())
            {
                string mapname = (string)reader[0];
                mapnames.Add(mapname);
            }
            Log.Info($"Loaded maplist from database ({mapnames.Count} maps)");

            if (!Directory.Exists(oldEntDir))
            {
                Log.Error($"Provided ent directory does not exist: {oldEntDir}");
                return;
            }

            Directory.CreateDirectory(MapDirectory);
            string dateUpdated = DateTime.SpecifyKind(DateTime.UtcNow, DateTimeKind.Utc).ToString(DateTimeFormat);

            int count = 0;
            foreach (string mapname in mapnames)
            {
                string oldEntFile = Path.Combine(oldEntDir, $"{mapname}.ent");
                if (!File.Exists(oldEntFile))
                {
                    Log.Warn($"Ent file not found: {oldEntFile}");
                    continue;
                }
                string newEntFile = Path.Combine(MapDirectory, $"{mapname}{EntExtension}");
                File.Copy(oldEntFile, newEntFile, overwrite: true);

                string hash = CalculateMD5(newEntFile);

                command = connection.CreateCommand();
                command.CommandText = $@"
                    UPDATE Maps
                    SET EntHashMD5 = '{hash}', EntDateUpdated = '{dateUpdated}', EntUpdatedBy = 'Migration'
                    WHERE MapName = '{mapname}'
                ";
                int rows = command.ExecuteNonQuery();
                if (rows != 1)
                {
                    Log.Error($"Could not update ent data for map: {mapname}");
                    continue;
                }
                count++;
            }
            Log.Info($"Loaded the ent data for {count} maps");
        }

        /// <summary>
        /// Migrates the old mset files to the new location.  Updates the database with mset file data.
        /// MSets are map specific variables, such as overridden gravity, number of checkpoints, etc.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="oldMSetDir">Directory that contains all of the old mset files (.cfg)</param>
        private static void LoadMSetsIntoDatabase(IDbConnection connection, string oldMSetDir)
        {
            HashSet<string> mapnames = new HashSet<string>();
            var command = connection.CreateCommand();
            command.CommandText = $@"
                SELECT MapName FROM Maps
            ";
            var reader = command.ExecuteReader();
            while (reader.Read())
            {
                string mapname = (string)reader[0];
                mapnames.Add(mapname);
            }
            Log.Info($"Loaded maplist from database ({mapnames.Count} maps)");

            if (!Directory.Exists(oldMSetDir))
            {
                Log.Error($"Provided mset directory does not exist: {oldMSetDir}");
                return;
            }

            Directory.CreateDirectory(MapDirectory);
            string dateUpdated = DateTime.SpecifyKind(DateTime.UtcNow, DateTimeKind.Utc).ToString(DateTimeFormat);

            int count = 0;
            foreach (string mapname in mapnames)
            {
                string oldMSetFile = Path.Combine(oldMSetDir, $"{mapname}.cfg");
                if (!File.Exists(oldMSetFile))
                {
                    Log.Warn($"MSet file not found: {oldMSetFile}");
                    continue;
                }
                string newMSetFile = Path.Combine(MapDirectory, $"{mapname}{MSetExtension}");
                File.Copy(oldMSetFile, newMSetFile, overwrite: true);

                string hash = CalculateMD5(newMSetFile);

                command = connection.CreateCommand();
                command.CommandText = $@"
                    UPDATE Maps
                    SET MSetHashMD5 = '{hash}', MSetDateUpdated = '{dateUpdated}', MSetUpdatedBy = 'Migration'
                    WHERE MapName = '{mapname}'
                ";
                int rows = command.ExecuteNonQuery();
                if (rows != 1)
                {
                    Log.Error($"Could not update mset data for map: {mapname}");
                    continue;
                }
                count++;
            }
            Log.Info($"Loaded the mset data for {count} maps");
        }

        /// <summary>
        /// Migrates the map files to the new location.  Updates the database with map file data.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="oldMapDir">Directory that contains all of the old map files (.bsp)</param>
        private static void LoadMapsIntoDatabase(IDbConnection connection, string oldMapDir)
        {
            HashSet<string> mapnames = new HashSet<string>();
            var command = connection.CreateCommand();
            command.CommandText = $@"
                SELECT MapName FROM Maps
            ";
            var reader = command.ExecuteReader();
            while (reader.Read())
            {
                string mapname = (string)reader[0];
                mapnames.Add(mapname);
            }
            Log.Info($"Loaded maplist from database ({mapnames.Count} maps)");

            if (!Directory.Exists(oldMapDir))
            {
                Log.Error($"Provided map directory does not exist: {oldMapDir}");
                return;
            }

            Directory.CreateDirectory(MapDirectory);
            string dateUpdated = DateTime.SpecifyKind(DateTime.UtcNow, DateTimeKind.Utc).ToString(DateTimeFormat);

            int count = 0;
            foreach (string mapname in mapnames)
            {
                string oldMapFile = Path.Combine(oldMapDir, $"{mapname}.bsp");
                if (!File.Exists(oldMapFile))
                {
                    Log.Warn($"Map file not found: {oldMapFile}");
                    continue;
                }
                string newMapFile = Path.Combine(MapDirectory, $"{mapname}.bsp");
                File.Copy(oldMapFile, newMapFile, overwrite: true);

                string hash = CalculateMD5(newMapFile);

                command = connection.CreateCommand();
                command.CommandText = $@"
                    UPDATE Maps
                    SET HashMD5 = '{hash}'
                    WHERE MapName = '{mapname}'
                ";
                int rows = command.ExecuteNonQuery();
                if (rows != 1)
                {
                    Log.Error($"Could not update map data for map: {mapname}");
                    continue;
                }
                count++;
            }
            Log.Info($"Loaded the map data for {count} maps");
        }

        /// <summary>
        /// Reads the old maplist.ini file and adds that list of maps into the database.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="oldMaplistFile">maplist.ini</param>
        private static void LoadMaplistIntoDatabase(IDbConnection connection, string oldMaplistFile)
        {
            List<string> mapnames = new List<string>();

            string text = File.ReadAllText(oldMaplistFile);
            var lines = text.Split('\n', StringSplitOptions.RemoveEmptyEntries);
            for (int i = 0; i < lines.Length; ++i)
            {
                if (lines[i].StartsWith('[') || lines[i].StartsWith('#') || string.IsNullOrEmpty(lines[i]))
                {
                    continue;
                }
                mapnames.Add(lines[i].Trim());
            }
            Log.Info($"Maplist {oldMaplistFile} contains {mapnames.Count} maps");

            string dateAdded = DateTime.SpecifyKind(DateTime.UtcNow, DateTimeKind.Utc).ToString(DateTimeFormat);

            int count = 0;
            foreach (string mapname in mapnames)
            {
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
                if (rows != 1)
                {
                    Log.Warn($"Error adding map: {mapname}");
                    continue;
                }
                count++;
            }
            Log.Info($"Added {count} maps to database");
        }

        private class OldUser
        {
            public int userid { get; set; }
            public string username { get; set; }
        }

        /// <summary>
        /// Reads the list of users from users.t and stores in the database.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="oldUserFile">users.t</param>
        private static void LoadUsersIntoDatabase(IDbConnection connection, string oldUserFile)
        {
            List<OldUser> users = new List<OldUser>();
            string line = File.ReadAllText(oldUserFile);
            var tokens = line.Split(' ', StringSplitOptions.RemoveEmptyEntries);
            for (int i = 0; i < tokens.Length; i += 4)
            {
                OldUser user = new OldUser();
                user.userid = Int32.Parse(tokens[i]);
                user.username = tokens[i + 3];
                users.Add(user);
            }
            Log.Info($"Loaded {users.Count} users from file: {oldUserFile}");

            int count = 0;
            foreach (OldUser user in users)
            {
                string password = RandomString(length: 8);
                var command = connection.CreateCommand();
                command.CommandText = $@"
                    INSERT OR IGNORE INTO Users (UserId, UserName, Password)
                    VALUES ({user.userid}, @username, @password)
                ";
                SqliteParameter paramUserName = new SqliteParameter("@username", SqliteType.Text);
                paramUserName.Value = user.username;
                SqliteParameter paramPassword = new SqliteParameter("@password", SqliteType.Text);
                paramPassword.Value = password;
                command.Parameters.Add(paramUserName);
                command.Parameters.Add(paramPassword);
                int rows = command.ExecuteNonQuery();
                if (rows != 1)
                {
                    Log.Warn($"Could not create user {user.username}");
                    continue;
                }
                count++;
            }
            Log.Info($"Added {count} users to the database");
        }

        /// <summary>
        /// Loads all of the old maptimes into the database.
        /// **Note: Assumes the user ids from the old maptimes are the same as the ones in the database.
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="oldMaptimesDir">Directory that contains the [mapname].t files</param>
        /// <param name="serverId"></param>
        private static void LoadMaptimesIntoDatabase(IDbConnection connection, string oldMaptimesDir, int serverId)
        {
            Log.Info($"Loading maptimes from {oldMaptimesDir}");
            int count = 0;
            foreach (var file in Directory.EnumerateFiles(oldMaptimesDir))
            {
                if (Path.GetExtension(file) != ".t" || Path.GetFileName(file) == "users.t")
                {
                    continue;
                }
                string mapname = Path.GetFileNameWithoutExtension(file);
                foreach (string line in File.ReadLines(file))
                {
                    var tokens = line.Split(' ', StringSplitOptions.RemoveEmptyEntries);
                    if (tokens.Length != 4)
                    {
                        Log.Warn($"Map not in correct format: {mapname}");
                        break;
                    }
                    DateTime dateTime = DateTime.ParseExact(tokens[0], "dd/MM/yy", CultureInfo.InvariantCulture);
                    dateTime = DateTime.SpecifyKind(dateTime, DateTimeKind.Utc);
                    string date = dateTime.ToString(DateTimeFormat);
                    int timeMs = (int)(float.Parse(tokens[1]) * 1000);
                    int pmoveTimeMs = -1;
                    int userid = int.Parse(tokens[2]);

                    var command = connection.CreateCommand();
                    command.CommandText = $@"
                        INSERT OR IGNORE INTO MapTimes (MapId, UserId, ServerId, TimeMs, PMoveTimeMs, Date)
                        VALUES (
                            (SELECT MapId FROM Maps WHERE MapName = @mapname),
                            {userid},
                            {serverId},
                            {timeMs},
                            {pmoveTimeMs},
                            '{date}'
                        )
                    ";
                    SqliteParameter paramMapName = new SqliteParameter("@mapname", SqliteType.Text);
                    paramMapName.Value = mapname;
                    command.Parameters.Add(paramMapName);
                    command.Prepare();
                    int rows = command.ExecuteNonQuery();
                    if (rows != 1)
                    {
                        Log.Warn($"Could not write user {userid} time for map {mapname}");
                        continue;
                    }
                }
                count++;
                if (count % 100 == 0)
                {
                    // Note this counts all maps that we have attempted to load, not just the successful ones
                    Log.Info($"Loaded maptimes from {count} maps");
                }
            }
            Log.Info($"Finished loading maptimes from {count} maps");
        }

        /// <summary>
        /// Deletes all records from the Maps, Users, and MapTimes database tables.
        /// </summary>
        /// <param name="connection"></param>
        private static void ClearDatabaseMapsUsersMapTimes(IDbConnection connection)
        {
            List<string> tableNames = new List<string>()
            {
                "Users",
                "Maps",
                "MapTimes"
            };
            foreach (var tableName in tableNames)
            {
                var command = connection.CreateCommand();
                command.CommandText = $@"
                    DELETE FROM {tableName}
                ";
                int rows = command.ExecuteNonQuery();
                if (rows < 0)
                {
                    Log.Warn($"Could not clear table {tableName}");
                }
                else
                {
                    Log.Info($"Cleared all records from table {tableName}");
                }
            }
        }

        /// {
        ///     "mapname": "mapname" (string, no file extension)
        ///     "username": "username" (string)
        ///     "date": 1610596223836 (int, Unix time s)
        ///     "time_ms": 234934 (int, ms)
        ///     "pmove_time_ms": 223320 (int, ms, -1 means no time)
        ///     "replay_data": "dGhpcyBpcyBhIHRlc3Q=" (string, base64 encoded)
        /// }
        private class NewReplayData
        {
            public string mapname { get; set; }
            public string username { get; set; }
            public long date { get; set; }
            public long time_ms { get; set; }
            public long pmove_time_ms { get; set; }
            public string replay_data { get; set; }
        }

        private static void ConvertReplays(IDbConnection connection, string oldReplayDir)
        {
            // Old replay [mapname]_[userid].dj3 file is array of:
            // {
            //     float32 angle[3];
            //     float32 origin[3];
            //     int32 frame;
            // }
            // This structure is packed with size 28 bytes.
            //
            // The 4 bytes of frame are a combo of fps and bitmask key states:
            // frame[0] = ignored
            // frame[1] = fps
            // frame[2] = key states
            // frame[3] = ignored
            //
            // Key states bitmask positions:
            // Up (jump) = 1
            // Down (crouch) = 2
            // Left = 4
            // Right = 8
            // Forward = 16
            // Back = 32
            // Attack = 64
            //
            // New format:
            // {
            //     vec3_t pos;         // (12 bytes) player position in the world
            //     vec3_t angles;      // (12 bytes) player view angles
            //     int32_t key_states; // (4 bytes) active inputs (jump, crouch, left, right, etc.)
            //     int32_t fps;        // (4 bytes) current fps
            //     int32_t reserved1;  // (4 bytes) reserved bytes for future use (checkpoints, weapons, etc.)
            //     int32_t reserved2;  // (4 bytes) reserved bytes for future use
            // }
            //
            // KEY_STATE_NONE = 0
            // KEY_STATE_FORWARD = 1 << 0
            // KEY_STATE_BACK = 1 << 1
            // KEY_STATE_LEFT = 1 << 2
            // KEY_STATE_RIGHT = 1 << 3
            // KEY_STATE_JUMP = 1 << 4
            // KEY_STATE_CROUCH = 1 << 5
            // KEY_STATE_ATTACK = 1 << 6

            const int OldReplayFrameSizeBytes = 28;
            const int NewReplayFrameSizeBytes = 40;

            int count = 0;
            foreach (var file in Directory.EnumerateFiles(oldReplayDir))
            {
                if (Path.GetExtension(file) != ".dj3")
                {
                    continue;
                }
                string filename = Path.GetFileName(file);
                int indexExt = filename.LastIndexOf(".dj3");
                int indexId = filename.LastIndexOf("_");
                if (indexExt == -1 || indexId == -1 || indexExt <= indexId)
                {
                    Log.Warn($"Invalid replay filename format (bad name): {filename}");
                    continue;
                }
                string userIdStr = filename.Substring(indexId + 1, indexExt - (indexId + 1));
                bool success = int.TryParse(userIdStr, out int userId);
                if (!success)
                {
                    Log.Warn($"Invalid replay filename format (bad user id): {filename}");
                    continue;
                }
                string mapname = filename.Substring(0, indexId);
                if (string.IsNullOrEmpty(mapname))
                {
                    Log.Warn($"Invalid replay filename format (bad mapname): {filename}");
                    continue;
                }
                byte[] oldReplay = File.ReadAllBytes(file);
                if (oldReplay.Length % OldReplayFrameSizeBytes != 0)
                {
                    Log.Warn($"Invalid replay file format (bad byte count): {filename}");
                    continue;
                }
                int frameCount = oldReplay.Length / OldReplayFrameSizeBytes;
                byte[] newReplay = new byte[frameCount * NewReplayFrameSizeBytes];
                for (int i = 0; i < frameCount; i++)
                {
                    // Extract the old values
                    int oldStart = i * OldReplayFrameSizeBytes;

                    byte[] oldAngle = new byte[12];
                    Array.Copy(oldReplay, oldStart, oldAngle, 0, 12);

                    byte[] oldOrigin = new byte[12];
                    Array.Copy(oldReplay, oldStart + 12, oldOrigin, 0, 12);

                    byte oldFps = oldReplay[oldStart + 24 + 1];
                    byte oldKeys = oldReplay[oldStart + 24 + 2];

                    // Create the new values
                    byte[] newAngle = oldAngle;
                    byte[] newPos = oldOrigin;

                    Int32 newFps = oldFps;
                    Int32 newReserved1 = 0;
                    Int32 newReserved2 = 0;

                    Int32 newKeys = 0;
                    if ((oldKeys & 1) != 0) // jump
                    {
                        newKeys |= 1 << 4;
                    }
                    if ((oldKeys & 2) != 0) // crouch
                    {
                        newKeys |= 1 << 5;
                    }
                    if ((oldKeys & 4) != 0) // left
                    {
                        newKeys |= 1 << 2;
                    }
                    if ((oldKeys & 8) != 0) // right
                    {
                        newKeys |= 1 << 3;
                    }
                    if ((oldKeys & 16) != 0) // forward
                    {
                        newKeys |= 1 << 0;
                    }
                    if ((oldKeys & 32) != 0) // back
                    {
                        newKeys |= 1 << 1;
                    }
                    if ((oldKeys & 64) != 0) // attack
                    {
                        newKeys |= 1 << 6;
                    }

                    // Copy the new values to the new array
                    int newStart = i * NewReplayFrameSizeBytes;

                    Array.Copy(newPos, 0, newReplay, newStart, 12);

                    Array.Copy(newAngle, 0, newReplay, newStart + 12, 12);

                    Array.Copy(BitConverter.GetBytes(newKeys), 0, newReplay, newStart + 24, 4);

                    Array.Copy(BitConverter.GetBytes(newFps), 0, newReplay, newStart + 28, 4);

                    Array.Copy(BitConverter.GetBytes(newReserved1), 0, newReplay, newStart + 32, 4);

                    Array.Copy(BitConverter.GetBytes(newReserved2), 0, newReplay, newStart + 36, 4);
                }

                // Construct the file data
                var command = connection.CreateCommand();
                command.CommandText = $@"
                    SELECT UserName FROM Users WHERE UserId = {userId}
                ";
                var reader = command.ExecuteReader();
                string username = string.Empty;
                while (reader.Read())
                {
                    username = (string)reader[0];
                }
                if (string.IsNullOrEmpty(username))
                {
                    Log.Warn($"Invalid username for replay file: {filename}");
                    continue;
                }

                command = connection.CreateCommand();
                command.CommandText = $@"
                    SELECT MapId FROM Maps WHERE MapName = @mapname
                ";
                SqliteParameter paramMapName = new SqliteParameter("@mapname", SqliteType.Text);
                paramMapName.Value = mapname;
                command.Parameters.Add(paramMapName);
                command.Prepare();
                reader = command.ExecuteReader();
                long mapId = -1;
                while (reader.Read())
                {
                    mapId = (long)reader[0];
                }
                if (mapId == -1)
                {
                    Log.Warn($"Invalid map id for replay file: {filename}");
                    continue;
                }

                command = connection.CreateCommand();
                command.CommandText = $@"
                    SELECT TimeMs, Date FROM MapTimes
                    WHERE
                        MapId = {mapId} AND UserId = {userId}
                ";
                reader = command.ExecuteReader();
                long timeMs = -1;
                string date = string.Empty;
                while (reader.Read())
                {
                    timeMs = (long)reader[0];
                    date = (string)reader[1];
                }
                if (timeMs == -1 || string.IsNullOrEmpty(date))
                {
                    Log.Warn($"Could not load maptimes for replay file: {filename}");
                    continue;
                }
                DateTime dateTime = DateTime.ParseExact(date, DateTimeFormat, CultureInfo.InvariantCulture);
                dateTime = DateTime.SpecifyKind(dateTime, DateTimeKind.Utc);
                long dateUnixS = ((DateTimeOffset)dateTime).ToUnixTimeSeconds();
                NewReplayData newReplayData = new NewReplayData()
                {
                    username = username,
                    mapname = mapname,
                    date = dateUnixS,
                    time_ms = timeMs,
                    pmove_time_ms = -1,
                    replay_data = Convert.ToBase64String(newReplay)
                };
                string json = JsonConvert.SerializeObject(newReplayData);
                Statistics.SaveReplayToFile(mapname, userId, json);
                count++;
                if (count % 100 == 0)
                {
                    Log.Info($"Converted {count} replays");
                }
            }
            Log.Info($"Converted a total of {count} replays");
        }

        /// <summary>
        /// Computes the MD5 hash for a given file.
        /// </summary>
        /// <param name="filename">Throws exception if file not found</param>
        /// <returns>Hash value</returns>
        private static string CalculateMD5(string filename)
        {
            using (var md5 = System.Security.Cryptography.MD5.Create())
            {
                using (var stream = File.OpenRead(filename))
                {
                    var hash = md5.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToLowerInvariant();
                }
            }
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
