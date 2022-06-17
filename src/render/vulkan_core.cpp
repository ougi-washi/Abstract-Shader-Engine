
#include "render/vulkan_core.h"
#include <stdio.h>
#include <stdlib.h>

//GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
//STB
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
//TINYOBJ
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
//SHADERC
#include <shaderc/shaderc.hpp>


VkResult use::initialize_vulkan_instance(vulkan_instance* instance)
{
	// Instance
	const VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO, 0, "UnknownShaderEngine_VK_Compute", 0, "", 0, VK_MAKE_VERSION(1, 0, 9) };
	const VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, 0, 0, &applicationInfo, 0, 0, 0, 0 };
	CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, 0, &instance->instance));

	// Devices
	CHECK_RESULT(vkEnumeratePhysicalDevices(instance->instance, &instance->device_count, 0));
	instance->devices = (vulkan_device*)malloc(sizeof(vulkan_device) * instance->device_count);
	// allocate temp memory
	VkPhysicalDevice* physical_devices_temp = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * instance->device_count);
	CHECK_RESULT(vkEnumeratePhysicalDevices(instance->instance, &instance->device_count, physical_devices_temp));
	// parse it and copy it to the instance devices
	for (u32 i = 0; i < instance->device_count; i++)
	{
		memcpy(instance->devices[i].physical, physical_devices_temp[i], sizeof(physical_devices_temp[i])); // TODO C6011
	}
	// free temp memory
	free(physical_devices_temp);
	for (u32 i = 0 ; i < instance->device_count ; i++)
	{
		CHECK_RESULT(initialize_vulkan_device(&instance->devices[i]));
	}

	return VK_SUCCESS;
}

VkResult use::initialize_vulkan_device(vulkan_device* device)
{
	u32 queueFamilyIndex = 0;
	CHECK_RESULT(get_best_compute_queue(device->physical, &queueFamilyIndex));

	const float queuePrioritory = 1.0f;
	const VkDeviceQueueCreateInfo deviceQueueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, 0, 0, queueFamilyIndex, 1, &queuePrioritory };
	const VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, 0, 0, 1, &deviceQueueCreateInfo, 0, 0, 0, 0, 0 };
	CHECK_RESULT(vkCreateDevice(device->physical, &deviceCreateInfo, 0, &device->logical));

	VkPhysicalDeviceMemoryProperties properties;
	vkGetPhysicalDeviceMemoryProperties(device->physical, &properties);

	// Init MEMORY
	const i32 bufferLength = 16384;
	const u32 bufferSize = sizeof(i32) * bufferLength;
	// we are going to need two buffers from this one memory
	device->size = bufferSize * 2;
	// set memoryTypeIndex to an invalid entry in the properties.memoryTypes array
	u32 memoryTypeIndex = VK_MAX_MEMORY_TYPES;

	for (u32 k = 0; k < properties.memoryTypeCount; k++) 
	{
		if ((VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & properties.memoryTypes[k].propertyFlags) &&
			(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & properties.memoryTypes[k].propertyFlags) &&
			(device->size < properties.memoryHeaps[properties.memoryTypes[k].heapIndex].size)) 
		{
			memoryTypeIndex = k;
			break;
		}
	}

	CHECK_RESULT(memoryTypeIndex == VK_MAX_MEMORY_TYPES ? VK_ERROR_OUT_OF_HOST_MEMORY : VK_SUCCESS);
	const VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, 0, device->size, memoryTypeIndex };
	CHECK_RESULT(vkAllocateMemory(device->logical, &memoryAllocateInfo, 0, &device->memory));

	return VK_SUCCESS;
}

VkResult use::get_best_transfer_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index)
{
	uint32_t queue_family_properties_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, 0);

	VkQueueFamilyProperties* const queue_family_properties = (VkQueueFamilyProperties*)_alloca(sizeof(VkQueueFamilyProperties) * queue_family_properties_count); // TODO: check warning

	// first try and find a queue that has just the transfer bit set
	for (uint32_t i = 0; i < queue_family_properties_count; i++)
	{
		// mask out the sparse binding bit that we aren't caring about (yet!)
		const VkQueueFlags masked_flags = (~VK_QUEUE_SPARSE_BINDING_BIT & queue_family_properties[i].queueFlags);

		if (!((VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT) & masked_flags) && (VK_QUEUE_TRANSFER_BIT & masked_flags))
		{
			*queue_family_index = i;
			return VK_SUCCESS;
		}
	}

	// otherwise we'll prefer using a compute-only queue,
	// remember that having compute on the queue implicitly enables transfer!
	for (uint32_t i = 0; i < queue_family_properties_count; i++)
	{
		// mask out the sparse binding bit that we aren't caring about (yet!)
		const VkQueueFlags masked_flags = (~VK_QUEUE_SPARSE_BINDING_BIT & queue_family_properties[i].queueFlags);

		if (!(VK_QUEUE_GRAPHICS_BIT & masked_flags) && (VK_QUEUE_COMPUTE_BIT & masked_flags))
		{
			*queue_family_index = i;
			return VK_SUCCESS;
		}
	}

	// lastly get any queue that'll work for us (graphics, compute or transfer bit set)
	for (uint32_t i = 0; i < queue_family_properties_count; i++)
	{
		// mask out the sparse binding bit that we aren't caring about (yet!)
		const VkQueueFlags masked_flags = (~VK_QUEUE_SPARSE_BINDING_BIT & queue_family_properties[i].queueFlags);

		if ((VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) & masked_flags)
		{
			*queue_family_index = i;
			return VK_SUCCESS;
		}
	}

	return VK_ERROR_INITIALIZATION_FAILED;
}

VkResult use::get_best_compute_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index)
{
	uint32_t queue_family_properties_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, 0);

	VkQueueFamilyProperties* const queue_family_properties = (VkQueueFamilyProperties*)_alloca(
		sizeof(VkQueueFamilyProperties) * queue_family_properties_count);

	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, queue_family_properties);

	// first try and find a queue that has just the compute bit set
	for (uint32_t i = 0; i < queue_family_properties_count; i++)
	{
		// mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
		const VkQueueFlags masked_flags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) &
			queue_family_properties[i].queueFlags);

		if (!(VK_QUEUE_GRAPHICS_BIT & masked_flags) && (VK_QUEUE_COMPUTE_BIT & masked_flags))
		{
			*queue_family_index = i;
			return VK_SUCCESS;
		}
	}

	// lastly get any queue that'll work for us
	for (uint32_t i = 0; i < queue_family_properties_count; i++)
	{
		// mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
		const VkQueueFlags masked_flags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) &
			queue_family_properties[i].queueFlags);

		if (VK_QUEUE_COMPUTE_BIT & masked_flags)
		{
			*queue_family_index = i;
			return VK_SUCCESS;
		}
	}

	return VK_ERROR_INITIALIZATION_FAILED;
}
