#include "engine_core.h"
#include "engine_utility.h"
#include "display_core.h"
#include "input/input_types.h"


i32 main()
{

	as::input_handle input;
	as::display_handle display_handle;
	as::create_display_handle(false, 800, 600, input, display_handle);

	as::configure();

	as::entity* entity = nullptr;
	as::parse_file("resources/objects/default_world.json", false, entity);
	
	as::world* world = nullptr;
	as::get_world_from_entity(entity, world);

	AS_LOG(LV_LOG, as::to_string(entity));
	input.on_receive_input = [&entity, &world](const as::key_params& params)
	{
		if (params.type == as::Z)
		{
			
		}
		if (params.input_event == as::PRESSED && params.type == as::R)
		{
			AS_LOG(LV_LOG, as::to_string(entity));
			if (as::parse_file("resources/objects/default_world.json", false, entity))
			{
				as::get_world_from_entity(entity, world);
			}
		}
	};

	while (as::should_display_loop(display_handle))
	{
		// next step figure out the file refresh and cash
		//as::process_input(display_handle);
		as::clear_background();
		as::draw(world, as::get_display_ratio(display_handle));
		as::update(display_handle);
	}

	as::delete_entity_data(entity);
	as::terminate_display();
}