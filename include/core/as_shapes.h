#pragma once

#include "as_types.h"
#include "as_math.h"

#define AS_MAX_VERTICES_SIZE 2048
#define AS_MAX_INDICES_SIZE 2048

 // use of _Alignof may be needed somewhere here
typedef struct as_vertex
{
	as_vec3 position;
	as_vec3 normal;
	as_vec3 color;
	as_vec2 tex_coord;
} as_vertex;
#define AS_VERTEX_VAR_COUNT 4

typedef struct as_shape
{
	as_vertex vertices[AS_MAX_VERTICES_SIZE];
	u16 indices[AS_MAX_INDICES_SIZE];
	sz vertices_size;
	sz indices_size;
} as_shape;

// TRIANGLE
extern const as_vertex as_shape_triangle_vertices[];
extern const u16 as_shape_triangle_indices[];
extern const i32 as_shape_triangle_vertices_size;
extern const i32 as_shape_triangle_indices_size;

// QUAD
extern const as_vertex as_shape_quad_vertices[];
extern const i32 as_shape_quad_vertices_size;
extern const u16 as_shape_quad_indices[];
extern const i32 as_shape_quad_indices_size;

extern as_shape as_generate_triangle();
extern as_shape as_generate_quad();
extern as_shape as_generate_sphere(const f32 radius, const i32 latitude_divisions, const i32 longitude_divisions);
