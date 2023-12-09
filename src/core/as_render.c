#include "core/as_render.h"
#include "core/as_shader.h"
#include "as_memory.h"
#include "as_utility.h"

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

typedef struct QueueFamilyIndices 
{
	u32 graphics_family;
	u32 present_family;
} QueueFamilyIndices;

const char* instance_extensions[] = 
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME,
#if AS_USE_VULKAN_VALIDATION_LAYER
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
};
const u32 instance_extensions_count = sizeof(instance_extensions) / sizeof(instance_extensions[0]);
const char* device_extensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
const u32 device_extensions_count = sizeof(device_extensions) / sizeof(device_extensions[0]);

const char* validation_layers[] =
{
	"VK_LAYER_KHRONOS_validation"
};

VkDebugUtilsMessengerEXT debug_messenger;

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

QueueFamilyIndices find_queue_families(const VkPhysicalDevice device, const VkSurfaceKHR surface) 
{
	QueueFamilyIndices indices = { UINT32_MAX, UINT32_MAX };

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

	VkQueueFamilyProperties* queue_families = AS_MALLOC(queue_family_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
	for (u32 i = 0; i < queue_family_count; i++) 
	{
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
		{
			indices.graphics_family = i;
		}
		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
		if (present_support) 
		{
			indices.present_family = i;
		}
		if (indices.graphics_family != UINT32_MAX && indices.present_family != UINT32_MAX) 
		{
			break;
		}
	}
	AS_FREE(queue_families);
	return indices;
}

void create_surface(as_renderer* renderer, void* display_context) 
{
	const VkResult create_surface_result = glfwCreateWindowSurface(renderer->instance, display_context, NULL, &renderer->surface);
	AS_ASSERT(create_surface_result == VK_SUCCESS, "Unable to create a surface");
}

VkSurfaceFormatKHR choose_swap_surface_format(const VkSurfaceFormatKHR* formats, u32 format_count)
{
	for (u32 i = 0; i < format_count; i++) {
		if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return formats[i];
		}
	}
	return formats[0];
}

VkPresentModeKHR choose_swap_present_mode(const VkPresentModeKHR* present_modes, u32 present_mode_count)
{
	for (u32 i = 0; i < present_mode_count; i++) {
		if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return present_modes[i];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR* capabilities, u32 width, u32 height)
{
	if (capabilities->currentExtent.width != UINT32_MAX) 
	{
		return capabilities->currentExtent;
	}
	else 
	{
		VkExtent2D actual_extent = { width, height };
		actual_extent.width = fmax(capabilities->minImageExtent.width, fmin(capabilities->maxImageExtent.width, actual_extent.width));
		actual_extent.height = fmax(capabilities->minImageExtent.height, fmin(capabilities->maxImageExtent.height, actual_extent.height));
		return actual_extent;
	}
}

void create_swap_chain(as_renderer* renderer, void* display_context)
{
	// Query swap chain support details
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer->physical_device, renderer->surface, &capabilities);

	u32 format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->physical_device, renderer->surface, &format_count, NULL);

	VkSurfaceFormatKHR* formats = AS_MALLOC(format_count * sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->physical_device, renderer->surface, &format_count, formats);

	u32 present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->physical_device, renderer->surface, &present_mode_count, NULL);

	VkPresentModeKHR* present_modes = AS_MALLOC(present_mode_count * sizeof(VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->physical_device, renderer->surface, &present_mode_count, present_modes);

	// Choose swap chain details
	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(formats, format_count);
	VkPresentModeKHR present_mode = choose_swap_present_mode(present_modes, present_mode_count);
	i32 display_width = 0;
	i32 display_height = 0;
	glfwGetWindowSize(display_context, &display_width, &display_height);
	VkExtent2D extent = choose_swap_extent(&capabilities, display_width, display_height);

	// Determine the number of images in the swap chain
	u32 image_count = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) 
	{
		image_count = capabilities.maxImageCount;
	}

	// Create swap chain
	VkSwapchainCreateInfoKHR create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = renderer->surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = find_queue_families(renderer->physical_device, renderer->surface);
	u32 queue_family_indices[] = { indices.graphics_family, indices.present_family };

	if (indices.graphics_family != indices.present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	}
	else 
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0; // Optional
		create_info.pQueueFamilyIndices = NULL; // Optional
	}

	create_info.preTransform = capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	const VkResult create_swap_chain_result = vkCreateSwapchainKHR(renderer->device, &create_info, NULL, &renderer->swap_chain);
	AS_ASSERT(create_swap_chain_result == VK_SUCCESS, "Failed to create swap chain");

	// Get swap chain images
	vkGetSwapchainImagesKHR(renderer->device, renderer->swap_chain, &image_count, NULL);
	renderer->swap_chain_images = AS_MALLOC(image_count * sizeof(VkImage));
	vkGetSwapchainImagesKHR(renderer->device, renderer->swap_chain, &image_count, renderer->swap_chain_images);

	renderer->swap_chain_image_format = surface_format.format;
	renderer->swap_chain_extent = extent;

	AS_FREE(formats);
	AS_FREE(present_modes);
}

void create_render_pass(as_renderer* renderer)
{
	VkAttachmentDescription color_attachment = { 0 };
	color_attachment.format = renderer->swap_chain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = { 0 };
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = { 0 };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_info = { 0 };
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	VkResult result = vkCreateRenderPass(renderer->device, &render_pass_info, NULL, &renderer->render_pass);
	AS_ASSERT(result == VK_SUCCESS, "Failed to create render pass");
}

const char* read_shader_code(const char* filename, const u8 shader_type)
{
	size_t file_size = 0;
	char* source_code = read_file(filename, &file_size);
	if (!source_code)
	{
		return NULL;
	}

	const char* entry_point = "main";  

	char* spirv_code;
	size_t spirv_size;
	i32 compile_result = as_compile_shader(source_code, entry_point, shader_type, &spirv_code, &spirv_size);
	free(source_code); 

	if (compile_result != 0)
	{
		// Handle shader compilation error
		return NULL;
	}

	char* spirv_string = (char*)AS_MALLOC(spirv_size + 1);
	memcpy(spirv_string, spirv_code, spirv_size);
	spirv_string[spirv_size] = '\0';

	free(spirv_code);
	return spirv_string;
}

VkShaderModule create_shader_module(VkDevice device, const char* code)
{
	VkShaderModuleCreateInfo create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = strlen(code);
	create_info.pCode = (const uint32_t*)code;

	VkShaderModule shader_module;
	VkResult result = vkCreateShaderModule(device, &create_info, NULL, &shader_module);
	AS_ASSERT(result == VK_SUCCESS, "Failed to create shader module");

	return shader_module;
}

void cleanup_shader_module(VkDevice device, VkShaderModule shader_module)
{
	vkDestroyShaderModule(device, shader_module, NULL);
}

void create_graphics_pipeline(as_renderer* renderer)
{
	// Load shader code from files or provide the shader code here
	const char* vertex_shader_code = read_shader_code("your_vertex_shader_code.glsl", AS_SHADER_TYPE_VERTEX);
	const char* fragment_shader_code = read_shader_code("your_fragment_shader_code.glsl", AS_SHADER_TYPE_FRAGMENT);

	// Create shader modules
	VkShaderModule vertex_shader_module = create_shader_module(renderer->device, vertex_shader_code);
	VkShaderModule fragment_shader_module = create_shader_module(renderer->device, fragment_shader_code);

	VkPipelineVertexInputStateCreateInfo vertex_input_info = { 0 };
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0; // Optional
	vertex_input_info.pVertexBindingDescriptions = NULL; // Optional
	vertex_input_info.vertexAttributeDescriptionCount = 0; // Optional
	vertex_input_info.pVertexAttributeDescriptions = NULL; // Optional

	VkPipelineInputAssemblyStateCreateInfo input_assembly = { 0 };
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_state = { 0 };
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = NULL; // Dynamic viewport state
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = NULL; // Dynamic scissor state

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

	VkPipelineColorBlendStateCreateInfo color_blending = { 0 };
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = NULL; // Use default blending state
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_info = { 0 };
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0; // Optional
	pipeline_layout_info.pSetLayouts = NULL; // Optional
	pipeline_layout_info.pushConstantRangeCount = 0; // Optional
	pipeline_layout_info.pPushConstantRanges = NULL; // Optional
	AS_ASSERT(
		vkCreatePipelineLayout(renderer->device, &pipeline_layout_info, NULL, &renderer->pipeline_layout) == VK_SUCCESS,
		"Failed to create pipeline layout"
	);

	VkGraphicsPipelineCreateInfo pipeline_info = { 0 };
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 0; // Number of shader stages
	pipeline_info.pStages = NULL; // Array of shader stages
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.layout = renderer->pipeline_layout;
	pipeline_info.renderPass = renderer->render_pass;
	pipeline_info.subpass = 0; // Index of the subpass in the render pass

	const VkResult create_graphics_pipeline_result = vkCreateGraphicsPipelines(renderer->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &renderer->graphics_pipeline);
	AS_ASSERT(create_graphics_pipeline_result == VK_SUCCESS, "Failed to create graphics pipeline");

	// Clean up shader modules
	cleanup_shader_module(renderer->device, vertex_shader_module);
	cleanup_shader_module(renderer->device, fragment_shader_module);

	// Free shader code strings
	free((void*)vertex_shader_code);
	free((void*)fragment_shader_code);
}

void create_command_pool(as_renderer* renderer) 
{
	// Implement command pool creation logic here
}

void as_renderer_create(as_renderer* renderer, void* display_context)
{
	// Create Vulkan instance
	VkApplicationInfo app_info = {0};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Abstract Shader Engine";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Abstract Shader Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledExtensionCount = instance_extensions_count;
	create_info.ppEnabledExtensionNames = instance_extensions;
	VkDebugUtilsMessengerCreateInfoEXT debug_create_info = { 0 };
	populate_debug_messenger_create_info(&debug_create_info);
	if (AS_USE_VULKAN_VALIDATION_LAYER)
	{
		create_info.enabledLayerCount = sizeof(validation_layers) / sizeof(validation_layers[0]);
		create_info.ppEnabledLayerNames = validation_layers;
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	}
	AS_ASSERT(vkCreateInstance(&create_info, NULL, &renderer->instance) == VK_SUCCESS, "Failed to create Vulkan instance");

	if (AS_USE_VULKAN_VALIDATION_LAYER)
	{
		const VkResult create_debug_utils_messenger_result = create_debug_utils_messenger_EXT(renderer->instance, &debug_create_info, NULL, &debug_messenger);
		AS_ASSERT(create_debug_utils_messenger_result == VK_SUCCESS, "Failed to set up Vulkan debug messenger");
	}

	// Pick a physical device
	u32 device_count = 0;
	vkEnumeratePhysicalDevices(renderer->instance, &device_count, NULL);
	AS_ASSERT(device_count > 0, "Failed to find GPUs with Vulkan support\n");

	VkPhysicalDevice* devices = (VkPhysicalDevice*)AS_MALLOC(device_count * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(renderer->instance, &device_count, devices);

	for (u32 i = 0; i < device_count; i++) 
	{
		if (is_device_suitable(devices[i]))
		{
			renderer->physical_device = devices[i];
			break;
		}
	}
	AS_FREE(devices);
	AS_ASSERT(renderer->physical_device, "Failed to find a suitable GPU");

	create_surface(renderer, display_context);

	// Create logical device and queues
	QueueFamilyIndices indices = find_queue_families(renderer->physical_device, renderer->surface);

	VkDeviceQueueCreateInfo queue_create_info = {0};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = indices.graphics_family;
	queue_create_info.queueCount = 1;

	float queue_priority = 1.0f;
	queue_create_info.pQueuePriorities = &queue_priority;

	VkPhysicalDeviceFeatures device_features = {0};

	VkDeviceCreateInfo device_create_info = {0};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = &queue_create_info;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pEnabledFeatures = &device_features;
	device_create_info.enabledExtensionCount = device_extensions_count;
	device_create_info.ppEnabledExtensionNames = device_extensions;
	const VkResult create_device_result = vkCreateDevice(renderer->physical_device, &device_create_info, NULL, &renderer->device);
	AS_ASSERT(create_device_result == VK_SUCCESS, "Failed to create logical device");

	vkGetDeviceQueue(renderer->device, indices.graphics_family, 0, &renderer->graphics_queue);
	vkGetDeviceQueue(renderer->device, indices.present_family, 0, &renderer->present_queue);


	// Create swap chain, render pass, graphics pipeline, etc.
	create_swap_chain(renderer, display_context);
	create_render_pass(renderer);
	create_graphics_pipeline(renderer);
	create_command_pool(renderer);

	// Other initialization steps as needed

	// Note: This is a simplified example, and error handling is minimal.
	// In a real-world application, you would need to add proper error checking and handle cleanup.
}