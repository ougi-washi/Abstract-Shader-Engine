
#include "vulkan_core.h"
//#include "shader/shaderc_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <set> // TODO : remove
#include <algorithm> // TODO: remove
#include <array> // TODO: remove
#include <fstream> // TODO: remove

//IMAGE
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif //STB_IMAGE_IMPLEMENTATION

//TINYOBJ
#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#endif // TINYOBJLOADER_IMPLEMENTATION


VkResult as::vk::create_vulkan_instance(VkInstance& out_instance, const instance_create_info& instance_create_info)
{
	if (instance_create_info.enable_validation_layers && !check_layers_support(instance_create_info.validation_layers))
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
	createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (instance_create_info.enable_validation_layers) 
	{
		createInfo.enabledLayerCount = static_cast<u32>(instance_create_info.validation_layers.size());
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

VkResult as::vk::create_surface(VkSurfaceKHR& out_surface, const VkInstance& instance, GLFWwindow* window)
{
	return glfwCreateWindowSurface(instance, window, nullptr, &out_surface);
}

VkResult as::vk::create_surface(const surface_create_info& create_info, VkSurfaceKHR& out_surface)
{
	return glfwCreateWindowSurface(create_info.instance, create_info.window, nullptr, &out_surface);
}

as::vk::QueueFamilyIndices as::vk::find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR* surface)
{
	QueueFamilyIndices indices;

	u32 queueFamilyCount = 0;
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

bool as::vk::check_device_extension_support(VkPhysicalDevice& physical_device, const std::vector<const char*> extensions)
{
	u32 extensionCount;
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

as::vk::SwapChainSupportDetails as::vk::query_swap_chain_support(VkPhysicalDevice physical_device, VkSurfaceKHR* surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, *surface, &details.capabilities);

	u32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *surface, &formatCount, details.formats.data());
	}

	u32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool as::vk::is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR* surface)
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

VkSampleCountFlagBits as::vk::get_max_usable_sample_count(VkPhysicalDevice* physical_device)
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

void as::vk::pick_physical_device(VkPhysicalDevice* out_physical_device, VkSampleCountFlagBits* out_msaa_samples, VkInstance* instance, VkSurfaceKHR* surface)
{
	u32 deviceCount = 0;
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

bool as::vk::pick_physical_device(const physical_device_create_info& create_info, VkPhysicalDevice& out_physical_device, VkSampleCountFlagBits& out_msaa_samples)
{
	u32 deviceCount = 0;
	vkEnumeratePhysicalDevices(create_info.instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		AS_LOG(LV_ERROR, "failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(create_info.instance, &deviceCount, devices.data());

	for (auto& device : devices)
	{
		if (is_device_suitable(device, create_info.surface))
		{
			out_physical_device = device;
			out_msaa_samples = get_max_usable_sample_count(&device);
			break;
		}
	}

	if (out_physical_device == VK_NULL_HANDLE)
	{
		AS_LOG(LV_ERROR, "failed to find a suitable GPU!");
		return false;
	}
	return true;
}

void as::vk::create_logical_device(const logical_device_create_info& create_info, VkDevice* out_logical_device, VkQueue* out_graphics_queue, VkQueue* out_present_queue)
{
	QueueFamilyIndices indices = find_queue_families(create_info.physical_device, create_info.surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<u32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (u32 queueFamily : uniqueQueueFamilies)
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

	createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<u32>(create_info.extensions.size());
	createInfo.ppEnabledExtensionNames = create_info.extensions.data();

	if (create_info.validation_layers.size() > 0)
	{
		createInfo.enabledLayerCount = static_cast<u32>(create_info.validation_layers.size());
		createInfo.ppEnabledLayerNames = create_info.validation_layers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(create_info.physical_device, &createInfo, nullptr, out_logical_device) != VK_SUCCESS)
	{
		AS_LOG(LV_ERROR, "failed to create logical device!");
	}

	vkGetDeviceQueue(*out_logical_device, indices.graphicsFamily.value(), 0, out_graphics_queue);
	vkGetDeviceQueue(*out_logical_device, indices.presentFamily.value(), 0, out_present_queue);
}

VkSurfaceFormatKHR as::vk::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR as::vk::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D as::vk::choos_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<u32>(width),
			static_cast<u32>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void as::vk::create_swap_chain(VkSwapchainKHR* out_swap_chain, std::vector<VkImage>* out_swap_chain_images, VkFormat* out_swap_chain_image_format, VkExtent2D* out_swap_chain_extent, VkDevice* logical_device, VkPhysicalDevice* physical_device, VkSurfaceKHR* surface, GLFWwindow* window)
{
	SwapChainSupportDetails swapChainSupport = query_swap_chain_support(*physical_device, surface);

	VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.formats);
	VkPresentModeKHR presentMode = choose_swap_present_mode(swapChainSupport.presentModes);
	VkExtent2D extent = choos_swap_extent(swapChainSupport.capabilities, window);

	u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
	{
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
	u32 queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
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

void as::vk::create_swapchain(const swapchain_create_info& create_info, swapchain& out_swapchain)
{
	SwapChainSupportDetails swapChainSupport = query_swap_chain_support(create_info.physical_device, create_info.surface);

	VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.formats);
	VkPresentModeKHR presentMode = choose_swap_present_mode(swapChainSupport.presentModes);
	VkExtent2D extent = choos_swap_extent(swapChainSupport.capabilities, create_info.window);

	u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = *create_info.surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = find_queue_families(create_info.physical_device, create_info.surface);
	u32 queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	CHECK_VK_RESULT(vkCreateSwapchainKHR(create_info.logical_device, &createInfo, nullptr, &out_swapchain.swapchainKHR));

	vkGetSwapchainImagesKHR(create_info.logical_device, out_swapchain.swapchainKHR, &imageCount, nullptr);
	out_swapchain.images.resize(imageCount);
	vkGetSwapchainImagesKHR(create_info.logical_device, out_swapchain.swapchainKHR, &imageCount, out_swapchain.images.data());

	out_swapchain.image_format = surfaceFormat.format;
	out_swapchain.extent = extent;
}

void as::vk::cleanup_swap_chain(VkDevice& logical_device, VkSwapchainKHR& swap_chain, std::vector<image_data>& images_data, std::vector<VkFramebuffer>& frame_buffers, std::vector<VkImageView>& swap_chain_image_views)
{
	for (auto current_image_data : images_data)
	{
		vkDestroyImageView(logical_device, current_image_data.view, nullptr);
		vkDestroyImage(logical_device, current_image_data.image, nullptr);
		vkFreeMemory(logical_device, current_image_data.memory, nullptr);
	}

	for (auto frame_buffer : frame_buffers) 
	{
		vkDestroyFramebuffer(logical_device, frame_buffer, nullptr);
	}

	for (auto image_view : swap_chain_image_views) 
	{
		vkDestroyImageView(logical_device, image_view, nullptr);
	}

	vkDestroySwapchainKHR(logical_device, swap_chain, nullptr);
}

void as::vk::create_image(VkPhysicalDevice& physical_device, VkDevice& logical_device, u32 width, u32 height, u32 mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
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

VkResult as::vk::create_image(const image_create_info& create_info, image_data& out_image_data)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = create_info.width;
	imageInfo.extent.height = create_info.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = create_info.mip_levels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = create_info.format;
	imageInfo.tiling = create_info.tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = create_info.usage;
	imageInfo.samples = create_info.num_samples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	CHECK_VK_RESULT(vkCreateImage(create_info.logical_device, &imageInfo, nullptr, &out_image_data.image));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(create_info.logical_device, out_image_data.image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = find_memory_type(create_info.physical_device, memRequirements.memoryTypeBits, create_info.properties);

	CHECK_VK_RESULT(vkAllocateMemory(create_info.logical_device, &allocInfo, nullptr, &out_image_data.memory));

	return vkBindImageMemory(create_info.logical_device, out_image_data.image, out_image_data.memory, 0);
}

u32 as::vk::find_memory_type(VkPhysicalDevice physical_device, u32 typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

	for (u32 i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	AS_LOG(LV_ERROR, "failed to find suitable memory type!");
}

void as::vk::create_color_resources(VkImageView& out_image_view, VkPhysicalDevice& physical_device, VkDevice& logical_device, VkImage& image, VkDeviceMemory& image_memory, VkFormat& swap_chain_image_format, VkExtent2D& swap_chain_extent, VkSampleCountFlagBits& msaa_samples)
{
	VkFormat colorFormat = swap_chain_image_format;
	create_image(physical_device, logical_device, swap_chain_extent.width, swap_chain_extent.height, 1, msaa_samples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, image_memory);
	out_image_view = create_image_view(&logical_device, image, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void as::vk::create_depth_resources(VkImageView& out_image_view, VkPhysicalDevice& physical_device, VkDevice& logical_device, VkImage& image, VkDeviceMemory& image_memory, VkFormat& swap_chain_image_format, VkExtent2D& swap_chain_extent, VkSampleCountFlagBits& msaa_samples)
{
	VkFormat depthFormat = find_depth_format(physical_device);
	create_image(physical_device, logical_device, swap_chain_extent.width, swap_chain_extent.height, 1, msaa_samples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, image_memory);
	out_image_view = create_image_view(&logical_device, image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

VkImageView as::vk::create_image_view(VkDevice* logical_device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, u32 mipLevels)
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

VkResult as::vk::create_image_view(const image_view_create_info& create_info, VkImageView& image_view)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = create_info.image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = create_info.format;
	viewInfo.subresourceRange.aspectMask = create_info.aspect_flags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = create_info.mip_levels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	return vkCreateImageView(create_info.logical_device, &viewInfo, nullptr, &image_view);
}

void as::vk::create_image_views(std::vector<VkImageView>* swap_chain_image_views, std::vector<VkFramebuffer>* swap_chain_framebuffers, std::vector<VkImage>* swap_chain_images, VkFormat* swap_chain_image_format, VkDevice* logical_device)
{
	swap_chain_image_views->resize(swap_chain_images->size());
	for (u32 i = 0; i < swap_chain_images->size(); i++)
	{
		(*swap_chain_image_views)[i] = create_image_view(logical_device, (*swap_chain_images)[i], *swap_chain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

VkResult as::vk::create_render_pass(VkFormat& swap_chain_image_format, VkSampleCountFlagBits& msaa_samples, VkRenderPass& render_pass, VkDevice& logical_device, VkPhysicalDevice& physical_device)
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
	renderPassInfo.attachmentCount = static_cast<u32>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	return vkCreateRenderPass(logical_device, &renderPassInfo, nullptr, &render_pass);
}

VkResult as::vk::create_render_pass(const render_pass_create_info& create_info, VkRenderPass& out_render_pass)
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = create_info.swap_chain_image_format;
	colorAttachment.samples = create_info.msaa_samples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = find_depth_format(create_info.physical_device);
	depthAttachment.samples = create_info.msaa_samples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = create_info.swap_chain_image_format;
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
	renderPassInfo.attachmentCount = static_cast<u32>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	return vkCreateRenderPass(create_info.logical_device, &renderPassInfo, nullptr, &out_render_pass);
}

VkFormat as::vk::find_supported_format(VkPhysicalDevice& physical_device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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

VkFormat as::vk::find_depth_format(VkPhysicalDevice physical_device)
{
	return find_supported_format(physical_device,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkResult as::vk::create_descriptor_set_layout(VkDevice& logical_device, VkDescriptorSetLayout& out_descriptor_set_layout)
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
	layoutInfo.bindingCount = static_cast<u32>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	return vkCreateDescriptorSetLayout(logical_device, &layoutInfo, nullptr, &out_descriptor_set_layout);
}

VkResult as::vk::create_pipeline(const pipeline_create_info& create_info, pipeline_data& out_pipeline)
{
	VkShaderModule vertShaderModule = create_shader_module(create_info.vert_shader_spv, create_info.logical_device);
	VkShaderModule fragShaderModule = create_shader_module(create_info.frag_shader_spv, create_info.logical_device);

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

	auto binding_description = vertex::get_binding_description();
	auto attribute_descriptions = vertex::get_attribute_descriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(attribute_descriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &binding_description;
	vertexInputInfo.pVertexAttributeDescriptions = attribute_descriptions.data();

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
	multisampling.rasterizationSamples = create_info.msaa_samples;

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
	dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &create_info.descriptor_set_layout;

	CHECK_VK_RESULT(vkCreatePipelineLayout(create_info.logical_device, &pipelineLayoutInfo, nullptr, &out_pipeline.layout));

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
	pipelineInfo.layout = out_pipeline.layout;
	pipelineInfo.renderPass = create_info.render_pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkResult pipeline_creation_result = vkCreateGraphicsPipelines(create_info.logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &out_pipeline.pipeline);

	vkDestroyShaderModule(create_info.logical_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(create_info.logical_device, vertShaderModule, nullptr);
	
	return pipeline_creation_result;
}

VkShaderModule as::vk::create_shader_module(const std::vector<char>& code, VkDevice logical_device)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const u32*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logical_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

VkResult as::vk::create_command_pool(VkCommandPool& out_command_pool, VkPhysicalDevice& physical_device, const VkDevice& logical_device, VkSurfaceKHR& surface)
{
	QueueFamilyIndices queueFamilyIndices = find_queue_families(physical_device, &surface);
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	return vkCreateCommandPool(logical_device, &poolInfo, nullptr, &out_command_pool);
}

VkResult as::vk::create_command_pool(const command_pool_create_info& create_info, VkCommandPool& out_command_pool)
{
	QueueFamilyIndices queueFamilyIndices = find_queue_families(create_info.physical_device, create_info.surface);
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	return vkCreateCommandPool(create_info.logical_device, &poolInfo, nullptr, &out_command_pool);
}

void as::vk::create_frame_buffers(std::vector<VkFramebuffer>& out_swap_chain_framebuffers, VkDevice& logical_device, std::vector<VkImageView>& swap_chain_image_views, VkImageView& color_image_view, VkImageView& depth_image_view, VkRenderPass& render_pass, VkExtent2D& swap_chain_extent)
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
		framebufferInfo.attachmentCount = static_cast<u32>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swap_chain_extent.width;
		framebufferInfo.height = swap_chain_extent.height;
		framebufferInfo.layers = 1;

		CHECK_VK_RESULT(vkCreateFramebuffer(logical_device, &framebufferInfo, nullptr, &out_swap_chain_framebuffers[i]));
	}
}

void as::vk::create_framebuffers(const framebuffers_create_info& create_info, std::vector<VkFramebuffer>& out_swap_chain_framebuffers)
{
	out_swap_chain_framebuffers.resize(create_info.swap_chain_image_views.size());

	for (size_t i = 0; i < create_info.swap_chain_image_views.size(); i++) 
	{
		std::array<VkImageView, 3> attachments =
		{
			create_info.color_image_view,
			create_info.depth_image_view,
			create_info.swap_chain_image_views[i]
		};
		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = create_info.render_pass;
		framebuffer_info.attachmentCount = static_cast<u32>(attachments.size());
		framebuffer_info.pAttachments = attachments.data();
		framebuffer_info.width = create_info.swap_chain_extent.width;
		framebuffer_info.height = create_info.swap_chain_extent.height;
		framebuffer_info.layers = 1;

		CHECK_VK_RESULT(vkCreateFramebuffer(create_info.logical_device, &framebuffer_info, nullptr, &out_swap_chain_framebuffers[i]));
	}
}

void as::vk::create_texture_image(VkImage& out_texture_image, const char* texture_path, u32& mip_levels, VkPhysicalDevice& physical_device, VkDevice& logical_device, VkCommandPool& command_pool, VkQueue& graphics_queue, VkDeviceMemory& texture_image_memory)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texture_path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	mip_levels = static_cast<u32>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!pixels) 
	{
		AS_LOG(LV_ERROR, "Could not load the image");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CHECK_VK_RESULT(create_buffer(stagingBuffer, physical_device, logical_device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferMemory));
	

	void* data;
	vkMapMemory(logical_device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(logical_device, stagingBufferMemory);

	stbi_image_free(pixels);

	create_image(physical_device, logical_device, texWidth, texHeight, mip_levels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, out_texture_image, texture_image_memory);

	transition_image_layout(logical_device, command_pool, graphics_queue, out_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);
	copy_buffer_to_image(logical_device, command_pool, graphics_queue, stagingBuffer, out_texture_image, static_cast<u32>(texWidth), static_cast<u32>(texHeight));
	//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

	vkDestroyBuffer(logical_device, stagingBuffer, nullptr);
	vkFreeMemory(logical_device, stagingBufferMemory, nullptr);

	generate_mipmaps(physical_device, logical_device, command_pool, graphics_queue, out_texture_image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mip_levels);
}

void as::vk::create_texture_image(const texture_image_create_info& create_info, texture_data& out_texture_data)
{
	i32 tex_width, tex_height, tex_channels;
	stbi_uc* pixels = stbi_load(create_info.texture_path, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
	VkDeviceSize imageSize = tex_width * tex_height * 4;
	out_texture_data.mip_levels = static_cast<u32>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;

	if (pixels)
	{
		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;
		as::vk::buffer_create_info buffer_create_info;
		buffer_create_info.logical_device = create_info.logical_device;
		buffer_create_info.physical_device = create_info.physical_device;
		buffer_create_info.size = imageSize;
		buffer_create_info.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		create_buffer(buffer_create_info, staging_buffer, staging_buffer_memory);

		void* data;
		vkMapMemory(create_info.logical_device, staging_buffer_memory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(create_info.logical_device, staging_buffer_memory);
		stbi_image_free(pixels);


		as::vk::image_create_info image_create_info;
		image_create_info.physical_device = create_info.physical_device;
		image_create_info.logical_device = create_info.logical_device;
		image_create_info.height = tex_height;
		image_create_info.width = tex_width;
		image_create_info.mip_levels = out_texture_data.mip_levels;
		image_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
		image_create_info.num_samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		image_create_info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		create_image(image_create_info, out_texture_data.image_data);

		transition_image_layout_info transition_info;
		transition_info.logical_device = create_info.logical_device;
		transition_info.graphics_queue = create_info.graphics_queue;
		transition_info.command_pool = create_info.command_pool;
		transition_info.mip_levels = out_texture_data.mip_levels;
		transition_info.format = VK_FORMAT_R8G8B8A8_SRGB;
		transition_info.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		transition_info.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		transition_info.image = out_texture_data.image_data.image;
		transition_image_layout(transition_info);

		as::vk::copy_buffer_to_image_info copy_buffer_to_image_info;
		copy_buffer_to_image_info.logical_device = create_info.logical_device;
		copy_buffer_to_image_info.command_pool = create_info.command_pool;
		copy_buffer_to_image_info.graphics_queue = create_info.graphics_queue;
		copy_buffer_to_image_info.buffer = staging_buffer;
		copy_buffer_to_image_info.image = out_texture_data.image_data.image;
		copy_buffer_to_image_info.height = static_cast<u32>(tex_height);
		copy_buffer_to_image_info.width = static_cast<u32>(tex_width);
		copy_buffer_to_image(copy_buffer_to_image_info);
		//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		vkDestroyBuffer(create_info.logical_device, staging_buffer, nullptr);
		vkFreeMemory(create_info.logical_device, staging_buffer_memory, nullptr);

		as::vk::generate_mipmaps_info generate_mipmaps_info;
		generate_mipmaps_info.physical_device = create_info.physical_device;
		generate_mipmaps_info.logical_device = create_info.logical_device;
		generate_mipmaps_info.command_pool = create_info.command_pool;
		generate_mipmaps_info.queue = create_info.graphics_queue;
		generate_mipmaps_info.imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
		generate_mipmaps_info.image = out_texture_data.image_data.image;
		generate_mipmaps_info.mip_levels = out_texture_data.mip_levels;
		generate_mipmaps_info.tex_height = tex_height;
		generate_mipmaps_info.tex_width = tex_width;
		generate_mipmaps(generate_mipmaps_info);
	}
	else
	{
		AS_LOG(LV_WARNING, "Could not load the image");
	}
}

void as::vk::create_texture_image_view(VkImageView& out_texture_image_view, VkDevice& logical_device, VkImage& image, const u32& mip_levels)
{
	out_texture_image_view = create_image_view(&logical_device, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mip_levels);
}

VkResult as::vk::create_texture_sampler(VkSampler& out_texture_sampler, VkPhysicalDevice& physical_device, VkDevice& logical_device, const u32& mip_levels)
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physical_device, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mip_levels);
	samplerInfo.mipLodBias = 0.0f;

	return vkCreateSampler(logical_device, &samplerInfo, nullptr, &out_texture_sampler);
}

VkResult as::vk::create_sampler(const sampler_create_info& create_info, VkSampler& out_sampler)
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(create_info.physical_device, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(create_info.mip_levels);
	samplerInfo.mipLodBias = 0.0f;

	return vkCreateSampler(create_info.logical_device, &samplerInfo, nullptr, &out_sampler);
}

VkResult as::vk::create_buffer(VkBuffer& out_buffer, VkPhysicalDevice& physical_device, VkDevice& logical_device, VkDeviceSize& size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& buffer_memory)
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

	VkResult allocation_result = vkAllocateMemory(logical_device, &allocInfo, nullptr, &buffer_memory);
	VkResult binding_result = vkBindBufferMemory(logical_device, out_buffer, buffer_memory, 0);
	if (allocation_result != VK_SUCCESS)
	{
		return allocation_result;
	}
	if (binding_result != VK_SUCCESS)
	{
		return binding_result;
	}
	return VK_SUCCESS;
}

VkResult as::vk::create_buffer(buffer_create_info& create_info, VkBuffer& out_buffer, VkDeviceMemory& out_memory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = create_info.size;
	bufferInfo.usage = create_info.usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	CHECK_VK_RESULT(vkCreateBuffer(create_info.logical_device, &bufferInfo, nullptr, &out_buffer));
	
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(create_info.logical_device, out_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = find_memory_type(create_info.physical_device, memRequirements.memoryTypeBits, create_info.properties);

	VkResult allocation_result = vkAllocateMemory(create_info.logical_device, &allocInfo, nullptr, &out_memory);
	VkResult binding_result = vkBindBufferMemory(create_info.logical_device, out_buffer, out_memory, 0);
	if (allocation_result != VK_SUCCESS)
	{
		return allocation_result;
	}
	if (binding_result != VK_SUCCESS)
	{
		return binding_result;
	}
	return VK_SUCCESS;
}

void as::vk::transition_image_layout(VkDevice& logical_device, VkCommandPool& command_pool, VkQueue& graphics_queue, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, u32 mipLevels)
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

void as::vk::transition_image_layout(transition_image_layout_info& info)
{
	VkCommandBuffer commandBuffer = begin_single_time_commands(info.logical_device, info.command_pool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = info.old_layout;
	barrier.newLayout = info.new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = info.image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = info.mip_levels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (info.old_layout == VK_IMAGE_LAYOUT_UNDEFINED && info.new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (info.old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && info.new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
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

	end_single_time_commands(info.logical_device, info.command_pool, commandBuffer, info.graphics_queue);
}

VkCommandBuffer as::vk::begin_single_time_commands(VkDevice logical_device, VkCommandPool command_pool)
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

void as::vk::end_single_time_commands(VkDevice& logical_device, VkCommandPool& command_pool, VkCommandBuffer command_buffer, VkQueue& graphics_queue)
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

void as::vk::copy_buffer_to_image(VkDevice& logical_device, VkCommandPool& command_pool, VkQueue& graphics_queue, VkBuffer buffer, VkImage image, u32 width, u32 height)
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

void as::vk::copy_buffer_to_image(copy_buffer_to_image_info& info)
{
	VkCommandBuffer commandBuffer = begin_single_time_commands(info.logical_device, info.command_pool);

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
		info.width,
		info.height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, info.buffer, info.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	end_single_time_commands(info.logical_device, info.command_pool, commandBuffer, info.graphics_queue);
}

void as::vk::generate_mipmaps(VkPhysicalDevice& physical_device, VkDevice& logical_device, VkCommandPool& command_pool, VkQueue& queue, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, u32 mip_levels)
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

	for (u32 i = 1; i < mip_levels; i++) {
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

void as::vk::generate_mipmaps(generate_mipmaps_info& info)
{
	// Check if image format supports linear blitting
	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(info.physical_device, info.imageFormat, &format_properties);

	if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer command_buffer = begin_single_time_commands(info.logical_device, info.command_pool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = info.image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = info.tex_width;
	int32_t mipHeight = info.tex_height;

	for (u32 i = 1; i < info.mip_levels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer,
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

		vkCmdBlitImage(command_buffer,
			info.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			info.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = info.mip_levels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(command_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	end_single_time_commands(info.logical_device, info.command_pool, command_buffer, info.queue);
}

void as::vk::load_model(const char* modle_path, std::vector<vertex>& out_vertices, std::vector<u32>& out_indices)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modle_path)) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.tex_coord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(out_vertices.size());
				out_vertices.push_back(vertex);
			}

			out_indices.push_back(uniqueVertices[vertex]);
		}
	}
}

void as::vk::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size, VkDevice& logical_device, VkCommandPool& command_pool, VkQueue& queue)
{
	VkCommandBuffer command_buffer = begin_single_time_commands(logical_device, command_pool);

	VkBufferCopy copy_region{};
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

	end_single_time_commands(logical_device, command_pool, command_buffer, queue);
}

void as::vk::copy_buffer(copy_buffer_info& info)
{
	VkCommandBuffer command_buffer = begin_single_time_commands(info.logical_device, info.command_pool);

	VkBufferCopy copy_region{};
	copy_region.size = info.size;
	vkCmdCopyBuffer(command_buffer, info.src_buffer, info.dst_buffer, 1, &copy_region);

	end_single_time_commands(info.logical_device, info.command_pool, command_buffer, info.queue);
}

void as::vk::create_vertex_buffer(VkBuffer& out_vertex_buffer, VkDeviceMemory& vertex_buffer_memory, VkPhysicalDevice& physical_device, VkDevice& logical_device, const std::vector<vertex>& vertices, VkCommandPool& command_pool, VkQueue& queue)
{
	VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	create_buffer(staging_buffer, physical_device, logical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer_memory);

	void* data;
	vkMapMemory(logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, vertices.data(), (size_t)buffer_size);
	vkUnmapMemory(logical_device, staging_buffer_memory);

	create_buffer(out_vertex_buffer, physical_device, logical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer_memory);
	 
	copy_buffer(staging_buffer, out_vertex_buffer, buffer_size, logical_device, command_pool, queue);

	vkDestroyBuffer(logical_device, staging_buffer, nullptr);
	vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
}

void as::vk::create_vertex_buffer(const vertex_buffer_create_info& create_info, VkBuffer& out_vertex_buffer, VkDeviceMemory& out_vertex_buffer_memory)
{
	VkDeviceSize buffer_size = sizeof(create_info.vertices[0]) * create_info.vertices.size();

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	as::vk::buffer_create_info buffer_create_info;
	buffer_create_info.physical_device = create_info.physical_device;
	buffer_create_info.logical_device = create_info.logical_device;
	buffer_create_info.size = buffer_size;
	buffer_create_info.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	CHECK_VK_RESULT(create_buffer(buffer_create_info, staging_buffer, staging_buffer_memory));

	void* data;
	vkMapMemory(create_info.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, create_info.vertices.data(), (size_t)buffer_size);
	vkUnmapMemory(create_info.logical_device, staging_buffer_memory);

	buffer_create_info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	CHECK_VK_RESULT(create_buffer(buffer_create_info, out_vertex_buffer, out_vertex_buffer_memory));
	
	as::vk::copy_buffer_info copy_buffer_info;
	copy_buffer_info.src_buffer = staging_buffer;
	copy_buffer_info.dst_buffer = out_vertex_buffer;
	copy_buffer_info.size = buffer_size;
	copy_buffer_info.logical_device = create_info.logical_device;
	copy_buffer_info.queue = create_info.queue;
	copy_buffer_info.command_pool = create_info.command_pool;
	copy_buffer(copy_buffer_info);

	vkDestroyBuffer(create_info.logical_device, staging_buffer, nullptr);
	vkFreeMemory(create_info.logical_device, staging_buffer_memory, nullptr);
}

void as::vk::create_index_buffer(VkBuffer& out_index_buffer, VkDeviceMemory& index_buffer_memory, VkPhysicalDevice& physical_device, VkDevice& logical_device, const std::vector<u32>& indices, VkCommandPool& command_pool, VkQueue& queue)
{
	VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	create_buffer(staging_buffer, physical_device, logical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer_memory);

	void* data;
	vkMapMemory(logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, indices.data(), (size_t)buffer_size);
	vkUnmapMemory(logical_device, staging_buffer_memory);

	create_buffer(out_index_buffer, physical_device, logical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer_memory);

	copy_buffer(staging_buffer, out_index_buffer, buffer_size, logical_device, command_pool, queue);

	vkDestroyBuffer(logical_device, staging_buffer, nullptr);
	vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
}

void as::vk::create_index_buffer(const index_buffer_create_info& create_info, VkBuffer& out_index_buffer, VkDeviceMemory& out_index_buffer_memory)
{
	VkDeviceSize buffer_size = sizeof(create_info.indices[0]) * create_info.indices.size();

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	as::vk::buffer_create_info buffer_create_info;
	buffer_create_info.physical_device = create_info.physical_device;
	buffer_create_info.logical_device = create_info.logical_device;
	buffer_create_info.size = buffer_size;
	buffer_create_info.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	CHECK_VK_RESULT(create_buffer(buffer_create_info, staging_buffer, staging_buffer_memory));

	void* data;
	vkMapMemory(create_info.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, create_info.indices.data(), (size_t)buffer_size);
	vkUnmapMemory(create_info.logical_device, staging_buffer_memory);

	buffer_create_info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	CHECK_VK_RESULT(create_buffer(buffer_create_info, out_index_buffer, out_index_buffer_memory));

	as::vk::copy_buffer_info copy_buffer_info;
	copy_buffer_info.src_buffer = staging_buffer;
	copy_buffer_info.dst_buffer = out_index_buffer;
	copy_buffer_info.size = buffer_size;
	copy_buffer_info.logical_device = create_info.logical_device;
	copy_buffer_info.queue = create_info.queue;
	copy_buffer_info.command_pool = create_info.command_pool;
	copy_buffer(copy_buffer_info);

	vkDestroyBuffer(create_info.logical_device, staging_buffer, nullptr);
	vkFreeMemory(create_info.logical_device, staging_buffer_memory, nullptr);
}

void as::vk::create_uniform_buffers(std::vector<VkBuffer>& out_uniform_buffers, std::vector<VkDeviceMemory>& out_uniform_buffers_memory, VkPhysicalDevice& physical_device, VkDevice& logical_device, const i8& max_frames_in_flight)
{
	VkDeviceSize buffer_size = sizeof(uniform_buffer_object);

	out_uniform_buffers.resize(max_frames_in_flight);
	out_uniform_buffers_memory.resize(max_frames_in_flight);

	for (i8 i = 0; i < max_frames_in_flight; i++)
	{
		create_buffer(out_uniform_buffers[i], physical_device, logical_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, out_uniform_buffers_memory[i]);
	}
}

VkResult as::vk::create_descriptor_pool(VkDevice& logical_device, const i8& max_frames_in_flight, VkDescriptorPool& descriptor_pool)
{
	std::array<VkDescriptorPoolSize, 2> pool_sizes{};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = static_cast<uint32_t>(max_frames_in_flight);
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	poolInfo.pPoolSizes = pool_sizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(max_frames_in_flight);

	return vkCreateDescriptorPool(logical_device, &poolInfo, nullptr, &descriptor_pool);
}

void as::vk::create_uniform_buffers(const uniform_buffers_create_info& create_info, std::vector<VkBuffer>& out_uniform_buffers, std::vector<VkDeviceMemory>& out_uniform_buffers_memory)
{
	out_uniform_buffers.resize(create_info.max_frames_in_flight);
	out_uniform_buffers_memory.resize(create_info.max_frames_in_flight);

	as::vk::buffer_create_info buffer_create_info;
	buffer_create_info.physical_device = create_info.physical_device;
	buffer_create_info.logical_device = create_info.logical_device;
	buffer_create_info.size = sizeof(uniform_buffer_object);
	buffer_create_info.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	for (i8 i = 0; i < create_info.max_frames_in_flight; i++)
	{
		CHECK_VK_RESULT(create_buffer(buffer_create_info, out_uniform_buffers[i], out_uniform_buffers_memory[i]));
	}
}


VkResult as::vk::create_descriptor_sets(std::vector<VkDescriptorSet>& out_descriptor_sets, VkDevice& logical_device, VkDescriptorSetLayout& descriptor_set_layout, VkDescriptorPool& descriptor_pool, const i8& max_frames_in_flight)
{
	std::vector<VkDescriptorSetLayout> layouts(max_frames_in_flight, descriptor_set_layout);
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptor_pool;
	alloc_info.descriptorSetCount = static_cast<uint32_t>(max_frames_in_flight);
	alloc_info.pSetLayouts = layouts.data();

	out_descriptor_sets.resize(max_frames_in_flight);
	return vkAllocateDescriptorSets(logical_device, &alloc_info, out_descriptor_sets.data());
}

VkResult as::vk::create_descriptor_sets(const descriptor_sets_create_info& create_info, std::vector<VkDescriptorSet>& out_descriptor_sets)
{
	std::vector<VkDescriptorSetLayout> layouts(create_info.max_frames_in_flight, create_info.descriptor_set_layout);
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = create_info.descriptor_pool;
	alloc_info.descriptorSetCount = static_cast<uint32_t>(create_info.max_frames_in_flight);
	alloc_info.pSetLayouts = layouts.data();

	out_descriptor_sets.resize(create_info.max_frames_in_flight);
	return vkAllocateDescriptorSets(create_info.logical_device, &alloc_info, out_descriptor_sets.data());
}

void as::vk::update_descriptor_sets(VkDevice& logical_device, std::vector<VkDescriptorSet>& out_descriptor_sets, std::vector<VkBuffer>& uniform_buffers, const i8& max_frames_in_flight, VkImageView& image_view, VkSampler& image_sampler)
{
	for (size_t i = 0; i < max_frames_in_flight; i++)
	{
		VkDescriptorBufferInfo buffer_info{};
		buffer_info.buffer = uniform_buffers[i];
		buffer_info.offset = 0;
		buffer_info.range = sizeof(uniform_buffer_object);

		VkDescriptorImageInfo image_info{};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = image_view;
		image_info.sampler = image_sampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = out_descriptor_sets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &buffer_info;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = out_descriptor_sets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &image_info;

		vkUpdateDescriptorSets(logical_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void as::vk::update_descriptor_sets(const descriptor_sets_update_info& update_info, std::vector<VkDescriptorSet>& out_descriptor_sets)
{
	for (size_t i = 0; i < update_info.max_frames_in_flight; i++)
	{
		VkDescriptorBufferInfo buffer_info{};
		buffer_info.buffer = update_info.uniform_buffers[i];
		buffer_info.offset = 0;
		buffer_info.range = sizeof(uniform_buffer_object);

		VkDescriptorImageInfo image_info{};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = update_info.image_view;
		image_info.sampler = update_info.image_sampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = out_descriptor_sets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &buffer_info;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = out_descriptor_sets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &image_info;

		vkUpdateDescriptorSets(update_info.logical_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

VkResult as::vk::create_command_buffers(std::vector<VkCommandBuffer>& command_buffers, VkDevice& logical_device, VkCommandPool& command_pool, const i8& max_frames_in_flight)
{
	command_buffers.resize(max_frames_in_flight);

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)command_buffers.size();

	return vkAllocateCommandBuffers(logical_device, &alloc_info, command_buffers.data());
}

VkResult as::vk::create_command_buffers(const command_buffers_create_info& create_info, std::vector<VkCommandBuffer>& command_buffers)
{
	command_buffers.resize(create_info.max_frames_in_flight);

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = create_info.command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)command_buffers.size();

	return vkAllocateCommandBuffers(create_info.logical_device, &alloc_info, command_buffers.data());
}

void as::vk::create_sync_objects(VkDevice& logical_device, std::vector<VkSemaphore>& image_available_semaphores, std::vector<VkSemaphore>& render_finished_semaphores, std::vector<VkFence>& inflight_fences, const i8& max_frames_in_flight)
{
	image_available_semaphores.resize(max_frames_in_flight);
	render_finished_semaphores.resize(max_frames_in_flight);
	inflight_fences.resize(max_frames_in_flight);

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < max_frames_in_flight; i++)
	{
		if (vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS ||
			vkCreateFence(logical_device, &fenceInfo, nullptr, &inflight_fences[i]) != VK_SUCCESS)
		{
			AS_LOG(LV_ERROR, "failed to create synchronization objects for a frame!");
		}
	}
}

void as::vk::create_sync_objects(const sync_objects_create_info& create_info, std::vector<VkSemaphore>& out_image_available_semaphores, std::vector<VkSemaphore>& out_render_finished_semaphores, std::vector<VkFence>& out_inflight_fences)
{
	out_image_available_semaphores.resize(create_info.max_frames_in_flight);
	out_render_finished_semaphores.resize(create_info.max_frames_in_flight);
	out_inflight_fences.resize(create_info.max_frames_in_flight);

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < create_info.max_frames_in_flight; i++)
	{
		CHECK_VK_RESULT(vkCreateSemaphore(create_info.logical_device, &semaphore_info, nullptr, &out_image_available_semaphores[i]));
		CHECK_VK_RESULT(vkCreateSemaphore(create_info.logical_device, &semaphore_info, nullptr, &out_render_finished_semaphores[i]));
		CHECK_VK_RESULT(vkCreateFence(create_info.logical_device, &fenceInfo, nullptr, &out_inflight_fences[i]));
	}
}

VkResult as::vk::setup_debug_messenger(VkInstance* instance, VkDebugUtilsMessengerEXT* debug_messenger)
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populate_debug_messenger_create_info(createInfo);
	return create_debug_utils_messenger_EXT(*instance, &createInfo, nullptr, debug_messenger);
}

VkResult as::vk::create_debug_utils_messenger_EXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL as::vk::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void as::vk::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

bool as::vk::check_layers_support(const std::vector<const char*> layers)
{
	u32 layerCount;
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

std::vector<const char*> as::vk::get_required_extensions(const bool& enable_validation_layers)
{
	u32 glfwExtensionCount = 0;
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
	if (!f)
	{
		AS_LOG(LV_WARNING, "File [" + std::string(filename) + "] not found, a crash may occur");
		return nullptr;
	}
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
