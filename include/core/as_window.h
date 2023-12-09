// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#include "as_types.h"
#include "defines/as_keys.h"

void* as_display_context_create(const i32 x, const i32 y, const char* title, void key_callback(void*, i32, i32, i32,i32));
void as_display_context_destroy(void* display_context);
bool as_display_context_should_close(void* display_context);
void as_display_context_poll_event();
void as_display_context_terminate();