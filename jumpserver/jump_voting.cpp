#include "g_local.h"
#include "jump_voting.h"
#include "jump_hud.h"
#include "jump_utils.h"

#include <sstream>
#include <algorithm>


namespace Jump
{
    static BaseVoteType* cur_vote_type;
    static float cur_vote_end_time;
    static std::vector<pmenu_t> cur_vote_menu;
    static std::string cur_caster_name = "N/A";

    static pmenuhnd_t* cur_pmenu_handle;

    static std::vector<voted_t> voted;
    static std::vector<int> participants;

    // Holds all the registered vote types.
    static std::vector<BaseVoteType*> _votetypes;


    //
    // Base vote type
    // New vote types register themselves to the list.
    //
    BaseVoteType::BaseVoteType(vote_type_t type, vote_cast_type_t cast_type) : vote_type(type), vote_cast(cast_type)
    {
        _votetypes.push_back(this);
    }

    BaseVoteType::~BaseVoteType()
    {
        if (cur_vote_type == this)
        {
            assert(0);
            cur_vote_type = nullptr;
        }

        _votetypes.erase(std::find(_votetypes.begin(), _votetypes.end(), this));
    }

    bool BaseVoteType::ParseArguments(edict_t* caster, const std::string& arguments)
    {
        return true;
    }

    void BaseVoteType::OnStartVote(edict_t* caster)
    {
        std::string caster_name;
        if (caster != nullptr)
        {
            caster_name = caster->client->pers.netname;
        }
        else
        {
            caster_name = "Server";
        }


        gi.bprintf(PRINT_HIGH, "%s has started a vote: %s\n",
            caster_name.c_str(),
            GetDescription().c_str());


        // Format hud vote hud
        if (GetCastType() != VOTECAST_PMENU)
        {
            gi.configstring(CS_JUMP_KEY_HUD_VOTE_INITIATED, (char*)GetGreenConsoleText(std::string("Vote by " + caster_name)).c_str());
            gi.configstring(CS_JUMP_KEY_HUD_VOTE_TYPE, (char*)GetShortDescription().c_str());
        }
    }

    float BaseVoteType::GetYesPercentage() const
    {
        // TODO: Roll this into a cvar?
        return 0.75f;
    }

    bool BaseVoteType::CanCastVote(edict_t* player, vote_option_t option) const
    {
        if (GetCastType() == VOTECAST_YESNO)
        {
            return option == VOTEOPTION_YES || option == VOTEOPTION_NO;
        }
        else
        {
            return (int)option < GetNumVoteOptions();
        }
    }

    float BaseVoteType::GetVoteTime() const
    {
        // TODO: Roll this into a cvar?
        return 30.0f;
    }

    int BaseVoteType::GetNumVoteOptions() const
    {
        return 2;
    }


    //
    //
    //
    TargetPlayerVoteType::TargetPlayerVoteType(vote_type_t type, vote_cast_type_t vote_cast) : BaseVoteType(type, vote_cast)
    {
    }

    bool TargetPlayerVoteType::ParseArguments(edict_t* caster, const std::string& arguments)
    {
        targets.clear();


        if (!arguments.size())
        {
            gi.cprintf(caster, PRINT_HIGH, "This command requires a target!\n");
            return false;
        }

        // Find the target.
        for (int i = 1; i <= game.maxclients; i++)
        {
            auto player = &(g_edicts[i]);
            auto client = player->client;

            if (!player->inuse || !client)
                continue;

            if (Q_stricmp(client->pers.netname, (char*)arguments.c_str()) == 0 && CanTarget(caster, player))
            {
                AddTarget(player);

                if (TargetsSinglePlayer())
                    break;
            }
        }

        if (!targets.size())
        {
            gi.cprintf(caster, PRINT_HIGH, "Could not find any players named '%s'!\n", arguments.c_str());
            return false;
        }

        return true;
    }

    void TargetPlayerVoteType::AddTarget(edict_t* player)
    {
        int player_num = player - g_edicts;

        targets.push_back({player_num, std::string(player->client->pers.netname)});
    }


    //
    // Vote system
    //
    void VoteSystem::Init()
    {
        cur_vote_type = nullptr;
        cur_vote_end_time = 0.0f;
        cur_pmenu_handle = nullptr;

        voted.reserve(MAX_CLIENTS);
        participants.reserve(MAX_CLIENTS);
    }

    bool VoteSystem::IsVoting()
    {
        return cur_vote_type != nullptr;
    }

    int VoteSystem::GetNumVotes(vote_option_t option)
    {
        assert(IsVoting());

        int ret = 0;

        for (auto& v : voted)
        {
            if (v.option == option)
                ++ret;
        }

        return ret;
    }

    int VoteSystem::GetNumVotesRequired()
    {
        assert(IsVoting());

        auto num_participants = participants.size();
        if (num_participants == 0)
        {
            return 0;
        }

        if (IsYesNoVote())
        {
            int i = (int)(num_participants * cur_vote_type->GetYesPercentage());
            if (i <= 0)
                return 1;

            return i;
        }


        float frac = 1.0f  / cur_vote_type->GetNumVoteOptions();
        frac  *= (cur_vote_type->GetNumVoteOptions() - 1);

        return (int)ceil(num_participants * frac);
    }

    bool VoteSystem::IsYesNoVote()
    {
        assert(IsVoting());

        return cur_vote_type->GetCastType() == VOTECAST_YESNO;
    }

    bool VoteSystem::IsHudVote()
    {
        assert(IsVoting());

        return cur_vote_type->GetCastType() != VOTECAST_PMENU;
    }

    bool VoteSystem::HasVoted(int player_num)
    {
        for (auto& v : voted)
        {
            if (player_num == v.player_index)
                return true;
        }

        return false;
    }

    bool VoteSystem::HasVoted(const edict_t* player)
    {
        return HasVoted(player - g_edicts);
    }

    bool VoteSystem::IsParticipant(edict_t* player)
    {
        int player_num = player - g_edicts;

        return std::find(participants.begin(), participants.end(), player_num) != participants.end();
    }

    float VoteSystem::GetTimeleft()
    {
        assert(IsVoting());
        if (!IsVoting())
        {
            return -1.0f;
        }

        return cur_vote_end_time - level.time;
    }

    BaseVoteType* VoteSystem::GetVoteType(vote_type_t vote_type)
    {
        for (auto& vote : _votetypes)
        {
            if (vote->GetVoteType() == vote_type)
                return vote;
        }

        assert(0);
        return nullptr;
    }

    void VoteSystem::StartVote(edict_t* caster, vote_type_t vote_type)
    {
        cur_vote_type = GetVoteType(vote_type);
        assert(cur_vote_type != nullptr);

        cur_vote_end_time = level.time + cur_vote_type->GetVoteTime();
        cur_caster_name = caster ? caster->client->pers.netname : "Server";


        voted.clear();
        participants.clear();


        // Collect participants
        for (int i = 1; i <= game.maxclients; i++)
        {
            auto player = &(g_edicts[i]);
            auto client = player->client;

            if (!player->inuse || !client)
                continue;

            // Don't allow the caster to participate in this vote.
            // We assume the caster wants this vote to pass!
            if (player == caster && IsYesNoVote())
            {
                continue;
            }

            // Idlers can't participate!
            if (client->jumppers->idle_state != IdleStateEnum::None)
                continue;

            if (!cur_vote_type->CanParticipate(player))
                continue;


            int player_num = player - g_edicts;

            participants.push_back(player_num);
        }


        cur_vote_type->OnStartVote(caster);


        if (IsHudVote())
        {
            UpdateHudVote();
        }
        else
        {
            // Create and open the menu for clients.
            cur_vote_menu.clear();
            cur_vote_type->CreatePMenu(cur_vote_menu);

            for (auto& part : participants)
            {
                edict_t* player = g_edicts + part;

                if (!player->inuse || !player->client)
                    continue;

	            if (player->client->menu)
		            PMenu_Close(player);

	            PMenu_Open(player, cur_vote_menu.data(), -1, cur_vote_menu.size(), NULL);	
            }
        }

        CheckConditions();
    }

    void VoteSystem::OnFrame()
    {
        if (IsVoting())
        {
            CheckConditions();

            if (IsHudVote())
            {
                UpdateHudVote();
            }
            else
            {
                UpdatePMenu();
            }
        }

    }

    void VoteSystem::UpdateHudVote()
    {
        assert(IsVoting());
        assert(IsHudVote());

        // Don't change the text if nothing has been changed.

        static int last_timeleft = 0;


        int timeleft = (int)ceil(cur_vote_end_time - level.time);

        if (timeleft != last_timeleft)
        {
            gi.configstring(CS_JUMP_KEY_HUD_VOTE_REMAINING, va("%d seconds", timeleft));
        }

        if (IsYesNoVote())
        {
            static int last_num_votes = 0;
            static int last_num_required = 0;

            int num_votes = GetNumVotes(VOTEOPTION_YES);
            int num_required = GetNumVotesRequired();

            if (num_votes != last_num_votes || num_required != last_num_required)
            {
                gi.configstring(CS_JUMP_KEY_HUD_VOTE_CAST, va("Votes: %d of %d", num_votes, num_required));
            }

            last_num_votes = num_votes;
            last_num_required = num_required;
        }
        else
        {
            // Not implemented.
            assert(0);

            //int num_options = cur_vote_type->GetNumVoteOptions();
            //for (int i = 0; i < num_options; i++)
            //{
            //    GetNumVotes((vote_option_t)i);
            //}

            //gi.configstring(CS_JUMP_KEY_HUD_VOTE_CAST, "");
        }
        

        last_timeleft = timeleft;

    }

    void VoteSystem::UpdatePMenu()
    {
        assert(IsVoting());
        assert(!IsHudVote());

        auto flags = cur_vote_type->UpdatePMenu(cur_vote_menu);
        if (flags == 0) // No fields were changed.
            return;


        for (auto& part : participants)
        {
            edict_t* player = g_edicts + part;

            if (!player->inuse || !player->client)
                continue;

            if (!player->client->menu)
                continue;

            // Check which entries were changed.
            // Yes, this is cancerous. Fuck PMenu shit.
            auto len = std::min(player->client->menu->num, (int)cur_vote_menu.size());
            for (int i = 1; i <= len; i++)
            {
                uint32_t f = (1 << i);
                if (flags & f)
                {
                    int index = i - 1;
                    PMenu_UpdateEntry(player->client->menu->entries + index,
                                      cur_vote_menu[index].text,
                                      cur_vote_menu[index].align,
                                      cur_vote_menu[index].SelectFunc);
                }
                
            }

	        PMenu_Update(player);	
        }
    }

    void VoteSystem::FinishVote(bool force_apply)
    {
        assert(IsVoting());

        auto option = GetWinningOption();

        auto is_yesnovote = IsYesNoVote();


        if ((is_yesnovote && option == VOTEOPTION_YES) || (!is_yesnovote && option != VOTEOPTION_NONE) || (force_apply && is_yesnovote))
        {
            gi.bprintf(PRINT_HIGH, "%s's vote has passed!\n", cur_caster_name.c_str());


            cur_vote_type->ApplyVote(option);
        }
        else
        {
            gi.bprintf(PRINT_HIGH, "%s's vote has failed!\n", cur_caster_name.c_str());
        }


        // Hide PMenu
        if (!IsHudVote())
        {
            for (auto& part : participants)
            {
                edict_t* player = g_edicts + part;

                if (!player->inuse || !player->client)
                    continue;

	            if (player->client->menu)
		            PMenu_Close(player);
            }
        }



        cur_vote_type = nullptr;
        participants.clear();
        voted.clear();
        cur_vote_end_time = 0;
    }

    bool VoteSystem::AttemptStartVote(edict_t* player, vote_type_t vote_type, const std::string& arguments)
    {
        if (IsVoting())
        {
            gi.cprintf(player, PRINT_HIGH, "There already is a vote going!\n");
            return false;
        }

        if (player->client->jumppers->idle_state != IdleStateEnum::None)
        {
            gi.cprintf(player, PRINT_HIGH, "You cannot start a vote while being idle!\n");
            return false;
        }

        auto vote = GetVoteType(vote_type);
        if (!vote)
        {
            assert(0);
            return false;
        }

        if (!vote->CanStartVote(player))
        {
            gi.cprintf(player, PRINT_HIGH, "You cannot start that vote!\n");
            return false;
        }

        if (!vote->ParseArguments(player, arguments))
        {
            return false;
        }


        StartVote(player, vote_type);
        return true;
    }

    bool VoteSystem::RemoveParticipant(edict_t* player)
    {
        int player_num = player - g_edicts;

        bool removed = false;


        // Remove from participants.
        auto part = std::find(participants.begin(), participants.end(), player_num);
        if (part != participants.end())
        {
            participants.erase(part);
            removed = true;
        }


        // Remove vote.
        for (auto it = voted.begin(); it != voted.end(); it++)
        {
            if (it->player_index == player_num)
            {
                voted.erase(it);
                break;
            }
        }

        return removed;
    }

    bool VoteSystem::CanCastVote(edict_t* player, vote_option_t option)
    {
        if (!IsVoting())
            return false;


        return cur_vote_type->CanCastVote(player, option);
    }

    void VoteSystem::CheckConditions()
    {
        if (!IsVoting())
            return;


        float timeleft = cur_vote_end_time - level.time;

        if (timeleft <= 0.0f || voted.size() >= participants.size() || CanVotePass())
        {
            FinishVote();
            return;
        }
    }

    bool VoteSystem::CanVotePass()
    {
        assert(IsVoting());

        int votes_required = GetNumVotesRequired();

        if (IsYesNoVote())
        {
            if (GetNumVotes(VOTEOPTION_YES) >= votes_required ||
                GetNumVotes(VOTEOPTION_NO) > (participants.size() - votes_required))
            {
                return true;
            }
        }
        else
        {
            for (int i = 0; i < cur_vote_type->GetNumVoteOptions(); i++)
            {
                if (GetNumVotes((vote_option_t)i) >= votes_required)
                {
                    return true;
                }
            }
        }

        return false;
    }

    vote_option_t VoteSystem::GetWinningOption()
    {
        assert(IsVoting());

        int votes_required = GetNumVotesRequired();

        if (IsYesNoVote())
        {
            if (GetNumVotes(VOTEOPTION_YES) >= votes_required)
            {
                return VOTEOPTION_YES;
            }

            return VOTEOPTION_NO;
        }

        // Find the highest voted option.
        vote_option_t win_option = VOTEOPTION_NONE;
        int num_win_votes = 0;

        for (int i = 0; i < cur_vote_type->GetNumVoteOptions(); i++)
        {
            int num_votes = GetNumVotes((vote_option_t)i);
            if (num_votes > num_win_votes)
            {
                win_option = (vote_option_t)i;
                num_win_votes = num_votes;
            }
        }

        return win_option;
    }

    bool VoteSystem::CastVote(edict_t* player, vote_option_t option)
    {
        if (!IsVoting())
        {
            gi.cprintf(player, PRINT_HIGH, "There is no vote going!\n");
            return false;
        }


        int player_num = player - g_edicts;

        if (!IsParticipant(player))
        {
            gi.cprintf(player, PRINT_HIGH, "You cannot participate in this vote!\n");
            return false;
        }

        // Remove old vote.
        for (auto it = voted.begin(); it != voted.end(); it++)
        {
            if (it->player_index == player_num)
            {
                //gi.cprintf(player, PRINT_HIGH, "You've already voted!\n");
                //return false;

                voted.erase(it);
                break;
            }
        }

        if (!CanCastVote(player, option))
        {
            return false;
        }


        voted.push_back({player_num, option});

        CheckConditions();

        return true;
    }

    void VoteSystem::VoteOption1(edict_t* player, pmenuhnd_t* p)
    {
        VoteSystem::CastVote(player, VOTEOPTION_1);
    }

    void VoteSystem::VoteOption2(edict_t* player, pmenuhnd_t* p)
    {
        VoteSystem::CastVote(player, VOTEOPTION_2);
    }

    void VoteSystem::VoteOption3(edict_t* player, pmenuhnd_t* p)
    {
        VoteSystem::CastVote(player, VOTEOPTION_3);
    }

    void VoteSystem::VoteOption4(edict_t* player, pmenuhnd_t* p)
    {
        VoteSystem::CastVote(player, VOTEOPTION_4);
    }
}
