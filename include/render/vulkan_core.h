#include "render/vulkan.h"

namespace use
{
	void create_vulkan_instance(VkInstance* out_instance)
	{

		const VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO, 0, "VKComputeSample", 0, "", 0, VK_MAKE_VERSION(1, 0, 9) };
		const VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, 0, 0, &applicationInfo, 0, 0, 0, 0 };

		VkInstance instance;
		CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, 0, &instance));

		uint32_t physicalDeviceCount = 0;
		CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, 0));

		VkPhysicalDevice* const physicalDevices = (VkPhysicalDevice*)malloc(
			sizeof(VkPhysicalDevice) * physicalDeviceCount);
	}
};