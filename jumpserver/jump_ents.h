#pragma once

#pragma once

#include "g_local.h"

namespace Jump
{
    // Returns true if successfully spawns the desired ent, else false
    bool SpawnJumpEnt(edict_t* ent);

    // Spawn functions
    void SP_jumpbox_small(edict_t* ent);
    void SP_jumpbox_medium(edict_t* ent);
    void SP_jumpbox_large(edict_t* ent);

}