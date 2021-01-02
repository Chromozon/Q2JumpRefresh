#pragma once

#include "q_shared.h"
#define GAME_INCLUDE
#include "game.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <unordered_set>

// Forward declarations
//struct edict_t;

const int REPLAY_FRAMES_PER_SECOND = 10;
const int MAX_REPLAY_LENGTH_SECONDS = 24 * 60 * 60; // 24 hours
const int MAX_REPLAY_FRAMES2 = MAX_REPLAY_LENGTH_SECONDS * REPLAY_FRAMES_PER_SECOND;

#define MAX_STORES 5
#define MAX_REPLAY_FRAMES 10000 // 10 frames/sec are recorded for replays

namespace Jump
{
    typedef enum
    {
        TEAM_SPECTATOR,
        TEAM_EASY,
        TEAM_HARD
    } team_t;

    typedef enum
    {
        LEVEL_STATE_FREEPLAY,
        LEVEL_STATE_VOTING,
        LEVEL_STATE_INTERMISSION
    } level_state_t;

    typedef enum
    {
        KEY_STATE_NONE = 0,
        KEY_STATE_FORWARD = 1 << 0,
        KEY_STATE_BACK = 1 << 1,
        KEY_STATE_LEFT = 1 << 2,
        KEY_STATE_RIGHT = 1 << 3,
        KEY_STATE_JUMP = 1 << 4,
        KEY_STATE_CROUCH = 1 << 5,
        KEY_STATE_ATTACK = 1 << 6,
    } key_state_t;

    typedef enum
    {
        REPLAY_SPEED_NEG_100 = -10000,
        REPLAY_SPEED_NEG_25 = -2500,
        REPLAY_SPEED_NEG_10 = -1000,
        REPLAY_SPEED_NEG_5 = -500,
        REPLAY_SPEED_NEG_2 = -200,
        REPLAY_SPEED_NEG_1 = -100,
        REPLAY_SPEED_NEG_HALF = -50,
        REPLAY_SPEED_NEG_FIFTH = -20,
        REPLAY_SPEED_NEG_TENTH = -10,
        REPLAY_SPEED_0 = 0,
        REPLAY_SPEED_TENTH = 10,
        REPLAY_SPEED_FIFTH = 20,
        REPLAY_SPEED_HALF = 50,
        REPLAY_SPEED_1 = 100,
        REPLAY_SPEED_2 = 200,
        REPLAY_SPEED_5 = 500,
        REPLAY_SPEED_10 = 1000,
        REPLAY_SPEED_25 = 2500,
        REPLAY_SPEED_100 = 10000,
    } replay_speed_t;

    // How much disk space is required to store a replay:
    // 40 bytes per replay frame, 10 frames per second (because server runs at 10 frames/s) = 400 bytes/s
    // 15 seconds = 6 kB
    // 1 minute = 24 kB
    // 5 minutes = 120 kB
    // 15 minutes = 360 kB
    // 1 hour = 1.44 MB
    // 24 hours = 34.56 MB
    //
    // Let's say on average 50 players complete each map, and the time is 2 minutes, and there are 3000 maps
    // 50 replays * 48 kB/replay * 3000 maps = 7.2 GB
    // 
    typedef struct
    {
        vec3_t pos;         // (12 bytes) player position in the world
        vec3_t angles;      // (12 bytes) player view angles
        int32_t key_states; // (4 bytes) active inputs (jump, crouch, left, right, etc.)
        int32_t fps;        // (4 bytes) current fps
        int32_t reserved1;  // (4 bytes) reserved bytes for future use (checkpoints, weapons, etc.)
        int32_t reserved2;  // (4 bytes) reserved bytes for future use
    } replay_frame_t;

    typedef struct
    {
        int64_t time_interval;
        vec3_t pos;
        vec3_t angles;
        // todo weapons
    } store_data_t;

    class store_buffer_t
    {
    public:
        store_buffer_t();
        void PushStore(const store_data_t& data);
        store_data_t GetStore(int prevNum);
        void Reset();
        bool HasStore();
    private:
        int numStores;
        int nextIndex;
        store_data_t stores[MAX_STORES];
    };

    class client_data_t
    {
    public:
        client_data_t();

        std::vector<replay_frame_t> replay_recording;
        std::vector<replay_frame_t> replay_spectating;
        int replay_spectating_framenum;
        bool update_replay_spectating;

        int fps;
        std::string ip;
        team_t team;
        int64_t timer_begin;
        int64_t timer_end;
        bool timer_paused;
        bool timer_finished;

        store_buffer_t stores;
        edict_t* store_ent;

        int key_states; // input actions that are currently active
    };

    class server_data_t
    {
    public:
        server_data_t()
        {
            level_state = LEVEL_STATE_FREEPLAY;
            time_added_mins = 0;
            replay_now_time_ms = INT64_MAX;
        }

        level_state_t level_state;    // freeplay, voting, or intermission
        int time_added_mins;

        std::string last_map1;
        std::string last_map2;
        std::string last_map3;

        std::unordered_set<std::string> fresh_times;   // username keys (lowercase)

        std::vector<replay_frame_t> replay_now_frames;
        std::string replay_now_username;
        int64_t replay_now_time_ms;
    };
}