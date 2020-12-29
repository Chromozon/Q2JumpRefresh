#pragma once

#include <string>

namespace Jump
{
    std::string GetModDir();
    std::string RemovePathFromFilename(const std::string& fileWithPath);
    std::string RemoveFileExtension(const std::string& filename);
    const char* GetCurrentTimeUTC();
    std::string AsciiToLower(const std::string& str);
    std::string AsciiToUpper(const std::string& str);
    std::string GetCompletionTimeDisplayString(int64_t time_ms);
    bool IsUsernameValid(const std::string& username);
}