#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <time.h>

// The server log file stores errors, warnings, info, and debug messages
#define PATH_TO_SERVER_LOGFILE "logs/log.txt"

// The server completions log stores a record of every time a client completes a map
#define PATH_TO_SERVER_COMPLETIONS_LOG "logs/completions.txt"

// To use the Logger, simply call the functions:
//
//   Logger::Error("Oh no, an error");
//   Logger::Warning("Something strange happened");
//   Logger::Info("Someone joined the server");
//   Logger::Debug("This will only show up in debug builds");
//
//   Logger::Completion("Slippery", "alt20", 17258);
//

namespace Jump
{
    class Logger
    {
    public:

        static void Error(const std::string& error)
        {
            if (GetLogHandle())
            {
                log_handle << GetCurrentTimeUTC() << '\t' << "ERROR: " << error << '\n';
                log_handle.flush();
            }
        }

        static void Warning(const std::string& warning)
        {
            if (GetLogHandle())
            {
                log_handle << GetCurrentTimeUTC() << '\t' << "WARNING: " << warning << '\n';
                log_handle.flush();
            }
        }

        static void Info(const std::string& info)
        {
            if (GetLogHandle())
            {
                log_handle << GetCurrentTimeUTC() << '\t' << "INFO: " << info << '\n';
            }
        }

        static void Debug(const std::string& debug)
        {
            #ifdef _DEBUG
            if (GetLogHandle())
            {
                log_handle << GetCurrentTimeUTC() << '\t' << "DEBUG: " << debug << '\n';
            }
            #endif
        }

        static void Completion(const std::string& client_name, const std::string& map_name, int64_t map_time_ms)
        {
            if (GetCompletionsHandle())
            {
                char buffer[1024];
                snprintf(buffer, sizeof(buffer), "%s\t%s\t%s\t%lld.%03lld\n",
                    GetCurrentTimeUTC(),
                    map_name.c_str(),
                    client_name.c_str(),
                    map_time_ms / 1000,
                    map_time_ms % 1000);
                completions_handle << buffer;
                completions_handle.flush();
            }
        }

    private:
        Logger() = delete;

        static constexpr char* log_path = PATH_TO_SERVER_LOGFILE;
        static constexpr char* completions_path = PATH_TO_SERVER_COMPLETIONS_LOG;
        static std::fstream log_handle;
        static std::fstream completions_handle;

        static bool GetLogHandle()
        {
            if (!log_handle.is_open())
            {
                log_handle.open(log_path, std::ios::app);
                if (!log_handle.is_open())
                {
                    std::cerr << "Could not open log file " << log_path << "\n";
                }
            }
            return log_handle.is_open();
        }

        static bool GetCompletionsHandle()
        {
            if (!completions_handle.is_open())
            {
                completions_handle.open(completions_path, std::ios::app);
                if (!completions_handle.is_open())
                {
                    std::cerr << "Could not open log file " << completions_path << "\n";
                }
            }
            return completions_handle.is_open();
        }

        static char* GetCurrentTimeUTC()
        {
            time_t when;
            ::time(&when);
            struct tm* timeinfo = ::gmtime(&when);
            return ::asctime(timeinfo);
        }
    };
}