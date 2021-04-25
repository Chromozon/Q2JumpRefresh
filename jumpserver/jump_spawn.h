#pragma once

#include "g_local.h"
#include <string>

namespace Jump
{

class Spawn
{
public:
    static const int MaxHealthAndAmmo;

    static void ClientOnEnterMap(edict_t* ent);

    static void JoinTeamEasy(edict_t* ent);
    static void JoinTeamHard(edict_t* ent);
    static void JoinTeamSpectator(edict_t* ent);
    static void PlayerRespawn(edict_t* ent, int storeNum = 0);

private:
    static std::string GetSkin(const std::string& username, team_t team);
    static std::string GetSkinEasy(const std::string& username);
    static std::string GetSkinHard(const std::string& username);
    static std::string GetSkinInvis(const std::string& username);
    static void AssignTeamSkin(edict_t* ent);

    static void InitDefaultSpawnVariables(edict_t* ent);
    static edict_t* SelectPlayerSpawn();
    static edict_t* SelectIntermissionSpawn();
    static void InitAsSpectator(edict_t* ent);
    static void MovePlayerToSpawn(edict_t* ent, edict_t* spawn, bool useTeleportEffects);
    static void MovePlayerToPosition(edict_t* ent, const vec3_t& origin, const vec3_t& angles, bool useTeleportEffects);
    static void MovePlayerToIntermission(edict_t* ent);
    static void InitializeClientEnt(edict_t* ent);
};

}