#pragma once

#include <string>

namespace Jump
{
    std::string GetModDir();
    std::string RemoveFileExtension(const std::string& filename);
    const char* GetCurrentTimeUTC();
    std::string AsciiToLower(const std::string& str);
    std::string GetCompletionTimeDisplayString(int64_t time_ms);
}