#version 450
#include "as_common.glsl"


layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in int instanceID;

layout(location = 0) out vec4 outColor;
  

void main() 
{
    
    float sphere_mask = 1. - (length(fragTexCoord - .5) - 0.03) * 5.;
    if (sphere_mask < .5)
    {
        discard;
    }
    outColor = sin(mod(ps.time, 3)) * sphere_mask* vec4(fragTexCoord.x, fragTexCoord.y, cos(ps.time * 5. * mod(instanceID, 3)) * 50., 0.) * texture(texSampler, fragTexCoord);
}
