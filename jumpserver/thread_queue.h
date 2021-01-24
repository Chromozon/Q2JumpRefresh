#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace Jump
{
    template<typename T>
    class thread_queue
    {
    public:
        // Constructor
        thread_queue() : m_stop_thread(false)
        {
        }

        // Pushes an item to the queue and notifies anyone waiting for items
        void push(T const& data)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.push(data);
            lock.unlock();
            m_condition_variable.notify_one();
        }

        // Returns true if the queue is empty
        bool empty() const
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_queue.empty();
        }

        // If there is an item in the queue, pops and returns true, otherwise returns false.
        bool try_pop(T& popped_value)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_queue.empty())
            {
                return false;
            }
            popped_value = m_queue.front();
            m_queue.pop();
            return true;
        }

        // Waits for an item to be added to the queue or for thread stop signal.  Returns false if thread stopped.
        bool wait_and_pop(T& popped_value)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (m_queue.empty())
            {
                // Unlocks the mutex and waits until condition variable is signaled
                m_condition_variable.wait(lock);
            }
            if (m_stop_thread)
            {
                return false;
            }
            popped_value = m_queue.front();
            m_queue.pop();
            return true;
        }

        // Waits for an item to be added to the queue or for thread stop signal.  Returns false if thread stopped.
        bool wait_and_front(T& front_value)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (m_queue.empty())
            {
                // Unlocks the mutex and waits until condition variable is signaled
                m_condition_variable.wait(lock);
            }
            if (m_stop_thread)
            {
                return false;
            }
            front_value = m_queue.front();
            return true;
        }

        // Call this function to signal that the thread should be stopped.
        void stop_thread()
        {
            m_stop_thread = true;
            m_condition_variable.notify_one();
        }

    private:
        std::queue<T> m_queue;
        std::mutex m_mutex;
        std::condition_variable m_condition_variable;
        bool m_stop_thread;
    };
}