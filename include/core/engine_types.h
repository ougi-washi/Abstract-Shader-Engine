#pragma once

#include "types.h"

#include "raylib.h"
#include "raymath.h"
#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

#include <vector>
#include <chrono>

#define MAX_BONE_INFLUENCE 4
#define MAX_TEXTURE_COUNT_PER_SHADER 256

#define MAX_MODELS_PER_WORLD 8192
#define MAX_CAMERAS_PER_WORLD 256
#define MAX_SUB_WORLDS_PER_WORLD 256
#define MAX_LIGHTS_PER_WORLD 64

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
	
	namespace ent
	{
		enum entity_type : u8
		{
			NONE = 0,
			WORLD = 1,
			MODEL = 2,
			SHADER = 3,
			MATERIAL = 4,
			TEXTURE = 5,
			CAMERA = 6,
			LIGHT = 7,
			MAX = 8
		};
	};

	// move to .cpp due to translation unit issue
	static std::vector<std::string> entity_type_strings =
	{
		"none",		// 0
		"world",	// 1
		"model",	// 2
		"shader",	// 3
		"material",	// 4
		"texture",	// 5
		"camera",	// 6
		"light",	// 7
		"max"		// 8
	};

	struct entity_data
	{
		void* start_fn = nullptr;
		void* tick_fn = nullptr;
		void* end_fn = nullptr;
		char path[256] = "";
	};

	//struct entity
	//{
	//	ent::entity_type type = ent::NONE;
	//	as::entity** sub_entities = nullptr;
	//	u32 sub_entities_count = 0;

	//	void* data_ptr = nullptr;
	//	void* fn_ptr = nullptr;
	//	char path[256] = "";
	//};

	struct texture
	{
		Texture data;
		as::entity_data entity_data;
	};

	struct shader
	{
		Shader data;
		as::entity_data entity_data;
	};

	struct material
	{
		Material data;
		as::entity_data entity_data;
	};

	struct model
	{
		Model data;
		as::entity_data entity_data;
	};
	
	struct camera
	{
		Camera data;
		u8 is_active : 1;
		as::entity_data entity_data;
	};

	struct light
	{
		Vector3 location = Vector3(0.f);
		Vector3 color = Vector3(1.f);
		f32 intensity = 1.f;
		f32 attenuation = 3.f;
		as::entity_data entity_data;
	};

	struct world
	{
		as::model models[MAX_MODELS_PER_WORLD];
		u16 models_count = 0;

		as::light lights[MAX_LIGHTS_PER_WORLD];
		u16 lights_count = 0;

		as::camera cameras[MAX_CAMERAS_PER_WORLD];
		u16 cameras_count = 0;

		as::world sub_worlds[MAX_SUB_WORLDS_PER_WORLD];
		u16 sub_worlds_count = 0;

		u8 is_active : 1;
		as::entity_data entity_data;
	};

	namespace timer
	{
		struct handle
		{
			std::chrono::system_clock::time_point start;
		};
	};
};

#define DEFAULT_WORLD_PATH "resources/objects/defaults/default_world.json"
#define DEFAULT_CAMERA_PATH "resources/objects/defaults/default_camera.json"
#define DEFAULT_SHADER_PATH "resources/objects/defaults/default_shader.json"
