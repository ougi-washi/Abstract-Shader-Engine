#pragma once
#include "render/vulkan_core.h"
#include "render/vulkan_create_infos.h"
#include "shader/shaderc_core.h"

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

    void create_image_resources()
    {
		as::vk::image_create_info color_image_create_info;
		color_image_create_info.physical_device = physicalDevice;
		color_image_create_info.logical_device = device;
		color_image_create_info.height = swapchain.extent.height;
		color_image_create_info.width = swapchain.extent.width;
		color_image_create_info.mip_levels = 1;
		color_image_create_info.num_samples = msaaSamples;
		color_image_create_info.format = swapchain.image_format;
		color_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		color_image_create_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		color_image_create_info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		as::vk::create_image(color_image_create_info, color_image);
		as::vk::image_view_create_info color_image_view_create_info;
		color_image_view_create_info.logical_device = device;
		color_image_view_create_info.image = color_image.image;
		color_image_view_create_info.mip_levels = 1;
		color_image_view_create_info.format = swapchain.image_format;
		color_image_view_create_info.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
		as::vk::create_image_view(color_image_view_create_info, color_image.view);

		VkFormat supported_depth_format = as::vk::find_depth_format(physicalDevice);
		as::vk::image_create_info depth_image_create_info = color_image_create_info;
		depth_image_create_info.format = supported_depth_format;
		depth_image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		as::vk::create_image(depth_image_create_info, depth_image);
		as::vk::image_view_create_info depth_image_view_create_info = color_image_view_create_info;
		depth_image_view_create_info.image = depth_image.image;
		depth_image_view_create_info.format = supported_depth_format;
		depth_image_view_create_info.aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
		as::vk::create_image_view(depth_image_view_create_info, depth_image.view);
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
			CHECK_VK_RESULT(create_image_view(image_view_create_info, swapchain.image_views[i]));
		}

        as::vk::render_pass_create_info render_pass_create_info;
        render_pass_create_info.physical_device = physicalDevice;
        render_pass_create_info.logical_device = device;
        render_pass_create_info.msaa_samples = msaaSamples;
        render_pass_create_info.swap_chain_image_format = swapchain.image_format;
        as::vk::create_render_pass(render_pass_create_info, render_pass);

        as::vk::create_descriptor_set_layout(device, descriptor_set_layout);
        
		// vertex shader
		as::sc::shader_compile_info compile_info_vertex = {};
		compile_info_vertex.file_name = new char[]("main");
		compile_info_vertex.source = as::read_file("shaders/shader.vert");
		compile_info_vertex.kind = shaderc_glsl_vertex_shader;
        as::sc::shader_binaries out_vertex_shader_bin;
        as::sc::compile_shader(&out_vertex_shader_bin, compile_info_vertex); // cache and compare next time, do not compile every time
        as::sc::write_shader_bin("shaders/vert.spv", out_vertex_shader_bin);
        std::vector<char> vert_shader_code = as::read_file(std::string("shaders/vert.spv"));

		// fragment shader
        as::sc::shader_compile_info compile_info_frag = {};
		compile_info_frag.file_name = new char[]("main");
		compile_info_frag.source = as::read_file("shaders/shader.frag");
		compile_info_frag.kind = shaderc_glsl_fragment_shader;
        as::sc::shader_binaries out_frag_shader_bin;
        as::sc::compile_shader(&out_frag_shader_bin, compile_info_frag); // cache and compare next time, do not compile every time
        as::sc::write_shader_bin("shaders/frag.spv", out_frag_shader_bin);
		std::vector<char> frag_shader_code = as::read_file(std::string("shaders/frag.spv"));
        
        as::vk::pipeline_create_info pipeline_create_info;
        pipeline_create_info.logical_device = device;
        pipeline_create_info.descriptor_set_layout = descriptor_set_layout;
        pipeline_create_info.render_pass = render_pass;
        pipeline_create_info.msaa_samples = msaaSamples;
        pipeline_create_info.vert_shader_spv = vert_shader_code;
        pipeline_create_info.frag_shader_spv = frag_shader_code;
        as::vk::create_pipeline(pipeline_create_info, graphics_pipeline);
        
        as::vk::command_pool_create_info command_pool_create_info;
        command_pool_create_info.physical_device = physicalDevice;
        command_pool_create_info.logical_device = device;
        command_pool_create_info.surface = &surface;
        as::vk::create_command_pool(command_pool_create_info, commandPool);

        // color and depth image data
        create_image_resources();
        
        as::vk::framebuffers_create_info framebuffers_create_info;
        framebuffers_create_info.color_image_view = color_image.view;
        framebuffers_create_info.depth_image_view = depth_image.view;
        framebuffers_create_info.logical_device = device;
        framebuffers_create_info.render_pass = render_pass;
        framebuffers_create_info.swap_chain_extent = swapchain.extent;
        framebuffers_create_info.swap_chain_image_views = swapchain.image_views;
        as::vk::create_framebuffers(framebuffers_create_info, swapchain.framebuffers);

        as::vk::texture_image_create_info texture_image_create_info;
        strcpy(texture_image_create_info.texture_path, TEXTURE_PATH.c_str());
        texture_image_create_info.physical_device = physicalDevice;
        texture_image_create_info.logical_device = device;
        texture_image_create_info.graphics_queue = graphicsQueue;
        texture_image_create_info.command_pool = commandPool;
        texture_image_create_info.texture_image_memory = texture.image_data.memory;
        as::vk::create_texture_image(texture_image_create_info, texture);

        as::vk::image_view_create_info texture_image_view_create_info;
        texture_image_view_create_info.logical_device = device;
        texture_image_view_create_info.image = texture.image_data.image;
        texture_image_view_create_info.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
        texture_image_view_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
        texture_image_view_create_info.mip_levels = texture.mip_levels;
        as::vk::create_image_view(texture_image_view_create_info, texture.image_data.view);
        
        as::vk::sampler_create_info sampler_create_info;
        sampler_create_info.logical_device = device;
        sampler_create_info.physical_device = physicalDevice;
        sampler_create_info.mip_levels = texture.mip_levels;
        as::vk::create_sampler(sampler_create_info, texture.sampler);

        as::vk::load_model(MODEL_PATH.c_str(), vertices, indices);

        as::vk::vertex_buffer_create_info vertex_buffer_create_info;
        vertex_buffer_create_info.physical_device = physicalDevice;
        vertex_buffer_create_info.logical_device = device;
        vertex_buffer_create_info.queue = graphicsQueue;
        vertex_buffer_create_info.command_pool = commandPool;
        vertex_buffer_create_info.vertices = vertices;
        as::vk::create_vertex_buffer(vertex_buffer_create_info, vertexBuffer, vertexBufferMemory);

        as::vk::index_buffer_create_info index_buffer_create_info;
        index_buffer_create_info.physical_device = physicalDevice;
        index_buffer_create_info.logical_device = device;
        index_buffer_create_info.queue = graphicsQueue;
        index_buffer_create_info.command_pool = commandPool;
        index_buffer_create_info.indices = indices;
        as::vk::create_index_buffer(index_buffer_create_info, indexBuffer, indexBufferMemory);

        as::vk::create_uniform_buffers(uniformBuffers, uniformBuffersMemory, physicalDevice, device, MAX_FRAMES_IN_FLIGHT);
        as::vk::create_descriptor_pool(descriptorPool, device, MAX_FRAMES_IN_FLIGHT);
        as::vk::create_descriptor_sets(descriptorSets, device, descriptor_set_layout, descriptorPool, MAX_FRAMES_IN_FLIGHT);
        as::vk::update_descriptor_sets(device, descriptorSets, uniformBuffers, MAX_FRAMES_IN_FLIGHT, texture.image_data.view, texture.sampler);
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
			color_image,
            depth_image
		};
		as::vk::cleanup_swap_chain(device, swapchain.swapchainKHR, images_data, swapchain.framebuffers, swapchain.image_views);
    }

    void cleanup() {
        cleanupSwapChain();

        vkDestroyPipeline(device, graphics_pipeline.pipeline, nullptr);
        vkDestroyPipelineLayout(device, graphics_pipeline.layout, nullptr);
        vkDestroyRenderPass(device, render_pass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        vkDestroySampler(device, texture.sampler, nullptr);
        vkDestroyImageView(device, texture.image_data.view, nullptr);

        vkDestroyImage(device, texture.image_data.image, nullptr);
        vkFreeMemory(device, texture.image_data.memory, nullptr);

        vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);

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
		create_image_resources();
		as::vk::create_frame_buffers(swapchain.framebuffers, device, swapchain.image_views, color_image.view, depth_image.view, render_pass, swapchain.extent);
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = render_pass;
        renderPassInfo.framebuffer = swapchain.framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchain.extent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline.pipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.extent.width;
		viewport.height = (float)swapchain.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapchain.extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline.layout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

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