#include "jump_admin.h"
#include "jump_local_database.h"
#include "jump_logger.h"


namespace Jump
{
void AdminSystem::Init()
{

}

void AdminSystem::PlayerConnected(edict_t* player)
{
    player->client->jumppers->admin.authenticated = false;
    player->client->jumppers->admin.admin_flags = AdminFlagEnum::ADMIN_FLAG_NONE;
}

void AdminSystem::PlayerDisconnected(edict_t* player)
{
    player->client->jumppers->admin.authenticated = false;
    player->client->jumppers->admin.admin_flags = AdminFlagEnum::ADMIN_FLAG_NONE;
}

void AdminSystem::Login(edict_t* player, const std::string& password)
{
    const std::string username = player->client->pers.netname;

    AdminFlagEnum flags = AdminFlagEnum::ADMIN_FLAG_NONE;
    auto authenticated = LocalDatabase::AdminAuthenticate(username, password, &flags);

    if (authenticated)
    {
        Logger::Activity(va("Player %s logged in as admin (flags: %i)", username.c_str(), static_cast<int>(flags)));
    }
    else
    {
        Logger::Activity(va("Player %s attempted to log in as admin!", username.c_str()));
    }
    

    player->client->jumppers->admin.authenticated = authenticated;
    player->client->jumppers->admin.admin_flags = flags;

    if (authenticated)
    {
        gi.bprintf(PRINT_HIGH, "%s is now logged in as an admin!\n", username.c_str());
    }
}

void AdminSystem::Logout(edict_t* player)
{
    const std::string username = player->client->pers.netname;

    if (!player->client->jumppers->admin.authenticated)
    {
        Logger::Activity(va("Player %s attempted to log out as admin!", username.c_str()));
        return;
    }

    auto prev_levels = player->client->jumppers->admin.admin_flags;

    player->client->jumppers->admin.authenticated = false;
    player->client->jumppers->admin.admin_flags = AdminFlagEnum::ADMIN_FLAG_NONE;

    gi.bprintf(PRINT_HIGH, "%s is no longer logged in as an admin!\n", username.c_str());

    Logger::Activity(va("Player %s logged out as admin (flags: %i)!", username.c_str(), prev_levels));
}

void AdminSystem::AddAdmin(edict_t* player, const std::string& username, const std::string& password)
{
    if (!CanPerformAction(player, AdminFlagEnum::ADMIN_FLAG_ADMINS))
    {
        gi.cprintf(player, PRINT_HIGH, "You cannot perform this command!\n");
        return;
    }

    LocalDatabase::AddAdmin(username, password);
}

void AdminSystem::RemoveAdmin(edict_t* player, const std::string& username)
{
    if (!CanPerformAction(player, AdminFlagEnum::ADMIN_FLAG_ADMINS))
    {
        gi.cprintf(player, PRINT_HIGH, "You cannot perform this command!\n");
        return;
    }

    LocalDatabase::RemoveAdmin(username);
}

bool AdminSystem::CanPerformAction(edict_t* player, AdminFlagEnum required_flags)
{
    if (!player->client->jumppers->admin.authenticated)
    {
        return false;
    }

    return CanPerformAction(required_flags, player->client->jumppers->admin.admin_flags);
}

bool AdminSystem::CanPerformAction(AdminFlagEnum required_flags, AdminFlagEnum comp_flags)
{
    return ((int)required_flags & (int)comp_flags) == (int)required_flags;
}

}
