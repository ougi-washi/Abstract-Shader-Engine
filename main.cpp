#include "render/vulkan_core.h"

int main() 
{
	use::vulkan_interface vk_interface;
	use::vulkan_interface_create_info vulkan_interface_create_info = {};
	CHECK_RESULT(use::init_vulkan(&vk_interface, vulkan_interface_create_info));
	
	vk_interface.instance;
}