// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
#include "as_2d_layout.glsl"

#define FONT_TEXTURE_SUB_X 16
#define FONT_TEXTURE_SUB_Y 16
const vec2 FONT_TEXTURE_CHAR_SIZE = 1. / (vec2(FONT_TEXTURE_SUB_X, FONT_TEXTURE_SUB_Y));

#define FONT_CHAR_PER_ROW       16
#define FONT_CHAR_PER_LINE      16
const vec2 FONT_CHAR_SCALE_XY = vec2(FONT_TEXTURE_SUB_X / FONT_CHAR_PER_ROW, FONT_TEXTURE_SUB_Y / FONT_CHAR_PER_LINE);

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

int get_text_length() { return int(ubo.custom_info[0][0]); }
int get_text_font_size() { return int(ubo.custom_info[0][1]); }
float get_text_spacing() { return ubo.custom_info[0][2] * FONT_CHAR_SCALE_XY.x; }
