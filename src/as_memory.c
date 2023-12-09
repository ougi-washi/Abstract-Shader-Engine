// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_memory.h"
#include "stdlib.h"

u32 allocations_count = 0;
u64 allocated_memory = 0;
as_allocation allocations[MAX_ALLOCATIONS_COUNT] = { 0 };

void* as_malloc_fn(const size_t _size, const char* _file, const u32 _line, const char* _type)
{
	strcpy(allocations[allocations_count].file, _file);
	strcpy(allocations[allocations_count].type, _type);
	allocations[allocations_count].line = _line;
	allocations[allocations_count].size = _size;
	allocations[allocations_count].ptr = malloc(_size);
	if (allocations[allocations_count].ptr)
	{
		memset(allocations[allocations_count].ptr, 0, _size);
	}
	allocated_memory += _size;
	allocations_count++;
	return allocations[allocations_count - 1].ptr;
}

void* as_realloc_fn(void* _ptr, const size_t _size, const char* _file, const u32 _line)
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
	return as_malloc_fn(_size, _file, _line, "");
}

void as_free_fn(void* _ptr)
{
	i64 removed_index = -1;
	for (u32 i = 0; i < allocations_count; i++)
	{
		if (removed_index == -1 && allocations[i].ptr == _ptr)
		{
			free(_ptr);
			_ptr = NULL;
			allocated_memory -= allocations[i].size;
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
}

char* as_allocation_to_string(as_allocation* _allocation)
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
	return NULL;
}

//void as_log_memory()
//{
//	as_LOG(LV_LOG, std::string("Total allocated memory = " + std::to_string(allocated_memory)));
//	for (u32 i = 0; i < allocations_count; i++)
//	{
//		char* str = as_allocation_to_string(&allocations[i]);
//		as_LOG(LV_LOG, str);
//		free(str);
//	}
//}
