// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_utility.h"
#include "as_memory.h"
#include <stdlib.h>

void as_i32_to_str(const i32 integer, char* out_str)
{
	sprintf(out_str, "%d", integer);
}

char* as_util_read_file(const char* path, sz* size)
{
	FILE* file = fopen(path, "rb");
	if (!file)
	{
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* content = (char*)AS_MALLOC(*size + 1);
	if (content)
	{
		fread(content, 1, *size, file);
		content[*size] = '\0';
	}

	fclose(file);
	return content;
}


sz as_util_get_file_size(FILE* file)
{
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return size;
}

void extract_base_path(const char* path, char* base_path)
{
	size_t len = strlen(path);
	size_t pos = len;

	// Find the last occurrence of '/'
	while (pos > 0 && path[pos - 1] != '/')
	{
		pos--;
	}

	// Copy the base path
	strncpy(base_path, path, pos);
	base_path[pos] = '\0';
}

void append_base_path(const char* base_path, const char* path, char* output_path)
{
	snprintf(output_path, AS_MAX_PATH_SIZE, "%s/%s", base_path, path);
}
void as_util_expand_file_includes(const char* path, char* output) 
{
	if (!output || !path) 
	{
		AS_LOG(LV_WARNING, "Cannot expand file includes, nullptr");
		return;
	}

	char file_path[AS_MAX_PATH_SIZE] = "";
	extract_base_path(path, file_path);

	FILE* file = fopen(path, "r");

	if (!file) 
	{
		AS_FLOG(LV_WARNING, "Error opening file: %s", path);
		return;
	}

	char* file_contents = (char*)AS_MALLOC(sizeof(char) * AS_MAX_FILE_SIZE);
	size_t read_bytes = fread(file_contents, 1, as_util_get_file_size(file), file);
	if (read_bytes == 0 && !feof(file)) 
	{
		AS_FLOG(LV_WARNING, "Error reading file: %s", path);
		fclose(file);
		AS_FREE(file_contents);
		return;
	}
	fclose(file);

	char* include_pos = strstr(file_contents, "#include");
	if (!include_pos) 
	{
		strcpy(output, file_contents); // No includes found, copy the entire file content
		AS_FREE(file_contents);
		return;
	}

	// Copy the portion before the first #include
	strncpy(output, file_contents, include_pos - file_contents);
	output[include_pos - file_contents] = '\0'; // Null-terminate the output string

	// Find and replace all includes
	while (include_pos) 
	{
		char* include_file = (char*)AS_MALLOC(sizeof(char) * AS_MAX_FILE_SIZE);
		char* quote_start = strchr(include_pos, '"');
		char* quote_end = strchr(quote_start + 1, '"');

		if (!quote_start || !quote_end) 
		{
			AS_FLOG(LV_WARNING, "Invalid include directive: %s", include_pos);
		}

		strncpy(include_file, quote_start + 1, quote_end - quote_start - 1);
		include_file[quote_end - quote_start - 1] = '\0';

		char* included_file = (char*)AS_MALLOC(sizeof(char) * AS_MAX_FILE_SIZE);
		char included_file_path[AS_MAX_PATH_SIZE] = "";
		append_base_path(file_path, include_file, included_file_path);
		as_util_expand_file_includes(included_file_path, included_file);

		// Append the included file content to the output
		strcat(output, included_file);

		include_pos = strstr(include_pos + 1, "#include");
		if (!include_pos) 
		{
			// Append the remaining file content after the last #include
			strcat(output, quote_end + 1);
		}
		else 
		{
			// Append the portion between the current and next #include
			strncat(output, quote_end + 1, include_pos - quote_end - 1);
		}
		AS_FREE(include_file);
		AS_FREE(included_file);
	}
	AS_FREE(file_contents);
}

void as_util_write_file(const char* path, const void* data, const sz size, const bool is_binary)
{
	FILE* file = fopen(path, (is_binary) ? "wb" : "w");
	AS_ASSERT(file, "Failed to open file for writing");
	const sz written = fwrite(data, 1, size, file);
	fclose(file);
	AS_ASSERT(written == size, "Failed to write the entire data to file");
}

void as_util_make_path(char* output, const char* base_path, const char* file_path)
{
	sprintf(output, "%s%s", base_path, file_path);
}

void as_util_extract_base_path(const char* path, char* base_path)
{
    size_t len = strlen(path);
    size_t pos = len;

    while (pos > 0 && path[pos - 1] != '/')
    {
        pos--;
    }
    strncpy(base_path, path, pos);
    base_path[pos] = '\0';
}

void as_util_extract_file_name(const char* path, char* file_name)
{
    size_t len = strlen(path);
    size_t pos = len;

    while (pos > 0 && path[pos - 1] != '/')
    {
        pos--;
    }
    strncpy(file_name, path + pos, len - pos);
    file_name[len - pos] = '\0';
}

void as_util_combine_path_and_file(const char* base_path, const char* file_name, char* output_path)
{
    snprintf(output_path, AS_MAX_PATH_SIZE, "%s/%s", base_path, file_name);
}

void create_directory(const char* directory)
{
#ifdef _WIN32
    CreateDirectory(directory, NULL);
#else
    mkdir(directory, 0777);
#endif
}

void as_util_ensure_directory_exists(const char* path)
{
    size_t path_length = strlen(path);
    char current_path[AS_MAX_PATH_SIZE];
    current_path[0] = '\0';

    for (size_t i = 0; i < path_length; ++i)
    {
        if (path[i] == '/')
        {
            strcat(current_path, "/");
            create_directory(current_path);
        }
        else
        {
            strncat(current_path, &path[i], 1);
        }
    }
}

as_file_handle* as_fp_make_handle(as_file_pool* pool)
{
	AS_WARNING_RETURN_VAL_IF_FALSE(pool, NULL, "invalid pool, cannot make handle");
	return AS_ARRAY_INCREMENT(*pool);
}

extern char* as_fp_make_handle_c(as_file_pool* pool)
{
	return as_fp_make_handle(pool)->content;
}

void as_fp_remove_handle(as_file_pool* pool, as_file_handle* handle)
{
	AS_WARNING_RETURN_IF_FALSE(pool, "invalid pool, cannot remove handle");
	AS_WARNING_RETURN_IF_FALSE(handle, "invalid handle, cannot remove handle");
	
	i32 found_index = -1;
	AS_ARRAY_FIND_PTR(*pool, handle, found_index);
	AS_ARRAY_REMOVE_AT(*pool, found_index);
}

clock_t get_current_time()
{
	return clock();
}

f64 calculate_delta_time(clock_t start, clock_t end)
{
	return ((f64)(end - start)) / (f64)CLOCKS_PER_SEC;
}

void sleep_seconds(const f64 seconds)
{
#ifdef _WIN32
	Sleep((DWORD)(seconds * 1000));
#else
	struct timespec sleepTime;
	sleepTime.tv_sec = (time_t)seconds;
	sleepTime.tv_nsec = (long)((seconds - sleepTime.tv_sec) * 1e9);
	nanosleep(&sleepTime, NULL);
#endif
}

void as_serialize_to_file(void* data, const sz size, const char* path)
{
	as_util_ensure_directory_exists(path);
    FILE* file = fopen(path, "wb");
	AS_ASSERT(file, "Failed to open file for writing"); 
	fwrite(data, size, 1, file); 
    fclose(file); 
}

void* as_deserialize_from_file(const sz size, const char* path)
{
	void* data = (void*)AS_MALLOC(size); 
    FILE* file = fopen(path, "rb"); 
    if (file) 
	{ 
        fread(data, size, 1, file);
        fclose(file); 
        data; 
    } 
	else 
	{ 
        AS_FREE(data); 
        return NULL; 
    } 
	return data;
}
