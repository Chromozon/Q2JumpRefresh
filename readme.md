## Q2 Jump Refresh
A reimplementation of Quake 2 jump mod.  The code currently compiles with C++17.

## Goals
- Redesign the code from the ground up so it's easy to understand and less bug-prone when adding new features.
- Global time database.
- Ability to host multiple servers on one machine.
- Multiple servers on one machine sharing the same data?  No need for this, use the global time database instead.

## Design
- Server data is stored in `C:/Quake2/jump/<port>/`.  Each server stores a local copy of all the maptimes.
- Logs are stored in `C:/Quake2/jump/<port>/logs/`.
  - Log file `server.txt` stores errors, warnings, info, and debug messages.
  - Log file `activity.txt` stores general server activity (players joining and leaving, players talking, etc.).
  - Log file `completions.txt` stores a record of every time a client completes a map.
- Map completion times and demos are stored in `C:/Quake2/jump/<port>/scores`.
  - There is a subfolder for each map, ex: `/scores/slipmap33/`, `/scores/ddrace/`, etc.
  - Inside each map subfolder are `<username>.time` files which store completion time, date, and number of completions.
  - Each map subfolder also contains a `<username>.demo` file.  This is an arbitrary length replay.

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

- Replay ghost

- "Store" should save equipped weapons and ammo, "recall" should reset the inventory back to the stored point

- [Big Project] Design and implement the global highscores table

## Notes
See `g_main.c GetGameAPI()` for the main logic entry points.
- `InitGame`
- `ShutdownGame`
- `SpawnEntities`
- `WriteGame`
- `ReadGame`
- `WriteLevel`
- `ReadLevel`
- `ClientThink`
- `ClientConnect`
- `ClientUserinfoChanged`
- `ClientDisconnect`
- `ClientBegin`
- `ClientCommand`
- `G_RunFrame`
- `ServerCommand`

- `ClientConnect` is called when the client first joins the server, and `ClientDisconnect` is called when leaving.
- When a client first joins a map and on map changes, `ClientBegin` will be called.
- `gi.cprintf(client, PRINT_HIGH, ...)` sends messages to the client's console.

### How to update the HUD
The layout of the HUD is sent to all clients with `gi.configstring(CS_STATUSBAR, "<HUD layout string>"`.  The HUD layout string is the same for all clients.
Values specific to each client (such as jump timer, current fps, and keys hit) are transferred by STAT_ values.
The HUD layout string has STAT_ placeholders for client specific values that change often.  These can be changed each server frame by setting them in `ent->client->ps.stats[STAT_]`.
Note that there are only 32 max STAT_ values available for use (0 to 31, but always treat 0 as an empty value).

STAT_ values can be used directly, or they can be used to lookup a display value in the configstring table.  For example, if the HUD layout string has `stat_string 26`,
the client will find the value of `ps.stats[26]` and will use that value to look up the corresponding string in the configstrings.
Let's say the server has set `gi.configstring(1075, "HELLO")`.  If the value `ps.stats[26]` is 1075, then the string "HELLO" will be displayed in the HUD layout at that token.

If you don't want to use the configstring lookup and want to use the value directly, don't put `stat_string` in the HUD layout string.  Instead, use the STAT_ index directly.
For example, if the HUD layout string has `num 4 17`, it will find the value of `ps.stats[17]` and display it formatted to 4 digits.

You can also use STAT_ value configstring lookup to display icons in the HUD.
For example, set `ent->client->ps.stats[25] = gi.imageindex("somepcx")`.  In the HUD layout string, use `pic 25` to do the pcx lookup for the value in `ps.stats[25]`.

An easy way to toggle the visibility of HUD layout tokens is to use the if syntax.  `if 18 stat_string 18` will only display the lookup value of `ps.stats[18]` if it is nonzero.
