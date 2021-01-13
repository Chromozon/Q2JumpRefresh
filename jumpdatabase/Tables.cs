using Microsoft.Data.Sqlite;
using System;
using System.Collections.Generic;
using System.Data;
using System.Text;

namespace jumpdatabase
{
    internal class Tables
    {
        // https://www.sqlitetutorial.net/sqlite-create-table/

        static void CreateAllTables(IDbConnection connection)
        {
            CreateTableUsers(connection);
            CreateTableServers(connection);
            CreateTableMaps(connection);
            CreateTableMapTimes(connection);
        }

        static void CreateTableUsers(IDbConnection connection)
        {
            var command = connection.CreateCommand();
            command.CommandText = @"
                CREATE TABLE Users
                (
                    UserId INTEGER NOT NULL,
	                UserName TEXT NOT NULL,
	                Password TEXT NOT NULL,
	                PRIMARY KEY (UserId AUTOINCREMENT)
                )
            ";
            command.ExecuteNonQuery();
        }

        static void CreateTableMaps(IDbConnection connection)
        {
            var command = connection.CreateCommand();
            command.CommandText = @"
                CREATE TABLE Maps
                (
                    MapId INTEGER NOT NULL,
	                MapName TEXT NOT NULL,
                    DateAdded TEXT NOT NULL,
	                PRIMARY KEY (MapId AUTOINCREMENT)
                )
            ";
            command.ExecuteNonQuery();
        }

        static void CreateTableServers(IDbConnection connection)
        {
            var command = connection.CreateCommand();
            command.CommandText = @"
                CREATE TABLE Servers
                (
                    ServerId INTEGER NOT NULL,
	                ServerName TEXT NOT NULL,
                    LoginToken TEXT NOT NULL,
                    DateAdded TEXT NOT NULL,
	                PRIMARY KEY (ServerId AUTOINCREMENT)
                )
            ";
            command.ExecuteNonQuery();
        }

        static void CreateTableMapTimes(IDbConnection connection)
        {
            var command = connection.CreateCommand();
            command.CommandText = @"
                CREATE TABLE MapTimes
                (
                    MapId INTEGER NOT NULL,
	                UserId INTEGER NOT NULL,
                    ServerId INTEGER NOT NULL,
                    TimeMs INTEGER NOT NULL,
                    Date TEXT NOT NULL,
                    PRIMARY KEY (MapId, UserId)
                )
            ";
            command.ExecuteNonQuery();
        }
    }
}
