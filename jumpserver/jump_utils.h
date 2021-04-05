#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace Jump
{
    // Performance timer class which can be used to measure execution time.
    // Usage:
    //   PerformanceTimer timer;
    //   timer.Start();
    //   <code>
    //   timer.End();
    //   int ms = timer.DurationMs();
    //
    class PerformanceTimer
    {
    public:
        PerformanceTimer() : _start(), _end()
        {
        }
        void Start()
        {
            _start = std::chrono::high_resolution_clock::now();
        }
        void End()
        {
            _end = std::chrono::high_resolution_clock::now();
        }
        int DurationMilliseconds()
        {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(_end - _start);
            return static_cast<int>(duration.count());
        }
        int DurationSeconds()
        {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(_end - _start);
            return static_cast<int>(duration.count());
        }
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> _start;
        std::chrono::time_point<std::chrono::high_resolution_clock> _end;
    };

    // String case insensitive comparison operator to use with STL containers.
    struct CaseInsensitiveLessCompare
    {
        // case-independent (ci) compare_less binary function
        struct nocase_compare
        {
            bool operator() (const unsigned char& c1, const unsigned char& c2) const {
                return tolower(c1) < tolower(c2);
            }
        };
        bool operator() (const std::string& s1, const std::string& s2) const {
            return std::lexicographical_compare
            (s1.begin(), s1.end(),   // source range
                s2.begin(), s2.end(),   // dest range
                nocase_compare());  // comparison
        }
    };

    // Returns the path to the mod files relative to the root q2 folder ("jumprefresh")
    std::string GetModDir();

    // Returns the path to the mod files relative to the root q2 folder ("jumprefresh/27910")
    std::string GetModPortDir();

    // Removes the folder path from a filename if there is one
    std::string RemovePathFromFilename(const std::string& filepath);

    // Removes the file extension (the last period and everything after)
    std::string RemoveFileExtension(const std::string& filename);

    // Returns a UTC time string of format "YYYY-MM-DD HH:MM:SS"
    const char* GetCurrentTimeUTC();

    // Converts an ASCII string to all lowercase
    std::string AsciiToLower(const std::string& str);

    // Converts an ASCII string to all uppercase
    std::string AsciiToUpper(const std::string& str);

    // Given a time in milliseconds, converts to a display string of the form "54.830"
    // This function expects the given time to always be > 0.
    std::string GetCompletionTimeDisplayString(int64_t time_ms);

    // We use the username in file paths, so make sure it doesn't contain any invalid characters or reserved words.
    // NOTE: Q2 allows the username to have special characters that aren't allowed in filenames.
    // This currently only affects a very small number of users, not sure any are active players.
    bool IsUsernameValid(const std::string& username);

    // Gets the console green text version of the given string.  The given string must be ASCII (0-127).
    // The conversion is to simply add 128 to each char value.  Invalid characters are replaced with space.
    std::string GetGreenConsoleText(const std::string& str);

    // Tries to convert a string to an int.  If the string is an int, returns true, else false.
    bool StringToIntMaybe(const std::string& str, int& num);

    // Tries to convert a string to a float.  If the string is a float, returns true, else false.
    bool StringToFloatMaybe(const std::string& str, float& num);

    // Returns the difference display string between a time and the record time.
    // If the time is better, returns a string "-2.586" as green text.
    // If the time is worse, returns a string "+5.230" as white text.
    std::string GetTimeDiffDisplayString(int64_t time_ms, int64_t record_ms);

    // Splits a string into pieces using the given delimiter.
    std::vector<std::string> SplitString(const std::string& str, char delim);

    // Returns true if two strings are case-insensitive equal.
    bool StringCompareInsensitive(const std::string& left, const std::string& right);

    // Reads the entire contents of a file into a buffer.  Returns true on success, false on failure.
    bool ReadFileIntoBuffer(const std::string& filepath, std::vector<uint8_t>& buffer);

    // Reads the entire contents of a file into a string.  Returns true on success, false on failure.
    bool ReadFileIntoString(const std::string& filepath, std::string& buffer);

    // Gets the date display string "2020/12/23" from a Unix timestamp (seconds)
    std::string GetDateStringFromTimestamp(int64_t unix_s);

    // Generates a random alphanumeric string of length len.
    std::string GenerateRandomString(int len);
    
    // Returns the number of elements in an array.
    template <typename T, size_t N>
    constexpr size_t ArraySize(const T (&arr)[N])
    {
        return N;
    }

    // Trims whitespace from beginning and end of string (in place).
    void TrimString(std::string& s);

    // Shortens a date (with time optional) string to the European format of MM/DD/YY.
    std::string GetEuropeanShortDate(const std::string& datetime);
}