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

// #define O out_color
// #define C(c) O+= char(U,64+c) ; U.x-=.5
// #define LF (U.y++)
// #define CR (U.x = (( font_uv.x - position.x)*64.0/font_size))
// #define NL (CR,LF)

// vec4 char(vec2 p, int c) 
// {
//     if (p.x<.0|| p.x>1. || p.y<0.|| p.y>1.) return vec4(0,0,0,1e5);
// 	return textureGrad( tex_sampler, p/16. + fract( vec2(c, 15-c/16) / 16. ), dFdx(p/16.),dFdy(p/16.) );
// }

// void main()
// {
//     //out_color = texture(tex_sampler, uv);
//     //out_color = vec4(1.);
//     vec2 font_uv = uv - vec2(.5);
//     //float box_mask = sd_box(uv - vec2(.2, .2), vec2(.1f));
//     //box_mask = smoothstep(0.0, .01, box_mask);
//     //box_mask = 1. - box_mask;

//     out_color = vec4(0.0);
//     float font_size = 2.;
//     vec2 position = vec2(0,1.0 - font_size / 64.0);
//     vec2 U = ((uv - position)*64.0/font_size);
//     C(8);C(5);C(12);C(12);C(15);C(-32);C(23);C(15);C(18);C(12);C(4);C(-31);
//     NL;
//     C(0); C(8);C(5);C(12);C(12);C(15);C(-32);C(23);C(15);C(18);C(12);C(4);C(-31);
//     out_color = out_color.xxxx;
// }

#define FontWidth 8
#define FontHeight 8
#define LineMaxLength 40

ivec2 font_data[37] = ivec2[](
    ivec2(0x00000000,0x00000000), //space
    ivec2(0x7e91897e,0x00000000), //0
    ivec2(0x01ff4121,0x00000000), //1
    ivec2(0x71898543,0x00000000), //2
    ivec2(0x6e919142,0x00000000), //3
    ivec2(0x08ff4838,0x00000000), //4
    ivec2(0x8e9191f2,0x00000000), //5
    ivec2(0x0e91916e,0x00000000), //6
    ivec2(0xc0b08f80,0x00000000), //7
    ivec2(0x6e91916e,0x00000000), //8
    ivec2(0x6e919162,0x00000000), //9
    ivec2(0x7f88887f,0x00000000), //A
    ivec2(0x6e9191ff,0x00000000), //B
    ivec2(0x4281817e,0x00000000), //C
    ivec2(0x7e8181ff,0x00000000), //D
    ivec2(0x919191ff,0x00000000), //E
    ivec2(0x909090ff,0x00000000), //F
    ivec2(0x4685817e,0x00000000), //G
    ivec2(0xff1010ff,0x00000000), //H
    ivec2(0x0081ff81,0x00000000), //I
    ivec2(0x80fe8182,0x00000000), //J
    ivec2(0x413608ff,0x00000000), //K
    ivec2(0x010101ff,0x00000000), //L
    ivec2(0x601060ff,0x000000ff), //M
    ivec2(0x0c1060ff,0x000000ff), //N
    ivec2(0x7e81817e,0x00000000), //O
    ivec2(0x609090ff,0x00000000), //P
    ivec2(0x7f83817e,0x00000001), //Q
    ivec2(0x619698ff,0x00000000), //R
    ivec2(0x4e919162,0x00000000), //S
    ivec2(0x80ff8080,0x00000080), //T
    ivec2(0xfe0101fe,0x00000000), //U
    ivec2(0x0e010ef0,0x000000f0), //V
    ivec2(0x031c03fc,0x000000fc), //W
    ivec2(0x340834c3,0x000000c3), //X
    ivec2(0x300f30c0,0x000000c0), //Y
    ivec2(0xe1918d83,0x00000081) //Z
);

vec3 font(vec2 uv0,int id)
{
    vec2 uv1 = uv0;
    uv0 *= 64.0;
    uv0 -= 10.;
    ivec2 texel = ivec2(uv0);
    int bit_offset = texel.x * FontWidth + texel.y;

    int s,t;
    s = font_data[id].x;
    t = font_data[id].y;

    int tex = 0;
    
    if(bit_offset <= 31){
        s = s >> bit_offset;
        s = s & 0x00000001;
        tex = s;
    }
    else{
        t = t >> (bit_offset - 32);
        t = t & 0x00000001;
        tex = t;
    }

    tex = (abs(uv1.x - 0.5) < 0.5 && abs(uv1.y - 0.5) < 0.5) ? tex : 0;
    return vec3(tex); 
}

vec4 get_char(float c) 
{
    vec2 uvOffset = vec2(mod(c, FONT_TEXTURE_SUB_X), floor(c / FONT_TEXTURE_SUB_X));
    vec2 texCoord = (uvOffset + uv) * FONT_CHAR_SIZE;
    float char_mask = step(distance(uv, texCoord + (vec2(FONT_CHAR_SIZE.x, FONT_CHAR_SIZE.y) / 2.) ), FONT_CHAR_SIZE.x/ 2.); //+ 
    return vec4(texture(tex_sampler, uv) * char_mask);
    // return ;

    //p = uv;
    //p = p - fract(p);
    //p /= vec2(FONT_TEXTURE_SUB_X, FONT_TEXTURE_SUB_Y);
    //p += vec2(c/FONT_TEXTURE_SUB_X  + 1./FONT_TEXTURE_SUB_X, c/FONT_TEXTURE_SUB_Y + 1./FONT_TEXTURE_SUB_Y);
    //return textureGrad(tex_sampler, p, dFdx(vec2(p)) - 0.001 , dFdy(vec2(p)) + 0.001);
}

void main()
{
    out_color = vec4(0.5, .3, 1., get_char(20).x);
   //out_color = vec4(font(uv, 28), 1.);
    // vec2 scaledUV = uv * 2.;
    // out_color = get_char(uv, 78.);
    // out_color *= smoothstep(0.0, 1.0, 1.0 - distance(scaledUV, vec2(0.5))); // Fade out edges if scaling down
}