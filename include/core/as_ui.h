// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#include "core/as_render.h"

typedef as_screen_object as_ui_text;

void as_ui_text_init(as_ui_text* ui_text, as_texture* font_texture, const u32 font_size, const f32 spacing, const char* text);
void as_ui_text_set_font(as_ui_text* ui_text, const u32 font_size, const f32 spacing);
void as_ui_text_set_text(as_ui_text* ui_text, const char* text);
void as_ui_text_set_font_texture(as_ui_text* ui_text, as_texture* font_texture);
const char* as_ui_text_get_text(as_ui_text* ui_text);
