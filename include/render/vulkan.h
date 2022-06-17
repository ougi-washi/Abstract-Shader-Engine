#pragma once
#include "types.h"

// GLFW (This includes vulkan)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace use
{
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
		VkDeviceMemory memory;
		VkDeviceSize size;

		/** buffers & synchronization */
		VkSubmitInfo submit_info;
		VkSemaphore compute_semaphore;
		VkSemaphore presentation_semaphore;
		VkSemaphore render_semaphore;
		VkFence compute_fence;
		VkFence presentation_fence;
	};

	struct vulkan_interface
	{
		VkInstance instance;
		u32 device_count = 0;
		vulkan_device* devices = nullptr;
	};

	struct vulkan_interface_create_info
	{
		u8 is_compute : 1; // change to array of types
		u8 debug : 1;
	};

	struct vulkan_device_create_info
	{
		// properties, features, memory properties, queue types
		vulkan_device_type type;
	};
};
