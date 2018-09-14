#pragma once

#include "g_local.h"

#define JUMP_STRING_VERSION "1.0slip"

namespace Jump
{
    void OpenMenu_Join(edict_t* ent);

    int CountPlayersEasy();
    int CountPlayersHard();

    void JoinTeam(edict_t* ent, team_t team);
    void JoinTeamEasy(edict_t* ent, pmenuhnd_t* hnd);
    void JoinTeamHard(edict_t* ent, pmenuhnd_t* hnd);
    void JoinChaseCam(edict_t* ent, pmenuhnd_t* hnd);

    bool JumpClientCommand(edict_t* ent);

    void Cmd_Jump_Inven(edict_t* ent);

    char* TeamNameStr(team_t team);
}