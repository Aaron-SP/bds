/* Copyright [2013-2018] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the Beyond Dying Skies.

Beyond Dying Skies is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Beyond Dying Skies is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Beyond Dying Skies.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace game
{

class work_item
{
  private:
    std::function<void(const size_t)> _f;
    size_t _begin;
    size_t _length;

  public:
    work_item(const std::function<void(const size_t)> &f, const size_t begin, const size_t length)
        : _f(f), _begin(begin), _length(length) {}

    void work() const
    {
        const size_t end = _begin + _length;
        for (size_t i = _begin; i < end; i++)
        {
            // Do the work for this item
            _f(i);
        }
    }
};

class thread_pool
{
  private:
    unsigned _thread_count;
    std::vector<std::vector<work_item>> _queue;
    std::vector<std::atomic<bool>> _sleep;
    std::vector<std::atomic<bool>> _state;
    std::vector<std::thread> _threads;
    std::mutex _sleep_lock;
    std::condition_variable _more_data;
    std::atomic<bool> _die;
    std::atomic<bool> _turbo;

    inline void notify()
    {
        // If we are sleeping, wait for all threads to sleep before killing queue
        if (!_turbo)
        {
            // Wake up idle threads
            _more_data.notify_all();
        }
    }
    inline void wait_sleep()
    {
        // Wait for all workers to quit
        while (!_turbo)
        {
            // Count threads asleep
            size_t count = 0;
            for (size_t i = 0; i < _thread_count - 1; i++)
            {
                // Make sure all threads are asleep
                std::unique_lock<std::mutex> lock(_sleep_lock);

                // See if worker is finished
                if (_sleep[i] == false)
                {
                    count++;
                }
            }

            // If all workers are finished
            if (count == _thread_count - 1)
            {
                break;
            }
        }
    }
    inline void wait_done() const
    {
        // Wait for all workers to quit
        while (true)
        {
            // Count threads asleep
            size_t count = 0;
            for (size_t i = 0; i < _thread_count - 1; i++)
            {
                // See if worker is finished
                if (_state[i] == false)
                {
                    count++;
                }
            }

            // If all workers are finished
            if (count == _thread_count - 1)
            {
                break;
            }
        }
    }
    inline void wait()
    {
        if (!_turbo)
        {
            wait_sleep();
        }
        else
        {
            wait_done();
        }
    }
    inline void work(const size_t index)
    {
        while (true)
        {
            // Sleep on condition
            if (!_turbo)
            {
                // Acquire mutex to sleep on condition
                std::unique_lock<std::mutex> lock(_sleep_lock);

                // ATOMIC: Signal Sleeping
                _sleep[index] = false;

                // Wait on more data
                _more_data.wait(lock, [this, index]() { return (_state[index] || _die) || _turbo; });
            }

            // Do work
            if (_state[index])
            {
                // Get access to work
                std::vector<work_item> &items = _queue[index];
                const size_t size = items.size();

                // Do all work in this queue
                for (size_t i = 0; i < size; i++)
                {
                    items[i].work();
                }

                // Clear the queue
                items.clear();

                // ATOMIC: Signal Finished
                _state[index] = false;
            }
            else if (_die)
            {
                // Kill thread
                break;
            }
        }
    }

  public:
    thread_pool() : _thread_count(std::thread::hardware_concurrency()), _queue(_thread_count - 1),
                    _sleep(_thread_count - 1), _state(_thread_count - 1), _threads(_thread_count - 1), _die(false), _turbo(false)
    {
        // Error out if can't determine core count
        if (_thread_count < 1)
        {
            throw std::runtime_error("thread_pool: can't determine number of CPU cores");
        }

        // Set all default synchronization flags
        for (size_t i = 0; i < _thread_count - 1; i++)
        {
            _sleep[i] = true;
            _state[i] = false;
        }

        // Boot all threads
        for (size_t i = 0; i < _thread_count - 1; i++)
        {
            // Boot the thread
            _threads[i] = std::thread(&thread_pool::work, this, i);
        }
    }
    ~thread_pool()
    {
        // Kill all threads in pool
        kill();

        // Join all threads
        for (size_t i = 0; i < _thread_count - 1; i++)
        {
            _threads[i].join();
        }
    }
    void kill()
    {
        // Wait for threads to finish work
        wait();

        // Signal dead queue
        _die = true;

        // Notify threads
        notify();
    }
    void sleep()
    {
        // ATOMIC: Set state of all workers to 'sleep'
        for (size_t i = 0; i < _thread_count - 1; i++)
        {
            // Signal finished
            _sleep[i] = true;
        }

        // Put all threads to sleep
        _turbo = false;
    }
    void wake()
    {
        // Wait for all workers to sleep
        wait_sleep();

        // Turn off sleeping
        _turbo = true;

        // Wake up idle threads
        _more_data.notify_all();
    }
    void run(const std::function<void(const size_t)> &f, const size_t start, const size_t stop)
    {
        // Wait for all workers to sleep
        wait();

        // Load queue with work
        const size_t length = (stop - start) / _thread_count;

        size_t begin = start;
        for (size_t i = 0; i < _thread_count - 1; i++)
        {
            // Create work for thread
            _queue[i].emplace_back(f, begin, length);

            // Increment next work item
            begin += length;
        }

        // ATOMIC: Set state of all workers to 'run'
        for (size_t i = 0; i < _thread_count - 1; i++)
        {
            // Signal that there is work to do
            _sleep[i] = true;
            _state[i] = true;
        }

        // Notify threads
        notify();

        // Boot the residual work on this thread
        const size_t remain = stop - begin;
        work_item item(f, begin, remain);
        item.work();

        // Wait for all workers to finish work
        wait_done();
    }
};
}

#endif
