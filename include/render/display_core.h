#pragma once

#include "types.h"
#include "display_types.h"

namespace as
{
	bool create_display_handle(const bool& is_raw_display, const u16& width, const u16& height, as::display_handle& display_handle);

	bool should_display_loop(const as::display_handle& display_handle);
	f32 get_delta_time(as::display_handle& display_handle);
	f32 get_display_ratio(const as::display_handle& display_handle);
	bool process_input(const as::display_handle& display_handle);

	bool draw(as::display_handle& display_handle);
	bool draw_on_raw_display(as::raw_display& raw_display, const u8* pixels);

	bool update(as::display_handle& display_handle);
	void terminate_display();
}