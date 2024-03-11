// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_render.h"
#include "core/as_shader.h"
#include "as_memory.h"
#include "as_threads.h"

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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define AS_USE_VULKAN_VALIDATION_LAYER 1


static clock_t start_time = 0;

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
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	// VK_KHR_MAINTENANCE1_EXTENSION_NAME
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
	VkSurfaceFormatKHR32 formats;
	VkPresentModeKHR32 present_modes;
	//u32 formats_count;
	//u32 present_modes_count;
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
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // VK_VERTEX_INPUT_RATE_INSTANCE
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
	attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[0].offset = offsetof(as_vertex, position);

	attribute_descriptions[1].binding = 0;
	attribute_descriptions[1].location = 1;
	attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[1].offset = offsetof(as_vertex, normal);

	attribute_descriptions[2].binding = 0;
	attribute_descriptions[2].location = 2;
	attribute_descriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[2].offset = offsetof(as_vertex, color);

	attribute_descriptions[3].binding = 0;
	attribute_descriptions[3].location = 3;
	attribute_descriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_descriptions[3].offset = offsetof(as_vertex, tex_coord);
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

VkFormat find_supported_format(as_render* render, const VkFormat* candidates, sz candidate_count, VkImageTiling tiling, VkFormatFeatureFlags features) 
{
	for (sz i = 0; i < candidate_count; ++i) 
	{
		VkFormat format = candidates[i];
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(render->physical_device, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
		{
			return format;
		}
	}

	AS_LOG(LV_ERROR, "Failed to find supported format");
	return VK_FORMAT_UNDEFINED;
}

VkFormat find_depth_format(as_render* render) 
{
	VkFormat depth_formats[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	return find_supported_format(render, depth_formats, AS_ARRAY_SIZE(depth_formats), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool has_stencil_component(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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

	f32 queue_priority = 1.0f;
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
	device_features.samplerAnisotropy = VK_TRUE;

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
		details.formats.size = format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data);
	}

	u32 present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL);
	if (present_mode_count != 0) 
	{
		details.present_modes.size = present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data);
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

		VkExtent2D actual_extent = 
		{
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

	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats.data, swap_chain_support.formats.size);
	VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.present_modes.data, swap_chain_support.present_modes.size);
	VkExtent2D extent = choose_swap_extent(&swap_chain_support.capabilities, display_context);

	render->swap_chain_images.size = swap_chain_support.capabilities.minImageCount + 1;
	if (swap_chain_support.capabilities.maxImageCount > 0 && 
		render->swap_chain_images.size > swap_chain_support.capabilities.maxImageCount)
	{
		render->swap_chain_images.size = swap_chain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = render->surface;

	create_info.minImageCount = render->swap_chain_images.size;
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

	vkGetSwapchainImagesKHR(render->device, render->swap_chain, &(u32)render->swap_chain_images.size, NULL);
	vkGetSwapchainImagesKHR(render->device, render->swap_chain, &(u32)render->swap_chain_images.size, render->swap_chain_images.data);

	render->swap_chain_image_format = surface_format.format;
	render->swap_chain_extent = extent;
}

VkImageView create_image_view(as_render* render, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags)
{
	VkImageViewCreateInfo view_info = { 0 };
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = format;
	view_info.subresourceRange.aspectMask = aspect_flags;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	VkImageView image_view;
	const VkResult create_image_view_result = vkCreateImageView(render->device, &view_info, NULL, &image_view);
	AS_ASSERT(create_image_view_result == VK_SUCCESS, "Failed to create texture image view!");

	return image_view;
}

void create_image_views(as_render* render) 
{
	render->swap_chain_image_views.size = render->swap_chain_images.size;

	for (sz i = 0; i < render->swap_chain_images.size; i++)
	{
		render->swap_chain_image_views.data[i] = create_image_view(render, render->swap_chain_images.data[i], render->swap_chain_image_format, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void create_render_pass(as_render* render) 
{
	VkAttachmentDescription color_attachment = { 0 };
	color_attachment.format = render->swap_chain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depth_attachment = { 0 };
	depth_attachment.format = find_depth_format(render);
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_attachment_ref = { 0 };
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = { 0 };
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = { 0 };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	VkSubpassDependency dependency = { 0 };
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[] = { color_attachment, depth_attachment };
	VkRenderPassCreateInfo render_pass_info = { 0 };
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = AS_ARRAY_SIZE(attachments);
	render_pass_info.pAttachments = attachments;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	AS_ASSERT(vkCreateRenderPass(render->device, &render_pass_info, NULL, &render->render_pass) == VK_SUCCESS,
		"Failed to create render pass");
}

void as_shader_set_locked(const u64 frame_count, as_shader* shader)
{
	shader->refresh_frame = frame_count;
	AS_WAIT_AND_LOCK(shader);
}

void as_shader_set_unlocked(as_shader* shader)
{
	AS_UNLOCK(shader);
}

bool as_shader_is_unlocked(const u64 frame_count, as_shader* shader)
{
	return shader->refresh_frame + 10 < frame_count && AS_IS_UNLOCKED(shader);
}

void as_shader_create_descriptor_set_layout(as_shader* shader) 
{
	AS_ASSERT(&shader->uniforms, "Cannot create_descriptor_set_layout_from_uniforms, NULL uniforms");

	const sz bindings_count = (shader->uniforms.size + 1); // ubo + uniforms
	VkDescriptorSetLayoutBinding* bindings = AS_MALLOC(sizeof(VkDescriptorSetLayoutBinding) * bindings_count);

	VkDescriptorSetLayoutBinding ubo_layout_binding = { 0 };
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.pImmutableSamplers = NULL;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[ubo_layout_binding.binding] = ubo_layout_binding;

	for (sz i = 0 ; i < shader->uniforms.size ; i++)
	{ 
		as_shader_uniform* uniform = AS_ARRAY_GET(shader->uniforms, i);
		VkDescriptorSetLayoutBinding uniform_layout_binding = { 0 };
		uniform_layout_binding.binding = i + 1; // ubo is 0, so + 1
		uniform_layout_binding.descriptorCount = 1;
		uniform_layout_binding.descriptorType = uniform->type;
		uniform_layout_binding.pImmutableSamplers = NULL;
		uniform_layout_binding.stageFlags = uniform->stage;

		bindings[uniform_layout_binding.binding] = uniform_layout_binding;
	}

	VkDescriptorSetLayoutCreateInfo layout_info = { 0 };
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = bindings_count;
	layout_info.pBindings = bindings;

	AS_ASSERT(vkCreateDescriptorSetLayout(*shader->device, &layout_info, NULL, &shader->descriptor_set_layout) == VK_SUCCESS,
		"Failed to create descriptor set layout!");
	AS_FREE(bindings);
}

VkShaderModule create_shader_module(VkDevice device, as_shader_binary* shader_bin)
{
	VkShaderModuleCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = shader_bin->binaries_size;
	create_info.pCode = shader_bin->binaries;

	VkShaderModule shader_module;
	VkResult result = vkCreateShaderModule(device, &create_info, NULL, &shader_module);
	AS_ASSERT(result == VK_SUCCESS, "Failed to create shader module");

	return shader_module;
}

void cleanup_shader_module(VkDevice device, VkShaderModule shader_module)
{
	vkDestroyShaderModule(device, shader_module, NULL);
}

void as_shader_create_graphics_pipeline_layout(as_render* render, VkPipelineLayout* pipeline_layout, VkDescriptorSetLayout* descriptor_set_layout)
{
	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(render->physical_device, &device_properties);
	AS_ASSERT(device_properties.limits.maxPushConstantsSize >= sizeof(as_push_const_buffer),
		"Cannot create graphics pipeline layout, invalid size of push const buffer");

	VkPushConstantRange push_constant_range_vert = { 0 };
	push_constant_range_vert.offset = 0;
	push_constant_range_vert.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	push_constant_range_vert.size = sizeof(as_push_const_buffer);

	VkPushConstantRange push_constant_range_frag = { 0 };
	push_constant_range_frag.offset = 0;
	push_constant_range_frag.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_range_frag.size = sizeof(as_push_const_buffer);

	VkPushConstantRange ranges[] = {push_constant_range_vert, push_constant_range_frag};

	VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = descriptor_set_layout;
	pipeline_layout_info.pPushConstantRanges = ranges;
	pipeline_layout_info.pushConstantRangeCount = 2;

	VkResult create_pipeline_layout_result = vkCreatePipelineLayout(render->device, &pipeline_layout_info, NULL, pipeline_layout);
	AS_ASSERT(create_pipeline_layout_result == VK_SUCCESS, "Failed to create pipeline layout");
}

void create_framebuffers(as_render* render)
{
	render->swap_chain_framebuffers.size = render->swap_chain_image_views.size;

	for (sz i = 0; i < render->swap_chain_image_views.size; i++) {
		VkImageView attachments[] = 
		{
			render->swap_chain_image_views.data[i],
			render->depth_image_view
		};

		VkFramebufferCreateInfo framebuffer_info = { 0 };
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = render->render_pass;
		framebuffer_info.attachmentCount = AS_ARRAY_SIZE(attachments);
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = render->swap_chain_extent.width;
		framebuffer_info.height = render->swap_chain_extent.height;
		framebuffer_info.layers = 1;

		const VkResult create_result = vkCreateFramebuffer(render->device, &framebuffer_info, NULL, &render->swap_chain_framebuffers.data[i]);
		AS_ASSERT(create_result == VK_SUCCESS, "Failed to create framebuffer!");
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

VkCommandBuffer begin_single_time_commands(as_render* render) 
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

	return command_buffer;
}

void end_single_time_commands(as_render* render, VkCommandBuffer command_buffer) 
{
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info = { 0 };
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	vkQueueSubmit(render->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(render->graphics_queue);

	vkFreeCommandBuffers(render->device, render->command_pool, 1, &command_buffer);
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

void copy_buffer_to_image(as_render* render, VkBuffer buffer, VkImage image, u32 width, u32 height) 
{
	VkCommandBuffer command_buffer = begin_single_time_commands(render);

	VkBufferImageCopy region = { 0 };
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = (VkOffset3D){ 0, 0, 0 };
	region.imageExtent = (VkExtent3D)
	{
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	end_single_time_commands(render, command_buffer);
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

void create_uniform_buffers_direct(as_uniform_buffers* uniform_buffers, as_render* render)
{
	VkDeviceSize buffer_size = sizeof(as_uniform_buffer_object);

	uniform_buffers->buffers.size = MAX_FRAMES_IN_FLIGHT;
	uniform_buffers->memories.size = MAX_FRAMES_IN_FLIGHT;
	uniform_buffers->buffers_mapped.size = MAX_FRAMES_IN_FLIGHT;

	for (sz i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		create_buffer(render, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&uniform_buffers->buffers.data[i], &uniform_buffers->memories.data[i]);

		vkMapMemory(render->device, uniform_buffers->memories.data[i], 0, buffer_size, 0, &uniform_buffers->buffers_mapped.data[i]);
	}
}

void create_descriptor_pool(VkDevice device, VkDescriptorPool* descriptor_pool) 
{
	VkDescriptorPoolSize pool_size = { 0 };
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = (u32)MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = &pool_size;
	pool_info.maxSets = (u32)MAX_FRAMES_IN_FLIGHT;

	AS_ASSERT(vkCreateDescriptorPool(device, &pool_info, NULL, descriptor_pool) == VK_SUCCESS, 
		"Failed to create descriptor pool");
}

void create_descriptor_sets_from_shader(VkDevice device, as_shader* shader)
{
	VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		layouts[i] = shader->descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = shader->descriptor_pool;
	alloc_info.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
	alloc_info.pSetLayouts = layouts;

	AS_ASSERT(vkAllocateDescriptorSets(device, &alloc_info, shader->descriptor_sets.data) == VK_SUCCESS, "Failed to allocate descriptor sets!");
	shader->descriptor_sets.size = MAX_FRAMES_IN_FLIGHT;

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		as_uniform_buffers* uniform_buffers = &shader->uniform_buffers;
		AS_ASSERT(uniform_buffers, TEXT("Cannot create descriptor sets, invalid uniform_buffers"));

		VkDescriptorBufferInfo buffer_info = { 0 };
		buffer_info.buffer = uniform_buffers->buffers.data[i];
		buffer_info.offset = 0;
		buffer_info.range = sizeof(as_uniform_buffer_object);

		const sz descriptor_writes_count = shader->uniforms.size + 1; // ubo + uniforms
		VkWriteDescriptorSet* descriptor_writes = (VkWriteDescriptorSet*)AS_MALLOC(sizeof(VkWriteDescriptorSet) * descriptor_writes_count);

		descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[0].dstSet = shader->descriptor_sets.data[i];
		descriptor_writes[0].dstBinding = 0;
		descriptor_writes[0].dstArrayElement = 0;
		descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_writes[0].descriptorCount = 1;
		descriptor_writes[0].pBufferInfo = &buffer_info;

		for (sz j = 0 ; j < shader->uniforms.size ; j++)
		{
			as_shader_uniform* uniform = AS_ARRAY_GET(shader->uniforms, j);
			if (!uniform)
			{
				continue;
			}
			
			const sz descriptor_writes_index = j + 1;
			
			descriptor_writes[descriptor_writes_index].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[descriptor_writes_index].dstSet = shader->descriptor_sets.data[i];
			descriptor_writes[descriptor_writes_index].dstBinding = descriptor_writes_index;
			descriptor_writes[descriptor_writes_index].dstArrayElement = 0;
			descriptor_writes[descriptor_writes_index].descriptorType = uniform->type;
			descriptor_writes[descriptor_writes_index].descriptorCount = 1;
			if (uniform->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				as_texture* texture = (as_texture*)uniform->data;
				if (texture)
				{
					VkDescriptorImageInfo image_info = { 0 };
					image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					image_info.imageView = texture->image_view;
					image_info.sampler = texture->sampler;
					descriptor_writes[descriptor_writes_index].pImageInfo = &image_info;
				}
			}
			// else if (uniform->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
			// {
			// 	// currently used for scene only
			// 	as_scene_gpu_buffer* scene_gpu = (as_scene_gpu_buffer*)uniform->data;
			// 	if (scene_gpu)
			// 	{
			// 		VkDescriptorBufferInfo buffer_info = { 0 };
			// 		buffer_info.buffer = scene_gpu->buffer;
			// 		buffer_info.offset = 0;
			// 		buffer_info.range = scene_gpu->size;
			// 		descriptor_writes[descriptor_writes_index].pBufferInfo = &buffer_info;
			// 	}
			// }
		}
		vkUpdateDescriptorSets(device, descriptor_writes_count, descriptor_writes, 0, NULL);
		AS_FREE(descriptor_writes);
	}
}

void create_command_buffers(as_render* render) 
{
	render->command_buffers.size = MAX_FRAMES_IN_FLIGHT;

	VkCommandBufferAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = render->command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

	AS_ASSERT(vkAllocateCommandBuffers(render->device, &alloc_info, render->command_buffers.data) == VK_SUCCESS, 
		"Failed to allocate command buffers");
}

void create_sync_objects(as_render* render) 
{
	render->image_available_semaphores.size = MAX_FRAMES_IN_FLIGHT;
	render->render_finished_semaphores.size = MAX_FRAMES_IN_FLIGHT;
	render->in_flight_fences.size = MAX_FRAMES_IN_FLIGHT;

	VkSemaphoreCreateInfo semaphore_info = { 0 };
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = { 0 };
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (sz i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		const VkResult image_available_semaphore_create_res = vkCreateSemaphore(render->device, &semaphore_info, NULL, &render->image_available_semaphores.data[i]);
		const VkResult render_finished_semaphore_create_res = vkCreateSemaphore(render->device, &semaphore_info, NULL, &render->render_finished_semaphores.data[i]);
		const VkResult in_flight_fence_create_res = vkCreateFence(render->device, &fence_info, NULL, &render->in_flight_fences.data[i]);
		AS_ASSERT(image_available_semaphore_create_res && render_finished_semaphore_create_res && in_flight_fence_create_res, "Failed to create synchronization objects for a frame!");
	}
}

as_mat4 as_get_camera_view_matrix(as_camera* camera) 
{
	return as_mat4_look_at(&camera->position, &camera->target, &camera->up);
}

void update_shader_uniform_buffer(as_render* render, as_scene* scene, as_shader* shader, as_camera* camera, const u32 current_image)
{
	as_uniform_buffer_object ubo = { 0 };
	as_mat4_set_identity(&ubo.model);
	if (scene)
	{
		memcpy(ubo.object_transforms, scene->gpu_data.objects_transforms, sizeof(ubo.object_transforms));
		ubo.scene_info.m[0][0] = (f32)scene->objects.size;
	}
	if (camera)
	{
		ubo.view = as_get_camera_view_matrix(camera);
	}
	else
	{
		ubo.view = as_mat4_look_at(&(as_vec3) { 10.f, 10.f, 10.f }, & (as_vec3) { 0.0f, 0.0f, 0.0f }, & (as_vec3) { 0.0f, 0.0f, 1.0f });
	}
	ubo.proj = as_mat4_perspective(as_radians(camera->fov), render->swap_chain_extent.width / (f32)render->swap_chain_extent.height, 0.01f, 1000.f);
	ubo.proj.m[1][1] *= -1;

	if (shader->uniform_buffers.buffers_mapped.size != 0)
	{
		memcpy(shader->uniform_buffers.buffers_mapped.data[current_image], &ubo, sizeof(ubo));
	}
}

as_push_const_buffer get_push_const_buffer(const as_object* object, const as_camera* camera, const as_render* render)
{
	as_mat4 buffer_data = {0};
	buffer_data.m[0][0] = camera->position.x;
	buffer_data.m[0][1] = camera->position.y;
	buffer_data.m[0][2] = camera->position.z;

	buffer_data.m[1][0] = camera->cached_direction.x;
	buffer_data.m[1][1] = camera->cached_direction.y;
	buffer_data.m[1][2] = camera->cached_direction.z;

	buffer_data.m[2][0] = as_render_get_time(render);
	buffer_data.m[2][1] = (f32)object->scene_gpu_index;

	return (as_push_const_buffer)
	{
		.data = buffer_data
	};
}

void record_command_buffer(as_render* render, VkCommandBuffer command_buffer, const u32 image_index, as_scene* scene, as_screen_objects_group* ui_objects_group)
{
	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkResult begin_command_buffer_result = vkBeginCommandBuffer(command_buffer, &begin_info);
	AS_ASSERT(begin_command_buffer_result == VK_SUCCESS, "Failed to begin recording command buffer!");

	VkRenderPassBeginInfo render_pass_info = { 0 };
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = render->render_pass;
	render_pass_info.framebuffer = render->swap_chain_framebuffers.data[image_index];
	render_pass_info.renderArea.offset.x = 0;
	render_pass_info.renderArea.offset.y = 0;
	render_pass_info.renderArea.extent = render->swap_chain_extent;

	VkClearColorValue clear_color_value = { {0.f, 0.f, 0.f, 1.f} };
	VkClearDepthStencilValue clear_depth_value = { 1.f, 0 };

	VkClearValue clear_value_c = { .color = clear_color_value,	.depthStencil = {0} };
	VkClearValue clear_value_d = { .color = {0},				.depthStencil = clear_depth_value };
	VkClearValue clear_values[2] = { clear_value_c, clear_value_d }; // same order as attachments

	render_pass_info.pClearValues = clear_values;
	render_pass_info.clearValueCount = AS_ARRAY_SIZE(clear_values);

	as_camera* camera = as_camera_get_main(scene);

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	{
		VkViewport viewport = { 0 };
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (f32)render->swap_chain_extent.width;
		viewport.height = (f32)render->swap_chain_extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor = { 0 };
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent = render->swap_chain_extent;
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);

		if (scene)
		{
			for (sz obj_index = 0 ; obj_index < scene->objects.size ; obj_index++)
			{
				as_object* object = AS_ARRAY_GET(scene->objects, obj_index);
				as_shader* shader = object->shader;
				as_push_const_buffer push_const = get_push_const_buffer(object, camera, render);
				if (!shader || !shader->graphics_pipeline || !as_shader_is_unlocked(render->frame_counter, shader)) { continue; }

				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->graphics_pipeline);
				vkCmdPushConstants(command_buffer, shader->graphics_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_const), &push_const);
				vkCmdBindVertexBuffers(command_buffer, 0, 1, &object->vertex_buffer, &(VkDeviceSize) { 0 });
				vkCmdBindIndexBuffer(command_buffer, object->index_buffer, 0, VK_INDEX_TYPE_UINT16);
				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->graphics_pipeline_layout, 0, 1, &shader->descriptor_sets.data[render->current_frame], 0, 0);
				vkCmdDrawIndexed(command_buffer, object->indices_size, object->instance_count, 0, 0, 0);
			}
		}

		if (ui_objects_group)
		{
			for (i32 i = 0; i < AS_ARRAY_GET_SIZE(*ui_objects_group); i++)
			{
				as_screen_object* screen_object = AS_ARRAY_GET(*ui_objects_group, i);
				if (!screen_object) { continue; }
				if (!screen_object->pipeline) { continue; }

				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, screen_object->pipeline);
				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, screen_object->pipeline_layout, 0, 1, &screen_object->descriptor_sets.data[render->current_frame], 0, 0);
				vkCmdDraw(command_buffer, 3, 1, 0, 0);
			}
		}
	}
	vkCmdEndRenderPass(command_buffer);

	VkResult end_command_buffer_result = vkEndCommandBuffer(command_buffer);
	AS_ASSERT(end_command_buffer_result == VK_SUCCESS, "Failed to record command buffer!");
}

void cleanup_swap_chain(as_render* render)
{
	for (sz i = 0; i < render->swap_chain_framebuffers.size; i++)
	{
		vkDestroyFramebuffer(render->device, render->swap_chain_framebuffers.data[i], NULL);
	}

	for (sz i = 0; i < render->swap_chain_images.size; i++)
	{
		vkDestroyImageView(render->device, render->swap_chain_image_views.data[i], NULL);
	}

	vkDestroyImageView(render->device, render->depth_image_view, NULL);
	vkDestroyImage(render->device, render->depth_image, NULL);
	vkFreeMemory(render->device, render->depth_image_memory, NULL);

	vkDestroySwapchainKHR(render->device, render->swap_chain, NULL);
}

void create_image(as_render* render, u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory) 
{
	VkImageCreateInfo image_info = { 0 };
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = format;
	image_info.tiling = tiling;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = usage;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	AS_ASSERT(vkCreateImage(render->device, &image_info, NULL, image) == VK_SUCCESS, "Failed to create image!");

	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(render->device, *image, &mem_requirements);

	VkMemoryAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = mem_requirements.size;
	allocInfo.memoryTypeIndex = find_memory_type(render, mem_requirements.memoryTypeBits, properties);

	AS_ASSERT(vkAllocateMemory(render->device, &allocInfo, NULL, imageMemory) == VK_SUCCESS, 
		"Failed to allocate image memory");

	vkBindImageMemory(render->device, *image, *imageMemory, 0);
}

void transition_image_layout(as_render* render, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) 
{
	VkCommandBuffer command_buffer = begin_single_time_commands(render);

	VkImageMemoryBarrier barrier = { 0 };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags source_stage;
	VkPipelineStageFlags destination_stage;

	if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (has_stencil_component(format)) 
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else 
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else 
	{
		AS_LOG(LV_ERROR, "Unsupported layout transition");
	}

	vkCmdPipelineBarrier(
		command_buffer,
		source_stage, destination_stage,
		0,
		0, NULL,
		0, NULL,
		1, &barrier
	);

	end_single_time_commands(render, command_buffer);
}

void create_depth_resources(as_render* render) {
	VkFormat depth_format = find_depth_format(render);

	create_image(render, render->swap_chain_extent.width, render->swap_chain_extent.height, depth_format,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &render->depth_image, &render->depth_image_memory);

	render->depth_image_view = create_image_view(render, render->depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void update_time(as_render* render)
{
	clock_t current_time = clock();
	render->time = ((f32)(current_time - start_time)) / CLOCKS_PER_SEC;
}

void recreate_swap_chain(as_render* render, void* display_context)
{
	i32 width = 0, height = 0;
	glfwGetFramebufferSize(display_context, &width, &height);

	while (width == 0 || height == 0)
	{
		return; // I think this is useless and locks the app because it has to be on the main thread, while here it's being executed on the render thread
		/*glfwGetFramebufferSize(display_context, &width, &height);
		glfwWaitEvents();*/
	}

	vkDeviceWaitIdle(render->device);

	cleanup_swap_chain(render);
	create_swap_chain(render, display_context);
	create_image_views(render);
	create_depth_resources(render);
	create_framebuffers(render);
}

as_render* as_render_create(void* display_context)
{
	as_render* render = (as_render*)AS_MALLOC(sizeof(as_render));
	create_instance(render);
	create_surface(render, display_context);
	pick_physical_device(render);
	create_logical_device(render);
	create_swap_chain(render, display_context);
	create_image_views(render);
	create_render_pass(render);
	create_depth_resources(render);
	create_framebuffers(render);
	create_command_pool(render);
	create_command_buffers(render);
	create_sync_objects(render);
	AS_SET_VALID(render);
	AS_LOG(LV_LOG, "Created render");
	return render;
}

void as_render_start_draw_loop(as_render* render)
{
	render->last_frame_time = get_current_time();
}

void as_render_end_draw_loop(as_render* render)
{
	render->current_time = get_current_time();

	const f32 remaining_time = as_render_get_remaining_time(render);
	if (remaining_time > 0)
	{
		sleep_seconds(remaining_time);
	}
	render->delta_time = calculate_delta_time(render->last_frame_time, get_current_time());
	render->last_frame_time = get_current_time();
}

void as_render_draw_frame(as_render* render, void* display_context, as_camera* camera, as_scene* scene, as_screen_objects_group* ui_objects_group)
{
	if (AS_IS_INVALID(render)){ return;};

	vkWaitForFences(render->device, 1, &render->in_flight_fences.data[render->current_frame], VK_TRUE, UINT64_MAX);

	u32 image_index = 0;
	VkResult result = vkAcquireNextImageKHR(render->device, render->swap_chain, UINT64_MAX, render->image_available_semaphores.data[render->current_frame], VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) 
	{
		recreate_swap_chain(render, display_context);
		return;
	}
	AS_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire swap chain image!");

	update_time(render);

	AS_WAIT_AND_LOCK(scene);
	if (scene)
	{
		for (sz obj_index = 0; obj_index < scene->objects.size; obj_index++)
		{
			as_object* object = AS_ARRAY_GET(scene->objects, obj_index);
			if (AS_IS_INVALID(object)) { continue; }
			as_shader* shader = object->shader;
			if (AS_IS_INVALID(shader)) { continue; }
			if (!shader->graphics_pipeline)
			{
				//as_shader_update(render, shader);
			}
			update_shader_uniform_buffer(render, scene, shader, camera, render->current_frame);
		}

		as_scene_gpu_update_data(scene);
		as_scene_gpu_update_buffer(render, scene);

		vkResetFences(render->device, 1, &render->in_flight_fences.data[render->current_frame]);

		vkResetCommandBuffer(render->command_buffers.data[render->current_frame], 0);
		record_command_buffer(render, render->command_buffers.data[render->current_frame], image_index, scene, ui_objects_group);
	}
	AS_UNLOCK(scene);

	VkSubmitInfo submit_info = { 0 };
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	
	VkSemaphore wait_semaphores[] = { render->image_available_semaphores.data[render->current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &render->command_buffers.data[render->current_frame];
	
	VkSemaphore signal_semaphores[] = { render->render_finished_semaphores.data[render->current_frame] };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;
	
	AS_ASSERT(vkQueueSubmit(render->graphics_queue, 1, &submit_info, render->in_flight_fences.data[render->current_frame]) == VK_SUCCESS, 
		"Failed to submit draw command buffer!");

	VkSwapchainKHR swap_chains[] = { render->swap_chain };

	VkPresentInfoKHR present_info = { 0 };
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &image_index;
	
	vkDeviceWaitIdle(render->device);
	result = vkQueuePresentKHR(render->present_queue, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || render->framebuffer_resized) 
	{
		render->framebuffer_resized = false;
		recreate_swap_chain(render, display_context);
	}
	else if (result != VK_SUCCESS) 
	{
		AS_LOG(LV_ERROR, "Failed to present swap chain image!");
	}

	render->current_frame = (render->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	render->frame_counter++;
}

void as_render_destroy(as_render* render)
{
	vkDeviceWaitIdle(render->device);

	cleanup_swap_chain(render);

	vkDestroyRenderPass(render->device, render->render_pass, NULL);

	for (sz i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(render->device, render->render_finished_semaphores.data[i], NULL);
		vkDestroySemaphore(render->device, render->image_available_semaphores.data[i], NULL);
		vkDestroyFence(render->device, render->in_flight_fences.data[i], NULL);
	}

	vkDestroyCommandPool(render->device, render->command_pool, NULL);

	vkDestroyDevice(render->device, NULL);

	if (AS_USE_VULKAN_VALIDATION_LAYER)
	{
		destroy_debug_utils_messenger_EXT(render->instance, render->debug_messenger, NULL);
	}

	vkDestroySurfaceKHR(render->instance, render->surface, NULL);
	vkDestroyInstance(render->instance, NULL);

	AS_IS_INVALID(render);
	AS_FREE(render);
	AS_LOG(LV_LOG, "Destroyed render");
}

u64 as_render_get_frame_count(as_render* render)
{
	if (render)
	{
		return render->frame_counter;
	}
	return 0;
}

extern u64* as_render_get_frame_count_ptr(as_render* render)
{
	if (render)
	{
		return &render->frame_counter;
	}
	return NULL;
}

f64 as_render_get_time(const as_render* render)
{
	return render->time;
}

f64 as_render_get_remaining_time(as_render* render)
{
	clock_t next_frame_time = get_current_time();
	f64 elapsed_time = calculate_delta_time(render->last_frame_time, next_frame_time);
	return (1. / AS_TARGET_FPS) - elapsed_time;
}

f64 as_render_get_delta_time(as_render* render)
{
	return render->delta_time;
}

void as_screen_object_create_pipeline_layout(as_render* render, as_screen_object* screen_object)
{
	AS_ASSERT(screen_object, "Cannot create pipeline layout for screen object, invalid screen object");
	AS_ASSERT(render, "Cannot create pipeline layout for screen object, invalid render");
	
	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(render->physical_device, &device_properties);
	AS_ASSERT(device_properties.limits.maxPushConstantsSize >= sizeof(as_push_const_buffer_screen_object),
		"Cannot create graphics pipeline layout, invalid size of push const buffer");

	VkPushConstantRange push_constant_range_vert = { 0 };
	push_constant_range_vert.offset = 0;
	push_constant_range_vert.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	push_constant_range_vert.size = sizeof(as_push_const_buffer_screen_object);

	VkPushConstantRange push_constant_range_frag = { 0 };
	push_constant_range_frag.offset = 0;
	push_constant_range_frag.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_range_frag.size = sizeof(as_push_const_buffer_screen_object);

	VkPushConstantRange ranges[] = { push_constant_range_vert, push_constant_range_frag };

	VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &screen_object->descriptor_set_layout;
	pipeline_layout_info.pPushConstantRanges = ranges;
	pipeline_layout_info.pushConstantRangeCount = 2;

	VkResult create_pipeline_layout_result = vkCreatePipelineLayout(render->device, &pipeline_layout_info, NULL, &screen_object->pipeline_layout);
	AS_ASSERT(create_pipeline_layout_result == VK_SUCCESS, "Failed to create pipeline layout");
}

void as_screen_object_create_pipeline(as_screen_object* screen_object)
{
	as_file_pool* file_pool = AS_MALLOC_SINGLE(as_file_pool);
	as_shader_binary_pool* shader_binary_pool = AS_MALLOC_SINGLE(as_shader_binary_pool);

	as_shader_binary* vert_shader_bin = as_shader_read_code(shader_binary_pool, file_pool, AS_PATH_DEFAULT_UI_VERT_SHADER, AS_SHADER_TYPE_VERTEX);
	as_shader_binary* frag_shader_bin = as_shader_read_code(shader_binary_pool, file_pool, screen_object->filename_fragment, AS_SHADER_TYPE_FRAGMENT);

	if (vert_shader_bin->binaries_size == 0 || frag_shader_bin->binaries_size == 0)
	{
		as_shader_destroy_binary(shader_binary_pool, frag_shader_bin, true);
		as_shader_destroy_binary(shader_binary_pool, vert_shader_bin, true);
		AS_FREE(shader_binary_pool);
		return;
	}

	VkShaderModule vert_shader_module = create_shader_module(*screen_object->device, vert_shader_bin);
	VkShaderModule frag_shader_module = create_shader_module(*screen_object->device, frag_shader_bin);

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

	VkPipelineVertexInputStateCreateInfo vertex_input_info = { 0 };
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.pVertexBindingDescriptions = NULL;
	vertex_input_info.vertexAttributeDescriptionCount = 0;
	vertex_input_info.pVertexAttributeDescriptions = NULL;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = { 0 };
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_state = { 0 };
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer = { 0 };
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = { 0 };
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_blend_attachment = { 0 };
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_TRUE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending = { 0 };
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state = { 0 };
	depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state.depthTestEnable = VK_TRUE;
	depth_stencil_state.depthWriteEnable = VK_TRUE;
	depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic_state = { 0 };
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = AS_ARRAY_SIZE(dynamic_states);
	dynamic_state.pDynamicStates = dynamic_states;
	
	VkGraphicsPipelineCreateInfo pipeline_info = { 0 };
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pDepthStencilState = &depth_stencil_state;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.layout = screen_object->pipeline_layout;
	pipeline_info.renderPass = *screen_object->render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	if (screen_object->pipeline != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle(*screen_object->device);
		vkDestroyPipeline(*screen_object->device, screen_object->pipeline, NULL);
	}
	vkDeviceWaitIdle(*screen_object->device);

	AS_ASSERT(vkCreateGraphicsPipelines(*screen_object->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &screen_object->pipeline) == VK_SUCCESS,
		"Could not create graphics pipeline for UI");

	vkDestroyShaderModule(*screen_object->device, vert_shader_module, NULL);
	vkDestroyShaderModule(*screen_object->device, frag_shader_module, NULL);

	as_shader_destroy_binary(shader_binary_pool, frag_shader_bin, true);
	as_shader_destroy_binary(shader_binary_pool, vert_shader_bin, true);

	AS_FREE(shader_binary_pool);
	AS_FREE(file_pool);
}
void as_screen_object_create_descriptor_set_layout(as_screen_object* screen_object)
{
	const sz bindings_count = (screen_object->uniforms.size + 1); // ubo + uniforms
	VkDescriptorSetLayoutBinding bindings[AS_MAX_SHADER_UNIFORMS_SIZE + 1] = {0};

	VkDescriptorSetLayoutBinding ubo_layout_binding = { 0 };
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.pImmutableSamplers = NULL;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[ubo_layout_binding.binding] = ubo_layout_binding;

	for (sz i = 0; i < screen_object->uniforms.size; i++)
	{
		as_shader_uniform* uniform = AS_ARRAY_GET(screen_object->uniforms, i);
		VkDescriptorSetLayoutBinding uniform_layout_binding = { 0 };
		uniform_layout_binding.binding = i + 1; // ubo is 0, so + 1
		uniform_layout_binding.descriptorCount = 1;
		uniform_layout_binding.descriptorType = uniform->type;
		uniform_layout_binding.pImmutableSamplers = NULL;
		uniform_layout_binding.stageFlags = uniform->stage;

		bindings[uniform_layout_binding.binding] = uniform_layout_binding;
	}

	VkDescriptorSetLayoutCreateInfo layout_info = { 0 };
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = bindings_count;
	layout_info.pBindings = bindings;

	if (screen_object->descriptor_set_layout)
	{
		vkDeviceWaitIdle(*screen_object->device);
		vkDestroyDescriptorSetLayout(*screen_object->device, screen_object->descriptor_set_layout, NULL);
	}
	vkDeviceWaitIdle(*screen_object->device);
	AS_ASSERT(vkCreateDescriptorSetLayout(*screen_object->device, &layout_info, NULL, &screen_object->descriptor_set_layout) == VK_SUCCESS,
		"Failed to create descriptor set layout!");
}

void as_screen_object_create_uniform_buffers_direct(as_uniform_buffers* uniform_buffers, as_render* render)
{
	VkDeviceSize buffer_size = sizeof(as_uniform_buffer_screen_object);

	uniform_buffers->buffers.size = MAX_FRAMES_IN_FLIGHT;
	uniform_buffers->memories.size = MAX_FRAMES_IN_FLIGHT;
	uniform_buffers->buffers_mapped.size = MAX_FRAMES_IN_FLIGHT;

	for (sz i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (uniform_buffers->buffers.data[i])
		{
			vkDestroyBuffer(render->device, uniform_buffers->buffers.data[i], NULL);
		}
		if (uniform_buffers->memories.data[i])
		{
			vkFreeMemory(render->device, uniform_buffers->memories.data[i], NULL);
		}
	}

	for (sz i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		create_buffer(render, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&uniform_buffers->buffers.data[i], &uniform_buffers->memories.data[i]);

		vkMapMemory(render->device, uniform_buffers->memories.data[i], 0, buffer_size, 0, &uniform_buffers->buffers_mapped.data[i]);
	}
}

void as_screen_object_create_descriptor_pool(as_screen_object* screen_object)
{
	VkDescriptorPoolSize pool_size = { 0 };
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = (u32)MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo pool_info = { 0 };
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = &pool_size;
	pool_info.maxSets = (u32)MAX_FRAMES_IN_FLIGHT;

	if (screen_object->descriptor_pool)
	{
		vkDeviceWaitIdle(*screen_object->device);
		vkDestroyDescriptorPool(*screen_object->device, screen_object->descriptor_pool, NULL);
	}
	vkDeviceWaitIdle(*screen_object->device);
	vkCreateDescriptorPool(*screen_object->device, &pool_info, NULL, &screen_object->descriptor_pool);
}

void as_screen_object_allocate_descriptor_set(as_screen_object* screen_object)
{
	VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		layouts[i] = screen_object->descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo alloc_info = { 0 };
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = screen_object->descriptor_pool;
	alloc_info.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
	alloc_info.pSetLayouts = layouts;

	AS_ASSERT(vkAllocateDescriptorSets(*screen_object->device, &alloc_info, screen_object->descriptor_sets.data) == VK_SUCCESS, "Failed to allocate descriptor sets!");
	screen_object->descriptor_sets.size = MAX_FRAMES_IN_FLIGHT;

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		as_uniform_buffers* uniform_buffers = &screen_object->uniform_buffers;
		AS_ASSERT(uniform_buffers, TEXT("Cannot create descriptor sets, invalid uniform_buffers"));

		VkDescriptorBufferInfo buffer_info = { 0 };
		buffer_info.buffer = uniform_buffers->buffers.data[i];
		buffer_info.offset = 0;
		buffer_info.range = sizeof(as_uniform_buffer_screen_object);

		const sz descriptor_writes_count = screen_object->uniforms.size + 1; // ubo + uniforms
		VkWriteDescriptorSet descriptor_writes[AS_MAX_SHADER_UNIFORMS_SIZE] = { 0 };

		descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[0].dstSet = screen_object->descriptor_sets.data[i];
		descriptor_writes[0].dstBinding = 0;
		descriptor_writes[0].dstArrayElement = 0;
		descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_writes[0].descriptorCount = 1;
		descriptor_writes[0].pBufferInfo = &buffer_info;

		for (sz j = 0; j < screen_object->uniforms.size; j++)
		{
			as_shader_uniform* uniform = AS_ARRAY_GET(screen_object->uniforms, j);
			if (!uniform)
			{
				continue;
			}

			const sz descriptor_writes_index = j + 1;

			descriptor_writes[descriptor_writes_index].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[descriptor_writes_index].dstSet = screen_object->descriptor_sets.data[i];
			descriptor_writes[descriptor_writes_index].dstBinding = descriptor_writes_index;
			descriptor_writes[descriptor_writes_index].dstArrayElement = 0;
			descriptor_writes[descriptor_writes_index].descriptorType = uniform->type;
			descriptor_writes[descriptor_writes_index].descriptorCount = 1;
			if (uniform->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				as_texture* texture = (as_texture*)uniform->data;
				if (texture)
				{
					VkDescriptorImageInfo image_info = { 0 };
					image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					image_info.imageView = texture->image_view;
					image_info.sampler = texture->sampler;
					descriptor_writes[descriptor_writes_index].pImageInfo = &image_info;
				}
			}
		}
		vkUpdateDescriptorSets(*screen_object->device, descriptor_writes_count, descriptor_writes, 0, NULL);
	}
}

void as_screen_object_init(as_render* render, as_screen_object* screen_object, const char* fragment_path)
{
	AS_ASSERT(render, "Cannot init screen object, invalid render");
	AS_ASSERT(screen_object, "Cannot init screen object, invalid screen_object");

	AS_FLOG(LV_LOG, "Init screen object %p", screen_object);

	screen_object->device = &render->device;
	screen_object->render_pass = &render->render_pass;
	if (fragment_path)
	{
		strcpy(screen_object->filename_fragment, fragment_path);
	}
	else
	{
		strcpy(screen_object->filename_fragment, AS_PATH_DEFAULT_UI_FRAG_SHADER);
	}
}

void as_screen_object_update(as_render* render, as_screen_object* screen_object)
{
	AS_ASSERT(render, "Cannot update screen object, invalid render");
	AS_ASSERT(screen_object, "Cannot update screen object, invalid screen_object");

	AS_FLOG(LV_LOG, "Update screen object %p", screen_object);
	
	as_screen_object_create_descriptor_set_layout(screen_object);
	as_screen_object_create_pipeline_layout(render, screen_object);
	as_screen_object_create_pipeline(screen_object);
	as_screen_object_create_uniform_buffers_direct(&screen_object->uniform_buffers, render);
	as_screen_object_create_descriptor_pool(screen_object);
	as_screen_object_allocate_descriptor_set(screen_object);

	AS_SET_VALID(screen_object);
}

void as_screen_object_destroy(as_screen_object* screen_object, const b8 free_ptr)
{
	if (!screen_object) { return; }
	if (AS_IS_INVALID(screen_object)) { return; }

	AS_FLOG(LV_LOG, "Destroy screen object %p", screen_object);

	AS_SET_INVALID(screen_object);

	if (screen_object->device)
	{
		for (sz i = 0; i < screen_object->uniforms.size; i++)
		{
			if (screen_object->uniforms.data[i].type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				as_texture* texture = (as_texture*)screen_object->uniforms.data[i].data;
				if (!texture) { continue; }
				as_texture_destroy(texture);
			}
			AS_FREE(screen_object->uniforms.data[i].data);
		}

		for (sz i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(*screen_object->device, screen_object->uniform_buffers.buffers.data[i], NULL);
			vkFreeMemory(*screen_object->device, screen_object->uniform_buffers.memories.data[i], NULL);
		}

		if (screen_object->pipeline)
		{
			vkDestroyPipeline(*screen_object->device, screen_object->pipeline, NULL);
		}
		if (screen_object->pipeline_layout)
		{
			vkDestroyPipelineLayout(*screen_object->device, screen_object->pipeline_layout, NULL);
		}
		if (screen_object->descriptor_pool)
		{
			vkDestroyDescriptorPool(*screen_object->device, screen_object->descriptor_pool, NULL);
		}
		if (screen_object->descriptor_set_layout)
		{
			vkDestroyDescriptorSetLayout(*screen_object->device, screen_object->descriptor_set_layout, NULL);
		}
	}

	if (free_ptr)
	{
		AS_FREE(screen_object);
	}
}

as_screen_objects_group* as_screen_objects_group_create()
{
	return AS_MALLOC_SINGLE(as_screen_objects_group);
}

void as_screen_objects_group_destroy(as_screen_objects_group* ui_objects_group)
{
	AS_ARRAY_FOR_EACH(*ui_objects_group, as_screen_object, screen_object,
	{
		as_screen_object_destroy(screen_object, false);
	});
	AS_FREE(ui_objects_group);
}

as_texture* as_texture_make(const char* path)
{
	as_texture* texture = AS_MALLOC_SINGLE(as_texture);
	strcpy(texture->filename, path);
	return texture;
}

void as_texture_init(as_texture* texture, const char* path)
{
	AS_ASSERT(texture, "Cannot init texture, invalid pointer");
	AS_FLOG(LV_LOG, "Init texture %p", texture);
	strcpy(texture->filename, path);
}

bool as_texture_update(as_render* render, as_texture* texture)
{
	AS_ASSERT(render, "Trying to update texture but render is NULL");
	AS_ASSERT(texture, "Trying to update texture but texture is NULL");

	AS_FLOG(LV_LOG, "Update texture %p", texture);


	AS_SET_INVALID(texture);

	if (texture->filename)
	{
		as_texture_destroy(texture);
	}

	texture->device = &render->device;

	u32 tex_width, tex_height, tex_channels;
	stbi_uc* pixels = stbi_load(texture->filename, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
	
	if (!pixels)
	{
		AS_LOG(LV_WARNING, "Failed to load texture image!");
		return false;
	}

	VkDeviceSize image_size = tex_width * tex_height * 4;

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	create_buffer(render, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

	void* data;
	vkMapMemory(render->device, staging_buffer_memory, 0, image_size, 0, &data);
	memcpy(data, pixels, (sz)image_size);
	vkUnmapMemory(render->device, staging_buffer_memory);

	stbi_image_free(pixels);

	create_image(render, tex_width, tex_height,
		VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&texture->image, &texture->memory);

	transition_image_layout(render, texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copy_buffer_to_image(render, staging_buffer, texture->image, tex_width, tex_height);
	transition_image_layout(render, texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(render->device, staging_buffer, NULL);
	vkFreeMemory(render->device, staging_buffer_memory, NULL);

	texture->image_view = create_image_view(render, texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	VkPhysicalDeviceProperties properties = { 0 };
	vkGetPhysicalDeviceProperties(render->physical_device, &properties);

	VkSamplerCreateInfo sampler_info = { 0 };
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.anisotropyEnable = VK_TRUE;
	sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	const VkResult create_sampler_result = vkCreateSampler(render->device, &sampler_info, NULL, &texture->sampler);
	AS_ASSERT(create_sampler_result == VK_SUCCESS, "Failed to create texture sampler!");
	AS_SET_VALID(texture);
	return true;
}

void as_texture_destroy(as_texture* texture)
{
	if (!texture) { return; }
	if (AS_IS_INVALID(texture)) { return; }

	AS_FLOG(LV_LOG, "Destroy texture %p", texture);

	AS_LOCK(texture);
	if (texture->device && *texture->device)
	{
		if (texture->image)
		{
			vkDestroyImage(*texture->device, texture->image, NULL);
		}
		if (texture->image_view)
		{
			vkDestroyImageView(*texture->device, texture->image_view, NULL);
		}
		if (texture->sampler)
		{
			vkDestroySampler(*texture->device, texture->sampler, NULL);
		}
		if (texture->memory)
		{
			vkFreeMemory(*texture->device, texture->memory, NULL);
		}
	}
	AS_SET_INVALID(texture);
}

as_textures_pool* as_textures_pool_create()
{
	return AS_MALLOC_SINGLE(as_textures_pool);
}

void as_textures_pool_destroy(as_textures_pool* textures_pool)
{
	AS_FLOG(LV_LOG, "Destroy texture pool %p", textures_pool);

	if (!textures_pool) { return; }
	AS_ARRAY_FOR_EACH(*textures_pool, as_texture, texture,
	{
		if (textures_pool->size < 0 || textures_pool->size > AS_MAX_TEXTURE_POOL_SIZE)
		{
			break;
		}
		as_texture_destroy(texture);
	});
	AS_FREE(textures_pool);
}

as_texture* as_texture_get_from_pool(as_textures_pool* textures_pool)
{
	AS_ASSERT(textures_pool, "Cannot get texture from pool, invalid textures_pool ptr");
	return AS_ARRAY_INCREMENT(*textures_pool);
}

void as_texture_remove_from_pool(as_textures_pool* textures_pool, as_texture* texture, const bool destory)
{
	AS_ASSERT(texture, "Cannot remove texture from pool, invalid texture ptr");
	AS_ASSERT(textures_pool, "Cannot remove texture from pool, invalid textures_pool ptr");

	if (destory)
	{
		as_texture_destroy(texture);
	}

	sz found_index = -1;
	AS_ARRAY_FIND_PTR(*textures_pool, texture, found_index);
	if (found_index > -1)
	{
		AS_ARRAY_REMOVE_AT(*textures_pool, found_index);
	}
}

void as_shader_create_graphics_pipeline(as_shader* shader)
{
	AS_FLOG(LV_LOG, "Creating graphics pipeline for shader %p", shader);
	if (strcmp(shader->filename_vertex, "") == 0 || strcmp(shader->filename_fragment, "") == 0)
	{
		return;
	}

	as_file_pool* file_pool = AS_MALLOC_SINGLE(as_file_pool);
	as_shader_binary_pool* shader_binary_pool = AS_MALLOC_SINGLE(as_shader_binary_pool);
	as_shader_binary* vert_shader_bin = as_shader_read_code(shader_binary_pool, file_pool, shader->filename_vertex, AS_SHADER_TYPE_VERTEX);
	as_shader_binary* frag_shader_bin = as_shader_read_code(shader_binary_pool, file_pool, shader->filename_fragment, AS_SHADER_TYPE_FRAGMENT);

	if (vert_shader_bin->binaries_size == 0 || frag_shader_bin->binaries_size == 0)
	{
		as_shader_destroy_binary(shader_binary_pool, frag_shader_bin, true);
		as_shader_destroy_binary(shader_binary_pool, vert_shader_bin, true);
		AS_FREE(shader_binary_pool);
		return;
	}

	VkShaderModule vert_shader_module = create_shader_module(*shader->device, vert_shader_bin);
	VkShaderModule frag_shader_module = create_shader_module(*shader->device, frag_shader_bin);

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

	VkPipelineVertexInputStateCreateInfo vertex_input_info = { 0 };
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkVertexInputBindingDescription binding_description = as_get_binding_description();
	VkVertexInputAttributeDescription* attribute_descriptions = (VkVertexInputAttributeDescription*)AS_MALLOC(sizeof(VkVertexInputAttributeDescription) * AS_VERTEX_VAR_COUNT);
	as_get_attribute_descriptions(attribute_descriptions);
	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.vertexAttributeDescriptionCount = AS_VERTEX_VAR_COUNT;
	vertex_input_info.pVertexBindingDescriptions = &binding_description;
	vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = { 0 };
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_state = { 0 };
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer = { 0 };
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = { 0 };
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depth_stencil = { 0 };
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = VK_TRUE;
	depth_stencil.depthWriteEnable = VK_TRUE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState color_blend_attachment = { 0 };
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;

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

	VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic_state = { 0 };
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = AS_ARRAY_SIZE(dynamic_states);
	dynamic_state.pDynamicStates = dynamic_states;

	VkGraphicsPipelineCreateInfo pipeline_info = { 0 };
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = shader->graphics_pipeline_layout;
	pipeline_info.renderPass = *shader->render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	// IDK IF NEEDED, TODO: figure this out
	if (shader->graphics_pipeline != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle(*shader->device);
		vkDestroyPipeline(*shader->device, shader->graphics_pipeline, NULL);
	}
	vkDeviceWaitIdle(*shader->device);
	VkResult create_graphics_pipeline_result = vkCreateGraphicsPipelines(*shader->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &shader->graphics_pipeline);
	AS_ASSERT(create_graphics_pipeline_result == VK_SUCCESS, "Failed to create graphics pipeline");

	vkDestroyShaderModule(*shader->device, frag_shader_module, NULL);
	vkDestroyShaderModule(*shader->device, vert_shader_module, NULL);

	as_shader_destroy_binary(shader_binary_pool, frag_shader_bin, true);
	as_shader_destroy_binary(shader_binary_pool, vert_shader_bin, true);

	AS_FREE(attribute_descriptions);
	AS_FREE(shader_binary_pool);
	AS_FREE(file_pool);
}

sz as_shader_add_uniform_float(as_shader_uniforms* uniforms, f32* value)
{
	AS_ASSERT(uniforms, "Trying to add float uniform but uniforms array is NULL");
	AS_ASSERT(value, "Trying to add float uniform but flaot value is NULL");

	as_shader_uniform shader_uniform = { 0 };
	AS_ARRAY_INSERT_AT((*uniforms), uniforms->size, shader_uniform);
	const sz index = uniforms->size - 1;
	uniforms->data[index].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	uniforms->data[index].data = AS_MALLOC(sizeof(f32));
	uniforms->data[index].data = value;

	return index;
}

sz as_shader_add_uniform_texture(as_shader_uniforms* uniforms, as_texture* texture)
{
	AS_ASSERT(uniforms, "Trying to add texture uniform but uniforms array is NULL");
	AS_ASSERT(texture, "Trying to add texture uniform but texture is NULL");

	as_shader_uniform shader_uniform = {0};
	AS_ARRAY_INSERT_AT((*uniforms), uniforms->size, shader_uniform);
	const sz index = uniforms->size - 1;
	uniforms->data[index].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uniforms->data[index].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	uniforms->data[index].data = texture;

	return index;
}

sz as_shader_add_scene_gpu(as_shader_uniforms* uniforms, as_scene_gpu_buffer* scene_gpu_buffer)
{
    AS_ASSERT(uniforms, "Trying to add scene GPU buffer but uniforms array is NULL");
    AS_ASSERT(scene_gpu_buffer, "Trying to add scene GPU buffer but buffer is NULL");

    as_shader_uniform shader_uniform = { 0 };
    AS_ARRAY_INSERT_AT((*uniforms), uniforms->size, shader_uniform);
    const sz index = uniforms->size - 1;
    uniforms->data[index].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	uniforms->data[index].stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uniforms->data[index].data = scene_gpu_buffer;

    return index;
}

as_shader* as_shader_make(as_render* render, const char* vertex_shader_path, const char* fragment_shader_path)
{
	AS_ASSERT(render, "Trying to create shader, but render is NULL");
	AS_ASSERT(vertex_shader_path, "Trying to create shader, but vertex_shader_path is NULL");
	AS_ASSERT(fragment_shader_path, "Trying to create shader, but fragment_shader_path is NULL");

	as_shader* shader = AS_MALLOC_SINGLE(as_shader);
	shader->device = &render->device;
	shader->render_pass = &render->render_pass;
	strcpy(shader->filename_fragment, fragment_shader_path);
	strcpy(shader->filename_vertex, vertex_shader_path);

	AS_SET_VALID(shader);
	return shader;
}

extern void as_shader_set_uniforms(as_render* render, as_shader* shader, as_shader_uniforms* uniforms)
{
	AS_ASSERT(render, "Trying to set shader uniforms, but render is NULL");
	AS_ASSERT(shader, "Trying to set shader uniforms, but shader is NULL");
	AS_ASSERT(uniforms, "Trying to set shader uniforms, but uniforms is NULL");

	shader->uniforms = *uniforms;
	as_shader_update(render, shader);
}

void as_shader_update(as_render* render, as_shader* shader)
{
	AS_ASSERT(render, "Trying to update shader, but render is NULL");
	AS_ASSERT(shader, "Trying to update shader, but shader is NULL");
	AS_FLOG(LV_LOG, "Updating shader %p", shader);

	shader->device = &render->device;
	shader->render_pass = &render->render_pass;
	as_shader_create_descriptor_set_layout(shader);
	as_shader_create_graphics_pipeline_layout(render, &shader->graphics_pipeline_layout, &shader->descriptor_set_layout);
	as_shader_create_graphics_pipeline(shader);
	create_uniform_buffers_direct(&shader->uniform_buffers, render);
	create_descriptor_pool(render->device, &shader->descriptor_pool);
	create_descriptor_sets_from_shader(render->device, shader);
	AS_SET_VALID(shader);
}

void as_shader_destroy(as_render* render, as_shader* shader)
{
	AS_ASSERT(render, "Trying to delete object, but object is NULL");

	if (!shader) { return; }
	if (AS_IS_INVALID(shader)) { return; }

	AS_FLOG(LV_LOG, "Destroying shader %p", shader);

	vkDestroyPipeline(render->device, shader->graphics_pipeline, NULL);
	vkDestroyPipelineLayout(render->device, shader->graphics_pipeline_layout, NULL);

	for (sz i = 0; i < shader->uniforms.size; i++)
	{
		if (shader->uniforms.data[i].type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		{
			as_texture* texture = (as_texture*)shader->uniforms.data[i].data;
			if (!texture) { continue; }
			as_texture_destroy(texture);
		}
		AS_FREE(shader->uniforms.data[i].data);
	}

	for (sz i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(render->device, shader->uniform_buffers.buffers.data[i], NULL);
		vkFreeMemory(render->device, shader->uniform_buffers.memories.data[i], NULL);
	}

	vkDestroyDescriptorPool(render->device, shader->descriptor_pool, NULL);
	vkDestroyDescriptorSetLayout(render->device, shader->descriptor_set_layout, NULL);

	AS_SET_INVALID(shader);
	AS_FREE(shader);
}

void as_camera_update_direction(as_camera* camera)
{
	as_vec3_sub(&camera->cached_direction, &camera->target, &camera->position); // update cached direction (needed for uniforms)
}

as_camera* as_camera_make(as_scene* scene, const as_vec3* position, const as_vec3* target)
{
	AS_ASSERT(position, "Trying to create camera, but position is NULL");
	AS_ASSERT(target, "Trying to create camera, but target is NULL");

	as_camera* camera = AS_ARRAY_INCREMENT(scene->cameras);
	camera->position = *position;
	camera->target = *target;
	camera->up = AS_VEC(as_vec3, 0.f, 0.f, 1.f);
	camera->fov = 45.f;
	camera->movement_speed = 40.f;
	as_camera_update_direction(camera);
	AS_IS_VALID(camera);
	return camera;
}

as_camera* as_camera_get_main(as_scene* scene)
{
	// no checks for speed's sake, this will be called every frame
	return AS_ARRAY_GET(scene->cameras, 0);
}

void as_camera_set_main(as_scene* scene, as_camera* camera)
{
	AS_ASSERT(scene, "Cannot set camera as main, scene is NULL");
	AS_ASSERT(camera, "Cannot set camera as main, camera is NULL");
	AS_ASSERT(scene->cameras.size > 0, "Cannot set camera as main, no cameras found in the scene");

	// main should be index 0

	if (AS_ARRAY_GET(scene->cameras, 0) == camera) // check if it's already 0
	{
		return;
	}

	sz found_camera_index = -1;
	AS_ARRAY_FIND_PTR(scene->cameras, camera, found_camera_index);
	if (found_camera_index >= 0)
	{
		AS_ARRAY_SWAP(as_camera, scene->cameras, found_camera_index, 0);
	}
}

void as_camera_set_position(as_camera* camera, const as_vec3* position)
{
	AS_ASSERT(camera, "Trying to set camera position, but camera is NULL");
	AS_ASSERT(position, "Trying to set camera position, but position is NULL");
	
	camera->position = *position;
	as_camera_update_direction(camera);
}

void as_camera_set_target(as_camera* camera, const as_vec3* target)
{
	AS_ASSERT(camera, "Trying to set camera target, but camera is NULL");
	AS_ASSERT(target, "Trying to set camera target, but target is NULL");
	
	camera->target = *target;
	as_camera_update_direction(camera);
}

as_object* as_object_consturct(as_render* render, as_scene* scene)	
{
	AS_ASSERT(scene, "Trying to construct object, but scene is NULL");

	as_object* object = AS_ARRAY_INCREMENT(scene->objects);
	as_mat4_set_identity(&object->transform);
	object->instance_count = 1;

	AS_FLOG(LV_LOG, "Constructed object %p", object);
	return object;
}

void as_object_update(as_render* render, as_object* object, as_shape* shape, as_shader* shader)
{
	AS_ASSERT(render, "Trying to update object, but render is NULL");
	AS_ASSERT(object, "Trying to update object, but render is NULL");
	AS_ASSERT(shape, "Trying to update object, but shape is NULL");
	AS_ASSERT(shader, "Trying to update object, but shader is NULL");

	object->shape = shape;

	// vertex buffer
	VkDeviceSize vertex_buffer_size = sizeof(shape->vertices[0]) * shape->vertices_size;
	VkBuffer vertex_staging_buffer;
	VkDeviceMemory vertex_staging_buffer_memory;
	create_buffer(render, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&vertex_staging_buffer, &vertex_staging_buffer_memory);

	void* data;
	vkMapMemory(render->device, vertex_staging_buffer_memory, 0, vertex_buffer_size, 0, &data);
	memcpy(data, shape->vertices, (sz)vertex_buffer_size);
	vkUnmapMemory(render->device, vertex_staging_buffer_memory);

	create_buffer(render, vertex_buffer_size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &object->vertex_buffer, &object->vertex_buffer_memory);

	copy_buffer(render, vertex_staging_buffer, object->vertex_buffer, vertex_buffer_size);

	vkDestroyBuffer(render->device, vertex_staging_buffer, NULL);
	vkFreeMemory(render->device, vertex_staging_buffer_memory, NULL);

	// index buffer

	VkDeviceSize index_buffer_size = sizeof(shape->indices[0]) * shape->indices_size;
	object->indices_size = shape->indices_size;
	VkBuffer index_staging_buffer;
	VkDeviceMemory index_staging_buffer_memory;
	create_buffer(render, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&index_staging_buffer, &index_staging_buffer_memory);

	vkMapMemory(render->device, index_staging_buffer_memory, 0, index_buffer_size, 0, &data);
	memcpy(data, shape->indices, (sz)index_buffer_size);
	vkUnmapMemory(render->device, index_staging_buffer_memory);

	create_buffer(render, index_buffer_size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &object->index_buffer, &object->index_buffer_memory);

	copy_buffer(render, index_staging_buffer, object->index_buffer, index_buffer_size);

	vkDestroyBuffer(render->device, index_staging_buffer, NULL);
	vkFreeMemory(render->device, index_staging_buffer_memory, NULL);

	object->shader = shader;
	AS_SET_VALID(object);

	AS_FLOG(LV_LOG, "Updated object %p", object);
}

void as_object_set_instance_count(as_object* object, const u32 instance_count)
{
	AS_ASSERT(object, "Trying to set instance count object, but object is NULL");

	object->instance_count = instance_count;
}

void as_object_set_translation(as_object* object, const as_vec3* translation)
{
	AS_ASSERT(object, TEXT("Trying to set object location, but object is NULL"));
	AS_ASSERT(translation, TEXT("Trying to set object location, but translation is NULL"));

	as_mat4_set_translation(&object->transform, translation);
}

void as_object_translate(as_object* object, const as_vec3* translation)
{
	AS_ASSERT(object, TEXT("Cannot translate object, but object is NULL"));
	AS_ASSERT(translation, TEXT("Cannot translate object, but translation is NULL"));

	as_mat4_translate(&object->transform, translation);
}

void as_object_set_rotation(as_object* object, const as_vec3* rotation)
{
	AS_ASSERT(object, TEXT("Trying to set object rotation, but object is NULL"));
	AS_ASSERT(rotation, TEXT("Trying to set object rotation, but rotation is NULL"));

	as_mat4_set_rotation(&object->transform, rotation);
}

void as_object_rotate(as_object* object, const f32 angle, const as_vec3* axis)
{
	AS_ASSERT(object, TEXT("Trying to rotate object, but object is NULL"));
	AS_ASSERT(axis, TEXT("Trying to rotate object, but axis is NULL"));

	as_mat4_rotate(&object->transform, angle, axis);
}

void as_object_rotate_around_pivot(as_object* object, const f32 angle, const as_vec3* axis, const as_vec3* pivot)
{
	AS_ASSERT(object, TEXT("Trying to rotate object, but object is NULL"));
	AS_ASSERT(axis, TEXT("Trying to rotate object, but axis is NULL"));

	as_mat4_rotate_around_pivot(&object->transform, angle, axis, pivot);
}

void as_object_set_scale(as_object* object, const as_vec3* scale)
{
	AS_ASSERT(object, TEXT("Trying to set object scale, but object is NULL"));
	AS_ASSERT(scale, TEXT("Trying to set object scale, but scale is NULL"));

	as_mat4_set_scale(&object->transform, scale);
}

const as_mat4* as_object_get_transform(const as_object* object)
{
	return &object->transform;
}

as_vec3 as_object_get_translation(const as_object* object)
{
	return as_mat4_get_translation(as_object_get_transform(object));
}

void as_object_destroy(as_render* render, as_object* object)
{
	AS_ASSERT(render, "Trying to delete object, but object is NULL");
	AS_FLOG(LV_LOG, "Destroying object %p", object);
	if (!object || AS_IS_INVALID(object)) { return; }

	as_shader_destroy(render, object->shader);

	vkDestroyBuffer(render->device, object->index_buffer, NULL);
	vkFreeMemory(render->device, object->index_buffer_memory, NULL);

	vkDestroyBuffer(render->device, object->vertex_buffer, NULL);
	vkFreeMemory(render->device, object->vertex_buffer_memory, NULL);

	AS_SET_INVALID(object);
}

void as_scene_gpu_create_descriptor(as_render* render, as_scene* scene)
{
	AS_ASSERT(render, TEXT("Trying to create scene gpu descriptor, but render is NULL"));
	AS_ASSERT(scene, TEXT("Trying to create scene gpu descriptor, but scene is NULL"));
}

VkDeviceSize as_scene_get_size(as_render* render)
{
	VkPhysicalDeviceProperties physical_device_properties;
	vkGetPhysicalDeviceProperties(render->physical_device, &physical_device_properties);
	VkDeviceSize min_uniform_buffer_offset_alignment = physical_device_properties.limits.minUniformBufferOffsetAlignment;
	return (sizeof(as_scene_gpu_data) + min_uniform_buffer_offset_alignment - 1) & ~(min_uniform_buffer_offset_alignment - 1);
}

as_scene* as_scene_create(as_render* render, const char* scene_path)
{
	as_scene* scene = AS_MALLOC_SINGLE(as_scene);
	strcpy(scene->path, scene_path);
	VkDeviceSize size = as_scene_get_size(render);
	//create_buffer(render, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &scene->gpu_buffer.buffer, &scene->gpu_buffer.memory);
	as_scene_gpu_update_data(scene);
	as_scene_gpu_update_buffer(render, scene);
	AS_SET_VALID(scene);
	return scene;
}

as_scene* as_scene_load(as_render* render, const char* scene_path)
{
	AS_ASSERT(render, "Cannot load scene, render is NULL");
	AS_ASSERT(scene_path, "Cannot load scene, scene_path is NULL");
	return NULL;
	//for (scene->cameras)
	//{
	//}
	/*as_scene* scene = AS_DESERIALIZE(as_scene, AS_PATH_DEFAULT_SCENE);
	if(!scene)
	{
		AS_LOG(LV_LOG, "Could not find scene to load, creating a new one")
		return as_scene_create(render, scene_path);
	}*/

	//AS_ARRAY_FOR_EACH(scene->objects, as_object, object,
	//{
	//	as_object_update(render, object, )
	//});

	/*AS_ARRAY_FOR_EACH(scene->cameras, as_camera, camera,
	{
		AS_SET_VALID(camera);
	});*/

	//VkDeviceSize size = as_scene_get_size(render);
	//create_buffer(render, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &scene->gpu_buffer.buffer, &scene->gpu_buffer.memory);
	//return scene;
}

static as_vec3 cached_camera_position = { 0 }; // used for compare by distance to camera
i32 compare_objects_by_distance_to_camera(const void* a, const void* b)
{
	const as_object* object_a = (const as_object*)a;
	const as_object* object_b = (const as_object*)b;

	const as_vec3 position_a = as_object_get_translation(object_a);
	const as_vec3 position_b = as_object_get_translation(object_b);

	const f32 distance_a = as_vec3_squared_distance(&position_a, &cached_camera_position);
	const f32 distance_b = as_vec3_squared_distance(&position_b, &cached_camera_position);

	if (distance_a < distance_b) return -1;
	else if (distance_a > distance_b) return 1;
	else return 0;
}

void as_order_scene_objects_by_distance_to_camera(as_scene* scene)
{
	as_camera* main_camera = as_camera_get_main(scene);
	if (main_camera)
	{
		cached_camera_position = main_camera->position;
	}

	qsort(scene->objects.data, scene->objects.size, sizeof(as_object),
		compare_objects_by_distance_to_camera);
}

void as_scene_gpu_update_data(as_scene* scene)
{
	AS_ASSERT(scene, "Cannot make GPU scene data, invalid scene");

	// scene->gpu_data.lights = scene->lights; // need to pack the lights in mat4 for alignments
	// AS_ARRAY_CLEAR(scene->gpu_data.objects_transforms);

	// info
	scene->gpu_data.info.m[0][0] = (f32)AS_ARRAY_GET_SIZE(scene->objects);
	
	//as_order_scene_objects_by_distance_to_camera(scene);

	// assign
	for (sz i = 0; i < AS_ARRAY_GET_SIZE(scene->objects); i++)
	{
		if (i >= AS_MAX_GPU_OBJECT_TRANSFORMS_SIZE)
		{
			break;
		}
		as_object* object = AS_ARRAY_GET(scene->objects, i);
		object->scene_gpu_index = i;
		scene->gpu_data.objects_transforms[i] = object->transform;
	}
}

void as_scene_gpu_update_buffer(as_render* render, as_scene* scene)
{
	AS_ASSERT(render, "Cannot make GPU scene buffer, invalid render");
	
	// ensure it has a proper alignment

	//scene->gpu_buffer.size = as_scene_get_size(render);
	//void* data = NULL;
	//vkMapMemory(render->device, scene->gpu_buffer.memory, 0, scene->gpu_buffer.size, 0, &data);
	//memcpy(data, &scene->gpu_data, (sz)scene->gpu_buffer.size);
	//vkUnmapMemory(render->device, scene->gpu_buffer.memory);
}

void as_scene_destroy(as_render *render, as_scene *scene)
{
	AS_ASSERT(render, "Trying to delete objects, but object is NULL");
	if (!scene) { return; }

	vkDeviceWaitIdle(render->device);

	for (sz i = 0; i < scene->objects.size; i++)
	{
		as_object_destroy(render, &scene->objects.data[i]);
	}

	//vkFreeMemory(render->device, scene->gpu_buffer.memory, NULL);
	//vkDestroyBuffer(render->device, scene->gpu_buffer.buffer, NULL);

	AS_FREE(scene);
}
