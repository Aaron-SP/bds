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

    size_t _light;
    size_t _proj_view_id;
    size_t _view_id;
    size_t _particle_id;
    size_t _preview_id;
    size_t _md5_id;

    std::vector<size_t> _ui_scale_id;
    std::vector<size_t> _ui_uv_id;
    std::vector<size_t> _chest_id;
    std::vector<size_t> _drone_id;
    std::vector<size_t> _drop_id;
    std::vector<size_t> _explode_id;
    std::vector<size_t> _missile_id;
    std::vector<size_t> _bone_id;

    inline void load_uniforms(const size_t ui, const size_t chests, const size_t drones, const size_t drops, const size_t explosives, const size_t missiles, const size_t bones)
    {
        // Change light alpha for placemark
        const min::vec4<float> col1(1.0, 1.0, 1.0, 1.0);
        const min::vec4<float> pos1(0.0, 100.0, 0.0, 1.0);
        const min::vec4<float> pow1(0.3, 0.7, 0.0, 1.0);
        _light1 = min::light<float>(col1, pos1, pow1);

        // Add light to buffer
        _light = _ub.add_light(_light1);

        // Load projection, view, camera, and md5 matrix into uniform buffer
        _proj_view_id = _ub.add_matrix(min::mat4<float>());
        _view_id = _ub.add_matrix(min::mat4<float>());
        _particle_id = _ub.add_matrix(min::mat4<float>());
        _preview_id = _ub.add_matrix(min::mat4<float>());
        _md5_id = _ub.add_matrix(min::mat4<float>());

        // Initialize ui scale matrices
        _ui_scale_id.resize(ui);
        for (size_t i = 0; i < ui; i++)
        {
            _ui_scale_id[i] = _ub.add_matrix(min::mat4<float>());
        }

        // Initialize ui scale matrices
        _ui_uv_id.resize(ui);
        for (size_t i = 0; i < ui; i++)
        {
            _ui_uv_id[i] = _ub.add_matrix(min::mat4<float>());
        }

        // Initialize chest matrices
        _chest_id.resize(chests);
        for (size_t i = 0; i < chests; i++)
        {
            _chest_id[i] = _ub.add_matrix(min::mat4<float>());
        }

        // Initialize drone matrices
        _drone_id.resize(drones);
        for (size_t i = 0; i < drones; i++)
        {
            _drone_id[i] = _ub.add_matrix(min::mat4<float>());
        }

        // Initialize drop matrices
        _drop_id.resize(drops);
        for (size_t i = 0; i < drops; i++)
        {
            _drop_id[i] = _ub.add_matrix(min::mat4<float>());
        }

        // Initialize explosive matrices
        _explode_id.resize(explosives);
        for (size_t i = 0; i < explosives; i++)
        {
            _explode_id[i] = _ub.add_matrix(min::mat4<float>());
        }

        // Initialize missile matrices
        _missile_id.resize(missiles);
        for (size_t i = 0; i < missiles; i++)
        {
            _missile_id[i] = _ub.add_matrix(min::mat4<float>());
        }

        // Initialize bone matrices
        _bone_id.resize(bones);
        for (size_t i = 0; i < bones; i++)
        {
            _bone_id[i] = _ub.add_matrix(min::mat4<float>());
        }

        // Load the buffer with data
        _ub.update();
    }

  public:
    uniforms() : _ub(1, 415, 0)
    {
        // Load the number of used uniforms into the buffer
        load_uniforms(110, 10, 10, 50, 10, 10, 100);
    }
    inline void bind() const
    {
        _ub.bind();
    }
    inline void set_program_lights(const min::program &p) const
    {
        _ub.set_program_lights(p);
    }
    inline void set_program_matrix(const min::program &p) const
    {
        _ub.set_program_matrix(p);
    }
    inline void set_program_vector(const min::program &p) const
    {
        _ub.set_program_vector(p);
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
    }
    inline void update_md5_model(const min::mat4<float> &model)
    {
        _ub.set_matrix(model, _md5_id);
    }
    inline void update_chests(const std::vector<min::mat4<float>> &matrices)
    {
        // Upload all chest matrices
        const size_t size = matrices.size();
        for (size_t i = 0; i < size; i++)
        {
            _ub.set_matrix(matrices[i], _chest_id[i]);
        }
    }
    inline void update_drones(const std::vector<min::mat4<float>> &matrices)
    {
        // Upload all drone matrices
        const size_t size = matrices.size();
        for (size_t i = 0; i < size; i++)
        {
            _ub.set_matrix(matrices[i], _drone_id[i]);
        }
    }
    inline void update_drops(const std::vector<min::mat4<float>> &matrices)
    {
        // Upload all drop matrices
        const size_t size = matrices.size();
        for (size_t i = 0; i < size; i++)
        {
            _ub.set_matrix(matrices[i], _drop_id[i]);
        }
    }
    inline void update_explosives(const std::vector<min::mat4<float>> &matrices)
    {
        // Upload all explosive matrices
        const size_t size = matrices.size();
        for (size_t i = 0; i < size; i++)
        {
            _ub.set_matrix(matrices[i], _explode_id[i]);
        }
    }
    inline void update_missiles(const std::vector<min::mat4<float>> &matrices)
    {
        // Upload all missile matrices
        const size_t size = matrices.size();
        for (size_t i = 0; i < size; i++)
        {
            _ub.set_matrix(matrices[i], _missile_id[i]);
        }
    }
    inline void update_ui(const std::vector<min::mat3<float>> &scale, const std::vector<min::mat3<float>> &uv)
    {
        // Upload all ui scale matrices
        const size_t s_size = scale.size();
        for (size_t i = 0; i < s_size; i++)
        {
            _ub.set_matrix(scale[i], _ui_scale_id[i]);
        }

        // Upload all ui uv matrices
        const size_t uv_size = uv.size();
        for (size_t i = 0; i < uv_size; i++)
        {
            _ub.set_matrix(uv[i], _ui_uv_id[i]);
        }
    }
    inline void update_preview(const min::mat4<float> &preview)
    {
        _ub.set_matrix(preview, _preview_id);
    }
};
}

#endif
