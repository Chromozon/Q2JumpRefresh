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
	                PRIMARY KEY (MapId AUTOINCREMENT)
                )
            ";
            command.ExecuteNonQuery();
        }

        static private void CreateTableServers(IDbConnection connection)
        {
            var command = connection.CreateCommand();
            command.CommandText = @"
                CREATE TABLE IF NOT EXISTS Servers
                (
                    ServerId INTEGER NOT NULL,
	                ServerName TEXT NOT NULL,
                    ServerNameShort TEXT NOT NULL,
                    LoginToken TEXT NOT NULL,
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
