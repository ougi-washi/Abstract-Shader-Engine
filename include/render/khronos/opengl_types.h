#ifndef _OPENGL_
#define _OPENGL_

#include "types.h"

//#include <glad/gl.h>

// GLFW
//#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <optional> // TODO: REMOVE
#include <array> // TODO: REMOVE
#include <vector> // TODO: REMOVE
#include <unordered_map>// TODO: REMOVE
#include <functional> // TODO: REMOVE

namespace as
{

	typedef std::vector<char> spv;

	struct vertex 
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 tex_coord;

		bool operator==(const vertex& other) const 
		{
			return pos == other.pos && color == other.color && tex_coord == other.tex_coord;
		}
	};

	struct uniform_buffer_object 
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};


	struct glfw
	{
		//GLFWwindow* window;
	};

	namespace gl
	{
		
	};
};

namespace std 
{
	template<> struct hash<::as::vertex> 
	{
		size_t operator()(::as::vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.tex_coord) << 1);
		}
	};
};

#endif //_OPENGL_
