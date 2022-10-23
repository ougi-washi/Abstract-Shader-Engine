
#include "render/vulkan_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <set> // TODO : remove
#include <algorithm> // TODO: remove
#include <array> // TODO: remove
#include <fstream> // TODO: remove




VkResult as::init_vulkan(vulkan_interface* out_interface, const vulkan_interface_create_info& create_info)
{
	AS_LOG(LV_LOG, "Initializing Vulkan");
	CHECK_RESULT(initialize_vulkan_instance(&out_interface->instance, create_info.debug));
	CHECK_RESULT(construct_vulkan_devices(out_interface));
	vulkan_device_create_info device_create_info = {};
	if (create_info.is_compute)
	{
		device_create_info.type = vulkan_device_type::COMPUTE;
	}
	CHECK_RESULT(initialize_vulkan_devices(out_interface->devices, out_interface->device_count, device_create_info));
	return VK_SUCCESS;
}

VkResult as::initialize_vulkan_instance(VkInstance* instance, const bool& enable_validation_layers)
{
	AS_LOG(LV_LOG, "Initializing Vulkan instance");
	if (enable_validation_layers && !check_validation_layer_support())
	{
		AS_LOG(LV_WARNING, "No validation layer support available")
	}

	VkApplicationInfo application_info = {};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pApplicationName = "AbstractShaderEngine_VK";
	application_info.pEngineName = "AbstractShaderEngine_VK";
	application_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instance_create_info = {};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pNext = NULL;
	instance_create_info.pApplicationInfo = &application_info;
	instance_create_info.enabledLayerCount = 1;
	const char* validation_layer_names[1] = {VULKAN_VALIDATION_LAYER};
	instance_create_info.ppEnabledLayerNames = validation_layer_names;

	// TODO: Extensions here

	return vkCreateInstance(&instance_create_info, 0, instance);
}

VkResult as::construct_vulkan_devices(vulkan_interface* in_interface)
{
	CHECK_RESULT(construct_vulkan_devices(&in_interface->instance, in_interface->devices, in_interface->device_count));
	return VK_SUCCESS;
}

VkResult as::construct_vulkan_devices(VkInstance* in_instance, vulkan_device* &out_devices, u32& out_device_count)
{
	AS_LOG(LV_LOG, "Constructing Vulkan devices");
	CHECK_RESULT(vkEnumeratePhysicalDevices(*in_instance, &out_device_count, 0));
	if (out_device_count == 0)
	{
		AS_LOG(LV_ERROR, "No physical devices found");
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
			AS_LOG(LV_ERROR, "Device construction, something went wrong with memcpy");
			return VK_ERROR_UNKNOWN;
		}
	}
	// free temp memory
	free(physical_devices_temp);
	return VK_SUCCESS;
}

VkResult as::initialize_vulkan_devices(vulkan_device* devices, const u32& device_count, const vulkan_device_create_info& create_info)
{
	AS_LOG(LV_LOG, "Initializing Vulkan devices");
	for (u32 i = 0; i < device_count; i++)
	{
		CHECK_RESULT(initialize_vulkan_device(&devices[i], create_info));
	}
	return VK_SUCCESS;
}

VkResult as::initialize_vulkan_device(vulkan_device* device, const vulkan_device_create_info& create_info)
{
	if (device)
	{
		AS_LOG(LV_LOG, "Initializing Vulkan device");

		// TODO: depends on the passed create info
		device->type = create_info.type;

		// Submit info
		VkPipelineStageFlags* wait_dest_stage_mask = (VkPipelineStageFlags*)malloc(sizeof VkPipelineStageFlags);
		*wait_dest_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // TODO: C6011
		device->submit_info.pWaitDstStageMask = wait_dest_stage_mask;
		device->submit_info.waitSemaphoreCount = 1;
		device->submit_info.pWaitSemaphores = &device->presentation_semaphore;
		device->submit_info.signalSemaphoreCount = 1;
		device->submit_info.pSignalSemaphores = &device->render_semaphore;

		CHECK_RESULT(get_best_compute_queue(device->physical, &device->queue_family_index));

		const f32 queue_prioritory = 1.0f;
		const VkDeviceQueueCreateInfo device_queue_create_info =
		{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, 0, 0, device->queue_family_index, 1, &queue_prioritory };
		const VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, 0, 0, 1, &device_queue_create_info, 0, 0, 0, 0, 0 };
		CHECK_RESULT(vkCreateDevice(device->physical, &deviceCreateInfo, 0, &device->logical));
		vkGetPhysicalDeviceMemoryProperties(device->physical, &device->properties);

		// Semaphore

		AS_LOG(LV_LOG, "Creating Semaphores");
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

		create_command_pool(device, device->queue_family_index);
		create_command_buffer(device, true);
		create_fences(device);
	}
	return VK_SUCCESS;
}

void as::destroy_vulkan(vulkan_interface* in_interface)
{
	assert(in_interface);
	assert(in_interface->devices);

	for (u8 i = 0 ; i < in_interface->device_count ; i++)
	{
		assert(&in_interface->devices[i].physical);
		assert(&in_interface->devices[i].logical);

		vkDestroyCommandPool(in_interface->devices[i].logical, in_interface->devices[i].command_pool, nullptr);
		
		if (in_interface->devices[i].type & as::vulkan_device_type::COMPUTE)
		{
			vkDestroySemaphore(in_interface->devices[i].logical, in_interface->devices[i].compute_semaphore, nullptr);
			vkDestroyFence(in_interface->devices[i].logical, in_interface->devices[i].compute_fence, nullptr);
		}
		if (in_interface->devices[i].type & as::vulkan_device_type::GRAPHICS)
		{
			vkDestroySemaphore(in_interface->devices[i].logical, in_interface->devices[i].presentation_semaphore, nullptr);
			vkDestroySemaphore(in_interface->devices[i].logical, in_interface->devices[i].render_semaphore, nullptr);
			vkDestroyFence(in_interface->devices[i].logical, in_interface->devices[i].presentation_fence, nullptr);
		}

		vkDestroyDevice(in_interface->devices[i].logical, nullptr);
	}
	
	assert(in_interface->instance);
	vkDestroyInstance(in_interface->instance, nullptr);

	//free(in_interface);

}

VkResult as::create_command_pool(VkCommandPool* out_command_pool, VkDevice* logical_device, const u32& queue_index)
{
	assert(out_command_pool);
	VkCommandPoolCreateInfo command_pool_info = {};
	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.queueFamilyIndex = queue_index;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // I am not sure if needed
	CHECK_RESULT(vkCreateCommandPool(*logical_device, &command_pool_info, nullptr, out_command_pool));
	return VK_SUCCESS;
}

VkResult as::create_command_pool(vulkan_device*& out_vulkan_device, const u32& queue_index)
{
	return create_command_pool(&out_vulkan_device->command_pool, &out_vulkan_device->logical, out_vulkan_device->queue_family_index);
}

VkResult as::create_command_buffer(vulkan_device*& device, const u8& start_buffer)
{
	return create_command_buffer(&device->command_buffer, &device->logical, &device->command_pool, start_buffer);
}

VkResult as::create_command_buffer(VkCommandBuffer* out_command_buffer, VkDevice* logical_device, VkCommandPool* command_pool, const u8& start_buffer)
{
	assert(out_command_buffer);

	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = *command_pool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = 1; // TODO: create a system to create multiple buffers
	CHECK_RESULT(vkAllocateCommandBuffers(*logical_device, &command_buffer_allocate_info, out_command_buffer));

	VkCommandBufferBeginInfo command_buffer_begin_info = {};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	command_buffer_begin_info.pInheritanceInfo = 0;
	CHECK_RESULT(vkBeginCommandBuffer(*out_command_buffer, &command_buffer_begin_info)); // TODO C6011
	return VK_SUCCESS;
}

VkResult as::get_memory_type(u32* out_type, const VkPhysicalDeviceMemoryProperties& memory_properties, u32& typeBits, const VkMemoryPropertyFlags& properties)
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

VkResult as::allocate_memory(vulkan_memory* out_memory, const vulkan_memory_create_info& create_info)
{
	AS_LOG(LV_LOG, "Allocating memory");
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

VkResult as::edit_memory_payload(vulkan_memory* memory, std::function<void(i32*)> payload_edit_fn)
{
	AS_LOG(LV_LOG, "Editing memory payload (this implies mapping and unmapping)");
	assert(memory);
	assert(memory->device);
	i32* payload;
	CHECK_RESULT(vkMapMemory(memory->device->logical, memory->device_memory, 0, memory->size, 0, (void**)&payload));
	payload_edit_fn(payload);
	vkUnmapMemory(memory->device->logical, memory->device_memory);
	return VK_SUCCESS;
}

void as::destroy_device_memory(VkDevice* in_device, VkDeviceMemory* in_device_memory)
{
	assert(in_device_memory);
	assert(in_device);

	vkFreeMemory(*in_device, *in_device_memory, nullptr);
}

VkResult as::create_buffer(VkBuffer* out_buffer, vulkan_memory* memory, const u32& queue_family_index)
{
	AS_LOG(LV_LOG, "Creating buffer");
	assert(memory);
	assert(memory->device);
	assert(memory->device->logical);
	assert(out_buffer);

	VkBufferCreateInfo buffer_create_info = {};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = memory->size;
	buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.queueFamilyIndexCount = 1;
	buffer_create_info.pQueueFamilyIndices = &queue_family_index;

	CHECK_RESULT(vkCreateBuffer(memory->device->logical, &buffer_create_info, 0, out_buffer));
	CHECK_RESULT(vkBindBufferMemory(memory->device->logical, *out_buffer, memory->device_memory, 0));
	return VK_SUCCESS;
}

void as::destroy_buffer(VkDevice* in_device, VkBuffer* in_buffer)
{
	assert(in_buffer);
	assert(in_device);

	vkDestroyBuffer(*in_device, *in_buffer, nullptr);
}

VkResult as::compile_shader(shader_binaries* out_compiled_shader, const shader_compile_info& compile_info)
{
	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level::shaderc_optimization_level_performance);
	shaderc_compile_options_set_generate_debug_info(options);
	//shaderc_compile_options_add_macro_definition(options, "test_var", 8, "1", 1); // add test_var val 1
	shaderc_compiler_t compiler;
	compiler = shaderc_compiler_initialize();

	shaderc_compilation_result_t result = shaderc_compile_into_spv(
		compiler, compile_info.source, std::strlen(compile_info.source), compile_info.kind,
		compile_info.file_name, "main", nullptr);
	shaderc_compilation_status status = shaderc_result_get_compilation_status(result); // TODO C26812
	if (status == shaderc_compilation_status_success)
	{
		if (!out_compiled_shader)
		{
			out_compiled_shader = (shader_binaries*)malloc(sizeof(shader_binaries));
		}

		if (out_compiled_shader->binaries)
		{
			free(out_compiled_shader->binaries);
		}

		u32* result_bytes = (u32*)shaderc_result_get_bytes(result);
		out_compiled_shader->size = shaderc_result_get_length(result);

		out_compiled_shader->binaries = (u32*)malloc(out_compiled_shader->size);
		memcpy(out_compiled_shader->binaries, result_bytes, out_compiled_shader->size);
	}
	else
	{
		AS_LOG(LV_ERROR, shaderc_result_get_error_message(result));
	}
	shaderc_result_release(result);
	shaderc_compiler_release(compiler);
	if (status == shaderc_compilation_status_success)
	{
		return VK_SUCCESS;
	}
	return VK_ERROR_UNKNOWN;
}

VkResult as::create_shader(vulkan_shader* out_shader, const vulkan_shader_create_info& create_info)
{
	AS_LOG(LV_LOG, "Creating shader");
	assert(out_shader);

	/** Compile shader */
	{
		AS_LOG(LV_LOG, "Compiling the shader"); // TODO check if it is cached and it does exist
		shader_binaries out_bin;
		shader_compile_info compile_info;
		compile_info.file_name = create_info.file_name;
		compile_info.source = create_info.source;
		compile_info.kind = shaderc_compute_shader;
		compile_info.optimize = true;
		CHECK_RESULT(compile_shader(&out_bin, compile_info));

		/** Create shader module */
		AS_LOG(LV_LOG, "Creating shader module");
		VkShaderModuleCreateInfo shader_module_create_info = {};
		shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.codeSize = out_bin.size;
		shader_module_create_info.pCode = out_bin.binaries;
		CHECK_RESULT(vkCreateShaderModule(*create_info.logical_device, &shader_module_create_info, nullptr, &out_shader->module));
	}

	/** Descriptors set layout */
	{
		AS_LOG(LV_LOG, "Creating shader descriptor set layout");
		VkDescriptorSetLayoutBinding descriptor_set_layout_binding_0 = {};
		descriptor_set_layout_binding_0.binding = 0;
		descriptor_set_layout_binding_0.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptor_set_layout_binding_0.descriptorCount = 1;
		descriptor_set_layout_binding_0.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		descriptor_set_layout_binding_0.pImmutableSamplers = 0;
		VkDescriptorSetLayoutBinding descriptor_set_layout_binding_1 = descriptor_set_layout_binding_0;
		descriptor_set_layout_binding_1.binding = 1;
		VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[2] =
		{	
			descriptor_set_layout_binding_0,
			descriptor_set_layout_binding_1
		};
		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
		descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.bindingCount = 2;
		descriptor_set_layout_create_info.pBindings = descriptor_set_layout_bindings;
		CHECK_RESULT(vkCreateDescriptorSetLayout(*create_info.logical_device, &descriptor_set_layout_create_info, 0, &out_shader->descriptor_set_layout));
	}

	/** Pipeline layout */
	{
		AS_LOG(LV_LOG, "Creating shader pipeline layout");
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = 1;
		pipeline_layout_create_info.pSetLayouts = &out_shader->descriptor_set_layout;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		pipeline_layout_create_info.pPushConstantRanges = 0;
		CHECK_RESULT(vkCreatePipelineLayout(*create_info.logical_device, &pipeline_layout_create_info, 0, &out_shader->pipeline_layout));
	}

	/** Pipeline */
	{
		AS_LOG(LV_LOG, "Creating compute shader pipeline");
		VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info = {};
		pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		pipeline_shader_stage_create_info.module = out_shader->module;
		pipeline_shader_stage_create_info.pName = "main";
		VkComputePipelineCreateInfo compute_pipeline_create = {};
		compute_pipeline_create.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		compute_pipeline_create.stage = pipeline_shader_stage_create_info;
		compute_pipeline_create.layout = out_shader->pipeline_layout;
		CHECK_RESULT(vkCreateComputePipelines(*create_info.logical_device, 0, 1, &compute_pipeline_create, 0, &out_shader->pipeline));
	}

	/** Descriptor pool */
	{
		AS_LOG(LV_LOG, "Creating descriptor pool");
		VkDescriptorPoolSize descriptor_pool_size = {};
		descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptor_pool_size.descriptorCount = 2;
		VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
		descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_create_info.maxSets = 1;
		descriptor_pool_create_info.poolSizeCount = 1;
		descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;
		vkCreateDescriptorPool(*create_info.logical_device, &descriptor_pool_create_info, 0, &out_shader->descriptor_pool);
	}

	/** Allocate descriptor sets */
	{
		AS_LOG(LV_LOG, "Allocate descriptor sets");
		VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
		descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptor_set_allocate_info.descriptorPool = out_shader->descriptor_pool;
		descriptor_set_allocate_info.descriptorSetCount = 1;
		descriptor_set_allocate_info.pSetLayouts = &out_shader->descriptor_set_layout;
		CHECK_RESULT(vkAllocateDescriptorSets(*create_info.logical_device, &descriptor_set_allocate_info, &out_shader->descriptor_set));
	}

	/** Update Descriptor Sets */
	{
		AS_LOG(LV_LOG, "Update Descriptor Sets");
		VkDescriptorBufferInfo in_descriptor_buffer_info = {};
		in_descriptor_buffer_info.buffer = *create_info.in_buffer;
		in_descriptor_buffer_info.range = VK_WHOLE_SIZE;
		VkDescriptorBufferInfo out_descriptor_buffer_info = {};
		out_descriptor_buffer_info.buffer = *create_info.out_buffer;
		out_descriptor_buffer_info.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet write_descriptor_set_0 = {};
		write_descriptor_set_0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_set_0.dstSet = out_shader->descriptor_set;
		write_descriptor_set_0.dstBinding = 0;
		write_descriptor_set_0.dstArrayElement = 0;
		write_descriptor_set_0.descriptorCount = 1;
		write_descriptor_set_0.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write_descriptor_set_0.pImageInfo = 0;
		write_descriptor_set_0.pBufferInfo = &in_descriptor_buffer_info;
		write_descriptor_set_0.pTexelBufferView = 0;

		VkWriteDescriptorSet write_descriptor_set_1 = {};
		write_descriptor_set_1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_set_1.dstSet = out_shader->descriptor_set;
		write_descriptor_set_1.dstBinding = 1;
		write_descriptor_set_0.dstArrayElement = 0;
		write_descriptor_set_1.descriptorCount = 1;
		write_descriptor_set_1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write_descriptor_set_1.pImageInfo = 0;
		write_descriptor_set_1.pBufferInfo = &out_descriptor_buffer_info;
		write_descriptor_set_1.pTexelBufferView = 0;

		VkWriteDescriptorSet write_descriptor_set[2] =
		{
			write_descriptor_set_0,
			write_descriptor_set_1
		};
		
		vkUpdateDescriptorSets(*create_info.logical_device, 2, write_descriptor_set, 0, 0);
	}

	AS_LOG(LV_LOG, "Done creating the shader");
	return VK_SUCCESS;
}

VkResult as::start_shader(vulkan_shader* in_shader, vulkan_device* in_device, const u32& buffer_size)
{
	assert(in_shader);
	assert(in_device);

	vkCmdBindPipeline(in_device->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, in_shader->pipeline);
	vkCmdBindDescriptorSets(in_device->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
							in_shader->pipeline_layout, 0, 1, &in_shader->descriptor_set, 0, 0);
	vkCmdDispatch(in_device->command_buffer, buffer_size / sizeof(i32), 1, 1);
	CHECK_RESULT(vkEndCommandBuffer(in_device->command_buffer));

	CHECK_RESULT(as::get_device_queue(&in_shader->queue, in_device));
	CHECK_RESULT(as::submit_queue(&in_shader->queue, &in_device->command_buffer));

	return VK_SUCCESS;
}

void as::destroy_shader(VkDevice* in_device, vulkan_shader* in_shader)
{
	assert(in_shader);
	assert(in_device);

	vkDestroyPipelineLayout(*in_device, in_shader->pipeline_layout, nullptr);
	vkDestroyPipeline(*in_device, in_shader->pipeline, nullptr);
	vkDestroyDescriptorSetLayout(*in_device, in_shader->descriptor_set_layout, nullptr);
	vkDestroyDescriptorPool(*in_device, in_shader->descriptor_pool, nullptr);
	vkDestroyShaderModule(*in_device, in_shader->module, nullptr);
}

VkResult as::get_depth_format(VkFormat* out_Format, VkPhysicalDevice* physical_device)
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

VkResult as::create_depth_stencil(vulkan_device*& device, const u32& width, const u32& height)
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

VkResult as::create_fences(vulkan_device*& device)
{
	assert(device);
	if (device->type & vulkan_device_type::COMPUTE)
	{
		CHECK_RESULT(create_fence(&device->compute_fence, &device->logical));
	}
	if (device->type & vulkan_device_type::GRAPHICS)
	{
		CHECK_RESULT(create_fence(&device->presentation_fence, &device->logical));
	}
	return VK_SUCCESS;
}

VkResult as::create_fence(VkFence* out_fence, VkDevice* logical_device)
{
	assert(logical_device);
	assert(out_fence);

	VkFenceCreateInfo fence_create_info = {};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	CHECK_RESULT(vkCreateFence(*logical_device, &fence_create_info, nullptr, out_fence));
	return VK_SUCCESS;
}

VkResult as::submit_queue(VkQueue* in_queue, VkCommandBuffer* in_command_buffer)
{
	AS_LOG(LV_LOG, "Submitting queue");
	assert(in_queue);
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pCommandBuffers = in_command_buffer;
	submit_info.commandBufferCount = 1;
	// TODO: semaphores

	CHECK_RESULT(vkQueueSubmit(*in_queue, 1, &submit_info, 0));
	CHECK_RESULT(vkQueueWaitIdle(*in_queue));
	return VK_SUCCESS;
}

VkResult as::get_best_transfer_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index)
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

VkResult as::get_best_compute_queue(const VkPhysicalDevice &physical_device, u32* queue_family_index)
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

VkResult as::get_device_queue(VkQueue* out_queue, vulkan_device* in_vulkan_device)
{
	assert(out_queue);
	assert(&in_vulkan_device->logical);
	vkGetDeviceQueue(in_vulkan_device->logical, in_vulkan_device->queue_family_index, 0, out_queue);
	return VK_SUCCESS;
}

bool as::check_validation_layer_support()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};
	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

char** as::get_required_extensions()
{
	u32 glfw_extension_count = 0;
	const char** glfw_extensions;
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	if (glfw_extensions)
	{
		//char** extensions = new char* [glfw_extension_count + 1];
		//memcpy(extensions, glfw_extensions, sizeof(glfw_extensions));
		
		//{ glfw_extensions, glfw_extensions + glfw_extension_count }

		//if (enableValidationLayers) 
		//{
		//	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		//}
		//return extensions;
	}
	
	return nullptr;
}

void as::compute_test()
{
	// VULKAN INIT
	as::vulkan_interface vk_interface;
	as::vulkan_interface_create_info vulkan_interface_create_info = {};
	vulkan_interface_create_info.debug = true;
	vulkan_interface_create_info.is_compute = true;
	CHECK_RESULT(as::init_vulkan(&vk_interface, vulkan_interface_create_info));

	// FILLING VRAM
	const i32 buffer_length = 16384;
	const u32 buffer_size = sizeof(i32) * buffer_length;
	const u64 memory_size = buffer_size * 2;
	as::vulkan_memory memory;
	as::vulkan_memory_create_info vulkan_memory_create_info = {};
	vulkan_memory_create_info.buffer_size = memory_size;
	vulkan_memory_create_info.device = &vk_interface.devices[0];
	CHECK_RESULT(as::allocate_memory(&memory, vulkan_memory_create_info));
	CHECK_RESULT(as::edit_memory_payload(&memory, [&](i32* payload)
		{
			for (u32 i = 1; i < memory_size / sizeof(i32); i++)
			{
				payload[i] = 1;
			}
		}));

	// BUFFERS
	VkBuffer in_buffer;
	CHECK_RESULT(as::create_buffer(&in_buffer, &memory, vk_interface.devices[0].queue_family_index));
	VkBuffer out_buffer;
	CHECK_RESULT(as::create_buffer(&out_buffer, &memory, vk_interface.devices[0].queue_family_index));
	//printf("%d", vk_interface.devices[0]);
	// SHADERS
	as::vulkan_shader shader;
	as::vulkan_shader_create_info shader_create_info = {};
	shader_create_info.logical_device = &vk_interface.devices[0].logical;
	shader_create_info.file_name = new char[]("main.comp");
	shader_create_info.source = new char[](
		"#version 310 es\n"
		"layout(local_size_x = 1, local_size_y = 16) in;\n"
		"layout (set=0, binding = 0) buffer in_buffer_struct {int data[16384];} in_buffer; \n"
		"layout (set=0, binding = 1) buffer out_buffer_struct {int data[16384];} out_buffer; \n"
		"void main() { out_buffer.data[gl_GlobalInvocationID.x]++; }\n"
		);
	shader_create_info.in_buffer = &in_buffer;
	shader_create_info.out_buffer = &out_buffer;
	CHECK_RESULT(as::create_shader(&shader, shader_create_info));
	CHECK_RESULT(as::start_shader(&shader, &vk_interface.devices[0], buffer_size));


	CHECK_RESULT(as::edit_memory_payload(&memory, [&](i32* payload)
		{
			char full_payload[500000] = "";
			for (u32 i = 1; i < memory_size / sizeof(i32); i++)
			{
				char current_payload[10];
				sprintf(current_payload, "%d", payload[i]);
				strcat(full_payload, current_payload);
				strcat(full_payload, "|");
			}
			AS_LOG(LV_LOG, "Display the full payload after executing the shader: ");
			printf("%s\n", full_payload);
		}));

	as::destroy_buffer(&vk_interface.devices[0].logical, &in_buffer);
	as::destroy_buffer(&vk_interface.devices[0].logical, &out_buffer);
	as::destroy_device_memory(&vk_interface.devices[0].logical, &memory.device_memory);
	as::destroy_shader(&vk_interface.devices[0].logical, &shader);
	as::destroy_vulkan(&vk_interface);
}

VkResult as::create_vulkan_instance(VkInstance& out_instance, const vulkan_instance_create_info& instance_create_info)
{
	if (instance_create_info.enable_validation_layers && !check_validation_layer_support())
	{
		AS_LOG(LV_ERROR, "Validation layers requested, but not available");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = get_required_extensions(instance_create_info.enable_validation_layers);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (instance_create_info.enable_validation_layers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(instance_create_info.validation_layers.size());
		createInfo.ppEnabledLayerNames = instance_create_info.validation_layers.data();
		populate_debug_messenger_create_info(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}
	return vkCreateInstance(&createInfo, nullptr, &out_instance);
}

VkResult as::create_surface(VkSurfaceKHR& out_surface, const VkInstance& instance, GLFWwindow* window)
{
	return glfwCreateWindowSurface(instance, window, nullptr, &out_surface);
}

as::QueueFamilyIndices as::find_queue_families(VkPhysicalDevice& physical_device, VkSurfaceKHR* surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) 
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, *surface, &presentSupport);

		if (presentSupport) 
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

bool as::check_device_extension_support(VkPhysicalDevice& physical_device, const std::vector<const char*> extensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

	for (const auto& extension : availableExtensions) 
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

as::SwapChainSupportDetails as::query_swap_chain_support(VkPhysicalDevice& physical_device, VkSurfaceKHR* surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, *surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool as::is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR* surface)
{
	QueueFamilyIndices indices = find_queue_families(physical_device, surface);

	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	bool extensionsSupported = check_device_extension_support(physical_device, deviceExtensions);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = query_swap_chain_support(physical_device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physical_device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

VkSampleCountFlagBits as::get_max_usable_sample_count(VkPhysicalDevice* physical_device)
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(*physical_device, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

void as::pick_physical_device(VkPhysicalDevice* out_physical_device, VkSampleCountFlagBits* out_msaa_samples, VkInstance* instance, VkSurfaceKHR* surface)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		AS_LOG(LV_ERROR, "failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

	for (auto& device : devices)
	{
		if (is_device_suitable(device, surface))
		{
			*out_physical_device = device;
			*out_msaa_samples = get_max_usable_sample_count(&device);
			break;
		}
	}

	if (out_physical_device == VK_NULL_HANDLE)
	{
		AS_LOG(LV_ERROR, "failed to find a suitable GPU!");
	}
}

void as::create_logical_device(VkDevice* out_logical_device, VkQueue* out_graphics_queue, VkQueue* out_present_queue, VkPhysicalDevice* physical_device, VkSurfaceKHR* surface, const std::vector<const char*> extensions, const std::vector<const char*> validation_layers)
{
	QueueFamilyIndices indices = find_queue_families(*physical_device, surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (validation_layers.size() > 0)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		createInfo.ppEnabledLayerNames = validation_layers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(*physical_device, &createInfo, nullptr, out_logical_device) != VK_SUCCESS)
	{
		AS_LOG(LV_ERROR, "failed to create logical device!");
	}

	vkGetDeviceQueue(*out_logical_device, indices.graphicsFamily.value(), 0, out_graphics_queue);
	vkGetDeviceQueue(*out_logical_device, indices.presentFamily.value(), 0, out_present_queue);
}

VkSurfaceFormatKHR as::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR as::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D as::choos_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void as::create_swap_chain(VkSwapchainKHR* out_swap_chain, std::vector<VkImage>* out_swap_chain_images, VkFormat* out_swap_chain_image_format, VkExtent2D* out_swap_chain_extent, VkDevice* logical_device, VkPhysicalDevice* physical_device, VkSurfaceKHR* surface, GLFWwindow* window)
{
	SwapChainSupportDetails swapChainSupport = query_swap_chain_support(*physical_device, surface);

	VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.formats);
	VkPresentModeKHR presentMode = choose_swap_present_mode(swapChainSupport.presentModes);
	VkExtent2D extent = choos_swap_extent(swapChainSupport.capabilities, window);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = *surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = find_queue_families(*physical_device, surface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(*logical_device, &createInfo, nullptr, out_swap_chain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(*logical_device, *out_swap_chain, &imageCount, nullptr);
	out_swap_chain_images->resize(imageCount);
	vkGetSwapchainImagesKHR(*logical_device, *out_swap_chain, &imageCount, out_swap_chain_images->data());

	*out_swap_chain_image_format = surfaceFormat.format;
	*out_swap_chain_extent = extent;
}

VkImageView as::create_image_view(VkDevice* logical_device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(*logical_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		AS_LOG(LV_ERROR, "failed to create texture image view!");
	}

	return imageView;
}

void as::create_image_views(std::vector<VkImageView>* swap_chain_image_views, std::vector<VkFramebuffer>* swap_chain_framebuffers, std::vector<VkImage>* swap_chain_images, VkFormat* swap_chain_image_format, VkDevice* logical_device)
{
	swap_chain_image_views->resize(swap_chain_images->size());
	for (uint32_t i = 0; i < swap_chain_images->size(); i++)
	{
		(*swap_chain_image_views)[i] = create_image_view(logical_device, (*swap_chain_images)[i], *swap_chain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

VkResult as::create_render_pass(VkFormat& swap_chain_image_format, VkSampleCountFlagBits& msaa_samples, VkRenderPass& render_pass, VkDevice& logical_device, VkPhysicalDevice& physical_device)
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swap_chain_image_format;
	colorAttachment.samples = msaa_samples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = find_depth_format(physical_device);
	depthAttachment.samples = msaa_samples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = swap_chain_image_format;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	return vkCreateRenderPass(logical_device, &renderPassInfo, nullptr, &render_pass);
}

VkFormat as::find_supported_format(VkPhysicalDevice& physical_device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

VkFormat as::find_depth_format(VkPhysicalDevice& physical_device)
{
	return find_supported_format(physical_device,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkResult as::create_descriptor_set_layout(VkDevice& logical_device, VkDescriptorSetLayout& out_descriptor_set_layout)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	return vkCreateDescriptorSetLayout(logical_device, &layoutInfo, nullptr, &out_descriptor_set_layout);
}

void as::create_graphics_pipeline(VkPipeline& out_graphics_pipeline, VkPipelineLayout& out_pipeline_layout, VkDevice& logical_device, VkSampleCountFlagBits& msaa_samples, VkDescriptorSetLayout& descriptor_set_layout, VkRenderPass& render_pass)
{
	// vertex shader
	shader_compile_info compile_info_vertex = {};
	compile_info_vertex.file_name = new char[]("main");
	compile_info_vertex.source = read_file("shaders/shader.vert");
	compile_info_vertex.kind = shaderc_glsl_vertex_shader;
	shader_binaries out_vertex_shader_bin;
	compile_shader(&out_vertex_shader_bin, compile_info_vertex);
	write_shader_bin("shaders/vert.spv", out_vertex_shader_bin);

	// fragment shader
	shader_compile_info compile_info_frag = {};
	compile_info_frag.file_name = new char[]("main");
	compile_info_frag.source = read_file("shaders/shader.frag");
	compile_info_frag.kind = shaderc_glsl_fragment_shader;
	as::shader_binaries out_frag_shader_bin;
	as::compile_shader(&out_frag_shader_bin, compile_info_frag);
	write_shader_bin("shaders/frag.spv", out_frag_shader_bin);

	auto vertShaderCode = read_file(std::string("shaders/vert.spv"));
	auto fragShaderCode = read_file(std::string("shaders/frag.spv"));

	VkShaderModule vertShaderModule = create_shader_module(vertShaderCode, logical_device);
	VkShaderModule fragShaderModule = create_shader_module(fragShaderCode, logical_device);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = msaa_samples;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptor_set_layout;

	if (vkCreatePipelineLayout(logical_device, &pipelineLayoutInfo, nullptr, &out_pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = out_pipeline_layout;
	pipelineInfo.renderPass = render_pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &out_graphics_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(logical_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(logical_device, vertShaderModule, nullptr);
}

VkShaderModule as::create_shader_module(const std::vector<char>& code, VkDevice& logical_device)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logical_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

VkResult as::create_command_pool(VkCommandPool& out_command_pool, VkPhysicalDevice& physical_device, const VkDevice& logical_device, VkSurfaceKHR& surface)
{
	QueueFamilyIndices queueFamilyIndices = find_queue_families(physical_device, &surface);
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	return vkCreateCommandPool(logical_device, &poolInfo, nullptr, &out_command_pool);
}

void as::create_image(VkPhysicalDevice& physical_device, VkDevice& logical_device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = numSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(logical_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(logical_device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = find_memory_type(physical_device, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(logical_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(logical_device, image, imageMemory, 0);
}

uint32_t as::find_memory_type(VkPhysicalDevice& physical_device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	AS_LOG(LV_ERROR, "failed to find suitable memory type!");
}

void as::create_color_resources(VkImageView& out_image_view, VkPhysicalDevice& physical_device, VkDevice& logical_device, VkImage& image, VkDeviceMemory& image_memory, VkFormat& swap_chain_image_format, VkExtent2D& swap_chain_extent, VkSampleCountFlagBits& msaa_samples)
{
	VkFormat colorFormat = swap_chain_image_format;
	create_image(physical_device, logical_device, swap_chain_extent.width, swap_chain_extent.height, 1, msaa_samples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, image_memory);
	out_image_view = create_image_view(&logical_device, image, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void as::create_depth_resources(VkImageView& out_image_view, VkPhysicalDevice& physical_device, VkDevice& logical_device, VkImage& image, VkDeviceMemory& image_memory, VkFormat& swap_chain_image_format, VkExtent2D& swap_chain_extent, VkSampleCountFlagBits& msaa_samples)
{
	VkFormat depthFormat = find_depth_format(physical_device);
	create_image(physical_device, logical_device, swap_chain_extent.width, swap_chain_extent.height, 1, msaa_samples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, image_memory);
	out_image_view = create_image_view(&logical_device, image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void as::create_frame_buffers(std::vector<VkFramebuffer>& out_swap_chain_framebuffers, VkDevice& logical_device, std::vector<VkImageView>& swap_chain_image_views, VkImageView& color_image_view, VkImageView& depth_image_view, VkRenderPass& render_pass, VkExtent2D& swap_chain_extent)
{
	out_swap_chain_framebuffers.resize(swap_chain_image_views.size());

	for (size_t i = 0; i < swap_chain_image_views.size(); i++) {
		std::array<VkImageView, 3> attachments =
		{
			color_image_view,
			depth_image_view,
			swap_chain_image_views[i]
		};
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = render_pass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swap_chain_extent.width;
		framebufferInfo.height = swap_chain_extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(logical_device, &framebufferInfo, nullptr, &out_swap_chain_framebuffers[i]) != VK_SUCCESS)
		{
			AS_LOG(LV_ERROR, "failed to create framebuffer!");
		}
	}
}

void as::create_texture_image(VkImage& out_texture_image, const char* texture_path, uint32_t& mip_levels, VkPhysicalDevice& physical_device, VkDevice& logical_device, VkCommandPool& command_pool, VkQueue& graphics_queue, VkDeviceMemory& texture_image_memory)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texture_path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	create_buffer(stagingBuffer, physical_device, logical_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, imageSize, stagingBufferMemory);

	void* data;
	vkMapMemory(logical_device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(logical_device, stagingBufferMemory);

	stbi_image_free(pixels);

	create_image(physical_device, logical_device, texWidth, texHeight, mip_levels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, out_texture_image, texture_image_memory);

	transition_image_layout(logical_device, command_pool, graphics_queue, out_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);
	copy_buffer_to_image(logical_device, command_pool, graphics_queue, stagingBuffer, out_texture_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

	vkDestroyBuffer(logical_device, stagingBuffer, nullptr);
	vkFreeMemory(logical_device, stagingBufferMemory, nullptr);

	generate_mipmaps(physical_device, logical_device, command_pool, graphics_queue, out_texture_image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mip_levels);
}

void as::create_buffer(VkBuffer& out_buffer, VkPhysicalDevice& physical_device, VkDevice& logical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& buffer_memory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(logical_device, &bufferInfo, nullptr, &out_buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(logical_device, out_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = find_memory_type(physical_device, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(logical_device, &allocInfo, nullptr, &buffer_memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(logical_device, out_buffer, buffer_memory, 0);
}

void as::transition_image_layout(VkDevice& logical_device, VkCommandPool& command_pool, VkQueue& graphics_queue, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mipLevels)
{
	VkCommandBuffer commandBuffer = begin_single_time_commands(logical_device, command_pool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	end_single_time_commands(logical_device, command_pool, commandBuffer, graphics_queue);
}

VkCommandBuffer as::begin_single_time_commands(VkDevice& logical_device, VkCommandPool& command_pool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(logical_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void as::end_single_time_commands(VkDevice& logical_device, VkCommandPool& command_pool, VkCommandBuffer command_buffer, VkQueue& graphics_queue)
{
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command_buffer;

	vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphics_queue);

	vkFreeCommandBuffers(logical_device, command_pool, 1, &command_buffer);
}

void as::copy_buffer_to_image(VkDevice& logical_device, VkCommandPool& command_pool, VkQueue& graphics_queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = begin_single_time_commands(logical_device, command_pool);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	end_single_time_commands(logical_device, command_pool, commandBuffer, graphics_queue);
}

void as::generate_mipmaps(VkPhysicalDevice& physical_device, VkDevice& logical_device, VkCommandPool& command_pool, VkQueue& queue, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mip_levels)
{
	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physical_device, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = begin_single_time_commands(logical_device, command_pool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mip_levels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	end_single_time_commands(logical_device, command_pool, commandBuffer, queue);
}

VkResult as::setup_debug_messenger(VkInstance* instance, VkDebugUtilsMessengerEXT* debug_messenger)
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populate_debug_messenger_create_info(createInfo);
	return create_debug_utils_messenger_EXT(*instance, &createInfo, nullptr, debug_messenger);
}

VkResult as::create_debug_utils_messenger_EXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL as::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void as::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

bool as::check_layers_support(const std::vector<const char*> layers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : layers) 
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) 
		{
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) 
		{
			return false;
		}
	}

	return true;
}

std::vector<const char*> as::get_required_extensions(const bool& enable_validation_layers)
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enable_validation_layers) 
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

std::vector<char> as::read_file(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

char* as::read_file(const char* filename)
{
	FILE* f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* string = (char*)malloc(fsize + 1);
	if (string)
	{
		fread(string, fsize, 1, f);
		fclose(f);
		string[fsize] = 0;
	}
	return string;
}

void as::write_file_str(const char* filename, const char* data)
{
	FILE* fptr = fopen(filename, "w");
	if (fptr == NULL)
	{
		printf("Error!");
		exit(1);
	}
	fprintf(fptr, "%s", data);
	fclose(fptr);
}

void as::write_shader_bin(const char* filename, const shader_binaries& shader_bin)
{
	FILE* fptr = fopen(filename, "wb");
	if (fptr)
	{
		fwrite(shader_bin.binaries, shader_bin.size, 1, fptr);
		fclose(fptr);
	}
}
