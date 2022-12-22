#pragma once

#include "shaderc.h"
#include <vector>

namespace as
{
	// Shaderc
	namespace sc
	{
		bool compile_shader(shader_binaries* out_compiled_shader, const shader_compile_info& compile_info);
		bool compile_shader_with_cache(const char* file_path, const shaderc_shader_kind& kind, std::vector<char>& out_spv);
		bool compile_vertex_shader(const char* file_path, std::vector<char>& out_spv);
		bool compile_fragment_shader(const char* file_path, std::vector<char>& out_spv);
		void write_shader_bin(const char* filename, const shader_binaries& shader_bin);
	}
};
