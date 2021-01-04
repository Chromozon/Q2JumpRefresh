#include "jump.h"
#include "jump_logger.h"
#include "jump_scores.h"
#include <unordered_map>
#include "jump_utils.h"

namespace Jump
{
    server_data_t jump_server;

    static const int DefaultHealth = 1000;

    static const char* SexTeamEasy = "female";
    static const char* SexTeamHard = "male";
    static const char* SkinTeamEasy = "ctf_r";
    static const char* SkinTeamHard = "ctf_b";

    static const int MaxMenuWidth = 32; // characters
    static const int MapNameLine = 2;
    static const int JoinEasyLine = 5;
    static const int NumPlayersEasyLine = 6;
    static const int NumPlayersHardLine = 8;
    static const int ChaseCamLine = 9;

    static pmenu_t Menu_Join[] = {
        { "*Quake II Jump",               PMENU_ALIGN_CENTER, NULL },
        { NULL,                           PMENU_ALIGN_CENTER, NULL },
        { NULL /* mapname */,             PMENU_ALIGN_CENTER, NULL },
        { NULL,                           PMENU_ALIGN_CENTER, NULL },
        { NULL,                           PMENU_ALIGN_CENTER, NULL },
        { "Join Easy Team",               PMENU_ALIGN_LEFT,   JoinTeamEasy },
        { NULL /* number of players */,   PMENU_ALIGN_LEFT,   NULL },
        { "Join Hard Team",               PMENU_ALIGN_LEFT,   JoinTeamHard },
        { NULL /* number of players */,   PMENU_ALIGN_LEFT,   NULL },
        { NULL /* chase camera */,        PMENU_ALIGN_LEFT,   JoinChaseCam },
        { NULL,                           PMENU_ALIGN_LEFT,   NULL }, // TODO help commands menu
        { NULL,                           PMENU_ALIGN_LEFT,   NULL },
        { "Highlight your choice and",    PMENU_ALIGN_LEFT,   NULL },
        { "press ENTER.",                 PMENU_ALIGN_LEFT,   NULL },
        { "v" JUMP_STRING_VERSION,        PMENU_ALIGN_RIGHT,  NULL },
    };


    // ent must be a client
    // No other menus can be open when this function is called
    void OpenMenu_Join(edict_t* ent)
    {
        char mapName[MaxMenuWidth] = { 0 };
        char playersEasy[MaxMenuWidth] = { 0 };
        char playersHard[MaxMenuWidth] = { 0 };
        char chaseCam[MaxMenuWidth] = { 0 };

        strncpy(mapName, level.mapname, MaxMenuWidth - 1);
        sprintf(playersEasy, "  (%d players)", CountPlayersOnTeam(TEAM_EASY));
        sprintf(playersHard, "  (%d players)", CountPlayersOnTeam(TEAM_HARD));

        Menu_Join[MapNameLine].text = mapName;
        Menu_Join[NumPlayersEasyLine].text = playersEasy;
        Menu_Join[NumPlayersHardLine].text = playersHard;
        if (ent->client->chase_target)
        {
            Menu_Join[ChaseCamLine].text = "Leave Chase Camera";
        }
        else
        {
            Menu_Join[ChaseCamLine].text = "Chase Camera";
        }

        int cursor = JoinEasyLine;
        PMenu_Open(ent, Menu_Join, cursor, sizeof(Menu_Join) / sizeof(pmenu_t), NULL);
    }

    int CountPlayersOnTeam(team_t team)
    {
        int count = 0;
        for (int i = 0; i < game.maxclients; ++i)
        {
            const gclient_t* client = &game.clients[i];
            if (client != NULL && client->pers.connected && client->jumpdata != NULL && client->jumpdata->team == team)
            {
                count++;
            }
        }
        return count;
    }

    void JoinTeam(edict_t* ent, team_t team)
    {
        ent->client->jumpdata->team = team;
        AssignTeamSkin(ent);
        SpawnForJumping(ent);
    }

    void JoinTeamEasy(edict_t* ent, pmenuhnd_t* hnd)
    {
        PMenu_Close(ent);
        JoinTeam(ent, TEAM_EASY);
    }

    void JoinTeamHard(edict_t* ent, pmenuhnd_t* hnd)
    {
        PMenu_Close(ent);
        JoinTeam(ent, TEAM_HARD);
    }

    void JoinChaseCam(edict_t* ent, pmenuhnd_t* hnd) {}

    void AssignTeamSkin(edict_t* ent)
    {
        // TODO: doesn't seem to be working with the model
        int playernum = ent - g_edicts - 1;

        switch (ent->client->jumpdata->team)
        {
        case TEAM_EASY:
            gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s/%s", ent->client->pers.netname, SexTeamEasy, SkinTeamEasy));
            break;
        case TEAM_HARD:
            gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s/%s", ent->client->pers.netname, SexTeamHard, SkinTeamHard));
            break;
        default:
            gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\female/invis", ent->client->pers.netname));
            break;
        }
    }

    edict_t* SelectJumpSpawnPoint()
    {
        edict_t* spot = G_Find(NULL, FOFS(classname), "info_player_start");
        if (spot != NULL)
        {
            return spot;
        }
        spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
        if (spot != NULL)
        {
            return spot;
        }
        return NULL;
    }

    // This is called once whenever a player first enters a map.
    // We want to initialize everything to a clean state,
    // make the player a spectator, and open the join team menu.
    void ClientBeginJump(edict_t* ent)
    {
        G_InitEdict(ent);
        memset(&ent->client->ps, 0, sizeof(ent->client->ps));
        InitClientResp(ent->client);

        char userinfo[MAX_INFO_STRING];
        memcpy(userinfo, ent->client->pers.userinfo, sizeof(userinfo));
        InitClientPersistant(ent->client);

        FetchClientEntData(ent);

        ent->groundentity = NULL;
        ent->takedamage = DAMAGE_AIM;
        ent->viewheight = 22;
        ent->inuse = true;
        ent->classname = "player";
        ent->mass = 200;
        ent->deadflag = DEAD_NO;
        ent->air_finished = level.time + 12; // TODO: this is water air time, need this?
        ent->clipmask = MASK_PLAYERSOLID;
        ent->pain = player_pain;
        ent->die = player_die;
        ent->waterlevel = 0;
        ent->watertype = 0;
        ent->flags &= ~FL_NO_KNOCKBACK;
        ent->svflags &= ~SVF_DEADMONSTER;

        vec3_t mins = { -16, -16, -24 };
        vec3_t maxs = { 16, 16, 32 };
        VectorCopy(mins, ent->mins);
        VectorCopy(maxs, ent->maxs);
        VectorClear(ent->velocity);

        ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

        // clear entity state values
        ent->s.effects = 0;
        ent->s.skinnum = ent - g_edicts - 1;
        ent->s.modelindex = 255;        // will use the skin specified model
        ent->s.modelindex2 = 255;       // custom gun model
        ent->s.skinnum = ent - g_edicts - 1;
        ent->s.frame = 0;

        InitAsSpectator(ent);
        ClientUserinfoChanged(ent, userinfo);

        if (level.intermissiontime || jump_server.level_state == LEVEL_STATE_VOTING)
        {
            // TODO move to intermission
            MoveClientToIntermission(ent);
        }
        else
        {
            vec3_t spawn_origin = { 0 };
            vec3_t spawn_angles = { 0 };
            SelectSpawnPoint(ent, spawn_origin, spawn_angles);
            MoveClientToPosition(ent, spawn_origin, spawn_angles);
            
            // Send updates to the client before opening the menu
            gi.linkentity(ent);

            OpenMenu_Join(ent);
        }

        gi.bprintf(PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

        // make sure all view stuff is valid
        ClientEndServerFrame(ent);
    }

    void InitAsSpectator(edict_t* ent)
    {
        ent->movetype = MOVETYPE_NOCLIP;
        ent->solid = SOLID_NOT;
        ent->svflags |= SVF_NOCLIENT;
        ent->client->resp.ctf_team = CTF_NOTEAM;
        ent->client->ps.gunindex = 0;
        ent->client->jumpdata->team = TEAM_SPECTATOR;
    }

    void MoveClientToPosition(edict_t* ent, vec3_t origin, vec3_t angles)
    {
        // Stop all previous movement
        ent->waterlevel = 0;
        ent->watertype = 0;
        VectorClear(ent->velocity);

        ent->client->ps.pmove.origin[0] = origin[0] * 8;
        ent->client->ps.pmove.origin[1] = origin[1] * 8;
        ent->client->ps.pmove.origin[2] = origin[2] * 8;

        VectorCopy(origin, ent->s.origin);
        ent->s.origin[2] += 1;	// make sure off ground
        VectorCopy(ent->s.origin, ent->s.old_origin);

        ent->s.angles[PITCH] = 0;
        ent->s.angles[YAW] = angles[YAW];
        ent->s.angles[ROLL] = 0;
        VectorCopy(ent->s.angles, ent->client->ps.viewangles);
        VectorCopy(ent->s.angles, ent->client->v_angle);
        VectorClear(ent->client->ps.viewoffset);

        for (int i = 0; i < 3; i++)
        {
            ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->s.angles[i] - ent->client->resp.cmd_angles[i]);
        }
    }

    void SpawnForJumping(edict_t* ent)
    {
        // Remove spectator flags
        ent->movetype = MOVETYPE_WALK;
        ent->solid = SOLID_BBOX;
        ent->flags = 0;
        ent->svflags = 0;
        ent->deadflag = DEAD_NO;

        // Clear inventory of all weapons and ammo and change to blaster
        // TODO: simplify the weapon change code
        char userinfo[MAX_INFO_STRING] = { 0 };
        memcpy(userinfo, ent->client->pers.userinfo, sizeof(userinfo));
        InitClientPersistant(ent->client); // TODO: don't like this, should just set all the vars
        ClientUserinfoChanged(ent, userinfo);

        InitClientForRespawn(ent);

        // Move to spawn
        vec3_t spawn_origin = { 0 };
        vec3_t spawn_angles = { 0 };
        SelectSpawnPoint(ent, spawn_origin, spawn_angles);
        MoveClientToPosition(ent, spawn_origin, spawn_angles);

        // Reset timer
        ent->client->jumpdata->timer_begin = 0;
        ent->client->jumpdata->timer_end = 0;
        ent->client->jumpdata->timer_paused = true;
        ent->client->jumpdata->timer_finished = false;

        // Clear replay
        ClearReplayData(ent);

        // Clear any special move effects
        ent->s.event = EV_NONE;
        ent->client->ps.pmove.pm_flags = 0;
        ent->client->ps.pmove.pm_time = 0;

        gi.linkentity(ent);
    }

    void InitClientForRespawn(edict_t* ent)
    {
        // Clear out the entire inventory
        memset(ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));

        // Unequip all weapons
        ent->client->pers.weapon = NULL;
        ent->client->pers.lastweapon = NULL;
        ent->client->pers.selected_item = 0;
        VectorClear(ent->client->ps.gunangles);
        VectorClear(ent->client->ps.gunoffset);
        ent->client->ps.gunframe = 0;
        ent->client->ps.gunindex = 0;

        ent->client->pers.health = DefaultHealth;
        ent->health = DefaultHealth;

        // Score is unused in Jump
        ent->client->resp.score = 0;
        ent->client->pers.score = 0;
    }

    void SpawnAtStorePosition(edict_t* ent, store_data_t data)
    {
        if (ent->client->jumpdata->team == TEAM_HARD) {
            // Should never get here!
            gi.error("Uh oh, tried to use a store on team hard.");
            return;
        }
        ent->client->jumpdata->timer_begin = Sys_Milliseconds() - data.time_interval;
        MoveClientToPosition(ent, data.pos, data.angles);
    }

    qboolean PickupWeapon(edict_t* weap, edict_t* ent)
    {
        if (ent->client->jumpdata->timer_finished)
        {
            // If we have already completed the map, ignore any weapon pickups
            return false;
            // TODO: make sure recall on easy mode resets timer_finished
        }

        // TODO
        // If rocket, grenade launcher, BFG
        // and mset enabled, don't finish timer

        if (!ent->client->jumpdata->timer_finished)
        {
            ent->client->jumpdata->timer_end = Sys_Milliseconds();
            int64_t time_diff = ent->client->jumpdata->timer_end - ent->client->jumpdata->timer_begin;
            if (ent->client->jumpdata->team == TEAM_EASY)
            {
                gi.cprintf(ent, PRINT_HIGH, "You would have obtained this weapon in %s seconds.\n",
                    GetCompletionTimeDisplayString(time_diff).c_str());
            }
            else if (ent->client->jumpdata->team == TEAM_HARD)
            {
                HandleMapCompletion(ent);
            }
            ent->client->jumpdata->timer_finished = true;
        }

        return false; // leave the weapon there
    }

    void HandleMapCompletion(edict_t* ent)
    {
        std::string username_lower = AsciiToLower(ent->client->pers.netname);

        int64_t time_ms = ent->client->jumpdata->timer_end - ent->client->jumpdata->timer_begin;

        auto it = jump_server.all_local_maptimes.find(level.mapname);
        if (it == jump_server.all_local_maptimes.end())
        {
            Logger::Warning("Cannot set a time for a map that is not in the maplist");
            return;
        }

        if (it->second.empty())
        {
            // No current times set on this map
            gi.bprintf(PRINT_HIGH, "%s finished in %s seconds (1st completion on the map)\n",
                ent->client->pers.netname, GetCompletionTimeDisplayString(time_ms).c_str());
            jump_server.fresh_times.insert(username_lower);
        }
        else
        {
            int64_t first_time_ms = it->second.front().time_ms;
            int64_t pb_time_ms = -1;
            for (const auto& record : it->second)
            {
                if (record.username_key == username_lower)
                {
                    pb_time_ms = record.time_ms;
                }
            }

            if (time_ms < first_time_ms)
            {
                // User has set a first place!
                if (pb_time_ms == -1)
                {
                    gi.bprintf(PRINT_HIGH, "%s finished in %s seconds (1st %s)\n",
                        ent->client->pers.netname,
                        GetCompletionTimeDisplayString(time_ms).c_str(),
                        GetTimeDiffDisplayString(time_ms, first_time_ms).c_str());
                    std::string first_msg = std::string(ent->client->pers.netname) + " has set a 1st place!";
                    gi.bprintf(PRINT_HIGH, "%s\n", GetGreenConsoleText(first_msg).c_str());
                    jump_server.fresh_times.insert(username_lower);
                }
                else
                {
                    gi.bprintf(PRINT_HIGH, "%s finished in %s seconds (PB %s | 1st %s)\n",
                        ent->client->pers.netname,
                        GetCompletionTimeDisplayString(time_ms).c_str(),
                        GetTimeDiffDisplayString(time_ms, pb_time_ms).c_str(),
                        GetTimeDiffDisplayString(time_ms, first_time_ms).c_str());
                    std::string first_msg = std::string(ent->client->pers.netname) + " has set a 1st place!";
                    gi.bprintf(PRINT_HIGH, "%s\n", GetGreenConsoleText(first_msg).c_str());
                    jump_server.fresh_times.insert(username_lower);
                }
            }
            else
            {
                // Not a first place :(
                if (pb_time_ms == -1)
                {
                    gi.bprintf(PRINT_HIGH, "%s finished in %s seconds (1st %s)\n",
                        ent->client->pers.netname,
                        GetCompletionTimeDisplayString(time_ms).c_str(),
                        GetTimeDiffDisplayString(time_ms, first_time_ms).c_str());
                    jump_server.fresh_times.insert(username_lower);
                }
                else
                {
                    gi.bprintf(PRINT_HIGH, "%s finished in %s seconds (PB %s | 1st %s)\n",
                        ent->client->pers.netname,
                        GetCompletionTimeDisplayString(time_ms).c_str(),
                        GetTimeDiffDisplayString(time_ms, pb_time_ms).c_str(),
                        GetTimeDiffDisplayString(time_ms, first_time_ms).c_str());
                    if (time_ms < pb_time_ms)
                    {
                        jump_server.fresh_times.insert(username_lower);
                    }
                }
            }
        }

        // TODO: we might need to push another replay frame here, could be cutting out the last one

        Logger::Completion(ent->client->pers.netname, ent->client->jumpdata->ip, level.mapname, time_ms);
        SaveMapCompletion(level.mapname, ent->client->pers.netname, time_ms, ent->client->jumpdata->replay_recording);

        if (time_ms < jump_server.replay_now_time_ms)
        {
            jump_server.replay_now_time_ms = time_ms;
            jump_server.replay_now_username = ent->client->pers.netname;
            jump_server.replay_now_recording = ent->client->jumpdata->replay_recording;
        }
    }

    void SaveReplayFrame(edict_t* ent)
    {
        replay_frame_t frame = {};
        VectorCopy(ent->s.origin, frame.pos);
        VectorCopy(ent->client->v_angle, frame.angles);
        frame.key_states = ent->client->jumpdata->key_states;
        frame.fps = ent->client->jumpdata->fps;
        ent->client->jumpdata->replay_recording.push_back(frame);
    }

    void ClearReplayData(edict_t* ent)
    {
        ent->client->jumpdata->replay_recording.clear();
    }

    void JumpClientConnect(edict_t* ent)
    {
        ent->client->jumpdata = new client_data_t();
    }

    void JumpClientDisconnect(edict_t* ent)
    {
        delete ent->client->jumpdata;
        ent->client->jumpdata = NULL;
    }

    void JumpInitGame()
    {
        LoadLocalMapList(jump_server.maplist);
        LoadAllLocalMaptimes(jump_server.maplist, jump_server.all_local_maptimes);
    }

    void AdvanceSpectatingReplayFrame(edict_t* ent)
    {
        if (ent->client->jumpdata->update_replay_spectating)
        {
            int frame_num = ent->client->jumpdata->replay_spectating_framenum;
            if (frame_num >= ent->client->jumpdata->replay_spectating.size())
            {
                Logger::Error("Replay advanced past the end of the replay spectating buffer");
                ent->client->jumpdata->update_replay_spectating = false;
                ent->client->jumpdata->replay_spectating_framenum = 0;
                return;
            }

            const replay_frame_t& frame = ent->client->jumpdata->replay_spectating[frame_num];

            VectorCopy(frame.pos, ent->s.origin);
            VectorCopy(frame.angles, ent->client->v_angle);
            VectorCopy(frame.angles, ent->client->ps.viewangles);
            // TODO keys, fps

            // Since we only send a position update every server frame (10 fps),
            // the client needs to smoothen the movement between the two frames.
            // Setting these flags will do this.
            ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
            ent->client->ps.pmove.pm_type = PM_FREEZE;

            for (int i = 0; i < 3; i++)
            {
                ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
            }

            ent->client->jumpdata->replay_spectating_framenum++;
            if (ent->client->jumpdata->replay_spectating_framenum >= ent->client->jumpdata->replay_spectating.size())
            {
                ent->client->ps.pmove.pm_flags = 0;
                ent->client->ps.pmove.pm_type = PM_SPECTATOR;
                ent->client->jumpdata->update_replay_spectating = false;
            }
        }
    }
}