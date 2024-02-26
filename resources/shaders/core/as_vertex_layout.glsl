// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec2 in_tex_coord;

layout(location = 0) out vec3 vert_pos;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec3 frag_color;
layout(location = 3) out vec2 frag_tex_coord;
layout(location = 4) out vec3 obj_position;
//layout(location = 5) out flat int instance_id; uncommment only if used
