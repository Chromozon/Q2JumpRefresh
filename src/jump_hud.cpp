
#include "g_local.h"
#include "jump_hud.h"
#include "jump_utils.h"

namespace Jump
{
    static const char* hud_layout =
        //
        // Note: characters seem to be in units of 8.
        // xl: x units from the left of the screen.
        // xr: x units from the right of the screen (value must be negative).
        // xv: (screen width / 2) - 160 + value
        // yt: y units from the top of the screen.
        // yb: y units from the bottom of the screen (value must be negative).
        // yv: (screen height / 2) - 120 + value
        //
        // string "Texthere" is white text
        // string2 "Texthere" is green text
        //
        // Attack key
        "if " XSTRINGIFY(STAT_JUMP_KEY_ATTACK) " "
        "xl 16 "
        "yb -56 "
        "string \"Attack\" "
        "endif "
        // Jump key
        "if " XSTRINGIFY(STAT_JUMP_KEY_JUMP) " "
        "xl 24 "
        "yb -48 "
        "string \"JUMP!\" "
        "endif "
        // Forward key
        "if " XSTRINGIFY(STAT_JUMP_KEY_FORWARD) " "
        "xl 16 "
        "yb -40 "
        "string \"Forward\" "
        "endif "
        // Left key
        "if " XSTRINGIFY(STAT_JUMP_KEY_LEFT) " "
        "xl 0 "
        "yb -32 "
        "string \"Left\" "
        "endif "
        // Right key
        "if " XSTRINGIFY(STAT_JUMP_KEY_RIGHT) " "
        "xl 48 "
        "yb -32 "
        "string \"Right\" "
        "endif "
        // Back key
        "if " XSTRINGIFY(STAT_JUMP_KEY_BACK) " "
        "xl 24 "
        "yb -24 "
        "string \"Back\" "
        "endif "
        // Crouch key
        "if " XSTRINGIFY(STAT_JUMP_KEY_CROUCH) " "
        "xl 8 "
        "yb -16 "
        "string \"DUCK DUCK\" "
        "endif "

        // Timer
        //
        // It seems that numbers need 16 units of space per digit.
        // The maximum displayed number is int16 max (32767).
        // Exceeding the max will cause rollover to a negative value.
        // TODO: we can specify each digit individually to have infinite time displayed, or
        // we can change from a number to a string value.
        //
        // Seconds
        "xr -108 "
        "yb -32 "
        "num 5 " XSTRINGIFY(STAT_JUMP_TIMER_SECONDS) " " // display at most 5 digits, right aligned
        ""
        // Period separator
        "xr -24 "
        "yb -16 "
        "string2 \".\" "
        ""
        // Milliseconds
        "xr -18 "
        "yb -32 "
        "num 1 " XSTRINGIFY(STAT_JUMP_TIMER_MS) " "
        ""
        // Time label on bottom
        "xr -32 "
        "yb -8 "
        "string2 \"Time\" "
        ""
        // List of current and previously played maps
        // Note: max mapname length is MAX_QPATH, so 63 characters.
        // None of the mapnames are anywhere near that long, so just use 32 characters.
        "xr -256 "
        "yt 2 "
        "string2 \"%32s\" " // current map
        "yt 10 "
        "string \"%32s\" " // last map 1
        "yt 18 "
        "string \"%32s\" " // last map 2
        "yt 26 "
        "string \"%32s\" " // last map 3
        ""
        // Number of maps on the server
        "xr -32 "
        "yt 42 "
        "string \"%d\" "
        "yt 50 "
        "string \"Maps\" "
        ""
        // Time left on current map
        "xr -32 "
        "yt 64 "
        "string2 \"Time\" "
        "xr	-50 "
        "yt 74 "
        "num 3 " XSTRINGIFY(STAT_JUMP_TIME_LEFT) " "
        ""
        // Time added
        "xr -32 "
        "yt 100 "
        "string \"%4s\" "
        ""
        // FPS
        "if " XSTRINGIFY(STAT_JUMP_FPS) " "
        "xl 0 "
        "yb -76 "
        "num 3 " XSTRINGIFY(STAT_JUMP_FPS) " "
        "xl 54 "
        "yb -60 "
        "string2 \"FPS\" "
        "endif "
        ""
        // Speedometer
        "if " XSTRINGIFY(STAT_JUMP_SPEED) " "
        "yb -32 "
        "xv 200 "
        "num 4 " XSTRINGIFY(STAT_JUMP_SPEED) " "
        "xv 226 "
        "yb -8 "
        "string2 \"Speed\" "
        "endif "
        ""
        // TODO footer strings (team, replay, race, chkpts)
        // Current replay as observer
        // TODO Replay: n and Race: n show up in the same spot depending on if you are observer or hard team
        // See jumpmod.c hud_footer() to see how to do this.
        // Basically you can send a configstring update to just a specific client,
        // so this allows the config strings to differ between clients.
        ;

    void SetConfigStrings()
    {
        gi.configstring(CS_JUMP_KEY_EMPTY, CS_JUMP_VAL_EMPTY);
        gi.configstring(CS_STATUSBAR, const_cast<char*>(GetLayoutString()));
    }

    void SetStats(edict_t* ent)
    {
        //#define STAT_JUMP_HUD_FOOTER_1 15   // set client-specific configstring for "Team: <team>"
        //#define STAT_JUMP_HUD_FOOTER_2 16   // set client-specific configstring for "Race: n/NOW" or "Replay: n/NOW"
        //#define STAT_JUMP_HUD_FOOTER_3 17   // set client-specific configstring for "Chkpts: n/n"
        //#define STAT_JUMP_CLIENT_TRACE 18   // set client-specific configstring for the name of the player in the crosshairs
        //#define STAT_JUMP_SPEED 19          // (int) speed
        //#define STAT_JUMP_FPS 27            // (int) FPS
        //#define STAT_JUMP_TIMER_SECONDS 28  // (int) timer (seconds) 
        //#define STAT_JUMP_TIMER_MS 29       // (int) timer (milliseconds)
        //#define STAT_JUMP_WEAPON 30         // (pcx) set to the equipped weapon pcx
        //#define STAT_JUMP_TIME_LEFT 31      // (int) time left in map (minutes)

        vec3_t velocity;
        velocity[0] = ent->velocity[0];
        velocity[1] = ent->velocity[1];
        velocity[2] = 0;
        ent->client->ps.stats[STAT_JUMP_SPEED] = static_cast<short>(VectorNormalize(velocity));

        ent->client->ps.stats[STAT_JUMP_CLIENT_TRACE] = 0; // TODO name of person user is looking at
        ent->client->ps.stats[STAT_JUMP_HUD_FOOTER_1] = 0;
        ent->client->ps.stats[STAT_JUMP_HUD_FOOTER_2] = 0;
        ent->client->ps.stats[STAT_JUMP_HUD_FOOTER_3] = 0;

        ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = false;
        ent->client->ps.stats[STAT_JUMP_KEY_BACK] = false;
        ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = false;
        ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = false;
        ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = false;
        ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = false;
        ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = false;

        if (ent->client->jumpdata->key_states & KEY_STATE_FORWARD)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = true;
        }
        else if (ent->client->jumpdata->key_states & KEY_STATE_BACK)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_BACK] = true;
        }

        if (ent->client->jumpdata->key_states & KEY_STATE_LEFT)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = true;
        }
        else if (ent->client->jumpdata->key_states & KEY_STATE_RIGHT)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = true;
        }

        if (ent->client->jumpdata->key_states & KEY_STATE_JUMP)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = true;
        }
        else if (ent->client->jumpdata->key_states & KEY_STATE_CROUCH)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = true;
        }

        if (ent->client->jumpdata->key_states & KEY_STATE_ATTACK)
        {
            ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = true;
        }

        ent->client->ps.stats[STAT_JUMP_FPS] = static_cast<short>(ent->client->jumpdata->fps);
        
        // TODO: time left
        // time left = mset_var(timelimit) * 60 + timeadded * 60 - level.time
        // level.time is how many seconds elapsed since map change (starts at 0 and counts up)
        ent->client->ps.stats[STAT_JUMP_TIME_LEFT] = 523;

        UpdateTimer(ent);

        //if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
        //#define WEAP_BLASTER			1 
        //#define WEAP_SHOTGUN			2 
        //#define WEAP_SUPERSHOTGUN		3 
        //#define WEAP_MACHINEGUN			4 
        //#define WEAP_CHAINGUN			5 
        //#define WEAP_GRENADES			6 
        //#define WEAP_GRENADELAUNCHER	7 
        //#define WEAP_ROCKETLAUNCHER		8 
        //#define WEAP_HYPERBLASTER		9 
        //#define WEAP_RAILGUN			10
        //#define WEAP_BFG				11

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
            int64_t time = ent->client->jumpdata->timer_end - ent->client->jumpdata->timer_begin;
            ent->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = static_cast<short>(time / 1000);
            ent->client->ps.stats[STAT_JUMP_TIMER_MS] = static_cast<short>((time % 1000) / 100);
        }
        else // active timer
        {
            // We can only show 32767 seconds (~9 hrs) on the timer
            int64_t time = Sys_Milliseconds() - ent->client->jumpdata->timer_begin;
            ent->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = static_cast<short>(time / 1000);
            ent->client->ps.stats[STAT_JUMP_TIMER_MS] = static_cast<short>((time % 1000) / 100);
        }
    }

    const char* GetLayoutString()
    {
        std::string current_map = level.mapname;
        std::string last_map1 = "slipmap10";
        std::string last_map2 = "ddrace";
        std::string last_map3 = "012345678912345";
        int total_maps = 3045;
        std::string time_added = "+750";
        static char buffer[1024];
        snprintf(buffer, sizeof(buffer), hud_layout,
            current_map.c_str(), last_map1.c_str(), last_map2.c_str(), last_map3.c_str(),
            total_maps,
            time_added.c_str());
        return buffer;
    }
}