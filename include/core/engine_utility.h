#pragma once
#include "types.h"
#include "engine_types.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

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
		i32 does_dir_exist(const char* path);
		bool does_file_exist(const std::string& path);
		void replace_char(std::string & string_to_update, const char& source, const char& target);
		std::vector<char> read_file(const std::string& filename);
		char* read_file(const char* filename);
		void write_file_str(const char* filename, const char* data);
	};

	namespace timer
	{
		void start_timer(timer::handle& handle);
		f64 get_current_time(timer::handle& handle);
	};
};