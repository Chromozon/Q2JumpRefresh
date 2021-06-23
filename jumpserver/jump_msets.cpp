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

// List of all the msets and their data.
Jump::MSets::MSetData Jump::MSets::_msets[] = {
    { &MSets::_fastTele, MSets::MSETTYPE_BOOLEAN, "fasttele", nullptr },
    { &MSets::_grenadelauncher, MSets::MSETTYPE_BOOLEAN, "grenadelauncher", nullptr },
    { &MSets::_rocket, MSets::MSETTYPE_BOOLEAN, "rocket", nullptr },
    { &MSets::_hyperblaster, MSets::MSETTYPE_BOOLEAN, "hyperblaster", nullptr },
    { &MSets::_bfg, MSets::MSETTYPE_BOOLEAN, "bfg", nullptr },
    { &MSets::_checkpointTotal, MSets::MSETTYPE_INTEGER, "checkpoints", nullptr },
    { &MSets::_gravity, MSets::MSETTYPE_INTEGER, "gravity", GravityChanged },
    { &MSets::_damage, MSets::MSETTYPE_BOOLEAN, "damage", nullptr },
};


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
/// Get mset data by name.
/// </summary>
/// <param name="mset_name">Name of the mset.</param>
/// <returns>Returns the pointer to the mset data if one is found. Nullptr otherwise.</returns>
MSets::MSetData* MSets::GetMSetByName(const std::string& mset_name)
{
    for (auto& mset : _msets)
    {
        if (StringCompareInsensitive(mset_name, mset.name))
        {
            return &mset;
        }
    }
    
    return nullptr;
}

/// <summary>
/// Get the value of an mset as a string.
/// </summary>
/// <param name="mset">Pointer to the mset.</param>
/// <returns>Value of the mset as a string.</returns>
std::string MSets::GetMSetValue(const MSetData* mset)
{
    assert(mset != nullptr);

    switch (mset->type) {
    case MSETTYPE_INTEGER:
        return va("%i", *(int*)mset->value);
    case MSETTYPE_BOOLEAN:
        return va("%s", *(bool*)mset->value ? "true" : "false");
    case MSETTYPE_REAL:
        return va("%.1f", *(float*)mset->value);
    case MSETTYPE_STRING:
        assert(0); // TODO: Implement me.
    default:
        return "";
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
        auto mset = GetMSetByName(keyValue.first);
        if (mset)
        {
            bool success = SetMSetSafe(mset, keyValue.second);

            if (!success)
            {
                Logger::Warning(va("Invalid mset for map %s! (%s: %s)",
                                   level.mapname,
                                   keyValue.first.c_str(),
                                   keyValue.second.c_str()));
            }
        }
    }
}

/// <summary>
/// Sets an mset and checks for errors.
/// </summary>
/// <param name="mset"></param>
/// <param name="value"></param>
/// <returns>True if value was valid and mset was set. False otherwise.</returns>
bool MSets::SetMSetSafe(MSetData* mset, const std::string value)
{
    int i_value = 0;
    float fl_value = 0;
    //std::string str_value;

    bool i_failed = !StringToIntMaybe(value, i_value);
    bool fl_failed = !StringToFloatMaybe(value, fl_value);

    bool failed = false;

    switch (mset->type)
    {
    case MSETTYPE_BOOLEAN:
        if (i_failed) {
            failed = true;
            break;
        }

        *((bool*)mset->value) = i_value ? true : false;
        break;
    case MSETTYPE_INTEGER:
        if (i_failed) {
            failed = true;
            break;
        }

        *((int*)mset->value) = i_value;
        break;
    case MSETTYPE_REAL:
        if (fl_failed) {
            failed = true;
            break;
        }

        *((float*)mset->value) = fl_value ? true : false;
        break;
    case MSETTYPE_STRING:
        assert(0); // TODO: Implement.
        break;
    default:
        assert(0); // Should not happen.
        break;
    }

    if (!failed)
    {
        // Call the callback function if one exists.
        if (mset->change_cb_post)
        {
            mset->change_cb_post();
        }
    }

    return !failed;
}

/// <summary>
/// Sets an mset value from a command. Useful for debugging.
/// </summary>
/// <param name="ent">Player or the server entity.</param>
/// <param name="mset_name">Name of the mset.</param>
/// <param name="value">New value of the mset.</param>
/// <returns>True if value was valid and mset was set. False otherwise.</returns>
bool MSets::SetMSetSafe_Cmd(edict_t *ent, const std::string mset_name, const std::string& value)
{
    auto mset = GetMSetByName(mset_name);
    if (!mset)
    {
        gi.cprintf(ent, PRINT_HIGH, "Mset '%s' does not exist!\n", mset_name.c_str());
        return false;
    }

    
    bool success = SetMSetSafe(mset, value);
    if (!success)
    {
        gi.cprintf(ent, PRINT_HIGH, "Failed to set mset value to '%s'!\n", value.c_str());
        return false;
    }

    gi.cprintf(ent, PRINT_HIGH, "%s = %s\n", mset_name.c_str(), value.c_str());

    return true;
}

/// <summary>
/// Prints the list of msets to player. Useful for debugging.
/// </summary>
/// <param name="ent">Player entity.</param>
void MSets::PrintMSetList(edict_t* ent)
{
    for (auto& mset : _msets)
    {
        gi.cprintf(ent, PRINT_HIGH, "%s = %s\n", mset.name.c_str(), GetMSetValue(&mset).c_str());
    }
}

/// <summary>
/// Callback for 'gravity'-mset.
/// </summary>
void MSets::GravityChanged()
{
    _isGravitySet = true;
}

}