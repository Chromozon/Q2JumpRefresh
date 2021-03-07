
#include "jump_ghost.h"
#include "jump_utils.h"
#include "jump_scores.h"
#include <cassert>

namespace Jump
{
	static std::vector<replay_frame_t> ghost_replay = {};
	static size_t ghost_replay_frame = 0;
	static bool change_replay = false;
	static edict_t* ghost = NULL;

	// Tell the ghost to reload the current replay
	void GhostChangeReplay()
	{
		change_replay = true;
	}

	// Advance the ghost by one frame
	void GhostRunFrame()
	{
		assert(ghost != NULL);
		if (ghost_replay.empty() || change_replay)
		{
			// Try to load an available replay
			// TODO: optimize this so that it doesn't run every frame if there is no replay
			std::vector<user_time_record> highscores = {};
			int players = 0;
			int completions = 0;
			if (GetHighscoresForMap(level.mapname, highscores, players, completions))
			{
				if (!highscores.empty())
				{
					LoadReplayFromFile(level.mapname, highscores[0].username_key, ghost_replay);
					ghost_replay_frame = 0;
					change_replay = false;
				}
			}

			if (ghost_replay.empty())
			{
				gi.unlinkentity(ghost);
				return;
			}
		}

		if (ghost_replay_frame >= ghost_replay.size())
		{
			ghost_replay_frame = 0;
		}

		VectorCopy(ghost_replay[ghost_replay_frame].pos, ghost->s.origin);
		VectorCopy(ghost_replay[ghost_replay_frame].pos, ghost->s.old_origin);
		VectorCopy(ghost_replay[ghost_replay_frame].angles, ghost->s.angles);
		ghost->s.frame = ghost_replay[ghost_replay_frame].animation_frame;
		ghost->svflags = SVF_PROJECTILE;
		gi.linkentity(ghost);

		ghost_replay_frame++;
	}

	// Initialize the default values for the ghost
	void GhostInit()
	{
		ghost = G_Spawn();
		ghost->svflags = SVF_NOCLIENT;	// hides the ent
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

		ghost->s.modelindex = gi.modelindex("players/ghost/penguin.md2");
		ghost->s.modelindex2 = 0;
		ghost->s.modelindex3 = 0;
		ghost->s.modelindex4 = 0;
		ghost->s.skinnum = 0;
		ghost->s.frame = 0;
		gi.unlinkentity(ghost);
	}
}