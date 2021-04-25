#include "jump_spawn.h"
#include "jump_menu.h"

namespace Jump
{

/// <summary>
/// Class variables
/// </summary>
const int Spawn::MaxHealthAndAmmo = 1000;

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
std::string Spawn::GetSkin(const std::string& username, team_t team)
{
    switch (team)
    {
    case TEAM_EASY:
        return GetSkinEasy(username);
    case TEAM_HARD:
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
    // Code taken from teleporter_touch().
    gi.unlinkentity(ent);

    VectorCopy(spawn->s.origin, ent->s.origin);
    VectorCopy(spawn->s.origin, ent->s.old_origin);
    ent->s.origin[2] += 10;
    VectorClear(ent->velocity);

    if (useTeleportEffects)
    {
        ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
    }

    // set angles
    for (int i = 0; i < 3; i++)
    {
        ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn->s.angles[i] - ent->client->resp.cmd_angles[i]);
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
    ent->client->jumpdata->team = TEAM_SPECTATOR;
    ent->client->jumpdata->timer_pmove_msec = 0;
    ent->client->jumpdata->timer_begin = 0;
    ent->client->jumpdata->timer_end = 0;
    ent->client->jumpdata->timer_paused = true;
    ent->client->jumpdata->timer_finished = false;
    ent->client->jumpdata->stores = {};
    ent->client->jumpdata->store_ent = nullptr;
    ent->client->jumpdata->key_states = 0;
    ent->client->jumpdata->scores_menu = SCORES_MENU_NONE;
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