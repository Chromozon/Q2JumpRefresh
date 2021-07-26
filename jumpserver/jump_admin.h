#pragma once

#include "g_local.h"

namespace Jump
{
    class AdminSystem
    {
    public:
        static void Init();

        static void PlayerConnected(edict_t* player);
        static void PlayerDisconnected(edict_t* player);

        static void Login(edict_t* player, const std::string& password);
        static void Logout(edict_t* player);

        static void AddAdmin(edict_t* player, const std::string& username, const std::string& password);
        static void RemoveAdmin(edict_t* player, const std::string& username);

        static bool CanPerformAction(edict_t* player, AdminFlagEnum required_flags);
        static bool CanPerformAction(AdminFlagEnum required_flags, AdminFlagEnum comp_flags);

    private:

    };
}
