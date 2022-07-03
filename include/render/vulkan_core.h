#pragma once
#include "render/vulkan.h"

// Stupid std include that I wanna get rid of
#include <functional>

#define VULKAN_VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

namespace as
{
	/** Initialization */

	// Main initialization function
	VkResult init_vulkan(vulkan_interface* out_interface, const vulkan_interface_create_info& create_info);

	VkResult initialize_vulkan_instance(VkInstance* instance, const bool& enable_validation_layers);
	VkResult construct_vulkan_devices(vulkan_interface* in_interface);
	VkResult construct_vulkan_devices(VkInstance* in_instance, vulkan_device* &out_devices, u32& out_device_count);
	VkResult initialize_vulkan_devices(vulkan_device* devices, const u32& device_count, const vulkan_device_create_info& create_info);
	VkResult initialize_vulkan_device(vulkan_device* device, const vulkan_device_create_info& create_info);

	/** Pool & Commands */

	VkResult create_command_pool(VkCommandPool* out_command_pool, VkDevice* logical_device, const u32& queue_index);
	VkResult create_command_pool(vulkan_device*& out_vulkan_device, const u32& queue_index);
	VkResult create_command_buffer(vulkan_device*& device, const u8& start_buffer = true);
	VkResult create_command_buffer(VkCommandBuffer* out_command_buffer, VkDevice* logical_device, VkCommandPool* command_pool, const u8& start_buffer = true);

	/** Memory */
	
	VkResult get_memory_type(u32* out_type, const VkPhysicalDeviceMemoryProperties& memory_properties, u32& typeBits, const VkMemoryPropertyFlags& properties);
	VkResult allocate_memory(vulkan_memory* out_memory, const vulkan_memory_create_info& create_info);
	VkResult edit_memory_payload(vulkan_memory* memory, std::function<void(i32*)> payload_edit_fn);

	/** Buffers */

	VkResult create_buffer(VkBuffer* out_buffer, vulkan_memory* memory, const u32& queue_family_index);

	/** Shaders & Descriptors */

	VkResult compile_shader(shader_binaries* out_compiled_shader, const shader_compile_info& compile_info);
	VkResult create_shader(vulkan_shader* out_shader, const vulkan_shader_create_info& create_info);
	VkResult start_shader(vulkan_shader* in_shader, VkCommandBuffer* in_command_buffer, const u32& buffer_size);

	/** Depth Stencil */

	VkResult get_depth_format(VkFormat* out_Format, VkPhysicalDevice* physical_device);
	VkResult create_depth_stencil(vulkan_device* &device, const u32& width, const u32& height);

	/** Synchronization */

	VkResult create_fences(vulkan_device*& device);
	VkResult construct_fence(VkFence* out_fence, VkDevice* logical_device);

	/** Queues */

	VkResult submit_queue(VkQueue* in_queue, VkCommandBuffer* in_command_buffer);
	VkResult get_best_transfer_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index);
	VkResult get_best_compute_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index);
	VkResult get_device_queue(VkQueue* out_queue, vulkan_device* in_vulkan_device);

	/** Validation layers & debug extensions */

	bool check_validation_layer_support();
	char** get_required_extensions();
};
