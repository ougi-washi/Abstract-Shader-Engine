// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"
#include "as_array.h"

#define AS_MAX_INPUT_BUFFER_SIZE

typedef struct as_input_key
{
    i32 key;
    i32 action;
} as_input_key;
AS_DECLARE_ARRAY(as_input_keys_128, 128, as_input_key);

typedef struct as_input_buffer
{
    as_input_keys_128 keys;
} as_input_buffer;

void as_input_add(as_input_buffer* buffer, const i32 key, const i32 action);
