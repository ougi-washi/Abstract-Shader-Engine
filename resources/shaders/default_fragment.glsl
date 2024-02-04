// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450
#include "core/as_common.glsl"

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 frag_tex_coord;
layout(location = 2) in vec3 frag_normal;
layout(location = 3) flat in int instance_id;

layout(location = 0) out vec4 out_color;

void main() 
{
    float sphere_mask = 1. - (length(frag_tex_coord - .5) - 0.03) * 5.;
    // if (sphere_mask < .5)
    // {
    // -+    discard;
    // }
    
    out_color = texture(tex_sampler, vec2(frag_tex_coord.x + ps.current_time, frag_tex_coord.y)) * .8;
}

