// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_shader.h"
#include <shaderc/shaderc.h>
#include "as_utility.h"
#include "as_memory.h"

const char* compilation_status_texts[] =
{	
	"shaderc_compilation_status_success",
	"shaderc_compilation_status_invalid_stage",
	"shaderc_compilation_status_compilation_error",
	"shaderc_compilation_status_internal_error",
	"shaderc_compilation_status_null_result_object",
	"shaderc_compilation_status_invalid_assembly",
	"shaderc_compilation_status_validation_error",
	"shaderc_compilation_status_transformation_error",
	"shaderc_compilation_status_configuration_error",
};

i32 as_shader_compile(const char* source, const char* entry_point, const as_shader_type shader_type, u32** spirv_code, size_t* spirv_size)
{
	size_t out_size = 0;

	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	if (!compiler) 
	{
		return -1;
	}

	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);

	shaderc_shader_kind kind;
	if		(shader_type == AS_SHADER_TYPE_VERTEX)		kind = shaderc_vertex_shader;
	else if (shader_type == AS_SHADER_TYPE_FRAGMENT)	kind = shaderc_fragment_shader;
	else 
	{
		// Add support for other shader types if needed
		shaderc_compiler_release(compiler);
		shaderc_compile_options_release(options);
		return -1;
	}

	shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);
	shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_performance);
	//shaderc_compile_options_set_auto_bind_uniforms(options, true);

	shaderc_compile_options_set_generate_debug_info(options);

	const i32 processed_source_size = strlen(source);
	shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, source, processed_source_size, kind, "source", entry_point, options);
	shaderc_compiler_release(compiler);
	shaderc_compile_options_release(options);

	const shaderc_compilation_status compilation_status = shaderc_result_get_compilation_status(result);
	AS_LOG(LV_LOG, (compilation_status_texts[compilation_status]));
	
	if (compilation_status != shaderc_compilation_status_success)
	{
		AS_LOG(LV_WARNING, (shaderc_result_get_error_message(result)));
		shaderc_result_release(result);
		return -1;
	}

	*spirv_size = shaderc_result_get_length(result);
	*spirv_code = (u32*)AS_MALLOC(*spirv_size);
	u32* found_binary = (u32*)shaderc_result_get_bytes(result);
	memcpy(*spirv_code, shaderc_result_get_bytes(result), *spirv_size);

	shaderc_result_release(result);
	return 0;
}

as_shader_binary* as_shader_read_code(const char* filename, const as_shader_type shader_type)
{
	as_shader_binary* output = AS_MALLOC_SINGLE(as_shader_binary);
	size_t file_size = 0;
	char* processed_source = (char*)AS_MALLOC(sizeof(char) * AS_MAX_FILE_SIZE);
	as_util_expand_file_includes(filename, processed_source);

	size_t spirv_size = 0;
	i32 compile_result = as_shader_compile(processed_source, "main", shader_type, &output->bin, &output->size);
	AS_FREE(processed_source);
	return output;
}

void as_shader_destroy_binary(as_shader_binary* shader_bin, const bool is_ptr)
{
	AS_FREE(shader_bin->bin);
	if (is_ptr)
	{
		AS_FREE(shader_bin);
	}
}

