
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
		*wait_dest_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // TODO: C6011
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
		vkGetPhysicalDeviceMemoryProperties(device->physical, &device->properties);

		create_command_pool(device, queue_family_index);
		create_command_buffer(device, true);
		create_fences(device);

		const i32 bufferLength = 16384;
		const u32 bufferSize = sizeof(i32) * bufferLength;

	}
	return VK_SUCCESS;
}

VkResult use::create_command_pool(vulkan_device* &device, const u32 &queue_index)
{
	return construct_command_pool(&device->command_pool, &device->logical, queue_index);
}

VkResult use::construct_command_pool(VkCommandPool* out_command_pool, VkDevice* logical_device, const u32& queue_index)
{
	VkCommandPoolCreateInfo command_pool_info = {};
	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.queueFamilyIndex = queue_index;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // I am not sure if needed
	if (!out_command_pool)
	{
		out_command_pool = (VkCommandPool*)malloc(sizeof VkCommandPool);
	}
	CHECK_RESULT(vkCreateCommandPool(*logical_device, &command_pool_info, nullptr, out_command_pool));
	return VK_SUCCESS;
}

VkResult use::create_command_buffer(vulkan_device*& device, const u8& start_buffer)
{
	return construct_command_buffer(&device->command_buffer, &device->logical, &device->command_pool, start_buffer);
}

VkResult use::construct_command_buffer(VkCommandBuffer* out_command_buffer, VkDevice* logical_device, VkCommandPool* command_pool, const u8& start_buffer)
{
	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.pNext = 0;
	command_buffer_allocate_info.commandPool = *command_pool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = 1; // TODO: create a system to create multiple buffers
	if (!out_command_buffer)
	{
		out_command_buffer = (VkCommandBuffer*)malloc(sizeof VkCommandBuffer);
	}
	CHECK_RESULT(vkAllocateCommandBuffers(*logical_device, &command_buffer_allocate_info, out_command_buffer));

	if (start_buffer)
	{
		VkCommandBufferBeginInfo command_buffer_begin_info = {};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.pNext = 0;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		command_buffer_begin_info.pInheritanceInfo = 0;
		CHECK_RESULT(vkBeginCommandBuffer(*out_command_buffer, &command_buffer_begin_info)); // TODO C6011
	}
	return VK_SUCCESS;
}

VkResult use::get_memory_type(u32* out_type, const VkPhysicalDeviceMemoryProperties& memory_properties, u32& typeBits, const VkMemoryPropertyFlags& properties)
{
	for (u32 i = 0; i < memory_properties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (!out_type)
				{
					return VK_ERROR_INITIALIZATION_FAILED;
				}
				*out_type = i;
				return VK_SUCCESS;
			}
		}
		typeBits >>= 1;
	}
	return VK_ERROR_INITIALIZATION_FAILED;
}

VkResult use::allocate_memory(vulkan_memory*& out_memory, const vulkan_memory_create_info& create_info)
{
	assert(out_memory);
	assert(create_info.device);
	out_memory->device = create_info.device;
	out_memory->size = create_info.buffer_size;

	// set memoryTypeIndex to an invalid entry in the properties.memoryTypes array
	u32 memoryTypeIndex = VK_MAX_MEMORY_TYPES;

	for (u32 k = 0; k < out_memory->device->properties.memoryTypeCount; k++)
	{
		if ((VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & out_memory->device->properties.memoryTypes[k].propertyFlags) &&
			(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & out_memory->device->properties.memoryTypes[k].propertyFlags) &&
			(out_memory->size < out_memory->device->properties.memoryHeaps[out_memory->device->properties.memoryTypes[k].heapIndex].size))
		{
			memoryTypeIndex = k;
			break;
		}
	}
	CHECK_RESULT(memoryTypeIndex == VK_MAX_MEMORY_TYPES ? VK_ERROR_OUT_OF_HOST_MEMORY : VK_SUCCESS);
	const VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, 0, out_memory->size, memoryTypeIndex };
	CHECK_RESULT(vkAllocateMemory(out_memory->device->logical, &memoryAllocateInfo, 0, &out_memory->device_memory));
	return VK_SUCCESS;
}

VkResult use::edit_memory_payload(vulkan_memory* memory, std::function<void(i32*)> payload_edit_fn)
{
	assert(memory);
	assert(memory->device);
	i32* payload;
	CHECK_RESULT(vkMapMemory(memory->device->logical, memory->device_memory, 0, memory->size, 0, (void**)&payload));
	payload_edit_fn(payload);
	vkUnmapMemory(memory->device->logical, memory->device_memory);
	return VK_SUCCESS;
}

VkResult use::create_buffer(VkBuffer*& out_buffer, vulkan_memory* memory, const u32& queue_family_index)
{
	assert(memory);
	assert(memory->device);
	assert(memory->device->logical);
	assert(out_buffer);

	VkBufferCreateInfo buffer_create_info = {};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.pNext = 0;
	buffer_create_info.flags = 0;
	buffer_create_info.size = memory->size;
	buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.queueFamilyIndexCount = 1;
	buffer_create_info.pQueueFamilyIndices = &queue_family_index;

	CHECK_RESULT(vkCreateBuffer(memory->device->logical, &buffer_create_info, 0, out_buffer));
	CHECK_RESULT(vkBindBufferMemory(memory->device->logical, *out_buffer, memory->device_memory, 0));
	return VK_SUCCESS;
}

VkResult use::create_shader_module(VkShaderModule*& out_shader_module, const vulkan_shader_create_info& create_info)
{
	return VK_SUCCESS;
}

VkResult use::compile_shader(char* out_bytes, const shader_compile_info& compile_info)
{
	//shaderc_compile_options_t options = *new shaderc_compile_options();
	//shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level::shaderc_optimization_level_performance);
	//
	//shaderc_compiler_t compiler;
	//compiler = shaderc_compiler_initialize();
	//
	//shaderc_compilation_result_t result = shaderc_compile_into_spv(
	//	compiler, compile_info.source, std::strlen(compile_info.source), compile_info.kind,
	//	compile_info.file_name, "main", nullptr);
	//shaderc_compilation_status status = shaderc_result_get_compilation_status(result); // TODO C26812
	//if (status == shaderc_compilation_status_success) 
	//{
	//	if (out_bytes)
	//	{
	//		USE_LOG(USE_LOG, int(status));
	//		//out_bytes = (char*)realloc(out_bytes, size);
	//	}
	//}
	//else
	//{
	//	USE_LOG(USE_ERROR, shaderc_result_get_error_message(result));
	//}
	//shaderc_result_release(result);
	//shaderc_compiler_release(compiler);

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

	{  // Compiling
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Like -DMY_DEFINE=1
		options.AddMacroDefinition("MY_DEFINE", "1");
		options.SetTargetSpirv(shaderc_spirv_version::shaderc_spirv_version_1_6);
		options.SetTargetEnvironment(shaderc_target_env::shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		//options.SetOptimizationLevel(shaderc_optimization_level_size);

		shaderc::AssemblyCompilationResult result = compiler.CompileGlslToSpvAssembly(
			compile_info.source, compile_info.kind, compile_info.file_name, options);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			std::cerr << result.GetErrorMessage();
		}
		std::string result_string = { result.cbegin(), result.cend() };
		std::cout << result_string << std::endl;

		shaderc::SpvCompilationResult result_module =
			compiler.CompileGlslToSpv(
				compile_info.source, compile_info.kind, compile_info.file_name, options);
		
		if (result_module.GetCompilationStatus() != shaderc_compilation_status_success) {
			std::cerr << result_module.GetErrorMessage();
		}
		std::vector<uint32_t> result_spv = { result_module.cbegin(), result_module.cend() };

		std::cout << "SPIR-V assembly:" << std::endl << result_string << std::endl;

		std::cout << "Compiled to a binary module with " << result_spv.size()
			<< " words." << std::endl;
	}



	return VK_SUCCESS;
}

VkResult use::get_depth_format(VkFormat* out_Format, VkPhysicalDevice* physical_device)
{
	const u8 total_formats = 5;
	VkFormat formats[total_formats] =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};
	for (u8 i = 0 ; i < total_formats ; i++)
	{
		VkFormatProperties format_properties;
		vkGetPhysicalDeviceFormatProperties(*physical_device, formats[i], &format_properties);
		// Format must support depth stencil attachment for optimal tiling
		if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			if (!out_Format)
			{
				out_Format = (VkFormat*)malloc(sizeof VkFormat);
			}
			*out_Format = formats[i];
			return VK_SUCCESS;
		}
	}
	return VK_ERROR_UNKNOWN;
}

VkResult use::create_depth_stencil(vulkan_device*& device, const u32& width, const u32& height)
{
	VkFormat depth_format;
	CHECK_RESULT(get_depth_format(&depth_format, &device->physical));

	VkImageCreateInfo image_create_info{};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = depth_format;
	image_create_info.extent = { width, height, 1 };
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	CHECK_RESULT(vkCreateImage(device->logical, &image_create_info, nullptr, &device->depth_stencil.image));
	VkMemoryRequirements memory_requirements{};
	vkGetImageMemoryRequirements(device->logical, device->depth_stencil.image, &memory_requirements);

	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memory_requirements.size;
	u32 memory_type = 0;
	CHECK_RESULT(get_memory_type(&memory_type, device->properties, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
	memory_allocate_info.memoryTypeIndex = memory_type;
	CHECK_RESULT(vkAllocateMemory(device->logical, &memory_allocate_info, nullptr, &device->depth_stencil.mem));
	CHECK_RESULT(vkBindImageMemory(device->logical, device->depth_stencil.image, device->depth_stencil.mem, 0));

	VkImageViewCreateInfo image_view_create_info{};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.image = device->depth_stencil.image;
	image_view_create_info.format = depth_format;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.levelCount = 1;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount = 1;
	image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
	if (depth_format >= VK_FORMAT_D16_UNORM_S8_UINT) 
	{
		image_view_create_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	CHECK_RESULT(vkCreateImageView(device->logical, &image_view_create_info, nullptr, &device->depth_stencil.view));
	return VK_SUCCESS;
}

VkResult use::create_fences(vulkan_device*& device)
{
	return construct_fence(&device->compute_fence, &device->logical);
}

VkResult use::construct_fence(VkFence* out_fence, VkDevice* logical_device)
{
	VkFenceCreateInfo fence_create_info = {};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.pNext = 0;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if (!out_fence)
	{
		out_fence = (VkFence*)malloc(sizeof VkFence);
	}
	CHECK_RESULT(vkCreateFence(*logical_device, &fence_create_info, nullptr, out_fence));
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