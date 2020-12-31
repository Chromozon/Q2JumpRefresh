#pragma once

// Data types that only have dependencies on the base game engine headers
#include "q_shared.h"
#include <stdint.h>
#include <vector>

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

    // TODO: the current frame of the replay buffer should be stored elsewhere
    typedef struct
    {
        replay_frame_t frames[MAX_REPLAY_FRAMES];
        int next_frame_index;
    } replay_buffer_t;

    typedef struct
    {
        int time_interval;
        vec3_t pos;
        vec3_t angles;
    } store_data_t;

    class StoreBuffer
    {
    public:
        StoreBuffer();
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
        client_data_t()
            : replay_buffer(), replay_buffer_spectating()
        {
            replay_buffer.reserve(10000);
        }

        std::vector<replay_frame_t> replay_buffer; // TODO: rename to replay_recording
        std::vector<replay_frame_t> replay_buffer_spectating; // TODO: rename to replay_spectating
    };
}