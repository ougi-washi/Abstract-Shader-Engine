// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "as_ui_layout.glsl"
#include "as_ui_sdf_shapes.glsl"

//layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

void main()
{
   // out_color = texture(tex_sampler, uv);
    out_color = vec4(1.);
    vec2 center_uv = uv - vec2(.5);
    float sphere_mask = sd_box(uv - .5, vec2(.1f));
    //float sphere_mask = length(center_uv) - .3f;
    sphere_mask = smoothstep(0.0, .3, sphere_mask);
    out_color = vec4(uv.y * out_color.x, out_color.y, out_color.z, sphere_mask);
}
