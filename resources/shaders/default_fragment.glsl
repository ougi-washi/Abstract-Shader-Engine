// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "core/as_common.glsl"
#include "core/as_fragment_layout.glsl"
#include "as_sdf.glsl"

sdf_result sdf_scene(vec3 p)
{
    float blended_dist = SDF_MAX_DIST;
    vec3 blended_color = vec3(0.f, 0.f, 0.f);

    for (int i = 0 ; i < get_object_count() ; i++)
    {
        float sphere_dist = SDF_MAX_DIST;
        vec3 sphere_color = vec3(0.);
        if (i == 0)
        {
            sphere_dist = sd_sphere(p + get_object_position(i), 0.4);
            sphere_color = vec3(1.0, 0.0, 0.0);
        }
        else if (i == 1)
        {
            sphere_dist = sd_sphere(p + get_object_position(i), 0.3);
            sphere_color = vec3(0.0, 0.0, 1.0);
        }

        blended_dist = op_smooth_union(blended_dist, sphere_dist, abs(cos(get_current_time())));
        blended_color = mix(blended_color, sphere_color, sphere_dist);
    }
    return sdf_result(p, blended_color, blended_dist);
}

void main()
{
    vec3 world_pos = vert_pos + get_current_object_position();
    vec3 ray_dir = - normalize(world_pos + get_camera_pos());
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
}

