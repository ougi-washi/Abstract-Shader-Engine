// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
// inspired by https://www.shadertoy.com/view/stVBRR

#version 450

#include "as_2d_layout.glsl"
#include "as_2d_sdf_shapes.glsl"

#define FONT_TEXTURE_SUB_X 16
#define FONT_TEXTURE_SUB_Y 16
const vec2 FONT_TEXTURE_CHAR_SIZE = 1. / (vec2(FONT_TEXTURE_SUB_X, FONT_TEXTURE_SUB_Y));

#define FONT_CHAR_PER_ROW       16
#define FONT_CHAR_PER_LINE      16
const vec2 FONT_CHAR_SCALE_XY = vec2(FONT_TEXTURE_SUB_X / FONT_CHAR_PER_ROW, FONT_TEXTURE_SUB_Y / FONT_CHAR_PER_LINE);

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

float get_char(float c, vec2 position) 
{
    vec2 new_uv = uv;
    vec2 uv_char_offset = vec2(mod(c, FONT_TEXTURE_SUB_X), floor(c / FONT_TEXTURE_SUB_X));
    vec2 uv_char_text_coord = (uv_char_offset + .5) * FONT_TEXTURE_CHAR_SIZE;
    float char_mask = step(distance(uv, position ), FONT_TEXTURE_CHAR_SIZE.x/ 2.); //+ 
    return texture(tex_sampler, uv - (position - uv_char_text_coord)).x * char_mask;
}

void main()
{
    float spacing = .03 * FONT_CHAR_SCALE_XY.x;
    float hello_world = 0.;
    hello_world += get_char(72, vec2(spacing * 1., .1)); // H
    hello_world += get_char(69, vec2(spacing * 2., .1)); // E
    hello_world += get_char(76, vec2(spacing * 3., .1)); // L
    hello_world += get_char(76, vec2(spacing * 4., .1)); // L
    hello_world += get_char(79, vec2(spacing * 5., .1)); // O
    hello_world += get_char(32, vec2(spacing * 6., .1)); // SPACE
    hello_world += get_char(87, vec2(spacing * 7., .1)); // W
    hello_world += get_char(79, vec2(spacing * 8., .1)); // O
    hello_world += get_char(82, vec2(spacing * 9., .1)); // R
    hello_world += get_char(76, vec2(spacing * 10., .1)); // L
    hello_world += get_char(68, vec2(spacing * 11., .1)); // D

    out_color = vec4(0.5, .3, 1., hello_world);
}