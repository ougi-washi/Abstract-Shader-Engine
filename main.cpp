#include "engine_core.h"

i32 main()
{
	i32 screenWidth = 800;
	i32 screenHeight = 450;

	//InitWindow(screenWidth, screenHeight, "Abstract Shader Engine");
	as::init_window(screenWidth, screenHeight, "Abstract Shader Engine");
	as::entity* out_entity = nullptr;
	parse_file("resources/objects/dragon_world.json", false, out_entity);
	as::world* out_world = nullptr;
	get_world_from_entity(out_entity, out_world);
	while (!WindowShouldClose())
	{
		if (out_world)
		{
			draw(out_world);
		}
	}
	return 0;
}