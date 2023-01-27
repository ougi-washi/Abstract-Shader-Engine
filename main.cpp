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

	as::camera camera;
	as::update_camera_vectors(camera);
	camera.position -= camera.front * 5.f;
	camera.aspect_ratio = as::get_display_ratio(display_handle);

	while (as::should_display_loop(display_handle))
	{
		as::process_input(display_handle);
		as::clear_background();
		as::script::draw({entity}, camera);
		as::update(display_handle);
	}

	as::script::delete_entity_data(entity);
	as::terminate_display();
}