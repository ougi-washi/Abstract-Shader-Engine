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
		float* vertices = nullptr;
		float* indices = nullptr;
		as::shader* shader_ptr = nullptr;
		u32 VBO; // Vertex Buffer Object
		u32 EBO; // Element Buffer Object
	};
};