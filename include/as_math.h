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

typedef struct as_int_vec2
{
	i32 x, y;
} as_int_vec2;

typedef struct as_mat4
{
	f32 m[4][4];
} as_mat4;

typedef as_mat4 as_transform;

// float
extern f32 as_radians(f32 degrees);

// vec3
extern void as_vec3_normalize(as_vec3* v);
extern f32 as_vec3_dot(const as_vec3* a, const as_vec3* b);
extern as_vec3 as_vec3_unit_z();

// mat4
extern as_mat4 as_mat4_identity();
extern void as_translate(as_mat4* m, const as_vec3* translation);
extern as_mat4 as_rotate(const as_mat4* m, const f32 angle, const as_vec3* v);
extern as_mat4 as_look_at(const as_vec3* eye, const as_vec3* center, const as_vec3* up);
extern as_mat4 as_perspective(const f32 fov, const f32 aspect, const f32 near_plane, const f32 far_plane);