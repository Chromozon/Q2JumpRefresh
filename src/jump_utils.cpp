#include "jump_utils.h"
#include "g_local.h"
#include <filesystem>
#include <unordered_set>

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

    // Removes the folder path from a filename if there is one
    std::string RemovePathFromFilename(const std::string& filepath)
    {
        return std::filesystem::path(filepath).filename().generic_string();
    }

    // Removes the file extension (the last period and everything after)
    std::string RemoveFileExtension(const std::string& filename)
    {
        return filename.substr(0, filename.find_last_of("."));
    }

    // Returns a UTC time string of format "YYYY-MM-DD HH:MM:SS"
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

    // Converts an ASCII string to all uppercase
    std::string AsciiToUpper(const std::string& str)
    {
        std::string upper;
        upper.resize(str.size());
        for (size_t i = 0; i < str.size(); ++i)
        {
            upper[i] = static_cast<char>(std::toupper(str[i]));
        }
        return upper;
    }

    // Given a time in milliseconds, converts to a display string of the form "54.830"
    std::string GetCompletionTimeDisplayString(int64_t time_ms)
    {
        char buff[64] = {};
        snprintf(buff, sizeof(buff), "%lld.%03lld", time_ms / 1000, time_ms % 1000);
        return buff;
    }

    // We use the username in file paths, so make sure it doesn't contain any invalid characters or reserved words.
    // NOTE: Q2 allows the username to have special characters that aren't allowed in filenames.
    // This currently only affects a very small number of users, not sure any are active players.
    bool IsUsernameValid(const std::string& username)
    {
        // Empty username
        if (username.size() == 0)
        {
            return false;
        }
        // Non-printable ASCII characters (0-31)
        for (char c : username)
        {
            if (c <= 31)
            {
                return false;
            }
        }
        // Username cannot end with period or space
        if (username.back() == '.' || username.back() == ' ')
        {
            return false;
        }
        // Certain special characters
        const char* reserved_chars = "<>:\"/\\|?*";
        if (username.find_first_of(reserved_chars) != std::string::npos)
        {
            return false;
        }
        static std::unordered_set<std::string> keywords =
        {
            "CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4",
            "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3",
            "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
        };
        std::string username_upper = AsciiToUpper(username);
        return keywords.find(username_upper) == keywords.end();
    }

} // namespace Jump