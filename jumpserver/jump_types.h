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
    typedef enum
    {
        TEAM_SPECTATOR,
        TEAM_EASY,
        TEAM_HARD
    } team_t;

    typedef enum
    {
        LEVEL_STATE_FREEPLAY,
        LEVEL_STATE_VOTING,
        LEVEL_STATE_INTERMISSION
    } level_state_t;

    typedef enum
    {
        KEY_STATE_NONE = 0,
        KEY_STATE_FORWARD = 1 << 0,
        KEY_STATE_BACK = 1 << 1,
        KEY_STATE_LEFT = 1 << 2,
        KEY_STATE_RIGHT = 1 << 3,
        KEY_STATE_JUMP = 1 << 4,
        KEY_STATE_CROUCH = 1 << 5,
        KEY_STATE_ATTACK = 1 << 6,
    } key_state_t;

    typedef enum
    {
        REPLAY_SPEED_NEG_100 = -10000,
        REPLAY_SPEED_NEG_25 = -2500,
        REPLAY_SPEED_NEG_10 = -1000,
        REPLAY_SPEED_NEG_5 = -500,
        REPLAY_SPEED_NEG_2 = -200,
        REPLAY_SPEED_NEG_1 = -100,
        REPLAY_SPEED_NEG_HALF = -50,
        REPLAY_SPEED_NEG_FIFTH = -20,
        REPLAY_SPEED_NEG_TENTH = -10,
        REPLAY_SPEED_0 = 0,
        REPLAY_SPEED_TENTH = 10,
        REPLAY_SPEED_FIFTH = 20,
        REPLAY_SPEED_HALF = 50,
        REPLAY_SPEED_1 = 100,
        REPLAY_SPEED_2 = 200,
        REPLAY_SPEED_5 = 500,
        REPLAY_SPEED_10 = 1000,
        REPLAY_SPEED_25 = 2500,
        REPLAY_SPEED_100 = 10000,
    } replay_speed_t;

    typedef enum
    {
        SCORES_MENU_NONE = 0,
        SCORES_MENU_HIGHSCORES = 1,
        SCORES_MENU_ACTIVEPLAYERS = 2,
    } scores_menu_t;

    // How much disk space is required to store a replay:
    // 64 bytes per replay frame, 10 frames per second (because server runs at 10 frames/s) = 640 bytes/s
    // 15 seconds = 9.6 kB
    // 1 minute = 38.4 kB
    // 5 minutes = 192 kB
    // 15 minutes = 576 kB
    // 1 hour = 2.3 MB
    // 24 hours = 55.3 MB
    //
    // Let's say on average 50 players complete each map, and the time is 2 minutes, and there are 3000 maps
    // 50 replays * 77 kB/replay * 3000 maps = 11.6 GB
    //
    typedef struct
    {
        vec3_t pos;                 // (byte 0) player position in the world
        vec3_t angles;              // (byte 12) player view angles

        int32_t animation_frame;    // (byte 24) animation frame number

        int32_t key_states;         // (byte 28) active inputs (jump, crouch, left, right, etc.)

        int16_t fps;                // (byte 32) current fps
        int8_t async;               // (byte 34) async 0 or 1
        int8_t checkpoints;         // (byte 35) number of checkpoints picked up

        uint16_t weapon_inven;      // (byte 36) picked up weapon bitset
        uint8_t weapon_equipped;    // (byte 38) enum for currently equipped weapon
        uint8_t reserved1;          // (byte 39)

        uint64_t reserved2;         // (byte 40)
        uint64_t reserved3;         // (byte 48)
        uint64_t reserved4;         // (byte 56)
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

    // Data unique to each client
    class client_data_t
    {
    public:
        client_data_t()
        {
            replay_recording.reserve(10000);
            replay_spectating_framenum = 0;
            update_replay_spectating = false;
            fps = 0;
            async = 0;
            team = TEAM_SPECTATOR;
            timer_pmove_msec = 0;
            timer_begin = 0;
            timer_end = 0;
            timer_paused = true;
            timer_finished = false;
            store_ent = NULL;
            key_states = 0;
            scores_menu = SCORES_MENU_NONE;
        }

        std::vector<replay_frame_t> replay_recording;
        std::vector<replay_frame_t> replay_spectating;
        int replay_spectating_framenum;
        bool update_replay_spectating;

        int fps;
        int async;
        std::string ip;
        team_t team;
        int64_t timer_pmove_msec;
        int64_t timer_begin;
        int64_t timer_end;
        bool timer_paused;
        bool timer_finished;

        store_buffer_t stores;
        edict_t* store_ent;

        int key_states; // input actions that are currently active

        scores_menu_t scores_menu;
    };

    class server_data_t
    {
    public:
        server_data_t()
        {
            level_state = LEVEL_STATE_FREEPLAY;
            time_added_mins = 0;
            replay_now_time_ms = INT64_MAX;
        }

        level_state_t level_state;    // freeplay, voting, or intermission
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
    };

    // Converts a replay buffer into a byte array
    std::vector<uint8_t> SerializeReplayBuffer(const std::vector<replay_frame_t>& replay_buffer);

    // Converts a byte array into a replay buffer
    std::vector<replay_frame_t> DeserializeReplayBuffer(const std::vector<uint8_t>& bytes);
}