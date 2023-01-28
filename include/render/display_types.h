#pragma once 
#include "engine_utility.h"

#ifdef __linux__ 
#include <linux/fb.h>
#elif _WIN32
#else

#endif

class GLFWwindow;
namespace as
{
	struct raw_display
	{
		i32 framebuffer_device = 0;
		i64 screen_size = 0;
		char* framebuffer_ptr = nullptr;
		
		// internal
#ifdef __linux__ 
		struct fb_var_screeninfo vinfo;
		struct fb_fix_screeninfo finfo;
#elif _WIN32
#endif
		operator bool() const { return framebuffer_ptr != nullptr; };
	};

	struct display_handle
	{
		// general
		u8 is_raw_display :1 ;
		u16 width; 
		u16 height;

		// frames
		f32 delta_time = 0.0f;
		f32 last_frame_time = 0.0f;
		timer::handle timer_handle;

		// internal
		GLFWwindow* GLFW = nullptr;
		as::raw_display raw_display;
	};
}