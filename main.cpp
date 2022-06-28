#include "render/vulkan_core.h"

int main() 
{
	use::vulkan_interface vk_interface;
	use::vulkan_interface_create_info vulkan_interface_create_info = {};
	CHECK_RESULT(use::init_vulkan(&vk_interface, vulkan_interface_create_info));
	
	const char shader_source[] =
		"#version 310 es\n"
		"void main() { int x = MY_DEFINE; }\n";
	u32* shader_bin = nullptr;
	use::shader_compile_info compile_info;
	use::compile_shader(shader_bin, compile_info);

}