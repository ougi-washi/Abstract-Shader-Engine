#pragma once
#include "render/vulkan.h"

namespace use
{
	/** Initialization */

	// Main initialization function
	VkResult initialize_vulkan_instance(vulkan_instance* instance);
	// This is called implicitly in instance initialization
	VkResult initialize_vulkan_device(vulkan_device* device);


	/** Queues */

	VkResult get_best_transfer_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index);
	VkResult get_best_compute_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index);
};