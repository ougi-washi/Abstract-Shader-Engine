#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() 
{
    vec3 gridSpacing = vec3(1.3, 1.3, 1.3); 
    ivec3 gridSize = ivec3(100, 100, 100); 

    int instanceIndexX = gl_InstanceIndex % gridSize.x;
    int instanceIndexY = (gl_InstanceIndex / gridSize.x) % gridSize.y;
    int instanceIndexZ = gl_InstanceIndex / (gridSize.x * gridSize.y);

    vec3 new_pos = inPosition + vec3(
        float(instanceIndexX) * gridSpacing.x,
        float(instanceIndexY) * gridSpacing.y,
        float(instanceIndexZ) * gridSpacing.z
    );

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(new_pos, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
