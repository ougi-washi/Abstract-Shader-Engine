// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
// Most operators rely on Inigo Quilez's approach

#define op_revolution_2d(primitive) primitive(vec2(length(p.xz) - o, p.y))
#define op_extrusion_2d(primitive, p, h) (min(max(vec2(primitive(p.xy), abs(p.z) - h), 0.0), length(max(vec2(primitive(p.xy), abs(p.z) - h), 0.0))))
#define op_elongate_3d(primitive, p, h) (primitive(p - clamp(p, -h, h)))
#define op_elongate_3d_alt(primitive, p, h) (primitive(max(abs(p) - h, 0.0)) + min(max(max(abs(p) - h).xyz, 0.0)))
#define op_round_3d(primitive, p, rad) (primitive(p) - rad)
#define op_onion(sdf, thickness) (abs(sdf) - thickness)
#define op_sym_x(primitive) (primitive(abs(p.x)))
#define op_sym_xz(primitive) (primitive(abs(p.xz)))
#define op_repetition_3d(primitive, p, s) (primitive(p - s * round(p / s)))
#define op_limited_repetition_3d(primitive, p, s, l) (primitive(p - s * clamp(round(p / s), -l, l)))
#define op_displace(primitive, p, displacement) (primitive(p) + displacement(p))
#define op_twist(primitive, p, k) \
    { \
        const float twist_k = (k); \
        float c_twist = cos(twist_k * (p).y); \
        float s_twist = sin(twist_k * (p).y); \
        mat2 twist_matrix = mat2(c_twist, -s_twist, s_twist, c_twist); \
        vec3 twisted_point = vec3(twist_matrix * (p).xz, (p).y); \
        primitive(twisted_point); \
    }
#define op_cheap_bend(primitive, p, k) \
    { \
        const float bend_k = (k); \
        float c_bend = cos(bend_k * (p).x); \
        float s_bend = sin(bend_k * (p).x); \
        mat2 bend_matrix = mat2(c_bend, -s_bend, s_bend, c_bend); \
        vec3 bent_point = vec3(bend_matrix * (p).xy, (p).z); \
        primitive(bent_point); \
    }


#define op_revolution_2d_1param(primitive, param1) primitive(vec2(length(p.xz) - o, p.y), param1)
#define op_extrusion_2d_1param(primitive, p, h, param1) (min(max(vec2(primitive(p.xy, param1), abs(p.z) - h), 0.0), length(max(vec2(primitive(p.xy, param1), abs(p.z) - h), 0.0))))
#define op_elongate_3d_1param(primitive, p, h, param1) primitive(p - clamp(p, -h, h), param1)
#define op_elongate_3d_alt_1param(primitive, p, h, param1) (primitive(max(abs(p) - h, 0.0), param1) + min(max(max(abs(p) - h).xyz, 0.0), param1))
#define op_round_3d_1param(primitive, p, rad, param1) (primitive(p, param1) - rad)
#define op_sym_x_1param(primitive, param1) (primitive(abs(p.x), param1))
#define op_sym_xz_1param(primitive, param1) (primitive(abs(p.xz), param1))
#define op_repetition_3d_1param(primitive, p, s, param1) (primitive(p - s * round(p / s), param1))
#define op_limited_repetition_3d_1param(primitive, p, s, l, param1) (primitive(p - s * clamp(round(p / s), -l, l), param1))

#define op_revolution_2d_2params(primitive, param1, param2) primitive(vec2(length(p.xz) - o, p.y), param1, param2)
#define op_extrusion_2d_2params(primitive, p, h, param1, param2) (min(max(vec2(primitive(p.xy, param1, param2), abs(p.z) - h), 0.0), length(max(vec2(primitive(p.xy, param1, param2), abs(p.z) - h), 0.0))))
#define op_elongate_3d_2params(primitive, p, h, param1, param2) (primitive(p - clamp(p, -h, h), param1, param2))
#define op_elongate_3d_alt_2params(primitive, p, h, param1, param2) (primitive(max(abs(p) - h, 0.0), param1, param2) + min(max(max(abs(p) - h).xyz, 0.0), param1, param2))
#define op_round_3d_2params(primitive, p, rad, param1, param2) (primitive(p, param1, param2) - rad)
#define op_sym_x_2params(primitive, param1, param2) (primitive(abs(p.x), param1, param2))
#define op_sym_xz_2params(primitive, param1, param2) (primitive(abs(p.xz), param1, param2))
#define op_repetition_3d_2params(primitive, p, s, param1, param2) (primitive(p - s * round(p / s), param1, param2))
#define op_limited_repetition_3d_2params(primitive, p, s, l, param1, param2) (primitive(p - s * clamp(round(p / s), -l, l), param1, param2))

#define op_revolution_2d_3params(primitive, param1, param2, param3) primitive(vec2(length(p.xz) - o, p.y), param1, param2, param3)
#define op_extrusion_2d_3params(primitive, p, h, param1, param2, param3) (min(max(vec2(primitive(p.xy, param1, param2, param3), abs(p.z) - h), 0.0), length(max(vec2(primitive(p.xy, param1, param2, param3), abs(p.z) - h), 0.0))))
#define op_elongate_3d_3params(primitive, p, h, param1, param2, param3) (primitive(p - clamp(p, -h, h), param1, param2, param3))
#define op_elongate_3d_alt_3params(primitive, p, h, param1, param2, param3) (primitive(max(abs(p) - h, 0.0), param1, param2, param3) + min(max(max(abs(p) - h).xyz, 0.0), param1, param2, param3))
#define op_round_3d_3params(primitive, p, rad, param1, param2, param3) (primitive(p, param1, param2, param3) - rad)
#define op_sym_x_3params(primitive, param1, param2, param3) (primitive(abs(p.x), param1, param2, param3))
#define op_sym_xz_3params(primitive, param1, param2, param3) (primitive(abs(p.xz), param1, param2, param3))
#define op_repetition_3d_3params(primitive, p, s, param1, param2, param3) (primitive(p - s * round(p / s), param1, param2, param3))
#define op_limited_repetition_3d_3params(primitive, p, s, l, param1, param2, param3) (primitive(p - s * clamp(round(p / s), -l, l), param1, param2, param3))

#define op_revolution_2d_4params(primitive, param1, param2, param3, param4) primitive(vec2(length(p.xz) - o, p.y), param1, param2, param3, param4)
#define op_extrusion_2d_4params(primitive, p, h, param1, param2, param3, param4) (min(max(vec2(primitive(p.xy, param1, param2, param3, param4), abs(p.z) - h), 0.0), length(max(vec2(primitive(p.xy, param1, param2, param3, param4), abs(p.z) - h), 0.0))))
#define op_elongate_3d_4params(primitive, p, h, param1, param2, param3, param4) (primitive(p - clamp(p, -h, h), param1, param2, param3, param4))
#define op_elongate_3d_alt_4params(primitive, p, h, param1, param2, param3, param4) (primitive(max(abs(p) - h, 0.0), param1, param2, param3, param4) + min(max(max(abs(p) - h).xyz, 0.0), param1, param2, param3, param4))
#define op_round_3d_4params(primitive, p, rad, param1, param2, param3, param4) (primitive(p, param1, param2, param3, param4) - rad)
#define op_sym_x_4params(primitive, param1, param2, param3, param4) (primitive(abs(p.x), param1, param2, param3, param4))
#define op_sym_xz_4params(primitive, param1, param2, param3, param4) (primitive(abs(p.xz), param1, param2, param3, param4))
#define op_repetition_3d_4params(primitive, p, s, param1, param2, param3, param4) (primitive(p - s * round(p / s), param1, param2, param3, param4))
#define op_limited_repetition_3d_4params(primitive, p, s, l, param1, param2, param3, param4) (primitive(p - s * clamp(round(p / s), -l, l), param1, param2, param3, param4))

float length2( vec3 p ) { p=p*p; return sqrt( p.x+p.y+p.z); }
float length6( vec3 p ) { p=p*p*p; p=p*p; return pow(p.x+p.y+p.z,1.0/6.0); }
float length8( vec3 p ) { p=p*p; p=p*p; p=p*p; return pow(p.x+p.y+p.z,1.0/8.0); }

float op_union( float d1, float d2 ) { return min(d1,d2); }
float op_subtraction( float d1, float d2 ) { return max(-d1,d2); }
float op_intersection( float d1, float d2 ) { return max(d1,d2); }
float op_xor(float d1, float d2 ) { return max(min(d1,d2),-max(d1,d2)); }

float op_smooth_union( float d1, float d2, float k )
{
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h);
}

float op_smooth_subtraction( float d1, float d2, float k )
{
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h);
}

float op_smooth_intersection( float d1, float d2, float k )
{
    float h = clamp( 0.5 - 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) + k*h*(1.0-h);
}

//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////
//////////////////////////////



