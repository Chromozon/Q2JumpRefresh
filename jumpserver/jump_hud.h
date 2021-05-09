#pragma once

namespace Jump
{
// Returns a string of the passed in token: STRINGIFY(STAT_JUMP_KEY_FORWARD) -> "STAT_JUMP_KEY_FORWARD"
#define STRINGIFY(s) #s

// Returns a string of the value of the passed in token: XSTRINGIFY(STAT_JUMP_KEY_FORWARD) -> "20"
#define XSTRINGIFY(s) STRINGIFY(s)

// STATs are values for HUD tokens which can be updated each server frame for each client
#define STAT_JUMP_AVAILABLE_FOR_USE0    0
#define STAT_JUMP_AVAILABLE_FOR_USE1    1
#define STAT_JUMP_AVAILABLE_FOR_USE2    2
#define STAT_JUMP_AVAILABLE_FOR_USE3    3
#define STAT_JUMP_AVAILABLE_FOR_USE4    4
#define STAT_JUMP_AVAILABLE_FOR_USE5    5
#define STAT_JUMP_HEALTH                6  // (int)
#define STAT_JUMP_ASYNC_0               7  // (bool) visible or not
#define STAT_JUMP_ASYNC_1               8  // (bool) visible or not
#define STAT_JUMP_HUD_VOTE_CAST         9  // (configstring) for "Votes: X of X" 
#define STAT_JUMP_HUD_VOTE_REMAINING    10 // (configstring) for "X remaining"
#define STAT_JUMP_HUD_VOTE_TYPE         11 // (configstring) for short vote description (eg. "Map: ddrace")
#define STAT_JUMP_HUD_VOTE_INITIATED    12 // (configstring) for "Vote by X"
// IMPORTANT: DO NOT USE 13, STAT_LAYOUTS IS HARDCODED IN CLIENTS.
#define STAT_JUMP_HUD_FOOTER_1          14 // (configstring) set client-specific string for "Team: <team>"
#define STAT_JUMP_HUD_FOOTER_2          15 // (configstring) set client-specific string for "Race: n/NOW" or "Replay: n/NOW"
#define STAT_JUMP_HUD_FOOTER_3          16 // (configstring) set client-specific string for "Chkpts: n/n"
#define STAT_JUMP_HUD_FOOTER_4          17 // (configstring) 
#define STAT_JUMP_CLIENT_TRACE          18 // (configstring) set client-specific string for the name of the player in the crosshairs
#define STAT_JUMP_SPEED                 19 // (int) speed in units/s
#define STAT_JUMP_KEY_FORWARD           20 // (pcx) visible icon or not
#define STAT_JUMP_KEY_BACK              21 // (pcx) visible icon or not
#define STAT_JUMP_KEY_LEFT              22 // (pcx) visible icon or not
#define STAT_JUMP_KEY_RIGHT             23 // (pcx) visible icon or not
#define STAT_JUMP_KEY_JUMP              24 // (pcx) visible icon or not
#define STAT_JUMP_KEY_CROUCH            25 // (pcx) visible icon or not
#define STAT_JUMP_KEY_ATTACK            26 // (pcx) visible icon or not
#define STAT_JUMP_FPS                   27 // (int) FPS
#define STAT_JUMP_TIMER_SECONDS         28 // (int) timer (seconds) 
#define STAT_JUMP_TIMER_MS              29 // (int) timer (milliseconds)
#define STAT_JUMP_WEAPON_ICON           30 // (pcx) set to the equipped weapon pcx
#define STAT_JUMP_TIME_LEFT             31 // (int) time left in map (minutes)
// MaxStats is currently set to 32 in q_shared.h, can only use 0-31.
// To enable more, we would need to update the client as well.
// TODO: can also combine left/right, forward/back, and jump/crouch to free up space

// Config strings IDs
#define CS_JUMP_KEY_EMPTY               (CS_GENERAL + 0)
#define CS_JUMP_KEY_HUD_FOOTER_1        (CS_GENERAL + 1)
#define CS_JUMP_KEY_HUD_FOOTER_2        (CS_GENERAL + 2)
#define CS_JUMP_KEY_HUD_FOOTER_3        (CS_GENERAL + 3)
#define CS_JUMP_KEY_HUD_FOOTER_4        (CS_GENERAL + 4)
#define CS_JUMP_KEY_HUD_VOTE_INITIATED  (CS_GENERAL + 5)
#define CS_JUMP_KEY_HUD_VOTE_TYPE       (CS_GENERAL + 6)
#define CS_JUMP_KEY_HUD_VOTE_CAST       (CS_GENERAL + 7)
#define CS_JUMP_KEY_HUD_VOTE_REMAINING  (CS_GENERAL + 8)
#define CS_JUMP_KEY_HUD_TEAM_EASY       (CS_GENERAL + 9)
#define CS_JUMP_KEY_HUD_TEAM_HARD       (CS_GENERAL + 10)

// Config string strings
#define CS_JUMP_VAL_EMPTY ""

class HUD
{
public:
    static void SetConfigStrings();
    static void SetAllStats(edict_t* ent);
    static const char* GetFormattedLayoutString();

private:
    static void SetStatTimeLeft(edict_t* ent);
    static void SetStatFps(edict_t* ent);
    static void SetStatAsync(edict_t* ent);
    static void SetStatKeyStates(edict_t* ent);
    static void SetStatSpeed(edict_t* ent);
    static void SetStatTimer(edict_t* ent);
    static void SetStatFooter1(edict_t* ent);
    static void SetStatFooter2(edict_t* ent);
    static void SetStatFooter3(edict_t* ent);
    static void SetStatFooter4(edict_t* ent);
    static void SetStatWeapon(edict_t* ent);
    static void SetStatHealth(edict_t* ent);
    static void SetStatVoting(edict_t* ent);

    static const char* _hudLayoutString;
};

}
