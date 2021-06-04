#include "jump_spawn.h"
#include "jump_menu.h"
#include "jump_local_database.h"

namespace Jump
{

/// <summary>
/// Class variables
/// </summary>
const int Spawn::MaxHealthAndAmmo = 1000;

/// <summary>
/// Client joins the easy team.
/// </summary>
/// <param name="ent"></param>
void Spawn::JoinTeamEasy(edict_t* ent)
{
    gi.unlinkentity(ent);
    ent->client->jumpdata->team = TeamEnum::Easy;
    AssignTeamSkin(ent);

    InitDefaultSpawnVariables(ent);

    if (ent->client->jumpdata->stores.HasStore())
    {
        store_data_t data = ent->client->jumpdata->stores.GetStore(1);
        // TODO: restore weapons/ammo from store
        MovePlayerToPosition(ent, data.pos, data.angles, false);
        ent->client->jumpdata->timer_begin = Sys_Milliseconds() - data.time_interval;
    }
    else
    {
        edict_t* spawn = SelectPlayerSpawn();
        MovePlayerToSpawn(ent, spawn, true);
    }
    gi.linkentity(ent);
}

/// <summary>
/// Client joins the hard team.
/// </summary>
/// <param name="ent"></param>
void Spawn::JoinTeamHard(edict_t* ent)
{
    gi.unlinkentity(ent);
    ent->client->jumpdata->team = TeamEnum::Hard;
    AssignTeamSkin(ent);

    InitDefaultSpawnVariables(ent);

    edict_t* spawn = SelectPlayerSpawn();
    MovePlayerToSpawn(ent, spawn, true);
    gi.linkentity(ent);
}

/// <summary>
/// Client joins the spectator team.
/// </summary>
/// <param name="ent"></param>
void Spawn::JoinTeamSpectator(edict_t* ent)
{
    ent->client->jumpdata->team = TeamEnum::Spectator;
    AssignTeamSkin(ent);
    InitAsSpectator(ent);
}

/// <summary>
/// Handles player respawning for all teams.
/// </summary>
/// <param name="ent"></param>
void Spawn::PlayerRespawn(edict_t* ent, int storeNum)
{
    if (ent->client->jumpdata->team == TeamEnum::Spectator)
    {
        return;
    }

    gi.unlinkentity(ent);
    InitDefaultSpawnVariables(ent);
    if (ent->client->jumpdata->team == TeamEnum::Easy && ent->client->jumpdata->stores.HasStore())
    {
        if (storeNum < 1)
        {
            storeNum = 1;
        }
        store_data_t data = ent->client->jumpdata->stores.GetStore(storeNum);
        // TODO: restore weapons/ammo from store
        MovePlayerToPosition(ent, data.pos, data.angles, false);
        ent->client->jumpdata->timer_begin = Sys_Milliseconds() - data.time_interval;
    }
    else // TeamEnum::Easy with no stores or TeamEnum::Hard
    {
        edict_t* spawn = SelectPlayerSpawn();
        MovePlayerToSpawn(ent, spawn, true);
    }
    gi.linkentity(ent);
}

/// <summary>
/// Sets the client to a good default state when spawning for jumping.
/// </summary>
/// <param name="ent"></param>
void Spawn::InitDefaultSpawnVariables(edict_t* ent)
{
    ent->s.modelindex = 255;
    ent->s.modelindex2 = 255;
    ent->s.modelindex3 = 0;
    ent->s.modelindex4 = 0;
    ent->s.frame = 0;
    ent->s.effects = 0;
    ent->s.renderfx = 0;
    ent->s.solid = SOLID_TRIGGER;

    ent->client->ps.pmove.pm_type = PM_NORMAL;
    VectorClear(ent->client->ps.pmove.origin);
    VectorClear(ent->client->ps.pmove.velocity);
    ent->client->ps.pmove.pm_flags = 0;
    ent->client->ps.pmove.pm_time = 0;
    VectorClear(ent->client->ps.pmove.delta_angles);

    VectorClear(ent->client->ps.viewangles);
    VectorClear(ent->client->ps.viewoffset);
    VectorClear(ent->client->ps.kick_angles);
    VectorClear(ent->client->ps.gunangles);
    VectorClear(ent->client->ps.gunoffset);
    ent->client->ps.gunindex = 0;
    ent->client->ps.gunframe = 0;
    ArrayClear(ent->client->ps.blend);
    ent->client->ps.rdflags = 0;

    ent->client->pers.health = MaxHealthAndAmmo;
    ent->client->pers.selected_item = 0;
    ArrayClear(ent->client->pers.inventory);
    ent->client->pers.weapon = nullptr;
    ent->client->pers.lastweapon = nullptr;

    ent->client->ammo_index = 0;
    ent->client->weapon_thunk = false;
    ent->client->newweapon = nullptr;
    ent->client->damage_armor = 0;
    ent->client->damage_parmor = 0;
    ent->client->damage_blood = 0;
    ent->client->damage_knockback = 0;
    VectorClear(ent->client->damage_from);
    ent->client->killer_yaw = 0;
    ent->client->weaponstate = WEAPON_READY;
    VectorClear(ent->client->kick_angles);
    VectorClear(ent->client->kick_origin);
    ent->client->v_dmg_roll = 0;
    ent->client->v_dmg_pitch = 0;
    ent->client->v_dmg_time = 0;
    ent->client->fall_time = 0;
    ent->client->fall_value = 0;
    ent->client->damage_alpha = 0;
    ent->client->bonus_alpha = 0;
    VectorClear(ent->client->damage_blend);
    ent->client->bobtime = 0;
    ent->client->next_drown_time = 0;
    ent->client->old_waterlevel = 0;
    ent->client->breather_sound = 0;
    ent->client->machinegun_shots = 0;
    ent->client->anim_end = 0;
    ent->client->anim_priority = ANIM_BASIC;
    ent->client->anim_duck = false;
    ent->client->anim_run = false;
    ent->client->quad_framenum = 0;
    ent->client->invincible_framenum = 0;
    ent->client->breather_framenum = 0;
    ent->client->enviro_framenum = 0;
    ent->client->grenade_blew_up = false;
    ent->client->grenade_time = 0;
    ent->client->silencer_shots = 0;
    ent->client->weapon_sound = 0;
    ent->client->pickup_msg_time = 0;
    ent->client->chase_target = nullptr;
    ent->client->update_chase = false;

    ent->client->jumpdata->replay_recording.clear();
    ent->client->jumpdata->replay_spectating.clear();
    ent->client->jumpdata->replay_spectating_framenum = 0;
    ent->client->jumpdata->update_replay_spectating = false;
    ent->client->jumpdata->replay_spectating_hud_string.clear();
    ent->client->jumpdata->timer_pmove_msec = 0;
    ent->client->jumpdata->timer_begin = 0;
    ent->client->jumpdata->timer_end = 0;
    ent->client->jumpdata->timer_paused = true;
    ent->client->jumpdata->timer_finished = false;
    ent->client->jumpdata->racing_framenum = 0;
    ent->client->jumpdata->checkpoint_total = 0;
    ent->client->jumpdata->checkpoints_obtained.clear();
    ent->client->jumpdata->timer_checkpoint_split = 0;
    ent->client->jumpdata->timer_trigger_spam = 0;

    ent->svflags = 0;
    ent->solid = SOLID_TRIGGER;
    ent->clipmask = 0;
    ent->movetype = MOVETYPE_WALK;
    ent->flags = 0;
    ent->air_finished = 0;
    ent->touch_debounce_time = 0;
    ent->pain_debounce_time = 0;
    ent->damage_debounce_time = 0;
    ent->fly_sound_debounce_time = 0;
    ent->last_move_time = 0;
    ent->health = MaxHealthAndAmmo;
    ent->deadflag = DEAD_NO;
    ent->show_hostile = 0;
    ent->powerarmor_time = 0;
    ent->takedamage = DAMAGE_YES;
    ent->dmg = 0;
    ent->radius_dmg = 0;
    ent->dmg_radius = 0;
    ent->chain = nullptr;
    ent->enemy = nullptr;
    ent->oldenemy = nullptr;
    ent->activator = nullptr;
    ent->groundentity = nullptr;
    ent->groundentity_linkcount = 0;
    ent->teamchain = nullptr;
    ent->teammaster = nullptr;
    ent->teleport_time = 0;
    ent->watertype = 0;
    ent->waterlevel = 0;
    ent->item = nullptr;

    // If the player dies and turns into gibs, it messes up the size and view of this entity
    ent->viewheight = 22;
    vec3_t mins = { -16, -16, -24 };
    vec3_t maxs = { 16, 16, 32 };
    VectorCopy(mins, ent->mins);
    VectorCopy(maxs, ent->maxs);
    VectorClear(ent->absmin); // These all get set when the ent is linked
    VectorClear(ent->absmax);
    VectorClear(ent->size);
}

/// <summary>
/// Selects the player spawn point.
/// </summary>
/// <returns>Spawn ent or null if not found</returns>
edict_t* Spawn::SelectPlayerSpawn()
{
    edict_t* spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
    if (spot != NULL)
    {
        return spot;
    }
    spot = G_Find(NULL, FOFS(classname), "info_player_start");
    if (spot != NULL)
    {
        return spot;
    }
    return NULL;
}

/// <summary>
/// Selects the intermission spawn ent if it exists, else selects the player spawn point.
/// </summary>
/// <returns>Spawn ent or null if not found</returns>
edict_t* Spawn::SelectIntermissionSpawn()
{
    edict_t* spot = G_Find(NULL, FOFS(classname), "info_player_intermission");
    if (spot != NULL)
    {
        return spot;
    }
    return SelectPlayerSpawn();
}

/// <summary>
/// Gets the skin string for the given username and team.
/// </summary>
/// <param name="username"></param>
/// <param name="team"></param>
/// <returns></returns>
std::string Spawn::GetSkin(const std::string& username, TeamEnum team)
{
    switch (team)
    {
    case TeamEnum::Easy:
        return GetSkinEasy(username);
    case TeamEnum::Hard:
        return GetSkinHard(username);
    default:
        return GetSkinInvis(username);
    }
}

/// <summary>
/// Gets the easy team skin string.
/// </summary>
/// <param name="username"></param>
/// <returns></returns>
std::string Spawn::GetSkinEasy(const std::string& username)
{
    return username + "\\female/ctf_r";
}

/// <summary>
/// Gets the hard team skin string.
/// </summary>
/// <param name="username"></param>
/// <returns></returns>
std::string Spawn::GetSkinHard(const std::string& username)
{
    return username + "\\female/ctf_b";
}

/// <summary>
/// Gets the invisible skin string.
/// </summary>
/// <param name="username"></param>
/// <returns></returns>
std::string Spawn::GetSkinInvis(const std::string& username)
{
    return username + "\\female/invis";
}

/// <summary>
/// Sets the skin for the given ent based on its team.  Updates all connected clients.
/// </summary>
/// <param name="ent"></param>
void Spawn::AssignTeamSkin(edict_t* ent)
{
    int playernum = ent - g_edicts - 1;
    for (int i = 0; i < game.maxclients; i++)
    {
        edict_t* user = &g_edicts[i + 1];
        if (user->inuse && user->client != nullptr)
        {
            std::string skin;
            if (!user->client->jumpdata->show_jumpers)
            {
                skin = GetSkinInvis(ent->client->pers.netname);
            }
            else
            {
                skin = GetSkin(ent->client->pers.netname, ent->client->jumpdata->team);
            }
            gi.WriteByte(svc_configstring);
            gi.WriteShort(CS_PLAYERSKINS + playernum);
            gi.WriteString(const_cast<char*>(skin.c_str()));
            gi.unicast(user, true);
        }
    }
}

/// <summary>
/// This is called once whenever a player first enters a map.  Initialize everything to a clean state,
/// make the player a spectator, and open the main menu.
/// </summary>
/// <param name="ent"></param>
void Spawn::ClientOnEnterMap(edict_t* ent)
{
    gi.unlinkentity(ent);
    InitializeClientEnt(ent);

    if (level.intermissiontime)
    {
        MovePlayerToIntermission(ent);
    }
    else
    {
        InitAsSpectator(ent);

        edict_t* spawn = SelectPlayerSpawn();
        MovePlayerToSpawn(ent, spawn, false);

        OpenMenu_Join(ent);
    }

    gi.bprintf(PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
    gi.linkentity(ent);
    ClientEndServerFrame(ent);

    // TODO: Put this somewhere else?
    ent->client->jumpdata->cached_time_msec = LocalDatabase::GetMapTime(level.mapname, ent->client->pers.netname);
    ent->client->jumpdata->cached_completions = LocalDatabase::GetPlayerCompletions(level.mapname, ent->client->pers.netname);
    ent->client->jumpdata->cached_maps_completed = LocalDatabase::GetPlayerMapsCompletedCount(ent->client->pers.netname);
}

/// <summary>
/// Configures the client as a spectator.  Does not change their position in the world.
/// </summary>
/// <param name="ent"></param>
void Spawn::InitAsSpectator(edict_t* ent)
{
    gi.unlinkentity(ent);

    ent->s.frame = 0;
    ent->s.effects = 0;
    ent->s.renderfx = 0;
    ent->s.solid = SOLID_NOT;

    ent->client->ps.pmove.pm_type = PM_SPECTATOR;
    VectorClear(ent->client->ps.pmove.velocity);
    ent->client->ps.pmove.pm_flags = 0;
    ent->client->ps.pmove.pm_time = 0;
    ent->client->ps.pmove.gravity = 0;

    VectorClear(ent->client->ps.kick_angles);
    VectorClear(ent->client->ps.gunangles);
    VectorClear(ent->client->ps.gunoffset);
    ent->client->ps.gunindex = 0;
    ent->client->ps.gunframe = 0;
    ArrayClear(ent->client->ps.blend);
    ent->client->ps.rdflags = 0;
    ArrayClear(ent->client->ps.stats);

    ent->client->pers.health = MaxHealthAndAmmo;
    ent->client->pers.selected_item = 0;
    ArrayClear(ent->client->pers.inventory);
    ent->client->pers.weapon = nullptr;
    ent->client->pers.lastweapon = nullptr;

    ent->client->old_pmove.pm_type = PM_SPECTATOR;
    VectorClear(ent->client->old_pmove.origin);
    VectorClear(ent->client->old_pmove.velocity);
    ent->client->old_pmove.pm_flags = 0;
    ent->client->old_pmove.pm_time = 0;
    ent->client->old_pmove.gravity = 0;
    VectorClear(ent->client->old_pmove.delta_angles);

    ent->client->ammo_index = 0;
    ent->client->weapon_thunk = false;
    ent->client->newweapon = nullptr;
    ent->client->damage_armor = 0;
    ent->client->damage_parmor = 0;
    ent->client->damage_blood = 0;
    ent->client->damage_knockback = 0;
    VectorClear(ent->client->damage_from);
    ent->client->killer_yaw = 0;
    ent->client->weaponstate = WEAPON_READY;
    VectorClear(ent->client->kick_angles);
    VectorClear(ent->client->kick_origin);
    ent->client->v_dmg_roll = 0;
    ent->client->v_dmg_pitch = 0;
    ent->client->v_dmg_time = 0;
    ent->client->fall_time = 0;
    ent->client->fall_value = 0;
    ent->client->damage_alpha = 0;
    ent->client->bonus_alpha = 0;
    VectorClear(ent->client->damage_blend);
    VectorClear(ent->client->v_angle);
    ent->client->bobtime = 0;
    VectorClear(ent->client->oldviewangles);
    VectorClear(ent->client->oldvelocity);
    ent->client->next_drown_time = 0;
    ent->client->old_waterlevel = 0;
    ent->client->breather_sound = 0;
    ent->client->machinegun_shots = 0;
    ent->client->anim_end = 0;
    ent->client->anim_priority = ANIM_BASIC;
    ent->client->anim_duck = false;
    ent->client->anim_run = false;
    ent->client->quad_framenum = 0;
    ent->client->invincible_framenum = 0;
    ent->client->breather_framenum = 0;
    ent->client->enviro_framenum = 0;
    ent->client->grenade_blew_up = false;
    ent->client->grenade_time = 0;
    ent->client->silencer_shots = 0;
    ent->client->weapon_sound = 0;
    ent->client->pickup_msg_time = 0;
    ent->client->chase_target = nullptr;
    ent->client->update_chase = false;

    ent->client->jumpdata->replay_recording.clear();
    ent->client->jumpdata->team = TeamEnum::Spectator;
    ent->client->jumpdata->timer_pmove_msec = 0;
    ent->client->jumpdata->timer_begin = 0;
    ent->client->jumpdata->timer_end = 0;
    ent->client->jumpdata->timer_paused = true;
    ent->client->jumpdata->timer_finished = false;
    ent->client->jumpdata->hud_footer1.clear();
    ent->client->jumpdata->hud_footer2.clear();
    ent->client->jumpdata->hud_footer3.clear();
    ent->client->jumpdata->hud_footer4.clear();
    ent->client->jumpdata->checkpoint_total = 0;
    ent->client->jumpdata->checkpoints_obtained.clear();
    ent->client->jumpdata->timer_checkpoint_split = 0;
    ent->client->jumpdata->timer_trigger_spam = 0;

    ent->svflags |= SVF_NOCLIENT;
    ent->solid = SOLID_NOT;
    ent->clipmask = 0;
    ent->movetype = MOVETYPE_NOCLIP;
    ent->flags = 0;
    ent->freetime = 0;
    ent->air_finished = 0;
    ent->gravity = 1.0f;
    ent->yaw_speed = 0;
    ent->ideal_yaw = 0;
    ent->touch_debounce_time = 0;
    ent->pain_debounce_time = 0;
    ent->damage_debounce_time = 0;
    ent->fly_sound_debounce_time = 0;
    ent->last_move_time = 0;
    ent->health = MaxHealthAndAmmo;
    ent->deadflag = DEAD_NO;
    ent->powerarmor_time = 0;
    ent->takedamage = DAMAGE_NO;
    ent->dmg = 0;
    ent->radius_dmg = 0;
    ent->dmg_radius = 0;
    ent->teleport_time = 0;
    ent->watertype = 0;
    ent->waterlevel = 0;

    gi.linkentity(ent);
}

/// <summary>
/// Moves the player to the intermission spawn and opens the highscores menu.
/// </summary>
/// <param name="ent"></param>
void Spawn::MovePlayerToIntermission(edict_t* ent)
{
    InitAsSpectator(ent);

    edict_t* spawn = SelectIntermissionSpawn();
    MovePlayerToSpawn(ent, spawn, false);

    PMenu_Close(ent);
    ShowBestTimesScoreboard(ent);
}

/// <summary>
/// Moves a player to a spawn position.
/// </summary>
/// <param name="ent"></param>
/// <param name="spawn"></param>
/// <param name="useTeleportEffects"></param>
void Spawn::MovePlayerToSpawn(edict_t* ent, edict_t* spawn, bool useTeleportEffects)
{
    MovePlayerToPosition(ent, spawn->s.origin, spawn->s.angles, useTeleportEffects);
}

/// <summary>
/// Moves a player to a position in the world specified by origin and angles.
/// </summary>
/// <param name="ent"></param>
/// <param name="origin"></param>
/// <param name="angles"></param>
/// <param name="useTeleportEffects"></param>
void Spawn::MovePlayerToPosition(edict_t* ent, const vec3_t& origin, const vec3_t& angles, bool useTeleportEffects)
{
    // Code taken from teleporter_touch().
    gi.unlinkentity(ent);

    VectorCopy(origin, ent->s.origin);
    VectorCopy(origin, ent->s.old_origin);
    ent->s.origin[2] += 10;
    VectorClear(ent->velocity);

    if (useTeleportEffects)
    {
        ent->client->ps.pmove.pm_time = 1; // this cannot be 0 when using the teleport flag or else the player cannot move
        ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
    }

    // set angles
    for (int i = 0; i < 3; i++)
    {
        ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(angles[i] - ent->client->resp.cmd_angles[i]);
    }

    VectorClear(ent->s.angles);
    VectorClear(ent->client->ps.viewangles);
    VectorClear(ent->client->v_angle);

    gi.linkentity(ent);
}

/// <summary>
/// Initializes the given player ent.  Sets all ent fields to clean default values.
/// </summary>
/// <param name="ent"></param>
void Spawn::InitializeClientEnt(edict_t* ent)
{
    // Set all fields of ent->s
    ent->s.number = ent - g_edicts;
    VectorClear(ent->s.origin);
    VectorClear(ent->s.angles);
    VectorClear(ent->s.old_origin);
    ent->s.modelindex = 255;
    ent->s.modelindex2 = 255;
    ent->s.modelindex3 = 0;
    ent->s.modelindex4 = 0;
    ent->s.frame = 0;
    ent->s.skinnum = ent - g_edicts - 1;
    ent->s.effects = 0;
    ent->s.renderfx = 0;
    ent->s.solid = SOLID_TRIGGER;
    ent->s.sound = 0;
    ent->s.event = 0;

    // Set all fields of ent->client
    ent->client->ps.pmove.pm_type = PM_SPECTATOR;
    VectorClear(ent->client->ps.pmove.origin);
    VectorClear(ent->client->ps.pmove.velocity);
    ent->client->ps.pmove.pm_flags = 0;
    ent->client->ps.pmove.pm_time = 0;
    ent->client->ps.pmove.gravity = 0; // TODO?
    VectorClear(ent->client->ps.pmove.delta_angles);

    VectorClear(ent->client->ps.viewangles);
    VectorClear(ent->client->ps.viewoffset);
    VectorClear(ent->client->ps.kick_angles);
    VectorClear(ent->client->ps.gunangles);
    VectorClear(ent->client->ps.gunoffset);
    ent->client->ps.gunindex = 0;
    ent->client->ps.gunframe = 0;
    ArrayClear(ent->client->ps.blend);
    ent->client->ps.fov; // TODO: already set?
    ent->client->ps.rdflags = 0;
    ArrayClear(ent->client->ps.stats);

    ent->client->ping; // TODO: already set?

    ent->client->pers.userinfo; // Already set
    ent->client->pers.netname; // Already set
    ent->client->pers.hand; // Already set
    ent->client->pers.connected = true;
    ent->client->pers.health = MaxHealthAndAmmo;
    ent->client->pers.max_health = MaxHealthAndAmmo;
    ent->client->pers.savedFlags = 0;
    ent->client->pers.selected_item = 0;
    ArrayClear(ent->client->pers.inventory);
    ent->client->pers.max_bullets = MaxHealthAndAmmo;
    ent->client->pers.max_shells = MaxHealthAndAmmo;
    ent->client->pers.max_rockets = MaxHealthAndAmmo;
    ent->client->pers.max_grenades = MaxHealthAndAmmo;
    ent->client->pers.max_cells = MaxHealthAndAmmo;
    ent->client->pers.max_slugs = MaxHealthAndAmmo;
    ent->client->pers.weapon = nullptr;
    ent->client->pers.lastweapon = nullptr;
    ent->client->pers.power_cubes = 0;
    ent->client->pers.score = 0;

    ent->client->resp.enterframe = level.framenum;
    ent->client->resp.score = 0;
    VectorClear(ent->client->resp.cmd_angles);
    ent->client->resp.game_helpchanged = 0;
    ent->client->resp.helpchanged = 0;

    ent->client->old_pmove.pm_type = PM_SPECTATOR;
    VectorClear(ent->client->old_pmove.origin);
    VectorClear(ent->client->old_pmove.velocity);
    ent->client->old_pmove.pm_flags = 0;
    ent->client->old_pmove.pm_time = 0;
    ent->client->old_pmove.gravity = 0; // TODO?
    VectorClear(ent->client->old_pmove.delta_angles);

    ent->client->showscores = false;
    ent->client->inmenu = false;
    ent->client->menu = nullptr;
    ent->client->showinventory = false;
    ent->client->showhelp = false;
    ent->client->showhelpicon = false;
    ent->client->ammo_index = 0;
    ent->client->buttons = 0;
    ent->client->oldbuttons = 0;
    ent->client->latched_buttons = 0;
    ent->client->weapon_thunk = false;
    ent->client->newweapon = nullptr;
    ent->client->damage_armor = 0;
    ent->client->damage_parmor = 0;
    ent->client->damage_blood = 0;
    ent->client->damage_knockback = 0;
    VectorClear(ent->client->damage_from);
    ent->client->killer_yaw = 0;
    ent->client->weaponstate = WEAPON_READY;
    VectorClear(ent->client->kick_angles);
    VectorClear(ent->client->kick_origin);
    ent->client->v_dmg_roll = 0;
    ent->client->v_dmg_pitch = 0;
    ent->client->v_dmg_time = 0;
    ent->client->fall_time = 0;
    ent->client->fall_value = 0;
    ent->client->damage_alpha = 0;
    ent->client->bonus_alpha = 0;
    VectorClear(ent->client->damage_blend);
    VectorClear(ent->client->v_angle);
    ent->client->bobtime = 0;
    VectorClear(ent->client->oldviewangles);
    VectorClear(ent->client->oldvelocity);
    ent->client->next_drown_time = 0;
    ent->client->old_waterlevel = 0;
    ent->client->breather_sound = 0;
    ent->client->machinegun_shots = 0;
    ent->client->anim_end = 0;
    ent->client->anim_priority = ANIM_BASIC;
    ent->client->anim_duck = false;
    ent->client->anim_run = false;
    ent->client->quad_framenum = 0;
    ent->client->invincible_framenum = 0;
    ent->client->breather_framenum = 0;
    ent->client->enviro_framenum = 0;
    ent->client->grenade_blew_up = false;
    ent->client->grenade_time = 0;
    ent->client->silencer_shots = 0;
    ent->client->weapon_sound = 0;
    ent->client->pickup_msg_time = 0;
    ent->client->flood_locktill = 0;
    ArrayClear(ent->client->flood_when);
    ent->client->flood_whenhead = 0;
    ent->client->respawn_time = 0; // TODO: could set this maybe
    ent->client->chase_target = nullptr;
    ent->client->update_chase = false;
    ent->client->menutime = 0;
    ent->client->menudirty = false;

    ent->client->jumpdata->replay_recording.clear();
    ent->client->jumpdata->replay_spectating.clear();
    ent->client->jumpdata->replay_spectating_framenum = 0;
    ent->client->jumpdata->update_replay_spectating = false;
    ent->client->jumpdata->replay_spectating_hud_string.clear();
    ent->client->jumpdata->localUserId; // Already set
    ent->client->jumpdata->fps; // Already set
    ent->client->jumpdata->async; // Already set
    ent->client->jumpdata->ip; // Already set
    ent->client->jumpdata->team = TeamEnum::Spectator;
    ent->client->jumpdata->timer_pmove_msec = 0;
    ent->client->jumpdata->timer_begin = 0;
    ent->client->jumpdata->timer_end = 0;
    ent->client->jumpdata->timer_paused = true;
    ent->client->jumpdata->timer_finished = false;
    ent->client->jumpdata->stores = {};
    ent->client->jumpdata->store_ent = nullptr;
    ent->client->jumpdata->key_states = 0;
    ent->client->jumpdata->scores_menu = ScoresMenuEnum::None;
    ent->client->jumpdata->racing = false;
    ent->client->jumpdata->racing_frames.clear();
    ent->client->jumpdata->racing_framenum = 0;
    ent->client->jumpdata->racing_delay_frames = 0;
    ent->client->jumpdata->racing_highscore = 0;
    ent->client->jumpdata->hud_footer1.clear();
    ent->client->jumpdata->hud_footer2.clear();
    ent->client->jumpdata->hud_footer3.clear();
    ent->client->jumpdata->hud_footer4.clear();
    ent->client->jumpdata->show_jumpers = true;
    ent->client->jumpdata->checkpoint_total = 0;
    ent->client->jumpdata->checkpoints_obtained.clear();
    ent->client->jumpdata->timer_checkpoint_split = 0;
    ent->client->jumpdata->timer_trigger_spam = 0;
    ent->client->jumpdata->cached_time_msec = 0;
    ent->client->jumpdata->cached_completions = 0;
    ent->client->jumpdata->cached_maps_completed = 0;

    // Set the rest of ent fields
    ent->inuse = true;

    ent->linkcount = 0; // All this link stuff gets set when the ent is linked
    ent->area.prev = nullptr;
    ent->area.next = nullptr;
    ent->num_clusters = 0;
    ArrayClear(ent->clusternums);
    ent->headnode = 0;
    ent->areanum = 0;
    ent->areanum2 = 0;

    ent->svflags = 0;

    vec3_t mins = { -16, -16, -24 };
    vec3_t maxs = { 16, 16, 32 };
    VectorCopy(mins, ent->mins);
    VectorCopy(maxs, ent->maxs);

    VectorClear(ent->absmin); // These all get set when the ent is linked
    VectorClear(ent->absmax);
    VectorClear(ent->size);

    ent->solid = SOLID_TRIGGER;
    ent->clipmask = 0;
    ent->owner = nullptr;
    ent->movetype = MOVETYPE_NOCLIP;
    ent->flags = 0;
    ent->model = "players/female/tris.md2";
    ent->freetime = 0;
    ent->message = nullptr;
    ent->classname = "player";
    ent->spawnflags = 0;
    ent->timestamp = 0;
    ent->angle = 0;
    ent->target = nullptr;
    ent->targetname = nullptr;
    ent->killtarget = nullptr;
    ent->team = nullptr;
    ent->pathtarget = nullptr;
    ent->deathtarget = nullptr;
    ent->combattarget = nullptr;
    ent->target_ent = nullptr;
    ent->speed = 0;
    ent->accel = 0;
    ent->decel = 0;
    VectorClear(ent->movedir);
    VectorClear(ent->pos1);
    VectorClear(ent->pos2);
    VectorClear(ent->velocity);
    VectorClear(ent->avelocity);
    ent->mass = 200;
    ent->air_finished = 0;
    ent->gravity = 1.0f;
    ent->goalentity = nullptr;
    ent->movetarget = nullptr;
    ent->yaw_speed = 0;
    ent->ideal_yaw = 0;
    ent->nextthink = 0;
    ent->prethink = nullptr;
    ent->think = nullptr;
    ent->blocked = nullptr;
    ent->touch = nullptr;
    ent->use = nullptr;
    ent->pain = player_pain;
    ent->die = player_die;
    ent->touch_debounce_time = 0;
    ent->pain_debounce_time = 0;
    ent->damage_debounce_time = 0;
    ent->fly_sound_debounce_time = 0;
    ent->last_move_time = 0;
    ent->health = MaxHealthAndAmmo;
    ent->max_health = MaxHealthAndAmmo;
    ent->gib_health = 0;
    ent->deadflag = DEAD_NO;
    ent->show_hostile = 0;
    ent->powerarmor_time = 0;
    ent->map = nullptr;
    ent->viewheight = 22;
    ent->takedamage = DAMAGE_NO;
    ent->dmg = 0;
    ent->radius_dmg = 0;
    ent->dmg_radius = 0;
    ent->sounds = 0;
    ent->count = 0;
    ent->chain = nullptr;
    ent->enemy = nullptr;
    ent->oldenemy = nullptr;
    ent->activator = nullptr;
    ent->groundentity = nullptr;
    ent->groundentity_linkcount = 0;
    ent->teamchain = nullptr;
    ent->teammaster = nullptr;
    ent->mynoise = nullptr;
    ent->mynoise2 = nullptr;
    ent->noise_index = 0;
    ent->noise_index2 = 0;
    ent->volume = 0;
    ent->attenuation = 0;
    ent->wait = 0;
    ent->delay = 0;
    ent->random = 0;
    ent->teleport_time = 0;
    ent->watertype = 0;
    ent->waterlevel = 0;
    VectorClear(ent->move_origin);
    VectorClear(ent->move_angles);
    ent->light_level = 0;
    ent->style = 0;
    ent->item = nullptr;
    memset(&ent->moveinfo, 0, sizeof(ent->moveinfo));
    memset(&ent->monsterinfo, 0, sizeof(ent->monsterinfo));
}

} // namespace Jump