#include "jump.h"

namespace Jump
{
    static const char* SexTeamEasy = "female";
    static const char* SexTeamHard = "male";
    static const char* SkinTeamEasy = "ctf_r";
    static const char* SkinTeamHard = "ctf_b";

    static const int MaxMenuWidth = 32; // characters
    static const int MapNameLine = 2;
    static const int JoinEasyLine = 5;
    static const int NumPlayersEasyLine = 6;
    static const int NumPlayersHardLine = 8;
    static const int ChaseCamLine = 9;

    static pmenu_t Menu_Join[] = {
        { "*Quake II Jump",               PMENU_ALIGN_CENTER, NULL },
        { NULL,                           PMENU_ALIGN_CENTER, NULL },
        { NULL /* mapname */,             PMENU_ALIGN_CENTER, NULL },
        { NULL,                           PMENU_ALIGN_CENTER, NULL },
        { NULL,                           PMENU_ALIGN_CENTER, NULL },
        { "Join Easy Team",               PMENU_ALIGN_LEFT,   JoinTeamEasy },
        { NULL /* number of players */,   PMENU_ALIGN_LEFT,   NULL },
        { "Join Hard Team",               PMENU_ALIGN_LEFT,   JoinTeamHard },
        { NULL /* number of players */,   PMENU_ALIGN_LEFT,   NULL },
        { NULL /* chase camera */,        PMENU_ALIGN_LEFT,   JoinChaseCam },
        { NULL,                           PMENU_ALIGN_LEFT,   NULL }, // TODO help commands menu
        { NULL,                           PMENU_ALIGN_LEFT,   NULL },
        { "Highlight your choice and",    PMENU_ALIGN_LEFT,   NULL },
        { "press ENTER.",                 PMENU_ALIGN_LEFT,   NULL },
        { "v" JUMP_STRING_VERSION,        PMENU_ALIGN_RIGHT,  NULL },
    };


    // ent must be a client
    // No other menus can be open when this function is called
    void OpenMenu_Join(edict_t* ent)
    {
        char mapName[MaxMenuWidth] = { 0 };
        char playersEasy[MaxMenuWidth] = { 0 };
        char playersHard[MaxMenuWidth] = { 0 };
        char chaseCam[MaxMenuWidth] = { 0 };

        strncpy(mapName, level.mapname, MaxMenuWidth - 1);
        sprintf(playersEasy, "  (%d players)", CountPlayersEasy());
        sprintf(playersHard, "  (%d players)", CountPlayersHard());

        Menu_Join[MapNameLine].text = mapName;
        Menu_Join[NumPlayersEasyLine].text = playersEasy;
        Menu_Join[NumPlayersHardLine].text = playersHard;
        if (ent->client->chase_target)
        {
            Menu_Join[ChaseCamLine].text = "Leave Chase Camera";
        }
        else
        {
            Menu_Join[ChaseCamLine].text = "Chase Camera";
        }

        int cursor = JoinEasyLine;
        PMenu_Open(ent, Menu_Join, cursor, sizeof(Menu_Join) / sizeof(pmenu_t), NULL);
    }

    // TODO
    int CountPlayersEasy()
    {
        return -1;
    }

    // TODO
    int CountPlayersHard()
    {
        return -1;
    }

    void JoinTeam(edict_t* ent, team_t team)
    {
        PMenu_Close(ent);
        
        ent->svflags &= ~SVF_NOCLIENT;
        ent->client->resp.jump_team = team;

        AssignTeamSkin(ent);

        PutClientInServer(ent);
        // add a teleportation effect
        ent->s.event = EV_PLAYER_TELEPORT;
        // hold in place briefly
        ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        ent->client->ps.pmove.pm_time = 14;
    }

    void JoinTeamEasy(edict_t* ent, pmenuhnd_t* hnd)
    {
        JoinTeam(ent, TEAM_EASY);
    }

    void JoinTeamHard(edict_t* ent, pmenuhnd_t* hnd)
    {
        JoinTeam(ent, TEAM_HARD);
    }

    void JoinChaseCam(edict_t* ent, pmenuhnd_t* hnd) {}


    bool JumpClientCommand(edict_t* ent)
    {
        char* cmd = gi.argv(0);

        if (Q_stricmp(cmd, "inven") == 0)
        {
            Cmd_Jump_Inven(ent);
            return true;
        }
        else if (Q_stricmp(cmd, "noclip") == 0)
        {
            Cmd_Jump_Noclip(ent);
            return true;
        }
        else
        {
            return false;
        }
    }

    void Cmd_Jump_Inven(edict_t* ent)
    {
        if (ent->client->menu) {
            PMenu_Close(ent);
            ent->client->update_chase = true;
        }
        else
        {
            OpenMenu_Join(ent);
        }
    }

    void Cmd_Jump_Noclip(edict_t* ent)
    {
        if (ent->client->resp.jump_team == TEAM_EASY)
        {
            if (ent->movetype == MOVETYPE_NOCLIP)
            {
                ent->movetype = MOVETYPE_WALK;
                gi.cprintf(ent, PRINT_HIGH, "noclip OFF\n");
            }
            else
            {
                ent->movetype = MOVETYPE_NOCLIP;
                gi.cprintf(ent, PRINT_HIGH, "noclip ON\n");
            }
        }
    }

    void AssignTeamSkin(edict_t* ent)
    {
        int playernum = ent - g_edicts - 1;

        switch (ent->client->resp.jump_team)
        {
        case TEAM_EASY:
            gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s/%s", ent->client->pers.netname, SexTeamEasy, SkinTeamEasy));
            break;
        case TEAM_HARD:
            gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s/%s", ent->client->pers.netname, SexTeamHard, SkinTeamHard));
            break;
        default:
            gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\female/invis", ent->client->pers.netname));
            break;
        }
    }


    StoreBuffer::StoreBuffer() : numStores(0), nextIndex(0), stores()
    {
    }

    void StoreBuffer::PushStore(const store_data_t& data)
    {
        stores[nextIndex] = data;
        nextIndex = (nextIndex + 1) % MAX_STORES;
        if (numStores < MAX_STORES)
        {
            numStores++;
        }
    }

    store_data_t StoreBuffer::GetStore(int prevNum)
    {
        int index = 0;
        if (prevNum > numStores)
        {
            // If a user asks for a prev that is larger than
            // what we have stored, we return the oldest data.
            index = (nextIndex - numStores) % MAX_STORES;
        }
        else if (prevNum > 0)
        {
            index = (nextIndex - prevNum) % MAX_STORES;
        }
        return stores[index];
    }

    bool StoreBuffer::HasStore()
    {
        return numStores > 0;
    }
}