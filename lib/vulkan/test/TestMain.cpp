#include <vulkan/vulkan.h>

int main()
{
    // This isn't expected to run, but is expected to compile (and link) properly.
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    return 1;
}
