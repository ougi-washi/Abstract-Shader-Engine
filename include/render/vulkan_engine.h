#ifndef _VULKAN_SHADER_ENGINE_
#define _VULKAN_SHADER_ENGINE_

#include "render/vulkan.h"
#include "render/vulkan_core.h"
#include "render/vulkan_create_infos.h"

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

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

namespace as
{
	namespace vk
	{
		GLFWwindow* window;

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
		uint32_t currentFrame = 0;

		bool framebufferResized = false;

		void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();
		void create_image_resources();

		void initVulkan();

		void draw_frame();


		void mainLoop();

		void cleanupSwapChain();

		void cleanup();

		void recreateSwapChain();

		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void updateUniformBuffer(uint32_t currentImage);

		void run();
	}
}

#endif //_VULKAN_SHADER_ENGINE_