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

float sd_scene(vec3 ray_pos, vec3 ray_dir) {
    float depth = MIN_DIST;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        float dist = sd_sphere(ray_pos + depth * ray_dir, .1);
        if (dist < EPSILON) {
            return depth;
        }
        depth += dist;
        if (depth >= MAX_DIST) {
            return MAX_DIST;
        }
    }
    return MAX_DIST;
}

vec4 raymarch(vec3 ray_pos, vec3 ray_dir) {
    float dist = sd_scene(ray_pos, ray_dir);
    if (dist >= MAX_DIST) {
        return vec4(0.0); 
    }

    vec3 intersection_point = ray_pos + dist * ray_dir;
    // shading
    
    return vec4(1.0); 
}

void main() {
    vec4 clip_pos = ps.object_transform * vec4(vert_pos, 1.0);
    vec3 world_pos = (clip_pos.xyz / clip_pos.w) - obj_position;
    vec3 ray_dir = normalize(world_pos - ps.camera_location);

    vec4 color = raymarch(ps.camera_location, ray_dir);

    if(color.a < .4)
    {
        discard;
    }
    out_color = vec4(color);
}
