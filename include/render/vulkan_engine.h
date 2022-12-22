#ifndef _VULKAN_SHADER_ENGINE_
#define _VULKAN_SHADER_ENGINE_

#include "vulkan_core.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct uniform_buffer_object {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

namespace as
{
	struct window
	{
		GLFWwindow* GLFW;
	};
	static u8 framebuffer_resized;

	void framebuffer_resize_callback(GLFWwindow* window, i32 width, i32 height);

	void init_window(as::window& in_window, const u32& width, const u32& height);

	namespace vk
	{

		struct engine
		{

			VkInstance instance;
			VkDebugUtilsMessengerEXT debugMessenger;
			VkSurfaceKHR surface;

			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
			VkDevice device;

			VkQueue graphicsQueue;
			VkQueue presentQueue;

			as::vk::swapchain swapchain;

			VkRenderPass render_pass;
			as::vk::pipeline_data graphics_pipeline;

			VkCommandPool commandPool;

			as::vk::image_data color_image;
			as::vk::image_data depth_image;

			VkDeviceMemory index_buffer_memory;

			std::vector<VkBuffer> buffers;
			std::vector<VkDeviceMemory> memory;

			std::vector<VkCommandBuffer> commandBuffers;

			std::vector<VkSemaphore> image_available_semaphores;
			std::vector<VkSemaphore> render_finished_semaphores;
			std::vector<VkFence> in_flight_fences;

			u8 max_frames_in_flight;
			u32 currentFrame = 0;

			std::vector<vk::object_data> objects;
			std::vector<vk::texture_data> textures;
			std::vector<vk::material_data> materials;
		};

		// init
		void init_vulkan(as::vk::engine& in_engine, as::window& in_window);
		void init_vulkan(as::vk::engine& in_engine, as::window& in_window, const u8& max_frames_in_flight);
		
		// main functionalities
		void start_main_loop(as::vk::engine& in_engine, as::window& in_window);
		void create_shader(as::vk::engine& in_engine, const char* in_path, as::spv &out_shader_binaries);
		void add_object(as::vk::engine& in_engine, const char* in_path, as::vk::object_data& out_object_data);
		void add_texture(as::vk::engine& in_engine, const char* in_path, as::vk::texture_data& out_texture_data);
		void add_material(as::vk::engine& in_engine, const char* in_vert_shader_path, const char* in_frag_shader_path, as::vk::material_data& out_material_data);
		void update_uniform_buffer(u32& currentImage, as::vk::engine& in_engine);
		void create_graphics_pipeline(as::vk::engine& in_engine);

		// internal 
		void draw_frame(as::vk::engine& in_engine, as::window& in_window);
		void recreate_swapchain(as::vk::engine& in_engine, as::window& in_window);
		void record_command_buffer(VkCommandBuffer& command_buffer, uint32_t& imageIndex, as::vk::engine& in_engine);
		void create_image_resources(as::vk::engine& in_engine);

		// cleanup
		void cleanup(as::vk::engine& in_engine, as::window& in_window);
		void cleanup_swapchain(as::vk::engine& in_engine);

		// debug
		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	}
}

#endif //_VULKAN_SHADER_ENGINE_