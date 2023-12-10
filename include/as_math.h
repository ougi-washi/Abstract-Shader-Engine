// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include <as_types.h>

// TODO : SIMD

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
	f32 M[4][4];
} as_mat4;

typedef as_mat4 as_transform;


#define AS_CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))