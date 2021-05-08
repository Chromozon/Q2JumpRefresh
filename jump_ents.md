# Jump Entities

## trigger_finish
Completes the map when the user hits this.  Allows the mapper to have a larger finish area without having to drop down a bunch of weapons.  Also allows the finish area to be placed where weapons cannot be placed- such as on the ceiling.

If you want your map to be playable in very old versions of Jump, make sure to always put a weapon_railgun down in addition to this ent.

Note: This cannot be used to give a player a weapon.  Use trigger_weapon instead.

## weapon_finish
Behaves the same as trigger_finish.  Do not use for new maps; use trigger_finish instead.

## trigger_weapon
Gives the player a specific weapon.  Follows the standard q2 weapon numbers.
Blaster = 1, shotgun = 2, supershotgun = 3, machinegun = 4, chaingun = 5, grenade launcher = 6, rocket launcher = 7, hyperblaster = 8, railgun = 9, bfg = 0.

## trigger_hurt
Deals damage to the player equal to the "dmg" field value.

In Jump, if "dmg = 1", it causes the player to lose all weapons in their inventory.