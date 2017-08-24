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
#ifndef __AI_PATH__
#define __AI_PATH__

#include <algorithm>
#include <game/cgrid.h>
#include <game/file.h>
#include <game/path.h>
#include <min/intersect.h>
#include <min/vec3.h>
#include <mml/nneat.h>
#include <mml/vec.h>

namespace game
{

class ai_path
{
  private:
    static constexpr size_t IN = 38;
    static constexpr size_t OUT = 4;
    static constexpr float _step_size = 1.0;
    mml::mapper<float> _map;
    mml::nneat<float, IN, OUT> _net;
    mutable path _path;
    mutable min::vec3<float> _step;
    mutable std::pair<size_t, size_t> _collisions;

    void load(const cgrid &grid, const path_data &data) const
    {
        // Set inputs
        mml::vector<float, IN> in;

        // Update the path
        _path.update(grid, data);

        // Get path data input properties
        const min::vec3<float> avoid = _path.avoid();
        const min::vec3<float> dfs = _path.dfs(grid, data);
        const min::vec3<float> ray = _path.ray_sorted(0);
        const min::vec3<float> &dest = data.get_destination();
        const min::vec3<float> &position = data.get_position();

        // Map DFS search data from [-1, 1] to [0, 1] range
        in[0] = _map.map(dfs.x());
        in[1] = _map.map(dfs.y());
        in[2] = _map.map(dfs.z());

        // Map avoid data from [-1, 1] to [0, 1] range
        in[3] = _map.map(avoid.x());
        in[4] = _map.map(avoid.y());
        in[5] = _map.map(avoid.z());

        // Map ray data from [-1, 1] to [0, 1] range
        in[6] = _map.map(ray.x());
        in[7] = _map.map(ray.y());
        in[8] = _map.map(ray.z());

        // Set the input for distance from destination [0, 1] range
        const float m = (dest - position).magnitude();
        in[9] = std::min(1.0 / m, 1.0);

        // Set input for number of possible colliding cells to [0, 1] range
        _collisions = grid.count_mob_collision_cells(position);
        in[10] = static_cast<float>(_collisions.second) / 27.0;

        // Get the 27 eye rays for inputting into sensor
        const float *const eye_mag = _path.get_eye_mag();

        // Create terrain input encoding
        for (size_t i = 0; i < 27; i++)
        {
            // This is a special case for out of grid, which is infinitely far away
            const float d = (eye_mag[i] < 0.001) ? 1E6 : eye_mag[i];

            // Map data to [0, 1] range
            in[11 + i] = std::min(1.0 / d, 1.0);
        }

        // Set input
        _net.set_input(in);
    }
    min::vec3<float> unload(const mml::vector<float, OUT> &output) const
    {
        // Unmap values
        const float x = _map.unmap(output[0]);
        const float y = _map.unmap(output[1]);
        const float z = _map.unmap(output[2]);
        const float step = output[3] * 2.0;

        // Calculate direction to move
        return min::vec3<float>(x, y, z) * step;
    }

  public:
    ai_path() : _map(-1.0, 1.0)
    {
        // 1/13 topology, 1/101 add_node, 1/113 remove_node, 1/23 scramble; else add_conn
        _net.set_topology_constants(101, 11, 5, 2);
        _net.set_connection_limit(864);
        _net.set_node_limit(74);
    }
    ai_path(const std::vector<uint8_t> &stream) : _map(-1.0, 1.0)
    {
        this->deserialize(stream);

        // 1/13 topology, 1/101 add_node, 1/113 remove_node, 1/23 scramble; else add_conn
        _net.set_topology_constants(101, 11, 5, 2);
        _net.set_connection_limit(864);
        _net.set_node_limit(74);
    }
    static inline ai_path breed(const ai_path &p1, const ai_path &p2)
    {
        // Create new path
        ai_path out;
        out._net = mml::nneat<float, IN, OUT>::breed(p1._net, p2._net);

        // return path
        return out;
    }
    inline min::vec3<float> &calculate(const cgrid &grid, const path_data &data) const
    {
        // Load neural net
        load(grid, data);

        // Calculate output
        const mml::vector<float, OUT> out = _net.calculate();

        // Unload output NOT NORMALIZED
        _step = unload(out);

        // return step
        return _step;
    }
    inline void deserialize(const std::vector<uint8_t> &stream)
    {
        // read data from stream into float format
        size_t next = 0;
        const std::vector<float> data = min::read_le_vector<float>(stream, next);

        // Deserialize the net
        _net.deserialize(data);
    }
    void debug() const
    {
        // Print detailed net information
        _net.debug_connections();

        // Print basic stuff
        std::cout << "Connection count: " << _net.get_connections() << std::endl;
        std::cout << "Node size: " << _net.get_nodes() << std::endl;
    }
    inline float fitness(const cgrid &grid, path_data &p_data) const
    {
        float score = 0.0;

        // Get new travel direction, THIS IS NOT NORMALIZED
        const min::vec3<float> &step_dir = calculate(grid, p_data);

        // Calculate distance to starting point, max step is sqrt(3*2*2)
        const min::vec3<float> next = p_data.step(step_dir, _step_size);

        // Check if we picked a bad direction and return start position
        const int8_t atlas = grid.grid_value(next);
        if (atlas != -1)
        {
            // Punish running into a wall
            score -= 11.0;
        }
        else
        {
            // Move to next point
            p_data.update(next);

            // Punish zero moves, always greater than zero
            const float travel_step = p_data.get_travel_step();
            if (travel_step < 0.15)
            {
                score -= 0.5;
            }
            else
            {
                // Reward based on traveling distance along direction vector
                const float angle_step = p_data.get_angle_step();

                // Scale by (1.0 / sqrt(3*2*2)), reward lvl 3
                score += angle_step * 0.2886;
            }
        }

        // Reward getting to goal
        const float remain = p_data.get_remain();
        if (remain >= 1.0)
        {
            // reward lvl 5
            score += 100.0 / remain;
        }
        else
        {
            score += 100.0;
        }

        // Reward being in populated areas with no collisions
        const size_t count = _collisions.first;
        if (count == 0)
        {
            // Scale by 1 / 27, reward lvl 2
            score += _collisions.second * 0.03703;
        }
        else
        {
            // Punish normalized to probability of collision
            const float surround = _collisions.second + 1;
            score -= count / surround;
        }

        // return fitness score
        return score;
    }
    inline const path &get_path() const
    {
        return _path;
    }
    inline void mutate(mml::net_rng<float> &rng)
    {
        _net.mutate(rng);
    }
    inline const min::vec3<float> &step() const
    {
        // THIS IS NOT NORMALIZED
        return _step;
    }
    inline void randomize(mml::net_rng<float> &rng)
    {
        _net.randomize(rng);
    }
    inline void serialize(std::vector<uint8_t> &stream) const
    {
        const std::vector<float> data = _net.serialize();

        // Write data into stream
        min::write_le_vector<float>(stream, data);
    }
};
}

#endif
