#include "engine_core.h"
#include "display_core.h"

i32 main()
{
	as::display_handle display_handle;
	as::create_display_handle(false, 800, 600, display_handle);
	
	as::shader shader;
	as::create_shader_from_files("resources/shaders/default_vertex_shader.glsl", "resources/shaders/default_fragment_shader.glsl", shader);

	u32 shader_program;
	as::create_shader_program(shader_program);
	as::bind_shaders_to_program(shader_program, shader);
	
	as::texture texture;
	as::load_texture("resources/textures/container.jpg", texture);

	as::uniform position_uniform;
	position_uniform.id = 0;
	position_uniform.size = 3;
	position_uniform.type = as::variable_type::FLOAT;
	position_uniform.normalized = 0;
	position_uniform.stride = 8 * sizeof(f32);
	position_uniform.position = 0;
	
	as::uniform colors_uniform;
	colors_uniform.id = 1;
	colors_uniform.size = 3;
	colors_uniform.type = as::variable_type::FLOAT;
	colors_uniform.normalized = 0;
	colors_uniform.stride = 8 * sizeof(f32);
	colors_uniform.position = 3 * sizeof(f32);

	as::uniform tex_coord_uniform;
	tex_coord_uniform.id = 2;
	tex_coord_uniform.size = 2;
	tex_coord_uniform.type = as::variable_type::FLOAT;
	tex_coord_uniform.normalized = false;
	tex_coord_uniform.stride = 8 * sizeof(f32);
	tex_coord_uniform.position = 6 * sizeof(f32);
	tex_coord_uniform.texture_ptr = &texture;

	as::add_uniform(position_uniform, shader);
	as::add_uniform(colors_uniform, shader);
	as::add_uniform(tex_coord_uniform, shader);


	const f32 vertices[] = 
	{
		// positions          // colors           // texture coordinates
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
	};
	const u32 vertices_count = 32;
	
	const i32 indices[] = 
	{
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	const u32 indices_count = 6;

	u32 VAO; // Vertex Array Object
	as::object triangle;

	as::initialize_object(vertices, vertices_count, indices, indices_count, VAO, triangle);
	as::assign_shader(triangle, VAO, shader);
	as::set_uniform_integer(shader_program, "uniform_texture", texture.id);

	while (as::should_display_loop(display_handle))
	{
		as::process_input(display_handle);
		as::clear_background();
		as::draw(shader_program, VAO, { triangle });
		as::update(display_handle);
	}

	as::delete_vertex_array(VAO);
	as::delete_shader(shader);
	as::delete_object_data(&triangle);
	as::delete_shader_program(shader_program);
	as::terminate_display();
}