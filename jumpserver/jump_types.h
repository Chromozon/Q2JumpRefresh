#pragma once

#include "q_shared.h"
#define GAME_INCLUDE
#include "game.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <thread>

#define MAX_STORES 5    // max number of stores for store/recall cmds
#define MAX_HIGHSCORES 15   // how many users to list on the highscores HUD
#define CONSOLE_HIGHSCORES_COUNT_PER_PAGE 20    // users per page of statistics in console (playertimes, playerscores, etc.)

#define SCORES_DIR "scores"
#define TIME_FILE_EXTENSION ".time"
#define DEMO_FILE_EXTENSION ".demo"
#define SEEN_DIR "seen"
#define SEEN_FILE_EXTENSION ".seen"
#define MAPLIST_FILENAME "maplist.txt"

namespace Jump
{
    enum class TeamEnum
    {
        Spectator,
        Easy,
        Hard,
    };

    enum class LevelStateEnum
    {
        FreePlay,
        Intermission,
    };

    enum class KeyStateEnum // flags
    {
        None = 0,
        Jump = 1,
        Crouch = 2,
        Left = 4,
        Right = 8,
        Forward = 16,
        Back = 32,
        Attack = 64,
    };

    // TODO remove this and just make an array of double like current version
    enum class ReplaySpeed
    {
        Neg_100 = -10000,
        Neg_25 = -2500,
        Neg_10 = -1000,
        Neg_5 = -500,
        Neg_2 = -200,
        Neg_1 = -100,
        Neg_Half = -50,
        Neg_Fifth = -20,
        Neg_Tenth = -10,
        Paused = 0,
        Pos_Tenth = 10,
        Pos_Fifth = 20,
        Pos_Half = 50,
        Pos_1 = 100,
        Pos_2 = 200,
        Pos_5 = 500,
        Pos_10 = 1000,
        Pos_25 = 2500,
        Pos_100 = 10000,
    };

    enum class ScoresMenuEnum
    {
        None,
        Highscores,
        ActivePlayers,
    };

    enum class AsyncEnum
    {
        Unknown = 0,
        One = 1,
        Zero = 2,
    };

    enum class IdleStateEnum : int
    {
        None = 0, // Player not idle.
        Auto, // Player was idle for too long.
        Self // Player used 'idle' command and set themselves idle.
    };

    // How much disk space is required to store a replay:
    // 48 bytes per replay frame, 10 frames per second (because server runs at 10 frames/s) = 480 bytes/s
    // 15 seconds = 7.2 kB
    // 1 minute = 28.8 kB
    // 5 minutes = 144 kB
    // 15 minutes = 432 kB
    // 1 hour = 1.7 MB
    // 24 hours = 41.5 MB
    //
    typedef struct
    {
        vec3_t angles;              // (byte 0-11) player view angles
        vec3_t pos;                 // (byte 12-23) player position in the world

        uint8_t animation_frame;    // (byte 24) animation frame of the player model
        uint8_t fps;                // (byte 25) fps
        uint8_t key_states;         // (byte 26) active inputs bitset (jump, crouch, left, right, etc.)
        uint8_t checkpoints;        // (byte 27) number of checkpoints picked up

        uint8_t async;              // (byte 28) async 1 = 1, async 0 = 2, unknown = 0
        uint8_t weapon_equipped;    // (byte 29) enum for currently equipped weapon
        uint16_t weapon_inven;      // (byte 30-31) picked up weapon bitset

        uint64_t reserved1;         // (byte 32-39)
        uint64_t reserved2;         // (byte 40-47)
    } replay_frame_t;

    typedef struct
    {
        int64_t time_interval;
        vec3_t pos;
        vec3_t angles;
        // todo weapons
    } store_data_t;

    typedef std::string username_key; // The username key is the username in lowercase

    class store_buffer_t
    {
    public:
        store_buffer_t();
        void PushStore(const store_data_t& data);
        store_data_t GetStore(int prevNum);
        void Reset();
        bool HasStore();
    private:
        int numStores;
        int nextIndex;
        store_data_t stores[MAX_STORES];
    };

    class user_time_record {
    public:
        user_time_record()
        {
            time_ms = 0;
            completions = 0;
        }

        std::string filepath;
        username_key username_key;
        int64_t time_ms;
        std::string date;
        int32_t completions;
    };

    typedef struct {
        std::string username;
        int32_t total_maps;
        std::vector<std::string> highscore_maps[MAX_HIGHSCORES]; // [0] is a list of all first places, [1] is all second places, etc.
    } user_overall_record;

    class user_highscores_t
    {
    public:
        user_highscores_t()
        {
            highscore_counts.resize(MAX_HIGHSCORES, 0);
            map_count = 0;
        }

        std::vector<int> highscore_counts;  // count of first places, second places, etc. by index (0-based)
        int map_count;
    };

    // Data unique to each client that is persists between level changes.
    class client_pers_data_t
    {
    public:
        client_pers_data_t()
        {
            prev_idle_upmove = 0;
            prev_idle_forwardmove = 0;
            prev_idle_sidemove = 0;
            idle_state = IdleStateEnum::None;
            idle_msec = 0;
        }

        int prev_idle_upmove;
        int prev_idle_forwardmove;
        int prev_idle_sidemove;
        IdleStateEnum idle_state;
        uint64_t idle_msec;
    };

    // Data unique to each client
    class client_data_t
    {
    public:
        std::vector<replay_frame_t> replay_recording;

        std::vector<replay_frame_t> replay_spectating;
        int replay_spectating_framenum = 0;
        bool update_replay_spectating = false;
        std::string replay_spectating_hud_string; // describes who we are currently replaying (now, self, 1, 2, etc.)
        ReplaySpeed replay_speed = ReplaySpeed::Pos_1;
        bool replay_repeating = false;

        int localUserId = -1;
        int fps = 0;
        AsyncEnum async = AsyncEnum::Unknown;
        std::string ip;
        TeamEnum team = TeamEnum::Spectator;
        int64_t timer_pmove_msec = 0;
        int64_t timer_begin = 0;
        int64_t timer_end = 0;
        bool timer_paused = true;
        bool timer_finished = false;

        store_buffer_t stores;
        edict_t* store_ent = nullptr;

        uint8_t key_states = 0; // input actions that are currently active

        ScoresMenuEnum scores_menu = ScoresMenuEnum::None;

        bool racing = false;
        std::vector<replay_frame_t> racing_frames;
        int racing_framenum = 0;
        int racing_delay_frames = 0;
        int racing_highscore = 0;

        std::string hud_footer1; // it's easier to store the footer strings here so that chasecam can just copy these instead of recalculating them
        std::string hud_footer2;
        std::string hud_footer3;
        std::string hud_footer4;

        bool show_jumpers = true;

        uint8_t checkpoint_total = 0;
        std::vector<edict_t*> checkpoints_obtained;
        int64_t timer_checkpoint_split = 0; // TODO rename from timer

        int64_t timer_trigger_spam = 0; // TODO rename from timer

        // Cache from local database.
        int cached_time_msec; // Player's team hard time.
        int cached_completions;
        int cached_maps_completed;
    };

    class server_data_t
    {
    public:
        server_data_t()
        {
            level_state = LevelStateEnum::FreePlay;
            time_added_mins = 0;
            replay_now_time_ms = INT64_MAX;
            local_map_id = -1;

            cvar_idle_time = nullptr;
        }

        LevelStateEnum level_state;    // freeplay, voting, or intermission
        int time_added_mins;

        std::string last_map1;
        std::string last_map2;
        std::string last_map3;

        std::unordered_set<username_key> fresh_times;

        std::vector<replay_frame_t> replay_now_recording;
        std::string replay_now_username;
        int64_t replay_now_time_ms;

        // List of all maps on the server.  Times will ony be recorded for these maps.
        std::unordered_set<std::string> maplist;

        // Table of all maps with the times sorted best to worst.  This list is kept up-to-date as
        // players set new times while playing maps.
        std::unordered_map<std::string /*mapname*/, std::vector<user_time_record>> all_local_maptimes;

        // List of all user highscores sorted by score best to worst.
        // Only updated on map change.
        std::vector<std::pair<username_key, user_highscores_t>> all_local_highscores;

        // List of all user map count sorted best to worst
        // Only updated on map change.
        std::vector<std::pair<username_key, int>> all_local_mapcounts;

        // List of all user mapscores (%) sorted best to worst
        // Only updated on map change.
        std::vector<std::pair<username_key, user_highscores_t>> all_local_mapscores;

        // Links the username_key (all lowercase) to the display username
        std::unordered_map<username_key, std::string> all_local_usernames; // TODO!! calc when doing maptimes

        // List of last seen times sorted newest to oldest
        std::vector<std::pair<std::string /*username*/, int64_t>> last_seen;

        // Thread that talks to the global database
        std::thread global_database_thread;

        // MapId in the local database for the current level
        int local_map_id;

        // How long the player needs to be idle for, in seconds, before they are set as idle. 0 = no auto-idling
        cvar_t* cvar_idle_time;
    };

    // Converts a replay buffer into a byte array
    std::vector<uint8_t> SerializeReplayBuffer(const std::vector<replay_frame_t>& replay_buffer);

    // Converts a byte array into a replay buffer
    std::vector<replay_frame_t> DeserializeReplayBuffer(const std::vector<uint8_t>& bytes);
}