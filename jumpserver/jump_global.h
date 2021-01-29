#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <condition_variable>
#include "thread_queue.h"
#include "jump_types.h"

namespace Jump
{
    // TODO
    static const char* LOGIN_TOKEN = "123456";

    static const char* DATABASE_SERVICE_URL = "localhost";
    static const int DATABASE_SERVICE_PORT = 57540;

    // All of the global database commands
    enum class global_cmd
    {
        playertimes,
        playermaps,
        playerscores,
        addtime,
        userlogin,
        changepassword,
        addmap,
        maptimes
    };

    // Interface class which all other global database command classes inherit from
    class global_cmd_base
    {
    public:
        virtual global_cmd get_type() const = 0;
    };

    // Global playertimes
    class global_cmd_playertimes : public global_cmd_base
    {
    public:
        global_cmd get_type() const override { return global_cmd::playertimes; }
        edict_t* user;
        int page;
        int count_per_page;
    };

    // Global playermaps
    class global_cmd_playermaps : public global_cmd_base
    {
    public:
        global_cmd get_type() const override { return global_cmd::playermaps; }
        edict_t* user;
        int page;
        int count_per_page;
    };

    // Global playerscores
    class global_cmd_playerscores : public global_cmd_base
    {
    public:
        global_cmd get_type() const override { return global_cmd::playerscores; }
        edict_t* user;
        int page;
        int count_per_page;
    };

    // Global command response
    class global_cmd_response
    {
    public:
        std::shared_ptr<global_cmd_base> cmd_base;
        std::string data;
        bool success;
    };

    std::string GetMaptimesCmdJson(const std::string& login_token, const std::string& mapname, int page, int count_per_page);

    std::string GetPlayersQueryCmdJson(const std::string& login_token, global_cmd query, int page, int count_per_page);

    std::string GetAddTimeCmdJson(
        const std::string& login_token,
        const std::string& username,
        const std::string& mapname,
        int64_t time_ms,
        int64_t pmove_time_ms,
        int64_t date,
        const std::vector<replay_frame_t>& replay_buffer);

    // Send a POST request to the server and get the response.
    // Returns true on success, false on failure.
    bool PostAndResponse(const std::string& json, std::string& response_data);
}