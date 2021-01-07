#pragma once

#include "g_local.h"

namespace Jump
{
    bool HandleJumpCommand(edict_t* client);

    void Cmd_Jump_Inven(edict_t* ent);
    void Cmd_Jump_Noclip(edict_t* ent);
    void Cmd_Jump_Test(edict_t* ent);
    void Cmd_Jump_Kill(edict_t* ent);
    void Cmd_Jump_Recall(edict_t* ent);
    void Cmd_Jump_Store(edict_t* ent);
    void Cmd_Jump_Reset(edict_t* ent);
    void Cmd_Jump_Replay(edict_t* ent);
    void Cmd_Jump_Void(edict_t* ent);
    void Cmd_Jump_Maptimes(edict_t* ent);
    void Cmd_Jump_Score(edict_t* ent);
    void Cmd_Jump_Score2(edict_t* ent);
    void Cmd_Jump_Playertimes(edict_t* ent);
    void Cmd_Jump_Playerscores(edict_t* ent);
}