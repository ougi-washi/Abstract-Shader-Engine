#include "render/vulkan_core.h"

int main() 
{
	// VULKAN INIT
	as::vulkan_interface vk_interface;
	as::vulkan_interface_create_info vulkan_interface_create_info = {};
	vulkan_interface_create_info.debug = true;
	vulkan_interface_create_info.is_compute = true;
	CHECK_RESULT(as::init_vulkan(&vk_interface, vulkan_interface_create_info));
	
	// FILLING VRAM
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
			payload[i] = 1;
		}
	}));
	
	// BUFFERS
	VkBuffer in_buffer;
	CHECK_RESULT(as::create_buffer(&in_buffer, &memory, vk_interface.devices[0].queue_family_index));
	VkBuffer out_buffer;
	CHECK_RESULT(as::create_buffer(&out_buffer, &memory, vk_interface.devices[0].queue_family_index));
	//printf("%d", vk_interface.devices[0]);
	// SHADERS
	as::vulkan_shader shader;
	as::vulkan_shader_create_info shader_create_info = {};
	shader_create_info.logical_device = &vk_interface.devices[0].logical;
	shader_create_info.file_name = new char[]("main.comp");
	shader_create_info.source = new char[](
		"#version 310 es\n"
		"layout(local_size_x = 1, local_size_y = 16) in;\n"
		"layout (set=0, binding = 0) buffer in_buffer_struct {int data[16384];} in_buffer; \n"
		"layout (set=0, binding = 1) buffer out_buffer_struct {int data[16384];} out_buffer; \n"
		"void main() { out_buffer.data[gl_GlobalInvocationID.x]++; }\n"
		);
	shader_create_info.in_buffer = &in_buffer;
	shader_create_info.out_buffer = &out_buffer;
	CHECK_RESULT(as::create_shader(&shader, shader_create_info));
	CHECK_RESULT(as::start_shader(&shader, &vk_interface.devices[0].command_buffer, buffer_size));
	
	// QUEUE
	VkQueue queue;
	CHECK_RESULT(as::get_device_queue(&queue, &vk_interface.devices[0]));
	CHECK_RESULT(as::submit_queue(&queue, &vk_interface.devices[0].command_buffer));
	CHECK_RESULT(as::edit_memory_payload(&memory, [&](i32* payload)
	{
		char full_payload[500000] = "";
		for (u32 i = 1; i < memory_size / sizeof(i32); i++)
		{
			char current_payload[10];
			sprintf(current_payload, "%d", payload[i]);
			strcat(full_payload, current_payload);
			strcat(full_payload, "|");
		}
		AS_LOG(LV_LOG, "Display the full payload after executing the shader: ");
		printf("%s\n", full_payload);
	}));

	as::destroy_buffer(&vk_interface.devices[0].logical, &in_buffer);
	as::destroy_buffer(&vk_interface.devices[0].logical, &out_buffer);
	as::destroy_device_memory(&vk_interface.devices[0].logical, &memory.device_memory);
	as::destroy_shader(&vk_interface.devices[0].logical, &shader);
	as::destroy_vulkan(&vk_interface);
	return 0;
}