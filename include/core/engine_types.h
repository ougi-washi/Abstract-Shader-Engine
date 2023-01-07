#pragma once

#include "types.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <chrono>

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
	};

	struct shader
	{
		i32 vertex_shader;
		i32 fragment_shader;
		std::vector<as::uniform> uniforms; // to change from vector
		std::vector<as::texture*> textures; // to change from vector
	};

	struct object
	{
		f32* vertices = nullptr;
		i64 vertices_size = 0;
		u32* indices = nullptr;
		i64 indices_size = 0;
		as::shader* shader_ptr = nullptr;
		u32 VBO; // Vertex Buffer Object
		u32 EBO; // Element Buffer Object
	};
	
	namespace timer
	{
		struct handle
		{
			std::chrono::system_clock::time_point start;
		};
	};
};