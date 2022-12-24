#include "vulkan_engine.h"

int main() 
{
	as::window window;
	as::vk::engine engine;
	as::init_window(window, 800, 600);

	as::vk::init_vulkan(engine, window, 2);

	//as::vk::texture_data viking_texture;
	//as::vk::add_texture(engine, "textures/viking_room.png", viking_texture);
	//as::vk::material_data viking_material;
	//viking_material.textures.push_back(&viking_texture);
	//as::vk::add_material(engine, "shaders/shader.vert", "shaders/shader.frag", viking_material);
	//as::vk::object_data viking_room;
	//viking_room.material = &viking_material;
	//as::vk::add_object(engine, "models/viking_room.obj", viking_room);

	as::vk::material_data bunny_material;
	add_material(engine, default_frag_shader_path, bunny_material);
	as::vk::object_data bunny;
	bunny.material = &bunny_material;
	as::vk::add_object(engine, "models/stanford-bunny.obj", bunny);

	create_graphics_pipeline(engine);
	as::vk::start_main_loop(engine, window);

	as::vk::cleanup(engine, window);

	return EXIT_SUCCESS;
}  