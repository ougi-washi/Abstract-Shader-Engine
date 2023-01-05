#include "glad/gl.h"
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

void as::delete_vertex_array(const u32& VAO)
{
	glDeleteVertexArrays(1, &VAO);
}

bool as::create_shader(const char* vert_shader_src, const char* frag_shader_src, shader& out_shader)
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
	glDeleteShader(shader.vertex_shader);
	glDeleteShader(shader.fragment_shader);
}

void as::delete_shader_program(const u32& shader_program)
{
	glDeleteProgram(shader_program);
}

bool as::initialize_object(const float* vertices, const i32& vertices_count, const float* indices, const i32& indices_count, u32& VAO, as::object& out_object)
{
	*out_object.vertices = *vertices;
	*out_object.indices = *indices;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &out_object.VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, out_object.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_count, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// uncomment this call to draw in wire-frame polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	return true;
}

void as::assign_shader(as::object& object, as::shader& shader)
{
	object.shader_ptr = &shader;
}

void as::delete_object_buffers(const as::object& object)
{
	glDeleteBuffers(1, &object.VBO);
}

void as::clear_background()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void as::draw(const u32& shader_program, const u32& VAO)
{
	glUseProgram(shader_program);
	glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
	glDrawArrays(GL_TRIANGLES, 0, 3);
	// glBindVertexArray(0); // no need to unbind it every time 
}
