#include "vulkan_engine.h"

int main() 
{
	as::window window;
	as::vk::engine engine;
	as::init_window(window, 800, 600);

	as::vk::init_vulkan(engine, window, 2);

	as::vk::object_data viking_room;
	as::vk::add_object(engine, "models/viking_room.obj", viking_room);
	
	as::vk::object_data bunny;
	as::vk::add_object(engine, "models/stanford-bunny.obj", bunny);
	
	as::vk::start_main_loop(engine, window);

	as::vk::cleanup(engine, window);

	return EXIT_SUCCESS;
}  