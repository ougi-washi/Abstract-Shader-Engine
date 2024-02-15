// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "core/as_common.glsl"
#include "core/as_vertex_layout.glsl"

void main() 
{
    vec3 grid_spacing = vec3(1.5, 1.5, 1.5); 
    ivec3 grid_size = ivec3(200, 200, 200); 
    int instance_index_x = gl_InstanceIndex % grid_size.x;
    int instance_index_y = (gl_InstanceIndex / grid_size.x) % grid_size.y;
    int instance_index_z = gl_InstanceIndex / (grid_size.x * grid_size.y);
    int movement_frequency_xy = int(mod(instance_index_x + instance_index_y, 3));
    int movement_frequency_xz = int(mod(instance_index_x + instance_index_y, 3));
    int movement_frequency_yz = int(mod(instance_index_x + instance_index_y, 3));

    vec3 new_pos = in_position.xyz + vec3(
        float(instance_index_x) * grid_spacing.x + cos(get_current_time() * movement_frequency_xy) 
        * sin(get_current_time() * movement_frequency_xy) * -1.2 ,
        float(instance_index_y) * grid_spacing.y + sin(get_current_time() * movement_frequency_xz) 
        * cos(get_current_time() * movement_frequency_xz) * -1.2 ,
        float(instance_index_z) * grid_spacing.z + sin(get_current_time() * movement_frequency_yz) 
        * cos(get_current_time() * movement_frequency_yz) * 1.2 
    );

    as_scene curr_scene = scene;
    as_mat4_array transforms = curr_scene.objects_transforms;
    mat4 object_transform = transforms.data[get_object_index()];
    gl_Position = ubo.proj * ubo.view * object_transform * vec4(new_pos, 1.);
    vert_pos = in_position;
    frag_normal = in_normal;
    frag_color = in_color;
    frag_tex_coord = in_tex_coord;
    instance_id = gl_InstanceIndex;
    obj_position = vec3(object_transform[3][0], object_transform[3][1], object_transform[3][2]);
}
