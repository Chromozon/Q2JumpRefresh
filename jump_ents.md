# Jump Entities

## Player Spawn Point
It is *strongly* recommended to use `info_player_start` as the player spawn point instead of `info_player_deathmatch`.  The deathmatch spawn shows up as a spawn pad which is very clunky and ugly.

If you want to show a particular view of your map during the short map change intermission, add an `info_player_intermission`.

## trigger_finish
Completes the map when the user hits this.  Allows the mapper to have a larger finish area without having to drop down a bunch of weapons.  Also allows the finish area to be placed where weapons cannot be placed- such as on the ceiling.

If you want your map to be playable in very old versions of Jump, make sure to always put a weapon_railgun down in addition to this ent.

Note: This cannot be used to give a player a weapon.  Use trigger_weapon instead.

## weapon_finish
Behaves the same as trigger_finish.  Do not use for new maps; use trigger_finish instead.

## trigger_weapon
Gives the player a specific weapon.  Follows the standard q2 weapon numbers.

Blaster = 1, shotgun = 2, supershotgun = 3, machinegun = 4, chaingun = 5, grenade launcher = 6, rocket launcher = 7, hyperblaster = 8, railgun = 9, bfg = 0, hand grenades = 11.

## trigger_hurt
Deals damage to the player equal to the "dmg" field value.

In Jump, if "dmg = 1", it removes all weapons from the player's inventory.


## Checkpoints
All `key_` ents can be used as checkpoints except for `key_commander_head`.  The commander head is the same model as our store/recall marker.

In older versions of Jump, some of these ents acted as map finish.  However, they only act as checkpoints now.

- key_airstrike_target
- key_blue_key
- key_red_key
- key_data_cd
- key_data_spinner
- key_pass
- key_power_cube
- key_pyramid

## Items
Items have no use in Jump and won't do anything when touched by the player.  Historically, there have been some items that acted as map finish.  However, picking up these items causes them to disappear, so other players were unable to use them to finish a map.  Over time, server admins have fixed these map issues by replacing the items with railguns or removing them.

### Items that don't do anything
- item_adrenaline
- item_ancient_head
- item_armor_shard
- item_armor_body
- item_armor_combat
- item_armor_jacket
- item_bandolier
- item_breather *Acts as a map finish in old versions*
- item_enviro *Acts as a map finish in old versions*
- item_health_small
- item_health_small
- item_health_large
- item_health_mega
- item_invulnerability *Acts as a map finish in old versions*
- item_pack
- item_power_screen *Acts as a map finish in old versions*
- item_power_shield *Acts as a map finish in old versions*
- item_quad *Acts as a map finish in old versions*
- item_silencer *Acts as a map finish in old versions*


## Ammo
In Jump, weapons always have unlimited ammo.
- ammo_bullets
- ammo_cells
- ammo_grenades
- ammo_rockets
- ammo_shells
- ammo_slugs