#pragma once

// Data types that only have dependencies on the base game engine headers
#include "q_shared.h"

namespace Jump
{
    typedef enum
    {
        TEAM_NONE,
        TEAM_EASY,
        TEAM_HARD
    } team_t;

    typedef struct
    {
        int time;
        vec3_t pos;
        vec3_t angles;
    } store_data_t;
}