#include "jump_ents.h"
#include "jump_utils.h"

namespace Jump
{
	typedef void (*SpawnFunction)(edict_t* ent);

	static std::unordered_map<std::string, SpawnFunction> SpawnTable = {
		{ "jumpbox_small", SP_jumpbox_small },
		{ "jumpbox_medium", SP_jumpbox_medium },
		{ "jumpbox_large", SP_jumpbox_large },
	};

	bool SpawnJumpEnt(edict_t* ent)
	{
		std::string name = ent->classname;
		name = AsciiToLower(name);
		const auto spawnPair = SpawnTable.find(name);
		if (spawnPair != SpawnTable.end())
		{
			SpawnFunction spawnFunction = spawnPair->second;
			spawnFunction(ent);
			return true;
		}
		return false;
	}

	void SP_jumpbox_small(edict_t* ent)
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

	void SP_jumpbox_medium(edict_t* ent)
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

	void SP_jumpbox_large(edict_t* ent)
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
}
