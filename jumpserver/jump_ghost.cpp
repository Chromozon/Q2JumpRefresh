
#include "jump_ghost.h"
#include "jump_utils.h"

namespace Jump
{
	static std::vector<replay_frame_t> ghost_replay;

	void GhostRunFrame()
	{
		edict_t* ghost = GhostInstance();
	}

	// Initialize the default values for the ghost
	void GhostInit()
	{
		edict_t* ghost = GhostInstance();
		ghost->svflags = SVF_PROJECTILE;
		ghost->movetype = MOVETYPE_NOCLIP;
		ghost->clipmask = MASK_SOLID;
		ghost->solid = SOLID_NOT;
		VectorClear(ghost->mins);
		VectorClear(ghost->maxs);
		VectorClear(ghost->s.angles);
		VectorClear(ghost->s.old_origin);
		VectorClear(ghost->s.origin);
		ghost->dmg = 0;
		ghost->classname = "ghost";

		ghost->s.modelindex = gi.modelindex(va("%s/players/ghost/penguin.md2", GetModDir().c_str()));
		ghost->s.modelindex2 = 0;
		ghost->s.modelindex3 = 0;
		ghost->s.modelindex4 = 0;
		ghost->s.skinnum = 0;
		ghost->s.frame = 0;
	}

	// Get the ghost singleton instance
	edict_t* GhostInstance()
	{
		static edict_t* ghost = NULL;
		if (ghost == NULL)
		{
			ghost = G_Spawn();
		}
		return ghost;
	}
}