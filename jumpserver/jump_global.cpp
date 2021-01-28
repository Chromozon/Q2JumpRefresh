#include "jump_global.h"
#include "jump_logger.h"
#include <string>
#include <fstream>
#include <vector>
#include "base64.h"
#include "jump_utils.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

// Windows is needed for the HTTP lib header
#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include "httplib.h"

namespace Jump
{
    // Globals
    thread_queue<int> g_addtime_queue;
    thread_queue<int> g_query_queue;

    void ThreadMainAddTime()
    {
        int item = 0;
        while (g_addtime_queue.wait_and_front(item))
        {

        }
    }

    void ThreadMainQuery()
    {
        int item = 0;
        while (g_query_queue.wait_and_pop(item))
        {

        }
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
    std::string GetPlayersQueryCmdJson(const std::string& login_token, player_query query, int page, int count_per_page)
    {
        std::string command;
        if (query == player_query::playertimes)
        {
            command = "playertimes";
        }
        else if (query == player_query::playermaps)
        {
            command = "playermaps";
        }
        else if (query == player_query::playerscores)
        {
            command = "playerscores";
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
    

    void SendTime()
    {
        std::string replay_file = "testreplay.demo";
        std::vector<uint8_t> replay_buffer;
        bool res = ReadFileIntoBuffer(replay_file, replay_buffer);
        std::string b64str = base64::encode(replay_buffer);

        std::string server_token = "1234567890";
        std::string username = "Slip";
        std::string mapname = "ddrace";
        int64_t time_ms = 10567;
        int64_t pmove_time_ms = 10500;
        int64_t date = 1611376216;
        std::string command = "addtime";

        rapidjson::StringBuffer ss;
        rapidjson::Writer<rapidjson::StringBuffer> writer(ss);

        writer.StartObject();
        writer.Key("login_token");
        writer.String(server_token.c_str());
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
            writer.String(b64str.c_str());
        }
        writer.EndObject();
        writer.EndObject(); // root object

        std::string json = ss.GetString();

        std::string url = "http://localhost:57540";
        httplib::Client client(url.c_str());
        int timeout_s = 5;
        client.set_connection_timeout(timeout_s);
        client.set_read_timeout(timeout_s);
        client.set_write_timeout(timeout_s);

        httplib::Result result = client.Post("", json.c_str(), "application/json");

        // TODO: format into HTTP message and send to global database service
    }

    // Send a POST request to the server and get the response.
    // Returns true on success, false on failure.  
    bool PostAndResponse(const std::string& json, std::string& response_data)
    {
        response_data.clear();

        std::string url = "localhost";
        httplib::Client client(url.c_str(), 57540);
        int timeout_s = 60;
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

    void TestHttp()
    {
        std::string json = GetPlayersQueryCmdJson("123456", player_query::playermaps, 1, 20);
        std::string response_data;
        bool success = PostAndResponse(json, response_data);
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