﻿using Microsoft.Data.Sqlite;
using System;
using System.Collections.Generic;
using System.Data;
using System.Text;

namespace jumpdatabase
{
    internal class Tables
    {
        // https://www.sqlitetutorial.net/sqlite-create-table/

        static public void DeleteAllTables(IDbConnection connection)
        {
            List<string> tableNames = new List<string>()
            {
                "Users",
                "Servers",
                "Maps",
                "MapTimes"
            };
            foreach (var tableName in tableNames)
            {
                var command = connection.CreateCommand();
                command.CommandText = $@"
                    DROP TABLE IF EXISTS {tableName}
                ";
                command.ExecuteNonQuery();
            }
        }

        static public void CreateAllTables(IDbConnection connection)
        {
            CreateTableUsers(connection);
            CreateTableServers(connection);
            CreateTableMaps(connection);
            CreateTableMapTimes(connection);
        }

        static private void CreateTableUsers(IDbConnection connection)
        {
            var command = connection.CreateCommand();
            command.CommandText = @"
                CREATE TABLE IF NOT EXISTS Users
                (
                    UserId INTEGER NOT NULL,
	                UserName TEXT NOT NULL UNIQUE,
	                Password TEXT NOT NULL,
	                PRIMARY KEY (UserId AUTOINCREMENT)
                )
            ";
            command.ExecuteNonQuery();
        }

        static private void CreateTableMaps(IDbConnection connection)
        {
            var command = connection.CreateCommand();
            command.CommandText = @"
                CREATE TABLE IF NOT EXISTS Maps
                (
                    MapId INTEGER NOT NULL,
	                MapName TEXT NOT NULL UNIQUE,
                    DateAdded TEXT NOT NULL,
                    HashMD5 TEXT NULL,
                    MSetHashMD5 TEXT NULL,
                    MSetDateUpdated TEXT NULL,
                    MSetUpdatedBy TEXT NULL,
                    EntHashMD5 TEXT NULL,
                    EntDateUpdated TEXT NULL,
                    EntUpdatedBy TEXT NULL,
	                PRIMARY KEY (MapId AUTOINCREMENT)
                )
            ";
            command.ExecuteNonQuery();
        }

        static private void CreateTableServers(IDbConnection connection)
        {
            // TODO: We could also have a server IP here to prevent fake times if someone happens
            // to steal the logintoken.  The server IPs should be pretty stable.
            var command = connection.CreateCommand();
            command.CommandText = @"
                CREATE TABLE IF NOT EXISTS Servers
                (
                    ServerId INTEGER NOT NULL,
	                ServerName TEXT NOT NULL,
                    ServerNameShort TEXT NOT NULL,
                    LoginToken TEXT NOT NULL UNIQUE,
                    DateAdded TEXT NOT NULL,
	                PRIMARY KEY (ServerId AUTOINCREMENT)
                )
            ";
            command.ExecuteNonQuery();
        }

        static private void CreateTableMapTimes(IDbConnection connection)
        {
            var command = connection.CreateCommand();
            command.CommandText = @"
                CREATE TABLE IF NOT EXISTS MapTimes
                (
                    MapId INTEGER NOT NULL,
	                UserId INTEGER NOT NULL,
                    ServerId INTEGER NOT NULL,
                    TimeMs INTEGER NOT NULL,
                    PMoveTimeMs INTEGER NOT NULL,
                    Date TEXT NOT NULL,
                    PRIMARY KEY (MapId, UserId)
                )
            ";
            command.ExecuteNonQuery();
        }
    }
}
