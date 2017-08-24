/* Copyright [2013-2016] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the MGLCraft.

MGLCraft is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MGLCraft is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MGLCraft.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <atomic>
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
    std::vector<std::atomic<bool>> _state;
    std::vector<std::thread> _threads;
    std::atomic<bool> _die;
    bool _launch;

    void work(const size_t index)
    {
        while (true)
        {
            // ATOMIC: Read running state
            const bool ready = _state[index];
            const bool die = _die;

            // Do work
            if (ready)
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

                // ATOMIC: Signal finished
                _state[index] = false;
            }
            else if (die)
            {
                // Kill thread
                break;
            }
        }
    }

  public:
    thread_pool() : _thread_count(std::thread::hardware_concurrency()),
                    _queue(_thread_count - 1), _state(_thread_count - 1), _threads(_thread_count - 1),
                    _die(false), _launch(false)
    {
        // Error out if can't determine core count
        if (_thread_count < 1)
        {
            throw std::runtime_error("thread_pool: can't determine number of CPU cores");
        }
    }
    ~thread_pool()
    {
        if (_launch)
        {
            // Kill all threads in pool
            kill();

            // Join all threads
            for (size_t i = 0; i < _thread_count - 1; i++)
            {
                _threads[i].join();
            }
        }
    }
    void kill()
    {
        _die = true;
    }
    void launch()
    {
        if (!_launch)
        {
            // Boot all threads
            for (size_t i = 0; i < _thread_count - 1; i++)
            {
                // Boot the thread
                _threads[i] = std::thread(&thread_pool::work, this, i);
            }

            // Set launch flag
            _launch = true;
        }
    }
    void run(const std::function<void(const size_t)> &f, const size_t start, const size_t stop)
    {
        // See if we launched the pool
        if (!_launch)
        {
            throw std::runtime_error("thread_pool: trying to run but pool never launched");
        }

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
            // Signal finished
            _state[i] = true;
        }

        // Boot the residual work on this thread
        const size_t remain = stop - begin;
        work_item item(f, begin, remain);
        item.work();

        // Wait for all workers to quit
        while (true)
        {
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
};
}

#endif
