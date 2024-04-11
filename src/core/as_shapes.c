#include "core/as_shapes.h"
#include "as_memory.h"

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

as_shape* as_generate_quad() 
{
    as_shape* quad = AS_MALLOC_SINGLE(as_shape);
    quad->vertices_size = 4;
    quad->indices_size = 6;

    quad->vertices[0].position = (as_vec3){-0.5f, -0.5f, 0.0f};
    quad->vertices[0].normal = (as_vec3){0.0f, 0.0f, 1.0f};
    quad->vertices[0].color = (as_vec3){1.0f, 0.0f, 0.0f};
    quad->vertices[0].tex_coord = (as_vec2){0.0f, 0.0f};

    quad->vertices[1].position = (as_vec3){0.5f, -0.5f, 0.0f};
    quad->vertices[1].normal = (as_vec3){0.0f, 0.0f, 1.0f};
    quad->vertices[1].color = (as_vec3){0.0f, 1.0f, 0.0f};
    quad->vertices[1].tex_coord = (as_vec2){1.0f, 0.0f};

    quad->vertices[2].position = (as_vec3){0.5f, 0.5f, 0.0f};
    quad->vertices[2].normal = (as_vec3){0.0f, 0.0f, 1.0f};
    quad->vertices[2].color = (as_vec3){0.0f, 0.0f, 1.0f};
    quad->vertices[2].tex_coord = (as_vec2){1.0f, 1.0f};

    quad->vertices[3].position = (as_vec3){-0.5f, 0.5f, 0.0f};
    quad->vertices[3].normal = (as_vec3){0.0f, 0.0f, 1.0f};
    quad->vertices[3].color = (as_vec3){0.5f, 0.5f, 0.5f};
    quad->vertices[3].tex_coord = (as_vec2){0.0f, 1.0f};

    quad->indices[0] = 0;
    quad->indices[1] = 1;
    quad->indices[2] = 2;
    quad->indices[3] = 2;
    quad->indices[4] = 3;
    quad->indices[5] = 0;

	AS_SET_VALID(quad);
    return quad;
}

as_shape* as_generate_cube()
{
    as_shape* cube = AS_MALLOC_SINGLE(as_shape);
    cube->vertices_size = 24;
    cube->indices_size = 36;

    as_vec3 vertices[] = {
        {-0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f},
        {-0.5f, -0.5f, 0.5f},
        {0.5f, -0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {-0.5f, 0.5f, 0.5f},
    };

    u32 indices[] = {
        2, 1, 0, 3, 2, 0,   // Front face (reverted)
        6, 5, 4, 7, 6, 4,   // Back face (reverted)
        0, 3, 7, 7, 4, 0,   // Left face (inverted)
        1, 5, 6, 6, 2, 1,   // Right face (inverted)
        1, 0, 4, 4, 5, 1,   // Bottom face (reverted)
        7, 3, 2, 2, 6, 7    // Top face (reverted)
    };

//  u32 indices[] = {
//         0, 1, 2, 2, 3, 0,
//         4, 5, 6, 6, 7, 4,
//         0, 4, 7, 7, 3, 0,
//         1, 5, 6, 6, 2, 1,
//         0, 1, 5, 5, 4, 0,
//         2, 3, 7, 7, 6, 2,
//     };

    for (sz i = 0; i < 8; i++)
    {
        cube->vertices[i].position = vertices[i];
        cube->vertices[i].normal = as_vec3_get_normal(&vertices[i]);
        cube->vertices[i].color = (as_vec3){1.0f, 1.0f, 1.0f};
        cube->vertices[i].tex_coord = (as_vec2){0.0f, 0.0f};
    }

    for (sz i = 8; i < 24; i += 4)
    {
        cube->vertices[i] = cube->vertices[i - 8];
        cube->vertices[i + 1] = cube->vertices[i - 7];
        cube->vertices[i + 2] = cube->vertices[i - 6];
        cube->vertices[i + 3] = cube->vertices[i - 5];
    }

    // Set indices
    for (sz i = 0; i < 36; i++)
    {
        cube->indices[i] = indices[i];
    }

	AS_SET_VALID(cube);
	return cube;
}

as_shape* as_generate_box(const f32 x_extent, const f32 y_extent, const f32 z_extent)
{
	as_shape* box = AS_MALLOC_SINGLE(as_shape);
	box->vertices_size = 24;
    box->indices_size = 36;

    float x_half = x_extent * 0.5f;
    float y_half = y_extent * 0.5f;
    float z_half = z_extent * 0.5f;

    as_vec3 vertices[] = {
        {-x_half, -y_half, -z_half},
        {x_half, -y_half, -z_half},
        {x_half, y_half, -z_half},
        {-x_half, y_half, -z_half},
        {-x_half, -y_half, z_half},
        {x_half, -y_half, z_half},
        {x_half, y_half, z_half},
        {-x_half, y_half, z_half},
    };

    uint32_t indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 4, 7, 7, 3, 0,
        1, 5, 6, 6, 2, 1,
        0, 1, 5, 5, 4, 0,
        2, 3, 7, 7, 6, 2,
    };

    for (int i = 0; i < 8; i++)
    {
        box->vertices[i].position = vertices[i];
        box->vertices[i].normal = as_vec3_get_normal(&vertices[i]);
        box->vertices[i].color = (as_vec3){1.0f, 1.0f, 1.0f};
        box->vertices[i].tex_coord = (as_vec2){0.0f, 0.0f};
    }

    for (int i = 8; i < 24; i += 4)
    {
        box->vertices[i] = box->vertices[i - 8];
        box->vertices[i + 1] = box->vertices[i - 7];
        box->vertices[i + 2] = box->vertices[i - 6];
        box->vertices[i + 3] = box->vertices[i - 5];
    }

    // Set indices
    for (int i = 0; i < 36; i++)
    {
        box->indices[i] = indices[i];
    }

	AS_SET_VALID(box);
	return box;
}

as_shape* as_generate_sphere(const f32 radius, const i32 latitude_divisions, const i32 longitude_divisions) 
{
    as_shape* sphere = AS_MALLOC_SINGLE(as_shape);
    sz vertex_index = 0;
    sz index_index = 0;

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

            sphere->vertices[vertex_index].position.x = x * radius;
            sphere->vertices[vertex_index].position.y = y * radius;
            sphere->vertices[vertex_index].position.z = z * radius;

            sphere->vertices[vertex_index].normal.x = x;
            sphere->vertices[vertex_index].normal.y = y;
            sphere->vertices[vertex_index].normal.z = z;

            sphere->vertices[vertex_index].color.x = 1.0f;
            sphere->vertices[vertex_index].color.y = 1.0f;
            sphere->vertices[vertex_index].color.z = 1.0f;

            sphere->vertices[vertex_index].tex_coord.x = (f32)lon / longitude_divisions;
            sphere->vertices[vertex_index].tex_coord.y = (f32)lat / latitude_divisions;

            vertex_index++;
        }
    }

    for (i32 lat = 0; lat < latitude_divisions; lat++) 
    {
        for (i32 lon = 0; lon < longitude_divisions; lon++) 
        {
            i32 current = lat * (longitude_divisions + 1) + lon;
            i32 next = current + longitude_divisions + 1;

            sphere->indices[index_index++] = current;
            sphere->indices[index_index++] = next;
            sphere->indices[index_index++] = current + 1;

            sphere->indices[index_index++] = next;
            sphere->indices[index_index++] = next + 1;
            sphere->indices[index_index++] = current + 1;
        }
    }

    sphere->vertices_size = vertex_index;
    sphere->indices_size = index_index;

    AS_SET_VALID(sphere);
    return sphere;
}

void as_destroy_shape(as_shape* shape)
{
    AS_ASSERT(shape, "Cannot destroy shape, already null");
    AS_FREE(shape);
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
