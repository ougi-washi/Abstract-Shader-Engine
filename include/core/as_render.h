// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi
#pragma once

#include "as_types.h"
#include "as_math.h"
#include "as_array.h"
#include "as_utility.h"
#include "as_threads.h"
#include "core/as_shapes.h"
#include "defines/as_global.h"
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2

#define CLOCKS_PER_SEC_DOUBLE ((f64)CLOCKS_PER_SEC)
#define AS_TARGET_FPS		80000
#define AS_FPS_UPDATE_RATE	50

// Arrays

AS_ARRAY_DECLARE(VkImages64, 64, VkImage);
AS_ARRAY_DECLARE(VkImageViews32, 32, VkImageView);
AS_ARRAY_DECLARE(VkFramebuffers32, 32, VkFramebuffer);
AS_ARRAY_DECLARE(VkSemaphores32, 32, VkSemaphore);
AS_ARRAY_DECLARE(VkFences32, 32, VkFence);
AS_ARRAY_DECLARE(VkCommandBuffers32, 32, VkCommandBuffer);
AS_ARRAY_DECLARE(VkDescriptorSets32, 32, VkDescriptorSet);
AS_ARRAY_DECLARE(VkBuffers32, 32, VkBuffer);
AS_ARRAY_DECLARE(VkDeviceMemories32, 32, VkDeviceMemory);
AS_ARRAY_DECLARE(VkSurfaceFormatKHR32, 32, VkSurfaceFormatKHR);
AS_ARRAY_DECLARE(VkPresentModeKHR32, 32, VkPresentModeKHR);


#define AS_MAX_GPU_OBJECT_TRANSFORMS_SIZE 64
typedef struct as_uniform_buffer_object
{
	as_mat4 model;
	as_mat4 view;
	as_mat4 proj;
	as_mat4 scene_info;
	as_mat4 object_transforms[AS_MAX_GPU_OBJECT_TRANSFORMS_SIZE];
} as_uniform_buffer_object;

typedef struct as_push_const_buffer
{
	as_mat4 data;
	// camera_position  X[0][0] Y[0][1] Z[0][2]
	// camera_direction X[1][0] Y[1][1] Z[1][2]
	// current_time		[2][0]
	// object_index		[2][1]
	// mouse_data		X[3][0] Y[3][1]
} as_push_const_buffer;

#define AS_MAX_GPU_SCREEN_OBJECT_CUSTOM_DATA_SIZE 64
// info define
#define AS_UI_TEXT_LENGTH_INDEX		m[0][0]
#define AS_UI_TEXT_FONT_SIZE_INDEX	m[0][1]
#define AS_UI_TEXT_SPACING_INDEX	m[0][2]
typedef struct as_uniform_buffer_screen_object
{
	as_mat4 custom_info;
	as_mat4 custom_data[AS_MAX_GPU_SCREEN_OBJECT_CUSTOM_DATA_SIZE];
} as_uniform_buffer_screen_object;

typedef struct as_push_const_buffer_screen_object
{
	as_mat4 data;
	// position	x[0][0] y[0][1]
	// rotation	x[0][2] y[0][3]
	// scale	x[1][0] y[1][1]
	// [2][...]	empty
	// [3][...]	empty
} as_push_const_buffer_screen_object;

typedef struct as_uniform_buffers
{
	VkBuffers32 buffers;
	VkDeviceMemories32 memories;
	voids32 buffers_mapped;
} as_uniform_buffers;

typedef struct as_texture
{
	AS_DECLARE_TYPE;

	VkDevice* device;

	VkImage image;
	VkDeviceMemory memory;
	VkImageView image_view;
	VkSampler sampler;

	char filename[AS_MAX_PATH_SIZE];
} as_texture;
AS_ARRAY_DECLARE(as_textures_pool, AS_MAX_TEXTURE_POOL_SIZE, as_texture);

typedef struct as_shader_uniform
{
	VkDescriptorType type;
	VkShaderStageFlagBits stage;
	void* data;
} as_shader_uniform;
AS_ARRAY_DECLARE(as_shader_uniforms, AS_MAX_SHADER_UNIFORMS_SIZE, as_shader_uniform);

typedef struct as_shader
{
	AS_DECLARE_TYPE; // adding this at the beginning is a must for better cache checks and avoid crashes

	VkDevice* device;
	VkRenderPass* render_pass;

	VkPipeline graphics_pipeline;
	VkPipelineLayout graphics_pipeline_layout;

	VkDescriptorPool descriptor_pool;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSets32 descriptor_sets;

	as_uniform_buffers uniform_buffers;
	as_shader_uniforms uniforms;

	char filename_vertex[AS_MAX_PATH_SIZE];
	char filename_fragment[AS_MAX_PATH_SIZE];

	u64 refresh_frame; // this will define whether or not to use the graphics_pipeline
	
}as_shader;

typedef struct as_object // TODO: Get GPU data out so they can loop faster in the drawcommands
{
	AS_DECLARE_TYPE;

	as_transform transform;
	as_shape* shape;
	as_shader* shader;
	u32 instance_count;
	
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;
	u32 indices_size;

	i32 scene_gpu_index; // index of the object in the GPU scene 
	
} as_object;
AS_ARRAY_DECLARE(as_scene_objects, AS_MAX_SCENE_OBJECTS, as_object);

typedef struct as_light
{
	as_vec3 position;
	float _padding_0; // Add padding to ensure 16-byte alignment
	as_vec3 color;
	f32 radius;
} as_light;
AS_ARRAY_DECLARE(as_scene_lights, AS_MAX_SCENE_LIGHTS, as_light);

typedef enum as_camera_type
{
	AS_CAMERA_FREE = 0,
	AS_CAMERA_MANUAL = 1
} as_camera_type;
typedef struct as_camera
{
	AS_DECLARE_TYPE;

	as_vec3 position;
	as_vec3 target;
	as_vec3 up;
	f32 fov;
	as_camera_type type;
	f64 movement_speed;

	as_vec3 cached_direction;
} as_camera;
AS_ARRAY_DECLARE(as_scene_cameras, AS_MAX_SCENE_CAMERAS, as_camera);

// currently, I am passing the whole scene, in the future it should only be the nearby objects that can impact the shader of the target object
// Also, alignment matters
typedef struct as_scene_gpu_data
{
	as_mat4 info;
	as_mat4 objects_transforms[AS_MAX_GPU_OBJECT_TRANSFORMS_SIZE]; // max is 512 since 65536 is the max possible size
	// as_lights_128 lights;
} as_scene_gpu_data;

typedef struct as_scene_gpu_buffer  
{
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDeviceSize size;
} as_scene_gpu_buffer;

typedef struct as_scene
{
	char path[AS_MAX_PATH_SIZE];
	as_scene_objects objects;
	as_scene_lights lights;
	as_scene_cameras cameras; // main camera is index 0

	as_scene_gpu_data gpu_data;
	//as_scene_gpu_buffer gpu_buffer;
	AS_DECLARE_TYPE;
} as_scene;

typedef enum as_screen_object_type
{
	AS_SO_DEFAULT	= 0,
	AS_SO_IMAGE		= 1,
	AS_SO_TEXT		= 2
}as_screen_object_type;

#define AS_SCREEN_OBJ_DATA_POS_X_INDEX m[0][0]
#define AS_SCREEN_OBJ_DATA_POS_Y_INDEX m[0][1]
#define AS_SCREEN_OBJ_DATA_ROT_X_INDEX m[0][2]
#define AS_SCREEN_OBJ_DATA_ROT_Y_INDEX m[0][3]
#define AS_SCREEN_OBJ_DATA_SCALE_X_INDEX m[1][0]
#define AS_SCREEN_OBJ_DATA_SCALE_Y_INDEX m[1][1]
typedef struct as_screen_object
{
	AS_DECLARE_TYPE;

	as_mat4 data;

	VkDevice* device;
	VkRenderPass* render_pass;

	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;

	VkDescriptorPool descriptor_pool;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSets32 descriptor_sets;

	as_uniform_buffers uniform_buffers;
	as_shader_uniforms uniforms;

	char filename_fragment[AS_MAX_PATH_SIZE];

	as_screen_object_type type;
	as_uniform_buffer_screen_object uniform_buffer;
} as_screen_object;
AS_ARRAY_DECLARE(as_screen_objects_group, AS_MAX_SCREEN_OBJECTS, as_screen_object);

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

	u64 current_frame; // this one is for rendering, do not use
	u64 frame_counter; // use this for frame tracking

	// TODO: move time to handle
	f64 time;
	f64 last_frame_time;	
	f64 delta_time;
	f64 current_time;

	// TODO: move fps to handle
	f64 fps_last_update;
	f64 fps_average;
	f64 fps_total;
	u64 fps_num_samples;

	AS_DECLARE_TYPE;
} as_render;

extern as_render* as_render_create(void* display_context);
extern void as_render_start_draw_loop(as_render* render);
extern void as_render_end_draw_loop(as_render* render);
extern void as_render_draw_frame(as_render* render, void* display_context, as_camera* camera, as_scene* scene, as_screen_objects_group* screen_objects_group);
extern void as_render_update_fps(as_render* render, const f64 update_rate);
extern void as_render_destroy(as_render* render);
extern u64 as_render_get_frame_count(as_render* render);
extern u64* as_render_get_frame_count_ptr(as_render* render);
extern f64 as_render_get_fps(const as_render* render);
extern f64 as_render_get_time(const as_render* render);
extern f64 as_render_get_remaining_time(as_render* render);
extern f64 as_render_get_delta_time(as_render* render);

extern void as_screen_object_init(as_render* render, as_screen_object* screen_object,const char* fragment_path);
extern void as_screen_object_update(as_render* render, as_screen_object* screen_object);
extern void as_screen_object_create_pipeline(as_screen_object* screen_object);
extern void as_screen_object_destroy(as_screen_object* screen_object, const b8 free_ptr);
extern as_screen_objects_group* as_screen_objects_group_create();
extern void as_screen_objects_group_destroy(as_screen_objects_group* screen_object_group);
extern void as_screen_object_set_position(as_screen_object* screen_object, const as_vec2* position);
extern void as_screen_object_set_rotation(as_screen_object* screen_object, const as_vec2* rotation);
extern void as_screen_object_set_scale(as_screen_object* screen_object, const as_vec2* scale);
extern as_vec2 as_screen_object_get_position(const as_screen_object* screen_object);
extern as_vec2 as_screen_object_get_rotation(const as_screen_object* screen_object);
extern as_vec2 as_screen_object_get_scale(const as_screen_object* screen_object);

extern as_texture* as_texture_make(const char* path);
extern void as_texture_init(as_texture* texture, const char* path);
extern bool as_texture_update(as_render* render, as_texture* texture);
extern void as_texture_destroy(as_texture* texture);
extern as_textures_pool* as_textures_pool_create();
extern void as_textures_pool_destroy(as_textures_pool* textures_pool);
extern as_texture* as_texture_get_from_pool(as_textures_pool* textures_pool);
extern void as_texture_remove_from_pool(as_textures_pool* textures_pool, as_texture* texture, const bool destory);

extern void as_shader_create_graphics_pipeline(as_shader* shader);
extern sz as_shader_add_uniform_float(as_shader_uniforms* uniforms, f32* value);
extern sz as_shader_add_uniform_texture(as_shader_uniforms* uniforms, as_texture* texture);
extern sz as_shader_add_scene_gpu(as_shader_uniforms* uniforms, as_scene_gpu_buffer* scene_gpu_buffer);
extern as_shader* as_shader_make(as_render* render, const char* vertex_shader_path, const char* fragment_shader_path);
extern void as_shader_set_uniforms(as_render* render, as_shader* shader, as_shader_uniforms* uniforms);
extern void as_shader_update(as_render* render, as_shader* shader);
extern void as_shader_destroy(as_render* render, as_shader* shader);

extern as_camera* as_camera_make(as_scene* scene, const as_vec3* position, const as_vec3* target);
extern as_camera* as_camera_get_main(as_scene* scene);
extern void as_camera_set_main(as_scene* scene, as_camera* camera);
extern void as_camera_set_position(as_camera* camera, const as_vec3* position);
extern void as_camera_set_target(as_camera* camera, const as_vec3* target);

extern as_object* as_object_consturct(as_render* render, as_scene* scene);
extern void as_object_update(as_render* render, as_object* object, as_shape* shape, as_shader* shader);
extern void as_object_set_instance_count(as_object* object, const u32 instance_count);
extern void as_object_set_translation(as_object* object, const as_vec3* translation);
extern void as_object_translate(as_object* object, const as_vec3* translation);
extern void as_object_set_rotation(as_object* object, const as_vec3* rotation);
extern void as_object_rotate(as_object* object, const f32 angle, const as_vec3* axis);
extern void as_object_rotate_around_pivot(as_object* object, const f32 angle, const as_vec3* axis, const as_vec3* pivot);
extern void as_object_set_scale(as_object* object, const as_vec3* scale);
extern const as_mat4* as_object_get_transform(const as_object* object);
extern as_vec3 as_object_get_translation(const as_object* object);
extern void as_object_destroy(as_render* render, as_object* object);

extern as_scene* as_scene_create(as_render* render, const char* scene_path);
extern as_scene* as_scene_load(as_render* render, const char* scene_path);
extern void as_scene_gpu_update_data(as_scene* scene);
extern void as_scene_gpu_update_buffer(as_render* render, as_scene* scene);
extern void as_scene_destroy(as_render* render, as_scene* scene);
