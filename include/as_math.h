// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include <as_types.h>

// TODO : SIMD

#define AS_PI 3.14159265358979323846f
#define AS_CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

typedef struct as_vec2
{
	f32 x, y, z;
} as_vec2;

typedef struct as_vec3
{
	f32 x, y, z;
} as_vec3;

typedef struct as_vec4
{
	f32 x, y, z, w;
} as_vec4;

typedef struct as_int_vec2
{
	i32 x, y;
} as_int_vec2;

typedef struct as_mat4
{
	f32 m[4][4];
} as_mat4;

typedef struct as_quat
{
	f32 x, y, z, w;
} as_quat;


typedef as_mat4 as_transform;



// float
extern f32 as_radians(f32 degrees);

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
extern as_mat4 as_mat4_rotate(const as_mat4* m, const f32 angle, const as_vec3* v);
extern as_mat4 as_mat4_rotate_around_center(const as_mat4* m, const f32 angle, const as_vec3* v, const as_vec3* center);
extern void as_mat4_set_rotation(as_mat4* m, const as_vec3* rotation);

extern as_vec3 as_mat4_get_scale(const as_mat4* m);
extern void as_mat4_set_scale(as_mat4* m, const as_vec3* scale);

extern as_mat4 as_mat4_look_at(const as_vec3* eye, const as_vec3* center, const as_vec3* up);
extern as_mat4 as_mat4_perspective(const f32 fov, const f32 aspect, const f32 near_plane, const f32 far_plane);

// quat
extern as_quat as_vec3_to_quat(const as_vec3* v);
as_vec3 as_quat_to_vec3(const as_quat* q);




// Macros for performance

// 2D Vector
#define AS_VEC2(_x, _y) (as_vec2) { _x, _y }
#define AS_VEC2_PTR(_x, _y) &(as_vec2) { _x, _y }

// 3D Vector
#define AS_VEC3(_x, _y, _z) (as_vec3) { _x, _y, _z }
#define AS_VEC3_PTR(_x, _y, _z) &(as_vec3) { _x, _y, _z }
#define AS_VEC3_X_AXIS (as_vec3) { 1.0f, 0.0f, 0.0f }
#define AS_VEC3_X_AXIS_PTR &(as_vec3) { 1.0f, 0.0f, 0.0f }
#define AS_VEC3_Y_AXIS (as_vec3) { 0.0f, 1.0f, 0.0f }
#define AS_VEC3_Y_AXIS_PTR &(as_vec3) { 0.0f, 1.0f, 0.0f }
#define AS_VEC3_Z_AXIS (as_vec3) { 0.0f, 0.0f, 1.0f }
#define AS_VEC3_Z_AXIS_PTR &(as_vec3) { 0.0f, 0.0f, 1.0f }

// 4D Vector
#define AS_VEC4(_x, _y, _z, _w) (as_vec4) { _x, _y, _z, _w }
#define AS_VEC4_PTR(_x, _y, _z, _w) &(as_vec4) { _x, _y, _z, _w }

// Integer 2D Vector
#define AS_IV2(_x, _y) (as_int_vec2) { _x, _y }
#define AS_IV2_PTR(_x, _y) &(as_int_vec2) { _x, _y }

// Integer 3D Vector
#define AS_IV3(_x, _y, _z) (as_int_vec3) { _x, _y, _z }
#define AS_IV3_PTR(_x, _y, _z) &(as_int_vec3) { _x, _y, _z }

// Integer 4D Vector
#define AS_IV4(_x, _y, _z, _w) (as_int_vec4) { _x, _y, _z, _w }
#define AS_IV4_PTR(_x, _y, _z, _w) &(as_int_vec4) { _x, _y, _z, _w }

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
