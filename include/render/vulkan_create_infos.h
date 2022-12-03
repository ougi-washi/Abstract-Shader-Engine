#ifndef _VULKAN_CREATE_INFO_
#define _VULKAN_CREATE_INFO_


#include "render/vulkan.h"

/** This file contains all the Vulkan create infos. in order to create OP_RESULT <func> (<params> , RESULT* outResult) */

namespace as
{
	namespace vk
	{
		struct vulkan_instance_create_info
		{
			char app_name[255] = "Abstract Shader Engine";
			char engine_name[255] = "Abstract Shader Engine";
			u8 enable_validation_layers : 1;
			const std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" }; // TODO: remove std
			vulkan_instance_create_info() : enable_validation_layers(true) {};
		};
	};
};

#endif //_VULKAN_CREATE_INFO_