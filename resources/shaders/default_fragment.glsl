// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450
#include "core/as_common.glsl"
#include "as_sdf.glsl"

layout(binding = 1) uniform sampler2D tex_sampler;

// has to be this order to match as_vertex
layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_color;
layout(location = 3) in vec2 frag_tex_coord;
layout(location = 4) in vec3 obj_position;// instances share the same obj position so they don't work, they have to be adjusted accordingly
layout(location = 5) in flat int instance_id; 

layout(location = 0) out vec4 out_color;


float sdf_scene(vec3 p)
{
    float sphere1 = sd_sphere(p, 0.6);
    vec3 sphere2_offset = vec3(cos(ps.current_time), sin(ps.current_time) * 2., 0.);
    float sphere2 = sd_sphere(p + sphere2_offset, .6);
    return op_smooth_union(sphere1, sphere2, 1.);
}

void main()
{
    vec4 clip_pos = ps.object_transform * vec4(vert_pos, 1.0);
    vec3 world_pos = (clip_pos.xyz / clip_pos.w) - obj_position;
    vec3 ray_dir = normalize(world_pos - ps.camera_location);
    vec4 color = raymarch(ps.camera_location, ray_dir);
    
    if(color.a < .4)
    {
        discard;
    }

    vec3 light_dir = vec3(1, -20, 10);
    float light_mask = dot(frag_normal.xyz, light_dir);

    color.rgb = vec3(clamp(smoothstep(.1, 30., light_mask), 0., 1.) + .003);
    out_color = vec4(color);
}
