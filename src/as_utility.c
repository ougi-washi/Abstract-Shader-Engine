// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_utility.h"
#include "as_memory.h"

void as_i32_to_str(const i32 integer, char* out_str)
{
	sprintf(out_str, "%d", integer);
}

char* as_util_read_file(const char* filename, sz* size)
{
	FILE* file = fopen(filename, "rb");
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

void extract_base_path(const char* filename, char* base_path)
{
	size_t len = strlen(filename);
	size_t pos = len;

	// Find the last occurrence of '/'
	while (pos > 0 && filename[pos - 1] != '/')
	{
		pos--;
	}

	// Copy the base path
	strncpy(base_path, filename, pos);
	base_path[pos] = '\0';
}

void append_base_path(const char* base_path, const char* filename, char* output_path)
{
	snprintf(output_path, AS_MAX_PATH_SIZE, "%s/%s", base_path, filename);
}
void as_util_expand_file_includes(const char* filename, char* output) 
{
	if (!output || !filename) 
	{
		AS_LOG(LV_WARNING, "Cannot expand file includes, nullptr");
		return;
	}

	char file_path[AS_MAX_PATH_SIZE] = "";
	extract_base_path(filename, file_path);

	FILE* file = fopen(filename, "r");

	if (!file) {
		AS_LOG(LV_WARNING, "Error opening file: ");
		AS_LOG(LV_WARNING, filename);
		return;
	}

	char file_contents[AS_MAX_FILE_SIZE] = "";
	size_t read_bytes = fread(file_contents, 1, as_util_get_file_size(file), file);
	if (read_bytes == 0 && !feof(file)) 
	{
		AS_LOG(LV_WARNING, "Error reading file: ");
		AS_LOG(LV_WARNING, filename);
		fclose(file);
		return;
	}
	fclose(file);

	char* include_pos = strstr(file_contents, "#include");
	if (!include_pos) 
	{
		strcpy(output, file_contents); // No includes found, copy the entire file content
		return;
	}

	// Copy the portion before the first #include
	strncpy(output, file_contents, include_pos - file_contents);
	output[include_pos - file_contents] = '\0'; // Null-terminate the output string

	// Find and replace all includes
	while (include_pos) 
	{
		char include_file[AS_MAX_FILE_SIZE] = "";
		char* quote_start = strchr(include_pos, '"');
		char* quote_end = strchr(quote_start + 1, '"');

		if (!quote_start || !quote_end) 
		{
			AS_LOG(LV_WARNING, "Invalid include directive: ");
			AS_LOG(LV_WARNING, include_pos);
		}

		strncpy(include_file, quote_start + 1, quote_end - quote_start - 1);
		include_file[quote_end - quote_start - 1] = '\0';

		char included_file[AS_MAX_FILE_SIZE] = "";
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
	}
}

void as_util_write_file(const char* filename, const void* data, const sz size, const bool is_binary)
{
	FILE* file = fopen(filename, (is_binary) ? "wb" : "w");
	AS_ASSERT(file, "Failed to open file for writing");
	const sz written = fwrite(data, 1, size, file);
	fclose(file);
	AS_ASSERT(written == size, "Failed to write the entire data to file");
}

void as_util_make_dir(const char* directory)
{
	mkdir(directory, 0777);
}

extern void as_util_make_path(char* output, const char* base_path, const char* file_path)
{
	sprintf(output, "%s%s", base_path, file_path);
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


void as_serialize(void* data, const sz size, const char* filename)
{
    FILE* file = fopen(filename, "wb");
	AS_ASSERT(file, "Failed to open file for writing"); 
	fwrite(data, size, 1, file); 
    fclose(file); 
}

void* as_deserialize(const sz size, const char* filename)
{
	void* data = (void*)AS_MALLOC(size); 
    FILE* file = fopen(filename, "rb"); 
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
