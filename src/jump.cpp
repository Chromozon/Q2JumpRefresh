#include "jump.h"

namespace Jump
{
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

    char* TeamNameStr(team_t team)
    {
        switch (team) {
        case TEAM_EASY:
            return "Easy";
        case TEAM_HARD:
            return "Hard";
        default:
            return "Unknown";
        }
    }
}