// Dumping ground for code that's not needed, but could be useful for reference

// Useful CTF stuff to keep around
#if 0

void CTFSay_Team(edict_t* who, char* msg)
{
	char outmsg[256];
	char buf[256];
	int i;
	char* p;
	edict_t* cl_ent;

	if (CheckFlood(who))
		return;

	outmsg[0] = 0;

	if (*msg == '\"') {
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 2; msg++) {
		if (*msg == '%') {
			switch (*++msg) {
			case 'l':
			case 'L':
				CTFSay_Team_Location(who, buf);
				if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
					strcpy(p, buf);
					p += strlen(buf);
				}
				break;
			case 'a':
			case 'A':
				CTFSay_Team_Armor(who, buf);
				if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
					strcpy(p, buf);
					p += strlen(buf);
				}
				break;
			case 'h':
			case 'H':
				CTFSay_Team_Health(who, buf);
				if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
					strcpy(p, buf);
					p += strlen(buf);
				}
				break;
			case 't':
			case 'T':
				CTFSay_Team_Tech(who, buf);
				if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
					strcpy(p, buf);
					p += strlen(buf);
				}
				break;
			case 'w':
			case 'W':
				CTFSay_Team_Weapon(who, buf);
				if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
					strcpy(p, buf);
					p += strlen(buf);
				}
				break;

			case 'n':
			case 'N':
				CTFSay_Team_Sight(who, buf);
				if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
					strcpy(p, buf);
					p += strlen(buf);
				}
				break;

			default:
				*p++ = *msg;
			}
		}
		else
			*p++ = *msg;
	}
	*p = 0;

	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.ctf_team == who->client->resp.ctf_team)
			gi.cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n",
				who->client->pers.netname, outmsg);
	}
}

/*-----------------------------------------------------------------------*/
/*QUAKED misc_ctf_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2
The origin is the bottom of the banner.
The banner is 248 tall.
*/
static void misc_ctf_banner_think(edict_t* ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_ctf_banner(edict_t* ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex("models/ctf/banner/tris.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity(ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_ctf_small_banner (1 .5 0) (-4 -32 0) (4 32 124) TEAM2
The origin is the bottom of the banner.
The banner is 124 tall.
*/
void SP_misc_ctf_small_banner(edict_t* ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex("models/ctf/banner/small.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity(ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

static void old_teleporter_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
	edict_t* dest;
	int			i;
	vec3_t		forward;

	if (!other->client)
		return;
	dest = G_Find(NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf("Couldn't find destination\n");
		return;
	}

	//ZOID
	CTFPlayerResetGrapple(other);
	//ZOID

		// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity(other);

	VectorCopy(dest->s.origin, other->s.origin);
	VectorCopy(dest->s.origin, other->s.old_origin);
	//	other->s.origin[2] += 10;

		// clear the velocity and hold them in place briefly
	VectorClear(other->velocity);
	other->client->ps.pmove.pm_time = 160 >> 3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->enemy->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i = 0; i < 3; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	other->s.angles[PITCH] = 0;
	other->s.angles[YAW] = dest->s.angles[YAW];
	other->s.angles[ROLL] = 0;
	VectorCopy(dest->s.angles, other->client->ps.viewangles);
	VectorCopy(dest->s.angles, other->client->v_angle);

	// give a little forward velocity
	AngleVectors(other->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 200, other->velocity);

	// kill anything at the destination
	if (!KillBox(other))
	{
	}

	gi.linkentity(other);
}

/*QUAKED trigger_teleport (0.5 0.5 0.5) ?
Players touching this will be teleported
*/
void SP_trigger_teleport(edict_t* ent)
{
	edict_t* s;
	int i;

	if (!ent->target)
	{
		gi.dprintf("teleporter without a target.\n");
		G_FreeEdict(ent);
		return;
	}

	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->touch = old_teleporter_touch;
	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	// noise maker and splash effect dude
	s = G_Spawn();
	ent->enemy = s;
	for (i = 0; i < 3; i++)
		s->s.origin[i] = ent->mins[i] + (ent->maxs[i] - ent->mins[i]) / 2;
	s->s.sound = gi.soundindex("world/hum1.wav");
	gi.linkentity(s);

}

/*QUAKED info_teleport_destination (0.5 0.5 0.5) (-16 -16 -24) (16 16 32)
Point trigger_teleports at these.
*/
void SP_info_teleport_destination(edict_t* ent)
{
	ent->s.origin[2] += 16;
}

#endif


#if 0

void ClientBegin(edict_t* ent)
{
	// Jump
	// We always want to use our overridden begin logic
	Jump::ClientBeginJump(ent);
	return;
	// Jump

	int		i;

	ent->client = game.clients + (ent - g_edicts - 1);

	if (deathmatch->value)
	{
		ClientBeginDeathmatch(ent);
		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i = 0; i < 3; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict(ent);
		ent->classname = "player";
		InitClientResp(ent->client);
		PutClientInServer(ent);
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission(ent);
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
			gi.WriteByte(svc_muzzleflash);
			gi.WriteShort(ent - g_edicts);
			gi.WriteByte(MZ_LOGIN);
			gi.multicast(ent->s.origin, MULTICAST_PVS);

			gi.bprintf(PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
		}
	}

	// make sure all view stuff is valid
	ClientEndServerFrame(ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch(edict_t* ent)
{
	G_InitEdict(ent);

	InitClientResp(ent->client);

	// locate ent at a spawn point
	PutClientInServer(ent);

	if (level.intermissiontime)
	{
		MoveClientToIntermission(ent);
	}
	else
	{
		// send effect
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ_LOGIN);
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}

	gi.bprintf(PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

	// make sure all view stuff is valid
	ClientEndServerFrame(ent);
}

edict_t* SelectCoopSpawnPoint(edict_t* ent)
{
	int		index;
	edict_t* spot = NULL;
	char* target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if (Q_stricmp(game.spawnpoint, target) == 0)
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}


/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot(edict_t* spot)
{
	edict_t* player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract(spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength(v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t* SelectRandomDeathmatchSpawnPoint(void)
{
	edict_t* spot, * spot1, * spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find(spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while (selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t* SelectFarthestDeathmatchSpawnPoint(void)
{
	edict_t* bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t* spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot(spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t* SelectDeathmatchSpawnPoint(void)
{
	if ((int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint();
	else
		return SelectRandomDeathmatchSpawnPoint();
}

/*
================
SelectCTFSpawnPoint

go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t* SelectCTFSpawnPoint(edict_t* ent)
{
	edict_t* spot, * spot1, * spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char* cname;

	if (ent->client->resp.ctf_state)
		if ((int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return NULL;
	//return SelectFarthestDeathmatchSpawnPoint ();
		else
			return NULL;
	//return SelectRandomDeathmatchSpawnPoint ();

	ent->client->resp.ctf_state++;

	switch (ent->client->resp.ctf_team) {
	case CTF_TEAM1:
		cname = "info_player_team1";
		break;
	case CTF_TEAM2:
		cname = "info_player_team2";
		break;
	default:
		return NULL;
		//return SelectRandomDeathmatchSpawnPoint();
	}

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find(spot, FOFS(classname), cname)) != NULL)
	{
		count++;
		//range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;
	//return SelectRandomDeathmatchSpawnPoint();

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find(spot, FOFS(classname), cname);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while (selection--);

	return spot;
}

void ClientObituary(edict_t* self, edict_t* inflictor, edict_t* attacker)
{
	int			mod;
	char* message;
	char* message2;
	qboolean	ff;


	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			message = "cratered";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		}
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			default:
				if (IsFemale(self))
					message = "killed herself";
				else
					message = "killed himself";
				break;
			}
		}
		if (message)
		{
			gi.bprintf(PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
				self->client->resp.score--;
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				break;
			case MOD_SHOTGUN:
				message = "was gunned down by";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				break;
			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG";
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade";
				break;
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
				//ZOID
			case MOD_GRAPPLE:
				message = "was caught by";
				message2 = "'s grapple";
				break;
				//ZOID

			}
			if (message)
			{
				gi.bprintf(PRINT_MEDIUM, "%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
				{
					if (ff)
						attacker->client->resp.score--;
					else
						attacker->client->resp.score++;
				}
				return;
			}
		}
	}

	gi.bprintf(PRINT_MEDIUM, "%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
		self->client->resp.score--;
}

void TossClientWeapon(edict_t* self)
{
	gitem_t* item;
	edict_t* drop;
	qboolean	quad;
	float		spread;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (!self->client->pers.inventory[self->client->ammo_index])
		item = NULL;
	if (item && (strcmp(item->pickup_name, "Blaster") == 0))
		item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item(self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}

	if (quad)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item(self, FindItemByClassname("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
}

qboolean IsFemale(edict_t* ent)
{
	char* info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey(ent->client->pers.userinfo, "skin");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules(void)
{
	int			i;
	gclient_t* cl;

	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;

	//ZOID
		//if (ctf->value && CTFCheckRules()) {
		//	EndDMLevel ();
		//	return;
		//}
		//if (CTFInMatch())
		//	return; // no checking in match mode
	//ZOID

	if (timelimit->value)
	{
		if (level.time >= timelimit->value * 60)
		{
			gi.bprintf(PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel();
			return;
		}
	}

	if (fraglimit->value)
		for (i = 0; i < maxclients->value; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i + 1].inuse)
				continue;

			if (cl->resp.score >= fraglimit->value)
			{
				gi.bprintf(PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel();
				return;
			}
		}
}

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer(edict_t* ent)
{
	vec3_t	mins = { -16, -16, -24 };
	vec3_t	maxs = { 16, 16, 32 };
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t* client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint(ent, spawn_origin, spawn_angles);

	index = ent - g_edicts - 1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy(userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant(client);
		ClientUserinfoChanged(ent, userinfo);
	}
	else if (coop->value)
	{
		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy(userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < MAX_ITEMS; n++)
		{
			if (itemlist[n].flags & IT_KEY)
				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
		}
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged(ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
	}
	else
	{
		memset(&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	memset(client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;

	// copy some data from the client to the entity
	FetchClientEntData(ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;

	VectorCopy(mins, ent->mins);
	VectorCopy(maxs, ent->maxs);
	VectorClear(ent->velocity);

	// clear playerstate values
	memset(&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0] * 8;
	client->ps.pmove.origin[1] = spawn_origin[1] * 8;
	client->ps.pmove.origin[2] = spawn_origin[2] * 8;
	//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	//ZOID

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;

	ent->s.frame = 0;
	VectorCopy(spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy(ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i = 0; i < 3; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy(ent->s.angles, client->ps.viewangles);
	VectorCopy(ent->s.angles, client->v_angle);

	// Jump note: KillBox does the telefrag on spawn, so we don't need this in our code!
	if (!KillBox(ent))
	{	// could't spawn in?
	}

	gi.linkentity(ent);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon(ent);
}


#endif