#include "as_types.h"

typedef struct 
{
	void* data;
	sz element_size;
	sz size;
	sz capacity;
} as_array;

as_array* as_create_array(sz capacity, sz element_size) 
{
	as_array* array = (as_array*)AS_MALLOC(sizeof(as_array));
	AS_ASSERT(array, "Error creating array");

	array->data = AS_MALLOC(capacity * element_size);
	AS_ASSERT(array->data, "Error creating array data");

	array->element_size = element_size;
	array->size = 0;
	array->capacity = capacity;

	return array;
}

void as_resize_array(as_array* array, sz new_capacity) 
{
	array->data = AS_REALLOC(array->data, new_capacity * array->element_size);
	AS_ASSERT(array->data, "Error creating array data");
	array->capacity = new_capacity;
}

void as_push_back_array(as_array* array, const void* element) 
{
	if (array->size == array->capacity) 
	{
		as_resize_array(array, array->capacity * 2);
	}

	char* dest = ((char*)array->data) + array->size * array->element_size;
	memcpy(dest, element, array->element_size);
	array->size++;
}

void as_insert_at_array(as_array* array, sz index, const void* element) 
{
	AS_ASSERT(index >= 0 && index <= array->size, "Array out of bounds");

	if (array->size == array->capacity) {
		as_resize_array(array, array->capacity * 2);
	}

	char* dest = ((char*)array->data) + index * array->element_size;
	memmove(dest + array->element_size, dest, (array->size - index) * array->element_size);
	memcpy(dest, element, array->element_size);
	array->size++;
}

void as_remove_at_array(as_array* array, sz index) 
{
	AS_ASSERT(index >= 0 && index < array->size, "Array out of bounds");

	char* dest = ((char*)array->data) + index * array->element_size;
	char* src = dest + array->element_size;
	memmove(dest, src, (array->size - index - 1) * array->element_size);

	array->size--;

	if (array->size > 0 && array->size <= array->capacity / 4) 
	{
		as_resize_array(array, array->capacity / 2);
	}
}

void* as_get_array_elem(const as_array* array, sz index) 
{
	AS_ASSERT(index >= 0 && index < array->size, "Array out of bounds");
	return ((char*)array->data) + index * array->element_size;
}

void AS_free_array(as_array* array) 
{
	AS_FREE(array->data);
	AS_FREE(array);
}
