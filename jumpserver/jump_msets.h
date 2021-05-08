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
    static bool GetWeapons();
    static bool GetRocket();
    static bool GetHyperBlaster();
    static bool GetBfg();
    static int GetCheckpointTotal();
    static int GetGravity();
    static bool GetDamage();

    static bool IsGravitySet();

    static void LoadMSets();

private:

    static void ResetMSets();

    static bool _fastTele;
    static bool _weapons;
    static bool _rocket;
    static bool _hyperblaster;
    static bool _bfg;
    static int _checkpointTotal;
    static int _gravity;
    static std::string _editedBy;
    static bool _damage;

    static bool _isGravitySet;
};

}