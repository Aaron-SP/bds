/* Copyright [2013-2018] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the Fractex.

Fractex is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fractex is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fractex.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __TERRAIN_GEOMETRY__
#define __TERRAIN_GEOMETRY__

#include <game/terrain_vertex.h>

#ifndef USE_GS_RENDER
#include <game/geometry.h>
#include <game/work_queue.h>
#endif

#include <game/memory_map.h>
#include <min/dds.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/texture_buffer.h>
#include <min/vertex_buffer.h>
#include <numeric>

namespace game
{

#ifdef USE_GS_RENDER

class terrain
{
  private:
    min::shader _tv;
    min::shader _tg;
    min::shader _tf;
    min::program _program;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _pb;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _gb;
    min::texture_buffer _tbuffer;
    GLuint _dds_id;

    inline void generate_indices(min::mesh<float, uint32_t> &mesh)
    {
        // Generate indices
        mesh.index.resize(mesh.vertex.size());
        std::iota(mesh.index.begin(), mesh.index.end(), 0);
    }
    inline void load_texture()
    {
        // Load texture
        const min::mem_file &atlas = memory_map::memory.get_file("data/texture/atlas.dds");
        min::dds tex(atlas);

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(tex);
    }
    inline void reserve_memory(const size_t chunks, const size_t chunk_size)
    {
        // Reserve maximum number of cells in a chunk
        const size_t cells = chunk_size * chunk_size * chunk_size;
        const size_t vertex = 24 * cells;
        const size_t index = 36 * cells;

        // Reserve vertex buffer memory for geometry
        for (size_t i = 0; i < chunks; i++)
        {
            _gb.set_buffer(i);
            _gb.reserve(vertex, index, 1);
        }

        // Reserve vertex buffer memory for preview
        _pb.reserve(vertex, index, 1);
    }

  public:
    terrain(const game::uniforms &uniforms, const size_t chunks, const size_t chunk_size)
        : _tv(memory_map::memory.get_file("data/shader/terrain_gs.vertex"), GL_VERTEX_SHADER),
          _tg(memory_map::memory.get_file("data/shader/terrain_gs.geometry"), GL_GEOMETRY_SHADER),
          _tf(memory_map::memory.get_file("data/shader/terrain_gs.fragment"), GL_FRAGMENT_SHADER),
          _program({_tv.id(), _tg.id(), _tf.id()}),
          _gb(chunks)
    {
        // Load texture
        load_texture();

        // Reserve memory based on chunk scale
        reserve_memory(chunks, chunk_size);

        // Load the uniform buffer with program we will use
        uniforms.set_program(_program);
    }
    inline void bind() const
    {
        // Use the terrain program for drawing
        _program.use();

        // Bind the terrain texture for drawing
        _tbuffer.bind(_dds_id, 0);
    }
    inline void draw_placemark(game::uniforms &uniforms) const
    {
        // Set uniforms to light2
        uniforms.set_light2();

        // Bind VAO
        _pb.bind();

        // Draw placemarker
        _pb.draw_all(GL_POINTS);

        // Set uniforms to light1
        uniforms.set_light1();
    }
    inline void draw_terrain(game::uniforms &uniforms, const std::vector<size_t> &index) const
    {
        // For all chunk meshes
        for (const auto &i : index)
        {
            // Bind VAO
            _gb.bind_buffer(i);

            // Draw graph-mesh
            _gb.draw_all(GL_POINTS);
        }
    }
    inline void upload_geometry(const size_t index, min::mesh<float, uint32_t> &child)
    {
        // Swap buffer index for this chunk
        _gb.set_buffer(index);

        // Reset the buffer
        _gb.clear();

        // Only add if contains cells
        if (child.vertex.size() > 0)
        {
            // Generate indices
            generate_indices(child);

            // Add mesh to vertex buffer
            _gb.add_mesh(child);

            // Unbind the last VAO to prevent scrambling buffers
            _gb.unbind();

            // Upload terrain geometry to geometry buffer
            _gb.upload();
        }
    }
    inline void upload_preview(min::mesh<float, uint32_t> &terrain)
    {
        // Reset the buffer
        _pb.clear();

        // Only add if contains cells
        if (terrain.vertex.size() > 0)
        {
            // Generate indices
            generate_indices(terrain);

            // Add mesh to the buffer
            _pb.add_mesh(terrain);

            // Unbind the last VAO to prevent scrambling buffers
            _pb.unbind();

            // Upload the preview geometry to preview buffer
            _pb.upload();
        }
    }
};

#elif USE_INST_RENDER

class terrain
{
  private:
    static constexpr size_t _largest_chunk_size = 32768;
    min::shader _tv;
    min::shader _tf;
    min::program _program;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _gb;
    std::vector<min::uniform_buffer<float>> _ub;
    std::vector<min::mat4<float>> _mat;
    min::texture_buffer _tbuffer;
    GLuint _dds_id;
    min::mat4<float> _uni_mat[2];
    GLint _mat_loc;
    min::light<float> _light1;
    min::light<float> _light2;
    size_t _size;

    inline void allocate_mesh_buffer(const std::vector<min::vec4<float>> &cell_buffer)
    {
        // Resize the parent mesh from cell size
        const size_t size = cell_buffer.size();

        // One matrix for each block
        _mat.resize(size);
    }
    static inline size_t get_buffer_size(const size_t chunk_size)
    {
        // Calculate buffer size limits
        const size_t max_size = min::uniform_buffer<float>::get_max_buffer_size();
        const size_t size = chunk_size * chunk_size * chunk_size;

        // Alert user that we need a big enough uniform size to continue
        std::cout << "terrain : Asking for cells in chunk of: " + std::to_string(size) << std::endl;
        std::cout << "terrain : Max uniform buffer size is: " + std::to_string(max_size) << std::endl;

        // Check if uniform buffer size is not large enough
        if (size > max_size)
        {
            throw std::runtime_error("terrain: maximum uniform buffer size is too small for specified chunk size");
        }

        // Size is big enough for 32 chunk size
        return size;
    }
    inline void load_box_model()
    {
        // Load drop data from geometry functions
        min::mesh<float, uint32_t> box_mesh("terrain");

        // Define the box min and max
        const min::vec3<float> min(-0.5, -0.5, -0.5);
        const min::vec3<float> max(0.5, 0.5, 0.5);

        // Allocate mesh space
        box_mesh.vertex.resize(24);
        box_mesh.uv.resize(24);
        box_mesh.normal.resize(24);
        box_mesh.index.resize(36);

        // Calculate block vertices
        block_vertex(box_mesh.vertex, 0, min, max);

        // Calculate block uv's
        block_uv(box_mesh.uv, 0);

        // Calculate block normals
        block_normal(box_mesh.normal, 0);

        // Calculate block indices
        block_index<uint32_t>(box_mesh.index, 0, 0);

        // Add mesh and update buffers
        _gb.add_mesh(box_mesh);

        // Unbind the last VAO to prevent scrambling buffers
        _gb.unbind();

        // Upload mesh to buffers
        _gb.upload();
    }
    inline void load_lights()
    {
        // Change light alpha for placemark
        const min::vec4<float> col1(1.0, 1.0, 1.0, 1.0);
        const min::vec4<float> pos1(0.0, 100.0, 0.0, 1.0);
        const min::vec4<float> pow1(0.3, 0.7, 0.0, 1.0);
        const min::vec4<float> pow2(0.3, 0.7, 0.0, 0.50);
        _light1 = min::light<float>(col1, pos1, pow1);
        _light2 = min::light<float>(col1, pos1, pow2);
    }
    inline void load_texture()
    {
        // Load texture
        const min::mem_file &atlas = memory_map::memory.get_file("data/texture/atlas.dds");
        min::dds tex(atlas);

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(tex);
    }
    inline void load_uniform_buffers(const size_t chunks)
    {
        // Load all uniform buffers for each chunk
        for (size_t i = 0; i < chunks; i++)
        {
            _ub[i].defer_construct(1, _size);
            _ub[i].add_light(_light1);
            _ub[i].update_lights();
        }

        // Load uniform buffers for preview
        _ub[chunks].defer_construct(1, _size);
        _ub[chunks].add_light(_light2);
        _ub[chunks].update_lights();
    }
    inline void reserve_memory(const size_t chunks, const size_t chunk_size)
    {
        // Reserve maximum number of cells in a chunk
        const size_t vertex = 24;
        const size_t index = 36;

        // Reserve vertex buffer memory for geometry
        _gb.reserve(vertex, index, 1);

        // Reserve uniform buffer memory
        const size_t cells = chunk_size * chunk_size * chunk_size;
        const size_t buffers = chunks + 1;
        for (size_t i = 0; i < buffers; i++)
        {
            _ub[i].reserve_matrix(cells);
        }

        // Reserve maximum size of chunk
        _mat.reserve(cells);
    }
    inline void set_cell(const size_t cell, std::vector<min::vec4<float>> &cell_buffer)
    {
        // Unpack the point and the atlas
        const min::vec4<float> &unpack = cell_buffer[cell];

        // Create bounding box of cell and get box dimensions
        const min::vec3<float> p = min::vec3<float>(unpack.x(), unpack.y(), unpack.z());

        // Scale uv's based off atlas id
        const int8_t atlas = static_cast<int8_t>(unpack.w());

        // Push back location
        _mat[cell].set_translation(p);

        // Pack the matrix with the atlas id
        _mat[cell].w(atlas + 2.1);
    }
    inline min::mem_file &set_uniform_size(min::mem_file &file, const size_t chunk_size)
    {
        // Get the GPU buffer size
        _size = get_buffer_size(chunk_size);

        // Abort flag
        bool found = false;

        // Find size repeating zeros in the file
        const size_t file_size = file.size();
        for (size_t i = 0; i < file_size; i++)
        {
            if (file[i] == '0')
            {
                size_t count = 1;
                const size_t start = i;
                while (file[++i] == '0')
                {
                    count++;
                }

                // Found the size to overwrite
                if (count == 6)
                {
                    // String of digits
                    const std::string dig_str = std::to_string(_size);
                    if (dig_str.size() > 6)
                    {
                        throw std::runtime_error("terrain: chunk size larger than six digits");
                    }
                    else
                    {
                        // Write the digits to the vertex shader
                        const size_t dig_end = start + 5;
                        const size_t str_size = dig_str.size();
                        const size_t str_end = str_size - 1;

                        // Overwrite the digits
                        for (size_t i = 0; i < str_size; i++)
                        {
                            // Write from the end to the beginning in reverse order
                            file[dig_end - i] = dig_str[str_end - i];
                        }

                        // Write spaces for the remaing zeroes
                        for (size_t i = str_size; i < 6; i++)
                        {
                            file[dig_end - i] = ' ';
                        }

                        // Mission accomplished, let's go home
                        found = true;

                        // Work done so break out
                        break;
                    }
                }
            }
        }

        // Did we write the size correctly?
        if (!found)
        {
            throw std::runtime_error("terrain: couldn't find six digit string in 'data/shader/terrain_inst.vertex'");
        }

        // Return the edited file
        return file;
    }

  public:
    terrain(const game::uniforms &uniforms, const size_t chunks, const size_t chunk_size)
        : _tv(set_uniform_size(memory_map::memory.get_file("data/shader/terrain_inst.vertex"), chunk_size), GL_VERTEX_SHADER),
          _tf(memory_map::memory.get_file("data/shader/terrain_inst.fragment"), GL_FRAGMENT_SHADER),
          _program(_tv, _tf), _gb(), _ub(chunks + 1)
    {
        // Load box model
        load_box_model();

        // Load lights
        load_lights();

        // Load texture
        load_texture();

        // Load uniform buffers
        load_uniform_buffers(chunks);

        // Reserve memory based on chunk scale
        reserve_memory(chunks, chunk_size);

        // Get the mat_loc from the program
        _mat_loc = glGetUniformLocation(_program.id(), "uni_mat");
        if (_mat_loc == -1)
        {
            throw std::runtime_error("terrain: could not find uniform 'uni_mat' in shader");
        }

        // Load the binding points in program
        _ub.back().set_program(_program);
    }
    inline void bind() const
    {
        // Use the terrain program for drawing
        _program.use();

        // Update matrix uniform data
        const float *data = reinterpret_cast<const float *>(&_uni_mat[0]);
        glUniformMatrix4fv(_mat_loc, 2, GL_FALSE, data);

        // Bind the terrain texture for drawing
        _tbuffer.bind(_dds_id, 0);
    }
    inline void draw_placemark(game::uniforms &uniforms) const
    {
        // Bind VAO
        _gb.bind();

        // Bind uniform buffer for preview
        _ub.back().bind();

        // Draw placemarker
        const size_t draw_size = _ub.back().matrix_size();
        _gb.draw_many(GL_TRIANGLES, 0, draw_size);

        // Rebind main uniform buffer
        uniforms.bind();
    }
    inline void draw_terrain(game::uniforms &uniforms, const std::vector<size_t> &index) const
    {
        // Bind VAO
        _gb.bind();

        // For all chunk meshes
        for (const auto &i : index)
        {
            // Bind uniform buffer for this chunk
            _ub[i].bind();

            // Draw graph-mesh
            const size_t draw_size = _ub[i].matrix_size();
            _gb.draw_many(GL_TRIANGLES, 0, draw_size);
        }

        // Rebind main uniform buffer
        uniforms.bind();
    }
    inline void update_matrices(const min::mat4<float> &pv, const min::mat4<float> &preview)
    {
        // Update PV matrix
        _uni_mat[0] = pv;

        // Update preview matrix
        _uni_mat[1] = preview;
    }
    inline void upload_geometry(const size_t index, min::mesh<float, uint32_t> &child)
    {
        // Convert cells to mesh in parallel
        const size_t size = child.vertex.size();
        if (size > 0)
        {
            // Reserve space in parent mesh
            allocate_mesh_buffer(child.vertex);

            // Parallelize on generating cells
            const auto work = [this, &child](const size_t i) {
                set_cell(i, child.vertex);
            };

            // Convert cells to mesh in parallel
            work_queue::worker.run(work, 0, size);

            // Clear the uniform matrix buffer
            _ub[index].clear_matrix();

            // Update block matrices
            _ub[index].insert_matrix(_mat);

            // Update the uniform buffer
            _ub[index].update_matrix();
        }
    }
    inline void upload_preview(min::mesh<float, uint32_t> &terrain)
    {
        // Only add if contains cells
        const size_t size = terrain.vertex.size();
        if (size > 0)
        {
            // Reserve space in parent mesh
            allocate_mesh_buffer(terrain.vertex);

            // Convert cells to mesh in parallel
            for (size_t i = 0; i < size; i++)
            {
                set_cell(i, terrain.vertex);
            }

            // Clear the uniform matrix buffer
            _ub.back().clear_matrix();

            // Update block matrices
            _ub.back().insert_matrix(_mat);

            // Update the uniform buffer
            _ub.back().update_matrix();
        }
    }
};

#else

class terrain
{
  private:
    min::shader _tv;
    min::shader _tf;
    min::program _program;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _pb;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _gb;
    min::texture_buffer _tbuffer;
    GLuint _dds_id;
    min::mesh<float, uint32_t> _parent;

    inline void allocate_mesh_buffer(const std::vector<min::vec4<float>> &cell_buffer)
    {
        // Resize the parent mesh from cell size
        const size_t size = cell_buffer.size();

        // Vertex sizes
        const size_t size24 = size * 24;
        _parent.vertex.resize(size24);
        _parent.uv.resize(size24);
        _parent.normal.resize(size24);

        // Index sizes
        const size_t size36 = size * 36;
        _parent.index.resize(size36);
    }
    static inline min::aabbox<float, min::vec3> create_box(const min::vec3<float> &center)
    {
        // Create box at center
        const min::vec3<float> min = center - min::vec3<float>(0.5, 0.5, 0.5);
        const min::vec3<float> max = center + min::vec3<float>(0.5, 0.5, 0.5);

        // return the box
        return min::aabbox<float, min::vec3>(min, max);
    }
    inline void load_texture()
    {
        // Load texture
        const min::mem_file &atlas = memory_map::memory.get_file("data/texture/atlas.dds");
        min::dds tex(atlas);

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(tex);
    }
    inline void reserve_memory(const size_t chunks, const size_t chunk_size)
    {
        // Reserve maximum number of cells in a chunk
        const size_t cells = chunk_size * chunk_size * chunk_size;
        const size_t vertex = 24 * cells;
        const size_t index = 36 * cells;

        // Reserve maximum size of chunk
        _parent.vertex.reserve(vertex);
        _parent.uv.reserve(vertex);
        _parent.normal.reserve(vertex);
        _parent.index.reserve(index);

        // Reserve vertex buffer memory for geometry
        for (size_t i = 0; i < chunks; i++)
        {
            _gb.set_buffer(i);
            _gb.reserve(vertex, index, 1);
        }

        // Reserve vertex buffer memory for preview
        _pb.reserve(vertex, index, 1);
    }
    inline void set_cell(const size_t cell, std::vector<min::vec4<float>> &cell_buffer)
    {
        // Unpack the point and the atlas
        const min::vec4<float> &unpack = cell_buffer[cell];

        // Calculate vertex start position
        const size_t vertex_start = 24 * cell;
        const size_t index_start = 36 * cell;

        // Create bounding box of cell and get box dimensions
        const min::vec3<float> p = min::vec3<float>(unpack.x(), unpack.y(), unpack.z());
        const min::aabbox<float, min::vec3> b = create_box(p);
        const min::vec3<float> &min = b.get_min();
        const min::vec3<float> &max = b.get_max();

        // Calculate block vertices
        block_vertex(_parent.vertex, vertex_start, min, max);

        // Calculate block uv's
        block_uv(_parent.uv, vertex_start);

        // Scale uv's based off atlas id
        const int8_t atlas_id = static_cast<int8_t>(unpack.w());
        block_uv_scale(_parent.uv, vertex_start, atlas_id);

        // Calculate block normals
        block_normal(_parent.normal, vertex_start);

        // Calculate block indices
        block_index<uint32_t>(_parent.index, index_start, vertex_start);
    }

  public:
    terrain(const game::uniforms &uniforms, const size_t chunks, const size_t chunk_size)
        : _tv(memory_map::memory.get_file("data/shader/terrain.vertex"), GL_VERTEX_SHADER),
          _tf(memory_map::memory.get_file("data/shader/terrain.fragment"), GL_FRAGMENT_SHADER),
          _program(_tv, _tf),
          _gb(chunks), _parent("parent")
    {
        // Load texture
        load_texture();

        // Reserve memory based on chunk scale
        reserve_memory(chunks, chunk_size);

        // Load the uniform buffer with program we will use
        uniforms.set_program(_program);
    }
    inline void bind() const
    {
        // Use the terrain program for drawing
        _program.use();

        // Bind the terrain texture for drawing
        _tbuffer.bind(_dds_id, 0);
    }
    inline void draw_placemark(game::uniforms &uniforms) const
    {
        // Set uniforms to light2
        uniforms.set_light2();

        // Bind VAO
        _pb.bind();

        // Draw placemarker
        _pb.draw_all(GL_TRIANGLES);

        // Set uniforms to light1
        uniforms.set_light1();
    }
    inline void draw_terrain(game::uniforms &uniforms, const std::vector<size_t> &index) const
    {
        // For all chunk meshes
        for (const auto &i : index)
        {
            // Bind VAO
            _gb.bind_buffer(i);

            // Draw graph-mesh
            _gb.draw_all(GL_TRIANGLES);
        }
    }
    inline void upload_geometry(const size_t index, min::mesh<float, uint32_t> &child)
    {
        // Swap buffer index for this chunk
        _gb.set_buffer(index);

        // Reset the buffer
        _gb.clear();

        // Convert cells to mesh in parallel
        const size_t size = child.vertex.size();
        if (size > 0)
        {
            // Reserve space in parent mesh
            allocate_mesh_buffer(child.vertex);

            // Parallelize on generating cells
            const auto work = [this, &child](const size_t i) {
                set_cell(i, child.vertex);
            };

            // Convert cells to mesh in parallel
            work_queue::worker.run(work, 0, size);

            // Add mesh to vertex buffer
            _gb.add_mesh(_parent);

            // Unbind the last VAO to prevent scrambling buffers
            _gb.unbind();

            // Upload terrain geometry to geometry buffer
            _gb.upload();
        }
    }
    inline void upload_preview(min::mesh<float, uint32_t> &terrain)
    {
        // Reset the buffer
        _pb.clear();

        // Only add if contains cells
        const size_t size = terrain.vertex.size();
        if (size > 0)
        {
            // Reserve space in parent mesh
            allocate_mesh_buffer(terrain.vertex);

            // Convert cells to mesh in parallel
            for (size_t i = 0; i < size; i++)
            {
                set_cell(i, terrain.vertex);
            }

            _pb.add_mesh(_parent);

            // Unbind the last VAO to prevent scrambling buffers
            _pb.unbind();

            // Upload the preview geometry to preview buffer
            _pb.upload();
        }
    }
};
#endif
}

#endif
