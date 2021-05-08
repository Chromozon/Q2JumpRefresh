#include "jump_ents.h"
#include "jump_utils.h"
#include "jump_msets.h"

namespace Jump
{

/// <summary>
/// Variables
/// </summary>
std::map<std::string, Entities::SpawnFunction> Entities::_spawnTable = {
    { "jumpbox_small", SP_jumpbox_small },
    { "jumpbox_medium", SP_jumpbox_medium },
    { "jumpbox_large", SP_jumpbox_large },
    { "cpbox_small", SP_cpbox_small },
    { "cpbox_medium", SP_cpbox_medium },
    { "cpbox_large", SP_cpbox_large },
    { "trigger_finish", SP_trigger_finish },
    { "weapon_finish", SP_weapon_finish },
};

/// <summary>
/// Calls the spawn function for the ent if it exists.
/// </summary>
/// <param name="ent"></param>
/// <returns>True if spawned, false if not.</returns>
bool Entities::SpawnJumpEnt(edict_t* ent)
{
    std::string name = ent->classname;
    name = AsciiToLower(name);
    const auto spawnPair = _spawnTable.find(name);
    if (spawnPair != _spawnTable.end())
    {
        SpawnFunction spawnFunction = spawnPair->second;
        spawnFunction(ent);
        return true;
    }
    return false;
}

/// <summary>
/// General checkpoint touch function that can be used for all checkpoints (keys, items, cpboxes).
/// </summary>
/// <param name="self">Checkpoint ent</param>
/// <param name="other">Player</param>
void Entities::TouchCheckpoint(edict_t* self, edict_t* other)
{
    TouchCpBox(self, other, nullptr, nullptr);
}

/// <summary>
/// Small box.
/// </summary>
/// <param name="ent"></param>
void Entities::SP_jumpbox_small(edict_t* ent)
{
    ent->classname = "jumpbox_small";
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    ent->s.renderfx |= RF_TRANSLUCENT;
    VectorSet(ent->mins, -16, -16, -16);
    VectorSet(ent->maxs, 16, 16, 16);
    ent->s.modelindex = gi.modelindex("models/jump/smallbox3/tris.md2");
    gi.linkentity(ent);
}

/// <summary>
/// Medium box.
/// </summary>
/// <param name="ent"></param>
void Entities::SP_jumpbox_medium(edict_t* ent)
{
    ent->classname = "jumpbox_medium";
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    ent->s.renderfx |= RF_TRANSLUCENT;
    VectorSet(ent->mins, -32, -32, -16);
    VectorSet(ent->maxs, 32, 32, 48);
    ent->s.modelindex = gi.modelindex("models/jump/mediumbox3/tris.md2");
    gi.linkentity(ent);
}

/// <summary>
/// Large box.
/// </summary>
/// <param name="ent"></param>
void Entities::SP_jumpbox_large(edict_t* ent)
{
    ent->classname = "jumpbox_large";
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    ent->s.renderfx |= RF_TRANSLUCENT;
    VectorSet(ent->mins, -64, -64, -32);
    VectorSet(ent->maxs, 64, 64, 96);
    ent->s.modelindex = gi.modelindex("models/jump/largebox3/tris.md2");
    gi.linkentity(ent);
}

/// <summary>
/// Small checkpoint box.
/// </summary>
/// <param name="ent"></param>
void Entities::SP_cpbox_small(edict_t* ent)
{
    ent->classname = "cpbox_small";
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_TRIGGER;
    ent->s.renderfx |= RF_TRANSLUCENT;
    VectorSet(ent->mins, -16, -16, -16);
    VectorSet(ent->maxs, 16, 16, 16);
    ent->s.modelindex = gi.modelindex("models/jump/smallbox3/tris.md2");
    ent->touch = TouchCpBox;
    gi.linkentity(ent);
}

/// <summary>
/// Medium checkpoint box.
/// </summary>
/// <param name="ent"></param>
void Entities::SP_cpbox_medium(edict_t* ent)
{
    ent->classname = "cpbox_medium";
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_TRIGGER;
    ent->s.renderfx |= RF_TRANSLUCENT;
    VectorSet(ent->mins, -32, -32, -16);
    VectorSet(ent->maxs, 32, 32, 48);
    ent->s.modelindex = gi.modelindex("models/jump/mediumbox3/tris.md2");
    ent->touch = TouchCpBox;
    gi.linkentity(ent);
}

/// <summary>
/// Large checkpoint box.
/// </summary>
/// <param name="ent"></param>
void Entities::SP_cpbox_large(edict_t* ent)
{
    ent->classname = "cpbox_large";
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_TRIGGER;
    ent->s.renderfx |= RF_TRANSLUCENT;
    VectorSet(ent->mins, -64, -64, -32);
    VectorSet(ent->maxs, 64, 64, 96);
    ent->s.modelindex = gi.modelindex("models/jump/largebox3/tris.md2");
    ent->touch = TouchCpBox;
    gi.linkentity(ent);
}

/// <summary>
/// Area ent to complete the map instead of picking up a weapon.  Use this instead of weapon_finish.
/// </summary>
/// <param name="ent"></param>
void Entities::SP_trigger_finish(edict_t* ent)
{
    ent->classname = "trigger_finish";
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_TRIGGER;
    ent->touch = TouchTriggerFinish;
    ent->svflags |= SVF_NOCLIENT;
    gi.setmodel(ent, ent->model);
    gi.linkentity(ent);
}

/// <summary>
/// Deprecated, but there are still enough maps that use this.  Use trigger_finish instead.
/// </summary>
/// <param name="ent"></param>
void Entities::SP_weapon_finish(edict_t* ent)
{
    ent->classname = "weapon_finish";
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_TRIGGER;
    ent->touch = TouchWeaponFinish;
    ent->svflags |= SVF_NOCLIENT;
    gi.setmodel(ent, ent->model);
    gi.linkentity(ent);
}

/// <summary>
/// Finish the map when the user touches this ent.
/// </summary>
/// <param name="self"></param>
/// <param name="other"></param>
/// <param name=""></param>
/// <param name=""></param>
void Entities::TouchTriggerFinish(edict_t* self, edict_t* other, cplane_t* /*plane*/, csurface_t* /*surf*/)
{
    PickupWeapon(nullptr, other);
}

/// <summary>
/// Finish the map when the user touches this ent.
/// </summary>
/// <param name="self"></param>
/// <param name="other"></param>
/// <param name=""></param>
/// <param name=""></param>
void Entities::TouchWeaponFinish(edict_t* self, edict_t* other, cplane_t* /*plane*/, csurface_t* /*surf*/)
{
    PickupWeapon(nullptr, other);
}

/// <summary>
/// Increment checkpoint counter when the user touches a checkpoint box.
/// </summary>
void Entities::TouchCpBox(edict_t* self, edict_t* other, cplane_t* /*plane*/, csurface_t* /*surf*/)
{
    // TODO: apparently we allow weapons to hit CP boxes and count too, so it'll be other->owner for the player

    if (MSets::GetCheckpointTotal() == 0)
    {
        // No checkpoints set for this map
        return;
    }
    if (other->client == nullptr || !other->inuse)
    {
        // Not a client
        return;
    }
    if ((other->client->jumpdata->team != TeamEnum::Easy) && (other->client->jumpdata->team != TeamEnum::Hard))
    {
        // Spectators cannot pick up checkpoints
        return;
    }
    if (std::find(other->client->jumpdata->checkpoints_obtained.begin(),
                  other->client->jumpdata->checkpoints_obtained.end(),
                  self)
        != other->client->jumpdata->checkpoints_obtained.end())
    {
        // Checkpoint already obtained
        return;
    }

    int64_t timeNowMs = Sys_Milliseconds();

    if (StringCompareInsensitive(self->target, "ordered"))
    {
        int checkpointNum = self->count;
        if (checkpointNum != (other->client->jumpdata->checkpoint_total + 1))
        {
            if ((other->client->jumpdata->timer_trigger_spam == 0) ||
                ((timeNowMs - other->client->jumpdata->timer_trigger_spam) > 5000))
            {
                gi.cprintf(other, PRINT_HIGH,
                    va("You must pick up this checkpoint in order. This is checkpoint %d.\n", checkpointNum));
                other->client->jumpdata->timer_trigger_spam = timeNowMs;
            }
            return;
        }
    }

    bool firstCheckpoint = other->client->jumpdata->checkpoint_total == 0;

    other->client->jumpdata->checkpoint_total++;
    other->client->jumpdata->checkpoints_obtained.push_back(self);
    
    int checkpointTotal = MSets::GetCheckpointTotal();
    int checkpointCurrent = other->client->jumpdata->checkpoint_total;
    std::string overallTimeStr = GetCompletionTimeDisplayString(timeNowMs - other->client->jumpdata->timer_begin);

    if (firstCheckpoint)
    {
        gi.cprintf(other, PRINT_HIGH, va("You reached checkpoint %d/%d in %s seconds.\n",
            checkpointCurrent, checkpointTotal, overallTimeStr.c_str()));
    }
    else
    {
        std::string splitTimeStr = GetCompletionTimeDisplayString(timeNowMs - other->client->jumpdata->timer_checkpoint_split);
        gi.cprintf(other, PRINT_HIGH, va("You reached checkpoint %d/%d in %s seconds. (split: %s)\n",
            checkpointCurrent, checkpointTotal, overallTimeStr.c_str(), splitTimeStr.c_str()));
    }

    other->client->jumpdata->timer_checkpoint_split = timeNowMs;
    // TODO: send CP text to spectators of this player
}

} // namespace Jump
