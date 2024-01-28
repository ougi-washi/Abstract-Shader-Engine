#pragma once

#include "core/as_render.h"


typedef struct as_shape
{
	as_vertex vertices;
	u16 indices;
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
extern const as_vertex as_shape_sphere_vertices[];
extern const u16 as_shape_sphere_indices[];
extern const i32 as_shape_sphere_vertices_size;
extern const i32 as_shape_sphere_indices_size;

// QUAD
extern const as_vertex as_shape_multi_quad_vertices[];
extern const i32 as_shape_multi_quad_vertices_size;
extern const u16 as_shape_multi_quad_indices[];
extern const i32 as_shape_multi_quad_indices_size;