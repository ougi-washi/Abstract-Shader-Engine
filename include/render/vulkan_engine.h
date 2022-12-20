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

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

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


struct UniformBufferObject {
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

	void framebuffer_resize_callback(GLFWwindow* window, i32 width, i32 height);;

	void init_window(as::window& in_window);

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

			as::vk::swapchain swapchain_;

			VkRenderPass render_pass;
			VkDescriptorSetLayout descriptor_set_layout;
			as::vk::pipeline_data graphics_pipeline;

			VkCommandPool commandPool;

			as::vk::image_data color_image;
			as::vk::image_data depth_image;

			as::vk::texture_data texture;

			std::vector<as::Vertex> vertices;
			std::vector<uint32_t> indices;
			VkBuffer vertexBuffer;
			VkDeviceMemory vertexBufferMemory;
			VkBuffer indexBuffer;
			VkDeviceMemory indexBufferMemory;

			std::vector<VkBuffer> uniformBuffers;
			std::vector<VkDeviceMemory> uniformBuffersMemory;

			VkDescriptorPool descriptorPool;
			std::vector<VkDescriptorSet> descriptorSets;

			std::vector<VkCommandBuffer> commandBuffers;

			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;

			u32 currentFrame = 0;
		};


		void create_image_resources(as::vk::engine& in_engine);

		void init_vulkan(as::vk::engine& in_engine, as::window& in_window);

		void draw_frame(as::vk::engine& in_engine, as::window& in_window);

		void start_main_loop(as::vk::engine& in_engine, as::window& in_window);

		void cleanup_swapchain(as::vk::engine& in_engine);

		void cleanup(as::vk::engine& in_engine, as::window& in_window);

		void recreate_swapchain(as::vk::engine& in_engine, as::window& in_window);

		void record_command_buffer(VkCommandBuffer& commandBuffer, uint32_t& imageIndex, as::vk::engine& in_engine);

		void update_uniform_buffer(u32& currentImage, as::vk::engine& in_engine);

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	}
}

#endif //_VULKAN_SHADER_ENGINE_