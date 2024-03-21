// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
// inspired by https://www.shadertoy.com/view/stVBRR

#version 450

#include "as_2d_layout.glsl"
#include "as_2d_sdf_shapes.glsl"

#define FONT_TEXTURE_SUB_X 16
#define FONT_TEXTURE_SUB_Y 16
const vec2 FONT_CHAR_SIZE = 1. / (vec2(FONT_TEXTURE_SUB_X, FONT_TEXTURE_SUB_Y) + 1);

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

vec4 get_char(float c) 
{
    vec2 uvOffset = vec2(mod(c, FONT_TEXTURE_SUB_X), floor(c / FONT_TEXTURE_SUB_X));
    vec2 texCoord = (uvOffset + uv) * FONT_CHAR_SIZE;
    float char_mask = step(distance(uv, texCoord + (vec2(FONT_CHAR_SIZE.x, FONT_CHAR_SIZE.y) / 2.) ), FONT_CHAR_SIZE.x/ 2.); //+ 
    return vec4(texture(tex_sampler, uv) * char_mask);
}

void main()
{
    out_color = vec4(0.5, .3, 1., get_char(20).x);
}