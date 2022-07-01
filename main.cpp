#include "render/vulkan_core.h"

int main() 
{

	// VULKAN INIT
	as::vulkan_interface vk_interface;
	as::vulkan_interface_create_info vulkan_interface_create_info = {};
	CHECK_RESULT(as::init_vulkan(&vk_interface, vulkan_interface_create_info));
	
	// MEMORY ALLOCATION AND FILLING VRAM
	const i32 buffer_length = 16384;
	const u32 buffer_size = sizeof(i32) * buffer_length;
	const u64 memory_size = buffer_size * 2;
	as::vulkan_memory memory;
	as::vulkan_memory_create_info vulkan_memory_create_info = {};
	vulkan_memory_create_info.buffer_size = memory_size;
	vulkan_memory_create_info.device = &vk_interface.devices[0];
	CHECK_RESULT(as::allocate_memory(&memory, vulkan_memory_create_info));
	CHECK_RESULT(as::edit_memory_payload(&memory, [&](i32* payload) 
	{
		for (u32 i = 1; i < memory_size / sizeof(i32); i++)
		{
			payload[i] = rand();
		}
	}));
	
	VkBuffer buffer;
	CHECK_RESULT(as::create_buffer(&buffer, &memory, vk_interface.devices[0].queue_family_index));


	// SHADER
	char shader_source[] =
		"#version 310 es\n"
		"void main() { int test = 5; test++; if (test == test + 1) {;;}}\n";
	u32* shader_bin = nullptr; 
	as::shader_compile_info compile_info = {};
	compile_info.file_name = new char[10]("main.vert");
	compile_info.source = shader_source;

	//CHECK_RESULT(as::compile_shader(shader_bin, compile_info));
	
	
	return 0;
}