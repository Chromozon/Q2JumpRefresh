#pragma once

#include "g_local.h"
#include "pgsql/include/libpq-fe.h"
#include <string>
#include <sstream>

namespace Jump
{
    const std::string Database_Address = "127.0.0.1";
    const std::string Database_Port = "5432";
    const std::string Database_Name = "q2jump";
    const std::string Database_User = "q2jumpserver";
    const std::string Database_Pass = "123456";
    const std::string Database_Params = "connect_timeout=5";

    class DatabaseConnection
    {
    public:
        DatabaseConnection() : m_conn(NULL)
        {
            //postgresql://[user[:password]@][netloc][:port][/dbname][?param1=value1&...]
            //postgresql://other@localhost/otherdb?connect_timeout=10&application_name=myapp

            std::string conn_str = "postgresql://"
                + Database_User + ":"
                + Database_Pass + "@"
                + Database_Address + ":"
                + Database_Port + "/"
                + Database_Name +"?"
                + Database_Params;

            m_conn = PQconnectdb(conn_str.c_str());
        }
        ~DatabaseConnection()
        {
            PQfinish(m_conn);
            m_conn = NULL;
        }
        bool Isvalid()
        {
            ConnStatusType status = PQstatus(m_conn);
            return status == CONNECTION_OK;
        }
        const char* GetError()
        {
            return PQerrorMessage(m_conn);
        }
        PGconn* GetConn()
        {
            return m_conn;
        }
    private:
        PGconn* m_conn;
    };

    class DatabaseActions
    {
    public:
        void AddNewUser(const char* name)
        {
            const char* paramValues[1] = { name };
            const char* cmd =
                "INSERT INTO Users"
                "    (UserId, UserName, LastSeen, DateAdded)"
                "VALUES"
                "    (DEFAULT, $1, DEFAULT, DEFAULT)";

            int res = PQsendQueryParams(m_db.GetConn(),
                cmd,
                1, /* 1 parameter */
                NULL, /* let the backend deduce param type */
                paramValues,
                NULL, /* don't need length for text */
                NULL, /* default to text params */
                0 /* text result format */
            );
            if (res == 0)
            {
                // Unexpected failure to send the command
                gi.error("Error from database: %s\n", PQerrorMessage(m_db.GetConn()));
            }
        }

        void UpdateUserLastSeen(int userId)
        {
            std::stringstream ss;
            ss <<
                "UPDATE Users"
                "SET LastSeen = CURRENT_TIMESTAMP"
                "WHERE UserId = " << userId;

            int res = PQsendQuery(m_db.GetConn(), ss.str().c_str());
            if (res == 0)
            {
                // Unexpected failure to send the command
                gi.error("Error from database: %s\n", PQerrorMessage(m_db.GetConn()));
            }
        }
    private:
        DatabaseConnection m_db;
    };
}