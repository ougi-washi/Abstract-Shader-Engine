// external dependencies (libraries)

#include "glad/gl.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#endif //STB_IMAGE_IMPLEMENTATION
#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
#endif // TINYOBJLOADER_IMPLEMENTATION

// internal dependencies (engine)
#include "engine_utility.h"
#include "engine_core.h"

bool check_compilation_status(const u32& shader_to_check)
{
	i32 success;
	char infoLog[512];
	glGetShaderiv(shader_to_check, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader_to_check, 512, NULL, infoLog);
		AS_LOG(LV_ERROR, std::string("Shader compilation failed: ") + infoLog);
		return false;
	}
	return true;
}

bool check_gl_error()
{
	u32 gl_error = glGetError();
	if (gl_error == GL_NO_ERROR)
	{
		return true;
	}
	AS_LOG(LV_ERROR, "GL error : " + std::to_string(gl_error));
	return false;
}

void as::delete_vertex_array(const u32& VAO)
{
	glDeleteVertexArrays(1, &VAO);
}

bool as::create_shader(const char* vert_shader_src, const char* frag_shader_src, as::shader& out_shader)
{
	bool vert_shader_result, frag_shader_result;
	// vertex shader
	out_shader.vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(out_shader.vertex_shader, 1, &vert_shader_src, NULL);
	glCompileShader(out_shader.vertex_shader);
	vert_shader_result = check_compilation_status(out_shader.vertex_shader);

	// fragment shader
	out_shader.fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(out_shader.fragment_shader, 1, &frag_shader_src, NULL);
	glCompileShader(out_shader.fragment_shader);
	frag_shader_result = check_compilation_status(out_shader.fragment_shader);

	return (vert_shader_result && frag_shader_result);
}

bool as::create_shader_from_files(const char* vert_shader_path, const char* frag_shader_path, as::shader& out_shader)
{
	if (vert_shader_path && frag_shader_path)
	{
		std::string vert_shader_path_string = util::get_current_path() + "/../" + std::string(vert_shader_path);
		std::string frag_shader_path_string = util::get_current_path() + "/../" + std::string(frag_shader_path);

		char* vert_shader = util::read_file(vert_shader_path_string.c_str());
		char* frag_shader = util::read_file(frag_shader_path_string.c_str());
		if (vert_shader && frag_shader)
		{
			return create_shader(vert_shader, frag_shader, out_shader);
		}
	}
	return false;
}

void as::create_shader_program(u32& out_shader_program)
{
	out_shader_program = glCreateProgram();
}

bool as::bind_shaders_to_program(const u32& shader_program, const shader& shader_to_attach)
{
	glAttachShader(shader_program, shader_to_attach.vertex_shader);
	glAttachShader(shader_program, shader_to_attach.fragment_shader);
	glLinkProgram(shader_program);

	i32 success;
	char infoLog[512];
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) 
	{
		glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
		AS_LOG(LV_ERROR, std::string("Binding shaders failed: ") + infoLog);
		return false;
	}
	return true;
}

void as::delete_shader(const as::shader& shader)
{
	AS_LOG(LV_LOG, "Deleting shader");
	glDeleteShader(shader.vertex_shader);
	glDeleteShader(shader.fragment_shader);
}

void as::delete_shader_program(const u32& shader_program)
{
	AS_LOG(LV_LOG, "Deleting shader program [" + std::to_string(shader_program) + "]");
	glDeleteProgram(shader_program);
}

void as::add_uniform(const as::uniform& uniform_to_add, as::shader& shader)
{
	shader.uniforms.push_back(uniform_to_add);
}

void as::set_uniform_bool(const u32& shader_program, const char* name, const bool& value)
{
	glUseProgram(shader_program);
	glUniform1i(glGetUniformLocation(shader_program, name), (i32)value);
}

void as::set_uniform_integer(const u32& shader_program, const char* name, const i32& value)
{
	glUseProgram(shader_program);
	glUniform1i(glGetUniformLocation(shader_program, name), value);
}

void as::set_uniform_f32(const u32& shader_program, const char* name, const f32& value)
{
	glUseProgram(shader_program);
	glUniform1f(glGetUniformLocation(shader_program, name), value);
}

void as::bind_uniforms(const as::shader& shader)
{
	for (u16 i = 0 ; i < shader.textures.size() ; i++)
	{
		if (shader.textures[i])
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, shader.textures[i]->id);
			check_gl_error();
		}
	}
}

void as::bind_uniforms(const as::object& object)
{
	if (object.shader_ptr)
	{
		bind_uniforms(*object.shader_ptr);
	}
}

void as::bind_uniforms(const std::vector<as::object>& objects)
{
	for (const as::object& current_object : objects)
	{
		bind_uniforms(current_object);
	}
}

bool as::initialize_object(const f32* vertices, const u32& vertices_size, const u32* indices, const u32& indices_size, u32& VAO, as::object& out_object)
{
	AS_LOG(LV_LOG, "Initializing object with " + std::to_string(vertices_size) + " vertices size and " + std::to_string(indices_size) + " indices size");
	out_object.vertices_size = vertices_size;
	out_object.indices_size = indices_size;

	out_object.vertices = (f32*)malloc(out_object.vertices_size);
	if (out_object.vertices)
	{
		memcpy(out_object.vertices, vertices, vertices_size);
	}
	out_object.indices = (u32*)malloc(out_object.indices_size);
	if (out_object.indices)
	{
		memcpy(out_object.indices, indices, indices_size);
	}

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &out_object.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, out_object.VBO);
	glBufferData(GL_ARRAY_BUFFER, out_object.vertices_size, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &out_object.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out_object.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, out_object.indices_size, indices, GL_STATIC_DRAW);

	// UNBIND

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return check_gl_error();
}

bool as::assign_shader(as::object& object, u32& VAO, as::shader& shader)
{
	AS_LOG(LV_LOG, "Assigning shader to object");
	object.shader_ptr = &shader;

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, object.VBO);
	glBufferData(GL_ARRAY_BUFFER, object.vertices_size, object.vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, object.indices_size, object.indices, GL_STATIC_DRAW);


	for (const as::uniform& current_uniform : shader.uniforms)
	{
		glVertexAttribPointer(current_uniform.id, current_uniform.size, current_uniform.type, current_uniform.normalized, current_uniform.stride, (void*)current_uniform.position);
		glEnableVertexAttribArray(current_uniform.id);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return check_gl_error();
}

bool as::delete_object_data(as::object* object)
{
	if (object)
	{
		AS_LOG(LV_LOG, "Deleting object data");
		free(object->vertices);
		object->vertices = nullptr;
		free(object->indices);
		object->indices = nullptr;
		glDeleteBuffers(1, &object->VBO);
		glDeleteBuffers(1, &object->EBO);
	}
	else
	{
		AS_LOG(LV_WARNING, "Cannot delete object data, nullptr");
	}
	return check_gl_error();
}

bool as::load_texture(const char* path, texture& out_texture)
{
	if (path)
	{
		std::string path_string = util::get_current_path() + "/../" + std::string(path);
		AS_LOG(LV_LOG, "Loading texture [" + path_string + "]");
		glGenTextures(1, &out_texture.id);
		glBindTexture(GL_TEXTURE_2D, out_texture.id);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load image, create texture and generate mipmaps
		stbi_set_flip_vertically_on_load(true);
		u8* data = stbi_load(path_string.c_str(), &out_texture.width, &out_texture.height, &out_texture.number_of_channels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, out_texture.width, out_texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			AS_LOG(LV_WARNING, "Cannot load texture [" + path_string + "]");
			return false;
		}
		stbi_image_free(data);
		return true;
	}
	AS_LOG(LV_WARNING, "Cannot load texture, nullptr");
	return false;
}

void as::clear_background()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
}

bool as::draw(const u32& shader_program, const u32& VAO, const std::vector<as::object>& objects)
{
	glUseProgram(shader_program);
	glBindVertexArray(VAO);
	bind_uniforms(objects);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	return check_gl_error();
}