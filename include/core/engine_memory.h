#pragma once

#include "engine_types.h"

size_t global_memory = 0;

struct allocation
{
	FILE file;
};

#define Alloc()