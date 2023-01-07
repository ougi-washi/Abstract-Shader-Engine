#include "engine_core.h"
#include "engine_utility.h"
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

	as::texture texture1;
	as::load_texture("resources/textures/viking_room.png", texture1);

	shader.textures.push_back(&texture);
	shader.textures.push_back(&texture1);

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

	as::add_uniform(position_uniform, shader);
	as::add_uniform(colors_uniform, shader);
	as::add_uniform(tex_coord_uniform, shader);

	f32 vertices[] = 
	{
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,		// top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,		// bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,		// bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f		// top left 
	};
	u32 indices[] = 
	{
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	u32 VAO; // Vertex Array Object
	as::object triangle;

	as::initialize_object(vertices, sizeof(vertices), indices, sizeof(indices), VAO, triangle);
	as::assign_shader(triangle, VAO, shader);
	as::set_uniform_integer(shader_program, "uniform_texture", 0);
	as::set_uniform_integer(shader_program, "uniform_texture1", 1);

	as::timer::handle timer_handle;
	as::timer::start_timer(timer_handle);

	while (as::should_display_loop(display_handle))
	{
		as::process_input(display_handle);
		as::clear_background();
		glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));
		transform = glm::rotate(transform, (f32)as::timer::get_current_time(timer_handle), glm::vec3(0.0f, 0.0f, 1.0f));
		as::set_uniform_mat4(shader_program, "transform", transform);
		as::draw(shader_program, VAO, { triangle });
		as::update(display_handle);
	}

	as::delete_vertex_array(VAO);
	as::delete_shader(shader);
	as::delete_object_data(&triangle);
	as::delete_shader_program(shader_program);
	as::terminate_display();
}