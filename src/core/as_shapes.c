#include "core/as_shapes.h"

// TRIANGLE
const as_vertex as_shape_triangle_vertices[] = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}}
};
const u16 as_shape_triangle_indices[] = { 0, 1, 2 };
const i32 as_shape_triangle_vertices_size = AS_ARRAY_SIZE(as_shape_triangle_vertices);
const i32 as_shape_triangle_indices_size = AS_ARRAY_SIZE(as_shape_triangle_indices);

// QUAD
const as_vertex as_shape_quad_vertices[] =
{
	{{-.5f, -.5f},	{1.f, 0.f, 0.f},	{1.f, 0.f}},
	{{.5f, -.5f},	{0.f, 1.f, 0.f},	{0.f, 0.f}},
	{{.5f, .5f},	{0.f, 0.f, 1.f},	{0.f, 1.f}},
	{{-.5f, .5f},	{1.f, 1.f, 1.f},	{1.f, 1.f}} 
};
const u16 as_shape_quad_indices[] = { 0, 1, 2, 2, 3, 0 };
const i32 as_shape_quad_vertices_size = AS_ARRAY_SIZE(as_shape_quad_vertices);
const i32 as_shape_quad_indices_size = AS_ARRAY_SIZE(as_shape_quad_indices);

// SPHERE
const as_vertex as_shape_sphere_vertices[] = 
{
	{{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.5f, 1.0f}},
	{{0.707f, 0.707f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.8f, 0.8f}},
	{{0.707f, -0.707f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.5f}},
	{{0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {0.5f, 0.0f}},
	{{-0.707f, -0.707f, 0.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.8f}},
	{{-1.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}, {0.2f, 0.2f}},
	{{-0.707f, 0.707f, 0.0f}, {0.8f, 0.8f, 0.8f}, {0.7f, 0.7f}},
};
const u16 as_shape_sphere_indices[] = {
	0, 1, 2,
	2, 3, 0,
	3, 4, 5,
	5, 6, 3,
	6, 7, 0,
	0, 7, 1,
	1, 7, 2,
	2, 7, 3,
	3, 7, 4,
	4, 7, 5,
	5, 7, 6,
};

const i32 as_shape_sphere_vertices_size = AS_ARRAY_SIZE(as_shape_sphere_vertices);
const i32 as_shape_sphere_indices_size = AS_ARRAY_SIZE(as_shape_sphere_indices);
