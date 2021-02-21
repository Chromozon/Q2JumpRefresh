#pragma once

namespace Jump
{
    // Returns a string of the passed in token: STRINGIFY(STAT_JUMP_KEY_FORWARD) -> "STAT_JUMP_KEY_FORWARD"
    #define STRINGIFY(s) #s

    // Returns a string of the value of the passed in token: XSTRINGIFY(STAT_JUMP_KEY_FORWARD) -> "20"
    #define XSTRINGIFY(s) STRINGIFY(s)

    // STATs are values for HUD tokens which can be updated each server frame
    #define STAT_JUMP_AVAILABLE_FOR_USE     0
    #define STAT_JUMP_AVAILABLE_FOR_USE     1
    #define STAT_JUMP_AVAILABLE_FOR_USE     2
    #define STAT_JUMP_AVAILABLE_FOR_USE     3
    #define STAT_JUMP_AVAILABLE_FOR_USE     4
    #define STAT_JUMP_AVAILABLE_FOR_USE     5
    #define STAT_JUMP_AVAILABLE_FOR_USE     6
    #define STAT_JUMP_AVAILABLE_FOR_USE     7
    #define STAT_JUMP_ASYNC_0               8  // (int) 0 or 1
    #define STAT_JUMP_ASYNC_1               9  // (int) 0 or 1
    #define STAT_JUMP_HUD_VOTE_CAST         10 // (configstring) for "Votes: X of X" 
    #define STAT_JUMP_HUD_VOTE_REMAINING    11 // (configstring) for "X remaining"
    #define STAT_JUMP_HUD_VOTE_TYPE         12 // (configstring) for short vote description (eg. "Map: ddrace")
    // IMPORTANT: DO NOT USE 13, STAT_LAYOUTS IS HARDCODED IN CLIENTS.
    #define STAT_JUMP_HUD_VOTE_INITIATED    14 // (configstring) for "Vote by X"
    #define STAT_JUMP_HUD_FOOTER_1          15 // (configstring) set client-specific string for "Team: <team>"
    #define STAT_JUMP_HUD_FOOTER_2          16 // (configstring) set client-specific string for "Race: n/NOW" or "Replay: n/NOW"
    #define STAT_JUMP_HUD_FOOTER_3          17 // (configstring) set client-specific string for "Chkpts: n/n"
    #define STAT_JUMP_CLIENT_TRACE          18 // (configstring) set client-specific string for the name of the player in the crosshairs
    #define STAT_JUMP_SPEED                 19 // (int) speed in units/s
    #define STAT_JUMP_KEY_FORWARD           20 // (bool) visible or not, TODO: can change to those ugly ugly brown pcx
    #define STAT_JUMP_KEY_BACK              21 // (bool) visible or not
    #define STAT_JUMP_KEY_LEFT              22 // (bool) visible or not
    #define STAT_JUMP_KEY_RIGHT             23 // (bool) visible or not
    #define STAT_JUMP_KEY_JUMP              24 // (bool) visible or not
    #define STAT_JUMP_KEY_CROUCH            25 // (bool) visible or not
    #define STAT_JUMP_KEY_ATTACK            26 // (bool) visible or not
    #define STAT_JUMP_FPS                   27 // (int) FPS
    #define STAT_JUMP_TIMER_SECONDS         28 // (int) timer (seconds) 
    #define STAT_JUMP_TIMER_MS              29 // (int) timer (milliseconds)
    #define STAT_JUMP_WEAPON_ICON           30 // (pcx) set to the equipped weapon pcx
    #define STAT_JUMP_TIME_LEFT             31 // (int) time left in map (minutes)
    // MaxStats is currently set to 32 in q_shared.h, can only use 0-31.
    // To enable more, we would need to update the client as well.

    // Config strings IDs
    #define CS_JUMP_KEY_EMPTY 1000
    #define CS_JUMP_KEY_HUD_FOOTER_1 1001
    #define CS_JUMP_KEY_HUD_FOOTER_2 1002
    #define CS_JUMP_KEY_HUD_FOOTER_3 1003
    #define CS_JUMP_KEY_HUD_VOTE_INITIATED 1004
    #define CS_JUMP_KEY_HUD_VOTE_TYPE 1005
    #define CS_JUMP_KEY_HUD_VOTE_CAST 1006
    #define CS_JUMP_KEY_HUD_VOTE_REMAINING 1007

    // Config string strings
    #define CS_JUMP_VAL_EMPTY ""

    void SetConfigStrings();
    void SetStats(edict_t* ent);
    void UpdateTimer(edict_t* ent);
    const char* GetLayoutString();
}
