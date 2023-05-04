#pragma once
#pragma warning(disable: 6011)
#pragma warning(disable: 6308)

#include "engine_types.h"

namespace as
{
	struct allocation
	{
		void* ptr = nullptr;
		size_t size = 0;
		char file[200] = "";
		u16 line = 0;
	};
};

u32 allocations_count = 0;
u64 allocated_memory = 0;
as::allocation** allocations = nullptr;

template<class T>
T* as_malloc(const u32& count)
{
	const u64 allocation_size = sizeof(T) * count;
	T* _ptr = nullptr;
	if (allocations)
	{
		allocations = (as::allocation**)realloc(allocations, sizeof(as::allocation*) * allocations_count);
	}
	else
	{
		allocations = (as::allocation**)malloc(sizeof(as::allocation*));
	}
	as::allocation* temp_allocation = (as::allocation*)malloc(sizeof(as::allocation));
	memcpy(temp_allocation->file, __FILE__, sizeof(__FILE__));
	temp_allocation->line = __LINE__;
	temp_allocation->size = allocation_size;
	_ptr = (T*)malloc(allocation_size);
	*_ptr = T();
	temp_allocation->ptr = _ptr;
	allocations[allocations_count] = temp_allocation;
	allocated_memory += allocation_size;
	allocations_count++;
	return _ptr;
};

#define AS_FREE(_ptr)\
{\
	i64 removed_index = -1;\
	for (u32 i = 0; i < allocations_count; i++)\
	{\
		if (removed_index == -1 && allocations[i]->ptr == _ptr)\
		{\
			free(_ptr);\
			_ptr = nullptr;\
			allocated_memory -= allocations[i]->size;\
			removed_index = i;\
		}\
		else if (removed_index >= 0 && i > 0)\
		{\
			allocations[i - 1] = allocations[i];\
		}\
	}\
	allocations_count--;\
	if (allocations_count == 0)\
	{\
		free(allocations);\
		allocations = nullptr;\
	}\
	else\
	{\
		allocations = (as::allocation**)realloc(allocations, sizeof(as::allocation*) * allocations_count);\
	}\
};