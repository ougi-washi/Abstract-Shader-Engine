// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
// inspired by https://www.shadertoy.com/view/stVBRR

#version 450

#include "as_2d_layout.glsl"
#include "as_2d_sdf_shapes.glsl"

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

#define O out_color
#define C(c) O+= char(U,64+c) ; U.x-=.5
#define LF (U.y++)
#define CR (U.x = (( font_uv.x - position.x)*64.0/font_size))
#define NL (CR,LF)

vec4 char(vec2 p, int c) 
{
    if (p.x<.0|| p.x>1. || p.y<0.|| p.y>1.) return vec4(0,0,0,1e5);
	return textureGrad( tex_sampler, p/16. + fract( vec2(c, 15-c/16) / 16. ), dFdx(p/16.),dFdy(p/16.) );
}

void main()
{
    //out_color = texture(tex_sampler, uv);
    //out_color = vec4(1.);
    vec2 font_uv = uv - vec2(.5);
    //float box_mask = sd_box(uv - vec2(.2, .2), vec2(.1f));
    //box_mask = smoothstep(0.0, .01, box_mask);
    //box_mask = 1. - box_mask;

    out_color = vec4(0.0);
    float font_size = 2.;
    vec2 position = vec2(0,1.0 - font_size / 64.0);
    vec2 U = ( uv - position)*64.0/font_size;
    C(8);C(5);C(12);C(12);C(15);C(-32);C(23);C(15);C(18);C(12);C(4);C(-31);
    NL;
    C(0); C(8);C(5);C(12);C(12);C(15);C(-32);C(23);C(15);C(18);C(12);C(4);C(-31);
    out_color = out_color.xxxx;
}
