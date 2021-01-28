#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <condition_variable>
#include "thread_queue.h"

namespace Jump
{
    enum class player_query
    {
        playertimes,
        playermaps,
        playerscores
    };

    extern thread_queue<int> g_addtime_queue;
    extern thread_queue<int> g_query_queue;

    // Send a POST request to the server and get the response.
    // Returns true on success, false on failure.  
    bool PostAndResponse(const std::string& json, std::string& response_data);

    std::string GetMaptimesCmdJson(const std::string& login_token, const std::string& mapname, int page, int count_per_page);
    std::string GetPlayersQueryCmdJson(const std::string& login_token, player_query query, int page, int count_per_page);

    void TestHttp();
}