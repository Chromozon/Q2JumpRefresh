#pragma once

// Data types that only have dependencies on the base game engine headers
#include "q_shared.h"

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
        STATE_FREEPLAY,
        STATE_VOTING,
        STATE_INTERMISSION
    } level_state_t;

    typedef struct
    {
        int time;
        vec3_t pos;
        vec3_t angles;
    } store_data_t;
}