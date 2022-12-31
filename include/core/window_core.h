#pragma once

#include "types.h"
#include "window_types.h"

namespace as
{
	bool create_display_handle(const bool& is_raw_display, const u16& width, const u16& height, as::display_handle& display_handle);
	bool create_window(GLFWwindow*& window, const u16& width, const u16& height);
	bool create_raw_display(as::raw_display& raw_display);

	bool should_display_loop(const as::display_handle& display_handle);
	bool process_input(const as::display_handle& display_handle);

	bool draw(as::display_handle& display_handle);
	bool draw_on_raw_display(as::raw_display& raw_display, const u8* pixels);
}