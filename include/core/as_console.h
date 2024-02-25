// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#include "as_threads.h"
#include "as_array.h"

#define AS_COMMAND_MAX_NAME 128
#define AS_COMMAND_MAX_DESC 1024

typedef void (*as_command_func)(const char*, const char*, const char*);
typedef struct as_command_mapping
{
	char name[AS_COMMAND_MAX_NAME];
	char desc[AS_COMMAND_MAX_DESC];
	as_command_func func;
	i32 arg_count;
} as_command_mapping;
AS_ARRAY_DECLARE(as_command_mapping_128, 128, as_command_mapping);

typedef struct as_console
{
	b8 is_running;
	as_thread thread;
	as_command_mapping_128 mappings;
}as_console;

extern as_console* as_console_create();
extern void as_console_destroy(as_console* console);
extern as_command_mapping_128* as_console_get_mappings(as_console* console);
