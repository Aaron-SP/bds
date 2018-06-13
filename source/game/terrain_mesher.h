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
#ifndef __TERRAIN_MESHER__
#define __TERRAIN_MESHER__

#include <game/callback.h>
#include <game/geometry.h>
#include <game/work_queue.h>
#include <min/mesh.h>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace game
{

class terrain_mesher
{
  private:
    min::mesh<float, uint32_t> _mesh;

    inline void allocate_mesh_buffer(const std::vector<min::vec4<float>> &cell_buffer)
    {
        // Resize the mesh from cell size
        const size_t size = cell_buffer.size();

        // Vertex sizes
        const size_t size6 = size * 6;
        _mesh.vertex.resize(size6);
        _mesh.uv.resize(size6);
        _mesh.normal.resize(size6);
    }
    static inline min::aabbox<float, min::vec3> create_box(const min::vec3<float> &center)
    {
        // Create box at center
        const min::vec3<float> min = center - min::vec3<float>(0.5, 0.5, 0.5);
        const min::vec3<float> max = center + min::vec3<float>(0.5, 0.5, 0.5);

        // return the box
        return min::aabbox<float, min::vec3>(min, max);
    }
    inline void reserve_memory(const size_t chunk_size)
    {
        // Reserve maximum number of cells in a chunk
        const size_t cells = chunk_size * chunk_size * chunk_size;

        // Reserve 6 faces per cell maximum
        const size_t max_vertex = cells * 36;

        // Reserve maximum size of chunk
        _mesh.vertex.reserve(max_vertex);
        _mesh.uv.reserve(max_vertex);
        _mesh.normal.reserve(max_vertex);
    }
    inline void set_face(const size_t cell, std::vector<min::vec4<float>> &cell_buffer)
    {
        // Unpack the point and the atlas
        const min::vec4<float> &unpack = cell_buffer[cell];

        // Calculate vertex start position
        const size_t vertex_start = cell * 6;

        // Create bounding box of cell and get box dimensions
        const min::vec3<float> p = min::vec3<float>(unpack.x(), unpack.y(), unpack.z());
        const min::aabbox<float, min::vec3> b = create_box(p);
        const min::vec3<float> &min = b.get_min();
        const min::vec3<float> &max = b.get_max();

        // Extract the face type and atlas
        const int_fast8_t face_type = static_cast<int>(unpack.w()) / 255;
        const int_fast8_t atlas_id = static_cast<int>(unpack.w()) % 255;

        // Calculate face vertices
        face_vertex(_mesh.vertex, vertex_start, min, max, face_type);

        // Calculate face uv's
        face_uv(_mesh.uv, vertex_start, face_type);

        // Scale uv's based off atlas id
        face_uv_scale(_mesh.uv, vertex_start, atlas_id);

        // Calculate face normals
        face_normal(_mesh.normal, vertex_start, face_type);
    }

  public:
    terrain_mesher(const size_t chunk_size) : _mesh("mesh")
    {
        // Reserve memory based on chunk scale
        reserve_memory(chunk_size);
    }
    inline void create_faces(min::mesh<float, uint32_t> &child)
    {
        // Convert faces to mesh in parallel
        const size_t size = child.vertex.size();
        if (size > 0)
        {
            // Reserve space in mesh
            allocate_mesh_buffer(child.vertex);

            // Parallelize on generating faces
            const auto work = [this, &child](std::mt19937 &gen, const size_t i) {
                set_face(i, child.vertex);
            };

            // Convert faces to mesh in parallel
            work_queue::worker.run(std::cref(work), 0, size);
        }
    }
    template <typename GB>
    inline void generate_chunk_faces(
        min::mesh<float, uint32_t> &mesh, const min::vec3<float> &p,
        const std::tuple<size_t, size_t, size_t> &index,
        const std::tuple<size_t, size_t, size_t> &edge, const GB &get_block, const float float_atlas) const
    {

        const size_t ix = std::get<0>(index);
        const size_t iy = std::get<1>(index);
        const size_t iz = std::get<2>(index);

        // Generate X Faces
        const bool on_edge_nx = ix == 0;
        const bool on_edge_px = ix == std::get<0>(edge);
        if (!on_edge_nx)
        {
            // Unsafely check if cell is within the grid
            const block_id x1 = get_block(std::make_tuple(ix - 1, iy, iz));
            if (x1 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 0.1));
            }
        }
        if (!on_edge_px)
        {
            const block_id x2 = get_block(std::make_tuple(ix + 1, iy, iz));
            if (x2 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 255.1));
            }
        }

        // Generate Y Faces
        const bool on_edge_ny = iy == 0;
        const bool on_edge_py = iy == std::get<1>(edge);
        if (!on_edge_ny)
        {
            // Unsafely check if cell is within the grid
            const block_id y1 = get_block(std::make_tuple(ix, iy - 1, iz));
            if (y1 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 510.1));
            }
        }
        if (!on_edge_py)
        {
            const block_id y2 = get_block(std::make_tuple(ix, iy + 1, iz));
            if (y2 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 765.1));
            }
        }

        // Generate Z Faces
        const bool on_edge_nz = iz == 0;
        const bool on_edge_pz = iz == std::get<2>(edge);
        if (!on_edge_nz)
        {
            // Unsafely check if cell is within the grid
            const block_id z1 = get_block(std::make_tuple(ix, iy, iz - 1));
            if (z1 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1020.1));
            }
        }
        if (!on_edge_pz)
        {
            const block_id z2 = get_block(std::make_tuple(ix, iy, iz + 1));
            if (z2 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1275.1));
            }
        }
    }
    inline void generate_place_faces_rotated(
        min::mesh<float, uint32_t> &mesh, const min::vec3<float> &p, const min::vec3<int> &offset,
        const std::tuple<size_t, size_t, size_t> &index,
        const std::tuple<size_t, size_t, size_t> &edge, const float float_atlas) const
    {
        const size_t ix = std::get<0>(index);
        const size_t iy = std::get<1>(index);
        const size_t iz = std::get<2>(index);

        // Generate X faces on edges accounting for offset rotation
        const bool on_edge_nx = ix == 0;
        const bool on_edge_px = ix == std::get<0>(edge);
        if ((on_edge_nx && offset.x() > 0) || (on_edge_px && offset.x() < 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 0.1));
        }
        if ((on_edge_nx && offset.x() < 0) || (on_edge_px && offset.x() > 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 255.1));
        }

        // Generate Y faces on edges accounting for offset rotation
        const bool on_edge_ny = iy == 0;
        const bool on_edge_py = iy == std::get<1>(edge);
        if ((on_edge_ny && offset.y() > 0) || (on_edge_py && offset.y() < 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 510.1));
        }
        if ((on_edge_ny && offset.y() < 0) || (on_edge_py && offset.y() > 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 765.1));
        }

        // Generate Z faces on edges accounting for offset rotation
        const bool on_edge_nz = iz == 0;
        const bool on_edge_pz = iz == std::get<2>(edge);
        if ((on_edge_nz && offset.z() > 0) || (on_edge_pz && offset.z() < 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1020.1));
        }
        if ((on_edge_nz && offset.z() < 0) || (on_edge_pz && offset.z() > 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1275.1));
        }
    }
    template <typename GB>
    inline void generate_chunk_faces_rotated(
        min::mesh<float, uint32_t> &mesh, const min::vec3<float> &p, const min::vec3<int> &offset,
        const std::tuple<size_t, size_t, size_t> &index,
        const std::tuple<size_t, size_t, size_t> &edge, const GB &get_block, const float float_atlas) const
    {
        const size_t ix = std::get<0>(index);
        const size_t iy = std::get<1>(index);
        const size_t iz = std::get<2>(index);

        // Generate X Faces
        const bool on_edge_nx = ix == 0;
        const bool on_edge_px = ix == std::get<0>(edge);
        if (!on_edge_nx && !on_edge_px)
        {
            // Unsafely check if cell is within the grid
            const block_id x1 = get_block(std::make_tuple(ix - 1, iy, iz));
            if (x1 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 0.1));
            }
        }
        else if ((on_edge_nx && offset.x() > 0) || (on_edge_px && offset.x() < 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 0.1));
        }
        if (!on_edge_nx && !on_edge_px)
        {
            const block_id x2 = get_block(std::make_tuple(ix + 1, iy, iz));
            if (x2 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 255.1));
            }
        }
        else if ((on_edge_nx && offset.x() < 0) || (on_edge_px && offset.x() > 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 255.1));
        }

        // Generate Y Faces
        const bool on_edge_ny = iy == 0;
        const bool on_edge_py = iy == std::get<1>(edge);
        if (!on_edge_ny && !on_edge_py)
        {
            // Unsafely check if cell is within the grid
            const block_id y1 = get_block(std::make_tuple(ix, iy - 1, iz));
            if (y1 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 510.1));
            }
        }
        else if ((on_edge_ny && offset.y() > 0) || (on_edge_py && offset.y() < 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 510.1));
        }
        if (!on_edge_ny && !on_edge_py)
        {
            const block_id y2 = get_block(std::make_tuple(ix, iy + 1, iz));
            if (y2 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 765.1));
            }
        }
        else if ((on_edge_ny && offset.y() < 0) || (on_edge_py && offset.y() > 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 765.1));
        }

        // Generate Z Faces
        const bool on_edge_nz = iz == 0;
        const bool on_edge_pz = iz == std::get<2>(edge);
        if (!on_edge_nz && !on_edge_pz)
        {
            // Unsafely check if cell is within the grid
            const block_id z1 = get_block(std::make_tuple(ix, iy, iz - 1));
            if (z1 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1020.1));
            }
        }
        else if ((on_edge_nz && offset.z() > 0) || (on_edge_pz && offset.z() < 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1020.1));
        }
        if (!on_edge_nz && !on_edge_pz)
        {
            const block_id z2 = get_block(std::make_tuple(ix, iy, iz + 1));
            if (z2 == block_id::EMPTY)
            {
                mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1275.1));
            }
        }
        else if ((on_edge_nz && offset.z() < 0) || (on_edge_pz && offset.z() > 0))
        {
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1275.1));
        }
    }
    const min::mesh<float, uint32_t> &get_mesh() const
    {
        return _mesh;
    }
};
}

#endif
