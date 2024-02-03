// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "core/as_common.glsl"

layout(binding = 0) uniform uniform_buffer_object 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_tex_coord;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec2 frag_tex_coord;
layout(location = 2) out int instance_id;

void main() 
{
    vec3 grid_spacing = vec3(1.1, 1.1, 1.1); 
    ivec3 grid_size = ivec3(20, 20, 20); 
    int instance_index_x = gl_InstanceIndex % grid_size.x;
    int instance_index_y = (gl_InstanceIndex / grid_size.x) % grid_size.y;
    int instance_index_z = gl_InstanceIndex / (grid_size.x * grid_size.y);
    int movement_frequency_xy = int(mod(instance_index_x + instance_index_y, 3));
    int movement_frequency_xz = int(mod(instance_index_x + instance_index_y, 3));
    int movement_frequency_yz = int(mod(instance_index_x + instance_index_y, 3));


    // vec3 to_camera = normalize(ps.camera_location - (ps.object_transform * vec4(in_position, 1.0)).xyz);
    // mat4 look_at_matrix = create_look_at(vec3(0.0), to_camera, vec3(0.0, 1.0, 0.0));
    // vec3 rotated_position = (look_at_matrix * vec4(in_position, 1.0)).xyz;
    // vec4 oriented_position = vec4(rotated_position, 1.0);


    vec3 new_pos = in_position.xyz + vec3(
        float(instance_index_x) * grid_spacing.x + cos(ps.current_time * movement_frequency_xy) 
        * sin(ps.current_time * movement_frequency_xy) * -1.2 ,
        float(instance_index_y) * grid_spacing.y + cos(ps.current_time * movement_frequency_xz) 
        * sin(ps.current_time * movement_frequency_xz) * -1.2 ,
        float(instance_index_z) * grid_spacing.z + cos(ps.current_time * movement_frequency_yz) 
        * sin(ps.current_time * movement_frequency_yz) * 1.2 
    );

    gl_Position = ubo.proj * ubo.view * ps.object_transform * vec4(new_pos, 1.);
    frag_color = in_color;
    frag_tex_coord = in_tex_coord;
    instance_id = gl_InstanceIndex;
}
