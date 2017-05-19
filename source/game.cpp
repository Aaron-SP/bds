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
#include <fps_camera.h>
#include <iostream>
#include <min/bmp.h>
#include <min/camera.h>
#include <min/loop_sync.h>
#include <min/program.h>
#include <min/settings.h>
#include <min/shader.h>
#include <min/uniform_buffer.h>
#include <min/utility.h>
#include <min/window.h>
#include <string>
#include <world_mesh.h>

class mglcraft
{
  private:
    // OpenGL pipeline
    min::window _win;
    min::shader _tv;
    min::shader _tf;
    min::program _terrain_program;

    // Camera and uniform data
    min::camera<float> _cam;
    min::uniform_buffer<float> _ubuffer;
    size_t _proj_view_id;
    size_t _view_id;
    size_t _model_id;
    size_t _light_id;

    // Path placement
    game::world_mesh _world;
    game::fps_camera _fps_camera;

  public:
    // Load window shaders and program
    mglcraft()
        : _win("MGLCRAFT: FPS: ", 720, 480, 3, 3),
          _tv("data/shader/terrain.vertex", GL_VERTEX_SHADER),
          _tf("data/shader/terrain.fragment", GL_FRAGMENT_SHADER),
          _terrain_program(_tv, _tf),
          _ubuffer(1, 3),
          _world("data/texture/atlas.bmp", 64),
          _fps_camera(&_win, &_cam)
    {
        // Set depth and cull settings
        min::settings::initialize();

        // Use the terrain program for drawing
        _terrain_program.use();

        // Get keyboard from the window and register 1 and 2 keys for switching textures
        auto &keyboard = _win.get_keyboard();
        keyboard.add(min::window::key_code::KEY1);
        keyboard.add(min::window::key_code::KEY2);
        keyboard.add(min::window::key_code::KEY3);
        keyboard.add(min::window::key_code::KEY4);

        // Register callback function KEY1 for switching texture to 'grass'
        keyboard.register_keydown(min::window::key_code::KEY1, mglcraft::switch_grass, (void *)&_world);

        // Register callback function KEY2 for switching texture to 'stone'
        keyboard.register_keydown(min::window::key_code::KEY2, mglcraft::switch_stone, (void *)&_world);

        // Register callback function KEY2 for switching texture to 'sand'
        keyboard.register_keydown(min::window::key_code::KEY3, mglcraft::switch_sand, (void *)&_world);

        // Register callback function KEY2 for switching texture to 'wood'
        keyboard.register_keydown(min::window::key_code::KEY4, mglcraft::switch_wood, (void *)&_world);

        // Register click callback function for placing path
        _win.register_data((void *)this);
        _win.register_click(mglcraft::place_block);

        // Put cursor in center of window
        update_cursor();
    }
    static void switch_grass(void *ptr, double step)
    {
        // Call back function for switching texture to grass
        if (ptr)
        {
            // Cast to world_mesh pointer type and set atlas id to '0'
            game::world_mesh *world = reinterpret_cast<game::world_mesh *>(ptr);
            world->set_atlas_id(0);
        }
    }
    static void switch_stone(void *ptr, double step)
    {
        // Call back function for switching texture to stone
        if (ptr)
        {
            // Cast to world_mesh pointer type and set atlas id to '1'
            game::world_mesh *world = reinterpret_cast<game::world_mesh *>(ptr);
            world->set_atlas_id(1);
        }
    }
    static void switch_sand(void *ptr, double step)
    {
        // Call back function for switching texture to sand
        if (ptr)
        {
            // Cast to world_mesh pointer type and set atlas id to '2'
            game::world_mesh *world = reinterpret_cast<game::world_mesh *>(ptr);
            world->set_atlas_id(2);
        }
    }
    static void switch_wood(void *ptr, double step)
    {
        // Call back function for switching texture to wood
        if (ptr)
        {
            // Cast to world_mesh pointer type and set atlas id to '3'
            game::world_mesh *world = reinterpret_cast<game::world_mesh *>(ptr);
            world->set_atlas_id(3);
        }
    }
    static void place_block(void *ptr, const uint16_t x, const uint16_t y)
    {
        // Call back function for adding path
        // 'ptr' is passed in by us in constructor
        if (ptr)
        {
            // Cast to camera pointer type and move camera
            mglcraft *game = reinterpret_cast<mglcraft *>(ptr);
            const min::vec3<float> &forward = game->_cam.get_forward();
            const min::vec3<float> &position = game->_cam.get_position();

            // Calculate new point to add
            const min::vec3<float> point = position + forward * 4.0;

            // Add block to world
            game->_world.add_block(point);

            // Generate a new mesh
            game->_world.generate();
        }
    }
    void clear_background() const
    {
        // blue background
        const float color[] = {0.690, 0.875f, 0.901f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, color);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    bool is_closed() const
    {
        return _win.get_shutdown();
    }
    void load_camera_uniforms()
    {
        // Move and camera to -X and look at origin
        min::vec3<float> pos = min::vec3<float>(-1.0, 2.0, 0.0);
        min::vec3<float> look = min::vec3<float>(0.0, 0.0, 0.0);

        // Test perspective projection
        // Create camera, set location and look at
        _cam.set_position(pos);
        _cam.set_look_at(look);
        _cam.set_perspective();

        // Load the uniform buffer with program we will use
        _ubuffer.set_program(_terrain_program);

        // Load light into uniform buffer
        min::vec4<float> light_color(1.0, 1.0, 1.0, 1.0);
        min::vec4<float> light_position(0.0, 100.0, 0.0, 1.0);
        min::vec4<float> light_alpha(0.5, 1.0, 0.0, 1.0);
        _light_id = _ubuffer.add_light(min::light<float>(light_color, light_position, light_alpha));

        // Load projection and view matrix into uniform buffer
        _proj_view_id = _ubuffer.add_matrix(_cam.get_pv_matrix());
        _view_id = _ubuffer.add_matrix(_cam.get_v_matrix());
        _model_id = _ubuffer.add_matrix(min::mat4<float>());

        // Load the buffer with data
        _ubuffer.update();
    }
    void draw()
    {
        // Update model matrix
        const min::vec3<float> &forward = _cam.get_forward();
        const min::vec3<float> &position = _cam.get_position();

        // Calculate new point to add
        const min::vec3<float> point = position + forward * 4.0;
        min::mat4<float> translate(_world.snap(point));

        // Change light alpha for terrain
        min::vec4<float> light_color(1.0, 1.0, 1.0, 1.0);
        min::vec4<float> light_position(0.0, 100.0, 0.0, 1.0);
        _ubuffer.set_light(min::light<float>(light_color, light_position, min::vec4<float>(0.5, 1.0, 0.0, 1.0)), _light_id);

        // Update matrix uniforms
        _ubuffer.set_matrix(_cam.get_pv_matrix(), _proj_view_id);
        _ubuffer.set_matrix(_cam.get_v_matrix(), _view_id);
        _ubuffer.set_matrix(translate, _model_id);
        _ubuffer.update();

        // Bind the world vertex and texture buffers
        _world.bind();

        // Draw the world geometry
        _world.draw_terrain();

        // Change light alpha for placemark
        _ubuffer.set_light(min::light<float>(light_color, light_position, min::vec4<float>(0.5, 1.0, 0.0, 0.5)), _light_id);
        _ubuffer.update_lights();

        // Draw the placemark
        _world.draw_placemark();
    }
    void set_title(const std::string &title)
    {
        _win.set_title(title);
    }
    void update_camera(const double step)
    {
        // Get the cursor coordinates
        const auto c = _win.get_cursor();

        auto &keyboard = _win.get_keyboard();
        keyboard.update(step);

        // Get the offset from screen center
        const float sensitivity = 0.1;
        float x = sensitivity * (c.first - (_win.get_width() / 2));
        float y = sensitivity * (c.second - (_win.get_height() / 2));

        // If the mouse coordinates moved at all
        if (std::abs(x) > 1E-3 || std::abs(y) > 1E-3)
        {
            // Limit maximum jump
            min::clamp<float>(x, -2.0, 2.0);
            min::clamp<float>(y, -2.0, 2.0);

            // Adjust the camera by the offset from screen center
            _cam.move_look_at(x, y);

            // Move the cursor back
            update_cursor();
        }
    }
    void update_cursor()
    {
        // Get the screen dimensions
        uint16_t h = _win.get_height();
        uint16_t w = _win.get_width();

        // Center cursor in middle of window
        _win.set_cursor(w / 2, h / 2);
    }
    void window_update()
    {
        // Update and swap buffers
        _win.update();
        _win.swap_buffers();
    }
};

void run()
{
    // Load window shaders and program, enable shader program
    mglcraft game;

    // Load the camera and fill uniform buffers with light and model matrix
    game.load_camera_uniforms();

    // Setup controller to run at 60 frames per second
    const int frames = 60;
    min::loop_sync sync(frames);
    double step = 0;

    // User can close with Q or use window manager
    while (!game.is_closed())
    {
        for (int i = 0; i < frames; i++)
        {
            // Start synchronizing the loop
            sync.start();

            // Clear the background color
            game.clear_background();

            // Update the camera movement
            game.update_camera(step);

            // Draw the model
            game.draw();

            // Update the window after draw command
            game.window_update();

            // Calculate needed delay to hit target
            step = sync.sync();
        }

        // Calculate the number of 'average' frames per second
        double fps = sync.get_fps();

        // Update the window title with FPS count of last frame
        game.set_title("MGLCRAFT: FPS: " + std::to_string(fps));
    }
}

int main()
{
    try
    {
        run();
    }
    catch (const std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}