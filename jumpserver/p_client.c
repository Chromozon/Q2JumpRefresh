/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "g_local.h"
#include "m_player.h"
#include "jump.h"
#include "jump_utils.h"
#include "jump_hud.h"
#include "jump_scores.h"
#include "jump_logger.h"
#include "g_chase.h"
#include "jump_local_database.h"
#include "jump_spawn.h"
#include "jump_types.h"
#include "jump_msets.h"

void SP_misc_teleporter_dest (edict_t *ent);

//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

static void SP_FixCoopSpots (edict_t *self)
{
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384)
		{
			if ((!self->targetname) || stricmp(self->targetname, spot->targetname) != 0)
			{
//				gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
				self->targetname = spot->targetname;
			}
			return;
		}
	}
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
	edict_t	*spot;

	if(stricmp(level.mapname, "security") == 0)
	{
		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 128;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		return;
	}
}


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
	if (!coop->value)
		return;
	if(stricmp(level.mapname, "security") == 0)
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_CreateCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}

	if((stricmp(level.mapname, "jail2") == 0)   ||
	   (stricmp(level.mapname, "jail4") == 0)   ||
	   (stricmp(level.mapname, "mine1") == 0)   ||
	   (stricmp(level.mapname, "mine2") == 0)   ||
	   (stricmp(level.mapname, "mine3") == 0)   ||
	   (stricmp(level.mapname, "mine4") == 0)   ||
	   (stricmp(level.mapname, "lab") == 0)     ||
	   (stricmp(level.mapname, "boss1") == 0)   ||
	   (stricmp(level.mapname, "fact3") == 0)   ||
	   (stricmp(level.mapname, "biggun") == 0)  ||
	   (stricmp(level.mapname, "space") == 0)   ||
	   (stricmp(level.mapname, "command") == 0) ||
	   (stricmp(level.mapname, "power2") == 0) ||
	   (stricmp(level.mapname, "strike") == 0))
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_FixCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(edict_t *ent)
{
}

void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	if (dir[0])
		self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
	else {
		self->client->killer_yaw = 0;
		if (dir[1] > 0)
			self->client->killer_yaw = 90;
		else if (dir[1] < 0)
			self->client->killer_yaw = -90;
	}
	if (self->client->killer_yaw < 0)
		self->client->killer_yaw += 360;
}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
//ZOID
	self->s.modelindex3 = 0;	// remove linked ctf flag
//ZOID

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		//ClientObituary (self, inflictor, attacker);
		//TossClientWeapon (self);
		//if (deathmatch->value && !self->client->showscores)
			//Cmd_Help_f (self);		// show scores
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;

	// clear inventory
	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

	if (self->health < -40)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);
//ZOID
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = 0;
//ZOID
		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i+1)%3;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			}
			else switch (i)
			{
			case 0:
				self->s.frame = FRAME_death101-1;
				self->client->anim_end = FRAME_death106;
				break;
			case 1:
				self->s.frame = FRAME_death201-1;
				self->client->anim_end = FRAME_death206;
				break;
			case 2:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			}
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;

	gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
	gitem_t		*item;

	memset (&client->pers, 0, sizeof(client->pers));

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;
	client->pers.lastweapon = item;

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;

	client->pers.connected = true;
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE|FL_NOTARGET|FL_POWER_ARMOR));
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->flags |= ent->client->pers.savedFlags;
	if (coop->value)
		ent->client->resp.score = ent->client->pers.score;
}


//======================================================================


void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < -40)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
	}
}

void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;


	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);

	gi.unlinkentity (body);
	body->s = ent->s;
	body->s.number = body - g_edicts;

	body->svflags = ent->svflags;
	VectorCopy (ent->mins, body->mins);
	VectorCopy (ent->maxs, body->maxs);
	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);
	body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	gi.linkentity (body);
}


void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
		if (self->movetype != MOVETYPE_NOCLIP)
			CopyToBodyQue (self);
		self->svflags &= ~SVF_NOCLIENT;
		//PutClientInServer (self);

		// add a teleportation effect
		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		self->client->respawn_time = level.time;

		return;
	}

	// restart the entire server
	gi.AddCommandString ("menu_loadgame\n");
}

//==============================================================






/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
    // Jump
    Jump::Spawn::ClientOnEnterMap(ent);
    // Jump
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s;
	int		playernum;

	// Check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		gi.cprintf(ent, PRINT_HIGH, "[Server] Invalid user config.\n");
		gi.AddCommandString(va("kick %d", ent - g_edicts - 1));
		return;
	}

	// Check for valid username
	std::string username = Info_ValueForKey(userinfo, "name");
	if (username.empty())
	{
		gi.cprintf(ent, PRINT_HIGH, "[Server] Invalid username.\n");
		gi.AddCommandString(va("kick %d", ent - g_edicts - 1));
		return;
	}

	// If the username has changed, update it
	std::string oldUsername = ent->client->pers.netname;
	if (!Jump::StringCompareInsensitive(username, oldUsername))
	{
		const int maxUsernameSize = sizeof(ent->client->pers.netname) - 1;
		strncpy(ent->client->pers.netname, username.c_str(), maxUsernameSize);
		ent->client->pers.netname[maxUsernameSize] = 0;
		Jump::Logger::Activity(va("User changed name from \"%s\" to \"%s\"", oldUsername.c_str(), username.c_str()));
		Jump::UpdateUserId(ent);
	}

    // Jump
    // We never let the user change their skin.
    // We set the value here so that it stays correct across connects, team changes, etc.
    //Jump::AssignTeamSkin(ent);

	std::string ip = Info_ValueForKey(userinfo, "ip");
	if (!ip.empty())
	{
		ent->client->jumpdata->ip = ip.substr(0, ip.find_first_of(':')); // remove port from ip address
	}

    // Jump

//	// set skin
//	s = Info_ValueForKey (userinfo, "skin");
//
	playernum = ent-g_edicts-1;
//
//	// combine name and skin into a configstring
////ZOID
//	if (ctf->value)
//		CTFAssignSkin(ent, s);
//	else
////ZOID
//		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

//ZOID
	// set player name field (used in id_state view)
	gi.configstring (CS_GENERAL+playernum, ent->client->pers.netname);
//ZOID

	// fov
    ent->client->ps.fov = atoi(Info_ValueForKey(ent->client->pers.userinfo, "fov"));
    if (ent->client->ps.fov < 1)
    {
        ent->client->ps.fov = 90;
    }
    else if (ent->client->ps.fov > 160)
    {
        ent->client->ps.fov = 160;
    }

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

    // fps
    s = Info_ValueForKey(userinfo, "cl_maxfps");
    if (strlen(s))
    {
        ent->client->jumpdata->fps = atoi(s);
        if (ent->client->jumpdata->fps < 20)
        {
            gi.cprintf(ent, PRINT_HIGH, "[Server] You have been kicked for lowering cl_maxfps below 20\n");
            gi.AddCommandString(va("kick %d", ent - g_edicts - 1));
        }
        else if (ent->client->jumpdata->fps > 120)
        {
            gi.cprintf(ent, PRINT_HIGH, "[Server] You have been kicked for raising cl_maxfps above 120\n");
			gi.AddCommandString(va("kick %d", ent - g_edicts - 1));
        }
    }

	// async
	s = Info_ValueForKey(userinfo, "cl_async");
	if (strlen(s))
	{
		int asyncValue = atoi(s);
		if (asyncValue == 0)
		{
			ent->client->jumpdata->async = Jump::AsyncEnum::Zero;
		}
		else if (asyncValue == 1)
		{
			ent->client->jumpdata->async = Jump::AsyncEnum::One;
		}
		else
		{
			ent->client->jumpdata->async = Jump::AsyncEnum::Unknown;
		}
	}

	// save off the userinfo in case we want to check something later
	strncpy(ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect(edict_t* ent, char* userinfo)
{
	char* value = NULL;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey(userinfo, "ip");
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}

	// check for a server password
	value = Info_ValueForKey(userinfo, "password");
	if (*password->string && strcmp(password->string, "none") && 
		strcmp(password->string, value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
		return false;
	}

	// check for invalid username
	value = Info_ValueForKey(userinfo, "name");
	if (!Jump::IsUsernameValid(value))
	{
		Info_SetValueForKey(userinfo, "rejmsg", "Invalid username. Cannot use special characters: <>:\"/\\|?*");
		return false;
	}

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// initialize the client
	memset(ent->client, 0, sizeof(*ent->client));

	ent->client->resp.enterframe = level.framenum;	// TODO: we might not need this for anything
	ent->client->pers.connected = true;
	ent->inuse = true;

	// Jump
	// TODO inline
	Jump::JumpClientConnect(ent);
	// Jump

	// TODO!!!
	// ClientBegin() is called right after this and basically resets the ent

	//ClientUserinfoChanged(ent, userinfo);

	Jump::Logger::Info(va("User \"%s\" connected", ent->client->pers.netname));
	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");

	// Jump
	Jump::JumpClientDisconnect(ent);
	// Jump
}


//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;

	level.current_entity = ent;
	client = ent->client;

    // Only start the timer if the player has moved
    if (ent->client->jumpdata->timer_paused)
    {
        if (abs(ucmd->forwardmove) > 0 || abs(ucmd->upmove) > 0 || abs(ucmd->sidemove) > 0)
        {
			ent->client->jumpdata->timer_pmove_msec = 0;
            ent->client->jumpdata->timer_begin = Sys_Milliseconds();
            ent->client->jumpdata->timer_paused = false;
        }
    }
	else
	{
		ent->client->jumpdata->timer_pmove_msec += ucmd->msec;
	}

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

	pm_passent = ent;

//ZOID
	if (ent->client->chase_target) {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		return;
	}
//ZOID

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != 255)
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	// Jump
	int baseGravity = sv_gravity->value;
	if (Jump::MSets::IsGravitySet())
	{
		baseGravity = Jump::MSets::GetGravity();
	}

	if (ent->gravity != 1.0f)
	{
		// If something has overriden the client gravity multiplier, apply the multiplier against
		// the default base value of 800 because that is how most people think to use the override multiplier.
		client->ps.pmove.gravity = 800 * ent->gravity;
	}
	else
	{
		// Nothing has overriden the ent gravity value, so use the world gravity value.
		client->ps.pmove.gravity = baseGravity;
	}
	// Jump

	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
	{
		pm.snapinitial = true;
//		gi.dprintf ("pmove changed!\n");
	}

	pm.cmd = *ucmd;

	pm.trace = PM_trace;	// adds default parms
	pm.pointcontents = gi.pointcontents;

	// perform a pmove
	gi.Pmove (&pm);

	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	for (i=0 ; i<3 ; i++)
	{
		ent->s.origin[i] = pm.s.origin[i]*0.125;
		ent->velocity[i] = pm.s.velocity[i]*0.125;
	}

	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}

	ent->viewheight = pm.viewheight;
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	if (pm.groundentity)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	if (ent->deadflag)
	{
		client->ps.viewangles[ROLL] = 40;
		client->ps.viewangles[PITCH] = -15;
		client->ps.viewangles[YAW] = client->killer_yaw;
	}
	else
	{
		VectorCopy (pm.viewangles, client->v_angle);
		VectorCopy (pm.viewangles, client->ps.viewangles);
	}

//ZOID
	//if (client->ctf_grapple)
	//	CTFGrapplePull((edict_t*)client->ctf_grapple);
//ZOID

	gi.linkentity (ent);

	if (ent->movetype != MOVETYPE_NOCLIP)
		G_TouchTriggers (ent);

	// touch other objects
	for (i=0 ; i<pm.numtouch ; i++)
	{
		other = pm.touchents[i];
		for (j=0 ; j<i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated
		if (!other->touch)
			continue;
		other->touch (other, ent, NULL, NULL);
	}


	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
		)
	{
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

//ZOID
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}

	if (client->menudirty && client->menutime <= level.time) {
		PMenu_Do_Update(ent);
		gi.unicast (ent, true);
		client->menutime = level.time;
		client->menudirty = false;
	}
//ZOID

    // Key states are not actually sent to the server frames.  They are used to compute a move state,
    // and that value is sent to the server.  So we need to figure out key states here.
    // Note: although the client can hold down forward/back or left/right at the same time,
    // only one of the keys is actually registered.
	uint8_t old_key_states = ent->client->jumpdata->key_states;
    ent->client->jumpdata->key_states = 0;

    if (ucmd->forwardmove > 10)
    {
        ent->client->jumpdata->key_states |= static_cast<uint8_t>(Jump::KeyStateEnum::Forward);
    }
    else if (ucmd->forwardmove < -10)
    {
        ent->client->jumpdata->key_states |= static_cast<uint8_t>(Jump::KeyStateEnum::Back);
    }

    if (ucmd->sidemove > 10)
    {
        ent->client->jumpdata->key_states |= static_cast<uint8_t>(Jump::KeyStateEnum::Right);
    }
    else if (ucmd->sidemove < -10)
    {
        ent->client->jumpdata->key_states |= static_cast<uint8_t>(Jump::KeyStateEnum::Left);
    }

    if (ucmd->upmove > 10)
    {
        ent->client->jumpdata->key_states |= static_cast<uint8_t>(Jump::KeyStateEnum::Jump);
    }
    else if (ucmd->upmove < -10)
    {
        ent->client->jumpdata->key_states |= static_cast<uint8_t>(Jump::KeyStateEnum::Crouch);
    }

	if (ucmd->buttons & BUTTON_ATTACK)
	{
		ent->client->jumpdata->key_states |= static_cast<uint8_t>(Jump::KeyStateEnum::Attack);
	}

	Jump::AdjustReplaySpeed(ent, old_key_states, ent->client->jumpdata->key_states);

	Jump::UpdatePlayerIdleState(ent, ucmd);

	// Apply a health regen rate of 10/frame (equal to 100/s)
	if (ent->health > 0)
	{
		ent->health += 10;
		if (ent->health > ent->max_health)
		{
			ent->health = ent->max_health;
		}
	}

    // Jump
    gi.unicast(ent, true);
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;

	if (level.intermissiontime)
		return;

	client = ent->client;

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
		)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			// in deathmatch, only wait for attack button
			if (deathmatch->value)
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;

			if ( ( client->latched_buttons & buttonMask ) ||
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ))
			{
				//respawn(ent);
				Jump::Spawn::PlayerRespawn(ent);
				client->latched_buttons = 0;
			}
		}
		return;
	}

	// add player trail so monsters can follow
	if (!deathmatch->value)
		if (!visible (ent, PlayerTrail_LastSpot() ) )
			PlayerTrail_Add (ent->s.old_origin);

	client->latched_buttons = 0;

    // Jump
	Jump::AdvanceSpectatingReplayFrame(ent);
	Jump::AdvanceRacingSpark(ent);
	#if 0
    if (ent->client->update_replay)
    {
        VectorCopy(level.replay_fastest_buffer.frames[ent->client->replay_current_frame].pos, ent->s.origin);
        VectorCopy(level.replay_fastest_buffer.frames[ent->client->replay_current_frame].angles, ent->client->v_angle);
        VectorCopy(level.replay_fastest_buffer.frames[ent->client->replay_current_frame].angles, ent->client->ps.viewangles);
        // TODO keys, fps

        // Since we only send a position update every server frame (10 fps),
        // the client needs to smoothen the movement between the two frames.
        // Setting these flags will do this.
        ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
        ent->client->ps.pmove.pm_type = PM_FREEZE;

        for (int i = 0; i < 3; i++)
        {
            ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
        }

        ent->client->replay_current_frame++;
        if (ent->client->replay_current_frame >= level.replay_fastest_buffer.next_frame_index)
        {
            ent->client->ps.pmove.pm_flags = 0;
            ent->client->ps.pmove.pm_type = PM_SPECTATOR;
            ent->client->update_replay = false;
        }
    }
	#endif
    // Jump
}

// Forces a client to execute a command as if they typed it into the console themselves
void StuffCmd(edict_t* user, const char* cmd)
{
	gi.WriteByte(11);
	gi.WriteString(const_cast<char*>(cmd));
	gi.unicast(user, true);
}
