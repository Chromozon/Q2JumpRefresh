#pragma once

#include <atomic>
#include <mutex>
#include <condition_variable>
#include "thread_queue.h"

namespace Jump
{
    extern thread_queue<int> g_addtime_queue;
    extern thread_queue<int> g_query_queue;
}