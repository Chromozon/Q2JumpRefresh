#pragma once

#include "g_local.h"

#define JUMP_STRING_VERSION "1.0slip"

namespace Jump
{
    void OpenMenu_Join(edict_t* ent);

    int CountPlayersEasy();
    int CountPlayersHard();

    void JoinTeamEasy(edict_t* ent, pmenuhnd_t* hnd);
    void JoinTeamHard(edict_t* ent, pmenuhnd_t* hnd);
    void JoinChaseCam(edict_t* ent, pmenuhnd_t* hnd);
}