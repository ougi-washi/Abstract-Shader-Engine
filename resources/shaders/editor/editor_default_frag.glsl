// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "../core/as_common.glsl"
#include "../core/as_fragment_layout.glsl"
#include "../sdf/as_sdf.glsl"

sdf_result sdf_scene(vec3 p)
{
    vec3 sdf_pos = get_current_object_position() - p;
    return sdf_result(p, frag_normal.xyz, sd_sphere(sdf_pos, .5));
}

void main()
{
    const sdf_result sdf = raymarch();
    if(sdf.alpha < .4) { discard; }
    out_color = vec4(sdf.color, 1.);
}
