#include "engine_utility.h"
#include "engine_memory.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


#define MAX_INCLUDE_DEPTH 10

FILE* as::util::try_open_file(const char* file_path, const char* mode)
{
	FILE* fptr = fopen(file_path, mode);
	if (!fptr)
	{
		std::string file_path_string(file_path);
		u16 offset_to_remove = -1;
		for (u16 i = file_path_string.size() - 1 ; i > 0 ; i--)
		{
			if (file_path_string[i] == '/')
			{
				offset_to_remove = i;
				break;
			}
		}
		file_path_string.erase(offset_to_remove + 1, 200);
		std::vector<std::string> folders_to_create;
		bool skip_first = false; // this is either the local disk or the current directory
		for (u16 i = 0 ; i < file_path_string.length() ; i++)
		{
			if ((file_path_string[i] == '/' || file_path_string[i] == '\\'))
			{
				if (skip_first)
				{
					std::string folder_to_create = file_path_string.substr(0, i);
					folders_to_create.push_back(folder_to_create);
				}
				else
				{
					skip_first = true;
				}
			}
		}
		for (std::string& folder_to_create : folders_to_create)
		{
			if (does_dir_exist(folder_to_create.c_str()) == 0)
			{
//#if defined(_WIN32)
//				bool mkdir_result = CreateDirectory(folder_to_create.c_str(), NULL);
//#else 
//				i32 mkdir_result = mkdir(folder_to_create.c_str(), 0700);
//#endif
//				if (!mkdir_result)
//				{
//					AS_LOG(LV_WARNING, "Folder [" + folder_to_create + "] could not be created, a crash may occur");
//				}
			}
		}
		fptr = fopen(file_path, mode);
	}
	if (!fptr)
	{
		AS_LOG(LV_WARNING, "File [" + std::string(file_path) + "] could not be open, a crash may occur");
	}
	return fptr;
}

std::string as::util::get_root_path()
{
	std::string path = std::filesystem::current_path().root_path().string();
	replace_char(path, '\\', '/');
	return path;
}

std::string as::util::get_current_path()
{
	std::string path = std::filesystem::current_path().string();
	replace_char(path, '\\', '/');
	return path;
}

std::string as::util::get_relative_path(const char* to_path)
{
	std::string path = std::filesystem::relative(to_path).string();
	replace_char(path, '\\', '/');
	return path;
}


std::string as::util::get_project_path()
{
	std::string current_path = get_current_path();
	u8 folders_to_remove = 2;
	u16 current_index = current_path.size();

	while (folders_to_remove > 0)
	{
		std::size_t bar_index = current_path.find_last_of("/");
		current_path = current_path.substr(0, bar_index);
		folders_to_remove--;
	}

	return current_path;
}

std::string as::util::get_directory(const std::string path)
{
	return path.substr(0, path.find_last_of('/'));
}

i32 as::util::does_dir_exist(const char* path)
{
	/******************************************************************************
	 * Checks to see if a directory exists. Note: This method only checks the
	 * existence of the full path AND if path leaf is a dir.
	 *
	 * @return  >0 if dir exists AND is a dir,
	 *           0 if dir does not exist OR exists but not a dir,
	 *          <0 if an error occurred (errno is also set)
	 *****************************************************************************/
	struct stat info;

	int statRC = stat(path, &info);
	if (statRC != 0)
	{
		if (errno == ENOENT) // something along the path does not exist
		{ 
			return 0; 
		} 
		if (errno == ENOTDIR)  // something in path prefix is not a dir
		{
			return 0;
		}
		return -1;
	}

	return (info.st_mode & S_IFDIR) ? 1 : 0;
}

bool as::util::does_file_exist(const std::string& path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

void as::util::replace_char(std::string& string_to_update, const char& source, const char& target)
{
	for (char& current_path_char : string_to_update)
	{
		if (current_path_char == source)
		{
			current_path_char = target;
		}
	}
}

std::string as::util::read_file(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	std::string result_buffer;

	if (file.is_open()) 
	{
		size_t file_size = (size_t)file.tellg();
		result_buffer.resize(file_size);
		file.seekg(0);
		file.read(result_buffer.data(), file_size);
		file.close();
	}
	else
	{
		AS_LOG(LV_WARNING, "File [" + filename + "] failed to open");
	}
	return result_buffer;
}

json as::util::read_json_file(const std::string& path)
{
	std::string json_string = read_file(path);
	if (does_file_exist(path))
	{
		return json::parse(json_string);
	}
	AS_LOG(LV_WARNING, "File [" + path + "] does not exist");
	return json();
}

std::string as::util::read_file(const char* filename)
{
	FILE* f = fopen(filename, "rb");
	std::string result_string;
	if (!f)
	{
		AS_LOG(LV_WARNING, "File [" + std::string(filename) + "] not found, a crash may occur");
		return result_string;
	}
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* string = (char*)malloc(fsize + 1);
	if (string)
	{
		fread(string, fsize, 1, f);
		fclose(f);
		string[fsize] = 0;
		result_string = std::string(string);
	}
	free(string);
	return result_string;
}

void as::util::write_file_str(const char* filename, const char* data)
{
	FILE* fptr = try_open_file(filename, "w");
	if (fptr)
	{
		fprintf(fptr, "%s", data);
		fclose(fptr);
	}
}

void as::util::expand_file_includes(const char* filename, const char* current_path, char* output) 
{
	if (!output || !current_path || !filename)
	{
		AS_LOG(LV_WARNING, "Cannot expand file includes, nullptr");
		return;
	}
	char* file_path = (char*)AS_MALLOC(sizeof(char) * MAX_PATH_SIZE);
    snprintf(file_path, MAX_PATH_SIZE, "%s/%s", current_path, filename);

    FILE* file = fopen(file_path, "r");
	AS_FREE(file_path);

    if (!file) 
    {
		AS_LOG(LV_WARNING, "Error opening file: " + std::string(file_path));
		return;
	}

    char* file_contents = (char*)AS_MALLOC(sizeof(char) * MAX_FILE_SIZE);
    size_t read_bytes = fread(file_contents, 1, get_file_size(file), file);
    if (read_bytes == 0 && !feof(file)) 
    {
        AS_LOG(LV_WARNING, "Error reading file: " + std::string(file_path));
        fclose(file);
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
    // Find and replace all includes
    while (include_pos) 
	{
		char* include_file = (char*)AS_MALLOC(sizeof(char) * MAX_FILE_SIZE);
        char* quote_start = strchr(include_pos, '"');
        char* quote_end = strchr(quote_start + 1, '"');

        if (!quote_start || !quote_end) 
		{
            AS_LOG(LV_WARNING, "Invalid include directive: " + std::string(include_pos));
        }

        strncpy(include_file, quote_start + 1, quote_end - quote_start - 1);
        include_file[quote_end - quote_start - 1] = '\0';

		char* included_file = (char*)AS_MALLOC(sizeof(char) * MAX_FILE_SIZE);
        expand_file_includes(include_file, current_path, included_file);

        // Append the included file content to the output
        strcat(output, included_file);

		AS_FREE(include_file);
		AS_FREE(included_file);

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

	AS_FREE(file_contents);
	file_contents = nullptr;
}

char* expand_file(const char* path, int depth) {
	if (depth > MAX_INCLUDE_DEPTH) {
		printf("Maximum depth reached. Aborting.\n");
		return nullptr;
	}

	

	FILE* file = fopen(path, "r");
	if (!file) {
		printf("Failed to open file %s.\n", path);
		return nullptr;
	}

	char line[1024];
	char* expanded_contents = (char*)AS_MALLOC(1);  // allocate initial memory for empty string
	expanded_contents[0] = '\0';  // initialize string as empty
	while (fgets(line, sizeof(line), file)) {
		if (strncmp(line, "#include", 8) == 0) {
			char included_file[1024];
			sscanf(line, "#include \"%[^\"]\"", included_file);
			char* included_contents = expand_file(included_file, depth - 1);
			size_t old_size = strlen(expanded_contents);
			size_t included_size = strlen(included_contents);
			expanded_contents = (char*)AS_REALLOC(expanded_contents, old_size + included_size + 1);
			if (!expanded_contents) {
				printf("Failed to allocate memory for expanded file.\n");
				return nullptr;
			}
			strcat(expanded_contents, included_contents);
			free(included_contents);
		}
		else {
			size_t old_size = strlen(expanded_contents);
			size_t line_size = strlen(line);
			expanded_contents = (char*)AS_REALLOC(expanded_contents, old_size + line_size + 1);
			if (!expanded_contents) {
				printf("Failed to allocate memory for expanded file.\n");
				return nullptr;
			}
			strcat(expanded_contents, line);
		}
	}

	fclose(file);
	return expanded_contents;
}

std::string as::util::read_file_expanded_includes(const std::string& file_path)
{
	std::string full_file_path = util::get_current_path() + "/../" + file_path;

	std::string test_str(expand_file(full_file_path.c_str(), 10));

	std::cout << test_str;

	std::string include_string = file_path;
	i64 found_index = file_path.find_last_of("/");
	std::string file_content = util::read_file(full_file_path.c_str());

	found_index = file_content.find("#version 330 core");
	if (found_index > -1)
	{
		file_content.erase(found_index, 18);
	}
	found_index = file_content.find("#include");
	if (found_index > -1)
	{
		include_string.erase(found_index, 10); // removes #include" (#include size + 1 for the ")
		found_index = include_string.find(L'"');
		if (found_index > -1)
		{
			include_string.erase(found_index, include_string.size() - 1);
		}
		std::string temp_string;
		for (i32 i = 0 ; i < include_string.size(); i++)
		{
			if (!isspace(include_string[i]))
			{
				temp_string.push_back(include_string[i]);
			}
		}
		include_string = temp_string;
		return read_file_expanded_includes(include_string) + file_content;
	}
	else
	{
		return file_content;
	}
}

void as::util::split_path(const char* path, char* directory, char* filename)
{
	const char* separator = strrchr(path, '/');
	if (!separator) {
		// No directory separator found, assume path is just a file name
		strcpy(filename, path);
		directory[0] = '\0';
	}
	else {
		size_t path_length = separator - path + 1;
		strncpy(directory, path, path_length);
		directory[path_length] = '\0';
		strcpy(filename, separator + 1);
	}
}

size_t as::util::get_file_size(FILE* file)
{
	if (file)
	{
		fseek(file, 0, SEEK_END); // seek to end of file
		size_t file_size = ftell(file); // get current file pointer
		fseek(file, 0, SEEK_SET); // seek back to beginning of file
		return file_size;
	}
	AS_LOG(LV_WARNING, "File is null, cannot get file size");
	return 0;
}

void as::util::update_path_to_absolute(char* path)
{
	std::string current_path_string = as::util::get_current_path();
	const char* current_path = current_path_string.c_str();
	const char* separator = "/../";

	// Calculate the lengths of strings
	size_t current_path_len = strlen(current_path);
	size_t separator_len = strlen(separator);
	size_t path_len = strlen(path);

	// Calculate the total length of the new path
	size_t new_path_len = current_path_len + separator_len + path_len + 1; // +1 for the null terminator

	// Check if the provided buffer is large enough
	if (new_path_len > MAX_PATH_SIZE)
	{
		// Handle the error accordingly
		return;
	}

	// Create a temporary buffer to hold the new path
	char new_path[MAX_PATH_SIZE];

	// Copy the current path to the new path buffer
	strcpy(new_path, current_path);

	// Concatenate the separator and the path
	strcat(new_path, separator);
	strcat(new_path, path);

	// Copy the updated path back to the provided buffer
	strcpy(path, new_path);
}

std::string as::util::vec3_to_string(const Vector3& vec)
{
	return std::to_string(vec.x) + ";" + std::to_string(vec.y) + ";" + std::to_string(vec.z);
}

std::string as::util::bool_to_string(const bool& boolean)
{
	return (boolean) ? "true" : "false";
}


bool as::util::ends_with(const char* str, const char* suffix)
{
	size_t str_len = strlen(str);
	size_t suffix_len = strlen(suffix);
	if (suffix_len > str_len)
	{
		return false;
	}
	return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

void as::timer::start_timer(timer::handle& handle)
{
	handle.start = std::chrono::system_clock::now();
}

f64 as::timer::get_current_time(timer::handle& handle)
{
	std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
	std::chrono::duration<f64> diff = current_time - handle.start;
	return diff.count();
}
