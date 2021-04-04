#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <time.h>

// The directory that stores the various log files.
// Note that the full path will be as follows: Quake2/<JumpGame>/<Port>/<SERVER_LOGS_DIR>/
#define LOGS_DIR "logs"

// This file stores general server activity (players joining and leaving, players talking, etc.)
#define ACTIVITY_LOG_FILENAME "activity.txt"

// The server log file stores errors, warnings, info, and debug messages
#define SERVER_LOG_FILENAME "server.txt"

// The server completions log stores a record of every time a client completes a map
#define COMPLETIONS_LOG_FILENAME "completions.txt"

// To use the Logger, simply call the functions:
//
//   Logger::Error("Oh no, an error");
//   Logger::Warning("Something strange happened");
//   Logger::Info("Someone joined the server");
//   Logger::Debug("This will only show up in debug builds");
//
//   Logger::Completion("Slippery", "alt20", 17258);
//
//   Logger::Activity("Someone joined the game");
///

namespace Jump
{
    class Logger
    {
    public:
        static void Fatal(const std::string& fatal);
        static void Error(const std::string& error);
        static void Warning(const std::string& warning);
        static void Info(const std::string& info);
        static void Debug(const std::string& debug);
        static void Completion(
            const std::string& client_name,
            const std::string& client_ip,
            const std::string& map_name,
            int64_t map_time_ms);
        static void Activity(const std::string& msg);
        static void DebugConsole(const std::string& debug);

    private:
        Logger() = delete;
        static bool GetServerLogHandle();
        static bool GetCompletionsLogHandle();
        static bool GetActivityLogHandle();
    };
}
