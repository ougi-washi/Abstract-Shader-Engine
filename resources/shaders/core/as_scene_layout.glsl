// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

struct as_light 
{
    vec3 position;
    vec3 color;
    float radius;
};

struct as_mat4_array 
{
    mat4 data[128];
    int size;
};

struct as_lights_array 
{
    as_light data[128];
    int size;
};

struct as_scene 
{
    as_mat4_array objects_transforms;
    as_lights_array lights;
};

layout(set = 0, binding = 0) uniform scene_buffer
{
    as_scene scene;
};
