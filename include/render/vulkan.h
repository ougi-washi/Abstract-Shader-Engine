#pragma once
#include "types.h"

// GLFW (This includes vulkan)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace use
{
	struct vulkan_device
	{
		VkPhysicalDevice physical;
		VkDevice logical;
		VkPhysicalDeviceMemoryProperties properties;
		VkDeviceMemory memory;
		VkDeviceSize size;
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
	};
};
