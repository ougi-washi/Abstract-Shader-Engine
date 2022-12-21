#include "shaderc_core.h"
#include "utility.h"

bool as::sc::compile_shader(shader_binaries* out_compiled_shader, const shader_compile_info& compile_info)
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

bool as::sc::compile_shader_with_cache(char* file_path, const shaderc_shader_kind& kind, std::vector<char>& out_spv)
{
	char* source = util::read_file(file_path);
	if (!source)
	{
		return false;
	}
	as::sc::shader_compile_info compile_info_vertex = {};
	compile_info_vertex.file_name = new char[]("main");
	compile_info_vertex.source = source;
	compile_info_vertex.kind = kind;
	std::string glsl_file_path = util::get_current_path() + std::string("/cache/") + std::string(file_path) + ".glsl";
	std::string spv_file_path = util::get_current_path() + std::string("/cache/") + std::string(file_path) + ".spv";
	std::vector<char> glsl_file_content = as::util::read_file(glsl_file_path);
	bool already_compiled = false;
	bool result = false;
	if (glsl_file_content.data() && *glsl_file_content.data() == *source && util::does_file_exist(spv_file_path))
	{
		already_compiled = true;
		result = true;
	}
	if (!already_compiled)
	{
		as::util::write_file_str(glsl_file_path.c_str(), source); // TODO: add version for caching (to check later)
		as::sc::shader_binaries out_vertex_shader_bin;
		result = as::sc::compile_shader(&out_vertex_shader_bin, compile_info_vertex); // TODO: cache and compare next time, do not compile every time
		as::sc::write_shader_bin(spv_file_path.c_str(), out_vertex_shader_bin);
	}
	out_spv = as::util::read_file(spv_file_path);
	return result;
}

bool as::sc::compile_vertex_shader(char* file_path, std::vector<char>& out_spv)
{
	return compile_shader_with_cache(file_path, shaderc_glsl_vertex_shader, out_spv);
}

bool as::sc::compile_fragment_shader(char* file_path, std::vector<char>& out_spv)
{
	return compile_shader_with_cache(file_path, shaderc_glsl_fragment_shader, out_spv);
}

void as::sc::write_shader_bin(const char* filename, const shader_binaries& shader_bin)
{
	FILE* fptr = fopen(filename, "wb");
	if (fptr)
	{
		fwrite(shader_bin.binaries, shader_bin.size, 1, fptr);
		fclose(fptr);
	}
}
