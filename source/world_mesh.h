#ifndef __WORLDMESH__
#define __WORLDMESH__

#include <cmath>
#include <cstdint>
#include <min/aabbox.h>
#include <min/bmp.h>
#include <min/convert.h>
#include <min/static_vertex.h>
#include <min/texture_buffer.h>
#include <min/vec2.h>
#include <min/vec3.h>
#include <min/vec4.h>
#include <min/vertex_buffer.h>
#include <stdexcept>
#include <vector>

namespace game
{

class shape
{
  public:
    enum class type : unsigned char
    {
        empty = 0,
        block = 1,
        floor = 2,
        wall = 3,
        ramp = 4,
    };

    uint32_t _key;
    type _id;
    uint8_t _atlas_id;

  public:
    shape() : _key(0), _id(type::empty), _atlas_id(0) {}
    const uint8_t get_atlas_id() const
    {
        return _atlas_id;
    }
    const type get_id() const
    {
        return _id;
    }
    const uint32_t get_key() const
    {
        return _key;
    }
    void set_atlas_id(const uint8_t id)
    {
        _atlas_id = id;
    }
    void set_id(const type id)
    {
        _id = id;
    }
    void set_key(const uint32_t key)
    {
        _key = key;
    }
};

class world_mesh
{
  private:
    min::bmp _bmp;
    min::aabbox<float, min::vec3> _root;
    uint32_t _scale;
    uint32_t _scale3;
    std::vector<shape> _grid;
    min::texture_buffer _tbuffer;
    GLuint _bmp_id;
    mutable min::vertex_buffer<float, uint32_t, min::static_vertex, GL_FLOAT, GL_UNSIGNED_INT> _buffer;
    uint8_t _atlas_id;

    static min::mesh<float, uint32_t> create_box_mesh(const min::aabbox<float, min::vec3> &box, const uint8_t atlas_id)
    {
        min::mesh<float, uint32_t> box_mesh = min::to_mesh<float, uint32_t>(box);
        if (atlas_id == 0)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.5;
                uv.y(uv.y() + 0.5);
            }
        }
        else if (atlas_id == 1)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.5;
                uv += 0.5;
            }
        }

        return box_mesh;
    }
    void create_placemark() const
    {
        // Create placemarker
        const min::vec3<float> min(-0.5, -0.5, -0.5);
        const min::vec3<float> max(0.5, 0.5, 0.5);

        // Generate the box mesh
        const min::aabbox<float, min::vec3> box(min, max);
        const min::mesh<float, uint32_t> marker = create_box_mesh(box, _atlas_id);

        // Add mesh to buffer and upload
        _buffer.add_mesh(marker);
        _buffer.upload();
    }
    min::vec3<float> get_center(const shape &s) const
    {
        const uint32_t key = s.get_key();
        return grid_center(key);
    }
    void generate_block(const shape &s) const
    {
        // Generate the box min and max
        const min::vec3<float> half_width(0.5, 0.5, 0.5);
        const min::vec3<float> center = get_center(s);
        const min::vec3<float> min = center - half_width;
        const min::vec3<float> max = center + half_width;

        // generate the box mesh
        const min::aabbox<float, min::vec3> box(min, max);
        const min::mesh<float, uint32_t> box_mesh = create_box_mesh(box, s.get_atlas_id());

        // upload meshes
        _buffer.add_mesh(box_mesh);
    }
    void generate_shape(const shape &s) const
    {
        switch (s.get_id())
        {
        case shape::type::block:
            generate_block(s);
            break;
        default:
            throw std::runtime_error("world_mesh: unknown shape type");
        }
    }

  public:
    world_mesh(const std::string &texture_file, const uint32_t size)
        : _bmp(texture_file),
          _root(min::vec3<float>(-(float)size, -(float)size, -(float)size), min::vec3<float>(size, size, size)),
          _scale(2.0 * size), _scale3(_scale * _scale * _scale), _grid(_scale3), _atlas_id(0)
    {
        // Load texture buffer
        _bmp_id = _tbuffer.add_bmp_texture(_bmp);

        // Create placemark indicator
        create_placemark();

        // Add spawn blocks to world
        add_block(min::vec3<float>(-0.5, 0.5, -0.5));
        add_block(min::vec3<float>(0.5, 0.5, -0.5));
        add_block(min::vec3<float>(-0.5, 0.5, 0.5));
        add_block(min::vec3<float>(0.5, 0.5, 0.5));

        // Generate mesh
        generate();
    }

    void add_block(const min::vec3<float> &center)
    {
        // snap position to center of cell
        min::vec3<float> p = snap(center);

        // Calculate the grid index for this shape
        const uint32_t index = grid_key(p);

        // Do not generate if cell is already filled
        if (_grid[index].get_id() != shape::type::empty)
        {
            // Remove block from grid
            _grid[index].set_id(shape::type::empty);
        }
        else
        {
            // Set the index for this shape in the grid
            _grid[index].set_key(index);
            _grid[index].set_id(shape::type::block);
            _grid[index].set_atlas_id(_atlas_id);
        }
    }
    void bind() const
    {
        // Bind this texture for drawing
        _tbuffer.bind(_bmp_id, 0);

        // Bind VAO
        _buffer.bind();
    }
    void draw_placemark() const
    {
        // Draw placemarker
        _buffer.draw(GL_TRIANGLES, 0);
    }
    void draw_terrain() const
    {
        // Draw graph-mesh
        _buffer.draw_all_after(GL_TRIANGLES, 0);
    }
    void generate() const
    {
        // Reset the buffer
        _buffer.clear();

        // Create placemark
        create_placemark();

        // Generate all shapes for rendering
        for (const shape &cell : _grid)
        {
            if (cell.get_id() != shape::type::empty)
            {
                generate_shape(cell);
            }
        }

        // Upload contents to the vertex buffer
        _buffer.upload();
    }
    inline uint32_t grid_key(const min::vec3<float> &point) const
    {
        if (!_root.point_inside(point))
        {
            throw std::runtime_error("world_mesh: point is not inside the world cell");
        }

        // Calculate grid index
        const uint32_t row = (point.x() - _root.get_min().x());
        const uint32_t col = (point.y() - _root.get_min().y());
        const uint32_t hei = (point.z() - _root.get_min().z());

        return row * _scale * _scale + col * _scale + hei;
    }
    inline min::vec3<float> grid_center(const uint32_t index) const
    {
        if (index >= _scale3)
        {
            throw std::runtime_error("world_mesh: index is not inside the world cell");
        }

        // Precalculate the square scale
        const uint32_t scale2 = _scale * _scale;

        // Calculate row, col and height
        const uint32_t row = index / scale2;
        const uint32_t col = (index - row * scale2) / _scale;
        const uint32_t hei = index - row * scale2 - col * _scale;

        const float x = row + _root.get_min().x() + 0.5;
        const float y = col + _root.get_min().y() + 0.5;
        const float z = hei + _root.get_min().z() + 0.5;

        return min::vec3<float>(x, y, z);
    }
    inline min::vec3<float> snap(const min::vec3<float> &point)
    {
        const float x = point.x();
        const float y = point.y();
        const float z = point.z();

        return min::vec3<float>(std::floor(x) + 0.5, std::floor(y) + 0.5, std::floor(z) + 0.5);
    }
    void set_atlas_id(const uint8_t id)
    {
        _atlas_id = id;

        // Regenerate the mesh
        generate();
    }
};
}

#endif