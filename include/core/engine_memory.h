#pragma once

#include "engine_types.h"

size_t global_memory = 0;
size_t allocation_count = 0;

struct allocation
{
	FILE file;
	LINE line;
	size_t size = 0;
};

#define Alloc()