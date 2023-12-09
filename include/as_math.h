// MMO_ECS_SERVER - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include <as_types.h>

// TODO : SIMD
typedef struct as_vec3
{
	f32 x, y, z;

} as_vec3;

typedef struct as_transform
{
	as_vec3 translation;
	as_vec3 rotation;
	as_vec3 scale;

} as_transform;