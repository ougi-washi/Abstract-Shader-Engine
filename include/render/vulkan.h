#ifndef _VULKAN_
#define _VULKAN_

#include "types.h"

// GLFW (This includes vulkan)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>


#include <optional> // TODO: REMOVE
#include <array> // TODO: REMOVE
#include <vector> // TODO: REMOVE
#include <unordered_map>// TODO: REMOVE
#include <functional> // TODO: REMOVE


namespace as
{

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}

		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};

	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	struct glfw
	{
		GLFWwindow* window;
	};

	namespace vk
	{
		struct instance
		{
			VkInstance instance;
			VkDebugUtilsMessengerEXT debugMessenger;
			VkSurfaceKHR surface;
		};

		struct device
		{
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
			VkDevice device;
		};

		struct queues
		{
			VkQueue graphicsQueue;
			VkQueue presentQueue;
		};

		struct swapchain
		{
			VkSwapchainKHR swapchainKHR;
			std::vector<VkImage> images;
			VkFormat image_format;
			VkExtent2D extent;
			std::vector<VkImageView> image_views;
			std::vector<VkFramebuffer> framebuffers;
		};

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		struct pipeline_data
		{
			VkPipeline pipeline;
			VkPipelineLayout layout;
		};

		struct command_pool
		{
			VkCommandPool commandPool;
		};

		struct image_data
		{
			VkImage image;
			VkImageView view;
			VkDeviceMemory memory;
		};

		struct texture_data
		{
			uint32_t mipLevels;
			VkImage textureImage;
			VkDeviceMemory textureImageMemory;
			VkImageView textureImageView;
			VkSampler textureSampler;
		};

		struct model_data
		{
			std::vector<as::Vertex> vertices;
			std::vector<uint32_t> indices;
			VkBuffer vertexBuffer;
			VkDeviceMemory vertexBufferMemory;
			VkBuffer indexBuffer;
			VkDeviceMemory indexBufferMemory;
		};

		struct uniform_buffers
		{
			std::vector<VkBuffer> uniformBuffers;
			std::vector<VkDeviceMemory> uniformBuffersMemory;
		};

		struct descriptor
		{
			VkDescriptorPool descriptorPool;
			std::vector<VkDescriptorSet> descriptorSets;
		};

		struct command_buffers
		{
			std::vector<VkCommandBuffer> commandBuffers;
		};

		struct synchronization
		{
			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;
		};

		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};
	};
};

namespace std 
{
	template<> struct hash<::as::Vertex> {
		size_t operator()(::as::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
};

#endif //_VULKAN_
