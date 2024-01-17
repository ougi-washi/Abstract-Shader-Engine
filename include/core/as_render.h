// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
#pragma once

#include "as_types.h"
#include "as_math.h"
#include "as_array.h"
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2


// Arrays
AS_DECLARE_ARRAY(as_VkImages, 64, VkImage);
AS_DECLARE_ARRAY(as_VkImageViews, 64, VkImageView);
AS_DECLARE_ARRAY(as_VkFramebuffer, 64, VkFramebuffer);
AS_DECLARE_ARRAY(as_VkSemaphores, 64, VkSemaphore);
AS_DECLARE_ARRAY(as_VkFences, 64, VkFence);

typedef struct as_uniform_buffer_object
{
	as_mat4 model;
	as_mat4 view;
	as_mat4 proj;
} as_uniform_buffer_object;

typedef struct as_vertex
{
	as_vec3 pos;
	as_vec3 color;
	as_vec2 tex_coord;
} as_vertex;
#define AS_VERTEX_VAR_COUNT 3

typedef struct as_texture
{
	VkImage texture_image;
	VkDeviceMemory texture_image_memory;
	VkImageView texture_image_view;
	VkSampler texture_sampler;
} as_texture;

typedef struct as_shader
{
	VkShaderModule vertex_shader;
	VkShaderModule fragment_shader;

	VkBuffer* uniform_buffers;
	u32 uniform_buffers_count;
	VkDeviceMemory* uniform_buffers_memory;
	u32 uniform_buffers_memory_count;
	void** uniform_buffers_mapped;
	u32 uniform_buffers_mapped_count;

	VkDescriptorPool descriptor_pool;
	VkDescriptorSet* descriptor_sets;
	u32 descriptor_sets_count;
}as_shader;

typedef struct as_object
{
	as_transform transform;

	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;


} as_object;

typedef struct as_render
{
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkSurfaceKHR surface;

	VkPhysicalDevice physical_device;
	VkDevice device;

	VkQueue graphics_queue;
	VkQueue present_queue;

	VkSwapchainKHR swap_chain;
	as_VkImages swap_chain_images;
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;
	as_VkImageViews swap_chain_image_views;
	as_VkFramebuffer swap_chain_framebuffers;
	bool framebuffer_resized;

	VkRenderPass render_pass;
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;

	VkCommandPool command_pool;

	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;

	VkBuffer* uniform_buffers;
	u32 uniform_buffers_count;
	VkDeviceMemory* uniform_buffers_memory;
	u32 uniform_buffers_memory_count;
	void** uniform_buffers_mapped;
	u32 uniform_buffers_mapped_count;

	VkDescriptorPool descriptor_pool;
	VkDescriptorSet* descriptor_sets;
	u32 descriptor_sets_count;

	VkCommandBuffer* command_buffers;
	u32 command_buffers_count;

	as_VkSemaphores image_available_semaphores;
	as_VkSemaphores render_finished_semaphores;
	as_VkFences in_flight_fences;

	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;

	VkImage texture_image;
	VkDeviceMemory texture_image_memory;
	VkImageView texture_image_view;
	VkSampler texture_sampler;

	u32 current_frame;
} as_render;

void as_render_create(as_render* render, void* display_context);
void as_render_draw_frame(as_render* render, void* display_context);
void as_render_destroy(as_render* render);

as_object* as_add_object(as_render* render, const as_transform* transform, const char* vertex_shader_path, const char* fragment_shader_path);
void as_delete_object(as_render* render, as_object* object);
