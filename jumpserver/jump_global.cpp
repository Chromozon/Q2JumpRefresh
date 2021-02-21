#include "jump_global.h"
#include "jump_logger.h"
#include <string>
#include <fstream>
#include <vector>
#include "base64.h"
#include "jump_utils.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <filesystem>

// Windows is needed for the HTTP lib header
#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include "httplib.h"
#include "jump_types.h"

namespace Jump
{
    // Queue of all commands that need to be sent to the global database
    static thread_queue<std::shared_ptr<global_cmd_base>> global_cmd_queue;

    // Queue of all command response data
    // TODO: don't really need a thread_queue for this, just need a queue and a simple lock
    static thread_queue<std::shared_ptr<global_cmd_response>> global_cmd_response_queue;

    // Main thread that sends commands to the global database service and receives the responses
    void ThreadMainGlobal()
    {
        std::shared_ptr<global_cmd_base> cmd_base;
        while (global_cmd_queue.wait_and_pop(cmd_base))
        {
            std::string temp_path;
            std::string json;
            global_cmd cmd_type = cmd_base->get_type();
            if (cmd_type == global_cmd::playertimes)
            {
                global_cmd_playertimes* cmd = dynamic_cast<global_cmd_playertimes*>(cmd_base.get());
                json = GetPlayersQueryCmdJson(LOGIN_TOKEN, cmd_type, cmd->page, cmd->count_per_page);
            }
            else if (cmd_type == global_cmd::playermaps)
            {
                global_cmd_playermaps* cmd = dynamic_cast<global_cmd_playermaps*>(cmd_base.get());
                json = GetPlayersQueryCmdJson(LOGIN_TOKEN, cmd_type, cmd->page, cmd->count_per_page);
            }
            else if (cmd_type == global_cmd::playerscores)
            {
                global_cmd_playerscores* cmd = dynamic_cast<global_cmd_playerscores*>(cmd_base.get());
                json = GetPlayersQueryCmdJson(LOGIN_TOKEN, cmd_type, cmd->page, cmd->count_per_page);
            }
            else if (cmd_type == global_cmd::maptimes)
            {
                global_cmd_maptimes* cmd = dynamic_cast<global_cmd_maptimes*>(cmd_base.get());
                json = GetMaptimesCmdJson(LOGIN_TOKEN, cmd->mapname, cmd->page, cmd->count_per_page);
            }
            else if (cmd_type == global_cmd::addtime)
            {
                global_cmd_addtime* cmd = dynamic_cast<global_cmd_addtime*>(cmd_base.get());
                json = GetAddTimeCmdJson(LOGIN_TOKEN, cmd->username, cmd->mapname, cmd->time_ms,
                    cmd->pmove_time_ms, cmd->date, cmd->replay_buffer);

                std::string queue_dir = GetModDir() + '/' + "global_uploads_tmp";
                std::filesystem::create_directories(queue_dir);
                temp_path = queue_dir + '/' + GenerateRandomString(16);
                std::ofstream temp_file(temp_path, std::ios::trunc);
                if (temp_file.is_open())
                {
                    temp_file << "username\t" << cmd->username << std::endl;
                    temp_file << "mapname\t" << cmd->mapname << std::endl;
                    temp_file << "date\t" << cmd->date << std::endl;
                    temp_file << "time_ms\t" << cmd->time_ms << std::endl;
                    temp_file << "pmove_time_ms\t" << cmd->pmove_time_ms << std::endl;
                }
                else
                {
                    // log error, but try to still send to server
                }
                temp_file.close();

            }
            std::string response_data;
            bool success = PostAndResponse(json, response_data);

            if (cmd_type == global_cmd::addtime && success)
            {
                std::filesystem::remove(temp_path);
            }

            std::shared_ptr<global_cmd_response> cmd_response = std::make_shared<global_cmd_response>();
            cmd_response->cmd_base = cmd_base;
            cmd_response->data = response_data;
            cmd_response->success = success;
            global_cmd_response_queue.push(cmd_response);
        }
    }

    // Tell the main thread to stop
    void StopThreadMainGlobal()
    {
        global_cmd_queue.stop_thread();
    }

    // Add a global command to the queue
    void QueueGlobalDatabaseCmd(std::shared_ptr<global_cmd_base> cmd) // TODO simplify name (remove Database)
    {
        global_cmd_queue.push(cmd);
    }

    // Try to get a response from the response queue if there is one.
    // Returns true if a response is returned, else false.
    bool TryGetGlobalDatabaseCmdResponse(std::shared_ptr<global_cmd_response>& response)
    {
        return global_cmd_response_queue.try_pop(response);
    }

    // Format the json string for "maptimes" command
    std::string GetMaptimesCmdJson(const std::string& login_token, const std::string& mapname, int page, int count_per_page)
    {
        std::string command = "maptimes";

        rapidjson::StringBuffer ss;
        rapidjson::Writer<rapidjson::StringBuffer> writer(ss);

        writer.StartObject();
        writer.Key("login_token");
        writer.String(login_token.c_str());
        writer.Key("command");
        writer.String(command.c_str());
        writer.Key("command_args");
        writer.StartObject();
        {
            writer.Key("mapname");
            writer.String(mapname.c_str());
            writer.Key("page");
            writer.Int(page);
            writer.Key("count_per_page");
            writer.Int(count_per_page);
        }
        writer.EndObject();
        writer.EndObject(); // root object

        std::string json = ss.GetString();
        return json;
    }

    // Format the json string for the player queries
    std::string GetPlayersQueryCmdJson(const std::string& login_token, global_cmd query, int page, int count_per_page)
    {
        std::string command;
        if (query == global_cmd::playertimes)
        {
            command = "playertimes";
        }
        else if (query == global_cmd::playermaps)
        {
            command = "playermaps";
        }
        else if (query == global_cmd::playerscores)
        {
            command = "playerscores";
        }
        else
        {
            assert(false);
            return command;
        }

        rapidjson::StringBuffer ss;
        rapidjson::Writer<rapidjson::StringBuffer> writer(ss);

        writer.StartObject();
        writer.Key("login_token");
        writer.String(login_token.c_str());
        writer.Key("command");
        writer.String(command.c_str());
        writer.Key("command_args");
        writer.StartObject();
        {
            writer.Key("page");
            writer.Int(page);
            writer.Key("count_per_page");
            writer.Int(count_per_page);
        }
        writer.EndObject();
        writer.EndObject(); // root object

        std::string json = ss.GetString();
        return json;
    }
    
    // Format the json string for the addtime command
    std::string GetAddTimeCmdJson(
        const std::string& login_token,
        const std::string& username,
        const std::string& mapname,
        int64_t time_ms,
        int64_t pmove_time_ms,
        int64_t date,
        const std::vector<replay_frame_t>& replay_buffer)
    {
        std::vector<uint8_t> replay_bytes = SerializeReplayBuffer(replay_buffer);
        std::string replay_bytes_b64 = base64::encode(replay_bytes);

        std::string command = "addtime";

        rapidjson::StringBuffer ss;
        rapidjson::Writer<rapidjson::StringBuffer> writer(ss);

        writer.StartObject();
        writer.Key("login_token");
        writer.String(login_token.c_str());
        writer.Key("command");
        writer.String(command.c_str());
        writer.Key("command_args");
        writer.StartObject();
        {
            writer.Key("mapname");
            writer.String(mapname.c_str());
            writer.Key("username");
            writer.String(username.c_str());
            writer.Key("date");
            writer.Int64(date);
            writer.Key("time_ms");
            writer.Int64(time_ms);
            writer.Key("pmove_time_ms");
            writer.Int64(pmove_time_ms);
            writer.Key("replay_data");
            writer.String(replay_bytes_b64.c_str());
        }
        writer.EndObject();
        writer.EndObject(); // root object

        std::string json = ss.GetString();
        return json;
    }

    // Send a POST request to the server and get the response.
    // Returns true on success, false on failure.  
    bool PostAndResponse(const std::string& json, std::string& response_data)
    {
        response_data.clear();

        httplib::Client client(DATABASE_SERVICE_URL, DATABASE_SERVICE_PORT);
        int timeout_s = 10;
        client.set_connection_timeout(timeout_s);
        client.set_read_timeout(timeout_s);
        client.set_write_timeout(timeout_s);

        httplib::Result result = client.Post("/", json.c_str(), "application/json");
        if (result.error() == httplib::Error::Success)
        {
            response_data = result.value().body;
            return true;
        }
        else
        {
            Logger::Error("Bad response from database service: " + std::to_string(result.error()));
            return false;
        }
    }

    // How to handle sending a time to global database:
    // -- MAIN THREAD
    // Save time to local filesystem
    // Save replay to local filesystem
    // Save replay to a temporary holding area on local filesystem (use unique name)
    // Lock write queue <-- this should be a file so that the queue doesn't go poof if the server dies
    // Push an addtime cmd to queue (add to end of file) (time and location of temp replay on filesystem)
    // Unlock write queue
    // Signal a new queue item has been added by incrementing a counting semaphore
    // Main thread resumes its business
    // -- HTTP THREAD
    // Lock write queue
    // Copy Top() of write queue to local variables (but don't dequeue yet)
    // Unlock write queue
    // Read the temp replay data from filesystem
    // Send an addtime cmd to global database and wait for response
    // If failed response, retry (how many times?)
    // If good response
    // Delete temp replay file
    // Lock write queue
    // Pop top of write queue (remove top line of file, shift everything else up; the file should never be very large so this should be fine)
    // Unlock write queue
    // <Optional> (Actually, I don't think we need this for addtime cmds- these are push only)
    // Lock response queue
    // Push response
    // Unlock response queue
    // Wait for next item in queue (counting semaphore)
    // -- MAIN THREAD
    // At end of server frame, look for responses in the response queue

    // We want to avoid the situation where the user sets multiple times before the first one
    // has written to the global database; the replay file gets overwritten in this case,
    // so we need to save each individual replay to a temp location.  This way we never
    // send a replay that doesn't match the time set.

    // We can have two HTTP threads- one thread for addtime and another thread for all client query cmds (maptimes, playerscores, etc.)
    // The addtime needs to have file system backup for the queue, the client cmds do not

}