#include "core/as_shapes.h"

as_shape as_generate_triangle() 
{
    as_shape triangle;
    triangle.vertices_size = 3;
    triangle.indices_size = 3;

    triangle.vertices[0].position = (as_vec3){-0.5f, -0.5f, 0.0f};
    triangle.vertices[0].normal = (as_vec3){0.0f, 0.0f, 1.0f};
    triangle.vertices[0].color = (as_vec3){1.0f, 0.0f, 0.0f};
    triangle.vertices[0].tex_coord = (as_vec2){0.0f, 0.0f};

    triangle.vertices[1].position = (as_vec3){0.5f, -0.5f, 0.0f};
    triangle.vertices[1].normal = (as_vec3){0.0f, 0.0f, 1.0f};
    triangle.vertices[1].color = (as_vec3){0.0f, 1.0f, 0.0f};
    triangle.vertices[1].tex_coord = (as_vec2){1.0f, 0.0f};

    triangle.vertices[2].position = (as_vec3){0.0f, 0.5f, 0.0f};
    triangle.vertices[2].normal = (as_vec3){0.0f, 0.0f, 1.0f};
    triangle.vertices[2].color = (as_vec3){0.0f, 0.0f, 1.0f};
    triangle.vertices[2].tex_coord = (as_vec2){0.5f, 1.0f};

    triangle.indices[0] = 0;
    triangle.indices[1] = 1;
    triangle.indices[2] = 2;

    return triangle;
}

as_shape as_generate_quad() 
{
    as_shape quad;
    quad.vertices_size = 4;
    quad.indices_size = 6;

    quad.vertices[0].position = (as_vec3){-0.5f, -0.5f, 0.0f};
    quad.vertices[0].normal = (as_vec3){0.0f, 0.0f, 1.0f};
    quad.vertices[0].color = (as_vec3){1.0f, 0.0f, 0.0f};
    quad.vertices[0].tex_coord = (as_vec2){0.0f, 0.0f};

    quad.vertices[1].position = (as_vec3){0.5f, -0.5f, 0.0f};
    quad.vertices[1].normal = (as_vec3){0.0f, 0.0f, 1.0f};
    quad.vertices[1].color = (as_vec3){0.0f, 1.0f, 0.0f};
    quad.vertices[1].tex_coord = (as_vec2){1.0f, 0.0f};

    quad.vertices[2].position = (as_vec3){0.5f, 0.5f, 0.0f};
    quad.vertices[2].normal = (as_vec3){0.0f, 0.0f, 1.0f};
    quad.vertices[2].color = (as_vec3){0.0f, 0.0f, 1.0f};
    quad.vertices[2].tex_coord = (as_vec2){1.0f, 1.0f};

    quad.vertices[3].position = (as_vec3){-0.5f, 0.5f, 0.0f};
    quad.vertices[3].normal = (as_vec3){0.0f, 0.0f, 1.0f};
    quad.vertices[3].color = (as_vec3){0.5f, 0.5f, 0.5f};
    quad.vertices[3].tex_coord = (as_vec2){0.0f, 1.0f};

    quad.indices[0] = 0;
    quad.indices[1] = 1;
    quad.indices[2] = 2;
    quad.indices[3] = 2;
    quad.indices[4] = 3;
    quad.indices[5] = 0;

    return quad;
}

extern as_shape as_generate_triangle(); 
extern as_shape as_generate_quad();
as_shape as_generate_sphere(const f32 radius, const i32 latitude_divisions, const i32 longitude_divisions) 
{
    as_shape sphere;
    size_t vertex_index = 0;
    size_t index_index = 0;

    for (i32 lat = 0; lat <= latitude_divisions; lat++) 
    {
        f32 theta = lat * AS_PI / latitude_divisions;
        f32 sin_theta = sin(theta);
        f32 cos_theta = cos(theta);

        for (i32 lon = 0; lon <= longitude_divisions; lon++) 
        {
            f32 phi = lon * 2 * AS_PI / longitude_divisions;
            f32 sin_phi = sin(phi);
            f32 cos_phi = cos(phi);

            f32 x = cos_phi * sin_theta;
            f32 y = cos_theta;
            f32 z = sin_phi * sin_theta;

            sphere.vertices[vertex_index].position.x = x * radius;
            sphere.vertices[vertex_index].position.y = y * radius;
            sphere.vertices[vertex_index].position.z = z * radius;

            sphere.vertices[vertex_index].normal.x = x;
            sphere.vertices[vertex_index].normal.y = y;
            sphere.vertices[vertex_index].normal.z = z;

            sphere.vertices[vertex_index].color.x = 1.0f;
            sphere.vertices[vertex_index].color.y = 1.0f;
            sphere.vertices[vertex_index].color.z = 1.0f;

            sphere.vertices[vertex_index].tex_coord.x = (f32)lon / longitude_divisions;
            sphere.vertices[vertex_index].tex_coord.y = (f32)lat / latitude_divisions;

            vertex_index++;
        }
    }

    for (i32 lat = 0; lat < latitude_divisions; lat++) 
    {
        for (i32 lon = 0; lon < longitude_divisions; lon++) 
        {
            i32 current = lat * (longitude_divisions + 1) + lon;
            i32 next = current + longitude_divisions + 1;

            sphere.indices[index_index++] = current;
            sphere.indices[index_index++] = next;
            sphere.indices[index_index++] = current + 1;

            sphere.indices[index_index++] = next;
            sphere.indices[index_index++] = next + 1;
            sphere.indices[index_index++] = current + 1;
        }
    }

    sphere.vertices_size = vertex_index;
    sphere.indices_size = index_index;

    return sphere;
}


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
