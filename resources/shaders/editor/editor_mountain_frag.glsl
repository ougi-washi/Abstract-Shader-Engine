// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#version 450

#include "../core/as_common.glsl"
#include "../core/as_fragment_layout.glsl"
#include "../sdf/as_sdf.glsl"

// Standard 2x2 hash algorithm.
vec2 hash22(vec2 p) 
{
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

float sdf_mountain(vec3 p, float noise)
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
        vec3 sdf_pos = p - get_object_position(i);
        float obj_dist = SDF_MAX_DIST;
        vec3 sphere_color = vec3(0.);
        
        float wind_influence = fBm(sdf_pos.xy * 30. + (get_current_time() * vec2(-.18, .2)));
        float mountain_noise_main = pow(fBm(sdf_pos.xy * 4.5), 7.) * 3.;
        float mountain_noise_secondary = pow(fBm(sdf_pos.xy * 75.5 + wind_influence * 1.), 8.) * 2.;
        float sdf_mountain_main = sdf_mountain(sdf_pos, mountain_noise_main);
        float sdf_mountain_secondary = sdf_mountain(sdf_pos, mountain_noise_secondary);
        
        // obj_dist = op_subtraction(sd_sphere(sdf_pos, 0.9), sdf_mountain_main);
        obj_dist = op_smooth_union(sdf_mountain_main, sdf_mountain_secondary, .5);
        sphere_color = vec3(0.0118, 0.8706, 0.0118) * smoothstep(-.01, .06, mountain_noise_main + mountain_noise_secondary) * pow(wind_influence, 2.);
            
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
