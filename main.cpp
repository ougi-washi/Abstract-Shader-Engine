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

	//as::texture texture;
	//as::load_texture("resources/textures/container.jpg", texture);
	//strcpy(texture.uniform_name, "uniform_texture");

	as::texture texture;
	as::load_texture("resources/textures/viking_room.png", texture);
	strcpy(texture.uniform_name, "uniform_texture");

	as::add_textures_to_shader({texture}, shader);
	
	as::object object;
	as::model testing_model;
	std::vector<as::texture> testing_textures;
	load_model("resources/models/viking_room.obj", testing_model, testing_textures);

	for (u32 i = 0 ; i < testing_model.meshes.size() ; i++)
	{
		as::assign_shader(shader, testing_model.meshes[i]);
	}
	//as::set_uniform_integer(shader_program, "uniform_texture", 0);
	//as::set_uniform_integer(shader_program, "uniform_texture1", 1);

	object.models.push_back(&testing_model);

	as::camera camera;
	as::update_camera_vectors(camera);
	camera.position -= camera.front * 5.f;

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
		//model = glm::rotate(model, glm::radians(120.f), glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, glm::radians(-40.f), glm::vec3(0., 1.f, 0.f));
		model = glm::rotate(model, (f32)as::timer::get_current_time(timer_handle) * glm::radians(30.f), glm::vec3(0.f, 0.f, 1.f));
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
	as::delete_model_data(testing_model);
	as::delete_shader_program(shader_program);
	as::terminate_display();
}