#pragma once

#include "types.h"

//GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace as
{
	/** Transforms */
	typedef glm::vec3 location;
	typedef glm::vec3 direction;
	typedef glm::vec3 rotation;
	typedef glm::vec4 quaternion;
	typedef glm::vec3 scale;
	/** Colors */
	typedef glm::vec3 color;

	struct time_stamp
	{
		f32 start;
		f32 end;
	};

	struct camera
	{
		location origin;
		location lower_left_corner;
		location horizontal, vertical;
		location u, v, w;

		time_stamp time; // shutter open/close time
		f32 lens_radius;
	};

	struct camera_create_info
	{
		location look_from;
		location look_at; 
		direction vup;

		f32 vfov;
		f32 aspect_ratio;
		f32 aperture;
		f32 focus_dist;
		time_stamp time;
	};

	struct ray
	{
		location loc;
		direction dir;
		f32 time;
	};

	struct material
	{
		color albedo;
		f32 metal;
	};

	struct sphere
	{
		location center;
		f32 radius;
		material* mat;
	};
}
