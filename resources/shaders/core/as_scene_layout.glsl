// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#define AS_MAX_GPU_OBJECT_TRANSFORMS_SIZE 128

// struct as_light 
// {
//     vec3 position;
//     float _padding_0;
//     vec3 color;
//     float radius;
// };

// struct as_mat4_array 
// {
//     mat4 data[128];
//     int size;
// };

// struct as_lights_array 
// {
//     as_light data[128];
//     int size;
// };

// struct as_scene 
// {
//     as_mat4_array objects_transforms;
//     as_lights_array lights;
// };

struct as_scene 
{
    mat4 info;
    mat4 objects_transforms[AS_MAX_GPU_OBJECT_TRANSFORMS_SIZE];
};

layout(set = 0, binding = 0) uniform scene_buffer
{
    as_scene scene;
};

mat4 get_object_transform(int index)
{
    as_scene curr_scene = scene;
    mat4 transforms[128] = curr_scene.objects_transforms;
    return transforms[index];
}

