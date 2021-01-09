
#include "jump_types.h"
#include "jump.h"

namespace Jump
{

    store_buffer_t::store_buffer_t() : numStores(0), nextIndex(0), stores()
    {
    }

    void store_buffer_t::PushStore(const store_data_t& data)
    {
        stores[nextIndex] = data;
        nextIndex = (nextIndex + 1) % MAX_STORES;
        if (numStores < MAX_STORES)
        {
            numStores++;
        }
    }

    store_data_t store_buffer_t::GetStore(int prevNum)
    {
        int desired = prevNum;
        if (desired > numStores)
        {
            // If a user asks for a prev that is larger than
            // what we have stored, we return the oldest data.
            desired = numStores;
        }
        int index = nextIndex - desired;
        if (index < 0)
        {
            index = MAX_STORES - abs(nextIndex - desired);
        }
        return stores[index];
    }

    void store_buffer_t::Reset()
    {
        nextIndex = 0;
        numStores = 0;
        memset(stores, 0, sizeof(stores));
    }

    bool store_buffer_t::HasStore()
    {
        return numStores > 0;
    }

    //
    // playertracetouch_guard_t
    //
    playertracetouch_guard_t::playertracetouch_guard_t(const edict_t* ignore_player)
    {
        ents.reserve(globals.num_edicts);
        
        // Collect all entities that this player should not interact with.
        for (int i = 0; i < globals.num_edicts; ++i)
        {
            auto ent = &(g_edicts[i]);

            if (!ent->inuse || ent == ignore_player)
                continue;

            // Already not solid.
            if (ent->solid == SOLID_NOT)
                continue;

            // Must be moving missile (rockets, blaster)
            // or bouncey boys (grenades)
            if (ent->movetype != MOVETYPE_FLYMISSILE && ent->movetype != MOVETYPE_BOUNCE)
                continue;

            // They are the owner, it should not collide with this player.
            if (ignore_player == ent->owner)
                continue;

            ents.push_back({ent, ent->solid});

            ent->solid = SOLID_NOT;
        }
    }

    void playertracetouch_guard_t::Free()
    {
        for (auto& data : ents)
        {
            assert(data.ent->inuse && data.ent->solid == SOLID_NOT);

            data.ent->solid = data.prev_solidtype;
        }

        ents.resize(0);
    }
}