// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"

#define AS_SHADER_TYPE_VERTEX	0
#define AS_SHADER_TYPE_FRAGMENT	1

i32 as_compile_shader(const char* source, const char* entry_point, const u8 shader_kind, char** spirv_code, size_t* spirv_size);
