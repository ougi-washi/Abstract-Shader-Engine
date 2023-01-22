#pragma once
#include "scripting.h"

namespace as
{
	namespace script
	{
		std::string variable_enum_to_string(const variable_type& in_type);
		variable_type variable_string_to_enum(const std::string& in_type_str);
		bool parse_file(const std::string& path, const bool& absolute_path, as::script::entity& out_entity);
		void clean(as::script::entity& entity);
	};
};