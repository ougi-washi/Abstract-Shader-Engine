// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"
#include "as_array.h"

#define AS_MAX_INPUT_BUFFER_SIZE 64

typedef struct as_input_key
{
    i32 key;
    i32 action;
} as_input_key;
AS_ARRAY_DECLARE(as_input_keys_128, 128, as_input_key);

typedef struct as_input_buffer
{
    as_input_keys_128 keys;
	void (*on_add)(const as_input_key*);
    AS_DECLARE_TYPE;
} as_input_buffer;

extern as_input_buffer* as_input_create();
extern void as_input_destory(as_input_buffer* buffer);
extern void as_input_add(as_input_buffer* buffer, const i32 key, const i32 action);
extern bool as_input_is_pressed(as_input_buffer* buffer, const i32 key);
// preferably run on separate thread and cache without locking
