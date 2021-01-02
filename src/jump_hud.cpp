
#include "g_local.h"
#include "jump_hud.h"
#include "jump_utils.h"

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
        gi.configstring(CS_JUMP_KEY_ATTACK, CS_STR_JUMP_KEY_ATTACK);
        gi.configstring(CS_JUMP_TEAM_EASY, const_cast<char*>(GetGreenConsoleText("Easy").c_str()));
        gi.configstring(CS_JUMP_TEAM_HARD, const_cast<char*>(GetGreenConsoleText("Hard").c_str()));
        gi.configstring(CS_JUMP_TEAM_SPEC, const_cast<char*>(GetGreenConsoleText("Observer").c_str()));

        gi.configstring(CS_STATUSBAR, const_cast<char*>(Jump_Status_Bar));
    }

    void SetStats(edict_t* ent)
    {
        //#define STAT_JUMP_SPEED 15
        //#define STAT_JUMP_RACE_NUM 16
        //#define STAT_JUMP_REPLAY_SPEC_NUM 17
        //#define STAT_JUMP_KEY_FORWARD 18
        //#define STAT_JUMP_KEY_BACK 19
        //#define STAT_JUMP_KEY_LEFT 20
        //#define STAT_JUMP_KEY_RIGHT 21
        //#define STAT_JUMP_KEY_JUMP 22
        //#define STAT_JUMP_KEY_CROUCH 23
        //#define STAT_JUMP_KEY_ATTACK 24
        //#define STAT_JUMP_FPS 25
        //#define STAT_JUMP_TIMER_SECONDS 26
        //#define STAT_JUMP_TIMER_MS 27
        //#define STAT_JUMP_TEAM 28
        //#define STAT_JUMP_WEAPON 29
        //#define STAT_JUMP_TIME_LEFT 30

        ent->client->ps.stats[STAT_JUMP_SPEED] = 0; // TODO speedometer
        //ent->client->ps.stats[] = 0;
        //ent->client->ps.stats[] = 0;
        //ent->client->ps.stats[] = 0;
        //ent->client->ps.stats[] = 0;
        //ent->client->ps.stats[] = 0;
        //ent->client->ps.stats[] = 0;
        //ent->client->ps.stats[] = 0;

        ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = 0;
        ent->client->ps.stats[STAT_JUMP_KEY_BACK] = 0;
        ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = 0;
        ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = 0;
        ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = 0;
        ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = 0;
        ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = 0;

        if (ent->client->jumpdata->key_states & KEY_STATE_FORWARD)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = 1;
        }
        else if (ent->client->jumpdata->key_states & KEY_STATE_BACK)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_BACK] = 1;
        }

        if (ent->client->jumpdata->key_states & KEY_STATE_LEFT)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = 1;
        }
        else if (ent->client->jumpdata->key_states & KEY_STATE_RIGHT)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = 1;
        }

        if (ent->client->jumpdata->key_states & KEY_STATE_JUMP)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = 1;
        }
        else if (ent->client->jumpdata->key_states & KEY_STATE_CROUCH)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = 1;
        }

        if (ent->client->jumpdata->key_states & KEY_STATE_ATTACK)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = 1;
        }

        ent->client->ps.stats[STAT_JUMP_FPS] = ent->client->jumpdata->fps;

        UpdateTimer(ent);

        //if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
        //    ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex(ent->client->pers.weapon->icon);
        //else
        //    ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
    }

    void UpdateTimer(edict_t * ent)
    {
        if (ent->client->jumpdata->timer_paused)
        {
            ent->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = 0;
            ent->client->ps.stats[STAT_JUMP_TIMER_MS] = 0;
        }
        else if (ent->client->jumpdata->timer_finished)
        {
            int time = ent->client->jumpdata->timer_end - ent->client->jumpdata->timer_begin;
            ent->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = time / 1000;
            ent->client->ps.stats[STAT_JUMP_TIMER_MS] = (time % 1000) / 100;
        }
        else // active timer
        {
            int time = Sys_Milliseconds() - ent->client->jumpdata->timer_begin;
            ent->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = time / 1000;
            ent->client->ps.stats[STAT_JUMP_TIMER_MS] = (time % 1000) / 100;
        }
    }
}