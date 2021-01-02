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
    #define CS_JUMP_KEY_EMPTY 1000
    #define CS_JUMP_KEY_HUD_FOOTER_1 1001
    #define CS_JUMP_KEY_HUD_FOOTER_2 1002
    #define CS_JUMP_KEY_HUD_FOOTER_3 1003

    // Config string strings
    #define CS_JUMP_VAL_EMPTY ""

    void SetConfigStrings();
    void SetStats(edict_t* ent);
    void UpdateTimer(edict_t* ent);
    const char* GetLayoutString();
}
