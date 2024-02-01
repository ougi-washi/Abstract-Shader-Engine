#version 450
#include "as_common.glsl"

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
  
void main() 
{
    float sphere_mask = 1. - (length(fragTexCoord - .5) - .1) * 3.;
    outColor = sphere_mask* vec4(fragTexCoord.x, fragTexCoord.y, 1., 0.) * texture(texSampler, fragTexCoord);
}
