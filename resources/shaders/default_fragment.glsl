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
layout(location = 4) in flat int instance_id;

layout(location = 0) out vec4 out_color;
 
void main() 

{
    // frag_tex_coord;- .5
    // float sdf_scene = sd_get_distance(ps.camera_location, ps.camera_direction) ;

    
    // out_color = vec4(sdf_scene);
    vec3 frag_pos = (inverse(ps.object_transform) * vec4(frag_tex_coord - .5, 0.0, 1.0)).xyz;
    vec3 ray_origin = ps.camera_direction - vert_pos;

    // vec3 ray_location = normalize(frag_pos - ps.camera_location);
  
    float distance = sd_get_distance(ray_origin, ps.camera_direction);
    // vec3 final_color = 1. - vec3(distance / MAX_DIST);
    
    // if (final_color.x < .5)
    // {
    //     discard; 
    // }
    out_color = vec4(vert_pos, 1.);

    // out_color = vec4(final_color, 1.0);
    
    // out_color = texture(tex_sampler, vec2(frag_tex_coord.x + ps.current_time, frag_tex_coord.y)) * .8;
}

