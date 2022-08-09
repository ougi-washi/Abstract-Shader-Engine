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
	/** Transforms 3D */
	typedef glm::vec3 location;
	typedef glm::vec3 direction;
	typedef glm::vec3 rotation;
	typedef glm::vec4 quaternion;
	typedef glm::vec3 scale;
	/** Transform 2D */
	typedef glm::vec2 location2D;
	typedef glm::vec2 direction2D;
	typedef glm::vec2 rotation2D;
	typedef glm::vec2 scale2D;
	/** Colors */
	typedef glm::vec3 color;
	typedef glm::vec3 rgb;
	typedef glm::vec4 rgba;

	/** forward declaration */

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
		f32 distance;
	};

	struct material
	{
		color albedo;
		f32 metal;
	};

	struct hit_record
	{
		location loc;
		direction normal;
		material* mat = nullptr;
		float time;
		u8 front_face : 1;
	};

	struct sphere
	{
		location center;
		f32 radius;
		material* mat = nullptr;
	};

	struct world
	{
		sphere* spheres = nullptr;
		u16 sphere_count = 0;
	};
}
