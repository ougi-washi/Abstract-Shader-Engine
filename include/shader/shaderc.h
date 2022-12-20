#pragma once
#include "types.h"
#include <shaderc/shaderc.h>

namespace as
{
	// Shaderc
	namespace sc
	{
		struct shader_binaries
		{
			u32* binaries;
			u32 size;

			shader_binaries() : binaries(nullptr), size(0) {};
			operator bool() const { return binaries!= nullptr; }
		};

		struct shader_compile_info
		{
			char* file_name;
			char* source;
			shaderc_shader_kind kind;
			u8 optimize : 1;
		};
	};
}
