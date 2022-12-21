#include "vulkan_engine.h"
#include "shaderc_core.h"
#include "utility.h"

void as::framebuffer_resize_callback(GLFWwindow* window, i32 width, i32 height)
{
	as::framebuffer_resized = true;
}

void as::init_window(window& in_window)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	in_window.GLFW = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	if (in_window.GLFW)
	{
		glfwSetWindowUserPointer(in_window.GLFW, nullptr);
		glfwSetFramebufferSizeCallback(in_window.GLFW, framebuffer_resize_callback);
	}
}

void as::vk::init_vulkan(engine& in_engine, as::window& in_window)
{
	as::vk::instance_create_info instance_create_info = {};
	as::vk::create_vulkan_instance(in_engine.instance, instance_create_info);
	if (instance_create_info.enable_validation_layers)
	{
		as::vk::setup_debug_messenger(&in_engine.instance, &in_engine.debugMessenger);
	}

	as::vk::surface_create_info surface_create_info = {};
	surface_create_info.instance = in_engine.instance;
	surface_create_info.window = in_window.GLFW;
	as::vk::create_surface(surface_create_info, in_engine.surface);

	as::vk::physical_device_create_info physical_device_create_info = {};
	physical_device_create_info.surface = &in_engine.surface;
	physical_device_create_info.instance = in_engine.instance;
	as::vk::pick_physical_device(physical_device_create_info, in_engine.physicalDevice, in_engine.msaaSamples);

	as::vk::logical_device_create_info logical_device_create_info = {};
	logical_device_create_info.physical_device = in_engine.physicalDevice;
	logical_device_create_info.surface = &in_engine.surface;
	logical_device_create_info.extensions = deviceExtensions;
	logical_device_create_info.validation_layers = validationLayers;
	as::vk::create_logical_device(logical_device_create_info, &in_engine.device, &in_engine.graphicsQueue, &in_engine.presentQueue);

	as::vk::swapchain_create_info swapchain_create_info = {};
	swapchain_create_info.logical_device = in_engine.device;
	swapchain_create_info.physical_device = in_engine.physicalDevice;
	swapchain_create_info.surface = &in_engine.surface;
	swapchain_create_info.window = in_window.GLFW;
	as::vk::create_swapchain(swapchain_create_info, in_engine.swapchain);

	as::vk::image_view_create_info image_view_create_info = {};
	image_view_create_info.logical_device = in_engine.device;
	image_view_create_info.format = in_engine.swapchain.image_format;
	image_view_create_info.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_create_info.mip_levels = 1;
	u32 images_array_size = in_engine.swapchain.images.size();
	in_engine.swapchain.image_views.resize(images_array_size);
	for (u32 i = 0; i < images_array_size; i++)
	{
		image_view_create_info.image = in_engine.swapchain.images[i];
		CHECK_VK_RESULT(as::vk::create_image_view(image_view_create_info, in_engine.swapchain.image_views[i]));
	}

	as::vk::render_pass_create_info render_pass_create_info;
	render_pass_create_info.physical_device = in_engine.physicalDevice;
	render_pass_create_info.logical_device = in_engine.device;
	render_pass_create_info.msaa_samples = in_engine.msaaSamples;
	render_pass_create_info.swap_chain_image_format = in_engine.swapchain.image_format;
	as::vk::create_render_pass(render_pass_create_info, in_engine.render_pass);

	as::vk::create_descriptor_set_layout(in_engine.device, in_engine.descriptor_set_layout);

	std::vector<char> vert_shader_code;
	char vert_shader_path[] = "shaders/shader.vert";
	sc::compile_vertex_shader(vert_shader_path, vert_shader_code);

	std::vector<char> frag_shader_code;
	char frag_shader_path[] = "shaders/shader.frag";
	sc::compile_fragment_shader(frag_shader_path, frag_shader_code);

	as::vk::pipeline_create_info pipeline_create_info;
	pipeline_create_info.logical_device = in_engine.device;
	pipeline_create_info.descriptor_set_layout = in_engine.descriptor_set_layout;
	pipeline_create_info.render_pass = in_engine.render_pass;
	pipeline_create_info.msaa_samples = in_engine.msaaSamples;
	pipeline_create_info.vert_shader_spv = vert_shader_code;
	pipeline_create_info.frag_shader_spv = frag_shader_code;
	as::vk::create_pipeline(pipeline_create_info, in_engine.graphics_pipeline);

	as::vk::command_pool_create_info command_pool_create_info;
	command_pool_create_info.physical_device = in_engine.physicalDevice;
	command_pool_create_info.logical_device = in_engine.device;
	command_pool_create_info.surface = &in_engine.surface;
	as::vk::create_command_pool(command_pool_create_info, in_engine.commandPool);

	// color and depth image data
	as::vk::create_image_resources(in_engine);

	as::vk::framebuffers_create_info framebuffers_create_info;
	framebuffers_create_info.color_image_view = in_engine.color_image.view;
	framebuffers_create_info.depth_image_view = in_engine.depth_image.view;
	framebuffers_create_info.logical_device = in_engine.device;
	framebuffers_create_info.render_pass = in_engine.render_pass;
	framebuffers_create_info.swap_chain_extent = in_engine.swapchain.extent;
	framebuffers_create_info.swap_chain_image_views = in_engine.swapchain.image_views;
	as::vk::create_framebuffers(framebuffers_create_info, in_engine.swapchain.framebuffers);

	as::vk::texture_image_create_info texture_image_create_info;
	strcpy(texture_image_create_info.texture_path, TEXTURE_PATH.c_str());
	texture_image_create_info.physical_device = in_engine.physicalDevice;
	texture_image_create_info.logical_device = in_engine.device;
	texture_image_create_info.graphics_queue = in_engine.graphicsQueue;
	texture_image_create_info.command_pool = in_engine.commandPool;
	texture_image_create_info.texture_image_memory = in_engine.texture.image_data.memory;
	as::vk::create_texture_image(texture_image_create_info, in_engine.texture);

	as::vk::image_view_create_info texture_image_view_create_info;
	texture_image_view_create_info.logical_device = in_engine.device;
	texture_image_view_create_info.image = in_engine.texture.image_data.image;
	texture_image_view_create_info.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
	texture_image_view_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
	texture_image_view_create_info.mip_levels = in_engine.texture.mip_levels;
	as::vk::create_image_view(texture_image_view_create_info, in_engine.texture.image_data.view);

	as::vk::sampler_create_info sampler_create_info;
	sampler_create_info.logical_device = in_engine.device;
	sampler_create_info.physical_device = in_engine.physicalDevice;
	sampler_create_info.mip_levels = in_engine.texture.mip_levels;
	as::vk::create_sampler(sampler_create_info, in_engine.texture.sampler);

	as::vk::load_model(MODEL_PATH.c_str(), in_engine.vertices, in_engine.indices);

	as::vk::vertex_buffer_create_info vertex_buffer_create_info;
	vertex_buffer_create_info.physical_device = in_engine.physicalDevice;
	vertex_buffer_create_info.logical_device = in_engine.device;
	vertex_buffer_create_info.queue = in_engine.graphicsQueue;
	vertex_buffer_create_info.command_pool = in_engine.commandPool;
	vertex_buffer_create_info.vertices = in_engine.vertices;
	as::vk::create_vertex_buffer(vertex_buffer_create_info, in_engine.vertex_buffer, in_engine.vertex_buffer_memory);

	as::vk::index_buffer_create_info index_buffer_create_info;
	index_buffer_create_info.physical_device = in_engine.physicalDevice;
	index_buffer_create_info.logical_device = in_engine.device;
	index_buffer_create_info.queue = in_engine.graphicsQueue;
	index_buffer_create_info.command_pool = in_engine.commandPool;
	index_buffer_create_info.indices = in_engine.indices;
	as::vk::create_index_buffer(index_buffer_create_info, in_engine.index_buffer, in_engine.index_buffer_memory);

	as::vk::uniform_buffers_create_info uniform_buffers_create_info;
	uniform_buffers_create_info.physical_device = in_engine.physicalDevice;
	uniform_buffers_create_info.logical_device = in_engine.device;
	uniform_buffers_create_info.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
	as::vk::create_uniform_buffers(uniform_buffers_create_info, in_engine.buffers, in_engine.memory);

	as::vk::create_descriptor_pool(in_engine.device, MAX_FRAMES_IN_FLIGHT, in_engine.descriptorPool);

	as::vk::descriptor_sets_create_info descriptor_sets_create_info;
	descriptor_sets_create_info.logical_device = in_engine.device;
	descriptor_sets_create_info.descriptor_pool = in_engine.descriptorPool;
	descriptor_sets_create_info.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
	descriptor_sets_create_info.descriptor_set_layout = in_engine.descriptor_set_layout;
	as::vk::create_descriptor_sets(descriptor_sets_create_info, in_engine.descriptorSets);

	as::vk::descriptor_sets_update_info descriptor_sets_update_info;
	descriptor_sets_update_info.logical_device = in_engine.device;
	descriptor_sets_update_info.image_view = in_engine.texture.image_data.view;
	descriptor_sets_update_info.image_sampler = in_engine.texture.sampler;
	descriptor_sets_update_info.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
	descriptor_sets_update_info.uniform_buffers = in_engine.buffers;
	as::vk::update_descriptor_sets(descriptor_sets_update_info, in_engine.descriptorSets);

	as::vk::command_buffers_create_info command_buffers_create_info;
	command_buffers_create_info.logical_device = in_engine.device;
	command_buffers_create_info.command_pool = in_engine.commandPool;
	command_buffers_create_info.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
	as::vk::create_command_buffers(command_buffers_create_info, in_engine.commandBuffers);

	as::vk::sync_objects_create_info sync_objects_create_info;
	sync_objects_create_info.logical_device = in_engine.device;
	sync_objects_create_info.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
	as::vk::create_sync_objects(sync_objects_create_info, in_engine.image_available_semaphores, in_engine.render_finished_semaphores, in_engine.in_flight_fences);
}

void as::vk::draw_frame(engine& in_engine, as::window& in_window)
{
	vkWaitForFences(in_engine.device, 1, &in_engine.in_flight_fences[in_engine.currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(in_engine.device, in_engine.swapchain.swapchainKHR, UINT64_MAX, in_engine.image_available_semaphores[in_engine.currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		as::vk::recreate_swapchain(in_engine, in_window);
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
	{
		AS_LOG(LV_ERROR,"failed to acquire swap chain image!");
	}

	as::vk::update_uniform_buffer(in_engine.currentFrame, in_engine);

	vkResetFences(in_engine.device, 1, &in_engine.in_flight_fences[in_engine.currentFrame]);

	vkResetCommandBuffer(in_engine.commandBuffers[in_engine.currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
	as::vk::record_command_buffer(in_engine.commandBuffers[in_engine.currentFrame], imageIndex, in_engine);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { in_engine.image_available_semaphores[in_engine.currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &in_engine.commandBuffers[in_engine.currentFrame];

	VkSemaphore signalSemaphores[] = { in_engine.render_finished_semaphores[in_engine.currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	CHECK_VK_RESULT(vkQueueSubmit(in_engine.graphicsQueue, 1, &submitInfo, in_engine.in_flight_fences[in_engine.currentFrame]));

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { in_engine.swapchain.swapchainKHR };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(in_engine.presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized) 
	{
		as::framebuffer_resized = false;
		as::vk::recreate_swapchain(in_engine, in_window);
	}
	else 
	{
		CHECK_VK_RESULT(result);
	}
	
	in_engine.currentFrame = (in_engine.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void as::vk::start_main_loop(engine& in_engine, as::window& in_window)
{
	while (!glfwWindowShouldClose(in_window.GLFW))
	{
		glfwPollEvents();
		as::vk::draw_frame(in_engine, in_window);
	}

	vkDeviceWaitIdle(in_engine.device);
}

void as::vk::cleanup_swapchain(engine& in_engine)
{
	std::vector<as::vk::image_data> images_data =
	{
		in_engine.color_image,
		in_engine.depth_image
	};
	as::vk::cleanup_swap_chain(in_engine.device, in_engine.swapchain.swapchainKHR, images_data, in_engine.swapchain.framebuffers, in_engine.swapchain.image_views);
}

void as::vk::recreate_swapchain(engine& in_engine, as::window& in_window)
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(in_window.GLFW, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(in_window.GLFW, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(in_engine.device);

	as::vk::cleanup_swapchain(in_engine);

	as::vk::create_swap_chain(&in_engine.swapchain.swapchainKHR, &in_engine.swapchain.images, &in_engine.swapchain.image_format, &in_engine.swapchain.extent, &in_engine.device, &in_engine.physicalDevice, &in_engine.surface, in_window.GLFW);
	as::vk::create_image_views(&in_engine.swapchain.image_views, &in_engine.swapchain.framebuffers, &in_engine.swapchain.images, &in_engine.swapchain.image_format, &in_engine.device);
	as::vk::create_image_resources(in_engine);
	as::vk::create_frame_buffers(in_engine.swapchain.framebuffers, in_engine.device, in_engine.swapchain.image_views, in_engine.color_image.view, in_engine.depth_image.view, in_engine.render_pass, in_engine.swapchain.extent);
}

void as::vk::record_command_buffer(VkCommandBuffer& commandBuffer, uint32_t& imageIndex, engine& in_engine)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	CHECK_VK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = in_engine.render_pass;
	renderPassInfo.framebuffer = in_engine.swapchain.framebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = in_engine.swapchain.extent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, in_engine.graphics_pipeline.pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)in_engine.swapchain.extent.width;
	viewport.height = (float)in_engine.swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = in_engine.swapchain.extent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	VkBuffer vertexBuffers[] = { in_engine.vertex_buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, in_engine.index_buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, in_engine.graphics_pipeline.layout, 0, 1, &in_engine.descriptorSets[in_engine.currentFrame], 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(in_engine.indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	CHECK_VK_RESULT(vkEndCommandBuffer(commandBuffer));
}

void as::vk::update_uniform_buffer(u32& currentImage, engine& in_engine)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	uniform_buffer_object ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), in_engine.swapchain.extent.width / (float)in_engine.swapchain.extent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(in_engine.device, in_engine.memory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(in_engine.device, in_engine.memory[currentImage]);
}

void as::vk::cleanup(engine& in_engine, as::window& in_window)
{
	as::vk::cleanup_swapchain(in_engine);

	vkDestroyPipeline(in_engine.device, in_engine.graphics_pipeline.pipeline, nullptr);
	vkDestroyPipelineLayout(in_engine.device, in_engine.graphics_pipeline.layout, nullptr);
	vkDestroyRenderPass(in_engine.device, in_engine.render_pass, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyBuffer(in_engine.device, in_engine.buffers[i], nullptr);
		vkFreeMemory(in_engine.device, in_engine.memory[i], nullptr);
	}

	vkDestroyDescriptorPool(in_engine.device, in_engine.descriptorPool, nullptr);

	vkDestroySampler(in_engine.device, in_engine.texture.sampler, nullptr);
	vkDestroyImageView(in_engine.device, in_engine.texture.image_data.view, nullptr);

	vkDestroyImage(in_engine.device, in_engine.texture.image_data.image, nullptr);
	vkFreeMemory(in_engine.device, in_engine.texture.image_data.memory, nullptr);

	vkDestroyDescriptorSetLayout(in_engine.device, in_engine.descriptor_set_layout, nullptr);

	vkDestroyBuffer(in_engine.device, in_engine.index_buffer, nullptr);
	vkFreeMemory(in_engine.device, in_engine.index_buffer_memory, nullptr);

	vkDestroyBuffer(in_engine.device, in_engine.vertex_buffer, nullptr);
	vkFreeMemory(in_engine.device, in_engine.vertex_buffer_memory, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(in_engine.device, in_engine.render_finished_semaphores[i], nullptr);
		vkDestroySemaphore(in_engine.device, in_engine.image_available_semaphores[i], nullptr);
		vkDestroyFence(in_engine.device, in_engine.in_flight_fences[i], nullptr);
	}

	vkDestroyCommandPool(in_engine.device, in_engine.commandPool, nullptr);

	vkDestroyDevice(in_engine.device, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(in_engine.instance, in_engine.debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(in_engine.instance, in_engine.surface, nullptr);
	vkDestroyInstance(in_engine.instance, nullptr);

	glfwDestroyWindow(in_window.GLFW);

	glfwTerminate();
}

void as::vk::create_image_resources(engine& in_engine)
{
	as::vk::image_create_info color_image_create_info;
	color_image_create_info.physical_device = in_engine.physicalDevice;
	color_image_create_info.logical_device = in_engine.device;
	color_image_create_info.height = in_engine.swapchain.extent.height;
	color_image_create_info.width = in_engine.swapchain.extent.width;
	color_image_create_info.mip_levels = 1;
	color_image_create_info.num_samples = in_engine.msaaSamples;
	color_image_create_info.format = in_engine.swapchain.image_format;
	color_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	color_image_create_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	color_image_create_info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	as::vk::create_image(color_image_create_info, in_engine.color_image);

	as::vk::image_view_create_info color_image_view_create_info;
	color_image_view_create_info.logical_device = in_engine.device;
	color_image_view_create_info.image = in_engine.color_image.image;
	color_image_view_create_info.mip_levels = 1;
	color_image_view_create_info.format = in_engine.swapchain.image_format;
	color_image_view_create_info.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
	as::vk::create_image_view(color_image_view_create_info, in_engine.color_image.view);

	VkFormat supported_depth_format = as::vk::find_depth_format(in_engine.physicalDevice);
	as::vk::image_create_info depth_image_create_info = color_image_create_info;
	depth_image_create_info.format = supported_depth_format;
	depth_image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	as::vk::create_image(depth_image_create_info, in_engine.depth_image);
	as::vk::image_view_create_info depth_image_view_create_info = color_image_view_create_info;
	depth_image_view_create_info.image = in_engine.depth_image.image;
	depth_image_view_create_info.format = supported_depth_format;
	depth_image_view_create_info.aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
	as::vk::create_image_view(depth_image_view_create_info, in_engine.depth_image.view);
}

void as::vk::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func) 
	{
		func(instance, debugMessenger, pAllocator);
	}
}