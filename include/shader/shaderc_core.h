#pragma once

#include "shaderc.h"

namespace as
{
	// Shaderc
	namespace sc
	{
		bool compile_shader(shader_binaries* out_compiled_shader, const shader_compile_info& compile_info)
		{
			shaderc_compile_options_t options = shaderc_compile_options_initialize();
			shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level::shaderc_optimization_level_performance);
			shaderc_compile_options_set_generate_debug_info(options);
			//shaderc_compile_options_add_macro_definition(options, "test_var", 8, "1", 1); // add test_var val 1
			shaderc_compiler_t compiler;
			compiler = shaderc_compiler_initialize();

			shaderc_compilation_result_t result = shaderc_compile_into_spv(
				compiler, compile_info.source, std::strlen(compile_info.source), compile_info.kind,
				compile_info.file_name, "main", nullptr);
			shaderc_compilation_status status = shaderc_result_get_compilation_status(result); // TODO C26812
			if (status == shaderc_compilation_status_success)
			{
				if (!out_compiled_shader)
				{
					out_compiled_shader = (shader_binaries*)malloc(sizeof(shader_binaries));
				}

				if (out_compiled_shader->binaries)
				{
					free(out_compiled_shader->binaries);
				}

				u32* result_bytes = (u32*)shaderc_result_get_bytes(result);
				out_compiled_shader->size = shaderc_result_get_length(result);

				out_compiled_shader->binaries = (u32*)malloc(out_compiled_shader->size);
				memcpy(out_compiled_shader->binaries, result_bytes, out_compiled_shader->size);
			}
			else
			{
				AS_LOG(LV_ERROR, shaderc_result_get_error_message(result));
			}
			shaderc_result_release(result);
			shaderc_compiler_release(compiler);
			if (status == shaderc_compilation_status_success)
			{
				return true;
			}
			return false;
		}

		void write_shader_bin(const char* filename, const shader_binaries& shader_bin)
		{
			FILE* fptr = fopen(filename, "wb");
			if (fptr)
			{
				fwrite(shader_bin.binaries, shader_bin.size, 1, fptr);
				fclose(fptr);
			}
		}

	}
};
