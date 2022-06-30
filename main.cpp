#include "render/vulkan_core.h"

int main() 
{
	use::vulkan_interface vk_interface;
	use::vulkan_interface_create_info vulkan_interface_create_info = {};
	CHECK_RESULT(use::init_vulkan(&vk_interface, vulkan_interface_create_info));
	
	const char shader_source[] =
		"#version 310 es\n"
		"void main() { }\n";
	char* shader_bin = nullptr;
	use::shader_compile_info compile_info = {};
	compile_info.file_name = new char[10]("main.vert");
	compile_info.source = new char[250](*shader_source);

	use::compile_shader(shader_bin, compile_info);
	USE_LOG(USE_LOG, "OK");
	return 0;
}