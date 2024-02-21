// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#include "as_array.h"

#define AS_COMMAND_MAX_NAME 128

typedef void (*as_command_func)(const char*, const char*, const char*);
typedef struct as_command_mapping
{
	char name[AS_COMMAND_MAX_NAME];
	as_command_func func;
	i32 arg_count;
} as_command_mapping;
AS_ARRAY_DECLARE(as_command_mapping_128, 128, as_command_mapping);

extern void as_console_init();
extern void as_console_clear();
extern as_command_mapping_128* as_console_get_mappings();
