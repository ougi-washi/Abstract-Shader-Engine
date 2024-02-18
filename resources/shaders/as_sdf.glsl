// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_sdf_shapes.glsl"
#include "as_sdf_operators.glsl"

#define SDF_MIN_DIST 0.001
#define SDF_MAX_DIST 10000
#define SDF_MAX_MARCHING_STEPS 64
#define SDF_EPSILON .01

struct sdf_result
{
    vec3 position;
    vec3 color;
    float alpha;
};

sdf_result sdf_scene(vec3 p);

sdf_result raymarch(vec3 ray_pos, vec3 ray_dir) 
{
    float depth = SDF_MIN_DIST;
    float dist = SDF_MIN_DIST;
    sdf_result result;

    for (int i = 0; i < SDF_MAX_MARCHING_STEPS; i++) 
    {
        result.position = ray_pos + depth * ray_dir;
        result = sdf_scene(result.position);

        dist = result.alpha;

        if (dist < SDF_EPSILON)
        {
            break;
        }

        depth += dist;

        if (depth >= SDF_MAX_DIST) 
        {
            return sdf_result(vec3(0.0), vec3(0.0), 0.0); 
        }
    }

    return sdf_result(result.position, result.color, 1.); // Maybe the alpha has to be based on distance?
}
