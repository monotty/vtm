// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_QUEUE_HPP
#define NETXS_QUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <deque>

namespace netxs
{
    template <class T>
    class mt_queue
    {
        std::mutex              d_mutex;
        std::condition_variable d_condition;
        std::deque<T>           d_queue;

    public:
        void push(T const& value)
        {
            auto guard = std::lock_guard{ d_mutex };
            d_queue.push_front(value);
            d_condition.notify_one();
        }
        T pop()
        {
            auto lock = std::unique_lock{ d_mutex };
            d_condition.wait(lock, [this] { return !d_queue.empty(); });
            T rc(std::move(d_queue.back()));
            d_queue.pop_back();
            return rc;
        }
        bool try_pop(T& v, std::chrono::milliseconds timeout)
        {
            auto lock = std::unique_lock{ d_mutex };
            if (!d_condition.wait_for(lock, timeout, [this] { return !d_queue.empty(); }))
            {
                return !true;
            }
            v = d_queue.back();
            d_queue.pop_back();
            return true;
        }
        size_t size()
        {
            auto guard = std::lock_guard{ d_mutex };
            return d_queue.size();
        }
        void clear()
        {
            auto guard = std::lock_guard{ d_mutex };
            d_queue.clear();
        }

        mt_queue() { }
        mt_queue(mt_queue<T>&& x)
        {
            auto guard = std::lock_guard{ d_mutex };
            d_queue = std::move(x.d_queue);
        }
        mt_queue<T>& operator=(mt_queue<T>&& x)
        {
            //todo *this is not a MT safe (only x.d_mutex is locked)
            auto guard = std::lock_guard{ d_mutex };
            d_queue = std::move(x.d_queue);
            return *this;
        }
    };
}

#endif // NETXS_QUEUE_HPP