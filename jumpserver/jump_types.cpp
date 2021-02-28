
#include "jump_types.h"
#include "jump_logger.h"
#include <iterator>

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

    // Converts a replay buffer into a byte array
    std::vector<uint8_t> SerializeReplayBuffer(const std::vector<replay_frame_t>& replay_buffer)
    {
        std::vector<uint8_t> bytes;
        if (replay_buffer.size() > 0)
        {
            size_t num_bytes = replay_buffer.size() * sizeof(replay_frame_t);
            bytes.resize(num_bytes);
            memcpy(&bytes[0], &replay_buffer[0], num_bytes);
        }
        return bytes;
    }

    // Converts a byte array into a replay buffer
    std::vector<replay_frame_t> DeserializeReplayBuffer(const std::vector<uint8_t>& bytes)
    {
        std::vector<replay_frame_t> replay_buffer;
        if (bytes.size() % sizeof(replay_frame_t) != 0)
        {
            Logger::Error("Invalid size of serialized replay buffer");
            return replay_buffer;
        }
        if (bytes.size() > 0)
        {
            replay_buffer.resize(bytes.size() / sizeof(replay_frame_t));
            memcpy(&replay_buffer[0], &bytes[0], bytes.size());
        }
        return replay_buffer;
    }
}