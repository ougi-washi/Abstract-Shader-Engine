#pragma once
#include "scripting.h"
#include "engine_core.h"

namespace as
{
	namespace script
	{
		// general
		std::string variable_enum_to_string(const variable_type& in_type);
		variable_type variable_string_to_enum(const std::string& in_type_str);
		bool parse_file(const std::string& path, const bool& absolute_path, as::script::entity& out_entity);
		void delete_entity_data(as::script::entity& entity);

		// types and getters
		bool get_model_from_entity(const as::script::entity& entity, as::model& out_model);

		// render
		bool draw(const std::vector<as::script::entity>& entities, const as::camera& camera);
	};
};