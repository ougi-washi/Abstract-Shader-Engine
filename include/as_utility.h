// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#include "as_types.h"
#include "as_array.h"

#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define AS_MAX_PATH_SIZE 1024
#define AS_MAX_FILE_NAME_SIZE 128
#define AS_MAX_FILE_SIZE 512 * 1024
#define AS_FILE_POOL_SIZE 16

// file pool system
typedef struct as_file_handle
{
	char content[AS_MAX_FILE_SIZE];
} as_file_handle;
AS_STATIC_ARRAY_DECLARE(as_file_handles, AS_FILE_POOL_SIZE, as_file_handle);

typedef struct as_file_pool
{
	AS_DECLARE_TYPE;
	as_file_handles handles;
} as_file_pool;

//static as_file_pool* file_pool = NULL;
extern void as_file_pool_create();
extern void as_file_pool_destroy();
extern as_file_handle* as_fp_make_handle(as_file_pool* file_pool);
extern void as_fp_remove_handle(as_file_pool* file_pool, as_file_handle* handle);

// conversions
extern void as_i32_to_str(const i32 integer, char* out_str);

// files
extern char* as_util_read_file(const char* path, sz* size);
extern sz as_util_get_file_size(FILE* file);
extern void as_util_expand_file_includes(as_file_pool* file_pool, const char* path, char* output);
extern void as_util_write_file(const char* path, const void* data, const sz size, const bool is_binary);
extern void as_util_make_path(char* output, const char* base_path, const char* file_path);
extern void as_util_extract_base_path(const char* path, char* base_path);
extern void as_util_extract_file_name(const char* path, char* file_name);
extern void as_util_combine_path_and_file(const char* base_path, const char* file_name, char* output_path);
extern void as_util_ensure_directory_exists(const char* path);

// time
extern clock_t get_current_time();
extern f64 calculate_delta_time(clock_t start, clock_t end);
extern void sleep_seconds(const f64 seconds);

void as_serialize_to_file(void* data, const sz size, const char* path);
#define AS_SERIALIZE_TO_FILE(_type, _data, _path) as_serialize_to_file(_data, sizeof(_type), _path)

void* as_deserialize_from_file(const sz size, const char* path);
#define AS_DESERIALIZE_FROM_FILE(_type, _path) (_type*)as_deserialize_from_file(sizeof(_type), _path)
