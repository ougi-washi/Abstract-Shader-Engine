// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#pragma warning(disable: 6011)
#pragma warning(disable: 6308)

#include "as_types.h"
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_ALLOCATIONS_COUNT 257400

typedef struct as_allocation
{
	void* ptr;
	size_t size;
	char file[256];
	u16 line;
	char type[128];
} as_allocation;

#define AS_INIT_ALLOCATION(_allocation)			\
_allocation.ptr = NULL;								\
_allocation.size = 0;								\
strcpy(allocations[allocations_count].file, "");	\
_allocation.line = 0;								\
strcpy(allocations[allocations_count].type, _type);

#define AS_DECLARE_ALLOCATION(_allocation)		\
as_allocation _allocation;						\
AS_INIT_ALLOCATION(_allocation);

extern u32 allocations_count;
extern u64 allocated_memory;
extern as_allocation allocations[];

extern void* as_malloc_fn(const size_t _size, const char* _file, const u32 _line, const char* _type);
extern void* as_realloc_fn(void* _ptr, const size_t _size, const char* _file, const u32 _line);
extern void as_free_fn(void* _ptr);
extern char* as_allocation_to_string(as_allocation* _allocation);
extern void as_log_memory();

#define AS_MALLOC(_size) (as_malloc_fn(_size, __FILE__, __LINE__, ""))
#define AS_MALLOC_WITH_TYPE(_size, _type) (as_malloc_fn(_size, __FILE__, __LINE__, _type))
#define AS_REALLOC(_ptr, _size) (as_realloc_fn(_ptr, _size, __FILE__, __LINE__))
#define AS_FREE(_ptr) (as_free_fn(_ptr))
#define AS_LOG_MEMORY() (as_log_memory())

#define AS_MALLOC_SINGLE(_type) (_type*)AS_MALLOC(sizeof(_type))
