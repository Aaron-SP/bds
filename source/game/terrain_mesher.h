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
#ifndef _BDS_TERRAIN_MESHER_BDS_
#define _BDS_TERRAIN_MESHER_BDS_

#include <game/def.h>
#include <game/geometry.h>
#include <game/id.h>
#include <game/work_queue.h>
#include <min/mesh.h>
#include <min/tri.h>
#include <min/vec3.h>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace game
{

class terrain_mesher
{
  private:
    mutable std::vector<min::vec4<float>> _cells;

    inline void allocate_mesh_vbo(min::mesh<float, uint32_t> &mesh) const
    {
        // Resize the mesh from cell size
        const size_t cell_size = _cells.size() * 6;

        // Vertex sizes
        const size_t size = cell_size;
        mesh.vertex.resize(size);
        mesh.uv.resize(size);
        mesh.normal.resize(size);
    }
    static inline min::aabbox<float, min::vec3> create_box(const min::vec3<float> &center)
    {
        // Create box at center
        const min::vec3<float> min = center - min::vec3<float>(0.5, 0.5, 0.5);
        const min::vec3<float> max = center + min::vec3<float>(0.5, 0.5, 0.5);

        // return the box
        return min::aabbox<float, min::vec3>(min, max);
    }
    inline void generate_chunk_gs(min::mesh<float, uint32_t> &mesh) const
    {
        // Copy vertices into mesh
        const size_t size = _cells.size();
        mesh.vertex.resize(size);
        for (size_t i = 0; i < size; i++)
        {
            mesh.vertex[i] = _cells[i];
        }
    }
    inline void generate_preview_gs(min::mesh<float, uint32_t> &mesh) const
    {
        // Copy vertices into mesh
        const size_t size = _cells.size();
        mesh.vertex.resize(size);
        for (size_t i = 0; i < size; i++)
        {
            mesh.vertex[i] = _cells[i];
        }
    }
    inline void generate_chunk_vbo(min::mesh<float, uint32_t> &mesh) const
    {
        // Convert faces to mesh in parallel
        const size_t cell_size = _cells.size();
        if (cell_size > 0)
        {
            // Reserve space in parent mesh
            allocate_mesh_vbo(mesh);

            // Parallelize on generating faces
            const auto work = [this, &mesh](std::mt19937 &gen, const size_t i) {
                set_face(i, mesh);
            };

            // Convert faces to mesh in parallel
            work_queue::worker.run(std::cref(work), 0, cell_size);
        }
    }
    inline void generate_preview_vbo(min::mesh<float, uint32_t> &mesh) const
    {
        // Only add if contains faces
        const size_t size = _cells.size();
        if (size > 0)
        {
            // Reserve space in parent mesh
            allocate_mesh_vbo(mesh);

            // Convert faces to mesh
            for (size_t i = 0; i < size; i++)
            {
                set_face(i, mesh);
            }
        }
    }
    inline void reserve_memory(const size_t chunk_size) const
    {
        // Reserve maximum number of cells in a chunk
        const size_t cells = chunk_size * chunk_size * chunk_size;
        _cells.reserve(cells);
    }
    inline void set_face(const size_t index, min::mesh<float, uint32_t> &mesh) const
    {
        // Unpack the point and the atlas
        const min::vec4<float> &unpack = _cells[index];

        // Calculate vertex start position
        const size_t vertex_start = index * 6;

        // Create bounding box of face and get box dimensions
        const min::vec3<float> p = min::vec3<float>(unpack.x(), unpack.y(), unpack.z());
        const min::aabbox<float, min::vec3> b = create_box(p);
        const min::vec3<float> &min = b.get_min();
        const min::vec3<float> &max = b.get_max();

        // Extract the face type and atlas
        const int_fast8_t face_type = static_cast<int>(unpack.w()) / 255;
        const int_fast8_t atlas_id = static_cast<int>(unpack.w()) % 255;

        // Calculate face vertices
        face_vertex(mesh.vertex, vertex_start, min, max, face_type);

        // Calculate face uv's
        face_uv(mesh.uv, vertex_start, face_type, atlas_id);

        // Calculate face normals
        face_normal(mesh.normal, vertex_start, face_type);
    }

  public:
    terrain_mesher(const size_t chunk_size)
    {
        reserve_memory(chunk_size);
    }
    inline void clear() const
    {
        _cells.clear();
    }
    template <typename GB>
    inline void generate_chunk_faces(
        const min::vec3<float> &p,
        const min::tri<size_t> &index,
        const min::tri<size_t> &edge, const GB &get_block, const float float_atlas) const
    {
        const size_t ix = index.x();
        const size_t iy = index.y();
        const size_t iz = index.z();

        // Generate X Faces
        const bool on_edge_nx = ix == 0;
        const bool on_edge_px = ix == edge.x();
        if (!on_edge_nx)
        {
            // Unsafely check if cell is within the grid
            const block_id x1 = get_block(min::tri<size_t>(ix - 1, iy, iz));
            if (x1 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 0.1));
            }
        }
        if (!on_edge_px)
        {
            const block_id x2 = get_block(min::tri<size_t>(ix + 1, iy, iz));
            if (x2 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 255.1));
            }
        }

        // Generate Y Faces
        const bool on_edge_ny = iy == 0;
        const bool on_edge_py = iy == edge.y();
        if (!on_edge_ny)
        {
            // Unsafely check if cell is within the grid
            const block_id y1 = get_block(min::tri<size_t>(ix, iy - 1, iz));
            if (y1 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 510.1));
            }
        }
        if (!on_edge_py)
        {
            const block_id y2 = get_block(min::tri<size_t>(ix, iy + 1, iz));
            if (y2 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 765.1));
            }
        }

        // Generate Z Faces
        const bool on_edge_nz = iz == 0;
        const bool on_edge_pz = iz == edge.z();
        if (!on_edge_nz)
        {
            // Unsafely check if cell is within the grid
            const block_id z1 = get_block(min::tri<size_t>(ix, iy, iz - 1));
            if (z1 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1020.1));
            }
        }
        if (!on_edge_pz)
        {
            const block_id z2 = get_block(min::tri<size_t>(ix, iy, iz + 1));
            if (z2 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1275.1));
            }
        }
    }
    inline void generate_place_faces_rotated(
        const min::vec3<float> &p, const min::tri<int> &offset,
        const min::tri<size_t> &index,
        const min::tri<size_t> &edge, const float float_atlas) const
    {
        const size_t ix = index.x();
        const size_t iy = index.y();
        const size_t iz = index.z();

        // Generate X faces on edges accounting for offset rotation
        const bool on_edge_nx = ix == 0;
        const bool on_edge_px = ix == edge.x();
        if ((on_edge_nx && offset.x() > 0) || (on_edge_px && offset.x() < 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 0.1));
        }
        if ((on_edge_nx && offset.x() < 0) || (on_edge_px && offset.x() > 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 255.1));
        }

        // Generate Y faces on edges accounting for offset rotation
        const bool on_edge_ny = iy == 0;
        const bool on_edge_py = iy == edge.y();
        if ((on_edge_ny && offset.y() > 0) || (on_edge_py && offset.y() < 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 510.1));
        }
        if ((on_edge_ny && offset.y() < 0) || (on_edge_py && offset.y() > 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 765.1));
        }

        // Generate Z faces on edges accounting for offset rotation
        const bool on_edge_nz = iz == 0;
        const bool on_edge_pz = iz == edge.z();
        if ((on_edge_nz && offset.z() > 0) || (on_edge_pz && offset.z() < 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1020.1));
        }
        if ((on_edge_nz && offset.z() < 0) || (on_edge_pz && offset.z() > 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1275.1));
        }
    }
    template <typename GB>
    inline void generate_chunk_faces_rotated(
        const min::vec3<float> &p, const min::tri<int> &offset,
        const min::tri<size_t> &index,
        const min::tri<size_t> &edge, const GB &get_block, const float float_atlas) const
    {
        const size_t ix = index.x();
        const size_t iy = index.y();
        const size_t iz = index.z();

        // Generate X Faces
        const bool on_edge_nx = ix == 0;
        const bool on_edge_px = ix == edge.x();
        if (!on_edge_nx && !on_edge_px)
        {
            // Unsafely check if cell is within the grid
            const block_id x1 = get_block(min::tri<size_t>(ix - 1, iy, iz));
            if (x1 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 0.1));
            }
        }
        else if ((on_edge_nx && offset.x() > 0) || (on_edge_px && offset.x() < 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 0.1));
        }
        if (!on_edge_nx && !on_edge_px)
        {
            const block_id x2 = get_block(min::tri<size_t>(ix + 1, iy, iz));
            if (x2 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 255.1));
            }
        }
        else if ((on_edge_nx && offset.x() < 0) || (on_edge_px && offset.x() > 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 255.1));
        }

        // Generate Y Faces
        const bool on_edge_ny = iy == 0;
        const bool on_edge_py = iy == edge.y();
        if (!on_edge_ny && !on_edge_py)
        {
            // Unsafely check if cell is within the grid
            const block_id y1 = get_block(min::tri<size_t>(ix, iy - 1, iz));
            if (y1 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 510.1));
            }
        }
        else if ((on_edge_ny && offset.y() > 0) || (on_edge_py && offset.y() < 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 510.1));
        }
        if (!on_edge_ny && !on_edge_py)
        {
            const block_id y2 = get_block(min::tri<size_t>(ix, iy + 1, iz));
            if (y2 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 765.1));
            }
        }
        else if ((on_edge_ny && offset.y() < 0) || (on_edge_py && offset.y() > 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 765.1));
        }

        // Generate Z Faces
        const bool on_edge_nz = iz == 0;
        const bool on_edge_pz = iz == edge.z();
        if (!on_edge_nz && !on_edge_pz)
        {
            // Unsafely check if cell is within the grid
            const block_id z1 = get_block(min::tri<size_t>(ix, iy, iz - 1));
            if (z1 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1020.1));
            }
        }
        else if ((on_edge_nz && offset.z() > 0) || (on_edge_pz && offset.z() < 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1020.1));
        }
        if (!on_edge_nz && !on_edge_pz)
        {
            const block_id z2 = get_block(min::tri<size_t>(ix, iy, iz + 1));
            if (z2 == block_id::EMPTY)
            {
                _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1275.1));
            }
        }
        else if ((on_edge_nz && offset.z() < 0) || (on_edge_pz && offset.z() > 0))
        {
            _cells.push_back(min::vec4<float>(p.x(), p.y(), p.z(), float_atlas + 1275.1));
        }
    }
    inline void generate_chunk(min::mesh<float, uint32_t> &mesh) const
    {
#ifdef MGL_GS_RENDER
        generate_chunk_gs(mesh);
#else
        generate_chunk_vbo(mesh);
#endif
    }
    inline void generate_preview(min::mesh<float, uint32_t> &mesh) const
    {
#ifdef MGL_GS_RENDER
        generate_preview_gs(mesh);
#else
        generate_preview_vbo(mesh);
#endif
    }
};
}

#endif
