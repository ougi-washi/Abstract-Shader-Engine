#pragma once
#include "types.h"
#include "engine_types.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

#define MAX_FILE_SIZE 30720
#define MAX_FILE_PATH_SIZE 1024
#define MAX_INCLUDES 10

namespace as
{
	namespace util
	{
		/** Files & Directories */
		FILE* try_open_file(const char* file_path, const char* mode);
		std::string get_root_path();
		std::string get_current_path();
		std::string get_relative_path(const char* to_path);
		std::string get_project_path();
		std::string get_directory(const std::string path);
		i32 does_dir_exist(const char* path);
		bool does_file_exist(const std::string& path);
		void replace_char(std::string & string_to_update, const char& source, const char& target);
		std::string read_file(const std::string& filename);
		json read_json_file(const std::string& path);
		std::string read_file(const char* filename);
		void write_file_str(const char* filename, const char* data);
		void expand_file_includes(const char* filename, const char* current_path, char* output);
		std::string read_file_expanded_includes(const std::string& file_path);
		void split_path(const char* path, char* directory, char* filename);
		size_t get_file_size(FILE* file);
		void update_path_to_absolute(char* path);

		/** string */
		std::string vec3_to_string(const Vector3& vec);
		std::string bool_to_string(const bool& boolean);
		bool ends_with(const char* str, const char* suffix);
	};

	namespace timer
	{
		void start_timer(timer::handle& handle);
		f64 get_current_time(timer::handle& handle);
	};
};
