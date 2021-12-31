#include <RayTracer/render.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

int main(int, char**) {
    
    render Render;
    Render.init();
    Render.process();

	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    glm::vec4 Position = glm::vec4(glm::vec3(0.0), 1.0);
    return 0;
}
