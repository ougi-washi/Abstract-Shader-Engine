#pragma once

#include "types.h"

#include "raylib.h"
#include "raymath.h"
#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

#include "json/json.hpp"
using json = nlohmann::json;
#define JSON_NOEXCEPTION 1

#include <vector>
#include <chrono>

// POOL SIZES
#define MAX_WORLD_POOL_SIZE 8
#define MAX_MODEL_POOL_SIZE 128
#define MAX_SHADER_POOL_SIZE 128
#define MAX_TEXTURE_POOL_SIZE 128
#define MAX_LIGHT_POOL_SIZE 64
#define MAX_CAMERA_POOL_SIZE 16

#define MAX_BONE_INFLUENCE 4
#define MAX_TEXTURE_COUNT_PER_SHADER 256

#define MAX_MODELS_PER_WORLD 256
#define MAX_CAMERAS_PER_WORLD 16
#define MAX_SUB_WORLDS_PER_WORLD 4
#define MAX_LIGHTS_PER_WORLD 12 // NO DEFERRED YET

#define MAX_UNIFORMS_PER_SHADER 64
#define MAX_UNIFORM_SIZE 8
#define MAX_UNIFORM_NAME_SIZE 256
#define MAX_MESHES_PER_MODEL 1024

#define MAX_FILE_SIZE 30720
#define MAX_PATH_SIZE 2048
#define MAX_INCLUDES 10

namespace as
{
	//typedef enum {
	//	SHADER_UNIFORM_FLOAT = 0,       // Shader uniform type: float
	//	SHADER_UNIFORM_VEC2,            // Shader uniform type: vec2 (2 float)
	//	SHADER_UNIFORM_VEC3,            // Shader uniform type: vec3 (3 float)
	//	SHADER_UNIFORM_VEC4,            // Shader uniform type: vec4 (4 float)
	//	SHADER_UNIFORM_INT,             // Shader uniform type: int
	//	SHADER_UNIFORM_IVEC2,           // Shader uniform type: ivec2 (2 int)
	//	SHADER_UNIFORM_IVEC3,           // Shader uniform type: ivec3 (3 int)
	//	SHADER_UNIFORM_IVEC4,           // Shader uniform type: ivec4 (4 int)
	//	SHADER_UNIFORM_SAMPLER2D        // Shader uniform type: sampler2d
	//} ShaderUniformDataType;
	//ShaderUniformDataType
	static std::vector<std::string> uniform_type_strings =
	{
		"float",		// 0
		"vec2",	// 1
		"vec3",		// 2
		"vec4",		// 3
		"int",	// 4
		"ivec2",	// 5
		"ivec3",	// 6
		"ivec4",	// 7
		"texture",	// 8
	};

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
		char path[MAX_PATH_SIZE] = "";
		u8 is_valid : 1;
		u8 is_dirty : 1;

		// init
		entity_data() : is_valid(false), is_dirty(true) {};
	};

	struct texture
	{
		Texture data = {};
		as::entity_data entity_data = {};
	};

	struct uniform
	{
		char name[MAX_UNIFORM_NAME_SIZE] = "";
		ShaderUniformDataType type = ShaderUniformDataType::SHADER_UNIFORM_FLOAT;
		u64 value[MAX_UNIFORM_SIZE] = {};
	};

	struct shader
	{
		Shader data;
		uniform uniforms[MAX_UNIFORMS_PER_SHADER];
		u16 uniforms_count = 0;
		u8 is_translucent : 1;
		as::entity_data entity_data;

		// init
		shader() : data(Shader()), is_translucent(false) {};
	};

	struct material
	{
		Material data = {};
		as::entity_data entity_data = {};
	};

	struct model
	{
		enum flag : u8
		{
			NONE = 0x00,
			TRANSLUCENT = 0x01
		};

		Model data = {};
		as::shader* shaders[MAX_MESHES_PER_MODEL] = {};
		u16 shader_count = 0;
		model::flag flags = {}; // TODO: Move to raylib for meshes, rather than models
		as::entity_data entity_data = {};
	};
	
	struct camera
	{
		Camera data;
		u8 is_active : 1;
		as::entity_data entity_data = {};
	};

	struct shadow_map
	{
		u16 size = 1024;
		RenderTexture texture = {};
		Camera camera = {};
	};

	struct light
	{
		Vector3 location = Vector3(0.f);
		Vector3 color = Vector3(1.f);
		f32 intensity = 1.f;
		f32 attenuation = 3.f;
		as::shadow_map shadow_map = {};
		as::entity_data entity_data = {};
	};

	struct world;
	struct world
	{
		as::model* models[MAX_MODELS_PER_WORLD] = { nullptr };
		u16 models_count = 0;

		as::light* lights[MAX_LIGHTS_PER_WORLD] = { nullptr };
		u16 lights_count = 0;

		as::camera* cameras[MAX_CAMERAS_PER_WORLD] = { nullptr };
		u16 cameras_count = 0;

		as::world* sub_worlds[MAX_SUB_WORLDS_PER_WORLD] = { nullptr };
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

	struct engine_entity_pool
	{
		as::world worlds[MAX_WORLD_POOL_SIZE] = {};

		as::model models[MAX_MODEL_POOL_SIZE] = {};

		as::shader shaders[MAX_SHADER_POOL_SIZE] = {};

		as::texture textures[MAX_TEXTURE_POOL_SIZE] = {};

		as::light lights[MAX_LIGHT_POOL_SIZE] = {};

		as::camera cameras[MAX_CAMERA_POOL_SIZE] = {};
	};

};

extern as::engine_entity_pool* engine_memory_pool;
extern f32 current_time;

#define DEFAULT_WORLD_PATH "resources/objects/defaults/default_world.json"
#define DEFAULT_CAMERA_PATH "resources/objects/defaults/default_camera.json"
#define DEFAULT_SHADER_PATH "resources/objects/defaults/default_shader.json"
