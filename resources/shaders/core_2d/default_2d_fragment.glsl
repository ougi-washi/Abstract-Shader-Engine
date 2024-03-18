// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "as_2d_layout.glsl"
#include "as_2d_sdf_shapes.glsl"

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

void main()
{
    out_color = texture(tex_sampler, uv * vec2(15., 8.f));
    vec2 center_uv = uv - vec2(.5);
    float box_mask = sd_box(uv - vec2(.35, .5), vec2(.1f));
    box_mask = smoothstep(0.0, .01, box_mask);
    box_mask = 1. - box_mask;
    out_color = vec4(uv.y, uv.x, 1., box_mask * out_color.x);
}
