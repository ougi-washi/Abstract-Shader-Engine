#include "core/as_ui.h"


void as_ui_text_init(as_ui_text* ui_text, as_texture* font_texture, const u32 font_size, const f32 spacing, const char* text)
{
	AS_ASSERT(ui_text, "Cannot init, invalid ui text");
	AS_ASSERT(text, "Cannot init, invalid text ptr");

	ui_text->type = AS_SO_TEXT;
	as_ui_text_set_font_texture(ui_text, font_texture);
	as_ui_text_set_font(ui_text, font_size, spacing);
	as_ui_text_set_text(ui_text, text);

	// TODO: encoding 4 ASCIIs in 32 bit value
	//for (sz i = 0; i < strlen(text); ++i) 
	//{
	//	u8 encoded_char = (u8)text[i];
	//	sz array_index = i / 4; // Each u32 can hold 4 characters (4 bytes)
	//	sz char_index_in_u32 = i % 4; // Index of the character within the u32
	//	ui_text->custom_data[array_index + AS_UI_TEXT_DATA_OFFSET] |= (u32)encoded_char << (8 * char_index_in_u32);
	//}
}

void as_ui_text_set_font(as_ui_text* ui_text, const u32 font_size, const f32 spacing)
{
	AS_ASSERT(ui_text, "Cannot set font, invalid ui text");
	ui_text->uniform_buffer.custom_info.AS_UI_TEXT_FONT_SIZE_INDEX = font_size;
	ui_text->uniform_buffer.custom_info.AS_UI_TEXT_SPACING_INDEX = spacing / 100.f;
}

void as_ui_text_set_text(as_ui_text* ui_text, const char* text)
{
	AS_ASSERT(ui_text, "Cannot set text, invalid ui text");
	AS_ASSERT(text, "Cannot set text, invalid text");

	ui_text->uniform_buffer.custom_info.AS_UI_TEXT_LENGTH_INDEX = strlen(text);
	memset(&ui_text->uniform_buffer.custom_data, 0, sizeof(ui_text->uniform_buffer.custom_data));
	f32* custom_data = (f32*)&ui_text->uniform_buffer.custom_data;
	for (sz i = 0; i < strlen(text); ++i)
	{
		AS_ASSERT(custom_data[i] == 0, "Cannot set text, error in encoding the data in the uniform buffer");
		custom_data[i] = (f32)text[i];
	}
}

void as_ui_text_set_font_texture(as_ui_text* ui_text, as_texture* font_texture)
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
