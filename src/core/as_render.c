#include "core/as_render.h"
#include "core/as_shader.h"
#include "core/as_shapes.h"
#include "as_memory.h"
#include "as_utility.h"
#include <time.h>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#if defined (_WIN32)
#include <vulkan/vulkan_win32.h>
#define VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(__linux__)
#include <vulkan/vulkan_xcb.h>
#define VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#define VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XCB_SURFACE_EXTENSION_NAME
#endif

#define AS_USE_VULKAN_VALIDATION_LAYER 1

typedef struct queue_family_indices 
{
	u32 graphics_family;
	u32 present_family;
} queue_family_indices;

const char* instance_extensions[] = 
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME,
#if AS_USE_VULKAN_VALIDATION_LAYER
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
};
const u32 instance_extensions_count = AS_ARRAY_SIZE(instance_extensions);
const char* device_extensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
const u32 device_extensions_count = AS_ARRAY_SIZE(device_extensions);

const char* validation_layers[] =
{
	"VK_LAYER_KHRONOS_validation"
};
const u32 validation_layers_count = AS_ARRAY_SIZE(validation_layers);

typedef struct swap_chain_support_details 
{
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR* formats;
	VkPresentModeKHR* present_modes;
	u32 formats_count;
	u32 present_modes_count;
} swap_chain_support_details;

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	AS_LOG(LV_LOG, pCallbackData->pMessage);
	return VK_FALSE;
}

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT* create_info)
{
	create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info->pfnUserCallback = debug_callback;
	create_info->pUserData = NULL;
}

VkResult create_debug_utils_messenger_EXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pMessenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != NULL) 
	{
		return func(instance, pCreateInfo, pAllocator, pMessenger);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
	return VK_ERROR_UNKNOWN;
}

void destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

VkVertexInputBindingDescription as_get_binding_description()
{
	VkVertexInputBindingDescription binding_description = {0};
	binding_description.binding = 0;
	binding_description.stride = sizeof(as_vertex);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_description;
}

void as_get_attribute_descriptions(VkVertexInputAttributeDescription* attribute_descriptions)
{
	if (!attribute_descriptions)
	{
		AS_LOG(LV_WARNING, "Cannot get attributes, nullptr");
		return;
	}

	attribute_descriptions[0].binding = 0;
	attribute_descriptions[0].location = 0;
	attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_descriptions[0].offset = offsetof(as_vertex, pos);

	attribute_descriptions[1].binding = 0;
	attribute_descriptions[1].location = 1;
	attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[1].offset = offsetof(as_vertex, color);
}

bool is_device_suitable(VkPhysicalDevice device) 
{
	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(device, &device_properties);
	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(device, &device_features);

	// Example criteria for device suitability
	const bool is_discrete_gpu = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	const bool has_geometry_shader = device_features.geometryShader;

	return is_discrete_gpu && has_geometry_shader;
}

queue_family_indices find_queue_families(const VkPhysicalDevice device, const VkSurfaceKHR surface) 
{
	queue_family_indices indices = { UINT32_MAX,  UINT32_MAX};

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

	VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)AS_MALLOC(queue_family_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

	for (u32 j = 0; j < queue_family_count; ++j) 
	{
		if (queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
		{
			indices.graphics_family = j;
		}

		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, j, surface, &present_support);

		if (present_support) 
		{
			indices.present_family = j;
		}

		if (indices.graphics_family != UINT32_MAX && indices.present_family != UINT32_MAX && indices.graphics_family != indices.present_family)
		{
			break;
		}
	}
	AS_FREE(queue_families);
	return indices;
}

void create_instance(as_render* render)
{
	VkApplicationInfo app_info = {0};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Abstract Shader Engine";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Abstract Shader Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledExtensionCount = instance_extensions_count;
	create_info.ppEnabledExtensionNames = instance_extensions;
	VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
	populate_debug_messenger_create_info(&debug_create_info);
	if (AS_USE_VULKAN_VALIDATION_LAYER)
	{
		create_info.enabledLayerCount = sizeof(validation_layers) / sizeof(validation_layers[0]);
		create_info.ppEnabledLayerNames = validation_layers;
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	}
	AS_ASSERT(vkCreateInstance(&create_info, NULL, &render->instance) == VK_SUCCESS, "Failed to create Vulkan instance");

	if (AS_USE_VULKAN_VALIDATION_LAYER)
	{
		const VkResult create_debug_utils_messenger_result = create_debug_utils_messenger_EXT(render->instance, &debug_create_info, NULL, &render->debug_messenger);
		AS_ASSERT(create_debug_utils_messenger_result == VK_SUCCESS, "Failed to set up Vulkan debug messenger");
	}
}

void pick_physical_device(as_render* render)
{
	u32 device_count = 0;
	vkEnumeratePhysicalDevices(render->instance, &device_count, NULL);
	AS_ASSERT(device_count > 0, "Failed to find GPUs with Vulkan support\n");

	VkPhysicalDevice* devices = (VkPhysicalDevice*)AS_MALLOC(device_count * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(render->instance, &device_count, devices);

	for (u32 i = 0; i < device_count; i++)
	{
		if (is_device_suitable(devices[i]))
		{
			render->physical_device = devices[i];
			break;
		}
	}
	AS_FREE(devices);
	AS_ASSERT(render->physical_device, "Failed to find a suitable GPU");
}

void create_surface(as_render* render, void* display_context) 
{
	const VkResult create_surface_result = glfwCreateWindowSurface(render->instance, display_context, NULL, &render->surface);
	AS_ASSERT(create_surface_result == VK_SUCCESS, "Unable to create a surface");
}

void create_logical_device(as_render* render)
{
	queue_family_indices indices = find_queue_families(render->physical_device, render->surface);

	VkDeviceQueueCreateInfo queue_create_infos[2] = {0};
	u32 unique_queue_families[2] = 
	{
		indices.graphics_family,
		indices.present_family
	};

	float queue_priority = 1.0f;
	for (u32 i = 0; i < 2; i++)
	{
		VkDeviceQueueCreateInfo queue_create_info = {0};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = unique_queue_families[i];
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos[i] = queue_create_info;
	}

	VkPhysicalDeviceFeatures device_features = {0};

	VkDeviceCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.queueCreateInfoCount = 2;
	create_info.pQueueCreateInfos = queue_create_infos;
	create_info.pEnabledFeatures = &device_features;
	create_info.enabledExtensionCount = device_extensions_count;
	create_info.ppEnabledExtensionNames = device_extensions;

	if (AS_USE_VULKAN_VALIDATION_LAYER) 
	{
		create_info.enabledLayerCount = validation_layers_count;
		create_info.ppEnabledLayerNames = validation_layers;
	}
	else 
	{
		create_info.enabledLayerCount = 0;
	}

	VkResult create_device_result = vkCreateDevice(render->physical_device, &create_info, NULL, &render->device);
	AS_ASSERT(create_device_result == VK_SUCCESS, "Unable to create device");
	vkGetDeviceQueue(render->device, indices.graphics_family, 0, &render->graphics_queue);
	vkGetDeviceQueue(render->device, indices.present_family, 0, &render->present_queue);
}

swap_chain_support_details query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	swap_chain_support_details details = {0};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	u32 format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, NULL);

	if (format_count != 0) 
	{
		details.formats_count = format_count;
		details.formats = (VkSurfaceFormatKHR*)AS_MALLOC(format_count * sizeof(VkSurfaceFormatKHR));
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats);
	}

	u32 present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL);

	if (present_mode_count != 0) 
	{
		details.present_modes_count = present_mode_count;
		details.present_modes = (VkPresentModeKHR*)AS_MALLOC(present_mode_count * sizeof(VkPresentModeKHR));
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes);
	}

	return details;
}

VkSurfaceFormatKHR choose_swap_surface_format(const VkSurfaceFormatKHR* formats, const u32 formats_count)
{
	for (u32 i = 0; i < formats_count; ++i) 
	{
		if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return formats[i];
		}
	}
	return formats[0];
}

VkPresentModeKHR choose_swap_present_mode(const VkPresentModeKHR* present_modes, u32 present_mode_count)
{
	for (u32 i = 0; i < present_mode_count; i++) 
	{
		if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
			return present_modes[i];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR* capabilities, void* display_context)
{
	if (capabilities->currentExtent.width != UINT32_MAX) 
	{
		return capabilities->currentExtent;
	}
	else 
	{
		i32 width, height;
		glfwGetFramebufferSize(display_context, &width, &height);

		VkExtent2D actual_extent = {
			.width = (u32)width,
			.height = (u32)height
		};

		actual_extent.width = AS_CLAMP(actual_extent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
		actual_extent.height = AS_CLAMP(actual_extent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);
		return actual_extent;
	}
	VkExtent2D empty_output = {0};
	return empty_output;
}

void create_swap_chain(as_render* render, void* display_context) // TODO : move swapchain to separate struct and pass it instead
{
	swap_chain_support_details swap_chain_support = query_swap_chain_support(render->physical_device, render->surface);

	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats, swap_chain_support.formats_count);
	VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.present_modes, swap_chain_support.present_modes_count);
	VkExtent2D extent = choose_swap_extent(&swap_chain_support.capabilities, display_context);

	render->swap_chain_images_count = swap_chain_support.capabilities.minImageCount + 1;
	if (swap_chain_support.capabilities.maxImageCount > 0 && 
		render->swap_chain_images_count > swap_chain_support.capabilities.maxImageCount)
	{
		render->swap_chain_images_count = swap_chain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = render->surface;

	create_info.minImageCount = render->swap_chain_images_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	queue_family_indices indices = find_queue_families(render->physical_device, render->surface);
	u32 queue_family_indices[] = { indices.graphics_family, indices.present_family};

	if (indices.graphics_family != indices.present_family) 
	{
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	}
	else 
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	create_info.preTransform = swap_chain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;

	VkResult create_swap_chain_result = vkCreateSwapchainKHR(render->device, &create_info, NULL, &render->swap_chain);
	AS_ASSERT(create_swap_chain_result == VK_SUCCESS, "Failed to create swap chain");

	vkGetSwapchainImagesKHR(render->device, render->swap_chain, &render->swap_chain_images_count, NULL);
	render->swap_chain_images = (VkImage*)AS_MALLOC(sizeof(VkImage) * render->swap_chain_images_count); // todo : unsure if it's needed
	vkGetSwapchainImagesKHR(render->device, render->swap_chain, &render->swap_chain_images_count, render->swap_chain_images);

	render->swap_chain_image_format = surface_format.format;
	render->swap_chain_extent = extent;
}

void create_image_views(as_render* render) 
{
	render->swap_chain_image_views = (VkImageView*)AS_MALLOC(render->swap_chain_images_count * sizeof(VkImageView));
	render->swap_chain_image_views_count = render->swap_chain_images_count;

	for (size_t i = 0; i < render->swap_chain_images_count; i++) {
		VkImageViewCreateInfo create_info = {0};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = render->swap_chain_images[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = render->swap_chain_image_format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		const VkResult create_image_view_result = vkCreateImageView(render->device, &create_info, NULL, &render->swap_chain_image_views[i]);
		AS_ASSERT(create_image_view_result == VK_SUCCESS, "Failed to create image views");
	}
}

void create_render_pass(as_render* render) 
{
	VkAttachmentDescription color_attachment = {0};
	color_attachment.format = render->swap_chain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {0};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {0};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	VkResult create_render_pass_result = vkCreateRenderPass(render->device, &render_pass_info, NULL, &render->render_pass);
	AS_ASSERT(create_render_pass_result == VK_SUCCESS, "Failed to create render pass");
}

void create_descriptor_set_layout(as_render* render) 
{
	VkDescriptorSetLayoutBinding ubo_layout_binding = {0};
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.pImmutableSamplers = NULL;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layout_info = {0};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 1;
	layout_info.pBindings = &ubo_layout_binding;

	VkResult create_descriptor_set_layout_result = vkCreateDescriptorSetLayout(render->device, &layout_info, NULL, &render->descriptor_set_layout);
	AS_ASSERT(create_descriptor_set_layout_result == VK_SUCCESS, "Failed to create descriptor set layout");
}


VkShaderModule create_shader_module(VkDevice device, as_shader_binary* shader_bin)
{
	VkShaderModuleCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = shader_bin->size;
	create_info.pCode = shader_bin->bin;

	VkShaderModule shader_module;
	VkResult result = vkCreateShaderModule(device, &create_info, NULL, &shader_module);
	AS_ASSERT(result == VK_SUCCESS, "Failed to create shader module");

	return shader_module;
}

void cleanup_shader_module(VkDevice device, VkShaderModule shader_module)
{
	vkDestroyShaderModule(device, shader_module, NULL);
}

void create_graphics_pipeline(as_render* render)
{
	// Load shader code
	as_shader_binary vert_shader_code = read_shader_code("../resources/shaders/default_vertex.glsl", AS_SHADER_TYPE_VERTEX);
	as_shader_binary frag_shader_code = read_shader_code("../resources/shaders/default_fragment.glsl", AS_SHADER_TYPE_FRAGMENT);

	// Create shader modules
	VkShaderModule vert_shader_module = create_shader_module(render->device, &vert_shader_code);
	VkShaderModule frag_shader_module = create_shader_module(render->device, &frag_shader_code);

	// Shader stage info
	VkPipelineShaderStageCreateInfo vert_shader_stage_info = { 0 };
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_shader_stage_info = { 0 };
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

	// Vertex input state
	VkPipelineVertexInputStateCreateInfo vertex_input_info = { 0 };
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkVertexInputBindingDescription binding_description = as_get_binding_description();
	VkVertexInputAttributeDescription* attribute_descriptions;
	attribute_descriptions = (VkVertexInputAttributeDescription*)AS_MALLOC(sizeof(VkVertexInputAttributeDescription) * AS_VERTEX_VAR_COUNT);
	as_get_attribute_descriptions(attribute_descriptions);

	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.vertexAttributeDescriptionCount = AS_VERTEX_VAR_COUNT;
	vertex_input_info.pVertexBindingDescriptions = &binding_description;
	vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions;

	// Input assembly state
	VkPipelineInputAssemblyStateCreateInfo input_assembly = { 0 };
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	// Viewport state
	VkPipelineViewportStateCreateInfo viewport_state = { 0 };
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	// Rasterization state
	VkPipelineRasterizationStateCreateInfo rasterizer = { 0 };
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// Multi-sample state
	VkPipelineMultisampleStateCreateInfo multisampling = { 0 };
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Color blend attachment state
	VkPipelineColorBlendAttachmentState color_blend_attachment = { 0 };
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	// Color blend state
	VkPipelineColorBlendStateCreateInfo color_blending = { 0 };
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	// Dynamic state
	VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic_state = { 0 };
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = 2;
	dynamic_state.pDynamicStates = dynamic_states;

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &render->descriptor_set_layout;

	VkResult create_pipeline_layout_result = vkCreatePipelineLayout(render->device, &pipeline_layout_info, NULL, &render->pipeline_layout);
	AS_ASSERT(create_pipeline_layout_result == VK_SUCCESS, "Failed to create pipeline layout");


	// Graphics pipeline
	VkGraphicsPipelineCreateInfo pipeline_info = { 0 };
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = render->pipeline_layout;
	pipeline_info.renderPass = render->render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	VkResult create_graphics_pipeline_result = vkCreateGraphicsPipelines(render->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &render->graphics_pipeline);
	AS_ASSERT(create_graphics_pipeline_result == VK_SUCCESS, "Failed to create graphics pipeline");

	vkDestroyShaderModule(render->device, frag_shader_module, NULL);
	vkDestroyShaderModule(render->device, vert_shader_module, NULL);

	AS_FREE(attribute_descriptions);
}

void create_framebuffers(as_render* render)
{
	render->swap_chain_framebuffers = (VkFramebuffer*)AS_MALLOC(render->swap_chain_image_views_count * sizeof(VkFramebuffer));
	render->swap_chain_framebuffers_count = render->swap_chain_image_views_count;

	for (size_t i = 0; i < render->swap_chain_image_views_count; i++) {
		VkImageView attachments[] = 
		{
			render->swap_chain_image_views[i]
		};

		VkFramebufferCreateInfo framebuffer_info = { 0 };
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = render->render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = render->swap_chain_extent.width;
		framebuffer_info.height = render->swap_chain_extent.height;
		framebuffer_info.layers = 1;

		VkResult create_graphics_pipeline_result = vkCreateFramebuffer(render->device, &framebuffer_info, NULL, &render->swap_chain_framebuffers[i]);
		AS_ASSERT(create_graphics_pipeline_result == VK_SUCCESS, "Failed to create framebuffer!");
	}
}

void create_command_pool(as_render* render) 
{
	queue_family_indices indices = find_queue_families(render->physical_device, render->surface);

	VkCommandPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = indices.graphics_family;

	AS_ASSERT(vkCreateCommandPool(render->device, &pool_info, NULL, &render->command_pool) == VK_SUCCESS, 
		"Failed to create graphics command pool!");
}

void create_buffer(as_render* render, VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* buffer_memory) 
{
	VkBufferCreateInfo buffer_info = { 0 };
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	AS_ASSERT(vkCreateBuffer(render->device, &buffer_info, NULL, buffer) == VK_SUCCESS,
		"Failed to create buffer!");

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(render->device, *buffer, &mem_requirements);

	VkMemoryAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = find_memory_type(render, mem_requirements.memoryTypeBits, properties);

	AS_ASSERT(vkAllocateMemory(render->device, &alloc_info, NULL, buffer_memory) == VK_SUCCESS,
		"Failed to allocate buffer memory!");

	vkBindBufferMemory(render->device, *buffer, *buffer_memory, 0);
}

void copy_buffer(as_render* render, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) 
{
	VkCommandBufferAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = render->command_pool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(render->device, &alloc_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(command_buffer, &begin_info);

	VkBufferCopy copy_region = { 0 };
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info = { 0 };
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	vkQueueSubmit(render->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(render->graphics_queue);

	vkFreeCommandBuffers(render->device, render->command_pool, 1, &command_buffer);
}

i32 find_memory_type(as_render* render, u32 type_filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties mem_properties;
	vkGetPhysicalDeviceMemoryProperties(render->physical_device, &mem_properties);

	for (i32 i = 0; i < mem_properties.memoryTypeCount; i++) 
	{
		if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) 
		{
			return i;
		}
	}

	AS_LOG(LV_ERROR, "Failed to find suitable memory type");
	return 0; 
}

void create_vertex_buffer(as_render* render) 
{
	const i32 test[] = {5, 6, 5};
	sizeof(test);
	VkDeviceSize buffer_size = sizeof(as_shape_quad_vertices[0]) * as_shape_quad_vertices_size;

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	create_buffer(render, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&staging_buffer, &staging_buffer_memory);

	void* data;
	vkMapMemory(render->device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, as_shape_quad_vertices, (size_t)buffer_size);
	vkUnmapMemory(render->device, staging_buffer_memory);

	create_buffer(render, buffer_size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &render->vertex_buffer, &render->vertex_buffer_memory);

	copy_buffer(render, staging_buffer, render->vertex_buffer, buffer_size);

	vkDestroyBuffer(render->device, staging_buffer, NULL);
	vkFreeMemory(render->device, staging_buffer_memory, NULL);
}

void create_index_buffer(as_render* render) 
{
	VkDeviceSize buffer_size = sizeof(as_shape_quad_indices[0]) * as_shape_quad_indices_size;

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	create_buffer(render, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&staging_buffer, &staging_buffer_memory);

	void* data;
	vkMapMemory(render->device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, as_shape_quad_indices, buffer_size);
	vkUnmapMemory(render->device, staging_buffer_memory);

	create_buffer(render, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &render->index_buffer, &render->index_buffer_memory);

	copy_buffer(render, staging_buffer, render->index_buffer, buffer_size);

	vkDestroyBuffer(render->device, staging_buffer, NULL);
	vkFreeMemory(render->device, staging_buffer_memory, NULL);
}

void create_uniform_buffers(as_render* render) 
{
	VkDeviceSize buffer_size = sizeof(as_uniform_buffer_object);

	render->uniform_buffers = (VkBuffer*)AS_MALLOC(MAX_FRAMES_IN_FLIGHT * sizeof(VkBuffer));
	render->uniform_buffers_memory = (VkDeviceMemory*)AS_MALLOC(MAX_FRAMES_IN_FLIGHT * sizeof(VkDeviceMemory));
	render->uniform_buffers_mapped = (void**)AS_MALLOC(MAX_FRAMES_IN_FLIGHT * sizeof(void*));
	render->uniform_buffers_count = MAX_FRAMES_IN_FLIGHT;
	render->uniform_buffers_memory_count = MAX_FRAMES_IN_FLIGHT;
	render->uniform_buffers_mapped_count = MAX_FRAMES_IN_FLIGHT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		create_buffer(render, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&render->uniform_buffers[i], &render->uniform_buffers_memory[i]);

		vkMapMemory(render->device, render->uniform_buffers_memory[i], 0, buffer_size, 0, &render->uniform_buffers_mapped[i]);
	}
}

void create_descriptor_pool(as_render* render) 
{
	VkDescriptorPoolSize pool_size = { 0 };
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = (u32)MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = &pool_size;
	pool_info.maxSets = (u32)MAX_FRAMES_IN_FLIGHT;

	AS_ASSERT(vkCreateDescriptorPool(render->device, &pool_info, NULL, &render->descriptor_pool) == VK_SUCCESS, 
		"Failed to create descriptor pool");
}

void create_descriptor_sets(as_render* render) 
{
	VkDescriptorSetLayout* layouts = (VkDescriptorSetLayout*)AS_MALLOC(MAX_FRAMES_IN_FLIGHT * sizeof(VkDescriptorSetLayout));
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		layouts[i] = render->descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = render->descriptor_pool;
	alloc_info.descriptorSetCount = (u32)MAX_FRAMES_IN_FLIGHT;
	alloc_info.pSetLayouts = layouts;

	render->descriptor_sets = (VkDescriptorSet*)AS_MALLOC(MAX_FRAMES_IN_FLIGHT * sizeof(VkDescriptorSet));
	AS_ASSERT(vkAllocateDescriptorSets(render->device, &alloc_info, render->descriptor_sets) == VK_SUCCESS, 
		"Failed to allocate descriptor sets!");

	AS_FREE(layouts);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		VkDescriptorBufferInfo buffer_info = { 0 };
		buffer_info.buffer = render->uniform_buffers[i];
		buffer_info.offset = 0;
		buffer_info.range = sizeof(as_uniform_buffer_object);

		VkWriteDescriptorSet descriptor_write = { 0 };
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = render->descriptor_sets[i];
		descriptor_write.dstBinding = 0;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pBufferInfo = &buffer_info;

		vkUpdateDescriptorSets(render->device, 1, &descriptor_write, 0, NULL);
	}
}

void create_command_buffers(as_render* render) 
{
	render->command_buffers = (VkCommandBuffer*)AS_MALLOC(MAX_FRAMES_IN_FLIGHT * sizeof(VkCommandBuffer));
	render->command_buffers_count = MAX_FRAMES_IN_FLIGHT;

	VkCommandBufferAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = render->command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

	AS_ASSERT(vkAllocateCommandBuffers(render->device, &alloc_info, render->command_buffers) == VK_SUCCESS, 
		"Failed to allocate command buffers");
}

void create_sync_objects(as_render* render) 
{
	render->image_available_semaphores = (VkSemaphore*)AS_MALLOC(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
	render->render_finished_semaphores = (VkSemaphore*)AS_MALLOC(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
	render->in_flight_fences = (VkFence*)AS_MALLOC(MAX_FRAMES_IN_FLIGHT * sizeof(VkFence));
	render->image_available_semaphores_count = MAX_FRAMES_IN_FLIGHT;
	render->render_finished_semaphores_count = MAX_FRAMES_IN_FLIGHT;
	render->in_flight_fences_count = MAX_FRAMES_IN_FLIGHT;

	VkSemaphoreCreateInfo semaphore_info = { 0 };
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		const VkResult image_available_semaphore_create_res = vkCreateSemaphore(render->device, &semaphore_info, NULL, &render->image_available_semaphores[i]);
		const VkResult render_finished_semaphore_create_res = vkCreateSemaphore(render->device, &semaphore_info, NULL, &render->render_finished_semaphores[i]);
		const VkResult in_flight_fence_create_res = vkCreateFence(render->device, &fence_info, NULL, &render->in_flight_fences[i]);
		AS_ASSERT(image_available_semaphore_create_res && render_finished_semaphore_create_res && in_flight_fence_create_res, "Failed to create synchronization objects for a frame!");
	}
}


void update_uniform_buffer(as_render* render, u32 current_image) 
{
	static clock_t start_time = 0;

	clock_t current_time = clock();
	const float time = ((float)(current_time - start_time)) / CLOCKS_PER_SEC;

	as_uniform_buffer_object ubo = { 0 };
	const as_mat4 identity = as_mat4_identity();
	const as_vec3 unit_z = as_vec3_unit_z();
	const f32 angle = as_radians(90.0f);
	ubo.model = as_rotate(&identity, time * angle, &unit_z);
	ubo.view = as_look_at(&(as_vec3) { 2.0f, 2.0f, 2.0f }, & (as_vec3) { 0.0f, 0.0f, 0.0f }, & (as_vec3) { 0.0f, 0.0f, 1.0f });
	ubo.proj = as_perspective(as_radians(45.0f), render->swap_chain_extent.width / (float)render->swap_chain_extent.height, 0.1f, 10.0f);
	ubo.proj.m[1][1] *= -1;

	memcpy(render->uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
}

void record_command_buffer(as_render* render, VkCommandBuffer command_buffer, u32 image_index)
{
	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkResult begin_command_buffer_result = vkBeginCommandBuffer(command_buffer, &begin_info);
	AS_ASSERT(begin_command_buffer_result == VK_SUCCESS, "Failed to begin recording command buffer!");

	VkRenderPassBeginInfo render_pass_info = { 0 };
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = render->render_pass;
	render_pass_info.framebuffer = render->swap_chain_framebuffers[image_index];
	render_pass_info.renderArea.offset.x = 0;
	render_pass_info.renderArea.offset.y = 0;
	render_pass_info.renderArea.extent = render->swap_chain_extent;

	VkClearValue clear_color = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	render_pass_info.clearValueCount = 1;
	render_pass_info.pClearValues = &clear_color;

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render->graphics_pipeline);

	VkViewport viewport = { 0 };
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)render->swap_chain_extent.width;
	viewport.height = (float)render->swap_chain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor = { 0 };
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = render->swap_chain_extent;
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	VkBuffer vertex_buffers[] = { render->vertex_buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

	vkCmdBindIndexBuffer(command_buffer, render->index_buffer, 0, VK_INDEX_TYPE_UINT16);

	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render->pipeline_layout, 0, 1, &render->descriptor_sets[render->current_frame], 0, NULL);

	vkCmdDrawIndexed(command_buffer, as_shape_quad_indices_size, 1, 0, 0, 0);

	vkCmdEndRenderPass(command_buffer);

	VkResult end_command_buffer_result = vkEndCommandBuffer(command_buffer);
	AS_ASSERT(end_command_buffer_result == VK_SUCCESS, "Failed to record command buffer!");
}

void cleanup_swap_chain(as_render* render)
{
	for (size_t i = 0; i < render->swap_chain_framebuffers_count; i++)
	{
		vkDestroyFramebuffer(render->device, render->swap_chain_framebuffers[i], NULL);
	}

	for (size_t i = 0; i < render->swap_chain_images_count; i++)
	{
		vkDestroyImageView(render->device, render->swap_chain_image_views[i], NULL);
	}

	vkDestroySwapchainKHR(render->device, render->swap_chain, NULL);
}

void recreate_swap_chain(as_render* render, void* display_context)
{
	i32 width = 0, height = 0;
	glfwGetFramebufferSize(display_context, &width, &height);

	while (width == 0 || height == 0) 
	{
		glfwGetFramebufferSize(display_context, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(render->device);
		
	cleanup_swap_chain(render);

	create_swap_chain(render, display_context);
	create_image_views(render);
	create_framebuffers(render);
}

void as_render_create(as_render* render, void* display_context)
{
	create_instance(render);
	create_surface(render, display_context);
	pick_physical_device(render);
	create_logical_device(render);
	create_swap_chain(render, display_context);
	create_image_views(render);
	create_render_pass(render);
	create_descriptor_set_layout(render);
	create_graphics_pipeline(render);
	create_framebuffers(render);
	create_command_pool(render);
	create_vertex_buffer(render);
	create_index_buffer(render);
	create_uniform_buffers(render);
	create_descriptor_pool(render);
	create_descriptor_sets(render);
	create_command_buffers(render);
	create_sync_objects(render);
}

void as_render_draw_frame(as_render* render, void* display_context)
{
	vkWaitForFences(render->device, 1, &render->in_flight_fences[render->current_frame], VK_TRUE, UINT64_MAX);

	u32 image_index;
	VkResult result = vkAcquireNextImageKHR(render->device, render->swap_chain, UINT64_MAX, render->image_available_semaphores[render->current_frame], VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) 
	{
		recreate_swap_chain(render, display_context);
		return;
	}
	AS_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire swap chain image!");

	update_uniform_buffer(render, render->current_frame);

	vkResetFences(render->device, 1, &render->in_flight_fences[render->current_frame]);

	vkResetCommandBuffer(render->command_buffers[render->current_frame], 0);
	record_command_buffer(render, render->command_buffers[render->current_frame], image_index);

	VkSubmitInfo submit_info = { 0 };
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { render->image_available_semaphores[render->current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &render->command_buffers[render->current_frame];

	VkSemaphore signal_semaphores[] = { render->render_finished_semaphores[render->current_frame] };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	AS_ASSERT(vkQueueSubmit(render->graphics_queue, 1, &submit_info, render->in_flight_fences[render->current_frame]) == VK_SUCCESS, "Failed to submit draw command buffer!");

	VkPresentInfoKHR present_info = { 0 };
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swap_chains[] = { render->swap_chain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;

	present_info.pImageIndices = &image_index;

	result = vkQueuePresentKHR(render->present_queue, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || render->framebuffer_resized) 
	{
		render->framebuffer_resized = false;
		recreate_swap_chain(render, display_context);
	}
	else if (result != VK_SUCCESS) {
		AS_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image!");
	}

	render->current_frame = (render->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void as_render_destroy(as_render* render)
{
	cleanup_swap_chain(render);

	vkDestroyPipeline(render->device, render->graphics_pipeline, NULL);
	vkDestroyPipelineLayout(render->device, render->pipeline_layout, NULL);
	vkDestroyRenderPass(render->device, render->render_pass, NULL);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(render->device, render->uniform_buffers[i], NULL);
		vkFreeMemory(render->device, render->uniform_buffers_memory[i], NULL);
	}

	vkDestroyDescriptorPool(render->device, render->descriptor_pool, NULL);

	vkDestroyDescriptorSetLayout(render->device, render->descriptor_set_layout, NULL);

	vkDestroyBuffer(render->device, render->index_buffer, NULL);
	vkFreeMemory(render->device, render->index_buffer_memory, NULL);

	vkDestroyBuffer(render->device, render->vertex_buffer, NULL);
	vkFreeMemory(render->device, render->vertex_buffer_memory, NULL);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(render->device, render->render_finished_semaphores[i], NULL);
		vkDestroySemaphore(render->device, render->image_available_semaphores[i], NULL);
		vkDestroyFence(render->device, render->in_flight_fences[i], NULL);
	}

	vkDestroyCommandPool(render->device, render->command_pool, NULL);

	vkDestroyDevice(render->device, NULL);

	if (AS_USE_VULKAN_VALIDATION_LAYER)
	{
		destroy_debug_utils_messenger_EXT(render->instance, render->debug_messenger, NULL);
	}

	vkDestroySurfaceKHR(render->instance, render->surface, NULL);
	vkDestroyInstance(render->instance, NULL);
}

as_object* as_add_object(as_render* render, const as_transform* transform, const char* vertex_shader_path, const char* fragment_shader_path)
{
	AS_LOG(LV_WARNING, "Not as_add_object is implemented");
	return NULL;
}

void as_delete_object(as_render* render, as_object* object)
{
	AS_LOG(LV_WARNING, "Not as_delete_object is implemented");
}
