#include "core/as_shapes.h"

// QUAD
const as_vertex as_shape_quad_vertices[] =
{	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}} };
const i32 as_shape_quad_indices[] = { 0, 1, 2, 2, 3, 0 };
const i32 as_shape_quad_vertices_size = AS_ARRAY_SIZE(as_shape_quad_vertices);
const i32 as_shape_quad_indices_size = AS_ARRAY_SIZE(as_shape_quad_indices);
