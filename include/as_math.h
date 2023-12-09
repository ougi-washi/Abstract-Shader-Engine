// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include <as_types.h>

// TODO : SIMD
typedef struct as_vec3
{
	f32 x, y, z;
} as_vec3;

typedef struct as_int_vec2
{
	i32 x, y;
} as_int_vec2;

typedef struct as_transform
{
	f32 M[4][4];
} as_transform;