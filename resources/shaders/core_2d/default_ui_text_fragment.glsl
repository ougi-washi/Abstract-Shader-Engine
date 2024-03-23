// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
// inspired by https://www.shadertoy.com/view/stVBRR

#version 450

#include "as_ui_text_layout.glsl"
#include "as_2d_sdf_shapes.glsl"

ivec4 decode_chars(uint encoded_chars) 
{
    uint c1 = (encoded_chars >> 24) & 0xFF;
    uint c2 = (encoded_chars >> 16) & 0xFF;
    uint c3 = (encoded_chars >> 8) & 0xFF;
    uint c4 = encoded_chars & 0xFF;
    return ivec4(c1, c2, c3, c4);
}

float get_char(vec2 position, uint c) 
{
    vec2 new_uv = uv;
    vec2 uv_char_offset = vec2(mod(c, FONT_TEXTURE_SUB_X), floor(c / FONT_TEXTURE_SUB_X));
    vec2 uv_char_text_coord = (uv_char_offset + .5) * FONT_TEXTURE_CHAR_SIZE;
    float char_mask = step(distance(uv, position ), FONT_TEXTURE_CHAR_SIZE.x/ 2.); //+ 
    return (smoothstep(.5, .55, 1. - texture(tex_sampler, uv - (position - uv_char_text_coord)).a)) * char_mask;
}

// float display_text(in uint[11] text, vec2 start_position) 
// {
    
//     return text_opacity;
// }

void main()
{
    // uint hello_world[11] = uint[11](72, 69, 76, 76, 79, 32, 87, 79, 82, 76, 68); // "HELLO WORLD" as a uint array
    // float opacity = display_text(hello_world, vec2(0.1, 0.1), 11); // Length of "HELLO WORLD" is 11

    vec2 position = vec2(0.1, 0.1); 
    float text_opacity = 0.0;

    // for (int i = 0; i < get_text_length(); ++i) 
    // {
    //     uint c1 = (ubo.custom_data[i] >> 24) & 0xFF;
    //     uint c2 = (ubo.custom_data[i] >> 16) & 0xFF;
    //     uint c3 = (ubo.custom_data[i] >> 8) & 0xFF;
    //     uint c4 = ubo.custom_data[i] & 0xFF;

    //     text_opacity += get_char(position, c1);
    //     position.x += FONT_CHAR_SPACING; 
    //     text_opacity += get_char(position, c2);
    //     position.x += FONT_CHAR_SPACING; 
    //     text_opacity += get_char(position, c3);
    //     position.x += FONT_CHAR_SPACING;
    //     text_opacity += get_char(position, c4);
    //     position.x += FONT_CHAR_SPACING; 
    // }

    out_color = vec4(0.5, 0.3, 1.0, text_opacity);
    out_color = vec4((get_text_length() == 1) ? 1. : 0.);
}