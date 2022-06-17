
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


VkResult use::init_vulkan(vulkan_interface* out_interface, const vulkan_interface_create_info& create_info)
{
	CHECK_RESULT(initialize_vulkan_instance(&out_interface->instance));
	CHECK_RESULT(construct_vulkan_devices(out_interface));
	if (create_info.is_compute || 1)
	{
		vulkan_device_create_info device_create_info = {};
		CHECK_RESULT(initialize_vulkan_devices(out_interface->devices, out_interface->device_count, device_create_info));
	}
	return VK_SUCCESS;
}

VkResult use::initialize_vulkan_instance(VkInstance* instance)
{
	VkApplicationInfo application_info = {};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pApplicationName = "UnknownShaderEngine_VK";
	application_info.pEngineName = "UnknownShaderEngine_VK";
	application_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instance_create_info = {};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pNext = NULL;
	instance_create_info.pApplicationInfo = &application_info;

	// TODO: Extensions here

	return vkCreateInstance(&instance_create_info, 0, instance);
}

VkResult use::construct_vulkan_devices(vulkan_interface* in_interface)
{
	CHECK_RESULT(construct_vulkan_devices(&in_interface->instance, in_interface->devices, in_interface->device_count));
	return VK_SUCCESS;
}

VkResult use::construct_vulkan_devices(VkInstance* in_instance, vulkan_device* &out_devices, u32& out_device_count)
{
	CHECK_RESULT(vkEnumeratePhysicalDevices(*in_instance, &out_device_count, 0));
	if (out_device_count == 0)
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	out_devices = (vulkan_device*)malloc(sizeof(vulkan_device) * out_device_count);
	// allocate temp memory
	VkPhysicalDevice* physical_devices_temp = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * out_device_count);
	CHECK_RESULT(vkEnumeratePhysicalDevices(*in_instance, &out_device_count, physical_devices_temp));
	// parse it and copy it to the instance devices
	for (u32 i = 0; i < out_device_count; i++)
	{
		memcpy(&out_devices[i].physical, &physical_devices_temp[i], sizeof(physical_devices_temp[i])); // TODO C6011
		if (out_devices[i].physical != physical_devices_temp[i])
		{
			return VK_ERROR_UNKNOWN;
		}
	}
	// free temp memory
	free(physical_devices_temp);
	return VK_SUCCESS;
}

VkResult use::initialize_vulkan_devices(vulkan_device* devices, const u32& device_count, const vulkan_device_create_info& create_info)
{
	for (u32 i = 0; i < device_count; i++)
	{
		CHECK_RESULT(initialize_vulkan_device(&devices[i], create_info));
	}
	return VK_SUCCESS;
}

VkResult use::initialize_vulkan_device(vulkan_device* device, const vulkan_device_create_info& create_info)
{
	if (device)
	{
		// TODO: depends on the passed create info

		// Semaphore
		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if (create_info.type & vulkan_device_type::COMPUTE)
		{
			CHECK_RESULT(vkCreateSemaphore(device->logical, &semaphore_create_info, nullptr, &device->compute_semaphore));
		}
		if (create_info.type & vulkan_device_type::GRAPHICS)
		{
			CHECK_RESULT(vkCreateSemaphore(device->logical, &semaphore_create_info, nullptr, &device->presentation_semaphore));
			CHECK_RESULT(vkCreateSemaphore(device->logical, &semaphore_create_info, nullptr, &device->render_semaphore));
		}

		// Submit info
		VkPipelineStageFlags* wait_dest_stage_mask = (VkPipelineStageFlags*)malloc(sizeof VkPipelineStageFlags);
		*wait_dest_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		device->submit_info.pWaitDstStageMask = wait_dest_stage_mask;
		device->submit_info.waitSemaphoreCount = 1;
		device->submit_info.pWaitSemaphores = &device->presentation_semaphore;
		device->submit_info.signalSemaphoreCount = 1;
		device->submit_info.pSignalSemaphores = &device->render_semaphore;



		u32 queue_family_index = 0;
		CHECK_RESULT(get_best_compute_queue(device->physical, &queue_family_index));

		const float queue_prioritory = 1.0f;
		const VkDeviceQueueCreateInfo device_queue_create_info =
		{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, 0, 0, queue_family_index, 1, &queue_prioritory };
		const VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, 0, 0, 1, &device_queue_create_info, 0, 0, 0, 0, 0 };
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
		//i32* payload;
		//CHECK_RESULT(vkMapMemory(device->logical, device->memory, 0, device->size, 0, (void*)&payload));

	}
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
