// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_shader.h"
#include <shaderc/shaderc.h>
#include "as_memory.h"
#include "defines/as_global.h"

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

i32 as_shader_compile(as_shader_binary* binary, const char* source, const char* entry_point, const as_shader_type shader_type)
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

	binary->binaries_size = shaderc_result_get_length(result);
	memcpy(binary->binaries, shaderc_result_get_bytes(result), binary->binaries_size);

	shaderc_result_release(result);
	return 0;
}

void as_shader_get_cached_path(char* out_path, const char* original_path)
{
	char file_name[AS_MAX_FILE_NAME_SIZE] = "";
	as_util_extract_file_name(original_path, file_name);	
	sprintf(out_path, "%s%s.as_shader_binary", AS_PATH_CACHED_SHADERS, file_name);
}

as_shader_binary* as_shader_read_code(const char* path, const as_shader_type shader_type)
{
	char processed_source[AS_MAX_FILE_SIZE] = { 0 };
	as_util_expand_file_includes(path, processed_source);

	char cached_path[AS_MAX_PATH_SIZE] = {0};
	as_shader_get_cached_path(cached_path, path);

	as_shader_binary* cached_binary = as_shader_binary_deserialize(cached_path);
	if (cached_binary && strcmp(cached_binary->source, processed_source) == 0)
	{
		return cached_binary;
	}
	as_shader_destroy_binary(cached_binary, true);

	as_shader_binary* ouput_binary = AS_MALLOC_SINGLE(as_shader_binary);
	i32 compile_result = as_shader_compile(ouput_binary, processed_source, "main", shader_type);
	
	strcpy(ouput_binary->source, processed_source);
	ouput_binary->source_size = AS_MAX_FILE_SIZE;

	as_shader_binary_serialize(ouput_binary, cached_path);
	return ouput_binary;
}

void as_shader_destroy_binary(as_shader_binary* shader_bin, const bool is_ptr)
{
	AS_FREE(shader_bin->binaries);
	if (is_ptr)
	{
		AS_FREE(shader_bin);
	}
}

bool as_shader_has_changed(const char* path, as_file_pool* file_pool)
{
	AS_WARNING_RETURN_VAL_IF_FALSE(file_pool, false, "Cannot check shader, invalid file pool");

	char proxy_path[AS_MAX_PATH_SIZE] = "";
	strcpy(proxy_path, path);

	//char* processed_source = as_fp_make_handle_c(file_pool);
	char* processed_source = (char*)AS_MALLOC(sizeof(char) * AS_MAX_FILE_SIZE);
	as_util_expand_file_includes(proxy_path, processed_source);

	char cached_path[AS_MAX_PATH_SIZE] = {0};
	as_shader_get_cached_path(cached_path, proxy_path);

	as_shader_binary* cached_binary = as_shader_binary_deserialize(cached_path);
	bool is_same = cached_binary && strcmp(cached_binary->source, processed_source) == 0;
	AS_FREE(processed_source);
	AS_FREE(cached_binary);
	return !is_same;
}

void as_shader_binary_serialize(const as_shader_binary* data, const char* path)
{
    char directory[AS_MAX_PATH_SIZE];
    as_util_extract_base_path(path, directory);
    as_util_ensure_directory_exists(directory);

	FILE* file = fopen(path, "wb");
	AS_ASSERT(file, "Failed to open file for writing");
	fwrite(data, sizeof(as_shader_binary), 1, file);
	fclose(file);
}

as_shader_binary* as_shader_binary_deserialize(const char* path)
{
	char directory[AS_MAX_PATH_SIZE];
    as_util_extract_base_path(path, directory);
    as_util_ensure_directory_exists(directory);

	as_shader_binary* data = AS_MALLOC_SINGLE(as_shader_binary);
	FILE* file = fopen(path, "rb");
	if (!file)
	{
		AS_FREE(data);
		return NULL;
	}

	fread(data, sizeof(as_shader_binary), 1, file);
	fclose(file);
	return data;
}
