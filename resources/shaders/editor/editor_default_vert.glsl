// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "../core/as_common.glsl"
#include "../core/as_vertex_layout.glsl"

void main() 
{
    mat4 object_transform = get_current_object_transform();
    gl_Position = ubo.proj * ubo.view * object_transform * vec4(in_position.xyz, 1.);
    vert_pos = in_position;
    frag_normal = in_normal;
    frag_color = in_color;
    frag_tex_coord = in_tex_coord;
    //instance_id = gl_InstanceIndex; // enable when used on the frag side
    obj_position = new_pos;
}
