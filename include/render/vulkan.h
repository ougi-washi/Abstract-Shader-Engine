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

	typedef std::vector<char> spv;

	struct vertex 
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 tex_coord;

		static VkVertexInputBindingDescription get_binding_description() //TODO: move from here
		{
			VkVertexInputBindingDescription binding_description{};
			binding_description.binding = 0;
			binding_description.stride = sizeof(vertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return binding_description;
		}

		static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions() //TODO: move from here
		{
			std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

			VkVertexInputAttributeDescription pos_attribute_description;
			pos_attribute_description.binding = 0;
			pos_attribute_description.location = 0;
			pos_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
			pos_attribute_description.offset = offsetof(vertex, pos);
			attribute_descriptions.push_back(pos_attribute_description);

			VkVertexInputAttributeDescription color_attribute_description;
			color_attribute_description.binding = 0;
			color_attribute_description.location = 1;
			color_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
			color_attribute_description.offset = offsetof(vertex, color);
			attribute_descriptions.push_back(color_attribute_description);

			VkVertexInputAttributeDescription tex_coor_attribute_description;
			tex_coor_attribute_description.binding = 0;
			tex_coor_attribute_description.location = 2;
			tex_coor_attribute_description.format = VK_FORMAT_R32G32_SFLOAT;
			tex_coor_attribute_description.offset = offsetof(vertex, tex_coord);
			attribute_descriptions.push_back(tex_coor_attribute_description);

			return attribute_descriptions;
		}

		bool operator==(const vertex& other) const {
			return pos == other.pos && color == other.color && tex_coord == other.tex_coord;
		}
	};

	struct uniform_buffer_object 
	{
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
			std::vector<VkPresentModeKHR> present_modes;
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

		struct uniform_buffers
		{
			std::vector<VkBuffer> buffers;
			std::vector<VkDeviceMemory> memory;
		};

		struct descriptor
		{
			VkDescriptorPool descriptorPool;
			VkDescriptorSetLayout descriptor_set_layout;
			std::vector<VkDescriptorSet> descriptorSets;
		};

		struct command_buffers
		{
			std::vector<VkCommandBuffer> commandBuffers;
		};

		struct synchronization
		{
			std::vector<VkSemaphore> image_available_semaphores;
			std::vector<VkSemaphore> render_finished_semaphores;
			std::vector<VkFence> in_flight_fences;
		};

		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() 
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct image_data
		{
			VkImage image;
			VkImageView view;
			VkDeviceMemory memory;
		};

		struct texture_data
		{
			image_data image_data;
			VkSampler sampler;
			u32 mip_levels;
		};

		struct material_data
		{
			// external usage:
			glm::vec3 color_multiplier = glm::vec3(1.f);
			std::vector<vk::texture_data*> textures;

			// internal usage:
			as::spv vertex_shader;
			as::spv fragment_shader;
			as::vk::descriptor descriptor;
		};

		struct object_data
		{
			// data
			std::vector<as::vertex> vertices;
			std::vector<uint32_t> indices;
			char file_path[200] = "";

			// memory
			VkBuffer vertex_buffer;
			VkDeviceMemory vertex_buffer_memory;
			VkBuffer index_buffer;
			VkDeviceMemory index_buffer_memory;

			// rendering
			as::vk::material_data* material = nullptr;
		};
	};
};

namespace std 
{
	template<> struct hash<::as::vertex> 
	{
		size_t operator()(::as::vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.tex_coord) << 1);
		}
	};
};

#endif //_VULKAN_
