#pragma once

namespace Jump
{
    typedef enum
    {
        VOTETYPE_NONE = 0,

        // Vote to change the map now!
        VOTETYPE_MAPCHANGE,

        // Nominate a map to the ending vote screen.
        VOTETYPE_NOMINATE,

        // Extend or subtract time from the current map's timelimit.
        VOTETYPE_VOTETIME,

        // Choose from 3 maps and extend option. (uses PMenu)
        VOTETYPE_MAPEND,

        // Silence (disallow chatting) a specific player.
        VOTETYPE_SILENCE,

        // Kick a specific player.
        VOTETYPE_KICK,
    } vote_type_t;


    typedef enum
    {
        VOTECAST_YESNO = 0,

        //VOTECAST_NUMBER_CHAT,

        VOTECAST_PMENU,
    } vote_cast_type_t;

    typedef enum
    {
        // Allow more than a simple yes/no votes

        VOTEOPTION_NONE = -1,

        VOTEOPTION_1,
        VOTEOPTION_YES = VOTEOPTION_1,
        
        VOTEOPTION_2,
        VOTEOPTION_NO = VOTEOPTION_2,

        VOTEOPTION_3,

        VOTEOPTION_4,

        VOTEOPTION_MAX

    } vote_option_t;

    struct voted_t
    {
        int player_index;
        vote_option_t option;
    };

    //
    // All vote types inherit from this.
    //
    class BaseVoteType
    {
    public:
        virtual void ApplyVote(vote_option_t winning_option) = 0;

        // Printed on the hud.
        virtual std::string GetShortDescription() const = 0;

        // Printed in the console.
        virtual std::string GetDescription() const = 0;

        // Return the percentage of participants voting yes for this vote to pass.
        virtual float       GetYesPercentage() const;


        virtual void        OnStartVote(edict_t* caster);

        // Returns whether this particular player can start the vote.
        virtual bool        CanStartVote(edict_t* caster) const { return true; }

        // Returns true if arguments are valid.
        // Vote should not take place otherwise.
        virtual bool        ParseArguments(edict_t* caster, const std::string& arguments);
        
        virtual bool        CanParticipate(edict_t* player) const { return true; }
        
        // Returns whether this particular player can cast this vote option.
        virtual bool        CanCastVote(edict_t* player, vote_option_t option) const;

        // Returns vote time in seconds.
        virtual float       GetVoteTime() const;

        // Returns number of options in the vote. A typical yes/no vote should have 2.
        virtual int         GetNumVoteOptions() const;

        virtual void        CreatePMenu(std::vector<pmenu_t>& menu) {}

        // Returns a bitfield of all the entries that were changed.
        virtual uint32_t    UpdatePMenu(std::vector<pmenu_t>& menu) { return 0; }


        vote_type_t         GetVoteType() const { return vote_type; }
        vote_cast_type_t    GetCastType() const { return vote_cast; }

    protected:
        BaseVoteType(vote_type_t type, vote_cast_type_t vote_cast);
        virtual ~BaseVoteType();

    private:
        vote_type_t vote_type;
        vote_cast_type_t vote_cast;
    };

    //
    // A vote type to target a single player or multiple players.
    //
    class TargetPlayerVoteType : public BaseVoteType
    {
    public:
        virtual bool ParseArguments(edict_t* caster, const std::string& arguments) override;

    protected:
        TargetPlayerVoteType(vote_type_t type, vote_cast_type_t vote_cast);


        struct target_t
        {
            int player_index;
            std::string player_name;
        };

        // Returns the array of target player indices.
        const std::vector<target_t>& GetTargets() const { return targets; }

        void AddTarget(edict_t* player);

    private:

        std::vector<target_t> targets;
    };

    class VoteSystem
    {
    public:
        static void Init();

        static bool IsVoting();

        // Returns number of seconds left in the vote.
        static float GetTimeleft();

        // A simple yes/no vote will always display on the hud.
        static bool IsYesNoVote();
        static bool IsHudVote();

        static int GetNumVotes(vote_option_t option);
        static int GetNumVotesRequired();

        static bool HasVoted(int player_num);
        static bool HasVoted(const edict_t* player);

        static bool AttemptStartVote(edict_t* player, vote_type_t vote_type, const std::string& arguments);
        
        static bool CastVote(edict_t* player, vote_option_t option);
        static bool CanCastVote(edict_t* player, vote_option_t option);

        static bool IsParticipant(edict_t* player);
        static bool RemoveParticipant(edict_t* player);

        static void OnFrame();

        // PMenu callbacks.
        static void VoteOption1(edict_t* player, pmenuhnd_t* p);
        static void VoteOption2(edict_t* player, pmenuhnd_t* p);
        static void VoteOption3(edict_t* player, pmenuhnd_t* p);
        static void VoteOption4(edict_t* player, pmenuhnd_t* p);

    private:
        static BaseVoteType* GetVoteType(vote_type_t vote_type);

        static void StartVote(edict_t* player, vote_type_t vote_type);
        static void FinishVote(bool force_apply = false);

        static vote_option_t GetWinningOption();
        static bool CanVotePass();

        static void UpdateHudVote();
        static void UpdatePMenu();

        // Check if the vote should end now.
        static void CheckConditions();
    };
}
