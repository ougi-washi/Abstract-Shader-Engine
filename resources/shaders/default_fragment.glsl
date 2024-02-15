// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "core/as_common.glsl"
#include "core/as_fragment_layout.glsl"
#include "as_sdf.glsl"

sdf_result sdf_scene(vec3 p)
{
    float sphere1_dist = sd_sphere(p, 0.4);
    vec3 sphere1_color = vec3(1.0, 0.0, 0.0);

    vec3 sphere2_offset = vec3(cos(get_current_time() * (instance_id + 1) * .6), sin(get_current_time() * (instance_id + 1 ) * .2) * 2., 0.);
    float sphere2_dist = sd_sphere(p + sphere2_offset, 0.4);
    vec3 sphere2_color = vec3(0.0, 0.0, 1.0);

    float blended_dist = op_smooth_union(sphere1_dist, sphere2_dist, 0.8);
    vec3 blended_color = mix(sphere1_color, sphere2_color, smoothstep(sphere2_dist, sphere1_dist, blended_dist));

    return sdf_result(p, blended_color, blended_dist);
}


void main()
{
    mat4 object_transform = get_object_transform(get_object_index());
    vec4 clip_pos = object_transform * vec4(vert_pos, 1.0);
    vec3 world_pos = (clip_pos.xyz / clip_pos.w) - obj_position;
    vec3 ray_dir = normalize(world_pos - get_camera_pos());
    sdf_result sdf = raymarch(get_camera_pos(), ray_dir);
    vec3 color = sdf.color;

    if(sdf.alpha < .4)
    {
        discard;
    }

    vec3 light_dir = vec3(1, -20, 10);
    float light_mask = dot(frag_normal.xyz, light_dir);

    // color = vec3(clamp(smoothstep(.1, 30., light_mask), 0., 1.) + .003) * color;
    out_color = vec4(color, 1.);
    // vec4(get_object_index());
}
