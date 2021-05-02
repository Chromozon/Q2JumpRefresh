
#include "jump_cmds.h"
#include "jump.h"
#include "jump_types.h"
#include <unordered_map>
#include <string>
#include "jump_scores.h"
#include "jump_menu.h"
#include "jump_utils.h"
#include "jump_logger.h"
#include "jump_global.h"
#include "jump_voting.h"
#include <algorithm>
#include "rapidjson/document.h"
#include "jump_local_database.h"
#include "jump_spawn.h"

namespace Jump
{
    typedef void (*CmdFunction)(edict_t* ent);

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
        { "score", Cmd_Jump_Score },
        { "help", Cmd_Jump_Score },
        { "score2", Cmd_Jump_Score2 },
        { "help2", Cmd_Jump_Score2 },
        { "playertimes", Cmd_Jump_Playertimes },
        { "playerscores", Cmd_Jump_Playerscores },
        { "playermaps", Cmd_Jump_Playermaps },
        { "!seen", Cmd_Jump_Seen },
        { "playertimesglobal", Cmd_Jump_PlayertimesGlobal },
        { "playerscoresglobal", Cmd_Jump_PlayerscoresGlobal} ,
        { "playermapsglobal", Cmd_Jump_PlayermapsGlobal },
        { "maptimesglobal", Cmd_Jump_MaptimesGlobal },
        { "race", Cmd_Jump_Race },
        { "mapsleft", Cmd_Jump_Mapsleft },
        { "mapsdone", Cmd_Jump_Mapsdone },
        { "jumpers", Cmd_Jump_Jumpers },

        { "votetime", Cmd_Jump_Vote_Time },
        { "timevote", Cmd_Jump_Vote_Time },
        { "nominate", Cmd_Jump_Vote_Nominate },
        { "yes", Cmd_Jump_Vote_CastYes },
        { "no", Cmd_Jump_Vote_CastNo },
        { "mapvote", Cmd_Jump_Vote_ChangeMap },
        { "votemap", Cmd_Jump_Vote_ChangeMap },
        { "silence", Cmd_Jump_Vote_Silence },
        { "votekick", Cmd_Jump_Vote_Kick },
#ifdef _DEBUG
        { "mapend", Cmd_Jump_Vote_MapEndVote },
#endif // _DEBUG

        // TODO
        { "showtimes", Cmd_Jump_Void },
        { "timevote", Cmd_Jump_Void },
        { "maplist", Cmd_Jump_Void },
        { "globaltimes", Cmd_Jump_Void },
        { "globalmaps", Cmd_Jump_Void },
        { "globalscores", Cmd_Jump_Void },
        { "playerlist", Cmd_Jump_Void },
        { "mapsdone", Cmd_Jump_Void },
        { "mapsleft", Cmd_Jump_Void },
        { "!stats", Cmd_Jump_Void },
        { "compare", Cmd_Jump_Void },
        { "1st", Cmd_Jump_Void },
        { "!help", Cmd_Jump_Void },
        { "boot", Cmd_Jump_Void },
        { "+hook", Cmd_Jump_Void },
        { "say_person", Cmd_Jump_Void },
        { "raceglobal", Cmd_Jump_Void }, // race against a global replay

        // TODO: all of the admin commands- remtime, addmap, etc.
    };

    bool HandleJumpCommand(edict_t* client)
    {
        std::string cmd = gi.argv(0);
        cmd = AsciiToLower(cmd);
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

        std::string testDate = GetEuropeanShortDate("2020-12-23");
        //LocalDatabase::AddUser("atestname!!");
        //int size2 = sizeof(replay_frame_t);
        //return;
        //std::vector<replay_frame_t> replay;
        //std::string file = "E:/Downloads/jump/jump/jumpdemo/slipmap23_250.dj3";
        //LocalDatabase::ConvertOldReplay(file, replay);

        //std::string folder = "E:/Downloads/jump/jump/jumpdemo";
        //LocalDatabase::MigrateReplays(folder);
        //return;

        std::vector<std::string> maplist;
        for (auto it = jump_server.maplist.begin(); it != jump_server.maplist.end(); ++it)
        {
            maplist.push_back(*it);
        }

        auto start = std::chrono::high_resolution_clock::now();

        //LocalDatabase::CalculateAllStatistics(maplist);
        LocalScores::CalculateAllStatistics();

        auto end = std::chrono::high_resolution_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        auto ms = durationMs.count();

        gi.cprintf(ent, PRINT_HIGH, "Calculated statistics in %d ms\n", (int)ms);
        Logger::Info(va("All stats ms: %d", ms));
        return;


        //LocalDatabase::MigrateAll();

        typedef struct
        {
            vec3_t		angle;
            vec3_t		origin;
            int			frame;
        } record_data;

        record_data test;
        int size = sizeof(test);

        size = sizeof(replay_frame_t);

        //ConvertOldHighscores();

        //CalculateAllLocalStatistics();

        int x = 5;

        //auto z = timelimit;

        //auto x = ent->client->pers.weapon;
        //auto y = level.time;

        //BestTimesScoreboardMessage(ent);

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
        if (ent->client->jumpdata->team == TeamEnum::Easy)
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
        Spawn::PlayerRespawn(ent);
    }

    void Cmd_Jump_Recall(edict_t* ent)
    {
        int storeNum = 0;
        if (gi.argc() >= 2)
        {
            StringToIntMaybe(gi.argv(1), storeNum);
        }
        Spawn::PlayerRespawn(ent, storeNum);
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

    void Cmd_Jump_Replay(edict_t* ent)
    {
        bool loaded = false;
        int timeMs = 0;
        std::string username;

        if (gi.argc() == 1)
        {
            // replay best time
            loaded = LocalDatabase::GetReplayByPosition(
                level.mapname, 1, ent->client->jumpdata->replay_spectating, timeMs, username);
            ent->client->jumpdata->replay_spectating_hud_string = "1";
        }
        else
        {
            std::string param = gi.argv(1);
            if (param == "self")
            {
                // replay self
                username = ent->client->pers.netname;
                loaded = LocalDatabase::GetReplayByUser(
                    level.mapname, username, ent->client->jumpdata->replay_spectating, timeMs);
                ent->client->jumpdata->replay_spectating_hud_string = "Self";
            }
            else if (param == "now")
            {
                // replay now
                if (!jump_server.replay_now_recording.empty())
                {
                    ent->client->jumpdata->replay_spectating = jump_server.replay_now_recording;
                    timeMs = jump_server.replay_now_time_ms;
                    username = jump_server.replay_now_username;
                    loaded = true;
                    ent->client->jumpdata->replay_spectating_hud_string = "Now";
                }
            }
            else
            {
                int num = 0;
                int completions = LocalScores::GetTotalTimesForMap(level.mapname); // TODO: this uses cached value and won't be accurate for new maps
                if (StringToIntMaybe(param, num) && num >= 1 && num <= completions)
                {
                    // replay n
                    loaded = LocalDatabase::GetReplayByPosition(
                        level.mapname, num, ent->client->jumpdata->replay_spectating, timeMs, username);
                    ent->client->jumpdata->replay_spectating_hud_string = param;
                }
                else
                {
                    // replay <username>
                    username = param;
                    loaded = LocalDatabase::GetReplayByUser(
                        level.mapname, username, ent->client->jumpdata->replay_spectating, timeMs);
                    ent->client->jumpdata->replay_spectating_hud_string = username;
                }
            }
        }

        if (!loaded)
        {
            gi.cprintf(ent, PRINT_HIGH, "No replay exists\n");
            return;
        }
        else
        {
            std::string timeStr = GetCompletionTimeDisplayString(timeMs);
            gi.cprintf(ent, PRINT_HIGH, "Replaying %s who finished in %s seconds.\n", username.c_str(), timeStr.c_str());
        }

        // Move client to a spectator
        if (ent->client->jumpdata->team != TeamEnum::Spectator)
        {
            Spawn::JoinTeamSpectator(ent);
        }

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
        LocalScores::PrintMapTimes(player);
    }

    void Cmd_Jump_Score(edict_t* ent)
    {
        ent->client->showinventory = false;
        ent->client->showhelp = false;
        ent->client->showscores = true;
        if (ent->client->menu)
        {
            PMenu_Close(ent);
        }

        if (ent->client->jumpdata->scores_menu == SCORES_MENU_NONE)
        {
            ActiveClientsScoreboardMessage(ent);
            ent->client->jumpdata->scores_menu = SCORES_MENU_ACTIVEPLAYERS;
        }
        else if (ent->client->jumpdata->scores_menu == SCORES_MENU_ACTIVEPLAYERS)
        {
            ShowBestTimesScoreboard(ent);
            ent->client->jumpdata->scores_menu = SCORES_MENU_HIGHSCORES;
        }
        else
        {
            // Same as g_cmds.c, Cmd_PutAway_f()
            ent->client->showscores = false;
            ent->client->showhelp = false;
            ent->client->showinventory = false;
            if (ent->client->menu)
            {
                PMenu_Close(ent);
            }
            ent->client->update_chase = true;
            ent->client->jumpdata->scores_menu = SCORES_MENU_NONE;
        }
    }

    void Cmd_Jump_Score2(edict_t* ent)
    {
        ent->client->showinventory = false;
        ent->client->showhelp = false;
        ent->client->showscores = true;
        if (ent->client->menu)
        {
            PMenu_Close(ent);
        }

        if (ent->client->jumpdata->scores_menu == SCORES_MENU_NONE)
        {
            ExtendedActiveClientsScoreboardMessage(ent);
            ent->client->jumpdata->scores_menu = SCORES_MENU_ACTIVEPLAYERS;
        }
        else
        {
            // Same as g_cmds.c, Cmd_PutAway_f()
            ent->client->showscores = false;
            ent->client->showhelp = false;
            ent->client->showinventory = false;
            if (ent->client->menu)
            {
                PMenu_Close(ent);
            }
            ent->client->update_chase = true;
            ent->client->jumpdata->scores_menu = SCORES_MENU_NONE;
        }
    }

    void Cmd_Jump_Playertimes(edict_t* ent)
    {
        LocalScores::PrintPlayerTimes(ent);
    }

    void Cmd_Jump_Playerscores(edict_t* ent)
    {
        LocalScores::PrintPlayerScores(ent);
    }

    void Cmd_Jump_Playermaps(edict_t* ent)
    {
        LocalScores::PrintPlayerMaps(ent);
    }

    void Cmd_Jump_Seen(edict_t* ent)
    {
        const int ms_per_second = 1000;
        const int ms_per_minute = ms_per_second * 60;
        const int ms_per_hour = ms_per_minute * 60;
        const int ms_per_day = ms_per_hour * 24;

        int page = 1;
        if (gi.argc() > 1)
        {
            StringToIntMaybe(gi.argv(1), page);
        }
        if (page < 1)
        {
            page = 1;
        }
        size_t index_start = (page - 1) * CONSOLE_HIGHSCORES_COUNT_PER_PAGE;
        if (index_start >= jump_server.last_seen.size())
        {
            gi.cprintf(ent, PRINT_HIGH, "There are no last seen times for this page.\n");
            return;
        }
        size_t index_end = std::min<size_t>(
            jump_server.last_seen.size() - 1,
            (page * CONSOLE_HIGHSCORES_COUNT_PER_PAGE) - 1);

        int64_t current_time = Sys_Milliseconds();

        // Header row
        gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------\n");
        std::string header = GetGreenConsoleText("No. Name            Days Hrs Mins Secs");
        gi.cprintf(ent, PRINT_HIGH, "%s\n", header.c_str());

        for (size_t i = index_start; i <= index_end; ++i)
        {
            const std::string& username = jump_server.last_seen[i].first;
            int64_t seen_time = jump_server.last_seen[i].second;

            int64_t diff = current_time - seen_time;
            if (diff < 0)
            {
                Logger::Warning("Invalid last seen time for user " + username);
                continue;
            }

            int days = diff / ms_per_day;
            diff -= days * ms_per_day;
            int hours = diff / ms_per_hour;
            diff -= hours * ms_per_hour;
            int minutes = diff / ms_per_minute;
            diff -= minutes * ms_per_minute;
            int seconds = diff / ms_per_second;

            gi.cprintf(ent, PRINT_HIGH, "%-3d %-15s %4d %3d %4d %4d\n",
                static_cast<int>(i + 1),
                username.c_str(),
                days,
                hours,
                minutes,
                seconds
            );
        }

        // Footer
        int total_pages = (jump_server.last_seen.size() / CONSOLE_HIGHSCORES_COUNT_PER_PAGE) + 1;
        gi.cprintf(ent, PRINT_HIGH, "Page %d/%d (%d users). Use !seen <page>\n",
            page, total_pages, static_cast<int>(jump_server.last_seen.size()));
        gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------\n");
    }

    void Cmd_Jump_PlayertimesGlobal(edict_t* ent)
    {
        int page = 1;
        if (gi.argc() > 1)
        {
            StringToIntMaybe(gi.argv(1), page);
        }
        if (page < 1)
        {
            page = 1;
        }

        gi.cprintf(ent, PRINT_HIGH, "Retrieving scores from global database. Please wait...\n");
        std::shared_ptr<global_cmd_playertimes> cmd = std::make_shared<global_cmd_playertimes>();
        cmd->user = ent;
        cmd->page = page;
        cmd->count_per_page = 20;
        QueueGlobalDatabaseCmd(cmd);
    }
    
    void Cmd_Jump_Vote_Time(edict_t* ent)
    {
        VoteSystem::AttemptStartVote(ent, VOTETYPE_VOTETIME, gi.args());
    }

    void Cmd_Jump_Vote_Nominate(edict_t* ent)
    {
        VoteSystem::AttemptStartVote(ent, VOTETYPE_NOMINATE, gi.args());
    }

    void Cmd_Jump_Vote_ChangeMap(edict_t* ent)
    {
        VoteSystem::AttemptStartVote(ent, VOTETYPE_MAPCHANGE, gi.args());
    }

    void Cmd_Jump_Vote_Silence(edict_t* ent)
    {
        VoteSystem::AttemptStartVote(ent, VOTETYPE_SILENCE, gi.args());
    }

    void Cmd_Jump_Vote_Kick(edict_t* ent)
    {
        VoteSystem::AttemptStartVote(ent, VOTETYPE_KICK, gi.args());
    }

#ifdef _DEBUG
    void Cmd_Jump_Vote_MapEndVote(edict_t* ent)
    {
        VoteSystem::AttemptStartVote(ent, VOTETYPE_MAPEND, gi.args());
    }
#endif // _DEBUG

    void Cmd_Jump_Vote_CastYes(edict_t* ent)
    {
        VoteSystem::CastVote(ent, VOTEOPTION_YES);
    }

    void Cmd_Jump_Vote_CastNo(edict_t* ent)
    {
        VoteSystem::CastVote(ent, VOTEOPTION_NO);
    }

    void Cmd_Jump_PlayerscoresGlobal(edict_t* ent)
    {
        int page = 1;
        if (gi.argc() > 1)
        {
            StringToIntMaybe(gi.argv(1), page);
        }
        if (page < 1)
        {
            page = 1;
        }

        gi.cprintf(ent, PRINT_HIGH, "Retrieving scores from global database. Please wait...\n");
        std::shared_ptr<global_cmd_playerscores> cmd = std::make_shared<global_cmd_playerscores>();
        cmd->user = ent;
        cmd->page = page;
        cmd->count_per_page = 20;
        QueueGlobalDatabaseCmd(cmd);
    }

    void Cmd_Jump_PlayermapsGlobal(edict_t* ent)
    {
        int page = 1;
        if (gi.argc() > 1)
        {
            StringToIntMaybe(gi.argv(1), page);
        }
        if (page < 1)
        {
            page = 1;
        }

        gi.cprintf(ent, PRINT_HIGH, "Retrieving scores from global database. Please wait...\n");
        std::shared_ptr<global_cmd_playermaps> cmd = std::make_shared<global_cmd_playermaps>();
        cmd->user = ent;
        cmd->page = page;
        cmd->count_per_page = 20;
        QueueGlobalDatabaseCmd(cmd);
    }

    void Cmd_Jump_MaptimesGlobal(edict_t* ent)
    {
        std::string mapname = level.mapname;
        if (gi.argc() >= 2)
        {
            mapname = gi.argv(1);
        }

        int page = 1;
        if (gi.argc() >= 3)
        {
            StringToIntMaybe(gi.argv(2), page);
        }
        if (page < 1)
        {
            page = 1;
        }

        gi.cprintf(ent, PRINT_HIGH, "Retrieving scores from global database. Please wait...\n");
        std::shared_ptr<global_cmd_maptimes> cmd = std::make_shared<global_cmd_maptimes>();
        cmd->user = ent;
        cmd->page = page;
        cmd->count_per_page = 20;
        cmd->mapname = mapname;
        QueueGlobalDatabaseCmd(cmd);
    }

    void Cmd_Jump_Race(edict_t* ent)
    {
        if (gi.argc() == 1)
        {
            // race (best time)
            int timeMs = 0;
            std::string username;
            bool loaded = LocalDatabase::GetReplayByPosition(
                level.mapname, 1, ent->client->jumpdata->racing_frames, timeMs, username);
            if (loaded)
            {
                ent->client->jumpdata->racing = true;
                ent->client->jumpdata->racing_delay_frames = 0;
                ent->client->jumpdata->racing_framenum = 0;
                ent->client->jumpdata->racing_highscore = 1;
                std::string timeStr = GetCompletionTimeDisplayString(timeMs);
                gi.cprintf(ent, PRINT_HIGH, "Racing replay 1: %s (%s seconds)\n", username.c_str(), timeStr.c_str());
                gi.cprintf(ent, PRINT_HIGH, "Other options: race delay <num>, race off, race now, race self, race <demonumber>, race <username>\n");
                return;
            }
            else
            {
                gi.cprintf(ent, PRINT_HIGH, "There is no demo to race.\n");
                return;
            }
        }

        if (gi.argc() == 2)
        {
            std::string arg = gi.argv(1);
            if (StringCompareInsensitive(arg, "now"))
            {
                // race now
                if (!jump_server.replay_now_recording.empty())
                {
                    ent->client->jumpdata->racing = true;
                    ent->client->jumpdata->racing_frames = jump_server.replay_now_recording;
                    ent->client->jumpdata->racing_delay_frames = 0;
                    ent->client->jumpdata->racing_framenum = 0;
                    ent->client->jumpdata->racing_highscore = 0; // TODO special value so we know to update if NOW becomes faster
                    std::string timeStr = GetCompletionTimeDisplayString(jump_server.replay_now_time_ms);
                    gi.cprintf(ent, PRINT_HIGH, "Racing fastest current time: %s (%s seconds)\n", jump_server.replay_now_username.c_str(), timeStr.c_str());
                    return;
                }
                else
                {
                    gi.cprintf(ent, PRINT_HIGH, "There is no demo to race.\n");
                    return;
                }
            }
            else if (StringCompareInsensitive(arg, "off"))
            {
                // race off
                gi.cprintf(ent, PRINT_HIGH, "No longer racing.\n");
                ent->client->jumpdata->racing = false;
                return;
            }
            else if (StringCompareInsensitive(arg, "self"))
            {
                // race self
                int timeMs = 0;
                bool loaded = LocalDatabase::GetReplayByUser(
                    level.mapname, ent->client->pers.netname, ent->client->jumpdata->racing_frames, timeMs);
                if (loaded)
                {
                    ent->client->jumpdata->racing = true;
                    ent->client->jumpdata->racing_delay_frames = 0;
                    ent->client->jumpdata->racing_framenum = 0;
                    ent->client->jumpdata->racing_highscore = 0; // TODO special value to update if we set a better time
                    std::string timeStr = GetCompletionTimeDisplayString(timeMs);
                    gi.cprintf(ent, PRINT_HIGH, "Racing your fastest time (%s seconds)\n", timeStr.c_str());
                    return;
                }
                else
                {
                    gi.cprintf(ent, PRINT_HIGH, "There is no demo to race.\n");
                    return;
                }
            }
            else if (StringCompareInsensitive(arg, "delay"))
            {
                if (!ent->client->jumpdata->racing)
                {
                    gi.cprintf(ent, PRINT_HIGH, "Race delay can only be set after race has been loaded.\n");
                    gi.cprintf(ent, PRINT_HIGH, "Race delay resets to 0 each time a race is loaded.\n");
                    return;
                }
                else
                {
                    float delaySeconds = ent->client->jumpdata->racing_delay_frames * 0.1;
                    gi.cprintf(ent, PRINT_HIGH, "Current race delay is %.1f seconds.\n", delaySeconds);
                    gi.cprintf(ent, PRINT_HIGH, "Use \"race delay <num>\" command to change the delay (value between 0.0 and 10.0 seconds).\n");
                    return;
                }
            }
            else
            {
                int num = 0;
                int completions = LocalScores::GetTotalTimesForMap(level.mapname); // TODO: this uses cached value and won't be accurate for new maps
                if (StringToIntMaybe(arg, num) && num >= 1 && num <= completions)
                {
                    // race n
                    int timeMs = 0;
                    std::string username;
                    bool loaded = LocalDatabase::GetReplayByPosition(
                        level.mapname, num, ent->client->jumpdata->racing_frames, timeMs, username);
                    if (loaded)
                    {
                        ent->client->jumpdata->racing = true;
                        ent->client->jumpdata->racing_delay_frames = 0;
                        ent->client->jumpdata->racing_framenum = 0;
                        ent->client->jumpdata->racing_highscore = num;
                        std::string timeStr = GetCompletionTimeDisplayString(timeMs);
                        gi.cprintf(ent, PRINT_HIGH, "Racing replay %d: %s (%s seconds)\n", num, username.c_str(), timeStr.c_str());
                        return;
                    }
                    else
                    {
                        gi.cprintf(ent, PRINT_HIGH, "There is no demo to race.\n");
                        return;
                    }
                }
                else
                {
                    // race <username>
                    int timeMs = 0;
                    std::string username = arg;
                    bool loaded = LocalDatabase::GetReplayByUser(
                        level.mapname, username, ent->client->jumpdata->racing_frames, timeMs);
                    if (loaded)
                    {
                        ent->client->jumpdata->racing = true;
                        ent->client->jumpdata->racing_delay_frames = 0;
                        ent->client->jumpdata->racing_framenum = 0;
                        ent->client->jumpdata->racing_highscore = 0; // TODO special value so we can update if sets a faster time
                        std::string timeStr = GetCompletionTimeDisplayString(timeMs);
                        gi.cprintf(ent, PRINT_HIGH, "Racing replay: %s (%s seconds)\n", username.c_str(), timeStr.c_str());
                        return;
                    }
                    else
                    {
                        gi.cprintf(ent, PRINT_HIGH, "There is no demo to race.\n");
                        return;
                    }
                }
            }
        }

        if (gi.argc() == 3)
        {
            std::string arg1 = gi.argv(1);
            std::string arg2 = gi.argv(2);
            if (!StringCompareInsensitive(arg1, "delay"))
            {
                gi.cprintf(ent, PRINT_HIGH, "Invalid race option.\n");
                gi.cprintf(ent, PRINT_HIGH, "Race options: race delay <num>, race off, race now, race self, race <demonumber>, race <username>\n");
                return;
            }
            else
            {
                if (!ent->client->jumpdata->racing)
                {
                    gi.cprintf(ent, PRINT_HIGH, "Race delay can only be set after race has been loaded.\n");
                    gi.cprintf(ent, PRINT_HIGH, "Race delay resets to 0 each time a race is loaded.\n");
                    return;
                }

                float delay = 0.0;
                if (StringToFloatMaybe(arg2, delay))
                {
                    if (delay >= 0.0 && delay <= 10.0)
                    {
                        ent->client->jumpdata->racing_delay_frames = delay / FRAMETIME;
                        gi.cprintf(ent, PRINT_HIGH, "Race delay is %.1f seconds.\n", delay);
                    }
                    else
                    {
                        // White "Invalid delay. Must provide number between 0.0 and 10.0 seconds"
                        gi.cprintf(ent, PRINT_HIGH, "Invalid delay. Value must be between 0.0 and 10.0 seconds.\n");
                        return;
                    }
                }
            }
        }
    }


    void Cmd_Jump_Mapsleft(edict_t* ent)
    {
        LocalScores::PrintMapsLeft(ent);
    }


    void Cmd_Jump_Mapsdone(edict_t* ent)
    {
        LocalScores::PrintMapsDone(ent);
    }

    void Cmd_Jump_Jumpers(edict_t* ent)
    {
        ent->client->jumpdata->show_jumpers = !ent->client->jumpdata->show_jumpers;

        if (ent->client->jumpdata->show_jumpers)
        {
            for (int i = 0; i < game.maxclients; i++)
            {
                edict_t* user = &g_edicts[i + 1];
                if (user->inuse && user->client != nullptr)
                {
                    const char* skin = nullptr;
                    if (user->client->jumpdata->team == TeamEnum::Spectator)
                    {
                        skin = va("%s\\female/invis", user->client->pers.netname);
                    }
                    else if (user->client->jumpdata->team == TeamEnum::Easy)
                    {
                        skin = va("%s\\female/ctf_r", user->client->pers.netname);
                    }
                    else // team Hard
                    {
                        skin = va("%s\\female/ctf_b", user->client->pers.netname);
                    }
                    gi.WriteByte(svc_configstring);
                    gi.WriteShort(CS_PLAYERSKINS + i);
                    gi.WriteString(const_cast<char*>(skin));
                    gi.unicast(ent, true);
                }
            }
            gi.cprintf(ent, PRINT_HIGH, "Player models are now ON.\n");
        }
        else
        {
            for (int i = 0; i < game.maxclients; i++)
            {
                edict_t* user = &g_edicts[i + 1];
                if (user->inuse && user->client != nullptr)
                {
                    const char* skin = va("%s\\female/invis", user->client->pers.netname);
                    gi.WriteByte(svc_configstring);
                    gi.WriteShort(CS_PLAYERSKINS + i);
                    gi.WriteString(const_cast<char*>(skin));
                    gi.unicast(ent, true);
                }
            }
            gi.cprintf(ent, PRINT_HIGH, "Player models are now OFF.\n");
        }
    }





    void HandleGlobalCmdResponse(const global_cmd_response& response)
    {
        global_cmd cmd_type = response.cmd_base->get_type();

        switch (cmd_type)
        {
        case global_cmd::playertimes:
            HandleGlobalPlayertimesResponse(response);
            break;
        case global_cmd::playerscores:
            HandleGlobalPlayerscoresResponse(response);
            break;
        case global_cmd::playermaps:
            HandleGlobalPlayermapsResponse(response);
            break;
        case global_cmd::maptimes:
            HandleGlobalMaptimesResponse(response);
            break;
        default:
            break;
        }
    }

    void HandleGlobalPlayertimesResponse(const global_cmd_response& response)
    {
        if (response.cmd_base->get_type() != global_cmd::playertimes)
        {
            assert(false);
            Logger::Error("ASSERT FAILURE: invalid input args to HandleGlobalPlayertimesResponse");
            return;
        }

        const global_cmd_playertimes* cmd = dynamic_cast<global_cmd_playertimes*>(response.cmd_base.get());
        if (!cmd->user->inuse)
        {
            return;
        }
        if (!response.success)
        {
            gi.cprintf(cmd->user, PRINT_HIGH, "Error contacting global database.\n");
            return;
        }
        if (response.data.empty())
        {
            Logger::Error("Global playertimes command returned no data");
            gi.cprintf(cmd->user, PRINT_HIGH, "Error contacting global database.\n");
            return;
        }
        rapidjson::Document json_obj;
        json_obj.Parse(response.data.c_str());

        size_t user_count = json_obj["user_records"].GetArray().Size();
        if (user_count == 0)
        {
            gi.cprintf(cmd->user, PRINT_HIGH, "There are no global playertimes for this page.\n");
            return;
        }
        
        // Point info
        gi.cprintf(cmd->user, PRINT_HIGH, "-----------------------------------------\n");
        gi.cprintf(cmd->user, PRINT_HIGH, "Point Values: 1-15: 25,20,16,13,11,10,9,8,7,6,5,4,3,2,1\n");
        gi.cprintf(cmd->user, PRINT_HIGH, "-----------------------------------------\n");

        // Header row
        std::string header = GetGreenConsoleText(
            "No. Name            1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th Score");
        gi.cprintf(cmd->user, PRINT_HIGH, "%s\n", header.c_str());

        // Highscores
        for (size_t i = 0; i < user_count; ++i)
        {
            int rank = json_obj["user_records"][i]["rank"].GetInt();

            const char* username = json_obj["user_records"][i]["username"].GetString();

            const char* highscore_counts_str = json_obj["user_records"][i]["highscore_counts"].GetString();
            std::vector<std::string> highscore_counts = SplitString(highscore_counts_str, ',');

            int score = json_obj["user_records"][i]["total_score"].GetInt();

            gi.cprintf(cmd->user, PRINT_HIGH, "%-3d %-15s %3d %3d %3d %3d %3d %3d %3d %3d %3d %4d %4d %4d %4d %4d %4d %5d\n",
                rank,
                username,
                std::stoi(highscore_counts[0]),
                std::stoi(highscore_counts[1]),
                std::stoi(highscore_counts[2]),
                std::stoi(highscore_counts[3]),
                std::stoi(highscore_counts[4]),
                std::stoi(highscore_counts[5]),
                std::stoi(highscore_counts[6]),
                std::stoi(highscore_counts[7]),
                std::stoi(highscore_counts[8]),
                std::stoi(highscore_counts[9]),
                std::stoi(highscore_counts[10]),
                std::stoi(highscore_counts[11]),
                std::stoi(highscore_counts[12]),
                std::stoi(highscore_counts[13]),
                std::stoi(highscore_counts[14]),
                score
            );
        }

        // Footer
        int page = json_obj["page"].GetInt();
        int total_pages = json_obj["max_pages"].GetInt();
        int total_users = json_obj["user_count"].GetInt();
        gi.cprintf(cmd->user, PRINT_HIGH, "Page %d/%d (%d users). Use playertimesglobal <page>\n",
            page, total_pages, total_users);
        gi.cprintf(cmd->user, PRINT_HIGH, "-----------------------------------------\n");
    }

    void HandleGlobalPlayerscoresResponse(const global_cmd_response& response)
    {
        if (response.cmd_base->get_type() != global_cmd::playerscores)
        {
            assert(false);
            Logger::Error("ASSERT FAILURE: invalid input args to HandleGlobalPlayerscoresResponse");
            return;
        }

        const global_cmd_playerscores* cmd = dynamic_cast<global_cmd_playerscores*>(response.cmd_base.get());
        if (!cmd->user->inuse)
        {
            return;
        }
        if (!response.success)
        {
            gi.cprintf(cmd->user, PRINT_HIGH, "Error contacting global database.\n");
            return;
        }
        if (response.data.empty())
        {
            Logger::Error("Global playerscores command returned no data");
            gi.cprintf(cmd->user, PRINT_HIGH, "Error contacting global database.\n");
            return;
        }
        rapidjson::Document json_obj;
        json_obj.Parse(response.data.c_str());

        size_t user_count = json_obj["user_records"].GetArray().Size();
        if (user_count == 0)
        {
            gi.cprintf(cmd->user, PRINT_HIGH, "There are no global playerscores for this page.\n");
            return;
        }

        // Point info
        gi.cprintf(cmd->user, PRINT_HIGH, "-----------------------------------------\n");
        gi.cprintf(cmd->user, PRINT_HIGH, "Point Values: 1-15: 25,20,16,13,11,10,9,8,7,6,5,4,3,2,1\n");
        gi.cprintf(cmd->user, PRINT_HIGH, "Score = (Your score) / (Potential score if 1st on all your completed maps\n");
        gi.cprintf(cmd->user, PRINT_HIGH, "Ex: 5 maps completed || 3 1st's, 2 3rd's = 107 pts || 5 1st's = 125 pts || 107/125 = 85.6%%\n");
        gi.cprintf(cmd->user, PRINT_HIGH, "You must complete 50 maps before your score is calculated.\n");
        gi.cprintf(cmd->user, PRINT_HIGH, "-----------------------------------------\n");

        // Header row
        std::string header = GetGreenConsoleText(
            "No. Name            1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th Score");
        gi.cprintf(cmd->user, PRINT_HIGH, "%s\n", header.c_str());

        // Highscores
        for (size_t i = 0; i < user_count; ++i)
        {
            int rank = json_obj["user_records"][i]["rank"].GetInt();

            const char* username = json_obj["user_records"][i]["username"].GetString();

            const char* highscore_counts_str = json_obj["user_records"][i]["highscore_counts"].GetString();
            std::vector<std::string> highscore_counts = SplitString(highscore_counts_str, ',');

            float percent_score = json_obj["user_records"][i]["percent_score"].GetFloat();

            gi.cprintf(cmd->user, PRINT_HIGH, "%-3d %-15s %3d %3d %3d %3d %3d %3d %3d %3d %3d %4d %4d %4d %4d %4d %4d %2.1f%%\n",
                rank,
                username,
                std::stoi(highscore_counts[0]),
                std::stoi(highscore_counts[1]),
                std::stoi(highscore_counts[2]),
                std::stoi(highscore_counts[3]),
                std::stoi(highscore_counts[4]),
                std::stoi(highscore_counts[5]),
                std::stoi(highscore_counts[6]),
                std::stoi(highscore_counts[7]),
                std::stoi(highscore_counts[8]),
                std::stoi(highscore_counts[9]),
                std::stoi(highscore_counts[10]),
                std::stoi(highscore_counts[11]),
                std::stoi(highscore_counts[12]),
                std::stoi(highscore_counts[13]),
                std::stoi(highscore_counts[14]),
                percent_score
            );
        }

        // Footer
        int page = json_obj["page"].GetInt();
        int total_pages = json_obj["max_pages"].GetInt();
        int total_users = json_obj["user_count"].GetInt();
        gi.cprintf(cmd->user, PRINT_HIGH, "Page %d/%d (%d users). Use playerscoresglobal <page>\n",
            page, total_pages, total_users);
        gi.cprintf(cmd->user, PRINT_HIGH, "-----------------------------------------\n");
    }

    void HandleGlobalPlayermapsResponse(const global_cmd_response& response)
    {
        if (response.cmd_base->get_type() != global_cmd::playermaps)
        {
            assert(false);
            Logger::Error("ASSERT FAILURE: invalid input args to HandleGlobalPlayermapsResponse");
            return;
        }

        const global_cmd_playermaps* cmd = dynamic_cast<global_cmd_playermaps*>(response.cmd_base.get());
        if (!cmd->user->inuse)
        {
            return;
        }
        if (!response.success)
        {
            gi.cprintf(cmd->user, PRINT_HIGH, "Error contacting global database.\n");
            return;
        }
        if (response.data.empty())
        {
            Logger::Error("Global playermaps command returned no data");
            gi.cprintf(cmd->user, PRINT_HIGH, "Error contacting global database.\n");
            return;
        }
        rapidjson::Document json_obj;
        json_obj.Parse(response.data.c_str());

        size_t user_count = json_obj["user_records"].GetArray().Size();
        if (user_count == 0)
        {
            gi.cprintf(cmd->user, PRINT_HIGH, "There are no global playermaps for this page.\n");
            return;
        }

        // Header
        gi.cprintf(cmd->user, PRINT_HIGH, "--------------------------------------\n");
        std::string header = GetGreenConsoleText("No. Name            Maps     %\n");
        gi.cprintf(cmd->user, PRINT_HIGH, "%s\n", header.c_str());

        // Highscores
        for (size_t i = 0; i < user_count; ++i)
        {
            int rank = json_obj["user_records"][i]["rank"].GetInt();

            const char* username = json_obj["user_records"][i]["username"].GetString();

            int completions = json_obj["user_records"][i]["completions"].GetInt();

            float percent_complete = json_obj["user_records"][i]["percent_complete"].GetFloat();

            gi.cprintf(cmd->user, PRINT_HIGH, "%-3d %-15s %4d  %2.1f\n",
                rank,
                username,
                completions,
                percent_complete
            );
        }

        // Footer
        int page = json_obj["page"].GetInt();
        int total_pages = json_obj["max_pages"].GetInt();
        int total_users = json_obj["user_count"].GetInt();
        gi.cprintf(cmd->user, PRINT_HIGH, "Page %d/%d (%d users). Use playermapsglobal <page>\n",
            page, total_pages, total_users);
        gi.cprintf(cmd->user, PRINT_HIGH, "--------------------------------------\n");
    }

    void HandleGlobalMaptimesResponse(const global_cmd_response& response)
    {
        if (response.cmd_base->get_type() != global_cmd::maptimes)
        {
            assert(false);
            Logger::Error("ASSERT FAILURE: invalid input args to HandleGlobalMaptimesResponse");
            return;
        }

        const global_cmd_maptimes* cmd = dynamic_cast<global_cmd_maptimes*>(response.cmd_base.get());
        if (!cmd->user->inuse)
        {
            return;
        }
        if (!response.success)
        {
            gi.cprintf(cmd->user, PRINT_HIGH, "Error contacting global database.\n");
            return;
        }
        if (response.data.empty())
        {
            Logger::Error("Global maptimes command returned no data");
            gi.cprintf(cmd->user, PRINT_HIGH, "Error contacting global database.\n");
            return;
        }
        rapidjson::Document json_obj;
        json_obj.Parse(response.data.c_str());

        size_t user_count = json_obj["user_records"].GetArray().Size();
        if (user_count == 0)
        {
            gi.cprintf(cmd->user, PRINT_HIGH, "There are no global maptimes for this page.\n");
            return;
        }

        // Header
        gi.cprintf(cmd->user, PRINT_HIGH, "--------------------------------------------------------\n");
        gi.cprintf(cmd->user, PRINT_HIGH, "Best Times for %s\n", cmd->mapname.c_str());
        std::string header = "No. Name            Date        Server        Time";
        header = GetGreenConsoleText(header);
        gi.cprintf(cmd->user, PRINT_HIGH, "%s\n", header.c_str());

        // Highscores
        for (size_t i = 0; i < user_count; ++i)
        {
            int rank = json_obj["user_records"][i]["rank"].GetInt();

            const char* username = json_obj["user_records"][i]["username"].GetString();

            const char* servername = json_obj["user_records"][i]["server_name_short"].GetString();

            int64_t date =  json_obj["user_records"][i]["date"].GetInt64();
            std::string date_str = GetDateStringFromTimestamp(date);

            int64_t time_ms = json_obj["user_records"][i]["time_ms"].GetInt64();
            std::string time_ms_str = GetCompletionTimeDisplayString(time_ms);

            int64_t pmove_time_ms = json_obj["user_records"][i]["pmove_time_ms"].GetInt64();

            gi.cprintf(cmd->user, PRINT_HIGH, "%-3d %-15s %-10s  %-6s %11s\n",
                rank,
                username,
                date_str.c_str(),
                servername,
                time_ms_str.c_str()
            );
        }

        // Footer
        int page = json_obj["page"].GetInt();
        int total_pages = json_obj["max_pages"].GetInt();
        int total_users = json_obj["user_count"].GetInt();
        gi.cprintf(cmd->user, PRINT_HIGH, "Page %d/%d (%d users). Use maptimesglobal <map> <page>\n",
            page, total_pages, total_users);
        gi.cprintf(cmd->user, PRINT_HIGH, "--------------------------------------------------------\n");
    }

} // namespace Jump