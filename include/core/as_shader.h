// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"
#include "as_utility.h"
#include "as_array.h"

#define AS_SHADER_TYPE_VERTEX		0
#define AS_SHADER_TYPE_FRAGMENT		1
#define AS_SHADER_BINARY_POOL_SIZE	16

typedef u8 as_shader_type;

typedef struct as_shader_binary
{
	char source[AS_MAX_FILE_SIZE];
	sz source_size;
	u32 binaries[AS_MAX_FILE_SIZE];
	sz binaries_size;
} as_shader_binary;

AS_STATIC_ARRAY_DECLARE(as_shader_binary_pool, AS_SHADER_BINARY_POOL_SIZE, as_shader_binary);

//extern void as_shader_binary_pool_create();
//extern void as_shader_binary_pool_destroy();

extern i32 as_shader_compile(as_shader_binary* binary, const char* source, const char* entry_point, const as_shader_type shader_type);
extern void as_shader_get_cached_path(char* out_path, const char* original_path);
extern as_shader_binary* as_shader_read_code(as_shader_binary_pool* shader_binary_pool, as_file_pool* file_pool, const char* path, const as_shader_type shader_type);
extern void as_shader_destroy_binary(as_shader_binary_pool* shader_binary_pool, as_shader_binary* shader_bin, const bool is_ptr);

extern bool as_shader_has_changed(as_shader_binary_pool* shader_binary_pool, as_file_pool* file_pool, const char* path);
extern void as_shader_binary_serialize(const as_shader_binary* data, const char* path);
extern as_shader_binary* as_shader_binary_deserialize(as_shader_binary_pool* shader_binary_pool, const char* path);
