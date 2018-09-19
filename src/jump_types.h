#pragma once

// Data types that only have dependencies on the base game engine headers
#include "q_shared.h"

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
        KEY_STATE_FORWARD = 1,
        KEY_STATE_BACK = 2,
        KEY_STATE_LEFT = 4,
        KEY_STATE_RIGHT = 8,
        KEY_STATE_JUMP = 16,
        KEY_STATE_CROUCH = 32
    } key_state_t;

    typedef struct
    {
        vec3_t pos;
        vec3_t angles;
        char key_states;
        char fps;
    } replay_frame_t;

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
}