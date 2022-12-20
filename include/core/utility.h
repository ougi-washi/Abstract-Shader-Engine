#pragma once
#include "types.h"

#include <iostream>
#include <fstream>
#include <filesystem>

namespace as
{
	namespace util
	{
		/** Files */
		FILE* try_open_file(const char* file_path, const char* mode);
		std::string get_root_path();
		std::string get_current_path();
		std::string get_relative_path(const char* to_path);
		i32 does_dir_exist(const char* path);
		void replace_char(std::string & string_to_update, const char& source, const char& target);
		std::vector<char> read_file(const std::string& filename);
		char* read_file(const char* filename);
		void write_file_str(const char* filename, const char* data);
	};
};