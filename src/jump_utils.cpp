#include "jump_utils.h"
#include "g_local.h"
#include <filesystem>

namespace Jump
{
    // Returns the path to the mod files relative to the root q2 folder ("jumprefresh/27910")
    std::string GetModDir()
    {
        // The server application current directory is always the root q2 folder because that
        // is where dedicated.exe or the client (r1q2.exe/q2pro.exe) is launched from.
        // All files used by this mod must be appended with the game folder path
        // so that we aren't writing them into the root q2 folder.
        std::string path;
        cvar_t* game = gi.cvar("game", "jumprefresh", 0);
        cvar_t* port = gi.cvar("port", "27910", 0);
        if (game != NULL && port != NULL)
        {
            path += game->string;
            path += '/';
            path += port->string;
        }
        return path;
    }

    // Removes anything after the last period
    std::string RemoveFileExtension(const std::string& filename)
    {
        return filename.substr(0, filename.find_last_of("."));
    }

    // Returns a time string of format "YYYY-MM-DD HH:MM:SS"
    const char* GetCurrentTimeUTC()
    {
        static char buffer[128] = {};
        time_t when;
        ::time(&when);
        struct tm* timeinfo = ::gmtime(&when);
        strftime(buffer, sizeof(buffer), "%F %T", timeinfo);
        return buffer;
    }

    // Converts an ASCII string to all lowercase
    std::string AsciiToLower(const std::string& str)
    {
        std::string lower;
        lower.resize(str.size());
        for (size_t i = 0; i < str.size(); ++i)
        {
            lower[i] = static_cast<char>(std::tolower(str[i]));
        }
        return lower;
    }

    // Given a time in milliseconds, converts to a display string of the form "54.830"
    std::string GetCompletionTimeDisplayString(int64_t time_ms)
    {
        char buff[64] = {};
        snprintf(buff, sizeof(buff), "%lld.%03lld", time_ms / 1000, time_ms % 1000);
        return buff;
    }

} // namespace Jump