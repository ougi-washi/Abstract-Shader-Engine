// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
#pragma once

#include "as_types.h"
#include "as_math.h"
#include <vulkan/vulkan.h>

typedef struct as_renderer
{
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swap_chain;
	VkFormat swap_chain_image_format;
	VkImage* swap_chain_images;
	VkExtent2D swap_chain_extent;
	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	VkCommandPool command_pool;
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
} as_renderer;

typedef struct as_object
{
	as_transform transform;
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
} as_object;

void as_renderer_create(as_renderer* renderer, void* display_context);
void as_renderer_draw_frame(as_renderer* renderer, void* display_context);
void as_renderer_destroy(as_renderer* renderer);

as_object* as_add_plane(as_renderer* renderer, const as_transform* transform);
void as_delete_object(as_renderer* renderer, as_object* object);
