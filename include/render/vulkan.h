#pragma once
#include "types.h"

// GLFW (This includes vulkan)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// SHADERC
//#include <shaderc/shaderc.hpp>
#include <shaderc/shaderc.hpp>
//#include <shaderc/third_party/spirv-tools>

namespace use
{
	/** device start */

	enum vulkan_device_type : u8
	{
		NONE = 0,
		GRAPHICS = 1 << 0,
		COMPUTE = 1 << 1
	};

	struct vulkan_device
	{
		/** main data */
		vulkan_device_type type;
		VkPhysicalDevice physical;
		VkDevice logical;
		VkPhysicalDeviceMemoryProperties properties;
		
		/** Commands */
		VkCommandPool command_pool;
		VkCommandBuffer command_buffer; // TODO: system for many buffers

		/** Buffers & Synchronization */
		VkSubmitInfo submit_info;
		VkSemaphore compute_semaphore;
		VkSemaphore presentation_semaphore;
		VkSemaphore render_semaphore;
		VkFence compute_fence;
		VkFence presentation_fence; // TODO: there should be a total number of fences equal to the number of buffers

		// Depth
		struct depth_stencil
		{
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		} depth_stencil;
	};

	struct vulkan_device_create_info
	{
		// properties, features, memory properties, queue types
		vulkan_device_type type;
	};

	/** device end */

	/** interface start */

	struct vulkan_interface
	{
		VkInstance instance;
		u32 device_count;
		vulkan_device* devices;

		vulkan_interface() : instance(VkInstance()), device_count(0), devices(nullptr) {};
	};

	struct vulkan_interface_create_info
	{
		u8 is_compute : 1; // change to array of types
		u8 debug : 1;
	};

	/** interface end */

	/** memory start */

	struct vulkan_memory
	{
		vulkan_device* device;
		VkDeviceMemory device_memory;
		VkDeviceSize size;

		vulkan_memory() : device(nullptr), device_memory(VkDeviceMemory()), size(VkDeviceSize()) {};
	};

	struct vulkan_memory_create_info
	{
		vulkan_device* device;
		i32 buffer_size;

		vulkan_memory_create_info() : device(nullptr), buffer_size(0) {};
	};

	/** memory end */

	/** shader start */

	struct vulkan_shader
	{
		char* data;
		VkShaderModule module;
		VkDescriptorSetLayout descriptor_set_layout;
		VkDescriptorSet descriptor_set;
		VkDescriptorPool descriptor_layout;
		VkPipelineLayout pipeline_layout;
		VkPipeline pipeline;
	};

	struct vulkan_shader_create_info
	{

	};

	struct shader_compile_info
	{
		char* file_name;
		char* source;
		shaderc_shader_kind kind;
	};

	/** shader end */

};
