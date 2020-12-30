## Q2 Jump Refresh
A reimplementation of Quake 2 jump mod.

## Goals
- Redesign the code from the ground up so it's easy to understand and less bug-prone when adding new features.
- Global time database.
- Ability to host multiple servers on one machine.
- Multiple servers on one machine sharing the same data?  No need for this, use the global time database instead.

## Design
Each server will store its data locally.  In the future, it will pull in the global highscores data.
Server data is stored in `C:/Quake2/jump/<port>/`.  This allows you to run multiple servers on the same machine.

## TODO
- Remove the unused CTF stuff
- There is this "tourney" mode which seems to be unused.
- The function "SelectSpawnPoint" can be cleaned up.  Currently it looks for the red/blue team spawn points, then deathmatch, then info_play_start.
- The "apply_time" function has a funky way of determining first place; could use cleanup overall.
- The kill and recall functions share some logic and have CTF overtime logic cluttering them up.

- There are a bunch of variables that have to deal with map completion time, but many seem unused:
	float			item_timer; // map completion time
	float			item_timer_penalty;
	int			item_timer_penalty_delay;
	float			stored_item_timer;
	qboolean	item_timer_allow;

- Replays need to record async 0/1, what weapon is equipped, and when a weapon is shot
    - When replaying a demo with weapons, need to store the source client and only show the rockets/grenade ents to that source client
    - Replay should store client used (r1q2, q2pro-speed, etc.)
    - Replay should store replay version in case there are future changes

- Replays should be arbitrary length.  There's no need to cap replays at 1000 seconds.

- "Store" should save equipped weapons and ammo, "recall" should reset the inventory back to the stored point
