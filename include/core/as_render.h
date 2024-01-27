// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
#pragma once

#include "as_types.h"
#include "as_math.h"
#include "as_array.h"
#include "as_utility.h"
#include "as_threads.h"
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2

#define CLOCKS_PER_SEC_DOUBLE ((f64)CLOCKS_PER_SEC)
#define TARGET_FPS 1000

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
	ADD_FLAG;
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

	char filename_vertex[AS_MAX_PATH_SIZE];
	char filename_fragment[AS_MAX_PATH_SIZE];

	ADD_FLAG;
}as_shader;

typedef struct as_shader_monitored
{
	VkPipeline graphics_pipeline;
	VkPipelineLayout graphics_pipeline_layout;

	char filename_vertex[AS_MAX_PATH_SIZE];
	char filename_fragment[AS_MAX_PATH_SIZE];
} as_shader_monitored;
AS_DECLARE_ARRAY(as_shaders_monitored_512, 512, as_shader_monitored);

typedef struct as_shader_monitor
{
	as_thread thread;
	as_mutex mutex;
	as_shaders_monitored_512 shaders;

	ADD_FLAG;
} as_shader_monitor;

typedef struct as_object
{
	as_transform transform;
	as_shader* shader;

	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;
	u32 indices_size;

	ADD_FLAG;
} as_object;
AS_DECLARE_ARRAY(as_objects_1024, 1024, as_object*);

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

	VkCommandPool command_pool;

	VkCommandBuffers32 command_buffers;

	VkSemaphores32 image_available_semaphores;
	VkSemaphores32 render_finished_semaphores;
	VkFences32 in_flight_fences;

	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;

	u32 current_frame;
	
	// move somewhere else maybe
	f32 time;
	f32 last_frame_time;
	f32 delta_time;
	f32 current_time;
} as_render;

extern as_render* as_render_create(void* display_context);
extern void as_render_start_draw_loop(as_render* render);
extern void as_render_end_draw_loop(as_render* render);
extern void as_render_draw_frame(as_render* render, void* display_context, as_objects_1024* objects);
extern void as_render_destroy(as_render* render);
extern f32 as_render_get_time(as_render* render);
extern f32 as_render_get_remaining_time(as_render* render);
extern f32 as_render_get_delta_time(as_render* render);

extern as_texture* as_texture_create(as_render* render, const char* path);
extern void as_texture_destroy(as_render* render, as_texture* texture);

extern as_shader_uniforms_32* as_uniforms_create();

extern sz as_shader_add_uniform_float(as_shader_uniforms_32* uniforms, f32* value);
extern sz as_shader_add_uniform_texture(as_shader_uniforms_32* uniforms, as_texture* texture);
extern as_shader* as_shader_create(as_render* render, as_shader_uniforms_32* uniforms, const char* vertex_shader_path, const char* fragment_shader_path);
extern void as_shader_destroy(as_render* render, as_shader* shader);

extern as_shader_monitor* as_shader_monitor_create();
extern void as_shader_monitored_destroy(as_shader_monitor* monitor);
extern void as_shader_monitor_add(as_shader_monitor* monitor, as_shader* shader);

extern as_object* as_object_create(as_render* render, as_shader* shader);
extern sz as_object_add(as_object* object, as_objects_1024* objects);
extern void as_object_set_translation(as_object* object, const as_vec3* translation);
extern void as_object_set_rotation(as_object* object, const as_vec3* rotation);
extern void as_object_rotate(as_object* object, const f32 angle, const as_vec3* axis);
extern void as_object_rotate_around_pivot(as_object* object, const f32 angle, const as_vec3* axis, const as_vec3* pivot);
extern void as_object_set_scale(as_object* object, const as_vec3* scale);
extern void as_object_destroy(as_render* render, as_object* object);

extern as_objects_1024* as_objects_create();
extern void as_objects_destroy(as_render* render, as_objects_1024* objects);