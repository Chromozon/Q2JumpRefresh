
#include "jump_ghost.h"
#include "jump_utils.h"
#include "jump_scores.h"
#include <cassert>
#include <fstream>
#include <filesystem>

namespace Jump
{

/// <summary>
/// Define the private variables
/// </summary>
std::vector<replay_frame_t> GhostReplay::_replay;
size_t GhostReplay::_replayFrame = 0;
edict_t* GhostReplay::_ghost = nullptr;
std::vector<std::string> GhostReplay::_ghostModels;

/// <summary>
/// Creates a new entity for the ghost.  Does not load the replay data.
/// </summary>
void GhostReplay::Init()
{
	_ghost = G_Spawn();
	_ghost->svflags = SVF_NOCLIENT;	// hides the ent
	_ghost->movetype = MOVETYPE_NOCLIP;
	_ghost->clipmask = MASK_SOLID;
	_ghost->solid = SOLID_NOT;
	VectorClear(_ghost->mins);
	VectorClear(_ghost->maxs);
	VectorClear(_ghost->s.angles);
	VectorClear(_ghost->s.old_origin);
	VectorClear(_ghost->s.origin);
	_ghost->dmg = 0;
	_ghost->classname = "ghost";

	std::string model = PickRandomGhostModel();
	std::string modelPath = std::string("players/ghost/") + model + ".md2";

	_ghost->s.modelindex = gi.modelindex(const_cast<char*>(modelPath.c_str()));
	_ghost->s.modelindex2 = 0;
	_ghost->s.modelindex3 = 0;
	_ghost->s.modelindex4 = 0;
	_ghost->s.skinnum = 0;
	_ghost->s.frame = 0;
	gi.unlinkentity(_ghost);

	_replay.clear();
	_replayFrame = 0;
}

/// <summary>
/// Loads the top time replay data for the ghost.
/// </summary>
void GhostReplay::LoadReplay()
{
	int timeMs = 0;
	std::string username;
	LocalDatabase::GetReplayByPosition(level.mapname, 1, _replay, timeMs, username);
	_replayFrame = 0;
}

/// <summary>
/// Advances the ghost position by one frame.
/// </summary>
void GhostReplay::RunFrame()
{
	assert(_ghost != NULL);
	if (_replay.empty())
	{
		gi.unlinkentity(_ghost);
		return;
	}

	if (_replayFrame >= _replay.size())
	{
		_replayFrame = 0;
	}

	VectorCopy(_replay[_replayFrame].pos, _ghost->s.origin);
	VectorCopy(_replay[_replayFrame].pos, _ghost->s.old_origin);
	VectorCopy(_replay[_replayFrame].angles, _ghost->s.angles);
	_ghost->s.frame = _replay[_replayFrame].animation_frame;
	_ghost->svflags = SVF_PROJECTILE;
	gi.linkentity(_ghost);

	_replayFrame++;
}

/// <summary>
/// Load all the ghost models from the jump/27910/ghost_models.cfg file.
/// </summary>
void GhostReplay::LoadGhostModels()
{
	_ghostModels.clear();
	std::string filename = GetModPortDir() + "/ghost_models.cfg";
	std::ifstream file(filename);
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			std::string modelPath = GetModDir() + "/players/ghost/" + line + ".md2";
			if (std::filesystem::exists(modelPath))
			{
				_ghostModels.push_back(line);
			}
		}
	}
}

/// <summary>
/// Chooses a random ghost model from the full loaded list.
/// </summary>
/// <returns></returns>
std::string GhostReplay::PickRandomGhostModel()
{
	std::string model = "penguin"; // default if nothing is loaded
	if (_ghostModels.size() > 0)
	{
		int index = ::rand() % _ghostModels.size();
		model = _ghostModels[index];
	}
	return model;
}

}