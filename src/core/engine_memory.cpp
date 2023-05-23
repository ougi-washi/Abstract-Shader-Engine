#include "engine_memory.h"
#include <string>

u32 allocations_count = 0;
u64 allocated_memory = 0;
as::allocation allocations[MAX_ALLOCATIONS_COUNT] = {};

void* as_malloc_fn(const size_t& _size, const char* _file, const u32& _line, const char* _type)
{
	void* _ptr = nullptr;
	memcpy(allocations[allocations_count].file, _file, 256);
	memcpy(allocations[allocations_count].type, _type, 128);
	allocations[allocations_count].line = _line;
	allocations[allocations_count].size = _size;
	_ptr = malloc(_size);
	if (_ptr)
	{
		memset(_ptr, 0, _size);
	}
	allocations[allocations_count].ptr = _ptr;
	allocated_memory += _size;
	allocations_count++;
	return _ptr;
}

void* as_realloc_fn(void* _ptr, const size_t& _size, const char* _file, const u32& _line)
{
	if (allocations && _ptr)
	{
		for (u32 i = 0; i < allocations_count; i++)
		{
			if (allocations[i].ptr == _ptr)
			{
				_ptr = realloc(_ptr, _size);
				allocated_memory -= allocations[i].size;
				allocated_memory += _size;

				memcpy(allocations[i].file, _file, 265);
				allocations[i].line = _line;
				allocations[i].ptr = _ptr;
				allocations[i].size = _size;
				return _ptr;
			}
		}
	}
	return as_malloc_fn(_size, _file, _line);
}

void as_free_fn(void* _ptr)
{
	i64 removed_index = -1;
	for (u32 i = 0; i < allocations_count; i++)
	{
		if (removed_index == -1 && allocations[i].ptr == _ptr)
		{
			free(_ptr);
			_ptr = nullptr;
			allocated_memory -= allocations[i].size;
			//free(allocations[i]);
			//allocations[i] = nullptr;
			removed_index = i;
		}
		else if (removed_index >= 0 && i > 0)
		{
			allocations[i - 1] = allocations[i];
		}
	}
	if (allocations_count > 0 && removed_index >= 0)
	{
		allocations_count--;
	}
	/*if (allocations_count == 0)
	{
		free(allocations);
		allocations = nullptr;
	}
	else
	{
		allocations = (as::allocation**)realloc(allocations, sizeof(as::allocation*) * allocations_count);
	}*/
}

char* as_allocation_to_string(as::allocation* _allocation)
{
	if (_allocation)
	{
		char* str = (char*)malloc(256);
		if (str)
		{
			sprintf(str, "ptr=%p, size=%zu, file=%s, line=%d", 
				_allocation->ptr, _allocation->size, _allocation->file, _allocation->line);
			return str;
		}
	}
	return nullptr;
}

void as_log_memory()
{
	AS_LOG(LV_LOG, std::string("Total allocated memory = " + std::to_string(allocated_memory)));
	for (u32 i = 0; i < allocations_count; i++)
	{
		char* str = as_allocation_to_string(&allocations[i]);
		AS_LOG(LV_LOG, str);
		free(str);
	}
}
