// internal dependencies (engine)
#include "engine_utility.h"
#include "engine_core.h"
#include "rlgl.h"

void as::init_window(const u16& width, const u16& height, const char* title)
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(width, height, title);
}

void as::init_gl()
{
	rlEnableDepthTest();
	rlEnableColorBlend();
	rlSetBlendMode(RL_BLEND_ALPHA);
	rlEnableBackfaceCulling();
}

as::engine_entity_pool* as::init_engine_entity_pool()
{
	engine_memory_pool = (as::engine_entity_pool*)AS_MALLOC(sizeof(as::engine_entity_pool));
	return engine_memory_pool;
}

void as::clear_engine_entity_pool()
{
	if (engine_memory_pool)
	{
		for (u16 i = 0 ; i < MAX_SHADER_POOL_SIZE ; i++)
		{
			UnloadShader(engine_memory_pool->shaders[i].data);
		}
		for (u16 i = 0 ; i < MAX_MODEL_POOL_SIZE ; i++)
		{
			UnloadModel(engine_memory_pool->models[i].data);
		}
		AS_FREE(engine_memory_pool);
		engine_memory_pool = nullptr;
	}
}

std::string get_path(const json& json_data, const char* path_json_type)
{
	if (json_data.contains(path_json_type))
	{
		std::string path = json_data[path_json_type].get<std::string>();
		if (json_data.contains("absolute_path"))
		{
			if (json_data["absolute_path"].get<bool>())
			{
				return path;
			}
			else
			{
				return as::util::get_current_path() + "/../" + path;
			}
		}
	}
	return std::string();
}

bool get_bool(const json& json_data, const char* json_field_name, bool& out_bool)
{
	if (json_data.contains(json_field_name))
	{
		out_bool = json_data[json_field_name].get<bool>();
		return true;
	}
	return false;
}

bool get_float(const json& json_data, const char* json_field_name, f32& out_float)
{
	if (json_data.contains(json_field_name))
	{
		out_float = json_data[json_field_name].get<f32>();
		return true;
	}
	return false;
}

bool get_vec3(const json& json_data, const char* json_field_name, Vector3& out_vector)
{
	if (json_data.contains(json_field_name) && json_data[json_field_name].size() >= 3)
	{
		out_vector.x = json_data[json_field_name][0];
		out_vector.y = json_data[json_field_name][1];
		out_vector.z = json_data[json_field_name][2];
		return true;
	}
	return false;
}

bool get_vec2(const json& json_data, const char* json_field_name, Vector2& out_vector)
{
	if (json_data.contains(json_field_name) && json_data[json_field_name].size() >= 2)
	{
		out_vector.x = json_data[json_field_name][0];
		out_vector.y = json_data[json_field_name][1];
		return true;
	}
	return false;
}

bool get_transform(const json& json_data, Transform& out_transform)
{
	if (json_data.contains("transform"))
	{
		json transform_json_data = json_data["transform"];
		bool is_valid_location = get_vec3(transform_json_data, "location", out_transform.translation);
		Vector3 rotation = Vector3();
		bool is_valid_rotation = get_vec3(transform_json_data, "rotation", rotation);
		if (is_valid_rotation)
		{
			out_transform.rotation = QuaternionFromEuler(rotation.x, rotation.y, rotation.z);
		}
		bool is_valid_scale = get_vec3(transform_json_data, "scale", out_transform.scale);
		return is_valid_location && is_valid_rotation && is_valid_scale;
	}
	return false;
}

std::string as::variable_enum_to_string(const as::entity_type& in_type)
{
	return entity_type_strings[in_type];
}

as::entity_type as::variable_string_to_enum(const std::string& in_type_str)
{
	for (u8 i = 0; i < entity_type_strings.size(); i++)
	{
		if (entity_type_strings[i] == in_type_str)
		{
			return (entity_type)i;
		}
	}
	return as::entity_type::NONE;
}


as::entity_type get_type(const json& json_data)
{
	if (json_data.contains("type"))
	{
		std::string entity_type = json_data["type"].get<std::string>();
		return as::variable_string_to_enum(entity_type);
	}
	return as::NONE;
}

bool as::get_updated_path(const char* in_path, const bool& absolute_path, char* out_path)
{
	if (in_path && out_path)
	{
		if (absolute_path)
		{
			strcpy(out_path, in_path);
		}
		else
		{
			std::string current_path = as::util::get_current_path();
			snprintf(out_path, MAX_PATH_SIZE, "%s/../%s", current_path.c_str(), in_path);
		}
		return true;
	}
	return false;
}

bool get_parsed_data(const char* path, const bool& absolute_path, char* out_updated_path, json & out_json_data)
{
	if (path && out_updated_path)
	{
		as::get_updated_path(path, absolute_path, out_updated_path);
		out_json_data = as::util::read_json_file(out_updated_path);
		return true;
	}
	return false;
}

#define PARSE_UPDATE_PATH(_path, _absolute_path,_updated_path, _out_json)\
{\
	AS_LOG(LV_LOG, "Parse file [" + std::string(_path) + "]");\
	char _current_path[MAX_PATH_SIZE];\
	json _out_json = json();\
	if (!get_parsed_data(_path, _absolute_path, _current_path, _out_json))\
	{\
		AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");\
	}\
}

as::world* as::get_world_from_file(const char* path, const bool& absolute_path)
{
	AS_LOG(LV_LOG, "Parse file [" + std::string(path) + "]");
	char current_path[MAX_PATH_SIZE];
	json json_data = json();
	if (!get_parsed_data(path, absolute_path, current_path, json_data))
	{
		AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
		return nullptr;
	}

	as::world* out_world = nullptr;
	if (get_type(json_data) == as::entity_type::WORLD)
	{
		out_world = get_empty_world_from_pool();

		if (!out_world)
		{
			AS_LOG(LV_WARNING, "Reached max pool size for worlds, cannot create world");
			return nullptr;
		}

		AS_SET_VALID_PTR(out_world);
		if (json_data.contains("models"))
		{
			std::vector<std::string> models_file_paths = json_data["models"].get<std::vector<std::string>>();
			out_world->models_count = models_file_paths.size();
			for (u16 i = 0; i < out_world->models_count; i++)
			{
				as::model* found_model = get_model_from_file(models_file_paths[i].c_str(), absolute_path);
				if (found_model)
				{
					add_model_to_world(out_world, found_model);
				}
			}
		}
		if (json_data.contains("lights"))
		{
			std::vector<std::string> lights_file_paths = json_data["lights"].get<std::vector<std::string>>();
			out_world->lights_count = lights_file_paths.size();
			for (u16 i = 0; i < out_world->lights_count; i++)
			{
				as::light* found_light = get_light_from_file(lights_file_paths[i].c_str(), absolute_path);
				if (found_light)
				{
					add_light_to_world(out_world, found_light);
				}
			}
		}
		if (json_data.contains("cameras"))
		{
			std::vector<std::string> cameras_file_paths = json_data["cameras"].get<std::vector<std::string>>();
			out_world->cameras_count = cameras_file_paths.size();
			for (u16 i = 0; i < out_world->cameras_count; i++)
			{
				as::camera* found_camera = get_camera_from_file(cameras_file_paths[i].c_str(), absolute_path);
				if (found_camera)
				{
					add_camera_to_world(out_world, found_camera);
				}
			}
		}
		if (json_data.contains("is_active"))
		{
			bool is_active = false;
			if (get_bool(json_data, "is_active", is_active))
			{
				out_world->is_active = is_active;
			}
		}
	}
	return out_world;
}

as::model* as::get_model_from_file(const char* path, const bool& absolute_path)
{
	AS_LOG(LV_LOG, "Parse file [" + std::string(path) + "]");
	char current_path[MAX_PATH_SIZE];
	json json_data = json();
	if (!get_parsed_data(path, absolute_path, current_path, json_data))
	{
		AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
		return nullptr;
	}

	as::model* out_model = nullptr;
	if (get_type(json_data) == as::entity_type::MODEL)
	{
		out_model = get_empty_model_from_pool();
		if (!out_model)
		{
			AS_LOG(LV_WARNING, "Reached max pool size for models, cannot create model");
			return nullptr;
		}

		AS_SET_VALID_PTR(out_model);
		if (json_data.contains("path"))
		{
			std::string model_path = as::util::get_current_path() + "/../" + json_data["path"].get<std::string>();
			out_model->data = LoadModel(model_path.c_str());
		}
		if (json_data.contains("shaders"))
		{
			std::vector<std::string> shaders = json_data["shaders"].get<std::vector<std::string>>();
			for (u16 i = 0; i < shaders.size(); i++)
			{
				as::shader* found_shader = get_shader_from_file(shaders[i].c_str(), absolute_path);
				if (found_shader && out_model->data.materialCount > i)
				{
					out_model->data.materials[i].shader = found_shader->data;
					if (found_shader->is_translucent)
					{
						out_model->flags = static_cast<as::model::flag>(out_model->flags | model::flag::TRANSLUCENT);
					}
				}
			}
			if (out_model->data.materialCount > shaders.size())
			{
				as::shader* found_shader = get_shader_from_file(DEFAULT_SHADER_PATH, absolute_path);
				if (found_shader)
				{
					for (u16 i = shaders.size(); i < out_model->data.materialCount; i++)
					{
						out_model->data.materials[i].shader = found_shader->data;
						if (found_shader->is_translucent)
						{
							out_model->flags = static_cast<as::model::flag>(out_model->flags | model::flag::TRANSLUCENT);
						}
					}
				}
			}
		}
		else
		{
			AS_LOG(LV_WARNING, "Model json file does not have shaders");
			return nullptr;
		}
		out_model->data.transform = MatrixIdentity();
		Vector3 out_vector = Vector3();
		if (get_vec3(json_data, "scale", out_vector))
		{
			as::apply_scale(out_vector, out_model->data.transform);
		};
		if (get_vec3(json_data, "rotation", out_vector))
		{
			as::apply_rotation(out_vector, out_model->data.transform);
		};
		if (get_vec3(json_data, "location", out_vector))
		{
			as::apply_location(out_vector, out_model->data.transform);
		};
	}
	return out_model;
}


as::shader* as::get_shader_from_file(const char* path, const bool& absolute_path)
{
	AS_LOG(LV_LOG, "Parse file [" + std::string(path) + "]");
	char current_path[MAX_PATH_SIZE];
	json json_data = json();
	if (!get_parsed_data(path, absolute_path, current_path, json_data))
	{
		AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
		return nullptr;
	}

	as::shader* out_shader = nullptr;
	if (get_type(json_data) == as::entity_type::SHADER)
	{
		out_shader = get_empty_shader_from_pool();
		if (!out_shader)
		{
			AS_LOG(LV_WARNING, "Reached max pool size for shaders, cannot create shader");
			return nullptr;
		}

		AS_SET_VALID_PTR(out_shader);
		std::string vertex_path;
		std::string fragment_path;
		if (json_data.contains("vertex_path"))
		{
			vertex_path = as::util::get_current_path() + "/../" + json_data["vertex_path"].get<std::string>();
		}
		else
		{
			AS_LOG(LV_WARNING, "Shader json file does not have vertex shader");
			return nullptr;
		}
		if (json_data.contains("fragment_path"))
		{
			fragment_path = as::util::get_current_path() + "/../" + json_data["fragment_path"].get<std::string>();
		}
		else
		{
			AS_LOG(LV_WARNING, "Shader json file does not have fragment shader");
			return nullptr;
		}
		{
			bool out_is_translucent = false;
			if (get_bool(json_data, "is_translucent", out_is_translucent))
			{
				out_shader->is_translucent = out_is_translucent;
			}
		}
		out_shader->data = LoadShader(vertex_path.c_str(), fragment_path.c_str());
		if (out_shader->data.id <= 0)
		{
			AS_LOG(LV_WARNING, "Could not create the shader");
			UnloadShader(out_shader->data);
			out_shader = nullptr;
		}
	}
	return out_shader;
}

as::texture* as::get_texture_from_file(const char* path, const bool& absolute_path)
{
	AS_LOG(LV_LOG, "Parse file [" + std::string(path) + "]");
	char current_path[MAX_PATH_SIZE];
	json json_data = json();
	if (!get_parsed_data(path, absolute_path, current_path, json_data))
	{
		AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
		return nullptr;
	}

	as::texture* out_texture = nullptr;
	if (get_type(json_data) == as::entity_type::TEXTURE)
	{
		if (json_data.contains("path"))
		{
			std::string texture_path = as::util::get_current_path() + "/../" + json_data["path"].get<std::string>();

			out_texture = get_empty_texture_from_pool();
			if (!out_texture)
			{
				AS_LOG(LV_WARNING, "Reached max pool size for textures, cannot create texture");
				return nullptr;
			}
			bool found_texture = false;
			// TODO: caching
			//for (size_t i = 0; i < allocations_count; i++)
			//{
			//	as::entity* found_entity_ptr = static_cast<as::entity*>(allocations[i]->ptr);
			//	if (found_entity_ptr && found_entity_ptr->type == as::TEXTURE)
			//	{
			//		as::texture* found_texture_ptr = static_cast<as::texture*>(found_entity_ptr->data_ptr);
			//		std::string path_string = util::get_current_path() + "/../" + std::string(texture_path);
			//		if (found_texture_ptr && strcmp(found_texture_ptr->path, path_string.c_str()) == 0)
			//		{
			//			AS_LOG(LV_LOG, "Texture " + texture_path + "already defined, using the found pointer.");
			//			out_entity = found_entity_ptr;
			//			found_texture = true;
			//		}
			//	}
			//}
			if (!found_texture)
			{
				out_texture->data = LoadTexture(texture_path.c_str());
				if (out_texture->data.id <= 0)
				{
					AS_SET_VALID_PTR(out_texture);
					return out_texture;
				}
				else
				{
					AS_LOG(LV_WARNING, "Could not load the texture");
				}
			}
		}
		else
		{
			AS_LOG(LV_WARNING, "Texture json file does not have path");
		}
	}
	return out_texture;
}

as::camera* as::get_camera_from_file(const char* path, const bool& absolute_path)
{
	AS_LOG(LV_LOG, "Parse file [" + std::string(path) + "]");
	char current_path[MAX_PATH_SIZE];
	json json_data = json();
	if (!get_parsed_data(path, absolute_path, current_path, json_data))
	{
		AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
		return nullptr;
	}

	as::camera* out_camera = nullptr;
	if (get_type(json_data) == as::entity_type::CAMERA)
	{
		out_camera = get_empty_camera_from_pool();
		if (!out_camera)
		{
			AS_LOG(LV_WARNING, "Reached max pool size for cameras, cannot create camera");
			return nullptr;
		}
		out_camera->data = { 0 };
		get_vec3(json_data, "position", out_camera->data.position);
		get_vec3(json_data, "target", out_camera->data.target);
		if (json_data.contains("is_active"))
		{
			out_camera->is_active = json_data["is_active"].get<bool>();
		}
		out_camera->data.up = Vector3(0.0f, 1.0f, 0.0f);          // Camera up vector (rotation towards target)
		out_camera->data.fovy = 75.f;                                // Camera field-of-view Y
		out_camera->data.projection = CAMERA_PERSPECTIVE;                   // Camera mode type
		AS_IS_INVALID_PTR(out_camera);
	}
	return out_camera;
}

as::light* as::get_light_from_file(const char* path, const bool& absolute_path)
{
	AS_LOG(LV_LOG, "Parse file [" + std::string(path) + "]");
	char current_path[MAX_PATH_SIZE];
	json json_data = json();
	if (!get_parsed_data(path, absolute_path, current_path, json_data))
	{
		AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
		return nullptr;
	}

	as::light* out_light = nullptr;
	if (get_type(json_data) == as::entity_type::LIGHT)
	{
		out_light = get_empty_light_from_pool();
		if (out_light)
		{
			AS_SET_VALID_PTR(out_light);
			get_vec3(json_data, "location", out_light->location);
			get_float(json_data, "intensity", out_light->intensity);
			get_float(json_data, "attenuation", out_light->attenuation);
			get_vec3(json_data, "color", out_light->color);
		}
		else
		{
			AS_LOG(LV_WARNING, "Reached max pool size for cameras, cannot create camera");
		}
	}
	return out_light;
}

bool as::add_model_to_world(as::world* world, as::model* model)
{
	if (world && model)
	{
		for (u16 i = 0 ; i < world->models_count ; i++)
		{
			if (!world->models[i])
			{
				world->models[i] = model;
				return true;
			}
		}
	}
	AS_LOG(LV_WARNING, "Could not add model to the world, the maximum reached, please increase the max models per world size");
	return false;
}


bool as::add_light_to_world(as::world* world, as::light* light)
{
	if (world && light)
	{
		for (u16 i = 0; i < world->lights_count; i++)
		{
			if (!world->lights[i])
			{
				world->lights[i] = light;
				return true;
			}
		}
	}
	AS_LOG(LV_WARNING, "Could not add light to the world, the maximum reached, please increase the max lights per world size");
	return false;
}

bool as::add_camera_to_world(as::world* world, as::camera* camera)
{
	if (world && camera)
	{
		for (u16 i = 0; i < world->cameras_count; i++)
		{
			if (!world->cameras[i])
			{
				world->cameras[i] = camera;
				return true;
			}
		}
	}
	AS_LOG(LV_WARNING, "Could not add camera to the world, the maximum reached, please increase the max cameras per world size");
	return false;
}

void as::apply_location(const Vector3& location, Matrix& transform_matrix)
{
	Matrix translation = MatrixTranslate(location.x, location.y, location.z);
	transform_matrix = MatrixMultiply(transform_matrix, translation);
}

void as::apply_rotation(const Vector3& rotation, Matrix& transform_matrix)
{
	Matrix rotationX = MatrixRotateX(DEG2RAD * rotation.x);
	Matrix rotationY = MatrixRotateY(DEG2RAD * rotation.y);
	Matrix rotationZ = MatrixRotateZ(DEG2RAD * rotation.z);
	transform_matrix = MatrixMultiply(transform_matrix, rotationX);
	transform_matrix = MatrixMultiply(transform_matrix, rotationY);
	transform_matrix = MatrixMultiply(transform_matrix, rotationZ);
}

void as::apply_scale(const Vector3& scale, Matrix& transform_matrix)
{
	Matrix scale_matrix = MatrixScale(scale.x, scale.y, scale.z);
	transform_matrix = MatrixMultiply(transform_matrix, scale_matrix);
}

void as::apply_transform(const Transform& transform, Matrix& transform_matrix)
{
	apply_location(transform.translation, transform_matrix);
	apply_rotation(QuaternionToEuler(transform.rotation), transform_matrix);
	apply_scale(transform.scale, transform_matrix);
}

void as::set_valid(as::entity_data& entity_data)
{
	entity_data.is_valid = true;
}

void as::set_invalid(as::entity_data& entity_data)
{
	entity_data.is_valid = false;
}

bool as::is_valid(const as::entity_data& entity_data)
{
	return entity_data.is_valid;
}

bool as::is_invalid(const as::entity_data& entity_data)
{
	return !entity_data.is_valid;
}

as::camera* as::find_active_camera(const as::world* world)
{
	for (u32 i = 0 ; i < world->cameras_count ; i++)
	{
		if (world->cameras[i] && world->cameras[i]->is_active && AS_IS_INVALID_PTR(world->cameras[i]))
		{
			return world->cameras[i];
		}
	}
	return nullptr;
}

void as::update_lights_uniforms(const Shader& shader, as::light** lights, const u32& lights_count)
{
	if (lights_count > 0)
	{
		{
			const i32 light_count_loc = GetShaderLocation(shader, "lights_count");
			SetShaderValue(shader, light_count_loc, &lights_count, SHADER_UNIFORM_INT);
		}

		for (u32 i = 0; i < lights_count; i++)
		{
			if (lights[i])
			{
				char buffer[64];
				i32 light_data_loc = -1;

				sprintf(buffer, "lights[%i].location", i);
				light_data_loc = GetShaderLocation(shader, buffer);
				SetShaderValue(shader, light_data_loc, &lights[i]->location, SHADER_UNIFORM_VEC3);

				sprintf(buffer, "lights[%i].color", i);
				light_data_loc = GetShaderLocation(shader, buffer);
				SetShaderValue(shader, light_data_loc, &lights[i]->color, SHADER_UNIFORM_VEC3);

				sprintf(buffer, "lights[%i].intensity", i);
				light_data_loc = GetShaderLocation(shader, buffer);
				SetShaderValue(shader, light_data_loc, &lights[i]->intensity, SHADER_UNIFORM_FLOAT);

				sprintf(buffer, "lights[%i].attenuation", i);
				light_data_loc = GetShaderLocation(shader, buffer);
				SetShaderValue(shader, light_data_loc, &lights[i]->attenuation, SHADER_UNIFORM_FLOAT);
			}
		}
	}
}

bool as::draw(const as::world* world)
{
	if (world)
	{
		as::camera* camera_to_use = find_active_camera(world);

		AS_ASSERT(world->models_count > 0, "Cannot draw 0 models, check the world content");
		AS_ASSERT(camera_to_use, "Cannot draw with no active camera, check the world content");

		as::light** lights = (as::light**)AS_MALLOC(sizeof(as::light*) * world->lights_count);
		memcpy(lights, world->lights, sizeof(as::light*) * world->lights_count);

		BeginDrawing();
		clear_background();
		BeginMode3D(camera_to_use->data);

		// Render opaque meshes
		for (u16 i = 0; i < MAX_MODELS_PER_WORLD; i++)
		{
			if (world->models[i] && AS_IS_VALID_PTR(world->models[i]) && !(world->models[i]->flags & model::flag::TRANSLUCENT))
			{
				for (u32 j = 0; j < world->models[i]->data.materialCount; j++)
				{
					update_lights_uniforms(world->models[i]->data.materials[j].shader, lights, world->lights_count);
				}
				Vector3 position = Vector3(world->models[i]->data.transform.m12, world->models[i]->data.transform.m13, world->models[i]->data.transform.m14);
				DrawModel(world->models[i]->data, position, 1.0, WHITE);
			}
		}
		// Render translucent meshes
		for (u16 i = 0; i < MAX_MODELS_PER_WORLD; i++)
		{
			if (world->models[i] && AS_IS_VALID_PTR(world->models[i]) && (world->models[i]->flags & model::flag::TRANSLUCENT))
			{
				for (u32 j = 0; j < world->models[i]->data.materialCount; j++)
				{
					update_lights_uniforms(world->models[i]->data.materials[j].shader, lights, world->lights_count);
				}
				Vector3 position = Vector3(world->models[i]->data.transform.m12, world->models[i]->data.transform.m13, world->models[i]->data.transform.m14);
				DrawModel(world->models[i]->data, position, 1.0, WHITE);
			}
		}
		DrawGrid(30, 10.f);
		EndMode3D();
		EndDrawing();

		AS_FREE(lights);
		lights = nullptr;
	}
	return false;
}

void as::clear_background()
{
	ClearBackground(Color(0.f, 0.f, 0.f));
}

as::world* as::get_empty_world_from_pool()
{
	// find world pointer
	for (u16 i = 0; i < MAX_WORLD_POOL_SIZE; i++)
	{
		if (AS_IS_INVALID(engine_memory_pool->worlds[i]))
		{
			return &engine_memory_pool->worlds[i];
		}
	}
	return nullptr;
}

as::model* as::get_empty_model_from_pool()
{
	// find model pointer
	for (u16 i = 0; i < MAX_MODEL_POOL_SIZE; i++)
	{
		if (AS_IS_INVALID(engine_memory_pool->models[i]))
		{
			return &engine_memory_pool->models[i];
		}
	}
	return nullptr;
}

as::shader* as::get_empty_shader_from_pool()
{
	// find shader pointer
	for (u16 i = 0; i < MAX_SHADER_POOL_SIZE; i++)
	{
		if (AS_IS_INVALID(engine_memory_pool->shaders[i]))
		{
			return &engine_memory_pool->shaders[i];
		}
	}
	return nullptr;
}

as::texture* as::get_empty_texture_from_pool()
{
	// find texture pointer
	for (u16 i = 0; i < MAX_TEXTURE_POOL_SIZE; i++)
	{
		if (AS_IS_INVALID(engine_memory_pool->textures[i]))
		{
			return &engine_memory_pool->textures[i];
		}
	}
	return nullptr;
}

as::camera* as::get_empty_camera_from_pool()
{
	// find camera pointer
	for (u16 i = 0; i < MAX_CAMERA_POOL_SIZE; i++)
	{
		if (AS_IS_INVALID(engine_memory_pool->cameras[i]))
		{
			return &engine_memory_pool->cameras[i];
		}
	}
	return nullptr;
}

as::light* as::get_empty_light_from_pool()
{
	// find light pointer
	for (u16 i = 0; i < MAX_LIGHT_POOL_SIZE; i++)
	{
		if (AS_IS_INVALID(engine_memory_pool->lights[i]))
		{
			return &engine_memory_pool->lights[i];
		}
	}
	return nullptr;
}
