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
			WORLD = 1,
			MODEL = 2,
			SHADER = 3,
			TEXTURE = 4
		};
		static std::vector<std::string> variable_type_strings = { "none", "world", "model", "shader", "texture"};

		struct entity
		{
			variable_type type;
			std::vector<script::entity> sub_entities;

			void* data_ptr = nullptr;
			void* fn_ptr = nullptr; // currently not in use
		};
	};
};