#pragma once

#include "core/as_render.h"

#define AS_MAX_VERTICES_SIZE 2048
#define AS_MAX_INDICES_SIZE 2048

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

// SPHERE
extern as_shape shape_sphere_10;

// QUAD
extern const as_vertex as_shape_multi_quad_vertices[];
extern const i32 as_shape_multi_quad_vertices_size;
extern const u16 as_shape_multi_quad_indices[];
extern const i32 as_shape_multi_quad_indices_size;