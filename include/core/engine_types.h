#pragma once

#include "types.h"
#include "glm/glm.hpp"
#include <vector>

namespace as
{
	enum variable_type
	{
		FLOAT = 0x1406,
		INT = 0x1404,
		BOOL = 0x8B56,
		TEXTURE = 0x1702,
		VECTOR // ??
	};

	struct texture
	{
		u32 id;
		char* path;
		i32 width;
		i32 height;
		i32 number_of_channels;
	};

	struct uniform
	{
		u32 id;
		i32 size;
		variable_type type;
		u8 normalized : 1;
		i32 stride;
		u64 position = 0;
		as::texture* texture_ptr = nullptr; // usable if TEXTURE variable type
	};

	struct shader
	{
		i32 vertex_shader;
		i32 fragment_shader;
		std::vector<uniform> uniforms; // to change from vector
	};

	struct object
	{
		float* vertices = nullptr;
		i64 vertices_size = 0;
		float* indices = nullptr;
		i64 indices_size = 0;
		as::shader* shader_ptr = nullptr;
		u32 VBO; // Vertex Buffer Object
		u32 EBO; // Element Buffer Object
	};
};