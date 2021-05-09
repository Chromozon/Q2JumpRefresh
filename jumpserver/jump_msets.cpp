#include "jump_msets.h"
#include "jump_utils.h"
#include <filesystem>
#include <fstream>
#include "jump_logger.h"

namespace Jump
{

/// <summary>
/// Variables
/// </summary>
bool MSets::_fastTele = false;
bool MSets::_weapons = false;
bool MSets::_rocket = false;
bool MSets::_hyperblaster = false;
bool MSets::_bfg = false;
int MSets::_checkpointTotal = 0;
int MSets::_gravity = 800;
std::string MSets::_editedBy = "";
bool MSets::_damage = true;
bool MSets::_grenadelauncher = false;

bool MSets::_isGravitySet = false;

/// <summary>
/// True if teleports should not hold the user in place for a brief period of time.
/// </summary>
/// <returns></returns>
bool MSets::GetFastTele()
{
    return _fastTele;
}

/// <summary>
/// True to allow the player to fire all weapons.  TODO: probably should remove this, don't think it's ever needed
/// TODO: this actually allows both RL and GL to be used as weapons (see map bja2).  It's easier to just change the
/// map cfg file for this instead of implementing weapons mset.
/// </summary>
/// <returns></returns>
bool MSets::GetWeapons()
{
    return _weapons;
}
/// <summary>
/// True if rocket launcher AND grenade launcher can be used as a weapon by the player.
/// TODO
/// </summary>
/// <returns></returns>
bool MSets::GetRocket()
{
    return _rocket;;
}

/// <summary>
/// True if hyperblaster can be used as a weapon by the player.
/// </summary>
/// <returns></returns>
bool MSets::GetHyperBlaster()
{
    return _hyperblaster;
}

/// <summary>
/// True if BFG can be used as a weapon by the player.
/// </summary>
/// <returns></returns>
bool MSets::GetBfg()
{
    return _bfg;;
}

/// <summary>
/// Total number of checkpoints needed to complete a map.
/// </summary>
/// <returns></returns>
int MSets::GetCheckpointTotal()
{
    return _checkpointTotal;
}

/// <summary>
/// Gets the overriden gravity value for the map, or returns the default value of 800.
/// </summary>
/// <returns></returns>
int MSets::GetGravity()
{
    return _gravity;
}

/// <summary>
/// If true, the player can take damage.  If false, always sets the T_Damage() value to 0.
/// </summary>
/// <returns></returns>
bool MSets::GetDamage()
{
    return _damage;
}

/// <summary>
/// If true, allows the player to pick up the grenade launcher and fire it.
/// </summary>
/// <returns></returns>
bool MSets::GetGrenadeLauncher()
{
    return _grenadelauncher;
}

/// <summary>
/// Returns true if there is a gravity mset value.
/// </summary>
/// <returns></returns>
bool MSets::IsGravitySet()
{
    return _isGravitySet;
}

/// <summary>
/// Resets all msets and loads the ones for the current map.
/// </summary>
void MSets::LoadMSets()
{
    ResetMSets();

    std::string filename = GetModDir() + "/ent/" + level.mapname + ".cfg";
    std::ifstream file(filename.c_str());
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            std::vector<std::string> pieces = SplitString(line, ' ');
            if (pieces.size() != 2)
            {
                Logger::Warning(va("Invalid mset for map %s: %s", level.mapname, line.c_str()));
                continue;
            }
            std::string lineName = pieces[0];
            std::string lineValue = TrimQuotes(pieces[1]);

            if (StringCompareInsensitive(lineName, "checkpoint_total"))
            {
                int num = 0;
                if (StringToIntMaybe(lineValue, num))
                {
                    _checkpointTotal = num;
                }
                else
                {
                    Logger::Warning(va("Invalid checkpoint_total mset for map %s: %s", level.mapname, line.c_str()));
                }
            }
            else if (StringCompareInsensitive(lineName, "edited_by"))
            {
                _editedBy = lineValue;
            }
            else if (StringCompareInsensitive(lineName, "rocket"))
            {
                if (lineValue != "0")
                {
                    _rocket = true;
                }
            }
            else if (StringCompareInsensitive(lineName, "hyperblaster"))
            {
                if (lineValue != "0")
                {
                    _hyperblaster = true;
                }
            }
            else if (StringCompareInsensitive(lineName, "bfg"))
            {
                if (lineValue != "0")
                {
                    _bfg = true;
                }
            }
            else if (StringCompareInsensitive(lineName, "gravity"))
            {
                int num = 0;
                if (StringToIntMaybe(lineValue, num))
                {
                    _gravity = num;
                    _isGravitySet = true;
                }
                else
                {
                    Logger::Warning(va("Invalid gravity mset for map %s: %s", level.mapname, line.c_str()));
                }
            }
            else if (StringCompareInsensitive(lineName, "weapons"))
            {
                if (lineValue != "0")
                {
                    _weapons = true;
                }
            }
            else if (StringCompareInsensitive(lineName, "fasttele"))
            {
                if (lineValue != "0")
                {
                    _fastTele = true;
                }
            }
            else if (StringCompareInsensitive(lineName, "damage"))
            {
                if (lineValue == "0")
                {
                    _damage = false;
                }
            }
            else if (StringCompareInsensitive(lineName, "grenadelauncher"))
            {
                if (lineValue != "0")
                {
                    _grenadelauncher = true;
                }
            }
        }
    }
}

/// <summary>
/// Sets all the mset variables to default values.
/// </summary>
void MSets::ResetMSets()
{
    _fastTele = false;
    _weapons = false;
    _rocket = false;
    _hyperblaster = false;
    _bfg = false;
    _checkpointTotal = 0;
    _gravity = 800;
    _editedBy = "";
    _damage = true;
    _grenadelauncher = false;

    _isGravitySet = false;
}

}