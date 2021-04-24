#pragma once

#include "g_local.h"
#include <unordered_map>

#define JUMP_VERSION_STRING "1.0slip"

#define STORE_MODEL "models/monsters/commandr/head/tris.md2"

namespace Jump
{
    extern server_data_t jump_server;

    void OpenMenu_Join(edict_t* ent);

    int CountPlayersOnTeam(team_t team);

    void UpdateUserId(edict_t* ent);
    void JoinTeam(edict_t* ent, team_t team);
    void JoinTeamEasy(edict_t* ent, pmenuhnd_t* hnd);
    void JoinTeamHard(edict_t* ent, pmenuhnd_t* hnd);
    void JoinChaseCam(edict_t* ent, pmenuhnd_t* hnd);

    std::string GetSkin(const std::string& username, team_t team);
    std::string GetSkinEasy(const std::string& username);
    std::string GetSkinHard(const std::string& username);
    std::string GetSkinInvis(const std::string& username);
    void AssignTeamSkin(edict_t* ent);

    edict_t* SelectJumpSpawnPoint();

    void ClientOnEnterMap(edict_t* ent);
    void InitAsSpectator(edict_t* ent);

    void MoveClientToPosition(edict_t* ent, vec3_t origin, vec3_t angles);

    void SpawnForJumping(edict_t* ent);
    void InitClientForRespawn(edict_t* ent);
    void SpawnAtStorePosition(edict_t* ent, store_data_t data);

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
}