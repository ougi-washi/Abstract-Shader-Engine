#include "utility.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <windows.h>
#endif

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
#if defined(_WIN32)
				bool mkdir_result = CreateDirectory(folder_to_create.c_str(), NULL);
#else 
				i32 mkdir_result = mkdir(folder_to_create.c_str(), 0700);
#endif
				if (!mkdir_result)
				{
					AS_LOG(LV_WARNING, "Folder [" + folder_to_create + "] could not be created, a crash may occur");
				}
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

std::vector<char> as::util::read_file(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	std::vector<char> buffer;

	if (file.is_open()) 
	{
		size_t file_size = (size_t)file.tellg();
		buffer.resize(file_size);
		file.seekg(0);
		file.read(buffer.data(), file_size);
		file.close();
	}
	else
	{
		AS_LOG(LV_WARNING, "File [" + filename + "] failed to open");
	}
	return buffer;
}

char* as::util::read_file(const char* filename)
{
	FILE* f = fopen(filename, "rb");
	if (!f)
	{
		AS_LOG(LV_WARNING, "File [" + std::string(filename) + "] not found, a crash may occur");
		return nullptr;
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
	}
	return string;
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
