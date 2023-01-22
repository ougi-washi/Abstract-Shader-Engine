#pragma once
#include "types.h"
#include <vector>

namespace as
{
	namespace script
	{
		enum variable_type : u8
		{
			NONE = 0,
			MODEL = 1,
			SHADER = 2,
			TEXTURE = 3
		};
		static std::vector<std::string> variable_type_strings = { "none", "model", "shader", "texture"};

		struct entity
		{
			variable_type type;
			std::vector<script::entity> sub_entities;

			void* data_ptr = nullptr;
			void* fn_ptr = nullptr; // currently not in use
		};
	};
};