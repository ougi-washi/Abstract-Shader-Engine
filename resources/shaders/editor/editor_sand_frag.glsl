// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "../core/as_common.glsl"
#include "../core/as_fragment_layout.glsl"
#include "../sdf/as_sdf.glsl"

layout(binding = 1) uniform sampler2D sand_mask_texture;

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

float sdf_sand(vec3 p, float noise)
{
    return sd_box(p, vec3(.5 , .5,  noise));
}

sdf_result sdf_scene(vec3 p)
{
    vec3 sdf_pos = get_current_object_position() - p - vec3(0., 0., .14);
    float wind_influence = fBm(sdf_pos.xy * 6. + (get_current_time() * vec2(-.18, .2)));
    float sand_noise_main = pow(fBm((.6 + sdf_pos.xy) * 1.75), 6.) * 1.;
    float sand_noise_secondary = pow(fBm(.2 + sdf_pos.xy * 3.75), 5.1) * 1.;
    float sdf_sand_main = sdf_sand(sdf_pos, sand_noise_main);
    float sdf_sand_secondary = sdf_sand(sdf_pos, sand_noise_secondary);
    float noise_mask = sand_noise_main + sand_noise_secondary;
    float obj_dist = op_smooth_union(sdf_sand_main, sdf_sand_secondary, .5);
    vec3 sphere_color =  mix(vec3(0.4431, 0.2588, 0.1098), vec3(0.7804, 0.5529, 0.0235), noise_mask * 3.) 
                        * pow(smoothstep(-.05, .1, noise_mask) 
                        * 1.1, 2.) * pow(wind_influence * 1.5, 2.);
    return sdf_result(p, sphere_color, obj_dist);
}

void main()
{
    const sdf_result sdf = raymarch();
    if(sdf.alpha < .4) { discard; }
    out_color = vec4(sdf.color, 1.);
}
