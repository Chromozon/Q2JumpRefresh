#pragma once

#include "g_local.h"
#include <map>
#include <string>

namespace Jump
{

class MSets
{
public:

    static bool GetFastTele();
    static bool GetGrenadeLauncher();
    static bool GetRocket();
    static bool GetHyperBlaster();
    static bool GetBfg();
    static int GetCheckpointTotal();
    static int GetGravity();
    static bool GetDamage();

    static bool IsGravitySet();

    static void ApplyAllMSets();
    static void LoadMapperMSets(const char* args);
    static void LoadServerMSets();

    static void PrintMSetList(edict_t* ent);
    static bool SetMSetSafe_Cmd(edict_t* ent, const std::string mset_name, const std::string& value);

private:
    enum MSetType_t {
        MSETTYPE_INVALID = 0,

        MSETTYPE_INTEGER,
        MSETTYPE_REAL,
        MSETTYPE_BOOLEAN,
        MSETTYPE_STRING, // Not implemented.
    };

    typedef void (*MSetChangeCbPost)();

    struct MSetData {
        volatile void* value; // Pointer to the data.
        
        const MSetType_t type; // Variable type.
        
        const std::string name; // Name of the mset.

        const MSetChangeCbPost change_cb_post; // Callback AFTER mset was changed.
    };

    static MSetData* GetMSetByName(const std::string& mset_name);
    static std::string GetMSetValue(const MSetData* mset);

    static void ResetMSets();
    static void ApplyMSets(const std::map<std::string, std::string>& msets);

    static bool SetMSetSafe(MSetData* mset, const std::string value);

    static bool _fastTele;
    static bool _grenadelauncher;
    static bool _rocket;
    static bool _hyperblaster;
    static bool _bfg;
    static int _checkpointTotal;
    static int _gravity;
    static bool _damage;

    static bool _isGravitySet;

    static std::map<std::string, std::string> _mapperMSets; // <mset name, value>
    static std::map<std::string, std::string> _serverMSets; // <mset name, value>

    static MSetData MSets::_msets[];


    static void GravityChanged();
};

}