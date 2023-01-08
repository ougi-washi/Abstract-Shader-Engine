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

	struct model
	{
		f32* vertices = nullptr;
		i64 vertices_size = 0;
		f32* indices = nullptr;
		i64 indices_size = 0;
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
	
	struct camera
	{
		// camera Attributes
		glm::vec3 position = glm::vec3(0.f);
		glm::vec3 front = glm::vec3(0.f, 0.f, -1.f);
		glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
		glm::vec3 right;
		glm::vec3 world_up;

		// euler Angles
		glm::vec3 rotation; // X : roll // Y : pitch // Z : yaw

		// camera options
		f32 movement_speed;
		f32 mouse_sensitivity;
		f32 zoom;
	};

	namespace timer
	{
		struct handle
		{
			std::chrono::system_clock::time_point start;
		};
	};
};