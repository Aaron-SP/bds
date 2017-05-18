#ifndef __FPSCAMERA__
#define __FPSCAMERA__

#include <iostream>
#include <min/camera.h>
#include <min/window.h>

namespace game
{

class fps_camera
{
  private:
  public:
    fps_camera(min::window *window, min::camera<float> *camera)
    {
        auto &keyboard = window->get_keyboard();

        // Add FPS(WADS) keys to watch
        keyboard.add(min::window::key_code::KEYQ);
        keyboard.add(min::window::key_code::KEYW);
        keyboard.add(min::window::key_code::KEYS);
        keyboard.add(min::window::key_code::KEYA);
        keyboard.add(min::window::key_code::KEYD);
        keyboard.add(min::window::key_code::ENTER);

        // Register callback function for closing window
        keyboard.register_keydown(min::window::key_code::KEYQ, fps_camera::close_window, (void *)window);

        // Register callback function W
        keyboard.register_keydown(min::window::key_code::KEYW, fps_camera::forward, (void *)camera);
        keyboard.set_per_frame(min::window::key_code::KEYW, true);

        // Register callback function A
        keyboard.register_keydown(min::window::key_code::KEYA, fps_camera::left, (void *)camera);
        keyboard.set_per_frame(min::window::key_code::KEYA, true);

        // Register callback function D
        keyboard.register_keydown(min::window::key_code::KEYD, fps_camera::right, (void *)camera);
        keyboard.set_per_frame(min::window::key_code::KEYD, true);

        // Register callback function S
        keyboard.register_keydown(min::window::key_code::KEYS, fps_camera::back, (void *)camera);
        keyboard.set_per_frame(min::window::key_code::KEYS, true);
    }
    static void close_window(void *ptr, double step)
    {
        // Call back function for closing window
        // 'ptr' is passed in by us in constructor
        if (ptr)
        {
            // Cast to window pointer type and call shut down on window
            min::window *win = reinterpret_cast<min::window *>(ptr);
            win->set_shutdown();
        }

        // Alert that we received the call back
        std::cout << "fps_camera: Shutdown called by user" << std::endl;
    }
    static void forward(void *ptr, double step)
    {
        if (ptr)
        {
            // Cast to camera pointer type and move camera
            min::camera<float> *cam = reinterpret_cast<min::camera<float> *>(ptr);
            const min::vec3<float> &direction = cam->get_forward();
            const min::vec3<float> &position = cam->get_position();
            cam->set_position(position + direction * step * 4.0);
        }
    }
    static void left(void *ptr, double step)
    {
        if (ptr)
        {
            // Cast to camera pointer type and move camera
            min::camera<float> *cam = reinterpret_cast<min::camera<float> *>(ptr);
            const min::vec3<float> &right = cam->get_frustum().get_right();
            const min::vec3<float> &position = cam->get_position();
            cam->set_position(position - right * step * 4.0);
        }
    }
    static void right(void *ptr, double step)
    {
        if (ptr)
        {
            // Cast to camera pointer type and move camera
            min::camera<float> *cam = reinterpret_cast<min::camera<float> *>(ptr);
            const min::vec3<float> &right = cam->get_frustum().get_right();
            const min::vec3<float> &position = cam->get_position();
            cam->set_position(position + right * step * 4.0);
        }
    }
    static void back(void *ptr, double step)
    {
        if (ptr)
        {
            // Cast to camera pointer type and move camera
            min::camera<float> *cam = reinterpret_cast<min::camera<float> *>(ptr);
            const min::vec3<float> direction = cam->get_forward();
            const min::vec3<float> &position = cam->get_position();
            cam->set_position(position - direction * step * 4.0);
        }
    }
};
}

#endif