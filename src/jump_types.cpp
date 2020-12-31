
#include "jump_types.h"

namespace Jump
{
    server_data_t jump_server;

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

    // Constructor
    client_data_t::client_data_t()
    {
        replay_recording.reserve(10000);
        replay_spectating_framenum = 0;
        update_replay_spectating = false;
        fps = 0;
        team = TEAM_SPECTATOR;
        timer_begin = 0;
        timer_end = 0;
        timer_paused = true;
        timer_finished = false;
        store_ent = NULL;
        key_states = 0;
    }
}