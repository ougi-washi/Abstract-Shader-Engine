#include "vulkan_engine.h"

int main() 
{
	as::window window;
	as::vk::engine engine;
	as::init_window(window, 800, 600);
	as::vk::init_vulkan(engine, window, 2);
	as::vk::start_main_loop(engine, window);
	as::vk::cleanup(engine, window);

	return EXIT_SUCCESS;
}  