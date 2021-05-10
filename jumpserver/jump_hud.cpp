
#include "g_local.h"
#include "jump_hud.h"
#include "jump_utils.h"
#include "jump_voting.h"
#include "jump_scores.h"
#include "jump_msets.h"

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
    // Health
    "if " XSTRINGIFY(STAT_JUMP_HEALTH) " "
    "yb	-32 "
    "xv	310 "
    "num 3 " XSTRINGIFY(STAT_JUMP_HEALTH) " "
    "yb -8 "
    "xv 312 "
    "string2 \"Health\" "
    "endif "
    ""
    // Equipped weapon
    "if " XSTRINGIFY(STAT_JUMP_WEAPON_ICON) " "
    "yb	-32 "
    "xv	280 "
    "pic " XSTRINGIFY(STAT_JUMP_WEAPON_ICON) " "
    "endif "
    // Keystates
    "xl 2 "
    "yb -42 "
    // Attack key
    "if " XSTRINGIFY(STAT_JUMP_KEY_ATTACK) " "
    "pic " XSTRINGIFY(STAT_JUMP_KEY_ATTACK) " "
    "endif "
    // Jump key
    "if " XSTRINGIFY(STAT_JUMP_KEY_JUMP) " "
    "pic " XSTRINGIFY(STAT_JUMP_KEY_JUMP) " "
    "endif "
    // Forward key
    "if " XSTRINGIFY(STAT_JUMP_KEY_FORWARD) " "
    "pic " XSTRINGIFY(STAT_JUMP_KEY_FORWARD) " "
    "endif "
    // Left key
    "if " XSTRINGIFY(STAT_JUMP_KEY_LEFT) " "
    "pic " XSTRINGIFY(STAT_JUMP_KEY_LEFT) " "
    "endif "
    // Right key
    "if " XSTRINGIFY(STAT_JUMP_KEY_RIGHT) " "
    "pic " XSTRINGIFY(STAT_JUMP_KEY_RIGHT) " "
    "endif "
    // Back key
    "if " XSTRINGIFY(STAT_JUMP_KEY_BACK) " "
    "pic " XSTRINGIFY(STAT_JUMP_KEY_BACK) " "
    "endif "
    // Crouch key
    "if " XSTRINGIFY(STAT_JUMP_KEY_CROUCH) " "
    "pic " XSTRINGIFY(STAT_JUMP_KEY_CROUCH) " "
    "endif "
    ""
    // Timer
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

/// <summary>
/// Set all stats.
/// </summary>
/// <param name="ent"></param>
void HUD::SetAllStats(edict_t* ent)
{
    SetStatFps(ent);
    SetStatAsync(ent);
    SetStatKeyStates(ent);
    SetStatSpeed(ent);
    SetStatTimer(ent);
    SetStatWeapon(ent);
    SetStatHealth(ent);
    SetStatFooter1(ent);
    SetStatFooter2(ent);
    SetStatFooter3(ent);
    SetStatFooter4(ent);
    SetStatVoting(ent);
    SetStatTimeLeft(ent);
    //SetStatTrace(ent);

    // Show the menu if it is open
    ent->client->ps.stats[STAT_LAYOUTS] = ent->client->showscores || ent->client->showhelp;
}

/// <summary>
/// Set the time left stat.
/// </summary>
/// <param name="ent"></param>
void HUD::SetStatTimeLeft(edict_t* ent)
{
    int timeleft = 999;
    if (timelimit != NULL && timelimit->value > 0)
    {
        timeleft = ((timelimit->value * 60) + (jump_server.time_added_mins * 60) - level.time) / 60;
    }
    ent->client->ps.stats[STAT_JUMP_TIME_LEFT] = timeleft;
}

/// <summary>
/// Set the voting stats.
/// </summary>
/// <param name="ent"></param>
void HUD::SetStatVoting(edict_t* ent)
{
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
}

/// <summary>
/// Set the health stat.
/// </summary>
/// <param name="ent"></param>
void HUD::SetStatHealth(edict_t* ent)
{
    if (ent->health < 1000)
    {
        ent->client->ps.stats[STAT_JUMP_HEALTH] = ent->health;
    }
    else
    {
        ent->client->ps.stats[STAT_JUMP_HEALTH] = 0;
    }
}

/// <summary>
/// Set the equipped weapon stat.
/// </summary>
/// <param name="ent"></param>
void HUD::SetStatWeapon(edict_t* ent)
{
    if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
    {
        ent->client->ps.stats[STAT_JUMP_WEAPON_ICON] = gi.imageindex(ent->client->pers.weapon->icon);
    }
    else
    {
        ent->client->ps.stats[STAT_JUMP_WEAPON_ICON] = 0;
    }
}

/// <summary>
/// Set the FPS stat.
/// </summary>
/// <param name="ent"></param>
void HUD::SetStatFps(edict_t* ent)
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
    else if (ent->client->jumpdata->team == TeamEnum::Spectator)
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
void HUD::SetStatAsync(edict_t* ent)
{
    bool showAsync = false;
    AsyncEnum async = AsyncEnum::Unknown;
    if (ent->client->jumpdata->update_replay_spectating)
    {
        // Watching a replay
        async = static_cast<AsyncEnum>(
            ent->client->jumpdata->replay_spectating[ent->client->jumpdata->replay_spectating_framenum].async);
        showAsync = true;
    }
    else if (ent->client->chase_target != nullptr)
    {
        // Chasing another player
        async = ent->client->chase_target->client->jumpdata->async;
        showAsync = true;
    }
    else if (ent->client->jumpdata->team == TeamEnum::Spectator)
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

    if (showAsync && async != AsyncEnum::Unknown)
    {
        ent->client->ps.stats[STAT_JUMP_ASYNC_0] = (async == AsyncEnum::Zero);
        ent->client->ps.stats[STAT_JUMP_ASYNC_1] = (async == AsyncEnum::One);
    }
    else
    {
        ent->client->ps.stats[STAT_JUMP_ASYNC_0] = false;
        ent->client->ps.stats[STAT_JUMP_ASYNC_1] = false;
    }
}

/// <summary>
/// Set the key states stats.
/// </summary>
/// <param name="ent"></param>
void HUD::SetStatKeyStates(edict_t* ent)
{
    uint8_t keyStates = 0;
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
    else if (ent->client->jumpdata->team == TeamEnum::Spectator)
    {
        // Not chasing anyone
        keyStates = 0;
    }
    else
    {
        // Playing
        keyStates = ent->client->jumpdata->key_states;
    }

    ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = 0;
    ent->client->ps.stats[STAT_JUMP_KEY_BACK] = 0;
    ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = 0;
    ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = 0;
    ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = 0;
    ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = 0;
    ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = 0;

    if (keyStates & static_cast<uint8_t>(KeyStateEnum::Forward))
    {
        ent->client->ps.stats[STAT_JUMP_KEY_FORWARD] = gi.imageindex("forward");
    }
    else if (keyStates & static_cast<uint8_t>(KeyStateEnum::Back))
    {
        ent->client->ps.stats[STAT_JUMP_KEY_BACK] = gi.imageindex("back");
    }

    if (keyStates & static_cast<uint8_t>(KeyStateEnum::Left))
    {
        ent->client->ps.stats[STAT_JUMP_KEY_LEFT] = gi.imageindex("left");
    }
    else if (keyStates & static_cast<uint8_t>(KeyStateEnum::Right))
    {
        ent->client->ps.stats[STAT_JUMP_KEY_RIGHT] = gi.imageindex("right");
    }

    if (keyStates & static_cast<uint8_t>(KeyStateEnum::Jump))
    {
        ent->client->ps.stats[STAT_JUMP_KEY_JUMP] = gi.imageindex("jump");
    }
    else if (keyStates & static_cast<uint8_t>(KeyStateEnum::Crouch))
    {
        ent->client->ps.stats[STAT_JUMP_KEY_CROUCH] = gi.imageindex("duck");
    }

    if (keyStates & static_cast<uint8_t>(KeyStateEnum::Attack))
    {
        ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = gi.imageindex("attack");
    }
}

/// <summary>
/// Set the speed stat.
/// </summary>
/// <param name="ent"></param>
void HUD::SetStatSpeed(edict_t* ent)
{
    bool quickRefresh = false;
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
        quickRefresh = true;
    }
    else if (ent->client->jumpdata->team == TeamEnum::Spectator)
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
        quickRefresh = true;
    }

    if (quickRefresh)
    {
        ent->client->ps.stats[STAT_JUMP_SPEED] = speed;
    }
    else
    {
        if (speed > ent->client->ps.stats[STAT_JUMP_SPEED] + 10 || speed < ent->client->ps.stats[STAT_JUMP_SPEED] - 10)
        {
            ent->client->ps.stats[STAT_JUMP_SPEED] = speed;
        }
    }
}

/// <summary>
/// Set the timer stat.
/// </summary>
/// <param name="ent"></param>
void HUD::SetStatTimer(edict_t* ent)
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
        else if (ent->client->chase_target->client->jumpdata->timer_finished)
        {
            timeMs = ent->client->chase_target->client->jumpdata->timer_end -
                ent->client->chase_target->client->jumpdata->timer_begin;
        }
        else
        {
            timeMs = Sys_Milliseconds() - ent->client->chase_target->client->jumpdata->timer_begin;
        }
    }
    else if (ent->client->jumpdata->team == TeamEnum::Spectator)
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
        else if (ent->client->jumpdata->timer_finished)
        {
            timeMs = ent->client->jumpdata->timer_end - ent->client->jumpdata->timer_begin;
        }
        else
        {
            timeMs = Sys_Milliseconds() - ent->client->jumpdata->timer_begin;
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
void HUD::SetStatFooter1(edict_t* ent)
{
    const char* TeamEasyStr = "  Team: Åáóù";
    const char* TeamHardStr = "  Team: Èáòä";
    const char* TeamSpecStr = "  Team: Ïâóåòöåò";
    std::string footer1;
    if (ent->client->jumpdata->update_replay_spectating)
    {
        // Watching a replay
        footer1 = TeamSpecStr;
    }
    else if (ent->client->chase_target != nullptr)
    {
        // Chasing another player
        if (ent->client->chase_target->client->jumpdata->team == TeamEnum::Easy)
        {
            footer1 = TeamEasyStr;
        }
        else
        {
            footer1 = TeamHardStr;
        }
    }
    else if (ent->client->jumpdata->team == TeamEnum::Spectator)
    {
        // Not chasing anyone
        footer1 = TeamSpecStr;
    }
    else
    {
        // Playing
        if (ent->client->jumpdata->team == TeamEnum::Easy)
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
void HUD::SetStatFooter2(edict_t* ent)
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
    else if (ent->client->jumpdata->team == TeamEnum::Spectator)
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

/// <summary>
/// Set the footer3 stat.
/// </summary>
/// <param name="ent"></param>
void HUD::SetStatFooter3(edict_t* ent)
{
    std::string footer3;
    int checkpointTotal = MSets::GetCheckpointTotal();

    if (checkpointTotal <= 0)
    {
        // No checkpoints, do nothing
    }
    else if (ent->client->jumpdata->update_replay_spectating)
    {
        // Watching a replay
        int checkpointCount =
            ent->client->jumpdata->replay_spectating[ent->client->jumpdata->replay_spectating_framenum].checkpoints;
        footer3 = va("Chkpts: %d/%d", checkpointCount, checkpointTotal);
    }
    else if (ent->client->chase_target != nullptr)
    {
        // Chasing another player
        if (ent->client->chase_target->client->jumpdata->team == TeamEnum::Easy || 
            ent->client->chase_target->client->jumpdata->team == TeamEnum::Hard)
        {
            int checkpointCount = ent->client->chase_target->client->jumpdata->checkpoint_total;
            footer3 = va("Chkpts: %d/%d", checkpointCount, checkpointTotal);
        }
    }
    else if (ent->client->jumpdata->team == TeamEnum::Spectator)
    {
        // Not chasing anyone, do nothing
    }
    else
    {
        // Playing
        int checkpointCount = ent->client->jumpdata->checkpoint_total;
        footer3 = va("Chkpts: %d/%d", checkpointCount, checkpointTotal);
    }

    // Footer3
    gi.WriteByte(svc_configstring);
    gi.WriteShort(CS_JUMP_KEY_HUD_FOOTER_3);
    gi.WriteString(const_cast<char*>(footer3.c_str()));
    gi.unicast(ent, true);

    ent->client->ps.stats[STAT_JUMP_HUD_FOOTER_3] = CS_JUMP_KEY_HUD_FOOTER_3;
}

/// <summary>
/// Set the footer4 stat.
/// </summary>
/// <param name="ent"></param>
void HUD::SetStatFooter4(edict_t* ent)
{
    std::string footer4;

    // Footer4
    // if (replaying, playing, or spectating AND laps) string = Laps: 1/5
    gi.WriteByte(svc_configstring);
    gi.WriteShort(CS_JUMP_KEY_HUD_FOOTER_4);
    gi.WriteString(const_cast<char*>(footer4.c_str()));
    gi.unicast(ent, true);

    ent->client->ps.stats[STAT_JUMP_HUD_FOOTER_4] = CS_JUMP_KEY_HUD_FOOTER_4;
}

} // namespace Jump