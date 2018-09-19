
#include "jump_cmds.h"
#include "jump.h"
#include "jump_types.h"

namespace Jump
{
    bool JumpClientCommand(edict_t* ent)
    {
        char* cmd = gi.argv(0);

        if (Q_stricmp(cmd, "inven") == 0)
        {
            Cmd_Jump_Inven(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "noclip") == 0)
        {
            Cmd_Jump_Noclip(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "test") == 0)
        {
            Cmd_Jump_Test(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "kill") == 0)
        {
            Cmd_Jump_Kill(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "recall") == 0)
        {
            Cmd_Jump_Recall(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "store") == 0)
        {
            Cmd_Jump_Store(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "reset") == 0)
        {
            Cmd_Jump_Reset(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "replay") == 0)
        {
            Cmd_Jump_Replay(ent);
            return true;
        }
        else
        {
            return false;
        }
    }

    // A function used to test stuff for development
    void Cmd_Jump_Test(edict_t* ent)
    {
        if (ent->client->menu)
        {
            PMenu_Close(ent);
        }
        ent->client->showscores = true;

        char string[1400] = { 0 };

        Com_sprintf(string, sizeof(string),
            "xv -16 yv 0 string2 \"Ping Pos Player          Best Comp Maps     %%\" ");

        gi.WriteByte(svc_layout);
        gi.WriteString(string);
        gi.unicast(ent, true);
    }

    void Cmd_Jump_Inven(edict_t* ent)
    {
        if (ent->client->menu) {
            PMenu_Close(ent);
            ent->client->update_chase = true; // TODO what is this for?
        }
        else
        {
            OpenMenu_Join(ent);
        }
    }

    void Cmd_Jump_Noclip(edict_t* ent)
    {
        if (ent->client->resp.jump_team == TEAM_EASY)
        {
            if (ent->movetype == MOVETYPE_NOCLIP)
            {
                ent->movetype = MOVETYPE_WALK;
                gi.cprintf(ent, PRINT_HIGH, "noclip OFF\n");
            }
            else
            {
                ent->movetype = MOVETYPE_NOCLIP;
                gi.cprintf(ent, PRINT_HIGH, "noclip ON\n");
            }
        }
    }

    void Cmd_Jump_Kill(edict_t* ent)
    {
        SpawnForJumping(ent);
    }

    void Cmd_Jump_Recall(edict_t* ent)
    {
        if (ent->client->resp.jump_team == TEAM_EASY)
        {
            if (ent->client->store_buffer.HasStore())
            {
                int num = 1;
                if (gi.argc() >= 2)
                {
                    num = atoi(gi.argv(1));
                    if (num <= 0)
                    {
                        num = 1; // convert "recall 0" into "recall 1"
                    }
                }
                store_data_t data = ent->client->store_buffer.GetStore(num);
                SpawnAtStorePosition(ent, data);
            }
            else
            {
                SpawnForJumping(ent);
            }
        }
        else
        {
            SpawnForJumping(ent);
        }
    }

    void Cmd_Jump_Store(edict_t* ent)
    {
        // Save all of the state
        store_data_t data = { 0 };
        data.time_interval = Sys_Milliseconds() - ent->client->resp.jump_timer_begin;
        VectorCopy(ent->s.origin, data.pos);
        VectorCopy(ent->s.angles, data.angles);
        data.angles[ROLL] = 0; // fixes the problem where the view is tilted after recall
        ent->client->store_buffer.PushStore(data);

        // Place an entity at the position
        if (ent->client->store_ent)
        {
            G_FreeEdict(ent->client->store_ent);
            ent->client->store_ent = NULL;
        }
        ent->client->store_ent = G_Spawn();
        VectorCopy(data.pos, ent->client->store_ent->s.origin);
        VectorCopy(data.pos, ent->client->store_ent->s.old_origin);
        ent->client->store_ent->s.old_origin[2] -= 10;
        ent->client->store_ent->s.origin[2] -= 10;
        ent->client->store_ent->svflags = SVF_PROJECTILE;
        VectorCopy(data.angles, ent->client->store_ent->s.angles);
        ent->client->store_ent->movetype = MOVETYPE_NONE;
        ent->client->store_ent->clipmask = MASK_PLAYERSOLID;
        ent->client->store_ent->solid = SOLID_NOT;
        ent->client->store_ent->s.renderfx = RF_TRANSLUCENT;
        VectorClear(ent->client->store_ent->mins);
        VectorClear(ent->client->store_ent->maxs);
        ent->client->store_ent->s.modelindex = gi.modelindex(STORE_MODEL);
        ent->client->store_ent->dmg = 0;
        ent->client->store_ent->classname = "store_ent";
        gi.linkentity(ent->client->store_ent);
    }

    void Cmd_Jump_Reset(edict_t* ent)
    {
        ent->client->store_buffer.Reset();
        if (ent->client->store_ent)
        {
            G_FreeEdict(ent->client->store_ent);
            ent->client->store_ent = NULL;
        }
    }

    void Cmd_Jump_Replay(edict_t* ent)
    {
        // Right now we only support "replay now"
        if (Q_stricmp(gi.argv(1), "now") == 0)
        {
            if (level.replay_fastest_time <= 0)
            {
                gi.cprintf(ent, PRINT_HIGH, "No time set\n");
            }
            else
            {
                ResetJumpTimer(ent); // is this needed?
                ClearReplayData(ent); // is this needed?

                // Move client to a spectator
                InitAsSpectator(ent);

                // Set to replay state
                ent->client->replay_current_frame = 0;
                ent->client->update_replay = true;

                gi.cprintf(ent, PRINT_HIGH, "Replaying %s with a time of %d.%03d seconds\n",
                    level.replay_fastest_name, 
                    level.replay_fastest_time / 1000, 
                    level.replay_fastest_time % 1000
                );
            }
        }
    }
}