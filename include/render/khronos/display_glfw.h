 #pragma once

 #include "types.h"

class GLFWwindow;

namespace as
{
	bool create_window(GLFWwindow*& window, const u16& width, const u16& height);
	bool should_loop(GLFWwindow* window);
	bool process_input(GLFWwindow* window);
	bool update(GLFWwindow* window);
	bool get_window_size(GLFWwindow* window, i32& width, i32& height);
	f32 get_display_ratio(GLFWwindow* window);
	void terminate_glfw();
};