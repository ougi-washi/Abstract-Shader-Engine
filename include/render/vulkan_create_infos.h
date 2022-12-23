#ifndef _VULKAN_CREATE_INFO_
#define _VULKAN_CREATE_INFO_

#include "vulkan.h"

/**
 * This file contains all the Vulkan function params.
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
			VkSampleCountFlagBits num_samples;
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
			std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
			VkSampleCountFlagBits msaa_samples;
			std::vector<spv> vert_shaders;
			std::vector<spv> frag_shaders;
		};

		struct command_pool_create_info
		{
			VkPhysicalDevice physical_device;
			VkDevice logical_device;
			VkSurfaceKHR* surface;
		};

		struct framebuffers_create_info
		{
			VkDevice logical_device;
			std::vector<VkImageView> swap_chain_image_views; 
			VkImageView color_image_view;
			VkImageView depth_image_view; 
			VkRenderPass render_pass;
			VkExtent2D swap_chain_extent;
		};

		struct texture_image_create_info
		{
			char texture_path[255] = "";
			VkPhysicalDevice physical_device; 
			VkDevice logical_device; 
			VkCommandPool command_pool; 
			VkQueue graphics_queue;
			VkDeviceMemory texture_image_memory;
		};

		struct buffer_create_info
		{
			VkPhysicalDevice physical_device;
			VkDevice logical_device;
			VkDeviceSize size;
			VkBufferUsageFlags usage; 
			VkMemoryPropertyFlags properties; 
		};

		struct transition_image_layout_info
		{
			VkDevice logical_device; 
			VkCommandPool command_pool; 
			VkQueue graphics_queue;
			VkImage image; 
			VkFormat format; 
			VkImageLayout old_layout; 
			VkImageLayout new_layout; 
			u32 mip_levels;
		};

		struct copy_buffer_info
		{
			VkBuffer src_buffer; 
			VkBuffer dst_buffer; 
			VkDeviceSize size; 
			VkDevice logical_device; 
			VkCommandPool command_pool; 
			VkQueue queue;
		};

		struct copy_buffer_to_image_info
		{
			VkDevice logical_device; 
			VkCommandPool command_pool; 
			VkQueue graphics_queue; 
			VkBuffer buffer;
			VkImage image;
			u32 width;
			u32 height;
		};

		struct generate_mipmaps_info
		{
			VkPhysicalDevice physical_device; 
			VkDevice logical_device; 
			VkCommandPool command_pool; 
			VkQueue queue; 
			VkImage image; 
			VkFormat imageFormat; 
			i32 tex_width; 
			i32 tex_height;
			u32 mip_levels;
		};

		struct sampler_create_info
		{
			VkPhysicalDevice physical_device;
			VkDevice logical_device; 
			u32 mip_levels;
		};

		struct vertex_buffer_create_info
		{
			VkPhysicalDevice physical_device; 
			VkDevice logical_device;
			VkCommandPool command_pool; 
			VkQueue queue;
			std::vector<vertex> vertices;
		};

		struct index_buffer_create_info
		{
			VkPhysicalDevice physical_device;
			VkDevice logical_device;
			VkCommandPool command_pool;
			VkQueue queue;
			std::vector<u32> indices;
		};

		struct uniform_buffers_create_info
		{
			VkPhysicalDevice physical_device;
			VkDevice logical_device; 
			i8 max_frames_in_flight;
		};

		struct descriptor_sets_create_info
		{
			VkDevice logical_device; 
			VkDescriptorSetLayout descriptor_set_layout; 
			VkDescriptorPool descriptor_pool;
			i8 max_frames_in_flight;
		};

		struct descriptor_sets_update_info
		{
			VkDevice logical_device;
			std::vector<VkBuffer> uniform_buffers; 
			i8 max_frames_in_flight;
			VkImageView image_view; 
			VkSampler image_sampler;
		};

		struct command_buffers_create_info
		{
			VkDevice logical_device; 
			VkCommandPool command_pool; 
			i8 max_frames_in_flight;
		};

		struct sync_objects_create_info
		{
			VkDevice logical_device; 
			i8 max_frames_in_flight;
		};
	};
};

#endif //_VULKAN_CREATE_INFO_