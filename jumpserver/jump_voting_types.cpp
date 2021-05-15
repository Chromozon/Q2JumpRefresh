#include "g_local.h"
#include "jump_voting.h"
#include "jump_utils.h"
#include <algorithm>
#include <random>
#include <iterator>
#include "jump_scores.h"

//
// Define vote types here
//
namespace Jump
{
    char vote_options[10][64];


    //
    // Change to a specific map. A simple yes/no vote.
    //
    class MapChangeVote : BaseVoteType
    {
    public:
        MapChangeVote() : BaseVoteType(VOTETYPE_MAPCHANGE, VOTECAST_YESNO), is_random(false) {}


        virtual void ApplyVote(vote_option_t winning_option) override
        {
            gi.bprintf(PRINT_HIGH, "Changing map to %s...\n", map_name.c_str());

            level.exitintermission = 1;
                
                
            map_name.copy(level.nextmap, sizeof(level.nextmap));
            level.changemap = level.nextmap;
        }

        virtual std::string GetShortDescription() const override
        {
            if (is_random)
            {
                return std::string(va("Random map: %s", map_name.c_str()));
            }

            return std::string(va("Change map: %s", map_name.c_str()));
        }

        virtual std::string GetDescription() const override
        {
            if (is_random)
            {
                return std::string(va("Change map to %s (random)", map_name.c_str()));
            }

            return std::string(va("Change map to %s", map_name.c_str()));
        }

        virtual bool ParseArguments(edict_t* caster, const std::string& arguments) override
        {
            if (gi.argc() < 2)
            {
                gi.cprintf(caster, PRINT_HIGH,
                    "Invalid args. Usage: mapvote <mapname>, mapvote random, mapvote todo, mapvote next");
                return false;
            }
            std::string arg = gi.argv(1);

            if (stricmp(arg.c_str(), "random") == 0)
            {
                map_name = LocalScores::GetRandomMap();
                return !map_name.empty();
            }
            else
            {
                if (LocalScores::IsMapInMaplist(arg))
                {
                    map_name = arguments;
                    return true;
                }
                else
                {
                    gi.cprintf(caster, PRINT_HIGH, "Map %s does not exist in the map list.\n", arg.c_str());
                    return false;
                }
            }
        }
    private:
        std::string map_name;

        bool is_random;
    };

    static MapChangeVote mapchangevote;


    //
    // Extend / subtract time. Simple yes/no vote.
    //
    class ExtendMapVote : BaseVoteType
    {
    public:
        ExtendMapVote() : BaseVoteType(VOTETYPE_VOTETIME, VOTECAST_YESNO), extend_amount(0) {}


        virtual void ApplyVote(vote_option_t winning_option) override
        {
            assert(timelimit->value != 0);

            jump_server.time_added_mins += extend_amount;
        }

        virtual std::string GetShortDescription() const override
        {
            return std::string(va("Extend map: %i mins", (int)extend_amount));
        }

        virtual std::string GetDescription() const override
        {
            return std::string(va("Extend map by %i minutes", (int)extend_amount));
        }

        virtual bool ParseArguments(edict_t* caster, const std::string& arguments) override
        {
            // TODO: Finish this
            if (timelimit->value == 0)
            {
                gi.cprintf(caster, PRINT_HIGH, "You cannot extend if the map has no timelimit!\n");
                return false;
            }

            extend_amount = atoi(arguments.c_str());

            if (extend_amount == 0)
            {
                gi.cprintf(caster, PRINT_HIGH, "Please give a valid value.\n");
                return false;
            }

            return true;
        }

    private:
        int extend_amount;
    };

    static ExtendMapVote extendmapvote;


    //
    // Nominate a map to the map ending vote screen. Simple yes/no vote.
    //
    class NominateMapVote : BaseVoteType
    {
    public:
        NominateMapVote() : BaseVoteType(VOTETYPE_NOMINATE, VOTECAST_YESNO) {}


        virtual void ApplyVote(vote_option_t winning_option) override
        {
            // TODO: Finish nomination

            //store.value_str.copy(level.nextmap, sizeof(level.nextmap));
        }

        virtual std::string GetShortDescription() const override
        {
            return std::string(va("Nominate: %s", map_name.c_str()));
        }

        virtual std::string GetDescription() const override
        {
            return std::string(va("Nominate map %s to voting list", map_name.c_str()));
        }

        virtual bool ParseArguments(edict_t* caster, const std::string& arguments) override
        {
            if (jump_server.maplist.find(arguments) != jump_server.maplist.end())
            {
                map_name = arguments;
                return true;
            }
            else
            {
                gi.cprintf(caster, PRINT_HIGH, "Map %s does not exist in the map list.\n", arguments.c_str());
                return false;
            }
        }

    private:
        std::string map_name;
    };

    static NominateMapVote nominatemapvote;


    //
    // When a map is about to end. Opens up a PMenu with 4 options.
    //
    class MapEndVote : BaseVoteType
    {
    public:
        MapEndVote() : BaseVoteType(VOTETYPE_MAPEND, VOTECAST_PMENU) {}


        virtual void ApplyVote(vote_option_t winning_option) override
        {
            switch (winning_option)
            {
            // Change map
            case VOTEOPTION_1:
            case VOTEOPTION_2:
            case VOTEOPTION_3:
            {
                int index = (int)winning_option;

                level.exitintermission = 1;
                map_names[index].copy(level.nextmap, sizeof(level.nextmap));
                level.changemap = level.nextmap;
                break;
            }
            // Extend
            case VOTEOPTION_4:
                jump_server.time_added_mins += 15;
                break;
            default:
                assert(0);
                break;
            }
             


        }

        virtual std::string GetShortDescription() const override
        {
            return GetDescription();
        }

        virtual std::string GetDescription() const override
        {
            return std::string(va("Map end vote: %s | %s | %s",
                map_names[0].c_str(),
                map_names[1].c_str(),
                map_names[2].c_str()));
        }

        virtual int GetNumVoteOptions() const override
        {
            return 4;
        }

        virtual bool ParseArguments(edict_t* caster, const std::string& arguments) override
        {
            auto names = SplitString(arguments, ' ');
            if (names.size() != 3)
            {
                return false;
            }

            // Check if maps are in the map list.
            for (auto& str : names)
            {
                if (jump_server.maplist.find(str) == jump_server.maplist.end())
                {
                    return false;
                }
            }


            map_names[0] = names[0];
            map_names[1] = names[1];
            map_names[2] = names[2];

            return true;
        }


        virtual void CreatePMenu(std::vector<pmenu_t>& menu) override
        {
            // TODO: This is ultra mega hack
            snprintf(vote_options[0], sizeof(vote_options[0]), "%s: 0", map_names[0].c_str());
            snprintf(vote_options[1], sizeof(vote_options[1]), "%s: 0", map_names[1].c_str());
            snprintf(vote_options[2], sizeof(vote_options[2]), "%s: 0", map_names[2].c_str());
            strncpy(vote_options[3], "Extend: 0", sizeof(vote_options[3]));
            snprintf(vote_options[4], sizeof(vote_options[4]), "Time: %.0f", GetVoteTime());

            pmenu_t votemenu[] = {
	            { "*Quake II JumpMod",                  PMENU_ALIGN_CENTER, nullptr },
	            { "* ",                                 PMENU_ALIGN_CENTER, nullptr },
	            { nullptr,			                    PMENU_ALIGN_CENTER, nullptr },
	            { vote_options[0],		                PMENU_ALIGN_LEFT,   VoteSystem::VoteOption1 },
	            { nullptr,				                PMENU_ALIGN_LEFT,   nullptr },
	            { nullptr,				                PMENU_ALIGN_CENTER, nullptr },
	            { vote_options[1],		                PMENU_ALIGN_LEFT,   VoteSystem::VoteOption2 },
	            { nullptr,				                PMENU_ALIGN_LEFT,   nullptr },
	            { nullptr,				                PMENU_ALIGN_LEFT,   nullptr },
	            { vote_options[2],		                PMENU_ALIGN_LEFT,   VoteSystem::VoteOption3 },
	            { nullptr,			                    PMENU_ALIGN_LEFT,   nullptr },
	            { nullptr,			                    PMENU_ALIGN_LEFT,   nullptr },
	            { vote_options[3],					    PMENU_ALIGN_LEFT,   VoteSystem::VoteOption4 },
	            { nullptr,				                PMENU_ALIGN_LEFT,   nullptr },
	            { "*\x8d\x8d\x8d\x8d\x8d = Hard map",	PMENU_ALIGN_CENTER, nullptr },
	            { "*    \x8d = Easy map",		        PMENU_ALIGN_CENTER, nullptr },
	            { "Highlight choice and ENTER",	        PMENU_ALIGN_CENTER, nullptr },
	            { vote_options[4],	                    PMENU_ALIGN_RIGHT,  nullptr },
            };


            menu.resize(Jump::ArraySize(votemenu));
            std::copy(votemenu, votemenu + Jump::ArraySize(votemenu), menu.begin());
        }

        virtual uint32_t UpdatePMenu(std::vector<pmenu_t>& menu) override
        {
            snprintf(vote_options[0], sizeof(vote_options[0]), "%s: %i", map_names[0].c_str(), VoteSystem::GetNumVotes(VOTEOPTION_1));
            snprintf(vote_options[1], sizeof(vote_options[1]), "%s: %i", map_names[1].c_str(), VoteSystem::GetNumVotes(VOTEOPTION_2));
            snprintf(vote_options[2], sizeof(vote_options[2]), "%s: %i", map_names[2].c_str(), VoteSystem::GetNumVotes(VOTEOPTION_3));
            snprintf(vote_options[3], sizeof(vote_options[3]), "Extend: %i", VoteSystem::GetNumVotes(VOTEOPTION_4));
            snprintf(vote_options[4], sizeof(vote_options[4]), "Time: %.0f", VoteSystem::GetTimeleft());
        
            // Return bitflags of the entries that were changed.
            return 1 << 4 | 1 << 7 | 1 << 10 | 1 << 13 | 1 << 18;
        }

    private:
        std::string map_names[3];
    };

    static MapEndVote mapendvote;


    //
    // Silence (no chatting) a specific player. Simple yes/no vote.
    //
    class SilencePlayerVote : TargetPlayerVoteType
    {
    public:
        SilencePlayerVote() : TargetPlayerVoteType(VOTETYPE_SILENCE, VOTECAST_YESNO) {}


        virtual void ApplyVote(vote_option_t winning_option) override
        {
            // TODO: Implement silence vote.
            assert(GetTargets().size() == 1);

            gi.bprintf(PRINT_HIGH, "Silenced %s.\n", GetTargets()[0].player_name.c_str());
        }

        virtual std::string GetShortDescription() const override
        {
            assert(GetTargets().size() == 1);
            return std::string(va("Silence: %s", GetTargets()[0].player_name.c_str()));
        }

        virtual std::string GetDescription() const override
        {
            assert(GetTargets().size() == 1);
            return std::string(va("Silence: %s", GetTargets()[0].player_name.c_str()));
        }

        virtual float GetYesPercentage() const override
        {
            // TODO: Take into account the fact that the target player's vote shouldn't count?
            return TargetPlayerVoteType::GetYesPercentage();
        }

        virtual bool TargetsSinglePlayer() const override
        {
            return true;
        }

        virtual bool CanTarget(edict_t* caster, edict_t* target) const override
        {
            // TODO: Take into account admin levels. You shouldn't be able to target player's with higher levels.
            return TargetPlayerVoteType::CanTarget(caster, target);
        }

        //
        // Arguments: <player name>
        //
        virtual bool ParseArguments(edict_t* caster, const std::string& arguments) override
        {
            return TargetPlayerVoteType::ParseArguments(caster, arguments);
        }
    };

    static SilencePlayerVote silenceplayervote;


    //
    // Kick a specific player. Simple yes/no vote.
    //
    class KickPlayerVote : public TargetPlayerVoteType
    {
    public:
        KickPlayerVote() : TargetPlayerVoteType(VOTETYPE_KICK, VOTECAST_YESNO) {}


        virtual void ApplyVote(vote_option_t winning_option) override
        {
            assert(GetTargets().size() == 1);

            int player_num = GetTargets()[0].player_index;

            auto* player = g_edicts + player_num;

            gi.AddCommandString(va("kick %d\n", player_num));

            // Already prints.
            //gi.bprintf(PRINT_HIGH, "Kicked %s.\n", GetTargets()[0].player_name.c_str());
        }

        virtual std::string GetShortDescription() const override
        {
            assert(GetTargets().size() == 1);
            return std::string(va("Kick: %s", GetTargets()[0].player_name.c_str()));
        }

        virtual std::string GetDescription() const override
        {
            assert(GetTargets().size() == 1);
            return std::string(va("Kick: %s", GetTargets()[0].player_name.c_str()));
        }

        virtual float GetYesPercentage() const override
        {
            // TODO: Take into account the fact that the target player's vote shouldn't count?
            return TargetPlayerVoteType::GetYesPercentage();
        }

        virtual bool TargetsSinglePlayer() const override
        {
            return true;
        }

        virtual bool CanTarget(edict_t* caster, edict_t* target) const override
        {
            // TODO: Take into account admin levels. You shouldn't be able to target player's with higher levels.
            return TargetPlayerVoteType::CanTarget(caster, target);
        }

        //
        // Arguments: <player name>
        //
        virtual bool ParseArguments(edict_t* caster, const std::string& arguments) override
        {
            return TargetPlayerVoteType::ParseArguments(caster, arguments);
        }
    };

    static KickPlayerVote kickplayervote;
}
