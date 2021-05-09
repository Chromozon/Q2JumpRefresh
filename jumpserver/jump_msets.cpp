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
bool MSets::_grenadelauncher = false;
bool MSets::_rocket = false;
bool MSets::_hyperblaster = false;
bool MSets::_bfg = false;
int MSets::_checkpointTotal = 0;
int MSets::_gravity = 800;
bool MSets::_damage = true;

bool MSets::_isGravitySet = false;

std::map<std::string, std::string> MSets::_mapperMSets;
std::map<std::string, std::string> MSets::_serverMSets;

/// <summary>
/// True if teleports should not hold the user in place for a brief period of time.
/// </summary>
/// <returns></returns>
bool MSets::GetFastTele()
{
    return _fastTele;
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
/// True if rocket launcher can be used as a weapon by the player.
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
    return _bfg;
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
void MSets::ApplyAllMSets()
{
    ResetMSets();
    ApplyMSets(_mapperMSets);
    ApplyMSets(_serverMSets);
}

/// <summary>
/// Load the msets that the mapper has set in the worldspawn "mset" field.
/// Example: "mset" "checkpoint_total 3 rocket 1 bfg 1"
/// </summary>
/// <param name="args"></param>
void MSets::LoadMapperMSets(const char* args)
{
    _mapperMSets.clear();
    if (args == nullptr)
    {
        return;
    }
    auto pieces = SplitString(args, ' ');
    if (pieces.empty() || ((pieces.size() % 2) != 0))
    {
        Logger::Warning(va("Invalid mset worldspawn value for map %s: %s", level.mapname, args));
        return;
    }
    for (size_t i = 0; i < pieces.size(); i += 2)
    {
        _mapperMSets.insert({ pieces[i], pieces[i + 1] });
    }
}

/// <summary>
/// Load the msets from the jump/ent/mapname.cfg file
/// </summary>
void MSets::LoadServerMSets()
{
    _serverMSets.clear();
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
            std::string key = pieces[0];
            std::string value = TrimQuotes(pieces[1]);
            _serverMSets.insert({ key, value });
        }
    }
}

/// <summary>
/// Sets all the mset variables to default values.
/// </summary>
void MSets::ResetMSets()
{
    _fastTele = false;
    _rocket = false;
    _hyperblaster = false;
    _bfg = false;
    _checkpointTotal = 0;
    _gravity = 800;
    _damage = true;
    _grenadelauncher = false;

    _isGravitySet = false;
}

/// <summary>
/// Parses the mset key-value pairs and sets the appropriate variables.
/// </summary>
/// <param name="msets"></param>
void Jump::MSets::ApplyMSets(const std::map<std::string, std::string>& msets)
{
    for (const auto& keyValue : msets)
    {
        if (StringCompareInsensitive(keyValue.first, "checkpoint_total"))
        {
            int num = 0;
            if (StringToIntMaybe(keyValue.second, num))
            {
                _checkpointTotal = num;
            }
            else
            {
                Logger::Warning(va("Invalid checkpoint_total mset for map %s: %s",
                    level.mapname, keyValue.second.c_str()));
            }
        }
        else if (StringCompareInsensitive(keyValue.first, "rocket"))
        {
            if (keyValue.second != "0")
            {
                _rocket = true;
            }
        }
        else if (StringCompareInsensitive(keyValue.first, "hyperblaster"))
        {
            if (keyValue.second != "0")
            {
                _hyperblaster = true;
            }
        }
        else if (StringCompareInsensitive(keyValue.first, "bfg"))
        {
            if (keyValue.second != "0")
            {
                _bfg = true;
            }
        }
        else if (StringCompareInsensitive(keyValue.first, "gravity"))
        {
            int num = 0;
            if (StringToIntMaybe(keyValue.second, num))
            {
                _gravity = num;
                _isGravitySet = true;
            }
            else
            {
                Logger::Warning(va("Invalid gravity mset for map %s: %s", level.mapname, keyValue.second.c_str()));
            }
        }
        else if (StringCompareInsensitive(keyValue.first, "fasttele"))
        {
            if (keyValue.second != "0")
            {
                _fastTele = true;
            }
        }
        else if (StringCompareInsensitive(keyValue.first, "damage"))
        {
            if (keyValue.second == "0")
            {
                _damage = false;
            }
        }
        else if (StringCompareInsensitive(keyValue.first, "grenadelauncher"))
        {
            if (keyValue.second != "0")
            {
                _grenadelauncher = true;
            }
        }
    }
}

}