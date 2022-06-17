#pragma once
#include "render/vulkan.h"

namespace use
{
	/** Initialization */

	// Main initialization function
	VkResult init_vulkan(vulkan_interface* out_interface, const vulkan_interface_create_info& create_info);

	VkResult initialize_vulkan_instance(VkInstance* instance);
	VkResult construct_vulkan_devices(vulkan_interface* in_interface);
	VkResult construct_vulkan_devices(VkInstance* in_instance, vulkan_device* &out_devices, u32& out_device_count);
	VkResult initialize_vulkan_devices(vulkan_device* devices, const u32& device_count, const vulkan_device_create_info& create_info);
	VkResult initialize_vulkan_device(vulkan_device* device, const vulkan_device_create_info& create_info);

	/** Queues */

	VkResult get_best_transfer_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index);
	VkResult get_best_compute_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index);
};