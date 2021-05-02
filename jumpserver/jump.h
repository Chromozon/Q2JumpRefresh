#pragma once

#include "g_local.h"
#include <unordered_map>

#define JUMP_VERSION_STRING "1.0slip"

#define STORE_MODEL "models/monsters/commandr/head/tris.md2"

namespace Jump
{
    extern server_data_t jump_server;

    void OpenMenu_Join(edict_t* ent);

    int CountPlayersOnTeam(TeamEnum team);

    void UpdateUserId(edict_t* ent);
    void JoinTeamEasyCommand(edict_t* ent, pmenuhnd_t* hnd);
    void JoinTeamHardCommand(edict_t* ent, pmenuhnd_t* hnd);
    void JoinChaseCamCommand(edict_t* ent, pmenuhnd_t* hnd);

    qboolean PickupWeapon(edict_t* weap, edict_t* ent);

    void SaveReplayFrame(edict_t* ent);
    void ClearReplayData(edict_t* ent);

    void JumpClientConnect(edict_t* ent);
    void JumpClientDisconnect(edict_t* ent);
    void JumpInitGame();
    void JumpRunFrame();

    void AdvanceSpectatingReplayFrame(edict_t* ent);

    void HandleMapCompletion(edict_t* ent);

    void AdvanceRacingSpark(edict_t* ent);

    void InitializeClientEnt(edict_t* ent);
}