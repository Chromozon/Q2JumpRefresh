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
    static void LoadAdminMSets();

private:

    static void ResetMSets();
    static void ApplyMSets(const std::map<std::string, std::string>& msets);

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
    static std::map<std::string, std::string> _adminMSets; // <mset name, value>
};

}