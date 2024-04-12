// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_sdf_shapes.glsl"

#define SDF_MIN_DIST 0.001
#define SDF_MAX_DIST 100000
#define SDF_MAX_MARCHING_STEPS 64
#define SDF_EPSILON .001

struct sdf_result
{
    vec3 position;
    vec3 color;
    float alpha;
};

sdf_result sdf_scene(vec3 p);

sdf_result raymarch() 
{
    const vec3 world_vert_pos = vert_pos + get_current_object_position() ;
    const vec3 ray_pos = get_camera_pos() - get_current_object_position();
    const vec3 ray_dir = normalize(world_vert_pos - ray_pos);
    
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
