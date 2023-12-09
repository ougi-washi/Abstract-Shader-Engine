// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_window.h"

void key_callback(void* window, const i32 key, const i32 scancode, const i32 action, const i32 mods)
{
	if (key == AS_KEY_ESCAPE && action == AS_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

i32 main() 
{
	void* display_context = as_display_context_create(1200, 720, "Abstract Shader Engine", &key_callback);
	if (!display_context)
	{
		AS_LOG(LV_WARNING, "invalid display context");
		return AS_ERROR_INVALID_DISPLAY_CONTEXT;
	}
	while (!as_display_context_should_close(display_context))
	{
		as_display_context_poll_event();
	}
	return 0;
}
