#include "engine_core.h"
#include "display_core.h"

#include <iostream>
#include <string>

const char* vertex_shader_source = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char* fragment_shader_source = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";

int main()
{
	as::display_handle display_handle;
	as::create_display_handle(false, 600, 300, display_handle);
	
	as::shader shader;
	as::create_shader(vertex_shader_source, fragment_shader_source, shader);

	u32 shader_program;
	as::create_shader_program(shader_program);
	as::bind_shaders_to_program(shader_program, shader);
	as::delete_shader(shader); // (optional)

	std::vector<glm::vec3> vertices = 
	{
		{- .7f, -.7f, 0.0f}, // left  
		{.7f, -.7f, 0.0f}, // right 
		{0.0f,  .9f, 0.0f}  // top   
	};
	std::vector<glm::vec3> indices = {};

	u32 vertex_array_object;
	as::object triangle;
	as::initialize_object(vertices, indices, vertex_array_object, triangle);
	as::assign_shader(triangle, shader);

	while (as::should_display_loop(display_handle))
	{
		as::process_input(display_handle);
		as::clear_background();
		as::draw(shader_program, vertex_array_object);
		as::update(display_handle);
	}

	as::delete_vertex_array(vertex_array_object);
	as::delete_object_buffers(triangle);
	as::delete_shader_program(shader_program);
	as::terminate_display();
}