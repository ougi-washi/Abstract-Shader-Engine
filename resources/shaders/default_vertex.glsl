#version 450

layout(binding = 0) uniform uniform_buffer_object 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;


#include "as_common.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() 
{
    vec3 gridSpacing = vec3(1.1, 1.1, 1.1); 
    ivec3 gridSize = ivec3(50, 50, 50); 

    int instance_index_x = gl_InstanceIndex % gridSize.x;
    int instance_index_y = (gl_InstanceIndex / gridSize.x) % gridSize.y;
    int instance_index_z = gl_InstanceIndex / (gridSize.x * gridSize.y);

    int movement_frequency_xy = int(mod(instance_index_x + instance_index_y, 3));
    int movement_frequency_xz = int(mod(instance_index_x + instance_index_y, 3));
    int movement_frequency_yz = int(mod(instance_index_x + instance_index_y, 3));

    vec3 new_pos = inPosition + vec3(
        float(instance_index_x) * gridSpacing.x + cos(ps.time * movement_frequency_xy) 
        * sin(ps.time * movement_frequency_xy) * -.2 ,
        float(instance_index_y) * gridSpacing.y + cos(ps.time * movement_frequency_xz) 
        * sin(ps.time * movement_frequency_xz) * -.2 ,
        float(instance_index_z) * gridSpacing.z + cos(ps.time * movement_frequency_yz) 
        * sin(ps.time * movement_frequency_yz) * 0.2 
    );
    gl_Position = ubo.proj * ubo.view * ps.transform * vec4(new_pos, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
