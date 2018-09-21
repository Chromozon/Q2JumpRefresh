
#include "g_local.h"
#include "jump_hud.h"

namespace Jump
{
    void SetConfigStrings()
    {
        gi.configstring(CS_JUMP_EMPTY, CS_STR_JUMP_EMPTY);
        gi.configstring(CS_JUMP_KEY_FORWARD, CS_STR_JUMP_KEY_FORWARD);
        gi.configstring(CS_JUMP_KEY_BACK, CS_STR_JUMP_KEY_BACK);
        gi.configstring(CS_JUMP_KEY_LEFT, CS_STR_JUMP_KEY_LEFT);
        gi.configstring(CS_JUMP_KEY_RIGHT, CS_STR_JUMP_KEY_RIGHT);
        gi.configstring(CS_JUMP_KEY_JUMP, CS_STR_JUMP_KEY_JUMP);
        gi.configstring(CS_JUMP_KEY_CROUCH, CS_STR_JUMP_KEY_CROUCH);
        gi.configstring(CS_JUMP_FPS, CS_STR_JUMP_FPS);
        //gi.configstring(CS_JUMP_TEAM_EASY, "    Team: ≈бущ");
        //gi.configstring(CS_JUMP_TEAM_HARD, "    Team: »бтд");
        //gi.configstring(CS_JUMP_TEAM_SPEC, "    Team: ѕвуетцет");
        gi.configstring(CS_STATUSBAR, const_cast<char*>(Jump_Status_Bar));
    }

    void SetStats(edict_t* ent)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = CS_JUMP_EMPTY;
        ent->client->ps.stats[STAT_JUMP_KEY_BACK] = CS_JUMP_EMPTY;
        ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = CS_JUMP_EMPTY;
        ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = CS_JUMP_EMPTY;
        ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = CS_JUMP_EMPTY;
        ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = CS_JUMP_EMPTY;

        if (ent->client->key_states & KEY_STATE_FORWARD)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = CS_JUMP_KEY_FORWARD;
        }
        else if (ent->client->key_states & KEY_STATE_BACK)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_BACK] = CS_JUMP_KEY_BACK;
        }

        if (ent->client->key_states & KEY_STATE_LEFT)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = CS_JUMP_KEY_LEFT;
        }
        else if (ent->client->key_states & KEY_STATE_RIGHT)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = CS_JUMP_KEY_RIGHT;
        }

        if (ent->client->key_states & KEY_STATE_JUMP)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = CS_JUMP_KEY_JUMP;
        }
        else if (ent->client->key_states & KEY_STATE_CROUCH)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = CS_JUMP_KEY_CROUCH;
        }

        ent->client->ps.stats[STAT_JUMP_FPS] = ent->client->pers.fps;

        UpdateTimer(ent);
    }

    void UpdateTimer(edict_t * ent)
    {
        if (ent->client->resp.jump_timer_paused)
        {
            ent->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = 0;
            ent->client->ps.stats[STAT_JUMP_TIMER_MS] = 0;
        }
        else if (ent->client->resp.jump_timer_finished)
        {
            int time = ent->client->resp.jump_timer_end - ent->client->resp.jump_timer_begin;
            ent->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = time / 1000;
            ent->client->ps.stats[STAT_JUMP_TIMER_MS] = (time % 1000) / 100;
        }
        else // active timer
        {
            int time = Sys_Milliseconds() - ent->client->resp.jump_timer_begin;
            ent->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = time / 1000;
            ent->client->ps.stats[STAT_JUMP_TIMER_MS] = (time % 1000) / 100;
        }
    }
}