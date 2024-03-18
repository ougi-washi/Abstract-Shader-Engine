#include "core/as_ui.h"

#define AS_UI_TEXT_MAX_SIZE 512

void as_ui_text_set_text(as_ui_text* ui_text, const f32 font_size, const char* text)
{
	AS_ASSERT(ui_text, "Cannot set text, invalid ui text");
	AS_ASSERT(text, "Cannot set text, invalid text ptr");

	ui_text->type = AS_SO_TEXT;
	ui_text->custom_info[0] = (u32)font_size;
	ui_text->custom_info[1] = (u32)strlen(text);
	memcpy(ui_text->custom_data, text, strlen(text));
}

void as_ui_text_set_font(as_ui_text* ui_text, as_texture* font_texture)
{
	AS_ASSERT(ui_text, "Cannot set text font, invalid ui text");
	AS_ASSERT(font_texture, "Cannot set text font, invalid font texture");

	const sz index = as_shader_add_uniform_texture(&ui_text->uniforms, font_texture);
}

const char* as_ui_text_get_text(as_ui_text* ui_text)
{
	AS_ASSERT(ui_text, "Cannot get text, invalid ui text");
	return NULL;
}
