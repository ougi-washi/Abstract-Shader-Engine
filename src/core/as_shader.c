// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_shader.h"
#include <shaderc/shaderc.h>
#include "as_utility.h"

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
			full_path = (char*)malloc(full_path_len);

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
		free(filename);
		free(full_path);

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
		char* new_source = (char*)malloc(new_source_len + 1);

		memcpy(new_source, processed_source, prefix_len);
		memcpy(new_source + prefix_len, processed_included_content, included_size);
		memcpy(new_source + prefix_len + included_size, full_suffix, suffix_len);
		new_source[new_source_len] = '\0';

		// Free temporary memory
		free(processed_source);
		free(included_content);
		free(processed_included_content);

		processed_source = new_source;

		// Search for the next #include directive
		include_start = strstr(processed_source, include_prefix);
	}

	*size = strlen(processed_source);
	return processed_source;
}

i32 as_compile_shader(const char* source, const char* entry_point, const u8 shader_kind, char** spirv_code, size_t* spirv_size)
{
	size_t out_size = 0;
	char* processed_source = process_shader_source_recursive(source, &out_size, NULL);
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	if (!compiler) 
	{
		free(processed_source);
		return -1;
	}

	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);

	shaderc_shader_kind kind;
	if		(shader_kind == AS_SHADER_TYPE_VERTEX)		kind = shaderc_vertex_shader;
	else if (shader_kind == AS_SHADER_TYPE_FRAGMENT)	kind = shaderc_fragment_shader;
	else 
	{
		// Add support for other shader types if needed
		shaderc_compiler_release(compiler);
		shaderc_compile_options_release(options);
		free(processed_source);
		return -1;
	}

	shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);
	shaderc_compile_options_set_auto_bind_uniforms(options, true);

	shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, processed_source, strlen(processed_source), kind, "source", entry_point, options);
	shaderc_compiler_release(compiler);
	shaderc_compile_options_release(options);

	free(processed_source);

	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) 
	{
		// Handle shader compilation error
		shaderc_result_release(result);
		return -1;
	}

	*spirv_size = shaderc_result_get_length(result);
	*spirv_code = (char*)malloc(*spirv_size);
	memcpy(*spirv_code, shaderc_result_get_bytes(result), *spirv_size);

	shaderc_result_release(result);
	return 0;
}

