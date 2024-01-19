// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
#pragma once

#include "as_types.h"
#include "as_math.h"
#include "as_array.h"
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2


// Arrays

AS_DECLARE_ARRAY(VkImages64, 64, VkImage);
AS_DECLARE_ARRAY(VkImageViews32, 32, VkImageView);
AS_DECLARE_ARRAY(VkFramebuffers32, 32, VkFramebuffer);
AS_DECLARE_ARRAY(VkSemaphores32, 32, VkSemaphore);
AS_DECLARE_ARRAY(VkFences32, 32, VkFence);
AS_DECLARE_ARRAY(VkCommandBuffers32, 32, VkCommandBuffer);
AS_DECLARE_ARRAY(VkDescriptorSets32, 32, VkDescriptorSet);
AS_DECLARE_ARRAY(VkBuffers32, 32, VkBuffer);
AS_DECLARE_ARRAY(VkDeviceMemories32, 32, VkDeviceMemory);
AS_DECLARE_ARRAY(VkSurfaceFormatKHR32, 32, VkSurfaceFormatKHR);
AS_DECLARE_ARRAY(VkPresentModeKHR32, 32, VkPresentModeKHR);


typedef struct as_uniform_buffer_object
{
	as_mat4 model;
	as_mat4 view;
	as_mat4 proj;
} as_uniform_buffer_object;

 // use of _Alignof may be needed somewhere here
typedef struct as_vertex
{
	as_vec3 pos;
	as_vec3 color;
	as_vec2 tex_coord;
} as_vertex;
#define AS_VERTEX_VAR_COUNT 3

typedef struct as_push_const_vertex_buffer
{
	as_mat4 transform;
	as_vec4 mouse_data;
	f32 time;
} as_push_const_vertex_buffer;

typedef struct as_uniform_buffers
{
	VkBuffers32 buffers;
	VkDeviceMemories32 memories;
	voids32 buffers_mapped;
} as_uniform_buffers;

typedef struct as_texture
{
	VkImage image;
	VkDeviceMemory memory;
	VkImageView image_view;
	VkSampler sampler;
} as_texture;

typedef struct as_shader_uniform
{
	VkDescriptorType type;
	void* data;
} as_shader_uniform;
AS_DECLARE_ARRAY(as_shader_uniforms_32, 32, as_shader_uniform);

typedef struct as_shader
{
	VkPipeline graphics_pipeline;
	VkPipelineLayout graphics_pipeline_layout;

	VkDescriptorPool descriptor_pool;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSets32 descriptor_sets;

	as_uniform_buffers uniform_buffers;
	as_shader_uniforms_32 uniforms;
}as_shader;

typedef struct as_object
{
	as_transform transform;

	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;
	u32 indices_size;

	as_shader* shader;
} as_object;
AS_DECLARE_ARRAY(as_objects_1024, 1024, as_object);

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
	VkImages64 swap_chain_images;
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;
	VkImageViews32 swap_chain_image_views;
	VkFramebuffers32 swap_chain_framebuffers;
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

	VkCommandBuffers32 command_buffers;

	VkSemaphores32 image_available_semaphores;
	VkSemaphores32 render_finished_semaphores;
	VkFences32 in_flight_fences;

	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;

	VkImage texture_image;
	VkDeviceMemory texture_image_memory;
	VkImageView texture_image_view;
	VkSampler texture_sampler;

	u32 current_frame;
	f32 time;
} as_render;

as_render* as_render_create(void* display_context);
void as_render_draw_frame(as_render* render, void* display_context, as_objects_1024* objects);
void as_render_destroy(as_render* render);

as_texture* as_texture_create(as_render* render, const char* path);

as_shader_uniforms_32* as_uniforms_create();

sz as_shader_add_uniform_float(as_shader_uniforms_32* uniforms, f32* value);
sz as_shader_add_uniform_texture(as_shader_uniforms_32* uniforms, as_texture* texture);
as_shader* as_shader_create(as_render* render, as_shader_uniforms_32* uniforms, const char* vertex_shader_path, const char* fragment_shader_path);
void as_shader_destroy(as_render* render, as_shader* shader);

as_object* as_object_create(as_render* render, as_shader* shader);
sz as_object_add(as_object* object, as_objects_1024* objects);
void as_object_set_translation(as_object* object, const as_vec3* translation);
void as_object_set_rotation(as_object* object, const as_vec3* rotation);
void as_object_set_scale(as_object* object, const as_vec3* scale);
void as_object_destroy(as_render* render, as_object* object);

as_objects_1024* as_objects_create();
void as_objects_destroy(as_render* render, as_objects_1024* objects);