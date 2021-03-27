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


    class IGlobalCommand
    {
    public:
        virtual void ActionBeforePost() = 0;
        virtual std::string GetJsonPayload() = 0;
        virtual void HandleResponse(int status, const std::string& data) = 0;
    };

    class GlobalCommandPlayertimes : public IGlobalCommand
    {
    public:

    };


    // Interface class which all other global database command classes inherit from
    class global_cmd_base
    {
    public:
        virtual global_cmd get_type() const = 0;
    };

    // Playertimes
    class global_cmd_playertimes : public global_cmd_base
    {
    public:
        global_cmd get_type() const override { return global_cmd::playertimes; }
        edict_t* user;
        int page;
        int count_per_page;
    };

    // Playermaps
    class global_cmd_playermaps : public global_cmd_base
    {
    public:
        global_cmd get_type() const override { return global_cmd::playermaps; }
        edict_t* user;
        int page;
        int count_per_page;
    };

    // Playerscores
    class global_cmd_playerscores : public global_cmd_base
    {
    public:
        global_cmd get_type() const override { return global_cmd::playerscores; }
        edict_t* user;
        int page;
        int count_per_page;
    };

    // Maptimes
    class global_cmd_maptimes : public global_cmd_base
    {
    public:
        global_cmd get_type() const override { return global_cmd::maptimes; }
        edict_t* user;
        int page;
        int count_per_page;
        std::string mapname;
    };

    // Addtime
    class global_cmd_addtime : public global_cmd_base
    {
    public:
        global_cmd get_type() const override { return global_cmd::addtime; }
        std::string mapname;
        std::string username;
        int64_t date;
        int64_t time_ms;
        int64_t pmove_time_ms;
        std::vector<replay_frame_t> replay_buffer;
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

    void ThreadMainGlobal();

    void StopThreadMainGlobal();

    bool TryGetGlobalDatabaseCmdResponse(std::shared_ptr<global_cmd_response>& response);

    void QueueGlobalDatabaseCmd(std::shared_ptr<global_cmd_base> cmd);

    // Send a POST request to the server and get the response.
    // Returns true on success, false on failure.
    bool PostAndResponse(const std::string& json, std::string& response_data);
}