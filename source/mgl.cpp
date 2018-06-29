#include <cstdint>
#include <min/aabbox.h>
#include <min/camera.h>
#include <min/cubic.h>
#include <min/frustum.h>
#include <min/grid.h>
#include <min/mat3.h>
#include <min/mat4.h>
#include <min/md5_model.h>
#include <min/mesh.h>
#include <min/physics_nt.h>
#include <min/quat.h>
#include <min/ray.h>
#include <min/tree.h>
#include <min/tri.h>
#include <min/vec2.h>
#include <min/vec3.h>
#include <min/vec4.h>

template class min::aabbox<float, min::vec2>;
template class min::aabbox<float, min::vec3>;
template class min::aabbox<float, min::vec4>;
template class min::bezier_deriv<float, min::vec3>;
template class min::body<float, min::vec3>;
template class min::camera<float>;
template class min::grid<float, uint_fast16_t, uint_fast32_t, min::vec3, min::aabbox, min::aabbox>;
template class min::frustum<float>;
template class min::mat3<float>;
template class min::mat4<float>;
template class min::md5_model<float, uint32_t, min::vec4, min::aabbox>;
template class min::mesh<float, uint32_t>;
template class min::physics<float, uint_fast16_t, uint_fast32_t, min::vec3, min::aabbox, min::aabbox, min::grid>;
template class min::quat<float>;
template class min::ray<float, min::vec3>;
template class min::tree<float, uint_fast8_t, uint_fast8_t, min::vec2, min::aabbox, min::aabbox>;
template class min::tri<int>;
template class min::tri<unsigned>;
template class min::vec2<float>;
template class min::vec3<float>;
template class min::vec4<float>;
