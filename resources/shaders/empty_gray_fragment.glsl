// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "core/as_common.glsl"
#include "core/as_fragment_layout.glsl"

void main()
{
    // vec3 world_pos = vert_pos + get_current_object_position();
    // vec3 ray_dir = - normalize(world_pos + get_camera_pos());
    // sdf_result sdf = raymarch(get_camera_pos(), ray_dir);
    out_color = vec4(0.6);
}
