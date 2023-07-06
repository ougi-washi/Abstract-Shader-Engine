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

void as::set_fps(const u16& fps)
{
	SetTargetFPS(fps);
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
			if (engine_memory_pool->shaders[i].data.id != -1)
			{
				UnloadShader(engine_memory_pool->shaders[i].data);
				engine_memory_pool->shaders[i].data.id = -1;
			}
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

Vector3 get_vec3(const json& json_data)
{
	Vector3 out_vector = {};
	if (json_data.size() >= 3)
	{
		out_vector.x = json_data[0];
		out_vector.y = json_data[1];
		out_vector.z = json_data[2];
	}
	return out_vector;
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

bool is_absolute_path(const json& json_data)
{
	if (json_data.contains("absolute_path"))
	{
		return json_data["absolute_path"].get<bool>();
	}
	return false;
}

bool is_active(const json& json_data)
{
	if (json_data.contains("is_active"))
	{
		return json_data["is_active"].get<bool>();
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


std::string as::var_type_enum_to_string(const var::variable_type& in_type)
{
	return as::var::variable_type_strings[in_type];
}

as::var::variable_type as::var_type_string_to_enum(const std::string& in_type_str)
{
	for (u8 i = 0; i < as::var::variable_type_strings.size(); i++)
	{
		if (as::var::variable_type_strings[i] == in_type_str)
		{
			return (as::var::variable_type)i;
		}
	}
	return as::var::variable_type::NONE;
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

json as::get_parsed_data(const char* path, const bool& absolute_path)
{
	json out_json_data;
	if (path)
	{
		char out_updated_path[MAX_PATH_SIZE] = "";
		as::get_updated_path(path, absolute_path, out_updated_path);
		AS_LOG(LV_LOG, "Parse file [" + std::string(path) + "]");
		out_json_data = as::util::read_json_file(out_updated_path);
	}
	return out_json_data; // performs a copy which sucks but I am using it as it is easier to use
}

as::world* as::get_world(const char* path, const bool& absolute_path)
{
	json json_data = get_parsed_data(path, absolute_path);
	if (!json_data.is_null() && get_type(json_data) == as::entity_type::WORLD)
	{
		as::world* out_world = get_world(json_data);
		if (out_world)
		{
			AS_SET_PATH_PTR(out_world, path);
		}
		return out_world;
	}
	AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
	return nullptr;
}

as::world* as::get_world(const json& json_data)
{
	as::world* out_world = get_empty_world_from_pool();

	if (!out_world)
	{
		AS_LOG(LV_WARNING, "Reached max pool size for worlds, cannot create world");
		return nullptr;
	}

	AS_INIT_PTR(out_world, as::world);
	AS_SET_VALID_PTR(out_world);

	const bool absolute_path = is_absolute_path(json_data);
	out_world->is_active = is_active(json_data);

	if (json_data.contains("models"))
	{
		json models = json_data["models"];
		out_world->models_count = models.size();
		for (u16 i = 0; i < out_world->models_count; i++)
		{
			if (as::model* found_model = get_model(models[i]))
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
			as::light* found_light = get_light(lights_file_paths[i].c_str(), absolute_path);
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
			as::camera* found_camera = get_camera(cameras_file_paths[i].c_str(), absolute_path);
			if (found_camera)
			{
				add_camera_to_world(out_world, found_camera);
			}
		}
	}
	return out_world;


	if (get_type(json_data) == as::entity_type::WORLD)
	{

	}
}

as::model* as::get_model(const char* path, const bool& absolute_path)
{
	json json_data = get_parsed_data(path, absolute_path);
	if (!json_data.is_null() && get_type(json_data) == as::entity_type::MODEL)
	{
		as::model* out_model = get_model(json_data);
		if (out_model)
		{
			AS_SET_PATH_PTR(out_model, path);
		}
		return out_model;
	}
	AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
	return nullptr;
}

as::model* as::get_model_instance(const json& json_data)
{
	// path
	as::model* out_model = nullptr;
	if (json_data.contains("path"))
	{
		std::string model_path = json_data["path"].get<std::string>();
		out_model = as::get_model(model_path.c_str(), is_absolute_path(json_data));
		AS_SET_VALID_PTR(out_model);
	}
	else
	{
		AS_LOG(LV_WARNING, "No given path in the model, failed to create a model instance");
		return nullptr;
	}
	update_model_transform(out_model, json_data, false);
	return out_model;
}

as::model* as::get_model(const json& json_data)
{
	// path
	char model_path[MAX_PATH_SIZE] = "";
	if (json_data.contains("path"))
	{
		std::string model_path_string = json_data["path"].get<std::string>();
		memcpy(model_path, model_path_string.c_str(), model_path_string.size() * sizeof(char));
		if (!is_absolute_path(json_data))
		{
			util::update_path_to_absolute(model_path);
		}
	}
	else
	{
		AS_LOG(LV_WARNING, "No given path in the model, failed to create a model");
		return nullptr;
	}

	// check if it's an instance
	if (util::ends_with(model_path, ".json"))
	{
		return get_model_instance(json_data);
	}
	
	// find in pool
	as::model* out_model = get_empty_model_from_pool();
	if (!out_model)
	{
		AS_LOG(LV_WARNING, "Reached max pool size for models, cannot create model");
		return nullptr;
	}
	AS_INIT_PTR(out_model, as::model);
	AS_SET_VALID_PTR(out_model);

	const bool absolute_path = is_absolute_path(json_data);
	out_model->data = LoadModel(model_path);

	if (json_data.contains("shaders"))
	{
		std::vector<std::string> shaders = json_data["shaders"].get<std::vector<std::string>>();
		for (u16 i = 0; i < shaders.size(); i++)
		{
			// TODO: this may change to have custom uniform during the instancing later on without recreating a json file for the shader
			as::shader* found_shader = get_shader(shaders[i].c_str(), absolute_path);
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
			as::shader* found_shader = get_shader(DEFAULT_SHADER_PATH, absolute_path);
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

	update_model_transform(out_model, json_data, true);
	return out_model;
}

as::shader* as::get_shader(const char* path, const bool& absolute_path)
{
	json json_data = get_parsed_data(path, absolute_path);
	if (!json_data.is_null() && get_type(json_data) == as::entity_type::SHADER)
	{
		as::shader* out_shader = get_shader(json_data);
		if (out_shader)
		{
			AS_SET_PATH_PTR(out_shader, path);
		}
		return out_shader;
	}
	AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
	return nullptr;
}

as::shader* as::get_shader(const json& json_data)
{
	as::shader* out_shader = get_empty_shader_from_pool();
	if (!out_shader)
	{
		AS_LOG(LV_WARNING, "Reached max pool size for shaders, cannot create shader");
		return nullptr;
	}

	AS_INIT_PTR(out_shader, as::shader);
	AS_SET_VALID_PTR(out_shader);
	std::string vertex_path;
	std::string fragment_path;
	if (json_data.contains("vertex_path"))
	{
		//vertex_path = as::util::get_current_path() + "/../" + json_data["vertex_path"].get<std::string>();
		vertex_path = json_data["vertex_path"].get<std::string>();
	}
	else
	{
		AS_LOG(LV_WARNING, "Shader json file does not have vertex shader");
		return nullptr;
	}
	if (json_data.contains("fragment_path"))
	{
		//fragment_path = as::util::get_current_path() + "/../" + json_data["fragment_path"].get<std::string>();
		fragment_path = json_data["fragment_path"].get<std::string>();
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

	// load shader into data
	std::string current_shader_path = as::util::get_current_path() + "/../";
	char* vertex_shader_code = (char*)AS_MALLOC(sizeof(char) * MAX_FILE_SIZE);
	char* fragment_shader_code = (char*)AS_MALLOC(sizeof(char) * MAX_FILE_SIZE);
	as::util::expand_file_includes(vertex_path.c_str(), current_shader_path.c_str(), vertex_shader_code);
	as::util::expand_file_includes(fragment_path.c_str(), current_shader_path.c_str(), fragment_shader_code);
	//out_shader->data = LoadShader(vertex_path.c_str(), fragment_path.c_str());
	out_shader->data = LoadShaderFromMemory(vertex_shader_code, fragment_shader_code);
	AS_FREE(vertex_shader_code);
	AS_FREE(fragment_shader_code);

	if (json_data.contains("uniforms"))
	{
		json uniforms = json_data["uniforms"];
		for (const json& current_uniform : uniforms)
		{
			out_shader->uniforms[out_shader->uniforms_count] = get_uniform(current_uniform);
			out_shader->uniforms_count++;
		}
	}

	if (out_shader->data.id <= 0)
	{
		AS_LOG(LV_WARNING, "Could not create the shader");
		UnloadShader(out_shader->data);
		out_shader = nullptr;
	}
	return out_shader;
}

as::texture* as::get_texture(const char* path, const bool& absolute_path)
{
	json json_data = get_parsed_data(path, absolute_path);
	if (!json_data.is_null() && get_type(json_data) == as::entity_type::TEXTURE)
	{
		as::texture* out_texture = get_texture(json_data);
		if (out_texture)
		{
			AS_SET_PATH_PTR(out_texture, path);
		}
		return out_texture;
	}
	AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
	return nullptr;
}

as::uniform as::get_uniform(const json& json_data)
{
	as::uniform out_uniform = {}; //TODO: maybe it shouldn't copy, so pass pointer in future?
	if (json_data.contains("type"))
	{
		std::string uniform_type = json_data["type"].get<std::string>();
		out_uniform.type = as::var_type_string_to_enum(uniform_type);
		switch (out_uniform.type)
		{

		case var::variable_type::FLOAT:
			out_uniform.value[0] = json_data["value"].get<float>();
			break;
		case var::variable_type::INT:
			out_uniform.value[0] = json_data["value"].get<i32>();
			break;
		case var::variable_type::BOOL:
			out_uniform.value[0] = json_data["value"].get<bool>();
			break;
		case var::variable_type::TEXTURE:
			out_uniform.value[0] = (u64)get_texture(json_data["value"]);
			break;
		case var::variable_type::VECTOR:
			Vector3 out_vec = get_vec3(json_data["value"]);
			out_uniform.value[0] = out_vec.x;
			out_uniform.value[1] = out_vec.y;
			out_uniform.value[2] = out_vec.z;
			break;
		default:
			break;
		}
	}
	return out_uniform;
}

as::texture* as::get_texture(const json& json_data)
{
	as::texture* out_texture = nullptr;
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
	return out_texture;
}

as::camera* as::get_camera(const char* path, const bool& absolute_path)
{
	json json_data = get_parsed_data(path, absolute_path);
	if (!json_data.is_null() && get_type(json_data) == as::entity_type::CAMERA)
	{
		as::camera* out_camera = get_camera(json_data);
		if (out_camera)
		{
			AS_SET_PATH_PTR(out_camera, path);
			AS_SET_VALID_PTR(out_camera);
		}
		return out_camera;
	}
	AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
	return nullptr;
}

as::camera* as::get_camera(const json& json_data)
{
	as::camera* out_camera = get_empty_camera_from_pool();
	if (!out_camera)
	{
		AS_LOG(LV_WARNING, "Reached max pool size for cameras, cannot create camera");
		return nullptr;
	}
	AS_INIT_PTR(out_camera, as::camera);
	AS_SET_VALID_PTR(out_camera);
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
	return out_camera;
}

as::light* as::get_light(const char* path, const bool& absolute_path)
{
	json json_data = get_parsed_data(path, absolute_path);
	if (!json_data.is_null() && get_type(json_data) == as::entity_type::LIGHT)
	{
		as::light* out_light = get_light(json_data);
		if (out_light)
		{
			AS_SET_PATH_PTR(out_light, path);
			AS_SET_VALID_PTR(out_light);
		}
		return out_light;
	}
	AS_LOG(LV_WARNING, "Invalid path, cannot parse json file");
	return nullptr;
}

as::light* as::get_light(const json& json_data)
{
	as::light* out_light = get_empty_light_from_pool();
	if (out_light)
	{
		AS_INIT_PTR(out_light, as::light);
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
	return out_light;
}

bool as::add_model_to_world(as::world* world, as::model* model)
{
	if (world && model)
	{
		for (u16 i = 0 ; i < MAX_MODELS_PER_WORLD ; i++)
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

void as::update_model_transform(as::model* model, const json& json_data, const bool& init)
{
	if (model)
	{
		if (init)
		{
			model->data.transform = MatrixIdentity();
		}
		Vector3 out_vector = Vector3();
		if (get_vec3(json_data, "scale", out_vector))
		{
			as::apply_scale(out_vector, model->data.transform);
		};
		if (get_vec3(json_data, "rotation", out_vector))
		{
			as::apply_rotation(out_vector, model->data.transform);
		};
		if (get_vec3(json_data, "location", out_vector))
		{
			as::apply_location(out_vector, model->data.transform);
		}
	}
	else
	{
		AS_LOG(LV_WARNING, "Cannot update model transform, model is nullptr");
	}
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

Vector3 as::get_location(const Matrix& transform_matrix)
{
	return Vector3(transform_matrix.m12, transform_matrix.m13, transform_matrix.m14);
}

Vector3 as::get_position(const Matrix& transform_matrix)
{
	return Vector3(transform_matrix.m12, transform_matrix.m13, transform_matrix.m14);
}

Quaternion as::get_rotation_quat(const Matrix& transform_matrix)
{
	Quaternion rotation;
	float trace = transform_matrix.m0 + transform_matrix.m5 + transform_matrix.m10;
	if (trace > 0.0f) {
		float s = 0.5f / sqrtf(trace + 1.0f);
		rotation.w = 0.25f / s;
		rotation.x = (transform_matrix.m6 - transform_matrix.m9) * s;
		rotation.y = (transform_matrix.m8 - transform_matrix.m2) * s;
		rotation.z = (transform_matrix.m1 - transform_matrix.m4) * s;
	}
	else if (transform_matrix.m0 > transform_matrix.m5 && transform_matrix.m0 > transform_matrix.m10) {
		float s = 2.0f * sqrtf(1.0f + transform_matrix.m0 - transform_matrix.m5 - transform_matrix.m10);
		rotation.w = (transform_matrix.m6 - transform_matrix.m9) / s;
		rotation.x = 0.25f * s;
		rotation.y = (transform_matrix.m4 + transform_matrix.m1) / s;
		rotation.z = (transform_matrix.m8 + transform_matrix.m2) / s;
	}
	else if (transform_matrix.m5 > transform_matrix.m10) {
		float s = 2.0f * sqrtf(1.0f + transform_matrix.m5 - transform_matrix.m0 - transform_matrix.m10);
		rotation.w = (transform_matrix.m8 - transform_matrix.m2) / s;
		rotation.x = (transform_matrix.m4 + transform_matrix.m1) / s;
		rotation.y = 0.25f * s;
		rotation.z = (transform_matrix.m9 + transform_matrix.m6) / s;
	}
	else {
		float s = 2.0f * sqrtf(1.0f + transform_matrix.m10 - transform_matrix.m0 - transform_matrix.m5);
		rotation.w = (transform_matrix.m1 - transform_matrix.m4) / s;
		rotation.x = (transform_matrix.m8 + transform_matrix.m2) / s;
		rotation.y = (transform_matrix.m9 + transform_matrix.m6) / s;
		rotation.z = 0.25f * s;
	}
	return rotation;
}

Vector3 as::get_rotation_vec(const Matrix& transform_matrix)
{
	Vector3 rotation;
	Quaternion quaternion = get_rotation_quat(transform_matrix);
	float angle = 2.0f * acosf(quaternion.w);
	float s = sqrtf(1.0f - quaternion.w * quaternion.w);
	if (s < 0.001f) {
		rotation.x = quaternion.x;
		rotation.y = quaternion.y;
		rotation.z = quaternion.z;
	}
	else {
		rotation.x = angle * quaternion.x / s;
		rotation.y = angle * quaternion.y / s;
		rotation.z = angle * quaternion.z / s;
	}
	return rotation;
}

Vector3 as::get_scale(const Matrix& transform_matrix)
{
	return Vector3(transform_matrix.m0, transform_matrix.m5, transform_matrix.m10);
}

i8 as::compare_distances(const Matrix& matrix_main, const Matrix& matrix_a, const Matrix& matrix_b)
{
	f32 distance_a = Vector3Distance(get_location(matrix_a), get_location(matrix_main));
	f32 distance_b = Vector3Distance(get_location(matrix_b), get_location(matrix_main));
	if (distance_a > distance_b) 
	{
		return -1;
	}
	else if (distance_a < distance_b) 
	{
		return 1;
	}
	return 0;
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

void as::set_path(as::entity_data& entity_data, const char* path)
{
	memcpy(entity_data.path, path, MAX_PATH_SIZE);
}

void as::swap_models(as::model* model1, as::model* model2)
{
	if (model1 && model2)
	{
		as::model temp = *model1;
		*model1 = *model2;
		*model2 = temp;
	}
	else
	{
		AS_LOG(LV_WARNING, "Invalid models, cannot swap models (swap_models)");
	}
}

i32 partition_models(as::model** models, const as::camera* camera, const i32& low, const i32& high)
{
	if (!models || !models[high] || !camera)
	{
		AS_LOG(LV_WARNING, "Model or Camera is invalid, cannot sort (partition_models)");
		return 0;
	}

	f32 pivot = Vector3Distance(as::get_position(models[high]->data.transform), camera->data.position);
	i32 i = low - 1;

	for (i32 j = low; j < high; j++)
	{
		if (models[j] && Vector3Distance(as::get_position(models[j]->data.transform), camera->data.position) > pivot)
		{
			i++;
			as::swap_models(models[i], models[j]);
		}
	}

	as::swap_models(models[i + 1], models[high]);
	return i + 1;
}

void as::quick_sort_models(as::model** models, as::camera* camera, const i32& low, const i32& high)
{
	if (!models || !camera)
	{
		AS_LOG(LV_WARNING, "Model or Camera is invalid, cannot sort (quick_sort_models)");
		return;
	}
	if (low < high)
	{
		i32 pivot_index = partition_models(models, camera, low, high);
		quick_sort_models(models, camera, low, pivot_index - 1);
		quick_sort_models(models, camera, pivot_index + 1, high);
	}
}

bool as::is_translucent(const as::model* model)
{
	if (model)
	{
		return model->flags & model::flag::TRANSLUCENT;
	}
	return false;
}

bool as::is_not_translucent(const as::model* model)
{
	return !is_translucent(model);
}

as::camera* as::find_active_camera(const as::world* world)
{
	for (u32 i = 0 ; i < world->cameras_count ; i++)
	{
		if (world->cameras[i] && world->cameras[i]->is_active && AS_IS_VALID_PTR(world->cameras[i]))
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
				//TODO (implement cashing for lights shader uniform location)

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

				sprintf(buffer, "lights[%i].shadow_map", i);
				light_data_loc = GetShaderLocation(shader, buffer);
				SetShaderValueTexture(shader, light_data_loc, lights[i]->shadow_map.texture.texture);

				sprintf(buffer, "lights_projection_matrix[%i]", i);
				light_data_loc = GetShaderLocation(shader, buffer);
				SetShaderValueMatrix(shader, light_data_loc, GetCameraMatrix(lights[i]->shadow_map.camera));
			}
		}
	}
}

void as::update_time_uniforms(const Shader& shader)
{
	const i32 time_location = GetShaderLocation(shader, "time");
	SetShaderValue(shader, time_location, &current_time, SHADER_UNIFORM_FLOAT);
}

void as::update_shadow_map(as::light* light)
{
	if (light)
	{
		if (!IsRenderTextureReady(light->shadow_map.texture))
		{
			// Create the shadow map texture
			light->shadow_map.texture = LoadRenderTexture(light->shadow_map.size, light->shadow_map.size);
			SetTextureFilter(light->shadow_map.texture.texture, TEXTURE_FILTER_POINT); // Set texture filter to avoid blurring
		}
		
		// Create the camera for the light
		Camera shadowCamera;
		light->shadow_map.camera.position = light->location;
		light->shadow_map.camera.target = Vector3Zero();
		light->shadow_map.camera.up = Vector3(0.0f, 1.0f, 0.0f);
		light->shadow_map.camera.fovy = 45.0f;
		light->shadow_map.camera.projection = CAMERA_PERSPECTIVE;
	}
	else
	{
		AS_LOG(LV_WARNING, "Cannot update shadow map, light is nullptr");
	}
}

bool as::draw(as::world* world)
{
	if (world)
	{
		as::camera* camera_to_use = find_active_camera(world);

		AS_ASSERT(world->models_count > 0, "Cannot draw 0 models, check the world content");
		AS_ASSERT(camera_to_use, "Cannot draw with no active camera, check the world content");

		// sort all models by distance to the camera
		quick_sort_models(world->models, camera_to_use, 0, world->models_count - 1);

		//lights
		//for (u32 i = 0; i < world->lights_count; i++)
		//{
		//	if (i == 0)
		//	{
		//		update_shadow_map(world->lights[i]);
		//	}
		//}
		//as::draw_light_maps(world);
		
		BeginDrawing();
		clear_background();
		BeginMode3D(camera_to_use->data);

		// Render opaque meshes
		//rlEnableDepthTest(); (should change depth here)
		for (u16 i = 0; i < world->models_count; i++)
		{
			if (world->models[i] && AS_IS_VALID_PTR(world->models[i]) && is_not_translucent(world->models[i]))
			{
				for (u32 j = 0; j < world->models[i]->data.materialCount; j++)
				{
					update_lights_uniforms(world->models[i]->data.materials[j].shader, world->lights, world->lights_count);
					update_time_uniforms(world->models[i]->data.materials[j].shader);
				}
				DrawModel(world->models[i]->data, get_location(world->models[i]->data.transform), 1.0, WHITE);
			}
		}
		DrawGrid(30, 10.f);

		// Render translucent meshes 
		//rlDisableDepthTest(); (should change depth here)
		for (u16 i = 0; i < world->models_count; i++)
		{
			if (world->models[i] && AS_IS_VALID_PTR(world->models[i]) && is_translucent(world->models[i]))
			{
				for (u32 j = 0; j < world->models[i]->data.materialCount; j++)
				{
					update_lights_uniforms(world->models[i]->data.materials[j].shader, world->lights, world->lights_count);
					
				}
				Vector3 position = Vector3(world->models[i]->data.transform.m12, world->models[i]->data.transform.m13, world->models[i]->data.transform.m14);
				DrawModel(world->models[i]->data, position, 1.0, WHITE);
			}
		}
		
		EndMode3D();
		EndDrawing();

		// Draw the shadow maps
		//BeginDrawing();
		//for (u16 i = 0; i < world->lights_count; i++)
		//{
		//	DrawTextureEx(world->lights[i]->shadow_map.texture.texture, { 0, 0 }, 0.0f, 1.0f, WHITE);
		//}
		//EndDrawing();

		//BeginDrawing();
		//ClearBackground(RAYWHITE);

		////// Draw the shadow map texture on the screen
		//DrawTexturePro(world->lights[1]->shadow_map.texture.texture, Rectangle( 0, 0, world->lights[0]->shadow_map.texture.texture.width, -world->lights[0]->shadow_map.texture.texture.height ), Rectangle( 0, 0, 500, 500), Vector2( 0, 0 ), 0.0f, WHITE);

		//// End drawing
		//EndDrawing();

		current_time += GetFrameTime();
	}
	return false;
}

bool as::draw_light_maps(as::world* world)
{
	if (!world)
	{
		AS_LOG(LV_WARNING, "Cannot draw light maps, invalid world");
		return false;
	}
	for (u16 i = 0; i < world->lights_count; i++)
	{
		// Set the shadow map render texture as the target
		BeginTextureMode(world->lights[i]->shadow_map.texture);

		// Clear the render texture to white
		ClearBackground(RAYWHITE);

		// Render the scene from the light's perspective
		BeginMode3D(world->lights[i]->shadow_map.camera);
		for (u16 j = 0; j < world->models_count; j++)
		{
			if (world->models[j] && AS_IS_VALID_PTR(world->models[j]) && is_not_translucent(world->models[j]))
			{
				DrawModel(world->models[j]->data, get_location(world->models[j]->data.transform), 1.0, WHITE);
			}
		}
		EndMode3D();

		// Reset the render target
		EndTextureMode();
	}
	return true;
}

void as::clear_background()
{
	rlClearScreenBuffers();
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
