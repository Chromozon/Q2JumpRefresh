#include "jump_utils.h"
#include "g_local.h"
#include <filesystem>
#include <unordered_set>
#include <fstream>
#include <time.h>

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
    // This function expects the given time to always be > 0.
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

    // Gets the console green text version of the given string.  The given string must be ASCII (0-127).
    // The conversion is to simply add 128 to each char value.  Invalid characters are replaced with space.
    std::string GetGreenConsoleText(const std::string& str)
    {
        std::string green;
        green.resize(str.size());
        for (size_t i = 0; i < str.size(); ++i)
        {
            unsigned char c = str[i];
            if (c > 127)
            {
                c = ' '; // replace invalid characters with space
            }
            else
            {
                c = c + 128;
            }
            green[i] = c;
        }
        return green;
    }

    // Tries to convert a string to an int.  If the string is an int, returns true, else false.
    bool StringToIntMaybe(const std::string& str, int& num)
    {
        try
        {
            num = std::stoi(str);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    // Returns the difference display string between a time and the record time.
    // If the time is better, returns a string "-2.586" as green text.
    // If the time is worse, returns a string "+5.230" as white text.
    std::string GetTimeDiffDisplayString(int64_t time_ms, int64_t record_ms)
    {
        if (time_ms < record_ms)
        {
            std::string s = std::string("-") + GetCompletionTimeDisplayString(record_ms - time_ms);
            return GetGreenConsoleText(s);
        }
        else
        {
            std::string s = std::string("+") + GetCompletionTimeDisplayString(time_ms - record_ms);
            return s;
        }
    }

    // Splits a string into pieces using the given delimiter.
    std::vector<std::string> SplitString(const std::string& str, char delim)
    {
        std::vector<std::string> tokens;
        size_t start;
        size_t end = 0;
        while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
        {
            end = str.find(delim, start);
            tokens.push_back(str.substr(start, end - start));
        }
        return tokens;
    }

    // Returns true if two strings are case-insensitive equal.
    bool StringCompareInsensitive(const std::string& left, const std::string& right)
    {
        if (left.size() != right.size())
        {
            return false;
        }
        return std::equal(left.begin(), left.end(), right.begin(),
            [](char a, char b)
            {
                return tolower(a) == tolower(b);
            }
        );
    }

    // Reads the entire contents of a file into a buffer.  Returns true on success, false on failure.
    bool ReadFileIntoBuffer(const std::string& filepath, std::vector<uint8_t>& buffer)
    {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            return false;
        }
        size_t num_bytes = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);
        buffer.resize(num_bytes);
        file.read(reinterpret_cast<char*>(&buffer[0]), num_bytes);
        return file.eof();
    }

    // Gets the date display string "2020/12/23" from a Unix timestamp (seconds)
    std::string GetDateStringFromTimestamp(int64_t unix_s)
    {
        std::string date_string(11, '\0');
        time_t time = unix_s;
        strftime(&date_string[0], 11, "%Y/%m/%d", gmtime(&time));
        return date_string;
    }

} // namespace Jump