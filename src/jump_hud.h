#pragma once

namespace Jump
{
    #define STRINGIFY(s) #s
    #define XSTRINGIFY(s) STRINGIFY(s)

    #define STAT_JUMP_KEY_FORWARD 18
    #define STAT_JUMP_KEY_BACK 19
    #define STAT_JUMP_KEY_LEFT 20
    #define STAT_JUMP_KEY_RIGHT 21
    #define STAT_JUMP_KEY_JUMP 22
    #define STAT_JUMP_KEY_CROUCH 23
    #define STAT_JUMP_FPS 24
    #define STAT_JUMP_TIMER_SECONDS 25
    #define STAT_JUMP_TIMER_MS 26
    // MaxStats is currently set to 32 in q_shared.h

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
    #define CS_JUMP_FPS 1010

    // Config string strings
    #define CS_STR_JUMP_EMPTY ""
    #define CS_STR_JUMP_TEAM_EASY "Easy"
    #define CS_STR_JUMP_TEAM_HARD "Hard"
    #define CS_STR_JUMP_TEAM_SPEC "Observer"
    #define CS_STR_JUMP_KEY_FORWARD "Forward"
    #define CS_STR_JUMP_KEY_BACK "Back"
    #define CS_STR_JUMP_KEY_LEFT "Left"
    #define CS_STR_JUMP_KEY_RIGHT "Right"
    #define CS_STR_JUMP_KEY_JUMP "JUMP!"
    #define CS_STR_JUMP_KEY_CROUCH "DUCK DUCK"
    #define CS_STR_JUMP_FPS "FPS"

    static const char* Jump_Status_Bar =
        //
        // Note: characters seem to be in units of 8.
        // xl: x units from the left of the screen.
        // xr: x units from the right of the screen.
        // yb: y units from the bottom of the screen.
        //
        // Forward key
        "xl 16 "
        "yb -40 "
        "stat_string " XSTRINGIFY(STAT_JUMP_KEY_FORWARD)
        " "
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

        // Timer
        //
        // It seems that numbers need 16 units of space per digit.
        // The maximum displayed number is int16 max (32767).
        // Exceeding the max will cause rollover to a negative value.
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
}
