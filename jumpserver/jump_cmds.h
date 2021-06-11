#pragma once

#include "g_local.h"
#include "jump_global.h"

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
    void Cmd_Jump_Playermaps(edict_t* ent);
    void Cmd_Jump_Seen(edict_t* ent);
    void Cmd_Jump_PlayertimesGlobal(edict_t* ent);
    void Cmd_Jump_PlayerscoresGlobal(edict_t* ent);
    void Cmd_Jump_PlayermapsGlobal(edict_t* ent);
    void Cmd_Jump_MaptimesGlobal(edict_t* ent);
    void Cmd_Jump_Race(edict_t* ent);
    void Cmd_Jump_Mapsleft(edict_t* ent);
    void Cmd_Jump_Mapsdone(edict_t* ent);
    void Cmd_Jump_Jumpers(edict_t* ent);
    void Cmd_Jump_Maplist(edict_t* ent);
    void Cmd_Jump_MaplistNew(edict_t* ent);
    void Cmd_Jump_MSet(edict_t* ent);
    void Cmd_Jump_MSetList(edict_t* ent);
    void Cmd_Jump_Team(edict_t* ent);

    // Global database cmd responses
    void HandleGlobalCmdResponse(const global_cmd_response& response);
    void HandleGlobalPlayertimesResponse(const global_cmd_response& response);
    void HandleGlobalPlayerscoresResponse(const global_cmd_response& response);
    void HandleGlobalPlayermapsResponse(const global_cmd_response& response);
    void HandleGlobalMaptimesResponse(const global_cmd_response& response);
    
    void Cmd_Jump_Vote_Time(edict_t* ent);
    void Cmd_Jump_Vote_Nominate(edict_t* ent);
    void Cmd_Jump_Vote_ChangeMap(edict_t* ent);
    void Cmd_Jump_Vote_Silence(edict_t* ent);
    void Cmd_Jump_Vote_Kick(edict_t* ent);
#ifdef _DEBUG
    void Cmd_Jump_Vote_MapEndVote(edict_t* ent);
#endif // _DEBUG
    void Cmd_Jump_Vote_CastYes(edict_t* ent);
    void Cmd_Jump_Vote_CastNo(edict_t* ent);
}