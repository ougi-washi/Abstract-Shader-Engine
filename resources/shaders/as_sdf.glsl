// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_sdf_shapes.glsl"

#define MIN_DIST 0.001
#define MAX_DIST 10000
#define MAX_MARCHING_STEPS 64
#define EPSILON .01

vec4 raymarch(vec3 ray_pos, vec3 ray_dir) 
{
    float depth = MIN_DIST;
    float dist = MIN_DIST;
    vec3 hit_point = vec3(MIN_DIST);
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) 
    {
        hit_point = ray_pos + depth * ray_dir;
        float dist = sd_sphere(hit_point, .5);
        if (dist < EPSILON)
        {
            break;
        }
        depth += dist;
        if (depth >= MAX_DIST) 
        {
            return vec4(0.0); 
        }
    }
    return vec4(hit_point, 1.0); 
}