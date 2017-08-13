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
#ifndef __SKY__
#define __SKY__

#include <min/dds.h>
#include <min/static_vertex.h>
#include <min/vertex_buffer.h>

namespace game
{

class sky
{
  private:
    // OpenGL stuff
    min::shader _sv;
    min::shader _sf;
    min::program _sky_program;

    // Texture stuff
    min::texture_buffer _tbuffer;
    GLuint _dds_id;

  public:
    sky(const min::uniform_buffer<float> &ub, const float extent)
        : _sv("data/shader/sky.vertex", GL_VERTEX_SHADER),
          _sf("data/shader/sky.fragment", GL_FRAGMENT_SHADER),
          _sky_program(_sv, _sf)
    {
        // Let this program use this uniform buffer
        ub.set_program(_sky_program);

        // Load texture
        min::dds tex("data/texture/sky.dds");

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(tex);
    }
    void draw()
    {
        // Bind this texture for drawing
        _tbuffer.bind(_dds_id, 0);

        // Use the sky program
        _sky_program.use();

        // Draw texture to background
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
};
}

#endif
