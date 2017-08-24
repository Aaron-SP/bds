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
#ifndef __THREAD_MAP__
#define __THREAD_MAP__

#include <functional>
#include <thread>
#include <vector>

namespace game
{

class thread_map
{
  private:
    std::vector<std::thread> _threads;
    unsigned _thread_count;
    static void work(const std::function<void(const size_t)> &f, const size_t begin, const size_t length)
    {
        const size_t end = begin + length;
        for (size_t i = begin; i < end; i++)
        {
            // Do the work for this item
            f(i);
        }
    }

  public:
    thread_map() : _thread_count(std::thread::hardware_concurrency())
    {
        // Error out if can't determine core count
        if (_thread_count < 1)
        {
            throw std::runtime_error("thread_map: can't determine number of CPU cores");
        }

        // Resize the thread pool
        _threads.resize(_thread_count - 1);
    }
    void run(const std::function<void(const size_t)> &f, const size_t start, const size_t stop)
    {
        // The amount of work per thread
        const size_t length = (stop - start) / _thread_count;

        size_t begin = start;
        for (size_t i = 0; i < _thread_count - 1; i++)
        {
            // Boot the thread
            _threads[i] = std::thread(&thread_map::work, f, begin, length);

            // Increment next work item
            begin += length;
        }

        // Boot the residual work on this thread
        const size_t remain = stop - begin;
        thread_map::work(f, begin, remain);

        // Join all threads
        for (size_t i = 0; i < _thread_count - 1; i++)
        {
            _threads[i].join();
        }
    }
};
}

#endif
