// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "../core/as_common.glsl"
#include "../core/as_fragment_layout.glsl"
#include "../sdf/as_sdf.glsl"

layout(binding = 1) uniform sampler2D sand_mask_texture;

// Inspired by IQ and Shane
mat2 rot2(in float a){ float c = cos(a), s = sin(a); return mat2(c, s, -s, c); }

// 3x1 hash function.
float hash( vec3 p ){ return fract(sin(dot(p, vec3(21.71, 157.97, 113.43)))*45758.5453); }

// IQ's smooth minium function. 
float smin(float a, float b , float s){
    
    float h = clamp( 0.5 + 0.5*(b-a)/s, 0. , 1.);
    return mix(b, a, h) - h*(1.0-h)*s;
}

// Smooth maximum, based on IQ's smooth minimum.
float smax(float a, float b, float s){
    
    float h = clamp( 0.5 + 0.5*(a-b)/s, 0., 1.);
    return mix(b, a, h) + h*(1.0-h)*s;
}

// Standard 2x2 hash algorithm.
vec2 hash22(vec2 p) {
    
    // Faster, but probaly doesn't disperse things as nicely as other methods.
    float n = sin(dot(p, vec2(113, 1)));
    p = fract(vec2(2097152, 262144)*n)*2. - 1.;
    return p;
}

float gradN2D(in vec2 f)
{
   const vec2 e = vec2(0, 1);
    vec2 p = floor(f);
    f -= p; // Fractional position within the cube.
    vec2 w = f*f*(3. - 2.*f); // Cubic smoothing. 
    float c = mix(mix(dot(hash22(p + e.xx), f - e.xx), dot(hash22(p + e.yx), f - e.yx), w.x),
                  mix(dot(hash22(p + e.xy), f - e.xy), dot(hash22(p + e.yy), f - e.yy), w.x), w.y);
    return c*.5 + .5; // Range: [0, 1].
}

// Gradient noise fBm.
float fBm(in vec2 p)
{
    return gradN2D(p)*.57 + gradN2D(p*2.)*.28 + gradN2D(p*4.)*.15;
}

float n2D(vec2 p) {

	vec2 i = floor(p); p -= i; 
    p *= p*(3. - p*2.);  
    return dot(mat2(fract(sin(mod(vec4(0, 1, 113, 114) + dot(i, vec2(1, 113)), 6.2831853))*
               43758.5453))*vec2(1. - p.y, p.y), vec2(1. - p.x, p.x) );

}

float grad(float x, float offs)
{
    x = abs(fract(x/6.283 + offs - .25) - .5    )*2.;
    float x2 = clamp(x*x*(-1. + 2.*x), 0., 1.); // Customed smoothed, peaky triangle wave.
    x = smoothstep(0., 1., x); // Basic smoothing - Equivalent to: x*x*(3. - 2.*x).
    return mix(x, x2, .15);
    
}

float sandL(vec2 p)
{
    vec2 q = rot2(3.14159/18.)*p; // Rotate the layer, but not too much.
    q.y += (gradN2D(q*18.) - .5)*.05; // Perturb the lines to make them look wavy.
    float grad1 = grad(q.y*80., 0.); // Repeat gradient lines.
   
    q = rot2(-3.14159/20.)*p; // Rotate the layer back the other way, but not too much.
    q.y += (gradN2D(q*12.) - .5)*.05; // Perturb the lines to make them look wavy.
    float grad2 = grad(q.y*80., .5); // Repeat gradient lines.
      
    q = rot2(3.14159/4.)*p;
   
    float a2 = dot(sin(q*12. - cos(q.yx*12.)), vec2(.25)) + .5;
    float a1 = 1. - a2;
    float c = 1. - (1. - grad1*a1)*(1. - grad2*a2);
   
    return c;
    
}

float sand_mask(vec2 p)
{
    p = vec2(p.y - p.x, p.x + p.y)*.7071/4.;
    float c1 = sandL(p);
    vec2 q = rot2(3.14159/12.)*p;
    float c2 = sandL(q*1.25);
    c1 = mix(c1, c2, smoothstep(.1, .9, gradN2D(p*vec2(4))));
    return c1/(1.015);
}

float sdf_sand(vec3 p, float noise)
{
    return sd_box(p, vec3(.5 , .5,  noise));
}

sdf_result sdf_scene(vec3 p)
{
    vec3 light_dir = vec3(1, -20, 10);
    float light_mask = dot(frag_normal.xyz, light_dir);
    vec3 light_color = vec3(clamp(smoothstep(.1, 30., light_mask), 0., 1.) + .003);

    float blended_dist = SDF_MAX_DIST;
    vec3 blended_color = vec3(0.);
    for (int i = 0 ; i < get_object_count() ; i++)
    {
        float obj_dist = SDF_MAX_DIST;
        vec3 sphere_color = vec3(0.);
        if (i == 0)
        {
            vec3 sand_p = p - get_object_position(i);
            // float sand_noise = sand_mask(sand_p.xy);
            //vec4 sand_texture = texture(sand_mask_texture, (sand_p.xy + .5) * 3.5);
            // float sand_noise = pow(smoothstep(-2., 2., max(max(sand_texture.x, sand_texture.y), sand_texture.y)), 2.);
            float sand_noise = pow(fBm(sand_p.xy * 3.5), 7.);
            obj_dist = sdf_sand(sand_p, sand_noise);

            sphere_color =  vec3(0.6392, 0.3961, 0.1961) * smoothstep(-.05, .1, sand_noise) ;
        }
        else if (i == 1)
        {
            obj_dist = sd_sphere(p - get_object_position(i), 0.6);
            sphere_color = vec3(0., 1., 0.);
        }   
        else if (i == 2)
        {
            obj_dist = sd_sphere(p - get_object_position(i), 0.3);
            sphere_color = vec3(0., .0, 1.);
        }
        float color_weight = 1. - smoothstep(0.0, 1., obj_dist);
        blended_dist = op_smooth_union(blended_dist, obj_dist, 1.);
        blended_color = mix(blended_color, sphere_color, color_weight);
    }
    return sdf_result(p, blended_color, blended_dist);
}

void main()
{
    const sdf_result sdf = raymarch();
    if(sdf.alpha < .4) { discard; }
    out_color = vec4(sdf.color, 1.);
}
