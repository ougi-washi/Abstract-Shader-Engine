// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_window.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void* as_display_context_create(const i32 x, const i32 y, const char* title, void key_callback(void*, i32, i32, i32, i32))
{
	if (!glfwInit())
	{
		return NULL;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	GLFWwindow* display_context = glfwCreateWindow(x, y, title, NULL, NULL);
	if (display_context)
	{
		glfwSetKeyCallback(display_context, key_callback);
	}
	if (!display_context) {
		AS_LOG(LV_ERROR, "GLFW window creation failed");
		glfwTerminate();
	}
	return display_context;
}

void as_display_context_destroy(void* display_context)
{
	glfwDestroyWindow(display_context);
}

bool as_display_context_should_close(void* display_context)
{
	return glfwWindowShouldClose(display_context);
}

void as_display_context_poll_event()
{
	glfwPollEvents();
}

void as_display_context_terminate()
{
	glfwTerminate();
}
