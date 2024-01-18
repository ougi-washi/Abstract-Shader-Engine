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

char* process_shader_source_recursive(const char* source, size_t* size, const char* base_path) 
{
	const char* include_prefix = "#include \"";
	size_t include_prefix_len = strlen(include_prefix);
	const char* include_suffix = "\"";

	char* processed_source = strdup(source);

	char* include_start = strstr(processed_source, include_prefix);
	while (include_start)
	{
		char* include_end = strstr(include_start + include_prefix_len, include_suffix);
		if (!include_end)
		{
			// Malformed #include directive
			break;
		}

		// Extract the filename from the include directive
		size_t filename_len = include_end - (include_start + include_prefix_len);
		char* filename = strdup(include_start + include_prefix_len, filename_len); //??

		// Resolve the full path to the included file
		char* full_path;
		if (base_path)
		{
			size_t base_path_len = strlen(base_path);
			size_t full_path_len = base_path_len + 1 + filename_len + 1;
			full_path = (char*)AS_MALLOC(full_path_len);

			memcpy(full_path, base_path, base_path_len);
			full_path[base_path_len] = '/';
			memcpy(full_path + base_path_len + 1, filename, filename_len);
			full_path[full_path_len - 1] = '\0';
		}
		else
		{
			full_path = strdup(filename);
		}

		// Read the content of the included file
		size_t included_size;
		char* included_content = read_file(full_path, &included_size);

		// Free temporary memory
		AS_FREE(filename);
		AS_FREE(full_path);

		if (!included_content)
		{
			// Failed to read the included file
			break;
		}

		// Process includes in the included content
		char* processed_included_content = process_shader_source_recursive(included_content, &included_size, base_path);

		// Replace the include directive with the processed included content
		size_t prefix_len = include_start - processed_source;
		char* full_suffix = include_end;
		strcat(full_suffix, include_suffix);
		size_t suffix_len = strlen(full_suffix);
		size_t new_source_len = prefix_len + included_size + suffix_len;
		char* new_source = (char*)AS_MALLOC(new_source_len + 1);

		memcpy(new_source, processed_source, prefix_len);
		memcpy(new_source + prefix_len, processed_included_content, included_size);
		memcpy(new_source + prefix_len + included_size, full_suffix, suffix_len);
		new_source[new_source_len] = '\0';

		// Free temporary memory
		AS_FREE(processed_source);
		AS_FREE(included_content);
		AS_FREE(processed_included_content);

		processed_source = new_source;

		// Search for the next #include directive
		include_start = strstr(processed_source, include_prefix);
	}

	*size = strlen(processed_source);
	return processed_source;
}

i32 as_shader_compile(const char* source, const char* entry_point, const as_shader_type shader_type, u32** spirv_code, size_t* spirv_size)
{
	size_t out_size = 0;
	char* processed_source = process_shader_source_recursive(source, &out_size, NULL);
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	if (!compiler) 
	{
		AS_FREE(processed_source);
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
		AS_FREE(processed_source);
		return -1;
	}

	shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);
	shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_performance);
	//shaderc_compile_options_set_auto_bind_uniforms(options, true);

	shaderc_compile_options_set_generate_debug_info(options);

	const i32 processed_source_size = strlen(processed_source);
	shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, processed_source, processed_source_size, kind, "source", entry_point, options);
	shaderc_compiler_release(compiler);
	shaderc_compile_options_release(options);

	AS_FREE(processed_source);

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
	char* source_code = read_file(filename, &file_size);
	if (!source_code)
	{
		return output;
	}

	//u32* spirv_code = NULL;
	size_t spirv_size = 0;
	i32 compile_result = as_shader_compile(source_code, "main", shader_type, &output->bin, &output->size);
	AS_FREE(source_code);
	return output;
	/*
	if (compile_result != 0)
	{
		return output;
	}*/
}

void as_shader_destroy_binary(as_shader_binary* shader_bin, const bool is_ptr)
{
	AS_FREE(shader_bin->bin);
	if (is_ptr)
	{
		AS_FREE(shader_bin);
	}
}

