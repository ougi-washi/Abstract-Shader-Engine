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

	/** Commands */

	VkResult create_command_pool(vulkan_device* &device);
	VkResult construct_command_pool(VkCommandPool* out_command_pool, VkDevice* logical_device, const u32& queue_index);
	VkResult create_command_buffer(vulkan_device*& device, const u8& start_buffer = true);
	VkResult construct_command_buffer(VkCommandBuffer* out_command_buffer, VkDevice* logical_device, VkCommandPool* command_pool, const u8& start_buffer = true);

	/** Queues */

	VkResult get_best_transfer_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index);
	VkResult get_best_compute_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index);
};