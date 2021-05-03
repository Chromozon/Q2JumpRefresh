#include "jump_ents.h"
#include "jump_utils.h"

namespace Jump
{

/// <summary>
/// Variables
/// </summary>
std::map<std::string, Entities::SpawnFunction> Entities::_spawnTable = {
	{ "jumpbox_small", SP_jumpbox_small },
	{ "jumpbox_medium", SP_jumpbox_medium },
	{ "jumpbox_large", SP_jumpbox_large },
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

} // namespace Jump
