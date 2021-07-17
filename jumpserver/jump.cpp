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
#include "jump_msets.h"

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

    bool PickupWeaponForFiring(edict_t* weap, edict_t* ent, const char* weaponClassname)
    {
        if (weap != nullptr && StringCompareInsensitive(weap->classname, weaponClassname))
        {
            // Add to inventory
            int index = ITEM_INDEX(weap->item);
            ent->client->pers.inventory[index] = 1;

            // Give unlimited ammo
            gitem_t* ammo = FindItem(weap->item->ammo);
            Add_Ammo(ent, ammo, MaxHealthAndAmmo);

            // Auto-equip if not holding a weapon
            if (ent->client->pers.weapon == nullptr)
            {
                ent->client->newweapon = weap->item;
                ChangeWeapon(ent);
            }
            return true;
        }
        return false;
    }

    void PickupWeapon(edict_t* weap, edict_t* ent)
    {
        if (ent->client == nullptr || !ent->inuse)
        {
            return;
        }

        if ((MSets::GetGrenadeLauncher() && PickupWeaponForFiring(weap, ent, "weapon_grenadelauncher")) ||
            (MSets::GetRocket() && PickupWeaponForFiring(weap, ent, "weapon_rocketlauncher")) ||
            (MSets::GetHyperBlaster() && PickupWeaponForFiring(weap, ent, "weapon_hyperblaster")) ||
            (MSets::GetBfg() && PickupWeaponForFiring(weap, ent, "weapon_bfg")))
        {
            return;
        }

        if (ent->client->jumpdata->timer_finished)
        {
            // If we have already completed the map, ignore any weapon pickups
            return;
            // TODO: make sure recall on easy mode resets timer_finished
        }

        // Checkpoint check
        int totalCheckpoints = MSets::GetCheckpointTotal();
        if ((totalCheckpoints > 0) && (ent->client->jumpdata->checkpoint_total < totalCheckpoints))
        {
            int64_t timeNowMs = Sys_Milliseconds();
            if ((ent->client->jumpdata->timer_trigger_spam == 0) ||
                ((timeNowMs - ent->client->jumpdata->timer_trigger_spam) > 5000))
            {
                gi.cprintf(ent, PRINT_HIGH,
                    va("You need %d checkpoint(s) before being able to finish the map. You currently have %d.\n",
                        totalCheckpoints, ent->client->jumpdata->checkpoint_total));
                ent->client->jumpdata->timer_trigger_spam = timeNowMs;
            }
            return;
        }

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

        // Update game's database cache
        ent->client->jumpdata->cached_completions++;

        if (ent->client->jumpdata->cached_time_msec > 0)
        {
            if (timeMs < ent->client->jumpdata->cached_time_msec)
            {
                ent->client->jumpdata->cached_time_msec = timeMs;
            }
        }
        else
        {
            ent->client->jumpdata->cached_time_msec = timeMs;
            ent->client->jumpdata->cached_maps_completed++;
        }

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
        frame.checkpoints = ent->client->jumpdata->checkpoint_total;
        
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

    /// <summary>
    // Every time a player joins the game. This does NOT get called between level changes.
    /// </summary>
    /// <param name="ent">Player entity</param>
    void JumpClientConnect(edict_t* ent)
    {
        ent->client->jumpdata = new client_data_t();

        // Only init persistent data here.
        ent->client->jumppers = new client_pers_data_t();
    }

    /// <summary>
    /// Every time a player leaves the game. This does NOT get called between level changes.
    /// </summary>
    /// <param name="ent"></param>
    void JumpClientDisconnect(edict_t* ent)
    {
        delete ent->client->jumpdata;
        ent->client->jumpdata = NULL;

        VoteSystem::RemoveParticipant(ent);

        assert(ent->client->jumppers != nullptr);
        delete ent->client->jumppers;
        ent->client->jumppers = nullptr;
    }

    void JumpInitCvars()
    {
        jump_server.cvar_idle_time = gi.cvar("jump_idle_time", "60", 0);
    }

    void JumpInitGame()
    {
        LocalDatabase::Init();
        LocalScores::LoadMaplist();
        jump_server.global_database_thread = std::thread(ThreadMainGlobal);
        VoteSystem::Init();
        GhostReplay::LoadGhostModels();
    }

    void JumpRunFrame()
    {
        GhostReplay::RunFrame();

        VoteSystem::OnFrame();
    }

    // TODO: for fractional replays, we use the level.framenum which causes snapping issues when adjusting speeds
    // The current version of jump keeps track of the fractional framenum so it has smooth transitions
    // See jumpmod.c, Replay_Recording()
    void AdvanceSpectatingReplayFrame(edict_t* ent)
    {
        if (!ent->client->jumpdata->update_replay_spectating)
        {
            return;
        }

        int currentFrameNum = ent->client->jumpdata->replay_spectating_framenum;
        int maxFrameNum = static_cast<int>(ent->client->jumpdata->replay_spectating.size()) - 1;
        if (currentFrameNum < 0 || currentFrameNum > maxFrameNum)
        {
            Logger::Error(va("Replay out of range, frame %d, max %d", currentFrameNum, maxFrameNum));
            ent->client->jumpdata->update_replay_spectating = false;
            ent->client->jumpdata->replay_spectating_framenum = 0;
            return;
        }

        vec3_t newPos = {};
        vec3_t newAngles = {};
        bool noChange = false;
        bool endReplay = false;
        int nextFrameNum = currentFrameNum;

        ReplaySpeed speed = ent->client->jumpdata->replay_speed;
        if (speed == ReplaySpeed::Paused)
        {
            noChange = true;
        }
        else if ((speed <= ReplaySpeed::Pos_Half) && (speed >= ReplaySpeed::Neg_Half))
        {
            // Fractional speed
            nextFrameNum = (int)speed < 0 ? (currentFrameNum - 1) : (currentFrameNum + 1);
            if (nextFrameNum < 0 || nextFrameNum > maxFrameNum)
            {
                endReplay = true;
            }
            else
            {
                double fraction = 0.0;
                bool partial = false;
                if (speed == ReplaySpeed::Neg_Tenth || speed == ReplaySpeed::Pos_Tenth)
                {
                    int remainder = level.framenum % 10;
                    if (remainder != 0)
                    {
                        partial = true;
                        fraction = remainder * 0.1;
                    }
                }
                else if (speed == ReplaySpeed::Neg_Fifth || speed == ReplaySpeed::Pos_Fifth)
                {
                    int remainder = level.framenum % 5;
                    if (remainder != 0)
                    {
                        partial = true;
                        fraction = remainder * 0.2;
                    }
                }
                else // speed == half
                {
                    int remainder = level.framenum % 2;
                    if (remainder != 0)
                    {
                        partial = true;
                        fraction = remainder * 0.5;
                    }
                }

                if (partial)
                {
                    // In order to maintain a smooth replay, we need to linear interpolate the position.
                    vec3_t diffPos = {};
                    vec3_t diffAngles = {};
                    VectorSubtract(
                        ent->client->jumpdata->replay_spectating[nextFrameNum].pos,
                        ent->client->jumpdata->replay_spectating[currentFrameNum].pos,
                        diffPos);
                    VectorSubtract(
                        ent->client->jumpdata->replay_spectating[nextFrameNum].angles,
                        ent->client->jumpdata->replay_spectating[currentFrameNum].angles,
                        diffAngles);
                    FixAngles(diffAngles);
                    diffPos[0] = diffPos[0] * fraction;
                    diffPos[1] = diffPos[1] * fraction;
                    diffPos[2] = diffPos[2] * fraction;
                    diffAngles[0] = diffAngles[0] * fraction;
                    diffAngles[1] = diffAngles[1] * fraction;
                    diffAngles[2] = diffAngles[2] * fraction;
                    VectorAdd(ent->client->jumpdata->replay_spectating[currentFrameNum].pos, diffPos, newPos);
                    VectorAdd(ent->client->jumpdata->replay_spectating[currentFrameNum].angles, diffAngles, newAngles);
                    nextFrameNum = currentFrameNum; // stay on current frame until we reach a whole frame advancement
                }
                else
                {
                    // Whole frame advancement
                    VectorCopy(ent->client->jumpdata->replay_spectating[nextFrameNum].pos, newPos);
                    VectorCopy(ent->client->jumpdata->replay_spectating[nextFrameNum].angles, newAngles);
                }
            }
        }
        else // Non-fractional speed
        {
            int framesToAdvance = (int)speed / 100;
            nextFrameNum = currentFrameNum + framesToAdvance;
            if (nextFrameNum < 0 || nextFrameNum > maxFrameNum)
            {
                endReplay = true;
            }
            else
            {
                VectorCopy(ent->client->jumpdata->replay_spectating[nextFrameNum].pos, newPos);
                VectorCopy(ent->client->jumpdata->replay_spectating[nextFrameNum].angles, newAngles);
            }
        }

        if (endReplay)
        {
            if (ent->client->jumpdata->replay_repeating)
            {
                // If the user has replay repeating, just reset back to first frame.
                ent->client->jumpdata->replay_spectating_framenum = 0;
                ent->client->jumpdata->replay_speed = ReplaySpeed::Pos_1;
            }
            else
            {
                // No replay repeating, so just end the replay.
                ent->client->ps.pmove.pm_flags = 0;
                ent->client->ps.pmove.pm_type = PM_SPECTATOR;
                ent->client->jumpdata->update_replay_spectating = false;
                ent->client->jumpdata->replay_spectating_framenum = 0;
            }
        }
        else
        {
            if (!noChange)
            {
                VectorCopy(newPos, ent->s.origin);
                VectorCopy(newAngles, ent->client->v_angle);
                VectorCopy(newAngles, ent->client->ps.viewangles);

                // Since we only send a position update every server frame (10 fps),
                // the client needs to smoothen the movement between the two frames.
                // Setting these flags will do this.
                ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
                ent->client->ps.pmove.pm_type = PM_FREEZE;

                for (int i = 0; i < 3; i++)
                {
                    ent->client->ps.pmove.delta_angles[i] =
                        ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
                }

                if (currentFrameNum != nextFrameNum)
                {
                    int currentCheckpoints = ent->client->jumpdata->replay_spectating[currentFrameNum].checkpoints;
                    int nextCheckpoints = ent->client->jumpdata->replay_spectating[nextFrameNum].checkpoints;
                    if (currentCheckpoints != nextCheckpoints)
                    {
                        int checkpointTotal = MSets::GetCheckpointTotal();
                        if (nextCheckpoints == 1)
                        {
                            int timeMs = nextFrameNum * 100;
                            std::string overallTimeStr = GetCompletionTimeDisplayString(timeMs);
                            gi.cprintf(ent, PRINT_HIGH, va("Replay reached checkpoint %d/%d in %s seconds.\n",
                                nextCheckpoints, checkpointTotal, overallTimeStr.c_str()));
                            ent->client->jumpdata->timer_checkpoint_split = timeMs;
                        }
                        else
                        {
                            int timeMs = nextFrameNum * 100;
                            std::string overallTimeStr = GetCompletionTimeDisplayString(timeMs);
                            int64_t splitMs = timeMs - ent->client->jumpdata->timer_checkpoint_split;
                            std::string splitTimeStr = GetCompletionTimeDisplayString(::abs(splitMs));
                            if (splitMs < 0)
                            {
                                splitTimeStr.insert(splitTimeStr.begin(), '-');
                            }
                            gi.cprintf(ent, PRINT_HIGH, va("Replay reached checkpoint %d/%d in %s seconds. (split: %s)\n",
                                nextCheckpoints, checkpointTotal, overallTimeStr.c_str(), splitTimeStr.c_str()));
                            ent->client->jumpdata->timer_checkpoint_split = timeMs;
                        }
                    }
                }

                // Finally, update the frame number
                ent->client->jumpdata->replay_spectating_framenum = nextFrameNum;
            }
        }
    }

    void FixAngles(vec3_t& angles)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (angles[i] > 180.0f)
            {
                angles[i] -= 360.0f;
            }
            else if (angles[i] < -180.0f)
            {
                angles[i] += 360.0f;
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

    void RemoveAllPlayerWeapons(edict_t* ent)
    {
        if (ent->client == nullptr || !ent->inuse)
        {
            return;
        }
        gitem_t* item = nullptr;
        item = FindItem("Blaster");
        ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
        item = FindItem("Shotgun");
        ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
        item = FindItem("Super Shotgun");
        ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
        item = FindItem("Machinegun");
        ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
        item = FindItem("Chaingun");
        ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
        item = FindItem("Grenades");
        ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
        item = FindItem("Grenade Launcher");
        ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
        item = FindItem("Rocket Launcher");
        ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
        item = FindItem("HyperBlaster");
        ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
        item = FindItem("Railgun");
        ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
        item = FindItem("BFG10K");
        ent->client->newweapon = nullptr;
        ChangeWeapon(ent);
        // TODO: does this work correctly with hand grenades?
        // TODO: does this work correctly by setting newweapon to nullptr?
    }

    void UpdatePlayerIdleState(edict_t* ent, usercmd_t* ucmd)
    {
        auto client = ent->client;

        auto prev_state = client->jumppers->idle_state;
        auto new_state = IdleStateEnum::None;

        const uint64_t max_idletime_msec = jump_server.cvar_idle_time->value * 1000;

        bool moved = false;

        auto moved_func = [&]()
        {
            // Never count spectator moving if already idle.
            if (prev_state != IdleStateEnum::None && client->jumpdata->team == TeamEnum::Spectator)
            {
                return false;
            }

            return  ucmd->forwardmove != client->jumppers->prev_idle_forwardmove ||
                    ucmd->sidemove != client->jumppers->prev_idle_sidemove ||
                    ucmd->upmove != client->jumppers->prev_idle_upmove;
        };

        if (moved_func())
        {
            client->jumppers->idle_msec = 0;

            moved = true;
        }
        else
        {
            client->jumppers->idle_msec += ucmd->msec;
        }

        if (!moved && max_idletime_msec != 0 && client->jumppers->idle_msec >= max_idletime_msec)
        {
            new_state = IdleStateEnum::Auto;
        }

        if (!moved && prev_state == IdleStateEnum::Self)
        {
            new_state = IdleStateEnum::Self;
        }
        
        if (prev_state != new_state)
        {
            Logger::Debug(va("Player's %s idle state changed from %i to %i.", ent->client->pers.netname, prev_state, new_state));

            NotifyPlayerIdleChange(ent, prev_state, new_state);
        }

        client->jumppers->idle_state = new_state;
        client->jumppers->prev_idle_forwardmove = ucmd->forwardmove;
        client->jumppers->prev_idle_sidemove = ucmd->sidemove;
        client->jumppers->prev_idle_upmove = ucmd->upmove;
    }

    void ForcePlayerIdleStateWakeup(edict_t* ent)
    {
        if (ent->client->jumppers->idle_state != IdleStateEnum::None)
        {
            Logger::Debug(va("Player's %s idle state was changed by forcing a wake up!", ent->client->pers.netname));

            NotifyPlayerIdleChange(ent, ent->client->jumppers->idle_state, IdleStateEnum::None);
        }

        ent->client->jumppers->idle_msec = 0;
        ent->client->jumppers->idle_state = IdleStateEnum::None;
    }

    void NotifyPlayerIdleChange(edict_t* ent, IdleStateEnum prev_state, IdleStateEnum new_state)
    {
        if (prev_state == new_state)
            return;


        if (new_state == IdleStateEnum::Auto)
        {
            gi.cprintf(ent, PRINT_HIGH, "You are now marked as idle for being inactive for too long.\n");
        }
        else if (new_state == IdleStateEnum::Self)
        {
            gi.cprintf(ent, PRINT_HIGH, "You are now marked as idle.\n");
        }
        else if (new_state == IdleStateEnum::None)
        {
            gi.cprintf(ent, PRINT_HIGH, "You are no longer marked as idle!\n");
        }
    }

    void AdjustReplaySpeed(edict_t* ent, uint8_t oldKeyStates, uint8_t newKeyStates)
    {
        //static const std::array<std::pair<ReplaySpeed, const char*>, 19> speeds2 =
        //{
        //    std::make_pair(ReplaySpeed::Neg_100, "-100.0"),
        //    // TODO
        //};

        static const std::array<ReplaySpeed, 19> speeds = {
            ReplaySpeed::Neg_100,
            ReplaySpeed::Neg_25,
            ReplaySpeed::Neg_10,
            ReplaySpeed::Neg_5,
            ReplaySpeed::Neg_2,
            ReplaySpeed::Neg_1,
            ReplaySpeed::Neg_Half,
            ReplaySpeed::Neg_Fifth,
            ReplaySpeed::Neg_Tenth,
            ReplaySpeed::Paused,
            ReplaySpeed::Pos_Tenth,
            ReplaySpeed::Pos_Fifth,
            ReplaySpeed::Pos_Half,
            ReplaySpeed::Pos_1,
            ReplaySpeed::Pos_2,
            ReplaySpeed::Pos_5,
            ReplaySpeed::Pos_10,
            ReplaySpeed::Pos_25,
            ReplaySpeed::Pos_100,
        };

        if (ent->client == nullptr || !ent->inuse)
        {
            return;
        }
        if (!ent->client->jumpdata->update_replay_spectating)
        {
            return;
        }
        if (oldKeyStates == newKeyStates)
        {
            return;
        }

        int currentSpeedIndex = -1;
        for (size_t i = 0; i < speeds.size(); ++i)
        {
            if (speeds[i] == ent->client->jumpdata->replay_speed)
            {
                currentSpeedIndex = static_cast<int>(i);
                break;
            }
        }
        if (currentSpeedIndex == -1)
        {
            // Should never happen...
            Logger::Error(va("Player has invalid replay speed of %d",
                static_cast<int>(ent->client->jumpdata->replay_speed)));
            ent->client->jumpdata->replay_speed = ReplaySpeed::Pos_1;
            return;
        }

        bool oldJump = (oldKeyStates & static_cast<uint8_t>(Jump::KeyStateEnum::Jump)) != 0;
        bool newJump = (newKeyStates & static_cast<uint8_t>(Jump::KeyStateEnum::Jump)) != 0;
        if (!oldJump && newJump)
        {
            ent->client->jumpdata->replay_repeating = !ent->client->jumpdata->replay_repeating;
            gi.cprintf(ent, PRINT_HIGH, "Replay repeating is %s\n",
                ent->client->jumpdata->replay_repeating ? "ON" : "OFF");
        }

        bool oldForward = (oldKeyStates & static_cast<uint8_t>(Jump::KeyStateEnum::Forward)) != 0;
        bool newForward = (newKeyStates & static_cast<uint8_t>(Jump::KeyStateEnum::Forward)) != 0;
        if (!oldForward && newForward)
        {
            if (currentSpeedIndex < (speeds.size() - 1))
            {
                currentSpeedIndex++;
            }
            ent->client->jumpdata->replay_speed = speeds[currentSpeedIndex];
            if (ent->client->jumpdata->replay_speed == ReplaySpeed::Paused)
            {
                gi.cprintf(ent, PRINT_HIGH, "%s\n", GetReplaySpeedString(ent->client->jumpdata->replay_speed));
            }
            else
            {
                gi.cprintf(ent, PRINT_HIGH, "Replaying at %s speed\n",
                    GetReplaySpeedString(ent->client->jumpdata->replay_speed));
            }
            return;
        }

        bool oldBack = (oldKeyStates & static_cast<uint8_t>(Jump::KeyStateEnum::Back)) != 0;
        bool newBack = (newKeyStates & static_cast<uint8_t>(Jump::KeyStateEnum::Back)) != 0;
        if (!oldBack && newBack)
        {
            if (currentSpeedIndex > 0)
            {
                currentSpeedIndex--;
            }
            ent->client->jumpdata->replay_speed = speeds[currentSpeedIndex];
            if (ent->client->jumpdata->replay_speed == ReplaySpeed::Paused)
            {
                gi.cprintf(ent, PRINT_HIGH, "%s\n", GetReplaySpeedString(ent->client->jumpdata->replay_speed));
            }
            else
            {
                gi.cprintf(ent, PRINT_HIGH, "Replaying at %s speed\n",
                    GetReplaySpeedString(ent->client->jumpdata->replay_speed));
            }
            return;
        }
    }

    const char* GetReplaySpeedString(ReplaySpeed speed)
    {
        switch (speed)
        {
        case ReplaySpeed::Neg_100:
            return "-100.0";
        case ReplaySpeed::Neg_25:
            return "-25.0";
        case ReplaySpeed::Neg_10:
            return "-10.0";
        case ReplaySpeed::Neg_5:
            return "-5.0";
        case ReplaySpeed::Neg_2:
            return "-2.0";
        case ReplaySpeed::Neg_1:
            return "-1.0";
        case ReplaySpeed::Neg_Half:
            return "-0.5";
        case ReplaySpeed::Neg_Fifth:
            return "-0.2";
        case ReplaySpeed::Neg_Tenth:
            return "-0.1";
        case ReplaySpeed::Paused:
            return "Paused";
        case ReplaySpeed::Pos_Tenth:
            return "0.1";
        case ReplaySpeed::Pos_Fifth:
            return "0.2";
        case ReplaySpeed::Pos_Half:
            return "0.5";
        case ReplaySpeed::Pos_1:
            return "1.0";
        case ReplaySpeed::Pos_2:
            return "2.0";
        case ReplaySpeed::Pos_5:
            return "5.0";
        case ReplaySpeed::Pos_10:
            return "10.0";
        case ReplaySpeed::Pos_25:
            return "25.0";
        case ReplaySpeed::Pos_100:
            return "100.0";
        default:
            return "";
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