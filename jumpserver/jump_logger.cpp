#include "jump_logger.h"
#include "g_local.h"
#include "jump_utils.h"
#include <filesystem>
#include <sstream>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace Jump
{
    static std::ofstream server_log_handle;
    static std::ofstream completions_log_handle;
    static std::ofstream activity_log_handle;

    void Logger::Fatal(const std::string& fatal)
    {
        if (GetServerLogHandle())
        {
            std::stringstream ss;
            ss << GetCurrentTimeUTC() << '\t' << "FATAL: " << fatal << '\n';
            server_log_handle << ss.str();
            server_log_handle.flush();
            DebugConsole(ss.str());
            gi.error("FATAL: %s", ss.str().c_str());
        }
    }

    void Logger::Error(const std::string& error)
    {
        if (GetServerLogHandle())
        {
            std::stringstream ss;
            ss << GetCurrentTimeUTC() << '\t' << "ERROR: " << error << '\n';
            server_log_handle << ss.str();
            server_log_handle.flush();
            DebugConsole(ss.str());
        }
    }

    void Logger::Warning(const std::string& warning)
    {
        if (GetServerLogHandle())
        {
            std::stringstream ss;
            ss << GetCurrentTimeUTC() << '\t' << "WARNING: " << warning << '\n';
            server_log_handle << ss.str();
            server_log_handle.flush();
            DebugConsole(ss.str());
        }
    }

    void Logger::Info(const std::string& info)
    {
        if (GetServerLogHandle())
        {
            std::stringstream ss;
            ss << GetCurrentTimeUTC() << '\t' << "INFO: " << info << '\n';
            server_log_handle << ss.str();
            DebugConsole(ss.str());
        }
    }

    void Logger::Debug(const std::string& debug)
    {
        #ifdef _DEBUG
        if (GetServerLogHandle())
        {
            std::stringstream ss;
            ss << GetCurrentTimeUTC() << '\t' << "DEBUG: " << debug << '\n';
            server_log_handle << ss.str();
            DebugConsole(ss.str());
        }
        #endif
    }

    void Logger::DebugConsole(const std::string& debug)
    {
        #ifdef _WIN32
        OutputDebugStringA(debug.c_str());
        #endif
    }

    void Logger::Completion(
        const std::string& client_name,
        const std::string& client_ip,
        const std::string& map_name,
        int64_t map_time_ms)
    {
        if (GetCompletionsLogHandle())
        {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "%s\t%s\t%s\t%s\t%lld.%03lld\n",
                GetCurrentTimeUTC(),
                client_name.c_str(),
                client_ip.c_str(),
                map_name.c_str(),
                map_time_ms / 1000,
                map_time_ms % 1000);
            completions_log_handle << buffer;
            completions_log_handle.flush();
            DebugConsole(buffer);
        }
    }

    void Logger::Activity(const std::string& msg)
    {
        if (GetActivityLogHandle())
        {
            std::stringstream ss;
            ss << GetCurrentTimeUTC() << '\t' << msg << '\n';
            activity_log_handle << ss.str();
            DebugConsole(ss.str());
        }
    }

    bool Logger::GetServerLogHandle()
    {
        if (!server_log_handle.is_open())
        {
            std::string path = GetModPortDir();
            path += '/';
            path += LOGS_DIR;
            std::filesystem::create_directories(path);
            path += '/';
            path += SERVER_LOG_FILENAME;
            server_log_handle.open(path, std::ios::app);
            if (!server_log_handle.is_open())
            {
                std::cerr << "Could not open log file \"" << path << "\"\n";
            }
        }
        return server_log_handle.is_open();
    }

    bool Logger::GetCompletionsLogHandle()
    {
        if (!completions_log_handle.is_open())
        {
            std::string path = GetModPortDir();
            path += '/';
            path += LOGS_DIR;
            std::filesystem::create_directories(path);
            path += '/';
            path += COMPLETIONS_LOG_FILENAME;
            completions_log_handle.open(path, std::ios::app);
            if (!completions_log_handle.is_open())
            {
                std::cerr << "Could not open log file \"" << path << "\"\n";
            }
        }
        return completions_log_handle.is_open();
    }

    bool Logger::GetActivityLogHandle()
    {
        if (!activity_log_handle.is_open())
        {
            std::string path = GetModPortDir();
            path += '/';
            path += LOGS_DIR;
            std::filesystem::create_directories(path);
            path += '/';
            path += ACTIVITY_LOG_FILENAME;
            activity_log_handle.open(path, std::ios::app);
            if (!activity_log_handle.is_open())
            {
                std::cerr << "Could not open log file \"" << path << "\"\n";
            }
        }
        return activity_log_handle.is_open();
    }

} // namespace Jump