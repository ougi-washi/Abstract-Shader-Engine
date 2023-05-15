#define AS_USE_RAYLIB

#include "engine_core.h"

i32 main()
{
	i32 screenWidth = 800;
	i32 screenHeight = 450;

	//InitWindow(screenWidth, screenHeight, "Abstract Shader Engine");
	as::init_window(screenWidth, screenHeight, "Abstract Shader Engine")

	while (!WindowShouldClose())
	{
	}
	CloseWindow();
	return 0;
}