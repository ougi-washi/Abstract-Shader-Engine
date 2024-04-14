// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "../core/as_common.glsl"
#include "../core/as_vertex_layout.glsl"

void main() 
{
    vec3 grid_spacing = vec3(12., 12., 13.); 
    ivec3 grid_size = ivec3(30, 30, 30); 
    int instance_index_x = gl_InstanceIndex % grid_size.x;
    int instance_index_y = (gl_InstanceIndex / grid_size.x) % grid_size.y;
    int instance_index_z = gl_InstanceIndex / (grid_size.x * grid_size.y);
    int movement_frequency_xy = int(mod(instance_index_x + instance_index_y, 2));
    int movement_frequency_xz = int(mod(instance_index_x + instance_index_y, 3));
    int movement_frequency_yz = int(mod(instance_index_x + instance_index_y, 4));

    vec3 new_pos = in_position.xyz + vec3(
        float(instance_index_x) * grid_spacing.x + movement_frequency_xy * (cos(get_current_time() * 5.) + sin(get_current_time() * 2.)),
        float(instance_index_y) * grid_spacing.y + movement_frequency_xz * (cos(get_current_time() * 3.) + sin(get_current_time() * 3.)) ,
        float(instance_index_z) * grid_spacing.z + movement_frequency_yz * (cos(get_current_time() * 2.) + sin(get_current_time() * 5.))
    );

    mat4 object_transform = get_current_object_transform();
    gl_Position = ubo.proj * ubo.view * object_transform * vec4(new_pos, 1.);
    vert_pos = in_position;
    frag_normal = in_normal;
    frag_color = in_color;
    frag_tex_coord = in_tex_coord;
    obj_position = new_pos;
}
