// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
// inspired by https://www.shadertoy.com/view/stVBRR

#version 450

#include "as_2d_layout.glsl"
#include "as_2d_sdf_shapes.glsl"

#define MAX_TEXT_SIZE 2048

#define FONT_TEXTURE_SUB_X 16
#define FONT_TEXTURE_SUB_Y 16
const vec2 FONT_TEXTURE_CHAR_SIZE = 1. / (vec2(FONT_TEXTURE_SUB_X, FONT_TEXTURE_SUB_Y));

#define FONT_CHAR_PER_ROW       16
#define FONT_CHAR_PER_LINE      16
const vec2 FONT_CHAR_SCALE_XY = vec2(FONT_TEXTURE_SUB_X / FONT_CHAR_PER_ROW, FONT_TEXTURE_SUB_Y / FONT_CHAR_PER_LINE);
const float FONT_CHAR_SPACING = .03 * FONT_CHAR_SCALE_XY.x;

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

float get_char(vec2 position, float c) 
{
    vec2 new_uv = uv;
    vec2 uv_char_offset = vec2(mod(c, FONT_TEXTURE_SUB_X), floor(c / FONT_TEXTURE_SUB_X));
    vec2 uv_char_text_coord = (uv_char_offset + .5) * FONT_TEXTURE_CHAR_SIZE;
    float char_mask = step(distance(uv, position ), FONT_TEXTURE_CHAR_SIZE.x/ 2.); //+ 
    return (smoothstep(.5, .55, 1. - texture(tex_sampler, uv - (position - uv_char_text_coord)).a)) * char_mask;
}

float display_text(in uint[11] text, vec2 start_position, int text_length) 
{
    vec2 position = start_position; // Starting position
    float text_opacity = 0.0;

    for (int i = 0; i < text_length; ++i) 
    {
        text_opacity += get_char(position, text[i]);
        position.x += FONT_CHAR_SPACING; // Move to the next character position
    }

    return text_opacity;
}

void main()
{
    uint hello_world[11] = uint[11](72, 69, 76, 76, 79, 32, 87, 79, 82, 76, 68); // "HELLO WORLD" as a uint array
    float opacity = display_text(hello_world, vec2(0.1, 0.1), 11); // Length of "HELLO WORLD" is 11
    out_color = vec4(0.5, 0.3, 1.0, opacity);
}