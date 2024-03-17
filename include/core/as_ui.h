// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#include "core/as_render.h"

#define AS_MAX_UI_TEXT_SIZE 4096

typedef struct as_ui_text 
{
	as_screen_object* screen_object;
	char text[AS_MAX_UI_TEXT_SIZE];
	float size;
} as_ui_text;

