#pragma once

namespace Jump
{
    // Returns a string of the passed in token: STRINGIFY(STAT_JUMP_KEY_FORWARD) -> "STAT_JUMP_KEY_FORWARD"
    #define STRINGIFY(s) #s

    // Returns a string of the value of the passed in token: XSTRINGIFY(STAT_JUMP_KEY_FORWARD) -> "18"
    #define XSTRINGIFY(s) STRINGIFY(s)

    // TODO voting string for HUD

    // q_shared.h defines stat strings 0-17.  We can overlap with those since we are setting everything
    // in the HUD and not relying on base game logic, but try to avoid it.

    #define STAT_JUMP_HUD_FOOTER_1 15   // set client-specific configstring for "Team: <team>"
    #define STAT_JUMP_HUD_FOOTER_2 16   // set client-specific configstring for "Race: n/NOW" or "Replay: n/NOW"
    #define STAT_JUMP_HUD_FOOTER_3 17   // set client-specific configstring for "Chkpts: n/n"
    #define STAT_JUMP_CLIENT_TRACE 18   // set client-specific configstring for the name of the player in the crosshairs
    #define STAT_JUMP_SPEED 19          // (int) speed
    #define STAT_JUMP_KEY_FORWARD 20    // (bool) visible or not
    #define STAT_JUMP_KEY_BACK 21       // (bool) visible or not
    #define STAT_JUMP_KEY_LEFT 22       // (bool) visible or not
    #define STAT_JUMP_KEY_RIGHT 23      // (bool) visible or not
    #define STAT_JUMP_KEY_JUMP 24       // (bool) visible or not
    #define STAT_JUMP_KEY_CROUCH 25     // (bool) visible or not
    #define STAT_JUMP_KEY_ATTACK 26     // (bool) visible or not
    #define STAT_JUMP_FPS 27            // (int) FPS
    #define STAT_JUMP_TIMER_SECONDS 28  // (int) timer (seconds) 
    #define STAT_JUMP_TIMER_MS 29       // (int) timer (milliseconds)
    #define STAT_JUMP_WEAPON 30         // (pcx) set to the equipped weapon pcx
    #define STAT_JUMP_TIME_LEFT 31      // (int) time left in map (minutes)
    // MaxStats is currently set to 32 in q_shared.h, can only use 0-31.
    // To enable more, we would need to update the client as well.

    // Config strings IDs
    #define CS_JUMP_EMPTY 1000
    #define CS_JUMP_TEAM_EASY 1001
    #define CS_JUMP_TEAM_HARD 1002
    #define CS_JUMP_TEAM_SPEC 1003
    #define CS_JUMP_KEY_FORWARD 1004
    #define CS_JUMP_KEY_BACK 1005
    #define CS_JUMP_KEY_LEFT 1006
    #define CS_JUMP_KEY_RIGHT 1007
    #define CS_JUMP_KEY_JUMP 1008
    #define CS_JUMP_KEY_CROUCH 1009
    #define CS_JUMP_KEY_ATTACK 1010
    #define CS_JUMP_FPS 1011

    // Config string strings
    #define CS_STR_JUMP_EMPTY ""
    #define CS_STR_JUMP_TEAM_EASY "Easy" // TODO needs to be green
    #define CS_STR_JUMP_TEAM_HARD "Hard"
    #define CS_STR_JUMP_TEAM_SPEC "Observer"
    #define CS_STR_JUMP_KEY_FORWARD "Forward"
    #define CS_STR_JUMP_KEY_BACK "Back"
    #define CS_STR_JUMP_KEY_LEFT "Left"
    #define CS_STR_JUMP_KEY_RIGHT "Right"
    #define CS_STR_JUMP_KEY_JUMP "JUMP!"
    #define CS_STR_JUMP_KEY_CROUCH "DUCK DUCK"
    #define CS_STR_JUMP_KEY_ATTACK "Attack"
    #define CS_STR_JUMP_FPS "FPS"

    static const char* Jump_Status_Bar =
        //
        // Note: characters seem to be in units of 8.
        // xl: x units from the left of the screen.
        // xr: x units from the right of the screen.
        // yt: y units from the top of the screen.
        // yb: y units from the bottom of the screen.
        //
        // string "Texthere" is white text
        // string2 "Texthere" is green text
        //
        // Forward key
        "if " XSTRINGIFY(STAT_JUMP_KEY_FORWARD) " "
        "xl 16 "
        "yb -40 "
        "string \"Forward\" "
        "endif "
        // Left key
        "xl 0 "
        "yb -32 "
        "stat_string " XSTRINGIFY(STAT_JUMP_KEY_LEFT)
        " "
        // Right key
        "xl 48 "
        "yb -32 "
        "stat_string " XSTRINGIFY(STAT_JUMP_KEY_RIGHT)
        " "
        // Back key
        "xl 24 "
        "yb -24 "
        "stat_string " XSTRINGIFY(STAT_JUMP_KEY_BACK)
        " "
        // Jump key
        "xl 24 "
        "yb -48 "
        "stat_string " XSTRINGIFY(STAT_JUMP_KEY_JUMP)
        " "
        // Crouch key
        "xl 8 "
        "yb -16 "
        "stat_string " XSTRINGIFY(STAT_JUMP_KEY_CROUCH)
        " "
        // Attack key
        "xl 16 "
        "yb -56 "
        "stat_string " XSTRINGIFY(STAT_JUMP_KEY_ATTACK)
        " "

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
        "num 5 " XSTRINGIFY(STAT_JUMP_TIMER_SECONDS) // display at most 5 digits, right aligned
        " "
        // Period separator
        "xr -24 "
        "yb -16 "
        "string2 \".\""
        " "
        // Milliseconds
        "xr -18 "
        "yb -32 "
        "num 1 " XSTRINGIFY(STAT_JUMP_TIMER_MS)
        " "
    ;

    void SetConfigStrings();
    void SetStats(edict_t* ent);
    void UpdateTimer(edict_t* ent);



    static const char* Jump_Status_Bar2 =
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
        "xr - 32 "
        "yb -8 "
        "string2 \"Time\" "
        ""
        // List of current and previously played maps
        "xr -128 "
        "yt 2 "
        "string2 \"%s\" " // current map
        "yt 10 "
        "string \"%s\" " // last map 1
        "yt 18 "
        "string \"%s\" " // last map 2
        "yt 26 "
        "string \"%s\" " // last map 3
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
        "xr -128 "
        "yt 84 "
        "string \"%s\" "
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
        // Team
        "xv 72 "
        "yb -32 "
        "string \"Team: \" "
        "xv 120 "
        "stat_string " XSTRINGIFY(STAT_JUMP_TEAM) " "
        ""
        // Current replay as observer
        // TODO Replay: n and Race: n show up in the same spot depending on if you are observer or hard team
        // See jumpmod.c hud_footer() to see how to do this.
        // Basically you can send a configstring update to just a specific client,
        // so this allows the config strings to differ between clients.
    ;

    static const char* GetLayoutString()
    {
        std::string current_map = level.mapname;
        std::string last_map1 = "slipmap10";
        std::string last_map2 = "ddrace";
        std::string last_map3 = "012345678912345";
        int total_maps = 3045;
        std::string time_added = "+750";
        static char buffer[1024];
        snprintf(buffer, sizeof(buffer), Jump_Status_Bar2,
            current_map.c_str(), last_map1.c_str(), last_map2.c_str(), last_map3.c_str(),
            total_maps,
            time_added.c_str());
        return buffer;
    }

}
