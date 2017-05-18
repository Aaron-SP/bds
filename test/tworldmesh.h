#ifndef __TESTWORLDMESH__
#define __TESTWORLDMESH__

#include <min/window.h>
#include <stdexcept>
#include <test.h>
#include <world_mesh.h>

bool test_world_mesh()
{
    bool out = true;

    // Create window for rendering
    min::window window("test world mesh", 720, 480, 3, 3);

    // Load the graph mesh with 128 pixel tile size
    game::world_mesh world("data/texture/atlas.bmp", 64);

    world.add_block(min::vec3<float>(0.0, 0.0, 0.0));
    world.add_block(min::vec3<float>(1.0, 1.0, 1.0));
    world.add_block(min::vec3<float>(2.0, 2.0, 2.0));

    // Generate the mesh
    world.generate();

    // Test snapping key
    const min::vec3<float> position(1.6, 2.6, 3.6);
    const min::vec3<float> snap = world.snap(position);
    out = out && compare(1.5, snap.x(), 1E-4);
    out = out && compare(2.5, snap.y(), 1E-4);
    out = out && compare(3.5, snap.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed world_mesh snap to grid");
    }

    // Test grid key and grid position converstion
    const uint32_t key = world.grid_key(position);
    const min::vec3<float> p = world.grid_center(key);
    out = out && compare(1.5, p.x(), 1E-4);
    out = out && compare(2.5, p.y(), 1E-4);
    out = out && compare(3.5, p.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed world_mesh key conversion");
    }

    // return status
    return out;
}

#endif