#include "jump.h"

namespace Jump
{
    static const int DefaultHealth = 1000;

    static const char* StoreModel = "models/monsters/commandr/head/tris.md2";

    static const char* SexTeamEasy = "female";
    static const char* SexTeamHard = "male";
    static const char* SkinTeamEasy = "ctf_r";
    static const char* SkinTeamHard = "ctf_b";

    static const int MaxMenuWidth = 32; // characters
    static const int MapNameLine = 2;
    static const int JoinEasyLine = 5;
    static const int NumPlayersEasyLine = 6;
    static const int NumPlayersHardLine = 8;
    static const int ChaseCamLine = 9;

    static pmenu_t Menu_Join[] = {
        { "*Quake II Jump",               PMENU_ALIGN_CENTER, NULL },
        { NULL,                           PMENU_ALIGN_CENTER, NULL },
        { NULL /* mapname */,             PMENU_ALIGN_CENTER, NULL },
        { NULL,                           PMENU_ALIGN_CENTER, NULL },
        { NULL,                           PMENU_ALIGN_CENTER, NULL },
        { "Join Easy Team",               PMENU_ALIGN_LEFT,   JoinTeamEasy },
        { NULL /* number of players */,   PMENU_ALIGN_LEFT,   NULL },
        { "Join Hard Team",               PMENU_ALIGN_LEFT,   JoinTeamHard },
        { NULL /* number of players */,   PMENU_ALIGN_LEFT,   NULL },
        { NULL /* chase camera */,        PMENU_ALIGN_LEFT,   JoinChaseCam },
        { NULL,                           PMENU_ALIGN_LEFT,   NULL }, // TODO help commands menu
        { NULL,                           PMENU_ALIGN_LEFT,   NULL },
        { "Highlight your choice and",    PMENU_ALIGN_LEFT,   NULL },
        { "press ENTER.",                 PMENU_ALIGN_LEFT,   NULL },
        { "v" JUMP_STRING_VERSION,        PMENU_ALIGN_RIGHT,  NULL },
    };


    // ent must be a client
    // No other menus can be open when this function is called
    void OpenMenu_Join(edict_t* ent)
    {
        char mapName[MaxMenuWidth] = { 0 };
        char playersEasy[MaxMenuWidth] = { 0 };
        char playersHard[MaxMenuWidth] = { 0 };
        char chaseCam[MaxMenuWidth] = { 0 };

        strncpy(mapName, level.mapname, MaxMenuWidth - 1);
        sprintf(playersEasy, "  (%d players)", CountPlayersOnTeam(TEAM_EASY));
        sprintf(playersHard, "  (%d players)", CountPlayersOnTeam(TEAM_HARD));

        Menu_Join[MapNameLine].text = mapName;
        Menu_Join[NumPlayersEasyLine].text = playersEasy;
        Menu_Join[NumPlayersHardLine].text = playersHard;
        if (ent->client->chase_target)
        {
            Menu_Join[ChaseCamLine].text = "Leave Chase Camera";
        }
        else
        {
            Menu_Join[ChaseCamLine].text = "Chase Camera";
        }

        int cursor = JoinEasyLine;
        PMenu_Open(ent, Menu_Join, cursor, sizeof(Menu_Join) / sizeof(pmenu_t), NULL);
    }

    int CountPlayersOnTeam(team_t team)
    {
        int count = 0;
        for (int i = 0; i < game.maxclients; ++i)
        {
            const gclient_t* client = &game.clients[i];
            if (client != NULL && client->resp.jump_team == team)
            {
                count++;
            }
        }
        return count;
    }

    void JoinTeam(edict_t* ent, team_t team)
    {
        ent->client->resp.jump_team = team;
        AssignTeamSkin(ent);
        SpawnForJumping(ent);
    }

    void JoinTeamEasy(edict_t* ent, pmenuhnd_t* hnd)
    {
        PMenu_Close(ent);
        JoinTeam(ent, TEAM_EASY);
    }

    void JoinTeamHard(edict_t* ent, pmenuhnd_t* hnd)
    {
        PMenu_Close(ent);
        JoinTeam(ent, TEAM_HARD);
    }

    void JoinChaseCam(edict_t* ent, pmenuhnd_t* hnd) {}


    bool JumpClientCommand(edict_t* ent)
    {
        char* cmd = gi.argv(0);

        if (Q_stricmp(cmd, "inven") == 0)
        {
            Cmd_Jump_Inven(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "noclip") == 0)
        {
            Cmd_Jump_Noclip(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "test") == 0)
        {
            Cmd_Jump_Test(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "kill") == 0)
        {
            Cmd_Jump_Kill(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "recall") == 0)
        {
            Cmd_Jump_Recall(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "store") == 0)
        {
            Cmd_Jump_Store(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "reset") == 0)
        {
            Cmd_Jump_Reset(ent);
            return true;
        }
        else
        {
            return false;
        }
    }

    void Cmd_Jump_Inven(edict_t* ent)
    {
        if (ent->client->menu) {
            PMenu_Close(ent);
            ent->client->update_chase = true; // TODO what is this for?
        }
        else
        {
            OpenMenu_Join(ent);
        }
    }

    void Cmd_Jump_Noclip(edict_t* ent)
    {
        if (ent->client->resp.jump_team == TEAM_EASY)
        {
            if (ent->movetype == MOVETYPE_NOCLIP)
            {
                ent->movetype = MOVETYPE_WALK;
                gi.cprintf(ent, PRINT_HIGH, "noclip OFF\n");
            }
            else
            {
                ent->movetype = MOVETYPE_NOCLIP;
                gi.cprintf(ent, PRINT_HIGH, "noclip ON\n");
            }
        }
    }

    // A function used to test stuff for development
    void Cmd_Jump_Test(edict_t* ent)
    {
        if (ent->client->menu) 
        {
            PMenu_Close(ent);
        }
        ent->client->showscores = true;
        
        char string[1400] = { 0 };

        Com_sprintf(string, sizeof(string),
            "xv -16 yv 0 string2 \"Ping Pos Player          Best Comp Maps     %%\" ");

        gi.WriteByte(svc_layout);
        gi.WriteString(string);
        gi.unicast(ent, true);
    }

    void Cmd_Jump_Kill(edict_t* ent)
    {
        SpawnForJumping(ent);
    }

    void Cmd_Jump_Recall(edict_t* ent)
    {
        if (ent->client->resp.jump_team == TEAM_EASY)
        {
            if (ent->client->store_buffer.HasStore())
            {
                int num = 1;
                if (gi.argc() >= 2)
                {
                    num = atoi(gi.argv(1));
                    if (num == 0)
                    {
                        num++; // convert "recall 0" into "recall 1"
                    }
                }
                store_data_t data = ent->client->store_buffer.GetStore(num);
                SpawnAtStorePosition(ent, data);
            }
            else
            {
                SpawnForJumping(ent);
            }
        }
        else
        {
            SpawnForJumping(ent);
        }
    }

    void Cmd_Jump_Store(edict_t* ent)
    {
        // Save all of the state
        store_data_t data = { 0 };
        data.time_interval = Sys_Milliseconds() - ent->client->resp.jump_timer_begin;
        VectorCopy(ent->s.origin, data.pos);
        VectorCopy(ent->s.angles, data.angles);
        data.angles[ROLL] = 0; // fixes the problem where the view is tilted after recall
        ent->client->store_buffer.PushStore(data);

        // Place an entity at the position
        if (ent->client->store_ent)
        {
            G_FreeEdict(ent->client->store_ent);
            ent->client->store_ent = NULL;
        }
        ent->client->store_ent = G_Spawn();
        VectorCopy(data.pos, ent->client->store_ent->s.origin);
        VectorCopy(data.pos, ent->client->store_ent->s.old_origin);
        ent->client->store_ent->s.old_origin[2] -= 10;
        ent->client->store_ent->s.origin[2] -= 10;
        ent->client->store_ent->svflags = SVF_PROJECTILE;
        VectorCopy(data.angles, ent->client->store_ent->s.angles);
        ent->client->store_ent->movetype = MOVETYPE_NONE;
        ent->client->store_ent->clipmask = MASK_PLAYERSOLID;
        ent->client->store_ent->solid = SOLID_NOT;
        ent->client->store_ent->s.renderfx = RF_TRANSLUCENT;
        VectorClear(ent->client->store_ent->mins);
        VectorClear(ent->client->store_ent->maxs);
        ent->client->store_ent->s.modelindex = gi.modelindex(const_cast<char*>(StoreModel));
        ent->client->store_ent->dmg = 0;
        ent->client->store_ent->classname = "store_ent";
        gi.linkentity(ent->client->store_ent);
    }

    void Cmd_Jump_Reset(edict_t* ent)
    {
        ent->client->store_buffer.Reset();
        if (ent->client->store_ent)
        {
            G_FreeEdict(ent->client->store_ent);
            ent->client->store_ent = NULL;
        }
    }

    void AssignTeamSkin(edict_t* ent)
    {
        // TODO: doesn't seem to be working with the model
        int playernum = ent - g_edicts - 1;

        switch (ent->client->resp.jump_team)
        {
        case TEAM_EASY:
            gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s/%s", ent->client->pers.netname, SexTeamEasy, SkinTeamEasy));
            break;
        case TEAM_HARD:
            gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s/%s", ent->client->pers.netname, SexTeamHard, SkinTeamHard));
            break;
        default:
            gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\female/invis", ent->client->pers.netname));
            break;
        }
    }

    edict_t* SelectJumpSpawnPoint()
    {
        edict_t* spot = G_Find(NULL, FOFS(classname), "info_player_start");
        if (spot != NULL)
        {
            return spot;
        }
        spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
        if (spot != NULL)
        {
            return spot;
        }
        return NULL;
    }

    void ResetJumpTimer(edict_t* ent)
    {
        ent->client->resp.jump_count = 0;
        ent->client->resp.jump_timer_begin = 0;
        ent->client->resp.jump_timer_finished = false;
        ent->client->resp.jump_timer_paused = true;
    }

    // This is called once whenever a player first enters a map.
    // We want to initialize everything to a clean state,
    // make the player a spectator, and open the join team menu.
    void ClientBeginJump(edict_t* ent)
    {
        G_InitEdict(ent);
        memset(&ent->client->ps, 0, sizeof(ent->client->ps));
        InitClientResp(ent->client);

        char userinfo[MAX_INFO_STRING];
        memcpy(userinfo, ent->client->pers.userinfo, sizeof(userinfo));
        InitClientPersistant(ent->client);

        FetchClientEntData(ent);

        ent->groundentity = NULL;
        ent->takedamage = DAMAGE_AIM;
        ent->viewheight = 22;
        ent->inuse = true;
        ent->classname = "player";
        ent->mass = 200;
        ent->deadflag = DEAD_NO;
        ent->air_finished = level.time + 12; // TODO: this is water air time, need this?
        ent->clipmask = MASK_PLAYERSOLID;
        ent->pain = player_pain;
        ent->die = player_die;
        ent->waterlevel = 0;
        ent->watertype = 0;
        ent->flags &= ~FL_NO_KNOCKBACK;
        ent->svflags &= ~SVF_DEADMONSTER;

        vec3_t mins = { -16, -16, -24 };
        vec3_t maxs = { 16, 16, 32 };
        VectorCopy(mins, ent->mins);
        VectorCopy(maxs, ent->maxs);
        VectorClear(ent->velocity);

        ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

        // clear entity state values
        ent->s.effects = 0;
        ent->s.skinnum = ent - g_edicts - 1;
        ent->s.modelindex = 255;        // will use the skin specified model
        ent->s.modelindex2 = 255;       // custom gun model
        ent->s.skinnum = ent - g_edicts - 1;
        ent->s.frame = 0;

        InitAsSpectator(ent);
        ClientUserinfoChanged(ent, userinfo);

        if (level.intermissiontime || level.state == STATE_VOTING)
        {
            // TODO move to intermission
            MoveClientToIntermission(ent);
        }
        else
        {
            vec3_t spawn_origin = { 0 };
            vec3_t spawn_angles = { 0 };
            SelectSpawnPoint(ent, spawn_origin, spawn_angles);
            MoveClientToPosition(ent, spawn_origin, spawn_angles);
            
            // Send updates to the client before opening the menu
            gi.linkentity(ent);

            OpenMenu_Join(ent);
        }

        gi.bprintf(PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

        // make sure all view stuff is valid
        ClientEndServerFrame(ent);
    }

    void InitAsSpectator(edict_t* ent)
    {
        ent->movetype = MOVETYPE_NOCLIP;
        ent->solid = SOLID_NOT;
        ent->svflags |= SVF_NOCLIENT;
        ent->client->resp.ctf_team = CTF_NOTEAM;
        ent->client->ps.gunindex = 0;
        ent->client->resp.jump_team = TEAM_SPECTATOR;
    }

    void MoveClientToPosition(edict_t* ent, vec3_t origin, vec3_t angles)
    {
        // Stop all previous movement
        ent->waterlevel = 0;
        ent->watertype = 0;
        VectorClear(ent->velocity);

        ent->client->ps.pmove.origin[0] = origin[0] * 8;
        ent->client->ps.pmove.origin[1] = origin[1] * 8;
        ent->client->ps.pmove.origin[2] = origin[2] * 8;

        VectorCopy(origin, ent->s.origin);
        ent->s.origin[2] += 1;	// make sure off ground
        VectorCopy(ent->s.origin, ent->s.old_origin);

        ent->s.angles[PITCH] = 0;
        ent->s.angles[YAW] = angles[YAW];
        ent->s.angles[ROLL] = 0;
        VectorCopy(ent->s.angles, ent->client->ps.viewangles);
        VectorCopy(ent->s.angles, ent->client->v_angle);
        VectorClear(ent->client->ps.viewoffset);

        for (int i = 0; i < 3; i++)
        {
            ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->s.angles[i] - ent->client->resp.cmd_angles[i]);
        }
    }

    void SpawnForJumping(edict_t* ent)
    {
        // Remove spectator flags
        ent->movetype = MOVETYPE_WALK;
        ent->solid = SOLID_BBOX;
        ent->flags = 0;
        ent->svflags = 0;
        ent->deadflag = DEAD_NO;

        // Clear inventory of all weapons and ammo and change to blaster
        // TODO: simplify the weapon change code
        char userinfo[MAX_INFO_STRING] = { 0 };
        memcpy(userinfo, ent->client->pers.userinfo, sizeof(userinfo));
        InitClientPersistant(ent->client); // TODO: don't like this, should just set all the vars
        ClientUserinfoChanged(ent, userinfo);

        InitClientForRespawn(ent);

        // Move to spawn
        vec3_t spawn_origin = { 0 };
        vec3_t spawn_angles = { 0 };
        SelectSpawnPoint(ent, spawn_origin, spawn_angles);
        MoveClientToPosition(ent, spawn_origin, spawn_angles);

        // Reset timer
        ent->client->resp.jump_count = 0;
        ent->client->resp.jump_timer_begin = 0;
        ent->client->resp.jump_timer_finished = false;
        ent->client->resp.jump_timer_paused = true;

        // Clear any special move effects
        ent->s.event = EV_NONE;
        ent->client->ps.pmove.pm_flags = 0;
        ent->client->ps.pmove.pm_time = 0;

        gi.linkentity(ent);
    }

    void InitClientForRespawn(edict_t* ent)
    {
        // Clear out the entire inventory
        memset(ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));

        // Unequip all weapons
        ent->client->pers.weapon = NULL;
        ent->client->pers.lastweapon = NULL;
        ent->client->pers.selected_item = 0;
        VectorClear(ent->client->ps.gunangles);
        VectorClear(ent->client->ps.gunoffset);
        ent->client->ps.gunframe = 0;
        ent->client->ps.gunindex = 0;

        ent->client->pers.health = DefaultHealth;
        ent->health = DefaultHealth;

        // Score is unused in Jump
        ent->client->resp.score = 0;
        ent->client->pers.score = 0;
    }

    void SpawnAtStorePosition(edict_t* ent, store_data_t data)
    {
        if (ent->client->resp.jump_team == TEAM_HARD) {
            // Should never get here!
            gi.error("Uh oh, tried to use a store on team hard.");
            return;
        }
        ent->client->resp.jump_timer_begin = Sys_Milliseconds() - data.time_interval;
        MoveClientToPosition(ent, data.pos, data.angles);
    }


    StoreBuffer::StoreBuffer() : numStores(0), nextIndex(0), stores()
    {
    }

    void StoreBuffer::PushStore(const store_data_t& data)
    {
        stores[nextIndex] = data;
        nextIndex = (nextIndex + 1) % MAX_STORES;
        if (numStores < MAX_STORES)
        {
            numStores++;
        }
    }

    store_data_t StoreBuffer::GetStore(int prevNum)
    {
        int desired = prevNum;
        if (desired > numStores)
        {
            // If a user asks for a prev that is larger than
            // what we have stored, we return the oldest data.
            desired = numStores;
        }
        int index = nextIndex - desired;
        if (index < 0)
        {
            index = MAX_STORES - abs(nextIndex - desired);
        }
        return stores[index];
    }

    void StoreBuffer::Reset()
    {
        nextIndex = 0;
        numStores = 0;
        memset(stores, 0, sizeof(stores));
    }

    bool StoreBuffer::HasStore()
    {
        return numStores > 0;
    }
}