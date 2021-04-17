
#include "g_local.h"
#include "jump_hud.h"
#include "jump_utils.h"
#include "jump_voting.h"
#include "jump_scores.h"

namespace Jump
{
#if 0
    // cursor positioning
    xl <value>
    xr <value>
    yb <value>
    yt <value>
    xv <value>
    yv <value>

    // drawing
    statpic <name>
    pic <stat>
    num <fieldwidth> <stat>
    string <stat>

    // control
    if <stat>
    ifeq <stat> <value>
    ifbit <stat> <value>
    endif
#endif

/// <summary>
/// Private variables
/// </summary>
const char* HUD::_hudLayoutString =
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
    ""
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
    "yb -86 "
    "num 3 " XSTRINGIFY(STAT_JUMP_FPS) " "
    "xl 54 "
    "yb -70 "
    "string2 \"FPS\" "
    "endif "
    ""
    // Async
    "if " XSTRINGIFY(STAT_JUMP_ASYNC_1) " "
    "xl 80 "
    "yb -70 "
    "string2 \"(1)\" "
    "endif "
    "if " XSTRINGIFY(STAT_JUMP_ASYNC_0) " "
    "xl 80 "
    "yb -70 "
    "string2 \"(0)\" "
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
    // Voting
    "if " XSTRINGIFY(STAT_JUMP_HUD_VOTE_INITIATED) " "
    "xl 2 "
    "yb -136 "
    "stat_string " XSTRINGIFY(STAT_JUMP_HUD_VOTE_INITIATED) " "
    "yb -128 "
    "stat_string " XSTRINGIFY(STAT_JUMP_HUD_VOTE_TYPE) " "
    "yb -112 "
    "stat_string " XSTRINGIFY(STAT_JUMP_HUD_VOTE_CAST) " "
    "yb -104 "
    "stat_string " XSTRINGIFY(STAT_JUMP_HUD_VOTE_REMAINING) " "
    "endif "
    ""
    // Footer
    "xv 72 "
    "yb -32 "
    "stat_string " XSTRINGIFY(STAT_JUMP_HUD_FOOTER_1) " "
    "yb -24 "
    "stat_string " XSTRINGIFY(STAT_JUMP_HUD_FOOTER_2) " "
    "yb -16 "
    "stat_string " XSTRINGIFY(STAT_JUMP_HUD_FOOTER_3) " "
    "yb -8 "
    "stat_string " XSTRINGIFY(STAT_JUMP_HUD_FOOTER_4) " "
    ""
;

/// <summary>
/// Sets the value for static config strings.
/// </summary>
void HUD::SetConfigStrings()
{
    gi.configstring(CS_JUMP_KEY_EMPTY, CS_JUMP_VAL_EMPTY);
    gi.configstring(CS_STATUSBAR, const_cast<char*>(GetFormattedLayoutString()));
}

void HUD::SetStatsReplaying(edict_t* ent)
{
    assert(ent->client->jumpdata->team == TEAM_SPECTATOR);
    assert(ent->client->jumpdata->update_replay_spectating);

    const replay_frame_t& frame =
        ent->client->jumpdata->replay_spectating[ent->client->jumpdata->replay_spectating_framenum];

    // Update the input keys
    ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_BACK] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = false;

    if (frame.key_states & KEY_STATE_FORWARD)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = true;
    }
    else if (frame.key_states & KEY_STATE_BACK)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_BACK] = true;
    }

    if (frame.key_states & KEY_STATE_LEFT)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = true;
    }
    else if (frame.key_states & KEY_STATE_RIGHT)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = true;
    }

    if (frame.key_states & KEY_STATE_JUMP)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = true;
    }
    else if (frame.key_states & KEY_STATE_CROUCH)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = true;
    }

    if (frame.key_states & KEY_STATE_ATTACK)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = true;
    }

    // Update fps, async
    ent->client->ps.stats[STAT_JUMP_FPS] = frame.fps;
    if (frame.async == 0)
    {
        ent->client->ps.stats[STAT_JUMP_ASYNC_0] = 1;
        ent->client->ps.stats[STAT_JUMP_ASYNC_1] = 0;
    }
    else
    {
        ent->client->ps.stats[STAT_JUMP_ASYNC_0] = 0;
        ent->client->ps.stats[STAT_JUMP_ASYNC_1] = 1;
    }

    // Update timer
    int timeMs = ent->client->jumpdata->replay_spectating_framenum * 100;
    ent->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = static_cast<short>(timeMs / 1000);
    ent->client->ps.stats[STAT_JUMP_TIMER_MS] = static_cast<short>((timeMs % 1000) / 100);

    // Update weapon
    // TODO STAT_JUMP_WEAPON_ICON

    // Update speed
    // To calculate speed, we calculate the distance traveled over 1 second.
    // This is less jumpy than doing it for every frame.
    if (ent->client->jumpdata->replay_spectating_framenum >= 10)
    {
        const replay_frame_t& prevFrame =
            ent->client->jumpdata->replay_spectating[ent->client->jumpdata->replay_spectating_framenum - 10];
        vec3_t velocity;
        VectorSubtract(frame.pos, prevFrame.pos, velocity);
        velocity[2] = 0;
        short speed = static_cast<short>(VectorLength(velocity));
        if (speed > ent->client->ps.stats[STAT_JUMP_SPEED] + 10 || speed < ent->client->ps.stats[STAT_JUMP_SPEED] - 10)
        {
            ent->client->ps.stats[STAT_JUMP_SPEED] = speed;
        }
    }
    else if (ent->client->jumpdata->replay_spectating_framenum >= 1)
    {
        // If we are less than 1 second into the replay, adjust for fewer frames.
        const replay_frame_t& prevFrame =
            ent->client->jumpdata->replay_spectating[0];
        vec3_t velocity;
        velocity[0] = (frame.pos[0] - prevFrame.pos[0]) / (ent->client->jumpdata->replay_spectating_framenum * 0.1f);
        velocity[1] = (frame.pos[1] - prevFrame.pos[1]) / (ent->client->jumpdata->replay_spectating_framenum * 0.1f);
        velocity[2] = 0;
        short speed = static_cast<short>(VectorLength(velocity));
        if (speed > ent->client->ps.stats[STAT_JUMP_SPEED] + 10 || speed < ent->client->ps.stats[STAT_JUMP_SPEED] - 10)
        {
            ent->client->ps.stats[STAT_JUMP_SPEED] = speed;
        }
    }
    else
    {
        ent->client->ps.stats[STAT_JUMP_SPEED] = 0;
    }
}

// TODO refactor
void HUD::SetStats(edict_t* ent)
{
    //if (ent->client->jumpdata->team == TEAM_EASY || ent->client->jumpdata->team == TEAM_HARD)
    //{
    //    SetStatsJumping(ent);
    //}
    //else
    //{
    //    if (ent->client->jumpdata->update_replay_spectating)
    //    {
    //        SetStatsReplaying(ent);
    //    }
    //    else
    //    {
    //        SetStatsSpectating(ent);
    //    }
    //}

    if (ent->client->jumpdata->update_replay_spectating)
    {
        SetStatsReplaying(ent);
        return;
        // TODO: need to update common STAT fields down below
    }

    vec3_t velocity;
    velocity[0] = ent->velocity[0];
    velocity[1] = ent->velocity[1];
    velocity[2] = 0;
    ent->client->ps.stats[STAT_JUMP_SPEED] = static_cast<short>(VectorNormalize(velocity));

    ent->client->ps.stats[STAT_JUMP_CLIENT_TRACE] = 0; // TODO name of person user is looking at
    ent->client->ps.stats[STAT_JUMP_HUD_FOOTER_1] = CS_JUMP_KEY_HUD_FOOTER_1;
    ent->client->ps.stats[STAT_JUMP_HUD_FOOTER_2] = CS_JUMP_KEY_HUD_FOOTER_2;
    ent->client->ps.stats[STAT_JUMP_HUD_FOOTER_3] = CS_JUMP_KEY_HUD_FOOTER_3;
    ent->client->ps.stats[STAT_JUMP_HUD_FOOTER_4] = CS_JUMP_KEY_HUD_FOOTER_4;

    FooterStrings(ent);

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

    if (ent->client->jumpdata->async == 0)
    {
        ent->client->ps.stats[STAT_JUMP_ASYNC_0] = 1;
        ent->client->ps.stats[STAT_JUMP_ASYNC_1] = 0;
    }
    else
    {
        ent->client->ps.stats[STAT_JUMP_ASYNC_0] = 0;
        ent->client->ps.stats[STAT_JUMP_ASYNC_1] = 1;
    }

    ent->client->ps.stats[STAT_JUMP_FPS] = static_cast<short>(ent->client->jumpdata->fps);
        
    int timeleft = 999;
    if (timelimit != NULL && timelimit->value > 0)
    {
        timeleft = ((timelimit->value * 60) + (jump_server.time_added_mins * 60) - level.time) / 60;
    }
    ent->client->ps.stats[STAT_JUMP_TIME_LEFT] = timeleft;


    if (VoteSystem::IsVoting() && VoteSystem::IsHudVote())
    {
        ent->client->ps.stats[STAT_JUMP_HUD_VOTE_INITIATED] = CS_JUMP_KEY_HUD_VOTE_INITIATED;
        ent->client->ps.stats[STAT_JUMP_HUD_VOTE_TYPE] = CS_JUMP_KEY_HUD_VOTE_TYPE;
        ent->client->ps.stats[STAT_JUMP_HUD_VOTE_CAST] = CS_JUMP_KEY_HUD_VOTE_CAST;
        ent->client->ps.stats[STAT_JUMP_HUD_VOTE_REMAINING] = CS_JUMP_KEY_HUD_VOTE_REMAINING;
    }
    else
    {
        ent->client->ps.stats[STAT_JUMP_HUD_VOTE_INITIATED] = 0;
    }

    UpdateTimer(ent);

    // Show the menu if it is open
    if (ent->client->showscores || ent->client->showhelp)
    {
        ent->client->ps.stats[STAT_LAYOUTS] = 1;
    }
    else
    {
        ent->client->ps.stats[STAT_LAYOUTS] = 0;
    }

    // TODO: show health if damage is taken
    //ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
    //ent->client->ps.stats[STAT_HEALTH] = ent->health;
        

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



/// <summary>
/// Set the FPS stat.
/// </summary>
/// <param name="ent"></param>
void SetStatFps(edict_t* ent)
{
    if (ent->client->jumpdata->update_replay_spectating)
    {
        // Watching a replay
        ent->client->ps.stats[STAT_JUMP_FPS] = 
            ent->client->jumpdata->replay_spectating[ent->client->jumpdata->replay_spectating_framenum].fps;
    }
    else if (ent->client->chase_target != nullptr)
    {
        // Chasing another player
        ent->client->ps.stats[STAT_JUMP_FPS] = ent->client->chase_target->client->jumpdata->fps;
    }
    else if (ent->client->jumpdata->team == TEAM_SPECTATOR)
    {
        // Not chasing anyone
        ent->client->ps.stats[STAT_JUMP_FPS] = 0;
    }
    else
    {
        // Playing
        ent->client->ps.stats[STAT_JUMP_FPS] = ent->client->jumpdata->fps;
    }
}

/// <summary>
/// Set the async stat.
/// </summary>
/// <param name="ent"></param>
void SetStatAsync(edict_t* ent)
{
    bool showAsync = false;
    int async = 0;
    if (ent->client->jumpdata->update_replay_spectating)
    {
        // Watching a replay
        async = ent->client->jumpdata->replay_spectating[ent->client->jumpdata->replay_spectating_framenum].async;
        showAsync = true;
    }
    else if (ent->client->chase_target != nullptr)
    {
        // Chasing another player
        async = ent->client->chase_target->client->jumpdata->async;
        showAsync = true;
    }
    else if (ent->client->jumpdata->team == TEAM_SPECTATOR)
    {
        // Not chasing anyone
        showAsync = false;
    }
    else
    {
        // Playing
        async = ent->client->jumpdata->async;
        showAsync = true;
    }

    ent->client->ps.stats[STAT_JUMP_ASYNC_0] = showAsync && (async == 0);
    ent->client->ps.stats[STAT_JUMP_ASYNC_1] = showAsync && (async == 1);
}

/// <summary>
/// Set the key states stats.
/// </summary>
/// <param name="ent"></param>
void SetStatKeyStates(edict_t* ent)
{
    int keyStates = 0;
    if (ent->client->jumpdata->update_replay_spectating)
    {
        // Watching a replay
        keyStates = ent->client->jumpdata->replay_spectating[ent->client->jumpdata->replay_spectating_framenum].key_states;
    }
    else if (ent->client->chase_target != nullptr)
    {
        // Chasing another player
        keyStates = ent->client->chase_target->client->jumpdata->key_states;
    }
    else if (ent->client->jumpdata->team == TEAM_SPECTATOR)
    {
        // Not chasing anyone
        keyStates = 0;
    }
    else
    {
        // Playing
        keyStates = ent->client->jumpdata->key_states;
    }

    ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_BACK] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = false;
    ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = false;

    if (keyStates & KEY_STATE_FORWARD)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = true;
    }
    else if (keyStates & KEY_STATE_BACK)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_BACK] = true;
    }

    if (keyStates & KEY_STATE_LEFT)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = true;
    }
    else if (keyStates & KEY_STATE_RIGHT)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = true;
    }

    if (keyStates & KEY_STATE_JUMP)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = true;
    }
    else if (keyStates & KEY_STATE_CROUCH)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = true;
    }

    if (keyStates & KEY_STATE_ATTACK)
    {
        ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = true;
    }
}

/// <summary>
/// Set the speed stat.
/// </summary>
/// <param name="ent"></param>
void SetStatSpeed(edict_t* ent)
{
    int speed = 0;
    if (ent->client->jumpdata->update_replay_spectating)
    {
        // Watching a replay
        // To calculate speed, we calculate the distance traveled over 1 second.
        // This is less jumpy than doing it for every frame.
        const replay_frame_t& frame =
            ent->client->jumpdata->replay_spectating[ent->client->jumpdata->replay_spectating_framenum];
        if (ent->client->jumpdata->replay_spectating_framenum >= 10)
        {
            const replay_frame_t& prevFrame =
                ent->client->jumpdata->replay_spectating[ent->client->jumpdata->replay_spectating_framenum - 10];
            vec3_t velocity;
            VectorSubtract(frame.pos, prevFrame.pos, velocity);
            velocity[2] = 0;
            speed = static_cast<int>(VectorLength(velocity));
        }
        else if (ent->client->jumpdata->replay_spectating_framenum >= 1)
        {
            // If we are less than 1 second into the replay, adjust for fewer frames.
            const replay_frame_t& prevFrame = ent->client->jumpdata->replay_spectating[0];
            vec3_t velocity;
            velocity[0] = (frame.pos[0] - prevFrame.pos[0]) / (ent->client->jumpdata->replay_spectating_framenum * 0.1f);
            velocity[1] = (frame.pos[1] - prevFrame.pos[1]) / (ent->client->jumpdata->replay_spectating_framenum * 0.1f);
            velocity[2] = 0;
            speed = static_cast<int>(VectorLength(velocity));
        }
    }
    else if (ent->client->chase_target != nullptr)
    {
        // Chasing another player
        vec3_t velocity;
        velocity[0] = ent->client->chase_target->velocity[0];
        velocity[1] = ent->client->chase_target->velocity[1];
        velocity[2] = 0;
        speed = static_cast<int>(VectorNormalize(velocity));
    }
    else if (ent->client->jumpdata->team == TEAM_SPECTATOR)
    {
        // Not chasing anyone
        speed = 0;
    }
    else
    {
        // Playing
        vec3_t velocity;
        velocity[0] = ent->velocity[0];
        velocity[1] = ent->velocity[1];
        velocity[2] = 0;
        speed = static_cast<int>(VectorNormalize(velocity));
    }

    if (speed > ent->client->ps.stats[STAT_JUMP_SPEED] + 10 || speed < ent->client->ps.stats[STAT_JUMP_SPEED] - 10)
    {
        ent->client->ps.stats[STAT_JUMP_SPEED] = speed;
    }
}

/// <summary>
/// Set the timer stat.
/// </summary>
/// <param name="ent"></param>
void SetStatTimer(edict_t* ent)
{
    int64_t timeMs;
    if (ent->client->jumpdata->update_replay_spectating)
    {
        // Watching a replay
        timeMs = static_cast<int64_t>(ent->client->jumpdata->replay_spectating_framenum) * 100;
    }
    else if (ent->client->chase_target != nullptr)
    {
        // Chasing another player
        if (ent->client->chase_target->client->jumpdata->timer_paused)
        {
            timeMs = 0;
        }
        else
        {
            timeMs = ent->client->chase_target->client->jumpdata->timer_end - ent->client->chase_target->client->jumpdata->timer_begin;
        }
    }
    else if (ent->client->jumpdata->team == TEAM_SPECTATOR)
    {
        // Not chasing anyone
        timeMs = 0;
    }
    else
    {
        // Playing
        if (ent->client->jumpdata->timer_paused)
        {
            timeMs = 0;
        }
        else
        {
            timeMs = ent->client->jumpdata->timer_end - ent->client->jumpdata->timer_begin;
        }
    }

    // Note that if the time exceeds 32767 seconds, it will show up as negative.
    ent->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = static_cast<short>(timeMs / 1000);
    ent->client->ps.stats[STAT_JUMP_TIMER_MS] = (timeMs % 1000) / 100;
}

/// <summary>
/// Set the footer1 stat.
/// </summary>
/// <param name="ent"></param>
void SetStatFooter1(edict_t* ent)
{
    const char* TeamEasyStr = "  Team: ≈·Û˘";
    const char* TeamHardStr = "  Team: »·Ú‰";
    const char* TeamSpecStr = "  Team: œ‚ÛÂÚˆÂÚ";
    std::string footer1;
    if (ent->client->jumpdata->update_replay_spectating)
    {
        // Watching a replay
        footer1 = TeamSpecStr;
    }
    else if (ent->client->chase_target != nullptr)
    {
        // Chasing another player
        if (ent->client->chase_target->client->jumpdata->team == TEAM_EASY)
        {
            footer1 = TeamEasyStr;
        }
        else
        {
            footer1 = TeamHardStr;
        }
    }
    else if (ent->client->jumpdata->team == TEAM_SPECTATOR)
    {
        // Not chasing anyone
        footer1 = TeamSpecStr;
    }
    else
    {
        // Playing
        if (ent->client->jumpdata->team == TEAM_EASY)
        {
            footer1 = TeamEasyStr;
        }
        else
        {
            footer1 = TeamHardStr;
        }
    }

    gi.WriteByte(svc_configstring);
    gi.WriteShort(CS_JUMP_KEY_HUD_FOOTER_1);
    gi.WriteString(const_cast<char*>(footer1.c_str()));
    gi.unicast(ent, true);

    ent->client->ps.stats[STAT_JUMP_HUD_FOOTER_1] = CS_JUMP_KEY_HUD_FOOTER_1;
}

/// <summary>
/// Set the footer2 stat.
/// </summary>
/// <param name="ent"></param>
void SetStatFooter2(edict_t* ent)
{
    // Footer2
// if (replaying) string = Replay: NOW or Replay: 14 or Replay: Self
// if (playing or spectating AND racing) string = Race: NOW or Race: 12 or Race: Self

    std::string footer2;
    if (ent->client->jumpdata->update_replay_spectating)
    {
        // Watching a replay
        // TODO: save who we are replaying when we use the replay cmd
    }
    else if (ent->client->chase_target != nullptr)
    {
        // Chasing another player
        // if (chase_target is racing)
        // Race: NOW or Race: 12 or Race: Self
    }
    else if (ent->client->jumpdata->team == TEAM_SPECTATOR)
    {
        // Not chasing anyone
        footer2 = "";
    }
    else
    {
        // Playing
        // if (racing)
        // Race: NOW or Race: 12 or Race: Self
    }

    gi.WriteByte(svc_configstring);
    gi.WriteShort(CS_JUMP_KEY_HUD_FOOTER_2);
    gi.WriteString(const_cast<char*>(footer2.c_str()));
    gi.unicast(ent, true);

    ent->client->ps.stats[STAT_JUMP_HUD_FOOTER_2] = CS_JUMP_KEY_HUD_FOOTER_2;
}


void SetAllStats(edict_t* ent)
{
    SetStatFps(ent);
    SetStatAsync(ent);
    SetStatKeyStates(ent);
    SetStatSpeed(ent);
    SetStatTimer(ent);
    //SetStatWeapon(ent);
    //SetStatHealth(ent);
    SetStatFooter1(ent);
    //SetStatFooter2(ent);
    //SetStatFooter3(ent);
    //SetStatFooter4(ent);
    //SetStatTrace(ent);

    // Set the common stats: time left, time added, vote active, last maps played, current map
}

// TODO: move to the SetStats functions
void HUD::UpdateTimer(edict_t * ent)
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

/// <summary>
/// Takes the HUD layout string and inserts the token values.
/// </summary>
/// <returns></returns>
const char* HUD::GetFormattedLayoutString()
{
    std::string current_map = level.mapname;
    std::string last_map1 = jump_server.last_map1;
    std::string last_map2 = jump_server.last_map2;
    std::string last_map3 = jump_server.last_map3;
    int total_maps = LocalScores::GetServerMapCount();
    std::string time_added = std::string("+") + std::to_string(jump_server.time_added_mins);

    assert(strlen(_hudLayoutString) < 1400 - 16);
    static char buffer[1400 - 16];
    snprintf(buffer, sizeof(buffer), _hudLayoutString,
        current_map.c_str(), last_map1.c_str(), last_map2.c_str(), last_map3.c_str(),
        total_maps,
        time_added.c_str());
    return buffer;
    // TODO: verify that the filled in string is less than 1400
}


void AllHud(edict_t* ent)
{
    // if (chasing)
    //   show "Chasing <name>"
    //   CommonStats(spec player, ent)
    // else if (replaying)
    //   get stats from replay frame
    // else
    //   get stats from self
}

void GetStatsFromEnt(edict_t* ent, int& fps, int& async, uint8_t& keyStates, int64_t& time)
{
    fps = static_cast<short>(ent->client->jumpdata->fps);
    async = static_cast<short>(ent->client->jumpdata->async);
    keyStates = ent->client->jumpdata->key_states;
    //time = 
}


void CommonStats(edict_t* source, edict_t* dest)
{
    // FPS

    // Key states

    // Footer strings

    // Timer
    if (source->client->jumpdata->timer_paused)
    {
        dest->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = 0;
        dest->client->ps.stats[STAT_JUMP_TIMER_MS] = 0;
    }
    else if (source->client->jumpdata->timer_finished)
    {
        int64_t time = source->client->jumpdata->timer_end - source->client->jumpdata->timer_begin;
        dest->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = static_cast<short>(time / 1000);
        dest->client->ps.stats[STAT_JUMP_TIMER_MS] = static_cast<short>((time % 1000) / 100);
    }
    else // active timer
    {
        // We can only show 32767 seconds (~9 hrs) on the timer
        int64_t time = Sys_Milliseconds() - source->client->jumpdata->timer_begin;
        dest->client->ps.stats[STAT_JUMP_TIMER_SECONDS] = static_cast<short>(time / 1000);
        dest->client->ps.stats[STAT_JUMP_TIMER_MS] = static_cast<short>((time % 1000) / 100);
    }
}


void HUD::FooterStrings(edict_t* ent)
{
    std::string footer1;
    std::string footer2;
    std::string footer3;
    std::string footer4;

    if (ent->client->jumpdata->team == TEAM_EASY)
    {
        footer1 = "  Team: ≈·Û˘";
    }
    else if (ent->client->jumpdata->team == TEAM_HARD)
    {
        footer1 = "  Team: »·Ú‰";
    }
    else
    {
        footer1 = "  Team: œ‚ÛÂÚˆÂÚ";
    }

    // Footer2
    // if (replaying) string = Replay: NOW or Replay: 14 or Replay: Self
    // if (playing or spectating AND racing) string = Race: NOW or Race: 12 or Race: Self

    // Footer3
    // if (replaying, playing, or spectating AND checkpoints) string = Chkpts: 1/5

    // Footer4
    // if (replaying, playing, or spectating AND laps) string = Laps: 1/5

    gi.WriteByte(svc_configstring);
    gi.WriteShort(CS_JUMP_KEY_HUD_FOOTER_1);
    gi.WriteString(const_cast<char*>(footer1.c_str()));
    gi.unicast(ent, true);

    gi.WriteByte(svc_configstring);
    gi.WriteShort(CS_JUMP_KEY_HUD_FOOTER_2);
    gi.WriteString(const_cast<char*>(footer2.c_str()));
    gi.unicast(ent, true);

    gi.WriteByte(svc_configstring);
    gi.WriteShort(CS_JUMP_KEY_HUD_FOOTER_3);
    gi.WriteString(const_cast<char*>(footer3.c_str()));
    gi.unicast(ent, true);

    gi.WriteByte(svc_configstring);
    gi.WriteShort(CS_JUMP_KEY_HUD_FOOTER_4);
    gi.WriteString(const_cast<char*>(footer4.c_str()));
    gi.unicast(ent, true);

#if 0
    edict_t* cl_ent;
    int i;
    char cp[4];
    char cptotal[4];
    char race[10];
    char lap[10];
    char laptotal[10];
    int strnr;

    if (!ent->client)
        return;

    // update statusbar for client if it's chasing someone...
    if (ent->client->chase_target) {
        gi.WriteByte(svc_configstring);
        gi.WriteShort(CONFIG_JUMP_HUDSTRING1);
        gi.WriteString(ent->client->chase_target->client->resp.hud[0].string);
        gi.unicast(ent, true);
        gi.WriteByte(svc_configstring);
        gi.WriteShort(CONFIG_JUMP_HUDSTRING2);
        gi.WriteString(ent->client->chase_target->client->resp.hud[1].string);
        gi.unicast(ent, true);
        gi.WriteByte(svc_configstring);
        gi.WriteShort(CONFIG_JUMP_HUDSTRING3);
        gi.WriteString(ent->client->chase_target->client->resp.hud[2].string);
        gi.unicast(ent, true);
        gi.WriteByte(svc_configstring);
        gi.WriteShort(CONFIG_JUMP_HUDSTRING4);
        gi.WriteString(ent->client->chase_target->client->resp.hud[3].string);
        gi.unicast(ent, true);
        return;
    }
    //else if client is not chasing someone......

    //rem old strings
    for (i = 0; i < 4; i++) {
        sprintf(ent->client->resp.hud[i].string, "");
    }

    //team (Team is always string1.)
    if (ent->client->resp.ctf_team == CTF_TEAM1)
        sprintf(ent->client->resp.hud[0].string, "  Team: ≈·Û˘");
    else if (ent->client->resp.ctf_team == CTF_TEAM2)
        sprintf(ent->client->resp.hud[0].string, "  Team: »·Ú‰");
    else
        sprintf(ent->client->resp.hud[0].string, "  Team: œ‚ÛÂÚˆÂÚ");

    //rest of the strings
    strnr = 1;
    // race
    if (ent->client->resp.replaying) { //if player is replaying, print replay string instead.
        sprintf(race, "%d", ent->client->resp.replaying);
        if (Q_stricmp(race, "16") == 0) {
            sprintf(race, "NOW");
        }
        sprintf(ent->client->resp.hud[strnr].string, "Replay: %s", HighAscii(race));
        strnr++;
    }
    else if (ent->client->resp.rep_racing) {
        sprintf(race, "%d", ent->client->resp.rep_race_number + 1);
        if (Q_stricmp(race, "16") == 0) {
            sprintf(race, "NOW");
        }
        sprintf(ent->client->resp.hud[strnr].string, "  Race: %s", HighAscii(race));
        strnr++;
    }

    // cp
    if (mset_vars->checkpoint_total) {
        sprintf(cptotal, "%d", mset_vars->checkpoint_total);
        sprintf(cp, "%d", ent->client->resp.store[0].checkpoints);
        sprintf(ent->client->resp.hud[strnr].string, "Chkpts: %s/%s", HighAscii(cp), HighAscii(cptotal));
        strnr++;
    }

    // lap
    if (mset_vars->lap_total) {
        sprintf(laptotal, "%d", mset_vars->lap_total);
        sprintf(lap, "%d", ent->client->pers.lapcount);
        sprintf(ent->client->resp.hud[strnr].string, "  Laps: %s/%s", HighAscii(lap), HighAscii(laptotal));
    }

    //UPDATE IT, also for chasers....
    for (i = 0; i < maxclients->value; i++) {
        cl_ent = g_edicts + 1 + i;

        if (!(cl_ent->client && cl_ent->inuse))
            continue;

        if (cl_ent != ent) {
            if (!cl_ent->client->chase_target)
                continue;
            if (cl_ent->client->chase_target->client != ent->client)
                continue;
        }
        gi.WriteByte(svc_configstring);
        gi.WriteShort(CONFIG_JUMP_HUDSTRING1);
        gi.WriteString(ent->client->resp.hud[0].string);
        gi.unicast(cl_ent, true);
        gi.WriteByte(svc_configstring);
        gi.WriteShort(CONFIG_JUMP_HUDSTRING2);
        gi.WriteString(ent->client->resp.hud[1].string);
        gi.unicast(cl_ent, true);
        gi.WriteByte(svc_configstring);
        gi.WriteShort(CONFIG_JUMP_HUDSTRING3);
        gi.WriteString(ent->client->resp.hud[2].string);
        gi.unicast(cl_ent, true);
        gi.WriteByte(svc_configstring);
        gi.WriteShort(CONFIG_JUMP_HUDSTRING4);
        gi.WriteString(ent->client->resp.hud[3].string);
        gi.unicast(cl_ent, true);
    }
    Update_CP_Ents();

#endif
}



} // namespace Jump