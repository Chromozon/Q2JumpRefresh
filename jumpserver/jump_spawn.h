#pragma once

#include "g_local.h"
#include <string>

namespace Jump
{

class Spawn
{
public:
    static const int MaxHealthAndAmmo;

    static edict_t* SelectPlayerSpawn();
    static edict_t* SelectIntermissionSpawn();
    static void ClientOnEnterMap(edict_t* ent);

private:
    static std::string GetSkin(const std::string& username, team_t team);
    static std::string GetSkinEasy(const std::string& username);
    static std::string GetSkinHard(const std::string& username);
    static std::string GetSkinInvis(const std::string& username);
    static void AssignTeamSkin(edict_t* ent);

    static void MovePlayerToSpawn(edict_t* ent, edict_t* spawn, bool useTeleportEffects);
    static void MovePlayerToIntermission(edict_t* ent);
    static void InitializeClientEnt(edict_t* ent);
};

}