#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <kernel/mandelbulb_asym.h>
#include <kernel/mandelbulb_exp.h>
#include <kernel/mandelbulb_sym.h>
#include <kernel/terrain_base.h>
#include <kernel/terrain_creative.h>
#include <kernel/terrain_height.h>
#include <limits>
#include <min/aabbox.h>
#include <min/array_buffer.h>
#include <min/bmp.h>
#include <min/camera.h>
#include <min/cubic.h>
#include <min/dds.h>
#include <min/emitter_buffer.h>
#include <min/grid.h>
#include <min/intersect.h>
#include <min/loop_sync.h>
#include <min/mat4.h>
#include <min/md5_anim.h>
#include <min/md5_model.h>
#include <min/mem_chunk.h>
#include <min/mesh.h>
#include <min/ogg.h>
#include <min/physics_nt.h>
#include <min/program.h>
#include <min/ray.h>
#include <min/serial.h>
#include <min/settings.h>
#include <min/shader.h>
#include <min/skeletal_vertex.h>
#include <min/sort.h>
#include <min/sound_buffer.h>
#include <min/static_vertex.h>
#include <min/strtoken.h>
#include <min/text_buffer.h>
#include <min/texture_buffer.h>
#include <min/tree.h>
#include <min/tri.h>
#include <min/uniform_buffer.h>
#include <min/vec2.h>
#include <min/vec3.h>
#include <min/vec4.h>
#include <min/vertex_buffer.h>
#include <min/wave.h>
#include <min/wavefront.h>
#include <min/window.h>
#include <mutex>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

extern template class min::aabbox<float, min::vec2>;
extern template class min::aabbox<float, min::vec3>;
extern template class min::aabbox<float, min::vec4>;
extern template class min::bezier_deriv<float, min::vec3>;
extern template class min::body<float, min::vec3>;
extern template class min::camera<float>;
extern template class min::grid<float, uint_fast16_t, uint_fast32_t, min::vec3, min::aabbox, min::aabbox>;
extern template class min::frustum<float>;
extern template class min::mat3<float>;
extern template class min::mat4<float>;
extern template class min::md5_model<float, uint32_t, min::vec4, min::aabbox>;
extern template class min::mesh<float, uint32_t>;
extern template class min::physics<float, uint_fast16_t, uint_fast32_t, min::vec3, min::aabbox, min::aabbox, min::grid>;
extern template class min::quat<float>;
extern template class min::ray<float, min::vec3>;
extern template class min::tree<float, uint_fast8_t, uint_fast8_t, min::vec2, min::aabbox, min::aabbox>;
extern template class min::tri<int>;
extern template class min::tri<unsigned>;
extern template class min::vec2<float>;
extern template class min::vec3<float>;
extern template class min::vec4<float>;
