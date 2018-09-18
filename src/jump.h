#pragma once

#include "g_local.h"

#define JUMP_STRING_VERSION "1.0slip"



namespace Jump
{


    void OpenMenu_Join(edict_t* ent);

    int CountPlayersOnTeam(team_t team);

    void JoinTeam(edict_t* ent, team_t team);
    void JoinTeamEasy(edict_t* ent, pmenuhnd_t* hnd);
    void JoinTeamHard(edict_t* ent, pmenuhnd_t* hnd);
    void JoinChaseCam(edict_t* ent, pmenuhnd_t* hnd);

    bool JumpClientCommand(edict_t* ent);

    void Cmd_Jump_Inven(edict_t* ent);
    void Cmd_Jump_Noclip(edict_t* ent);
    void Cmd_Jump_Test(edict_t* ent);
    void Cmd_Jump_Kill(edict_t* ent);
    void Cmd_Jump_Recall(edict_t* ent);
    void Cmd_Jump_Store(edict_t* ent);
    void Cmd_Jump_Reset(edict_t* ent);

    void AssignTeamSkin(edict_t* ent);

    edict_t* SelectJumpSpawnPoint();

    void ResetJumpTimer(edict_t* ent);

    void ClientBeginJump(edict_t* ent);
    void InitAsSpectator(edict_t* ent);

    void MoveClientToPosition(edict_t* ent, vec3_t origin, vec3_t angles);

    void SpawnForJumping(edict_t* ent);
    void InitClientForRespawn(edict_t* ent);
    void SpawnAtStorePosition(edict_t* ent, store_data_t data);
}