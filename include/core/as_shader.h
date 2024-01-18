// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"

#define AS_SHADER_TYPE_VERTEX	0
#define AS_SHADER_TYPE_FRAGMENT	1
#define AS_MAX_SHADER_BIN_SIZE 2048

typedef u8 as_shader_type;

typedef struct as_shader_binary
{
	u32* bin;
	size_t size;
} as_shader_binary;

i32 as_shader_compile(const char* source, const char* entry_point, const as_shader_type shader_type, u32** spirv_code, size_t* spirv_size);
as_shader_binary* as_shader_read_code(const char* filename, const as_shader_type shader_type);
void as_shader_destroy_binary(as_shader_binary* shader_bin, const bool is_ptr);
