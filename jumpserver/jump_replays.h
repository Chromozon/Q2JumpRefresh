#pragma once

#include <map>
#include <vector>
#include <string>
#include "jump_types.h"

namespace Jump
{

enum class StatusCode
{
    Success = 0,

    // Errors are negative values and all end in "Error"
    GenericError = -1,
    InvalidArgumentError = -2,
    GlobalReplayNotCachedError = -3,

};


class Replays
{
public:



    void LoadLocalTop15(std::string mapname);

    void GetLocalReplay(std::string username);
    void GetGlobalReplay(std::string username);

    void GetLocalReplay(int position);

    StatusCode GetGlobalReplay(int position)
    {
        if (position < 1 || position > 15)
        {
            return StatusCode::InvalidArgumentError;
        }
        if (position > m_highscores_global.size())
        {
            return StatusCode::InvalidArgumentError;
        }
        username_key username = m_highscores_global[position];
        auto it = m_replays_global.find(username);
        if (it == m_replays_global.end())
        {
            // Global replay has not been cached yet
            // TODO: queue up a get replay command to the global thread
            return StatusCode::GlobalReplayNotCachedError;
        }
        return StatusCode::Success;
    }

    void ClearReplayCache();

private:
    typedef std::string username_key;

    typedef struct
    {
        username_key username;
        int64_t time_ms;
        int64_t time_pmove_ms;
        std::vector<replay_frame_t> replay_buffer;
    } replay_data;

    std::map<username_key, replay_data> m_replays_local;
    std::map<username_key, replay_data> m_replays_global;

    std::vector<username_key> m_highscores_local;
    std::vector<username_key> m_highscores_global;
};

}