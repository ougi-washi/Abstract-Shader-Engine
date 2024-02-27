// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"
#include "as_math.h"
#include "core/as_shapes.h"

typedef struct as_shader_serialized
{
	char filename_vertex[AS_MAX_PATH_SIZE];
	char filename_fragment[AS_MAX_PATH_SIZE];

	//struct as_shader_uniforms_32 uniforms; resolve this later
} as_shader_serialized;

typedef struct as_object_serialized
{
	as_transform transform;
	u32 instance_count;
	as_shape shape;
	as_shader_serialized shader;
	i32 scene_gpu_index; // index of the object in the GPU scene 

} as_object_serialized;

typedef struct as_light_serialized
{
	as_vec3 position;
	as_vec3 color;
	f32 radius;

} as_light;

typedef struct as_scene_serialized 
{
	
} as_scene_serialized;