#pragma once

#include <string>
#include <fstream>
#include <time.h>

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

        static constexpr char* log_path = "logs/log.txt";
        static constexpr char* completions_path = "logs/completions.txt";
        static std::fstream log_handle;
        static std::fstream completions_handle;

        static bool GetLogHandle()
        {
            if (!log_handle.is_open())
            {
                log_handle.open(log_path, std::ios::app);
            }
            return log_handle.is_open();
        }

        static bool GetCompletionsHandle()
        {
            if (!completions_handle.is_open())
            {
                completions_handle.open(completions_path, std::ios::app);
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