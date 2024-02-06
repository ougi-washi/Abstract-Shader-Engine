// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_sdf_shapes.glsl"

#define MIN_DIST 0.001
#define MAX_DIST 10000
#define MAX_MARCHING_STEPS 64
#define EPSILON .01

float sd_get_distance(vec3 ray_pos, vec3 ray_dir) 
{
    float depth = MIN_DIST;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) 
    {
        float dist = length(ray_pos) - 0.6; //sd_sphere(ray_pos + depth * ray_dir, .6);
        if (dist < EPSILON) 
        {
			return depth;
        }
        //depth += dist;
        ray_pos += ray_dir;
        if (depth >= MAX_DIST) 
        {
            return MAX_DIST;
        }
    }
    return MAX_DIST;
}

