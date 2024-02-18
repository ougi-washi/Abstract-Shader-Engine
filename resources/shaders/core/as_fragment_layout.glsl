// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

layout(binding = 1) uniform sampler2D tex_sampler;

// has to be this order to match as_vertex
layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_color;
layout(location = 3) in vec2 frag_tex_coord;
layout(location = 4) in vec3 obj_position;// instances share the same obj position so they don't work, they have to be adjusted accordingly
layout(location = 5) in flat int instance_id; 

layout(location = 0) out vec4 out_color;
