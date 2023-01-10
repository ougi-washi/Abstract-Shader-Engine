#include "engine_core.h"
#include "engine_utility.h"
#include "display_core.h"

i32 main()
{
	as::display_handle display_handle;
	as::create_display_handle(false, 800, 600, display_handle);

	as::configure();

	as::shader shader;
	as::create_shader_from_files("resources/shaders/default_vertex_shader.glsl", "resources/shaders/default_fragment_shader.glsl", shader);

	u32 shader_program;
	as::create_shader_program(shader_program);
	as::bind_shaders_to_program(shader_program, shader);
	
	as::texture texture;
	as::load_texture("resources/textures/container.jpg", texture);
	strcpy(texture.uniform_name, "uniform_texture");

	as::texture texture1;
	as::load_texture("resources/textures/viking_room.png", texture1);
	strcpy(texture1.uniform_name, "uniform_texture1");

	shader.textures.push_back(&texture);
	shader.textures.push_back(&texture1);

	as::object object;
	as::model bunny;
	std::vector<as::texture> bunny_textures;
	load_model("resources/models/stanford-bunny.obj", bunny, bunny_textures);

	//as::create_mesh(vertices, sizeof(vertices), indices, sizeof(indices), VAO, triangle);
	for (u32 i = 0 ; i < bunny.meshes.size() ; i++)
	{
		as::assign_shader(shader, bunny.meshes[i]);
	}
	as::set_uniform_integer(shader_program, "uniform_texture", 0);
	as::set_uniform_integer(shader_program, "uniform_texture1", 1);

	object.models.push_back(&bunny);

	as::camera camera;
	as::update_camera_vectors(camera);
	camera.position -= camera.front * 1.f;

	as::timer::handle timer_handle;
	as::timer::start_timer(timer_handle);

	while (as::should_display_loop(display_handle))
	{
		as::process_input(display_handle);
		as::clear_background();
		// create transformations
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = as::get_matrix_view(camera);
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), as::get_display_ratio(display_handle), 0.1f, 100.0f);
		model = glm::rotate(model, (f32)as::timer::get_current_time(timer_handle) * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
		// pass transformation matrices to the shader
		as::set_uniform_mat4(shader_program, "projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		as::set_uniform_mat4(shader_program, "view", view);
		as::set_uniform_mat4(shader_program, "model", model);

		//camera.position.x -= display_handle.delta_time;
		as::update_camera_vectors(camera);
		as::draw({ object });
		as::update(display_handle);
	}

	as::delete_shader(shader);
	as::delete_model_data(bunny);
	as::delete_shader_program(shader_program);
	as::terminate_display();
}