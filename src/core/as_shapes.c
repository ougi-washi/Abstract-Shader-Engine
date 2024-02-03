#include "core/as_shapes.h"

// TRIANGLE
const as_vertex as_shape_triangle_vertices[] = {
	{{-0.5f, -0.5f, 0.f},	{1.0f, 0.0f, 0.0f},	{1.0f, 0.0f}},
	{{0.5f, -0.5f, 0.f},	{0.0f, 1.0f, 0.0f},	{0.0f, 0.0f}},
	{{0.0f, 0.5f, 0.f},		{0.0f, 0.0f, 1.0f},	{0.5f, 1.0f}}
};
const u16 as_shape_triangle_indices[] = { 0, 1, 2 };
const i32 as_shape_triangle_vertices_size = AS_ARRAY_SIZE(as_shape_triangle_vertices);
const i32 as_shape_triangle_indices_size = AS_ARRAY_SIZE(as_shape_triangle_indices);

// QUAD
const as_vertex as_shape_quad_vertices[] =
{
	{{-.5f, -.5f, 0.f},	{1.f, 0.f, 0.f},	{1.f, 0.f}},
	{{.5f, -.5f, 0.f},	{0.f, 1.f, 0.f},	{0.f, 0.f}},
	{{.5f, .5f, 0.f},	{0.f, 0.f, 1.f},	{0.f, 1.f}},
	{{-.5f, .5f, 0.f},	{1.f, 1.f, 1.f},	{1.f, 1.f}}
};
const u16 as_shape_quad_indices[] = { 0, 1, 2, 2, 3, 0 };
const i32 as_shape_quad_vertices_size = AS_ARRAY_SIZE(as_shape_quad_vertices);
const i32 as_shape_quad_indices_size = AS_ARRAY_SIZE(as_shape_quad_indices);

// SPHERE
const as_vertex as_shape_sphere_vertices[] = 
{
	{{0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f}},
    {{0.309017, 0.951057, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
    {{0.587785, 0.809017, 0.f}, {0.f, 0.f, 1.f}, {0.f, 1.f}},
    {{0.809017, 0.587785, 0.f}, {1.f, 1.f, 1.f}, {1.f, 1.f}},
    {{1.f, 0.309017, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f}},
    {{0.951057, 0.f, -0.309017}, {0.f, 1.f, 0.f}, {0.f, 1.f}},
    {{0.809017, -0.587785, 0.f}, {0.f, 0.f, 1.f}, {1.f, 0.f}},
    {{0.587785, -0.809017, 0.f}, {1.f, 1.f, 1.f}, {0.f, 1.f}},
    {{0.309017, -0.951057, 0.f}, {1.f, 0.f, 0.f}, {1.f, 1.f}},
    {{0.f, -1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
};
const u16 as_shape_sphere_indices[] = {
    0, 1, 2,
    2, 3, 0,
    3, 4, 0,
    4, 5, 0,
    5, 6, 0,
    6, 7, 0,
    7, 8, 0,
    8, 9, 0,
    9, 1, 0,
    1, 2, 3,
    3, 4, 5,
    5, 6, 7,
    7, 8, 9,
    9, 1, 2,
};
const i32 as_shape_sphere_vertices_size = AS_ARRAY_SIZE(as_shape_sphere_vertices);
const i32 as_shape_sphere_indices_size = AS_ARRAY_SIZE(as_shape_sphere_indices);

as_shape shape_sphere_10 =
{
	.vertices = 
	{
		{{0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f}},
    	{{0.309017, 0.951057, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
    	{{0.587785, 0.809017, 0.f}, {0.f, 0.f, 1.f}, {0.f, 1.f}},
    	{{0.809017, 0.587785, 0.f}, {1.f, 1.f, 1.f}, {1.f, 1.f}},
    	{{1.f, 0.309017, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f}},
    	{{0.951057, 0.f, -0.309017}, {0.f, 1.f, 0.f}, {0.f, 1.f}},
    	{{0.809017, -0.587785, 0.f}, {0.f, 0.f, 1.f}, {1.f, 0.f}},
    	{{0.587785, -0.809017, 0.f}, {1.f, 1.f, 1.f}, {0.f, 1.f}},
    	{{0.309017, -0.951057, 0.f}, {1.f, 0.f, 0.f}, {1.f, 1.f}},
    	{{0.f, -1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
	},
	.indices =
	{
	    0, 1, 2,
    	2, 3, 0,
    	3, 4, 0,
    	4, 5, 0,
    	5, 6, 0,
    	6, 7, 0,
    	7, 8, 0,
    	8, 9, 0,
    	9, 1, 0,
    	1, 2, 3,
    	3, 4, 5,
    	5, 6, 7,
    	7, 8, 9,
    	9, 1, 2,
	},
	.vertices_size = 10,
	.indices_size = 42
};

// MULTI-QUAD (for depth testing)
const as_vertex as_shape_multi_quad_vertices[] =
{
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};
const u16 as_shape_multi_quad_indices[] = 
{
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};
const i32 as_shape_multi_quad_vertices_size = AS_ARRAY_SIZE(as_shape_multi_quad_vertices);
const i32 as_shape_multi_quad_indices_size = AS_ARRAY_SIZE(as_shape_multi_quad_indices);