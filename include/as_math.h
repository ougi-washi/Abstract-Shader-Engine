// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include <as_types.h>
#include <math.h>

// TODO : SIMD

#define AS_PI 3.14159265358979323846f
#define AS_CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

#define AS_VEC_GENERATE_OP(_type, _operation, _size, _function_name) \
static inline void _function_name (_type* _result, const _type* _val1, const _type* _val2) { \
    for (u8 i = 0; i < _size; ++i) { (_result)->data[i] = (_val1)->data[i] _operation (_val2)->data[i]; }  \
}

#define AS_VEC_GENERATE_NEGATE(_output_type, _input_type, _size, _function_name) \
static inline _output_type* _function_name (_input_type* _val) { \
    for (u8 i = 0; i < _size; ++i) { (_val)->data[i] = - (_val)->data[i]; } \
    return _val;\
}

#define AS_VEC_GENERATE_ALL_OP(_type, _size)                        \
AS_VEC_GENERATE_OP(_type, +, _size, _type##_add)                    \
AS_VEC_GENERATE_OP(_type, -, _size, _type##_sub)                    \
AS_VEC_GENERATE_OP(_type, *, _size, _type##_mul)                    \
AS_VEC_GENERATE_OP(_type, /, _size, _type##_div)                    \
AS_VEC_GENERATE_NEGATE(_type, _type, _size, _type##_negate)        \
AS_VEC_GENERATE_NEGATE(const _type, _type, _size, _type##_cnegate)

#define AS_VEC_DEFINE_VECTOR(_name, _type, _size) \
typedef struct _name { \
    union { \
        _type data[_size]; \
        struct { \
            _type x; \
            _type y; \
            _type z; \
            _type w; \
        }; \
    }; \
} _name; \
AS_VEC_GENERATE_ALL_OP(_name, _size)

// Define all vectors 
AS_VEC_DEFINE_VECTOR(as_vec2, f32, 2)
AS_VEC_DEFINE_VECTOR(as_vec3, f32, 3)
AS_VEC_DEFINE_VECTOR(as_vec4, f32, 4)
AS_VEC_DEFINE_VECTOR(as_quat, f32, 4)
AS_VEC_DEFINE_VECTOR(as_ivec2, i32, 2)
AS_VEC_DEFINE_VECTOR(as_ivec3, i32, 3)
AS_VEC_DEFINE_VECTOR(as_ivec4, i32, 4)

typedef struct as_mat4
{
	f32 m[4][4];
} as_mat4;
typedef as_mat4 as_transform;

// float
extern f32 as_radians(const f32 degrees);

// vec3
extern void as_vec3_normalize(as_vec3* v);
extern f32 as_vec3_dot(const as_vec3* a, const as_vec3* b);
extern as_vec3 as_vec3_unit_z();

// mat4
extern as_mat4 as_mat4_identity();
extern void as_mat4_set_identity(as_mat4* m);
as_mat4 as_mat4_multiply(const as_mat4* a, const as_mat4* b);

extern as_vec3 as_mat4_get_translation(const as_mat4* m);
extern void as_mat4_translate(as_mat4* m, const as_vec3* translation);
extern void as_mat4_set_translation(as_mat4* m, const as_vec3* translation);

extern as_quat as_mat4_get_rotation(const as_mat4* m);
extern void  as_mat4_rotate(as_mat4* m, const f32 angle, const as_vec3* axis);
extern void as_mat4_rotate_around_pivot(as_mat4* m, const f32 angle, const as_vec3* axis, const as_vec3* pivot);
extern void as_mat4_rotate_with_quat(as_mat4* m, const as_quat* q);
extern void as_mat4_set_rotation(as_mat4* m, const as_vec3* rotation);
extern as_vec3 as_mat4_multiply_vec3(const as_mat4* m, const as_vec3* v);
extern as_vec4 as_mat4_multiply_vec4(const as_mat4* m, const as_vec4* v);
extern as_vec3 as_mat4_get_scale(const as_mat4* m);
extern void as_mat4_set_scale(as_mat4* m, const as_vec3* scale);

extern as_mat4 as_mat4_look_at(const as_vec3* eye, const as_vec3* center, const as_vec3* up);
extern as_mat4 as_mat4_perspective(const f32 fov, const f32 aspect, const f32 near_plane, const f32 far_plane);

// quat
extern as_quat as_vec3_to_quat(const as_vec3* v);
extern as_vec3 as_quat_to_vec3(const as_quat* q);
extern void as_quat_rotate(as_quat* q, const f32 angle, const as_vec3* axis);
extern as_quat as_mat4_to_quat(const as_mat4* m);


// Macros for performance
#define AS_VEC(_type, ...) ( _type ) { __VA_ARGS__ }
#define AS_VEC_PTR(_type, ...) &( _type ) { __VA_ARGS__ }
#define AS_VEC_ZERO_PTR(_type) &( _type ) { 0 }

// Macros for specific vectors
#define AS_VEC3_X_AXIS AS_VEC(as_vec3, 1.0f, 0.0f, 0.0f)
#define AS_VEC3_X_AXIS_PTR AS_VEC_PTR(as_vec3, 1.0f, 0.0f, 0.0f)
#define AS_VEC3_Y_AXIS AS_VEC(as_vec3, 0.0f, 1.0f, 0.0f)
#define AS_VEC3_Y_AXIS_PTR AS_VEC_PTR(as_vec3, 0.0f, 1.0f, 0.0f)
#define AS_VEC3_Z_AXIS AS_VEC(as_vec3, 0.0f, 0.0f, 1.0f)
#define AS_VEC3_Z_AXIS_PTR AS_VEC_PTR(as_vec3, 0.0f, 0.0f, 1.0f)

// Matrix 4x4
#define AS_MAT4_IDENTITY &(as_mat4) \
{{										\
    {1.0f, 0.0f, 0.0f, 0.0f},			\
    {0.0f, 1.0f, 0.0f, 0.0f},			\
    {0.0f, 0.0f, 1.0f, 0.0f},			\
    {0.0f, 0.0f, 0.0f, 1.0f}			\
}}

// Matrix Multiplication
#define AS_MAT4_MUL(m1, m2) \
{{ \
    {                                                                                                                       \
        (m1).m[0][0]*(m2).m[0][0] + (m1).m[0][1]*(m2).m[1][0] + (m1).m[0][2]*(m2).m[2][0] + (m1).m[0][3]*(m2).m[3][0],      \
        (m1).m[0][0]*(m2).m[0][1] + (m1).m[0][1]*(m2).m[1][1] + (m1).m[0][2]*(m2).m[2][1] + (m1).m[0][3]*(m2).m[3][1],      \
        (m1).m[0][0]*(m2).m[0][2] + (m1).m[0][1]*(m2).m[1][2] + (m1).m[0][2]*(m2).m[2][2] + (m1).m[0][3]*(m2).m[3][2],      \
        (m1).m[0][0]*(m2).m[0][3] + (m1).m[0][1]*(m2).m[1][3] + (m1).m[0][2]*(m2).m[2][3] + (m1).m[0][3]*(m2).m[3][3]       \
    }, \
    { \
        (m1).m[1][0]*(m2).m[0][0] + (m1).m[1][1]*(m2).m[1][0] + (m1).m[1][2]*(m2).m[2][0] + (m1).m[1][3]*(m2).m[3][0],      \
        (m1).m[1][0]*(m2).m[0][1] + (m1).m[1][1]*(m2).m[1][1] + (m1).m[1][2]*(m2).m[2][1] + (m1).m[1][3]*(m2).m[3][1],      \
        (m1).m[1][0]*(m2).m[0][2] + (m1).m[1][1]*(m2).m[1][2] + (m1).m[1][2]*(m2).m[2][2] + (m1).m[1][3]*(m2).m[3][2],      \
        (m1).m[1][0]*(m2).m[0][3] + (m1).m[1][1]*(m2).m[1][3] + (m1).m[1][2]*(m2).m[2][3] + (m1).m[1][3]*(m2).m[3][3]       \
    }, \
    { \
        (m1).m[2][0]*(m2).m[0][0] + (m1).m[2][1]*(m2).m[1][0] + (m1).m[2][2]*(m2).m[2][0] + (m1).m[2][3]*(m2).m[3][0],       \
        (m1).m[2][0]*(m2).m[0][1] + (m1).m[2][1]*(m2).m[1][1] + (m1).m[2][2]*(m2).m[2][1] + (m1).m[2][3]*(m2).m[3][1],       \
        (m1).m[2][0]*(m2).m[0][2] + (m1).m[2][1]*(m2).m[1][2] + (m1).m[2][2]*(m2).m[2][2] + (m1).m[2][3]*(m2).m[3][2],       \
        (m1).m[2][0]*(m2).m[0][3] + (m1).m[2][1]*(m2).m[1][3] + (m1).m[2][2]*(m2).m[2][3] + (m1).m[2][3]*(m2).m[3][3]        \
    }, \
    { \
        (m1).m[3][0]*(m2).m[0][0] + (m1).m[3][1]*(m2).m[1][0] + (m1).m[3][2]*(m2).m[2][0] + (m1).m[3][3]*(m2).m[3][0],       \
        (m1).m[3][0]*(m2).m[0][1] + (m1).m[3][1]*(m2).m[1][1] + (m1).m[3][2]*(m2).m[2][1] + (m1).m[3][3]*(m2).m[3][1],       \
        (m1).m[3][0]*(m2).m[0][2] + (m1).m[3][1]*(m2).m[1][2] + (m1).m[3][2]*(m2).m[2][2] + (m1).m[3][3]*(m2).m[3][2],       \
        (m1).m[3][0]*(m2).m[0][3] + (m1).m[3][1]*(m2).m[1][3] + (m1).m[3][2]*(m2).m[2][3] + (m1).m[3][3]*(m2).m[3][3]        \
    } \
}}

// Translation Matrix
#define AS_MAT4_TRANSLATION(tx, ty, tz) (as_mat4) \
{{ \
    {1.0f, 0.0f, 0.0f, tx}, \
    {0.0f, 1.0f, 0.0f, ty}, \
    {0.0f, 0.0f, 1.0f, tz}, \
    {0.0f, 0.0f, 0.0f, 1.0f} \
}}

// Scaling Matrix
#define AS_MAT4_SCALING(sx, sy, sz) (as_mat4) \
{{ \
    {sx, 0.0f, 0.0f, 0.0f}, \
    {0.0f, sy, 0.0f, 0.0f}, \
    {0.0f, 0.0f, sz, 0.0f}, \
    {0.0f, 0.0f, 0.0f, 1.0f} \
}}

// Rotation Matrix (around X-axis)
#define AS_MAT4_ROTATION_X(angle) \
{{ \
    {1.0f, 0.0f, 0.0f, 0.0f}, \
    {0.0f, cosf(angle), -sinf(angle), 0.0f}, \
    {0.0f, sinf(angle), cosf(angle), 0.0f}, \
    {0.0f, 0.0f, 0.0f, 1.0f} \
}}

// Rotation Matrix (around Y-axis)
#define AS_MAT4_ROTATION_Y(angle) \
{{ \
    {cosf(angle), 0.0f, sinf(angle), 0.0f}, \
    {0.0f, 1.0f, 0.0f, 0.0f}, \
    {-sinf(angle), 0.0f, cosf(angle), 0.0f}, \
    {0.0f, 0.0f, 0.0f, 1.0f} \
}}

// Rotation Matrix (around Z-axis)
#define AS_MAT4_ROTATION_Z(angle) \
{{ \
    {cosf(angle), -sinf(angle), 0.0f, 0.0f}, \
    {sinf(angle), cosf(angle), 0.0f, 0.0f}, \
    {0.0f, 0.0f, 1.0f, 0.0f}, \
    {0.0f, 0.0f, 0.0f, 1.0f} \
}}
