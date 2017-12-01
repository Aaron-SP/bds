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
#ifndef __UNIFORMS__
#define __UNIFORMS__

#include <min/camera.h>
#include <min/uniform_buffer.h>
#include <min/vec4.h>

namespace game
{

class uniforms
{
  private:
    min::uniform_buffer<float> _ub;
    min::light<float> _light1;
    min::light<float> _light2;

    size_t _light;
    size_t _proj_view_id;
    size_t _view_id;
    size_t _camera_id;
    size_t _preview_id;
    size_t _md5_id;

    std::vector<size_t> _mob_id;
    std::vector<size_t> _bone_id;

    inline void load_uniforms(const size_t mobs, const size_t bones)
    {
        // Change light alpha for placemark
        const min::vec4<float> col1(1.0, 1.0, 1.0, 1.0);
        const min::vec4<float> pos1(0.0, 100.0, 0.0, 1.0);
        const min::vec4<float> pow1(0.3, 0.7, 0.0, 1.0);
        const min::vec4<float> pow2(0.3, 0.7, 0.0, 0.50);
        _light1 = min::light<float>(col1, pos1, pow1);
        _light2 = min::light<float>(col1, pos1, pow2);

        // Add light to buffer
        _light = _ub.add_light(_light1);

        // Load projection, view, camera, and md5 matrix into uniform buffer
        _proj_view_id = _ub.add_matrix(min::mat4<float>());
        _view_id = _ub.add_matrix(min::mat4<float>());
        _camera_id = _ub.add_matrix(min::mat4<float>());
        _preview_id = _ub.add_matrix(min::mat4<float>());
        _md5_id = _ub.add_matrix(min::mat4<float>());

        // Upload all mob positions
        _mob_id.resize(mobs);
        for (size_t i = 0; i < mobs; i++)
        {
            _mob_id[i] = _ub.add_matrix(min::mat4<float>());
        }

        // Add bones matrices to uniform buffer
        _bone_id.resize(bones);
        for (size_t i = 0; i < bones; i++)
        {
            _bone_id[i] = _ub.add_matrix(min::mat4<float>());
        }

        // Load the buffer with data
        _ub.update();
    }

  public:
    uniforms()
        : _ub(1, 115)
    {
        // Load the number of used uniforms into the buffer
        load_uniforms(10, 100);
    }
    inline void bind() const
    {
        _ub.bind();
    }
    void set_program(const min::program &p) const
    {
        _ub.set_program(p);
    }
    void set_light1()
    {
        _ub.set_light(_light1, _light);
        _ub.update_lights();
    }
    void set_light2()
    {
        _ub.set_light(_light2, _light);
        _ub.update_lights();
    }
    inline void update_matrix_buffer()
    {
        _ub.update_matrix();
    }
    inline void update_bones(const std::vector<min::mat4<float>> &bones)
    {
        // Update model bones matrices
        const size_t size = bones.size();
        for (size_t i = 0; i < size; i++)
        {
            _ub.set_matrix(bones[i], _bone_id[i]);
        }
    }
    inline void update_camera(min::camera<float> &cam)
    {
        _ub.set_matrix(cam.get_pv_matrix(), _proj_view_id);
        _ub.set_matrix(cam.get_v_matrix(), _view_id);
        _ub.set_matrix(min::mat4<float>(cam.get_position()), _camera_id);
    }
    inline void update_md5_model(const min::mat4<float> &model)
    {
        _ub.set_matrix(model, _md5_id);
    }
    inline void update_mobs(const std::vector<min::vec3<float>> &positions)
    {
        // Upload all mob positions
        const size_t size = positions.size();
        for (size_t i = 0; i < size; i++)
        {
            // Update position matrix
            const min::mat4<float> m(positions[i]);
            _ub.set_matrix(m, _mob_id[i]);
        }
    }
    inline void update_preview(const min::mat4<float> &preview)
    {
        _ub.set_matrix(preview, _preview_id);
    }
};
}

#endif
