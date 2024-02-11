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

layout(set = 0, binding = 0) buffer SceneBuffer 
{
    as_scene scene;
};

layout(binding = 1) uniform sampler2D tex_sampler;

// has to be this order to match as_vertex
layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_color;
layout(location = 3) in vec2 frag_tex_coord;
layout(location = 4) in vec3 obj_position;// instances share the same obj position so they don't work, they have to be adjusted accordingly
layout(location = 5) in flat int instance_id; 

layout(location = 0) out vec4 out_color;
