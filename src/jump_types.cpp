
#include "jump_types.h"

namespace Jump
{
    StoreBuffer::StoreBuffer() : numStores(0), nextIndex(0), stores()
    {
    }

    void StoreBuffer::PushStore(const store_data_t& data)
    {
        stores[nextIndex] = data;
        nextIndex = (nextIndex + 1) % MAX_STORES;
        if (numStores < MAX_STORES)
        {
            numStores++;
        }
    }

    store_data_t StoreBuffer::GetStore(int prevNum)
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

    void StoreBuffer::Reset()
    {
        nextIndex = 0;
        numStores = 0;
        memset(stores, 0, sizeof(stores));
    }

    bool StoreBuffer::HasStore()
    {
        return numStores > 0;
    }
}