#ifndef _VULKAN_CREATE_INFO_
#define _VULKAN_CREATE_INFO_


#include "render/vulkan.h"

/**
 * This file contains all the Vulkan create infos.
 * In order to create OP_RESULT <func> (<params> , RESULT* outResult
 * 
 * Note that many Vulkan types are pointers' typedef. 
 * Therefore, it's needless to have them as a pointer here.
 */

namespace as
{
	namespace vk
	{
		struct instance_create_info
		{
			char app_name[255] = "Abstract Shader Engine";
			char engine_name[255] = "Abstract Shader Engine";
			u8 enable_validation_layers : 1;
			const std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" }; // TODO: remove std
			instance_create_info() : enable_validation_layers(true) {};
		};

		struct surface_create_info
		{
			VkInstance instance;
			GLFWwindow* window = nullptr;
		};

		struct physical_device_create_info
		{
			VkInstance instance;
			VkSurfaceKHR* surface;
		};

		struct logical_device_create_info
		{
			VkPhysicalDevice physical_device;
			VkSurfaceKHR* surface;
			std::vector<const char*> extensions;
			std::vector<const char*> validation_layers;
		};

		struct swapchain_create_info
		{
			VkDevice logical_device;
			VkPhysicalDevice physical_device;
			VkSurfaceKHR* surface;
			GLFWwindow* window;
		};

		struct image_create_info
		{
			VkPhysicalDevice physical_device;
			VkDevice logical_device;
			u32 width; 
			u32 height; 
			u32 mip_levels;
			VkSampleCountFlagBits numSamples;
			VkFormat format;
			VkImageTiling tiling;
			VkImageUsageFlags usage;
			VkMemoryPropertyFlags properties;
		};

		struct image_view_create_info
		{
			VkDevice logical_device;
			VkImage image;
			VkFormat format;
			VkImageAspectFlags aspect_flags;
			u32 mip_levels;
		};

		struct render_pass_create_info
		{
			VkDevice logical_device;
			VkPhysicalDevice physical_device;
			VkFormat swap_chain_image_format; 
			VkSampleCountFlagBits msaa_samples; 			
		};

		struct pipeline_create_info
		{
			VkDevice logical_device;
			VkRenderPass render_pass;
			VkDescriptorSetLayout descriptor_set_layout;
			VkSampleCountFlagBits msaa_samples;
			std::vector<char> vert_shader_spv;
			std::vector<char> frag_shader_spv;
		};

		struct command_pool_create_info
		{
			VkPhysicalDevice physical_device;
			VkDevice logical_device;
			VkSurfaceKHR* surface;
		};
	};
};

#endif //_VULKAN_CREATE_INFO_