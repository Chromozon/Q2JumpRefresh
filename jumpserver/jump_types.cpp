
#include "jump_types.h"
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
            replay_frame_t temp;
            size_t framesize =
                sizeof(temp.pos) +
                sizeof(temp.angles) +
                sizeof(temp.key_states) +
                sizeof(temp.fps) +
                sizeof(temp.reserved1) +
                sizeof(temp.reserved2);
            size_t num_bytes = replay_buffer.size() * framesize;
            bytes.resize(num_bytes);
            for (size_t i = 0; i < replay_buffer.size(); ++i)
            {
                int offset = 0;
                uint8_t* start = &bytes[i * framesize];
                const replay_frame_t& frame = replay_buffer[i];

                memcpy(start + offset, &frame.pos, sizeof(frame.pos));
                offset += sizeof(frame.pos);

                memcpy(start + offset, &frame.angles, sizeof(frame.angles));
                offset += sizeof(frame.angles);

                memcpy(start + offset, &frame.key_states, sizeof(frame.key_states));
                offset += sizeof(frame.key_states);

                memcpy(start + offset, &frame.fps, sizeof(frame.fps));
                offset += sizeof(frame.fps);

                memcpy(start + offset, &frame.reserved1, sizeof(frame.reserved1));
                offset += sizeof(frame.reserved1);

                memcpy(start + offset, &frame.reserved2, sizeof(frame.reserved2));
            }
        }
        return bytes;
    }

    // Converts a byte array into a replay buffer
    std::vector<replay_frame_t> DeserializeReplayBuffer(const std::vector<uint8_t>& bytes)
    {
        std::vector<replay_frame_t> replay_buffer;
        if (bytes.size() > 0)
        {
            replay_frame_t temp;
            size_t framesize =
                sizeof(temp.pos) +
                sizeof(temp.angles) +
                sizeof(temp.key_states) +
                sizeof(temp.fps) +
                sizeof(temp.reserved1) +
                sizeof(temp.reserved2);
            size_t num_frames = bytes.size() / framesize;
            replay_buffer.resize(num_frames);
            for (size_t i = 0; i < num_frames; ++i)
            {
                const uint8_t* start = &bytes[i * framesize];
                replay_frame_t& frame = replay_buffer[i];
                int offset = 0;

                memcpy(&frame.pos, start + offset, sizeof(frame.pos));
                offset += sizeof(frame.pos);

                memcpy(&frame.angles, start + offset, sizeof(frame.angles));
                offset += sizeof(frame.angles);

                memcpy(&frame.key_states, start + offset, sizeof(frame.key_states));
                offset += sizeof(frame.key_states);

                memcpy(&frame.fps, start + offset, sizeof(frame.fps));
                offset += sizeof(frame.fps);

                memcpy(&frame.reserved1, start + offset, sizeof(frame.reserved1));
                offset += sizeof(frame.reserved1);

                memcpy(&frame.reserved2, start + offset, sizeof(frame.reserved2));
            }
        }
        return replay_buffer;
    }
}