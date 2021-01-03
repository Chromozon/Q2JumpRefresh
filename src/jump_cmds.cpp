
#include "jump_cmds.h"
#include "jump.h"
#include "jump_types.h"
#include <unordered_map>
#include <string>
#include "jump_scores.h"
#include "jump_menu.h"
#include "jump_utils.h"
#include "jump_logger.h"

namespace Jump
{
    typedef void(*CmdFunction)(edict_t* ent);

    static std::unordered_map<std::string, CmdFunction> CmdTable = {
        { "inven", Cmd_Jump_Inven },
        { "noclip", Cmd_Jump_Noclip },
        { "test", Cmd_Jump_Test },
        { "kill", Cmd_Jump_Kill},
        { "recall", Cmd_Jump_Recall },
        { "store", Cmd_Jump_Store },
        { "reset", Cmd_Jump_Reset },
        { "replay", Cmd_Jump_Replay },
        { "maptimes", Cmd_Jump_Maptimes },

        // TODO
        { "showtimes", Cmd_Jump_Void },
        { "nominate", Cmd_Jump_Void },
        { "votetime", Cmd_Jump_Void },
        { "timevote", Cmd_Jump_Void },
        { "mapvote", Cmd_Jump_Void },
        { "yes", Cmd_Jump_Void },
        { "no", Cmd_Jump_Void },
        { "maplist", Cmd_Jump_Void },
        { "playertimes", Cmd_Jump_Void },
        { "playermaps", Cmd_Jump_Void },
        { "playerscores", Cmd_Jump_Void },
        { "globaltimes", Cmd_Jump_Void },
        { "globalmaps", Cmd_Jump_Void },
        { "globalscores", Cmd_Jump_Void },
        { "jumpers", Cmd_Jump_Void },
        { "playerlist", Cmd_Jump_Void },
        { "mapsdone", Cmd_Jump_Void },
        { "mapsleft", Cmd_Jump_Void },
        { "!stats", Cmd_Jump_Void },
        { "compare", Cmd_Jump_Void },
        { "1st", Cmd_Jump_Void },
        { "!seen", Cmd_Jump_Void },
        { "!help", Cmd_Jump_Void },
        { "boot", Cmd_Jump_Void },
        { "silence", Cmd_Jump_Void },
        { "+hook", Cmd_Jump_Void },
        { "race", Cmd_Jump_Void }, // race n, race now, race delay n, race off

        // TODO: all of the admin commands- remtime, addmap, etc.
    };

    bool HandleJumpCommand(edict_t* client)
    {
        std::string cmd = gi.argv(0);
        const auto cmdPair = CmdTable.find(cmd);
        if (cmdPair != CmdTable.end())
        {
            CmdFunction cmdFunction = cmdPair->second;
            cmdFunction(client);
            return true;
        }
        return false;
    }

    // A function used to test stuff for development
    void Cmd_Jump_Test(edict_t* ent)
    {
        auto x = ent->client->pers.weapon;
        auto y = level.time;

        BestTimesScoreboardMessage(ent);

        //LoadAllLocalMaptimes();
        //LoadTimesForMap(level.mapname);

        //if (ent->client->menu)
        //{
        //    PMenu_Close(ent);
        //}
        //ent->client->showscores = true;

        //char string[1400] = { 0 };

        //Com_sprintf(string, sizeof(string),
        //    "xv -16 yv 0 string2 \"Ping Pos Player          Best Comp Maps     %%\" ");

        //gi.WriteByte(svc_layout);
        //gi.WriteString(string);
        //gi.unicast(ent, true);
    }

    void Cmd_Jump_Inven(edict_t* ent)
    {
        if (ent->client->menu) {
            PMenu_Close(ent);
            ent->client->update_chase = true; // TODO what is this for?
        }
        else
        {
            OpenMenu_Join(ent);
        }
    }

    void Cmd_Jump_Noclip(edict_t* ent)
    {
        if (ent->client->jumpdata->team == TEAM_EASY)
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

    void Cmd_Jump_Kill(edict_t* ent)
    {
        SpawnForJumping(ent);
    }

    void Cmd_Jump_Recall(edict_t* ent)
    {
        if (ent->client->jumpdata->team == TEAM_EASY)
        {
            if (ent->client->jumpdata->stores.HasStore())
            {
                int num = 1;
                if (gi.argc() >= 2 && StringToIntMaybe(gi.argv(1), num))
                {
                    if (num <= 0)
                    {
                        num = 1;
                    }
                }
                store_data_t data = ent->client->jumpdata->stores.GetStore(num);
                SpawnAtStorePosition(ent, data);
            }
            else
            {
                SpawnForJumping(ent);
            }
        }
        else
        {
            SpawnForJumping(ent);
        }
    }

    void Cmd_Jump_Store(edict_t* ent)
    {
        // Save all of the state
        store_data_t data = { 0 };
        data.time_interval = Sys_Milliseconds() - ent->client->jumpdata->timer_begin;
        VectorCopy(ent->s.origin, data.pos);
        VectorCopy(ent->s.angles, data.angles);
        data.angles[ROLL] = 0; // fixes the problem where the view is tilted after recall
        ent->client->jumpdata->stores.PushStore(data);

        // Place an entity at the position
        if (ent->client->jumpdata->store_ent != NULL)
        {
            G_FreeEdict(ent->client->jumpdata->store_ent);
            ent->client->jumpdata->store_ent = NULL;
        }
        ent->client->jumpdata->store_ent = G_Spawn();
        VectorCopy(data.pos, ent->client->jumpdata->store_ent->s.origin);
        VectorCopy(data.pos, ent->client->jumpdata->store_ent->s.old_origin);
        ent->client->jumpdata->store_ent->s.old_origin[2] -= 10;
        ent->client->jumpdata->store_ent->s.origin[2] -= 10;
        ent->client->jumpdata->store_ent->svflags = SVF_PROJECTILE;
        VectorCopy(data.angles, ent->client->jumpdata->store_ent->s.angles);
        ent->client->jumpdata->store_ent->movetype = MOVETYPE_NONE;
        ent->client->jumpdata->store_ent->clipmask = MASK_PLAYERSOLID;
        ent->client->jumpdata->store_ent->solid = SOLID_NOT;
        ent->client->jumpdata->store_ent->s.renderfx = RF_TRANSLUCENT;
        VectorClear(ent->client->jumpdata->store_ent->mins);
        VectorClear(ent->client->jumpdata->store_ent->maxs);
        ent->client->jumpdata->store_ent->s.modelindex = gi.modelindex(STORE_MODEL);
        ent->client->jumpdata->store_ent->dmg = 0;
        ent->client->jumpdata->store_ent->classname = "jumpdata->store_ent";
        gi.linkentity(ent->client->jumpdata->store_ent);
    }

    void Cmd_Jump_Reset(edict_t* ent)
    {
        ent->client->jumpdata->stores.Reset();
        if (ent->client->jumpdata->store_ent != NULL)
        {
            G_FreeEdict(ent->client->jumpdata->store_ent);
            ent->client->jumpdata->store_ent = NULL;
        }
    }

    // TODO: support replay self, replay n, replay, replay <username>
    void Cmd_Jump_Replay(edict_t* ent)
    {
        if (gi.argc() == 1)
        {
            // replay now
        }
        else
        {
            std::string param = gi.argv(1);
            if (param == "self")
            {
                LoadReplayFromFile(level.mapname, ent->client->pers.netname, ent->client->jumpdata->replay_spectating);
            }
            else if (param == "now")
            {
                // replay now
            }
            else
            {
                int num = 0;
                if (StringToIntMaybe(param, num) && num >= 1 && num <= 15)
                {
                    // replay n (1-15)
                }
                else
                {
                    // replay <username>
                }
            }
        }

        LoadReplayFromFile(level.mapname, ent->client->pers.netname, ent->client->jumpdata->replay_spectating);

        // Move client to a spectator
        InitAsSpectator(ent);

        // Set to replay state
        ent->client->jumpdata->replay_spectating_framenum = 0;
        ent->client->jumpdata->update_replay_spectating = true;
    }

    void Cmd_Jump_Void(edict_t * ent)
    {
        // Empty
    }

    void Cmd_Jump_Maptimes(edict_t* player)
    {
        std::string mapname = level.mapname;
        if (gi.argc() >= 2)
        {
            mapname = gi.argv(1);
        }

        std::vector<user_time_record> highscores;
        int completions = 0;
        if (!GetHighscoresForMap(mapname, highscores, completions))
        {
            gi.cprintf(player, PRINT_HIGH, "Invalid map.\n");
            return;
        }
        if (highscores.size() == 0)
        {
            gi.cprintf(player, PRINT_HIGH, "No times for %s\n", mapname.c_str());
            return;
        }

        gi.cprintf(player, PRINT_HIGH, "--------------------------------------------------------\n");
        gi.cprintf(player, PRINT_HIGH, "Best Times for %s\n", mapname.c_str());

        std::string header = "No. Name             Date                           Time";
        header = GetGreenConsoleText(header);
        gi.cprintf(player, PRINT_HIGH, "%s\n", header.c_str());

        int64_t best_time = highscores[0].time_ms;

        for (size_t i = 0; i < highscores.size(); ++i)
        {
            std::string username = RemoveFileExtension(RemovePathFromFilename(highscores[i].filepath));
            std::string date = highscores[i].date.substr(0, highscores[i].date.find_first_of(' '));
            std::string time = GetCompletionTimeDisplayString(highscores[i].time_ms);

            int64_t time_diff = highscores[i].time_ms - best_time;
            std::string time_diff_str = "0.000";
            if (time_diff > 0)
            {
                time_diff_str = GetCompletionTimeDisplayString(time_diff);
                time_diff_str.insert(0, "-");
            }

            gi.cprintf(player, PRINT_HIGH, "%-3d %-16s %s %12s %11s\n",
                i + 1, username.c_str(), date.c_str(), time_diff_str.c_str(), time.c_str());
        }

        bool completed = HasUserCompletedMap(mapname, player->client->pers.netname);
        if (completed)
        {
            gi.cprintf(player, PRINT_HIGH, "You have completed this map\n");
        }
        else
        {
            gi.cprintf(player, PRINT_HIGH, "You have NOT completed this map\n");
        }
        gi.cprintf(player, PRINT_HIGH, "--------------------------------------------------------\n");
    }

} // namespace Jump