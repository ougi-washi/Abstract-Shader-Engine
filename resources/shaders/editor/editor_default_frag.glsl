// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "../core/as_common.glsl"
#include "../core/as_fragment_layout.glsl"
#include "../sdf/as_sdf.glsl"

sdf_result sdf_scene(vec3 p)
{
    p *= op_rotate_x(get_current_time());
    p *= op_rotate_y(get_current_time());
    p *= op_rotate_z(get_current_time());

    vec3 sdf_pos = get_current_object_position() - p;
    float sphere_dist = sd_sphere(sdf_pos, .5);
    float pyramid_dist = sd_round_box(sdf_pos, vec3(.5), .03);
    return sdf_result(p, frag_normal.xyz, mix(pyramid_dist, sphere_dist, abs(sin(get_current_time() * .2))));
}

void main()
{
    const sdf_result sdf = raymarch();
    if(sdf.alpha < .4) { discard; }
    out_color = vec4(sdf.color, 1.);
}
