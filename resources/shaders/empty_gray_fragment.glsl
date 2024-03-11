// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "core/as_common.glsl"
#include "core/as_fragment_layout.glsl"
layout(binding = 1) uniform sampler2D tex_sampler;

void main()
{
   out_color = texture(tex_sampler, frag_tex_coord);
}
