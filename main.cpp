#include "engine_core.h"
#include "engine_utility.h"
#include "display_core.h"
#include "scripting/scripting_core.h"

i32 main()
{

	as::display_handle display_handle;
	as::create_display_handle(false, 800, 600, display_handle);

	as::configure();

	as::script::entity entity;
	as::script::parse_file("resources/objects/viking_room_model.json", false, entity);


	//as::shader shader;
	//as::create_shader_from_files("resources/shaders/default_vertex_shader.glsl", "resources/shaders/default_fragment_shader.glsl", shader);
	as::model testing_model = *static_cast<as::model*>(entity.data_ptr); // issue with the mesh data being null (copying problem)



	//as::texture texture;
	//as::load_texture("resources/textures/viking_room.png", texture);
	//strcpy(texture.uniform_name, "uniform_texture");

	//as::add_textures_to_shader({texture}, shader);

	//as::model testing_model;
	//std::vector<as::texture> testing_textures;
	//load_model("resources/models/viking_room.obj", testing_model, testing_textures);

	//as::assign_shader(shader, testing_model.meshes);

	as::camera camera;
	as::update_camera_vectors(camera);
	camera.position -= camera.front * 5.f;
	camera.aspect_ratio = as::get_display_ratio(display_handle);

	while (as::should_display_loop(display_handle))
	{
		as::process_input(display_handle);
		as::clear_background();
		as::draw({ testing_model }, camera);
		as::update(display_handle);
	}

	as::script::clean(entity);
	//as::delete_shader(shader);
	as::delete_model_data(testing_model);
	//as::delete_shader_program(shader_program);
	as::terminate_display();
}