// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#include "as_types.h"

#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define AS_MAX_PATH_SIZE 256
#define AS_MAX_FILE_SIZE 4096



inline void as_i32_to_str(const i32 integer, char* out_str)
{
	sprintf(out_str, "%d", integer);
};

extern char* as_util_read_file(const char* filename, sz* size);
extern sz as_util_get_file_size(FILE* file);
extern void as_util_expand_file_includes(const char* filename, char* output);
extern void as_util_write_file(const char* filename, const void* data, const sz size, const bool is_binary);
extern void as_util_make_dir(const char* directory);
// time
extern clock_t get_current_time();
extern f32 calculate_delta_time(clock_t start, clock_t end);
extern void sleep_seconds(double seconds);