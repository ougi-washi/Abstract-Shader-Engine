#pragma once

#include "types.h"
#include "glm/glm.hpp"
#include <vector>

namespace as
{
	struct shader
	{
		i32 vertex_shader;
		i32 fragment_shader;
	};

	struct object
	{
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> indices;
		as::shader* shader_ptr = nullptr;
		u32 VBO;
	};
};