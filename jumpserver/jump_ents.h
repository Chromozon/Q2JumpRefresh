#pragma once

#pragma once

#include "g_local.h"
#include <map>
#include <string>

namespace Jump
{

class Entities
{
public:
    // Returns true if successfully spawns the desired ent, else false
    static bool SpawnJumpEnt(edict_t* ent);

private:
    // Spawn functions
    static void SP_jumpbox_small(edict_t* ent);
    static void SP_jumpbox_medium(edict_t* ent);
    static void SP_jumpbox_large(edict_t* ent);
    static void SP_trigger_finish(edict_t* ent);
    static void SP_weapon_finish(edict_t* ent);

    // Touch functions
    static void TouchTriggerFinish(edict_t* self, edict_t* other, cplane_t* /*plane*/, csurface_t* /*surf*/);
    static void TouchWeaponFinish(edict_t* self, edict_t* other, cplane_t* /*plane*/, csurface_t* /*surf*/);

    // Spawn table
    typedef void (*SpawnFunction)(edict_t* ent);
    static std::map<std::string, SpawnFunction> _spawnTable;
};

}