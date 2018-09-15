#pragma once

#include "g_local.h"

#define JUMP_STRING_VERSION "1.0slip"

#define MAX_STORES 5

namespace Jump
{
    class StoreBuffer
    {
    public:
        StoreBuffer();
        void PushStore(const store_data_t& data);
        store_data_t GetStore(int prevNum);
        bool HasStore();
    private:
        int numStores;
        int nextIndex;
        store_data_t stores[MAX_STORES];
    };

    void OpenMenu_Join(edict_t* ent);

    int CountPlayersEasy();
    int CountPlayersHard();

    void JoinTeam(edict_t* ent, team_t team);
    void JoinTeamEasy(edict_t* ent, pmenuhnd_t* hnd);
    void JoinTeamHard(edict_t* ent, pmenuhnd_t* hnd);
    void JoinChaseCam(edict_t* ent, pmenuhnd_t* hnd);

    bool JumpClientCommand(edict_t* ent);

    void Cmd_Jump_Inven(edict_t* ent);
    void Cmd_Jump_Noclip(edict_t* ent);
    void Cmd_Jump_Test(edict_t* ent);

    void AssignTeamSkin(edict_t* ent);

    edict_t* SelectJumpSpawnPoint();

    void ResetJumpTimer(edict_t* ent);
}