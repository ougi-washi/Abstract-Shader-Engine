#pragma once
#pragma warning(disable: 6011)
#pragma warning(disable: 6308)

#include "types.h"

#define MAX_ALLOCATIONS_COUNT 257400

namespace as
{
	struct allocation
	{
		void* ptr = nullptr;
		size_t size = 0;
		char file[256] = "";
		u16 line = 0;
		char type[128] = "";
	};
};

extern u32 allocations_count;
extern u64 allocated_memory;
extern as::allocation allocations[];

extern void* as_malloc_fn(const size_t& _size, const char* _file, const u32& _line, const char* _type = "");
extern void* as_realloc_fn(void* _ptr, const size_t& _size, const char* _file, const u32& _line);
extern void as_free_fn(void* _ptr);
extern char* as_allocation_to_string(as::allocation* _allocation);
extern void as_log_memory();

#define AS_MALLOC(_size) (as_malloc_fn(_size, __FILE__, __LINE__))
#define AS_MALLOC_WITH_TYPE(_size, _type) (as_malloc_fn(_size, __FILE__, __LINE__, _type))
#define AS_REALLOC(_ptr, _size) (as_realloc_fn(_ptr, _size, __FILE__, __LINE__))
#define AS_FREE(_ptr) (as_free_fn(_ptr))
#define AS_LOG_MEMORY() (as_log_memory())