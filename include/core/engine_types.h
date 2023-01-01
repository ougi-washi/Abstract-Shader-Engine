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
		float vertices[50000];
		float indices[50000];
		as::shader* shader_ptr = nullptr;
		u32 VBO;
	};
};