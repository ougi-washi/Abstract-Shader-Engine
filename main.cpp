#include "vulkan_engine.h"

int main() 
{
	as::window window;
	as::vk::engine engine;
	as::init_window(window);
	as::vk::init_vulkan(engine, window);
	as::vk::start_main_loop(engine, window);
	as::vk::cleanup(engine, window);

	return EXIT_SUCCESS;
}  