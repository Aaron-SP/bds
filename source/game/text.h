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
#ifndef __TEXT__
#define __TEXT__

#include <iomanip>
#include <min/dds.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/text_buffer.h>
#include <sstream>
#include <vector>

namespace game
{

class text
{
  private:
    // Text OpenGL stuff
    min::shader _text_vertex;
    min::shader _text_fragment;
    min::program _text_prog;

    // Buffer for holding text
    min::text_buffer _text_buffer;
    std::vector<size_t> _indices;
    std::ostringstream _stream;
    size_t _font;
    bool _draw;

    inline void add_text(const std::string &s, const float x, const float y)
    {
        const size_t index = _text_buffer.add_text(s, x, y);

        // Add text index to index buffer
        _indices.push_back(index);
    }
    inline void clear_stream()
    {
        _stream.clear();
        _stream.str(std::string());
    }
    inline void reposition_text(const float width, const float height)
    {
        // Rescale all text items
        uint16_t y = height - 20;
        for (const size_t index : _indices)
        {
            // Update the text location
            _text_buffer.set_text_location(index, 10, y);
            y -= _font;
        }
    }
    inline void update_text(const std::string &s, const size_t index)
    {
        _text_buffer.set_text(s, index);
    }
    void upload()
    {
        // Upload the text glyphs to the GPU
        _text_buffer.upload();
    }

  public:
    text(const size_t font, const uint16_t width, const uint16_t height)
        : _text_vertex("data/shader/text.vertex", GL_VERTEX_SHADER),
          _text_fragment("data/shader/text.fragment", GL_FRAGMENT_SHADER),
          _text_prog(_text_vertex, _text_fragment),
          _text_buffer("data/fonts/open_sans.ttf", font),
          _font(font), _draw(false)
    {
        // Set the texture channel for this program, we need to do this here because we render text on channel '1'
        // _text_prog will be in use by the end of this call
        _text_buffer.set_texture_uniform(_text_prog, "in_texture", 1);

        // Update the text buffer screen dimensions
        _text_buffer.set_screen(width, height);

        // Add title text
        uint16_t y = height - 20;
        add_text("Fractex: Official Demo", 10, y);

        // Add character position
        y -= font;
        add_text("X: Y: Z:", 10, y);

        // Add character direction
        y -= font;
        add_text("X: Y: Z:", 10, y);

        // Game mode
        y -= font;
        add_text("MODE: PLAY:", 10, y);

        // Destination
        y -= font;
        add_text("DEST:", 10, y);

        // Energy
        y -= font;
        add_text("ENERGY:", 10, y);

        // FPS
        y -= font;
        add_text("FPS:", 10, y);

        // IDLE
        y -= font;
        add_text("IDLE:", 10, y);
    }
    void draw()
    {
        if (_draw)
        {
            // Bind the text_buffer vao, and textures on channel '1'
            _text_buffer.bind(1);

            // Bind the text program
            _text_prog.use();

            // Draw the FPS text
            _text_buffer.draw_all();
        }
    }
    bool get_draw() const
    {
        return _draw;
    }
    void toggle_draw()
    {
        _draw = !_draw;
    }
    void set_screen(const float width, const float height)
    {
        // Update the text buffer screen dimensions
        _text_buffer.set_screen(width, height);

        // Rescale all text on the screen
        reposition_text(width, height);

        // Upload new text
        upload();
    }
    void update_text(const min::vec3<float> &p, const min::vec3<float> &f, const std::string &mode,
                     const min::vec3<float> &goal, const double energy, const double fps, const double idle)
    {
        // If drawing text mode is on, update text
        if (get_draw())
        {
            // Update player position debug text
            _stream << std::fixed << std::setprecision(4) << "POS- X: " << p.x() << ", Y: " << p.y() << ", Z: " << p.z();
            update_text(_stream.str(), 1);

            // Clear and reset the _stream
            clear_stream();

            // Update player direction debug text
            _stream << "DIR- X: " << f.x() << ", Y: " << f.y() << ", Z: " << f.z();
            update_text(_stream.str(), 2);

            // Clear and reset the _stream
            clear_stream();

            // Update the game mode text
            update_text(mode, 3);

            // Update the destination text
            _stream << "DEST- X: " << goal.x() << ", Y: " << goal.y() << ", Z: " << goal.z();
            update_text(_stream.str(), 4);

            // Clear and reset the _stream
            clear_stream();

            // Update the energy text
            _stream << "ENERGY: " << energy;
            update_text(_stream.str(), 5);

            // Clear and reset the _stream
            clear_stream();

            // Update FPS and IDLE
            _stream << "FPS: " << std::round(fps);
            update_text(_stream.str(), 6);

            // Clear and reset the _stream
            clear_stream();

            // Update FPS and IDLE
            _stream << "IDLE: " << idle;
            update_text(_stream.str(), 7);

            // Clear and reset the _stream
            clear_stream();

            // Upload changes
            upload();
        }
    }
};
}

#endif
