#pragma once
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

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
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
    as::vk::render render;
    //VkRenderPass render.pass;
    //VkDescriptorSetLayout render.descriptor_set_layout;
    //VkPipelineLayout render.pipeline_layout;
    //VkPipeline render.graphics_pipeline;

    VkCommandPool commandPool;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    uint32_t mipLevels;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

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

    void initWindow() 
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        
        as::vk::instance_create_info instance_create_info = {};
        as::vk::create_vulkan_instance(instance, instance_create_info);
        if (instance_create_info.enable_validation_layers)
        {
			as::vk::setup_debug_messenger(&instance, &debugMessenger);
        }

        as::vk::surface_create_info surface_create_info = {};
        surface_create_info.instance = instance;
        surface_create_info.window = window;
		as::vk::create_surface(surface_create_info, surface);

        as::vk::physical_device_create_info physical_device_create_info = {};
        physical_device_create_info.surface = &surface;
        physical_device_create_info.instance = instance;
        as::vk::pick_physical_device(physical_device_create_info, physicalDevice, msaaSamples);

        as::vk::logical_device_create_info logical_device_create_info = {};
        logical_device_create_info.physical_device = physicalDevice;
        logical_device_create_info.surface = &surface;
        logical_device_create_info.extensions = deviceExtensions;
        logical_device_create_info.validation_layers = validationLayers;
        as::vk::create_logical_device(logical_device_create_info, &device, &graphicsQueue, &presentQueue);

		as::vk::swapchain_create_info swapchain_create_info = {};
        swapchain_create_info.logical_device = device;
        swapchain_create_info.physical_device = physicalDevice;
        swapchain_create_info.surface = &surface;
        swapchain_create_info.window = window;
        as::vk::create_swapchain(swapchain_create_info, swapchain);

        as::vk::image_view_create_info image_view_create_info = {};
        image_view_create_info.logical_device = device;
        image_view_create_info.format = swapchain.image_format;
        image_view_create_info.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.mip_levels = 1;
		u32 images_array_size = swapchain.images.size();
        swapchain.image_views.resize(images_array_size);
		for (u32 i = 0; i < images_array_size; i++)
		{
            image_view_create_info.image = swapchain.images[i];
			CHECK_VK_RESULT(create_image_view(image_view_create_info, &swapchain.image_views[i]));
		}

        as::vk::render_pass_create_info render_pass_create_info;
        render_pass_create_info.physical_device = physicalDevice;
        render_pass_create_info.logical_device = device;
        render_pass_create_info.msaa_samples = msaaSamples;
        render_pass_create_info.swap_chain_image_format = swapchain.image_format;
        as::vk::create_render_pass(render_pass_create_info, render.pass);

        as::vk::create_descriptor_set_layout(device, render.descriptor_set_layout);

        as::vk::create_graphics_pipeline(render.graphics_pipeline, render.pipeline_layout, device, msaaSamples, render.descriptor_set_layout, render.pass);
        as::vk::create_command_pool(commandPool, physicalDevice, device, surface);
        as::vk::create_color_resources(colorImageView, physicalDevice, device, colorImage, colorImageMemory, swapchain.image_format, swapchain.extent, msaaSamples);
        as::vk::create_depth_resources(depthImageView, physicalDevice, device, depthImage, depthImageMemory, swapchain.image_format, swapchain.extent, msaaSamples);
        as::vk::create_frame_buffers(swapchain.framebuffers, device, swapchain.image_views, colorImageView, depthImageView, render.pass, swapchain.extent);
        as::vk::create_texture_image(textureImage, TEXTURE_PATH.c_str(), mipLevels, physicalDevice, device, commandPool, graphicsQueue, textureImageMemory);
        as::vk::create_texture_image_view(textureImageView, device, textureImage, mipLevels);
        as::vk::create_texture_sampler(textureSampler, physicalDevice, device, mipLevels);
        as::vk::load_model(MODEL_PATH.c_str(), vertices, indices);
        as::vk::create_vertex_buffer(vertexBuffer, vertexBufferMemory, physicalDevice, device, vertices, commandPool, graphicsQueue);
        as::vk::create_index_buffer(indexBuffer, indexBufferMemory, physicalDevice, device, indices, commandPool, graphicsQueue);
        as::vk::create_uniform_buffers(uniformBuffers, uniformBuffersMemory, physicalDevice, device, MAX_FRAMES_IN_FLIGHT);
        as::vk::create_descriptor_pool(descriptorPool, device, MAX_FRAMES_IN_FLIGHT);
        as::vk::create_descriptor_sets(descriptorSets, device, render.descriptor_set_layout, descriptorPool, MAX_FRAMES_IN_FLIGHT);
        as::vk::update_descriptor_sets(device, descriptorSets, uniformBuffers, MAX_FRAMES_IN_FLIGHT, textureImageView, textureSampler);
        as::vk::create_command_buffers(commandBuffers, device, commandPool, MAX_FRAMES_IN_FLIGHT);
        as::vk::create_sync_objects(device, imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences, MAX_FRAMES_IN_FLIGHT);
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }

    void cleanupSwapChain() {
		std::vector<as::vk::image_data> images_data =
		{
			{colorImage, colorImageView, colorImageMemory},
			{depthImage, depthImageView, depthImageMemory}
		};
		as::vk::cleanup_swap_chain(device, swapchain.swapchainKHR, images_data, swapchain.framebuffers, swapchain.image_views);
    }

    void cleanup() {
        cleanupSwapChain();

        vkDestroyPipeline(device, render.graphics_pipeline, nullptr);
        vkDestroyPipelineLayout(device, render.pipeline_layout, nullptr);
        vkDestroyRenderPass(device, render.pass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        vkDestroySampler(device, textureSampler, nullptr);
        vkDestroyImageView(device, textureImageView, nullptr);

        vkDestroyImage(device, textureImage, nullptr);
        vkFreeMemory(device, textureImageMemory, nullptr);

        vkDestroyDescriptorSetLayout(device, render.descriptor_set_layout, nullptr);

        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexBufferMemory, nullptr);

        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        cleanupSwapChain();

		as::vk::create_swap_chain(&swapchain.swapchainKHR, &swapchain.images, &swapchain.image_format, &swapchain.extent, &device, &physicalDevice, &surface, window);
		as::vk::create_image_views(&swapchain.image_views, &swapchain.framebuffers, &swapchain.images, &swapchain.image_format, &device);
		as::vk::create_color_resources(colorImageView, physicalDevice, device, colorImage, colorImageMemory, swapchain.image_format, swapchain.extent, msaaSamples);
		as::vk::create_depth_resources(depthImageView, physicalDevice, device, depthImage, depthImageMemory, swapchain.image_format, swapchain.extent, msaaSamples);
		as::vk::create_frame_buffers(swapchain.framebuffers, device, swapchain.image_views, colorImageView, depthImageView, render.pass, swapchain.extent);
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = render.pass;
        renderPassInfo.framebuffer = swapchain.framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchain.extent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render.graphics_pipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)swapchain.extent.width;
            viewport.height = (float)swapchain.extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapchain.extent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            VkBuffer vertexBuffers[] = {vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render.pipeline_layout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapchain.extent.width / (float)swapchain.extent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        void* data;
        vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
    }

    void drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapchain.swapchainKHR, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateUniformBuffer(currentFrame);

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapchain.swapchainKHR};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
};