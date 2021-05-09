# MSets
Msets are variables that only apply to a specific map.  They can be used to add functionality (such as enabling checkpoints) or to override the original map behavior (such as changing the gravity).

MSets are configured through separate files for each map: `jump/ent/<mapname>.cfg`

Here is an example mset file:
```
checkpoint_total "5"
gravity "400"
fasttele "1"
edited_by "slippery"
```

## checkpoint_count
For checkpoints to work on a map, this variable must be set.  This is the number of checkpoints that must be picked up before the player can finish the map.  Valid values are integers from 1 to 255.

## gravity
Overrides the worldspawn gravity set by the mapper.  The q2 default gravity is 800.  Valid values for this are integers between -32768 and 32767.  Practically, gravity values larger than +- 5000 have no additional effect on the physics.

The most common values for this: 800, 1200, 2000, 400

TODO: will a value of 0 work here?

## fasttele
By default, whenever a player uses a teleporter, their movement speed is reset to 0 and they are held in place briefly.  Some maps require smooth teleports that do not reset player movement.

To enable, set value = 1.

## damage
By default, things in the world are allowed to damage the player (trigger_hurt, etc.).  Set this value to 0 to prevent the player from taking any damage.  This mset is mainly used to fix old maps that have buggy or annoying death planes.

To disable damage, set value = 0.

## rocket
Allows a player to pick up and shoot rocket launchers instead of them acting as map finish.

To enable, set value = 1.

## hyperblaster
Allows a player to pick up and shoot hyperblasters instead of them acting as map finish.

To enable, set value = 1.

## bfg
Allows a player to pick up and shoot BFGs instead of them acting as map finish.

To enable, set value = 1.

## grenadelauncher
Allows a player to pick up and shoot the grenade launcher instead of it acting as a map finish.

To enable, set value = 1.

## TODO
- `droptofloor`: need to find if any maps actually need this or if it's set randomly
- `rocketjump_fix`: used to make rocket jumps more consistent; need to do a deeper dive into this

## Unused and not implemented
- `addedtimeoverride`: useless
- `allowsrj`: anti super rocket jump; don't know what this is for; seems unused
- `blaster`: allows a player to fire the blaster; not needed for any map
- `cmsg`: hides center_print messages to the player; seems useless
- `droptofloor`
- `ezmode`: useless, mappers just use trigger_teleports now
- `falldamage`: useless, no need to ever enable fall damage
- `fast_firing`: unused
- `fastdoors`: doors are always fast now, don't see a reason to let them be slow
- `ghost`
- `ghost_model`
- `health`: useless, we always have 1000 health
- `lap_total`: yea... no.
- `quad_damage`: unused
- `regen`: useless, regen is always a high enough value
- `singlespawn`: only creates the first `info_player_deathmatch`; does not seem to be used
- `slowdoors`: unused
- `timelimit`: no need to override time, can just vote more
- `weapons`: replaced with grenadelauncher, TODO: add grenadelauncher to pyrrhic and bja2
