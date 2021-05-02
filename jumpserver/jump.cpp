#include "jump.h"
#include "jump_logger.h"
#include "jump_scores.h"
#include <unordered_map>
#include "jump_utils.h"
#include "jump_global.h"
#include "jump_voting.h"
#include "jump_ghost.h"
#include "jump_local_database.h"
#include "jump_spawn.h"

namespace Jump
{
    server_data_t jump_server;

    static const int MaxHealthAndAmmo = 1000;

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
        { "Join Easy Team",               PMENU_ALIGN_LEFT,   JoinTeamEasyCommand },
        { NULL /* number of players */,   PMENU_ALIGN_LEFT,   NULL },
        { "Join Hard Team",               PMENU_ALIGN_LEFT,   JoinTeamHardCommand },
        { NULL /* number of players */,   PMENU_ALIGN_LEFT,   NULL },
        { NULL /* chase camera */,        PMENU_ALIGN_LEFT,   JoinChaseCamCommand },
        { NULL,                           PMENU_ALIGN_LEFT,   NULL }, // TODO help commands menu
        { NULL,                           PMENU_ALIGN_LEFT,   NULL },
        { "Highlight your choice and",    PMENU_ALIGN_LEFT,   NULL },
        { "press ENTER.",                 PMENU_ALIGN_LEFT,   NULL },
        { "v" JUMP_VERSION_STRING,        PMENU_ALIGN_RIGHT,  NULL },
    };


    // ent must be a client
    // No other menus can be open when this function is called
    void OpenMenu_Join(edict_t* ent)
    {
        static char mapName[MaxMenuWidth] = { 0 };
        static char playersEasy[MaxMenuWidth] = { 0 };
        static char playersHard[MaxMenuWidth] = { 0 };
        static char chaseCam[MaxMenuWidth] = { 0 };

        strncpy(mapName, level.mapname, MaxMenuWidth - 1);
        sprintf(playersEasy, "  (%d players)", CountPlayersOnTeam(TeamEnum::Easy));
        sprintf(playersHard, "  (%d players)", CountPlayersOnTeam(TeamEnum::Hard));

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

    int CountPlayersOnTeam(TeamEnum team)
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

    void UpdateUserId(edict_t* ent)
    {
        int userId = LocalDatabase::GetUserId(ent->client->pers.netname);
        if (userId == -1)
        {
            // TODO: update the username userid cache in LocalScores
            userId = LocalDatabase::AddUser(ent->client->pers.netname);
        }
        ent->client->jumpdata->localUserId = userId;
        if (userId == -1)
        {
            gi.cprintf(ent, PRINT_HIGH, "[Server] Could not create user. Times will not be saved.\n");
            Jump::Logger::Warning(va("Could not create userId for user %s", ent->client->pers.netname));
        }
    }

    void JoinTeamEasyCommand(edict_t* ent, pmenuhnd_t* hnd)
    {
        PMenu_Close(ent);
        Spawn::JoinTeamEasy(ent);
    }

    void JoinTeamHardCommand(edict_t* ent, pmenuhnd_t* hnd)
    {
        PMenu_Close(ent);
        Spawn::JoinTeamHard(ent);
    }

    void JoinChaseCamCommand(edict_t* ent, pmenuhnd_t* hnd)
    {
        PMenu_Close(ent);
        if (ent->client->jumpdata->team == TeamEnum::Spectator)
        {
            // TODO: Try to find someone to chase
        }
        else
        {
            Spawn::JoinTeamSpectator(ent);
        }
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
            if (ent->client->jumpdata->team == TeamEnum::Easy)
            {
                gi.cprintf(ent, PRINT_HIGH, "You would have obtained this weapon in %s seconds.\n",
                    GetCompletionTimeDisplayString(time_diff).c_str());
            }
            else if (ent->client->jumpdata->team == TeamEnum::Hard)
            {
                HandleMapCompletion(ent);
            }
            ent->client->jumpdata->timer_finished = true;
        }

        return false; // leave the weapon there
    }

    void HandleMapCompletion(edict_t* ent)
    {
        if (ent->client->jumpdata->timer_begin == 0 || ent->client->jumpdata->replay_recording.empty())
        {
            Logger::Error(va("Invalid zero time map completion, user %s, map %s",
                ent->client->pers.netname, level.mapname));
            return;
        }

        int timeMs = static_cast<int>(ent->client->jumpdata->timer_end - ent->client->jumpdata->timer_begin);
        int pmoveTimeMs = static_cast<int>(ent->client->jumpdata->timer_pmove_msec);

        if (!LocalScores::IsMapInMaplist(level.mapname))
        {
            gi.cprintf(ent, PRINT_HIGH, "You have finished in %s seconds.\n", GetCompletionTimeDisplayString(timeMs).c_str());
            gi.cprintf(ent, PRINT_HIGH, "WARNING: Map not in maplist.  Time will not be saved.\n");
            return;
        }

        int userBestTimeMs = LocalDatabase::GetMapTime(level.mapname, ent->client->pers.netname);

        std::vector<MapTimesEntry> bestTimeEntry;
        LocalDatabase::GetMapTimes(bestTimeEntry, level.mapname, 1, 0);

        LocalDatabase::AddMapTime(level.mapname, ent->client->pers.netname,
            timeMs, pmoveTimeMs, ent->client->jumpdata->replay_recording);

        if (bestTimeEntry.empty())
        {
            // No current times set on this map
            gi.bprintf(PRINT_HIGH, "%s finished in %s seconds (1st completion on the map)\n",
                ent->client->pers.netname, GetCompletionTimeDisplayString(timeMs).c_str());
            GhostReplay::LoadReplay();
        }
        else
        {
            if (timeMs < bestTimeEntry[0].timeMs)
            {
                // New best time for map!
                if (userBestTimeMs == -1)
                {
                    gi.bprintf(PRINT_HIGH, "%s finished in %s seconds (1st %s)\n",
                        ent->client->pers.netname,
                        GetCompletionTimeDisplayString(timeMs).c_str(),
                        GetTimeDiffDisplayString(timeMs, bestTimeEntry[0].timeMs).c_str());
                    std::string first_msg = std::string(ent->client->pers.netname) + " has set a 1st place!";
                    gi.bprintf(PRINT_HIGH, "%s\n", GetGreenConsoleText(first_msg).c_str());
                }
                else
                {
                    gi.bprintf(PRINT_HIGH, "%s finished in %s seconds (PB %s | 1st %s)\n",
                        ent->client->pers.netname,
                        GetCompletionTimeDisplayString(timeMs).c_str(),
                        GetTimeDiffDisplayString(timeMs, userBestTimeMs).c_str(),
                        GetTimeDiffDisplayString(timeMs, bestTimeEntry[0].timeMs).c_str());
                    std::string first_msg = std::string(ent->client->pers.netname) + " has set a 1st place!";
                    gi.bprintf(PRINT_HIGH, "%s\n", GetGreenConsoleText(first_msg).c_str());
                }
                GhostReplay::LoadReplay();
            }
            else
            {
                // Not a first place
                if (userBestTimeMs == -1)
                {
                    gi.bprintf(PRINT_HIGH, "%s finished in %s seconds (1st %s)\n",
                        ent->client->pers.netname,
                        GetCompletionTimeDisplayString(timeMs).c_str(),
                        GetTimeDiffDisplayString(timeMs, bestTimeEntry[0].timeMs).c_str());
                }
                else
                {
                    gi.bprintf(PRINT_HIGH, "%s finished in %s seconds (PB %s | 1st %s)\n",
                        ent->client->pers.netname,
                        GetCompletionTimeDisplayString(timeMs).c_str(),
                        GetTimeDiffDisplayString(timeMs, userBestTimeMs).c_str(),
                        GetTimeDiffDisplayString(timeMs, bestTimeEntry[0].timeMs).c_str());
                }
            }
        }

        jump_server.fresh_times.insert(ent->client->pers.netname);
        if (timeMs < jump_server.replay_now_time_ms)
        {
            jump_server.replay_now_time_ms = timeMs;
            jump_server.replay_now_username = ent->client->pers.netname;
            jump_server.replay_now_recording = ent->client->jumpdata->replay_recording;
        }
        Logger::Completion(ent->client->pers.netname, ent->client->jumpdata->ip, level.mapname, timeMs);

        // TODO: need to automatically update the replays of anyone racing
        // The could be the first race replay or whatever else position they are racing
        // People usually only race the first place replay

        // TODO: send to global database
    }

    void SaveReplayFrame(edict_t* ent)
    {   
        replay_frame_t frame = {};

        VectorCopy(ent->s.origin, frame.pos);
        VectorCopy(ent->client->v_angle, frame.angles);

        frame.animation_frame = static_cast<uint8_t>(ent->s.frame);

        frame.key_states = ent->client->jumpdata->key_states;

        frame.fps = static_cast<uint8_t>(ent->client->jumpdata->fps);
        frame.async = static_cast<uint8_t>(ent->client->jumpdata->async);
        frame.checkpoints = 0; // TODO
        
        frame.weapon_inven = 0; // TODO
        frame.weapon_equipped = 0; // TODO

        frame.reserved1 = 0;
        frame.reserved2 = 0;

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

        VoteSystem::RemoveParticipant(ent);
    }

    void JumpInitGame()
    {
        LocalDatabase::Init();
        LocalScores::LoadMaplist();
        jump_server.global_database_thread = std::thread(ThreadMainGlobal);
        VoteSystem::Init();
    }

    void JumpRunFrame()
    {
        GhostReplay::RunFrame();

        VoteSystem::OnFrame();
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
                ent->client->jumpdata->replay_spectating_framenum = 0;
            }
        }
    }

    void AdvanceRacingSpark(edict_t* ent)
    {
        // TODO: if racing a time and someone sets a better time, need to update racing frames

        if (ent->client->jumpdata->racing)
        {          
            if (ent->client->jumpdata->replay_recording.size() == 0)
            {
                // Player has not started the timer yet
                return;
            }

            int player_start_framenum = ent->client->jumpdata->racing_delay_frames + 3;
            if (player_start_framenum > ent->client->jumpdata->replay_recording.size())
            {
                // Not time for the race to start yet
                return;
            }

            // TODO: this pushes the race frame 1 frame ahead of the player; this might be what we want
            if (ent->client->jumpdata->racing_framenum < 4)
            {
                // The first frame of the race always has to start at 4
                ent->client->jumpdata->racing_framenum = 4;
            }

            if (ent->client->jumpdata->racing_framenum >= ent->client->jumpdata->racing_frames.size())
            {
                // Race is finished (or race has less than 4 frames)
                return;
            }

            // Show the last three race positions
            gi.WriteByte(svc_temp_entity);
            gi.WriteByte(TE_BFG_LASER);
            gi.WritePosition(ent->client->jumpdata->racing_frames[ent->client->jumpdata->racing_framenum - 3].pos);
            gi.WritePosition(ent->client->jumpdata->racing_frames[ent->client->jumpdata->racing_framenum - 2].pos);

            gi.WriteByte(svc_temp_entity);
            gi.WriteByte(TE_BFG_LASER);
            gi.WritePosition(ent->client->jumpdata->racing_frames[ent->client->jumpdata->racing_framenum - 2].pos);
            gi.WritePosition(ent->client->jumpdata->racing_frames[ent->client->jumpdata->racing_framenum - 1].pos);

            gi.WriteByte(svc_temp_entity);
            gi.WriteByte(TE_BFG_LASER);
            gi.WritePosition(ent->client->jumpdata->racing_frames[ent->client->jumpdata->racing_framenum - 1].pos);
            gi.WritePosition(ent->client->jumpdata->racing_frames[ent->client->jumpdata->racing_framenum].pos);

            gi.unicast(ent, true);
            ent->client->jumpdata->racing_framenum++;
        }
    }

    void DoStuffOnMapChange()
    {
        jump_server.replay_now_recording.clear();
        jump_server.replay_now_time_ms = 0;
        jump_server.replay_now_username.clear();

        jump_server.fresh_times.clear();
    }


    void InitializeClientEnt(edict_t* ent)
    {
        // Set all fields of ent->s
        ent->s.number = ent - g_edicts;
        VectorClear(ent->s.origin);
        VectorClear(ent->s.angles);
        VectorClear(ent->s.old_origin);
        ent->s.modelindex = 255;
        ent->s.modelindex2 = 255;
        ent->s.modelindex3 = 0;
        ent->s.modelindex4 = 0;
        ent->s.frame = 0;
        ent->s.skinnum = ent - g_edicts - 1;
        ent->s.effects = 0;
        ent->s.renderfx = 0;
        ent->s.solid = SOLID_TRIGGER;
        ent->s.sound = 0;
        ent->s.event = 0;

        // Set all fields of ent->client
        ent->client->ps.pmove.pm_type = PM_SPECTATOR;
        VectorClear(ent->client->ps.pmove.origin);
        VectorClear(ent->client->ps.pmove.velocity);
        ent->client->ps.pmove.pm_flags = 0;
        ent->client->ps.pmove.pm_time = 0;
        ent->client->ps.pmove.gravity = 0; // TODO?
        VectorClear(ent->client->ps.pmove.delta_angles);

        VectorClear(ent->client->ps.viewangles);
        VectorClear(ent->client->ps.viewoffset);
        VectorClear(ent->client->ps.kick_angles);
        VectorClear(ent->client->ps.gunangles);
        VectorClear(ent->client->ps.gunoffset);
        ent->client->ps.gunindex = 0;
        ent->client->ps.gunframe = 0;
        ArrayClear(ent->client->ps.blend);
        ent->client->ps.fov; // TODO: already set?
        ent->client->ps.rdflags = 0;
        ArrayClear(ent->client->ps.stats);

        ent->client->ping; // TODO: already set?

        ent->client->pers.userinfo; // Already set
        ent->client->pers.netname; // Already set
        ent->client->pers.hand; // Already set
        ent->client->pers.connected = true;
        ent->client->pers.health = MaxHealthAndAmmo;
        ent->client->pers.max_health = MaxHealthAndAmmo;
        ent->client->pers.savedFlags = 0;
        ent->client->pers.selected_item = 0;
        ArrayClear(ent->client->pers.inventory);
        ent->client->pers.max_bullets = MaxHealthAndAmmo;
        ent->client->pers.max_shells = MaxHealthAndAmmo;
        ent->client->pers.max_rockets = MaxHealthAndAmmo;
        ent->client->pers.max_grenades = MaxHealthAndAmmo;
        ent->client->pers.max_cells = MaxHealthAndAmmo;
        ent->client->pers.max_slugs = MaxHealthAndAmmo;
        ent->client->pers.weapon = nullptr;
        ent->client->pers.lastweapon = nullptr;
        ent->client->pers.power_cubes = 0;
        ent->client->pers.score = 0;

        ent->client->resp.enterframe = level.framenum;
        ent->client->resp.score = 0;
        VectorClear(ent->client->resp.cmd_angles);
        ent->client->resp.game_helpchanged = 0;
        ent->client->resp.helpchanged = 0;

        ent->client->old_pmove.pm_type = PM_SPECTATOR;
        VectorClear(ent->client->old_pmove.origin);
        VectorClear(ent->client->old_pmove.velocity);
        ent->client->old_pmove.pm_flags = 0;
        ent->client->old_pmove.pm_time = 0;
        ent->client->old_pmove.gravity = 0; // TODO?
        VectorClear(ent->client->old_pmove.delta_angles);

        ent->client->showscores = false;
        ent->client->inmenu = false;
        ent->client->menu = nullptr;
        ent->client->showinventory = false;
        ent->client->showhelp = false;
        ent->client->showhelpicon = false;
        ent->client->ammo_index = 0;
        ent->client->buttons = 0;
        ent->client->oldbuttons = 0;
        ent->client->latched_buttons = 0;
        ent->client->weapon_thunk = false;
        ent->client->newweapon = nullptr;
        ent->client->damage_armor = 0;
        ent->client->damage_parmor = 0;
        ent->client->damage_blood = 0;
        ent->client->damage_knockback = 0;
        VectorClear(ent->client->damage_from);
        ent->client->killer_yaw = 0;
        ent->client->weaponstate = WEAPON_READY;
        VectorClear(ent->client->kick_angles);
        VectorClear(ent->client->kick_origin);
        ent->client->v_dmg_roll = 0;
        ent->client->v_dmg_pitch = 0;
        ent->client->v_dmg_time = 0;
        ent->client->fall_time = 0;
        ent->client->fall_value = 0;
        ent->client->damage_alpha = 0;
        ent->client->bonus_alpha = 0;
        VectorClear(ent->client->damage_blend);
        VectorClear(ent->client->v_angle);
        ent->client->bobtime = 0;
        VectorClear(ent->client->oldviewangles);
        VectorClear(ent->client->oldvelocity);
        ent->client->next_drown_time = 0;
        ent->client->old_waterlevel = 0;
        ent->client->breather_sound = 0;
        ent->client->machinegun_shots = 0;
        ent->client->anim_end = 0;
        ent->client->anim_priority = ANIM_BASIC;
        ent->client->anim_duck = false;
        ent->client->anim_run = false;
        ent->client->quad_framenum = 0;
        ent->client->invincible_framenum = 0;
        ent->client->breather_framenum = 0;
        ent->client->enviro_framenum = 0;
        ent->client->grenade_blew_up = false;
        ent->client->grenade_time = 0;
        ent->client->silencer_shots = 0;
        ent->client->weapon_sound = 0;
        ent->client->pickup_msg_time = 0;
        ent->client->flood_locktill = 0;
        ArrayClear(ent->client->flood_when);
        ent->client->flood_whenhead = 0;
        ent->client->respawn_time = 0; // TODO: could set this maybe
        ent->client->chase_target = nullptr;
        ent->client->update_chase = false;
        ent->client->menutime = 0;
        ent->client->menudirty = false;

        ent->client->jumpdata->replay_recording.clear();
        ent->client->jumpdata->replay_spectating.clear();
        ent->client->jumpdata->replay_spectating_framenum = 0;
        ent->client->jumpdata->update_replay_spectating = false;
        ent->client->jumpdata->replay_spectating_hud_string.clear();
        ent->client->jumpdata->localUserId; // Already set
        ent->client->jumpdata->fps; // Already set
        ent->client->jumpdata->async; // Already set
        ent->client->jumpdata->ip; // Already set
        ent->client->jumpdata->team = TeamEnum::Spectator;
        ent->client->jumpdata->timer_pmove_msec = 0;
        ent->client->jumpdata->timer_begin = 0;
        ent->client->jumpdata->timer_end = 0;
        ent->client->jumpdata->timer_paused = true;
        ent->client->jumpdata->timer_finished = false;
        ent->client->jumpdata->stores = {};
        ent->client->jumpdata->store_ent = nullptr;
        ent->client->jumpdata->key_states = 0;
        ent->client->jumpdata->scores_menu = ScoresMenuEnum::None;
        ent->client->jumpdata->racing = false;
        ent->client->jumpdata->racing_frames.clear();
        ent->client->jumpdata->racing_framenum = 0;
        ent->client->jumpdata->racing_delay_frames = 0;
        ent->client->jumpdata->racing_highscore = 0;
        ent->client->jumpdata->hud_footer1.clear();
        ent->client->jumpdata->hud_footer2.clear();
        ent->client->jumpdata->hud_footer3.clear();
        ent->client->jumpdata->hud_footer4.clear();
        ent->client->jumpdata->show_jumpers = true;

        // Set the rest of ent fields
        ent->inuse = true;

        ent->linkcount = 0; // All this link stuff gets set when the ent is linked
        ent->area.prev = nullptr;
        ent->area.next = nullptr;
        ent->num_clusters = 0;
        ArrayClear(ent->clusternums);
        ent->headnode = 0;
        ent->areanum = 0;
        ent->areanum2 = 0;

        ent->svflags = 0;

        vec3_t mins = { -16, -16, -24 };
        vec3_t maxs = { 16, 16, 32 };
        VectorCopy(mins, ent->mins);
        VectorCopy(maxs, ent->maxs);

        VectorClear(ent->absmin); // These all get set when the ent is linked
        VectorClear(ent->absmax);
        VectorClear(ent->size);

        ent->solid = SOLID_TRIGGER;
        ent->clipmask = 0;
        ent->owner = nullptr;
        ent->movetype = MOVETYPE_NOCLIP;
        ent->flags = 0;
        ent->model = "players/female/tris.md2";
        ent->freetime = 0;
        ent->message = nullptr;
        ent->classname = "player";
        ent->spawnflags = 0;
        ent->timestamp = 0;
        ent->angle = 0;
        ent->target = nullptr;
        ent->targetname = nullptr;
        ent->killtarget = nullptr;
        ent->team = nullptr;
        ent->pathtarget = nullptr;
        ent->deathtarget = nullptr;
        ent->combattarget = nullptr;
        ent->target_ent = nullptr;
        ent->speed = 0;
        ent->accel = 0;
        ent->decel = 0;
        VectorClear(ent->movedir);
        VectorClear(ent->pos1);
        VectorClear(ent->pos2);
        VectorClear(ent->velocity);
        VectorClear(ent->avelocity);
        ent->mass = 200;
        ent->air_finished = 0;
        ent->gravity = 1.0f;
        ent->goalentity = nullptr;
        ent->movetarget = nullptr;
        ent->yaw_speed = 0;
        ent->ideal_yaw = 0;
        ent->nextthink = 0;
        ent->prethink = nullptr;
        ent->think = nullptr;
        ent->blocked = nullptr;
        ent->touch = nullptr;
        ent->use = nullptr;
        ent->pain = player_pain;
        ent->die = player_die;
        ent->touch_debounce_time = 0;
        ent->pain_debounce_time = 0;
        ent->damage_debounce_time = 0;
        ent->fly_sound_debounce_time = 0;
        ent->last_move_time = 0;
        ent->health = MaxHealthAndAmmo;
        ent->max_health = MaxHealthAndAmmo;
        ent->gib_health = 0;
        ent->deadflag = DEAD_NO;
        ent->show_hostile = 0;
        ent->powerarmor_time = 0;
        ent->map = nullptr;
        ent->viewheight = 22;
        ent->takedamage = DAMAGE_NO;
        ent->dmg = 0;
        ent->radius_dmg = 0;
        ent->dmg_radius = 0;
        ent->sounds = 0;
        ent->count = 0;
        ent->chain = nullptr;
        ent->enemy = nullptr;
        ent->oldenemy = nullptr;
        ent->activator = nullptr;
        ent->groundentity = nullptr;
        ent->groundentity_linkcount = 0;
        ent->teamchain = nullptr;
        ent->teammaster = nullptr;
        ent->mynoise = nullptr;
        ent->mynoise2 = nullptr;
        ent->noise_index = 0;
        ent->noise_index2 = 0;
        ent->volume = 0;
        ent->attenuation = 0;
        ent->wait = 0;
        ent->delay = 0;
        ent->random = 0;
        ent->teleport_time = 0;
        ent->watertype = 0;
        ent->waterlevel = 0;
        VectorClear(ent->move_origin);
        VectorClear(ent->move_angles);
        ent->light_level = 0;
        ent->style = 0;
        ent->item = nullptr;
        memset(&ent->moveinfo, 0, sizeof(ent->moveinfo));
        memset(&ent->monsterinfo, 0, sizeof(ent->monsterinfo));
    }


}