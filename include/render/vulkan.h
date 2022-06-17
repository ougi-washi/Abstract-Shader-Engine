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

	struct vulkan_instance
	{
		VkInstance instance;
		u32 device_count = 0;
		vulkan_device* devices = nullptr;
	};

};
