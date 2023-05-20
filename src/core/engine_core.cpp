// internal dependencies (engine)
#include "engine_utility.h"
#include "engine_core.h"
#include "rlgl.h"

void as::init_gl()
{
	rlEnableDepthTest();
	rlEnableColorBlend();
	rlEnableBackfaceCulling();
}

void as::init_window(const u16& width, const u16& height, const char* title)
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(width, height, title);
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

std::string as::variable_enum_to_string(const as::ent::entity_type& in_type)
{
	return entity_type_strings[in_type];
}

as::ent::entity_type as::variable_string_to_enum(const std::string& in_type_str)
{
	for (u8 i = 0; i < entity_type_strings.size(); i++)
	{
		if (entity_type_strings[i] == in_type_str)
		{
			return (ent::entity_type)i;
		}
	}
	return as::ent::entity_type::NONE;
}

bool as::get_model_from_entity(const as::entity& entity, as::model& out_model)
{
	if (entity.type == as::ent::entity_type::MODEL && entity.data_ptr)
	{
		as::model* found_model = static_cast<as::model*>(entity.data_ptr);
		if (found_model)
		{
			out_model = *found_model;
			return true;
		}
	}
	return false;
}

bool as::get_model_from_entity(const as::entity* entity, as::model*& out_model)
{
	if (entity)
	{
		if (entity->type == as::ent::entity_type::MODEL && entity->data_ptr)
		{
			out_model = static_cast<as::model*>(entity->data_ptr);
			return true;
		}
	}
	return false;
}

bool as::get_camera_from_entity(const as::entity& entity, as::camera& out_camera)
{
	if (entity.type == as::ent::entity_type::CAMERA)
	{
		if (entity.data_ptr)
		{
			out_camera = *static_cast<as::camera*>(entity.data_ptr);
			return true;
		}
		else
		{
			AS_LOG(LV_ERROR, "Cannot get camera from entity, data is nullptr")
		}
	}
	return false;
}

bool as::get_camera_from_entity(const as::entity& entity, as::camera*& out_camera)
{
	if (entity.type == as::ent::entity_type::CAMERA)
	{
		if (entity.data_ptr)
		{
			out_camera = (as::camera*)entity.data_ptr;
			return true;
		}
		else
		{
			AS_LOG(LV_ERROR, "Cannot get camera from entity, data is nullptr")
		}
	}
	return false;
}

bool as::get_camera_from_entity(const as::entity* entity, as::camera*& out_camera)
{
	if (entity)
	{
		if (entity->type == as::ent::entity_type::CAMERA)
		{
			if (entity->data_ptr)
			{
				out_camera = (as::camera*)entity->data_ptr;
				return true;
			}
			else
			{
				AS_LOG(LV_ERROR, "Cannot get camera from entity, data is nullptr")
			}
		}
	}
	else
	{
		AS_LOG(LV_ERROR, "Cannot get camera from entity, entity is nullptr")
	}
	return false;
}

bool as::get_world_from_entity(const as::entity& entity, as::world& out_world)
{
	if (entity.type == as::ent::entity_type::WORLD)
	{
		if (entity.data_ptr)
		{
			out_world = *static_cast<as::world*>(entity.data_ptr);
			return true;
		}
		else
		{
			AS_LOG(LV_ERROR, "Cannot get world from entity, data is nullptr")
		}
	}
	return false;
}

bool as::get_world_from_entity(const as::entity* entity, as::world*& out_world)
{
	if (entity)
	{
		if (entity->type == as::ent::entity_type::WORLD)
		{
			if (entity->data_ptr)
			{
				out_world = static_cast<as::world*>(entity->data_ptr);
				return true;
			}
			else
			{
				AS_LOG(LV_ERROR, "Cannot get world from entity, data is nullptr")
			}
		}
	}
	return false;
}

bool as::get_shader_from_entity(const as::entity& entity, as::shader& out_shader)
{
	if (entity.type == as::ent::entity_type::SHADER)
	{
		if (entity.data_ptr)
		{
			out_shader = *static_cast<as::shader*>(entity.data_ptr);
			return true;
		}
		else
		{
			AS_LOG(LV_ERROR, "Cannot get shader from entity, data is nullptr")
		}
	}
	return false;
}


bool as::get_material_from_entity(const as::entity* entity, as::material*& out_material)
{
	if (entity->type == as::ent::entity_type::MATERIAL)
	{
		if (entity->data_ptr)
		{
			out_material = static_cast<as::material*>(entity->data_ptr);
			return true;
		}
		else
		{
			AS_LOG(LV_ERROR, "Cannot get material from entity, data is nullptr")
		}
	}
	return false;
}

bool as::get_shader_from_entity(const as::entity* entity, as::shader*& out_shader)
{
	if (entity)
	{
		if (entity->type == as::ent::entity_type::SHADER)
		{
			if (entity->data_ptr)
			{
				out_shader = static_cast<as::shader*>(entity->data_ptr);
				return true;
			}
			else
			{
				AS_LOG(LV_ERROR, "Cannot get shader from entity, data is nullptr")
			}
		}
	}
	return false;
}

bool as::get_texture_from_entity(const as::entity& entity, as::texture& out_texture)
{
	if (entity.type == as::ent::entity_type::TEXTURE)
	{
		if (entity.data_ptr)
		{
			out_texture = *static_cast<as::texture*>(entity.data_ptr);
			return true;
		}
		else
		{
			AS_LOG(LV_ERROR, "Cannot get shader from entity, data is nullptr")
		}
	}
	return false;
}

bool as::get_texture_from_entity(const as::entity* entity, as::texture*& out_texture)
{
	if (entity)
	{
		if (entity->type == as::ent::entity_type::TEXTURE)
		{
			if (entity->data_ptr)
			{
				out_texture = static_cast<as::texture*>(entity->data_ptr);
				return true;
			}
			else
			{
				AS_LOG(LV_ERROR, "Cannot get shader from entity, data is nullptr")
			}
		}
	}
	return false;
}

bool as::parse_file(const std::string& path, const bool& absolute_path, as::entity*& out_entity)
{
	AS_LOG(LV_LOG, "Parse file [" + path + "]");

	std::string new_path;
	if (absolute_path)
	{
		new_path = path;
	}
	else
	{
		new_path = as::util::get_current_path() + "/../" + path;
	}

	if (out_entity)
	{
		AS_LOG(LV_LOG, "_______________PRE-FREE-START_______________");
		AS_LOG_MEMORY();
		AS_LOG(LV_LOG, "_______________PRE-FREE-END_______________");
		as::delete_entity_data(out_entity);
		AS_LOG(LV_LOG, "_______________POST-FREE-START_______________");
		AS_LOG_MEMORY();
		AS_LOG(LV_LOG, "_______________POST-FREE-END_______________");
	}

	out_entity = (as::entity*)AS_MALLOC(sizeof(as::entity));
	*out_entity = as::entity();
	memcpy(out_entity->path, new_path.c_str(), 256);

	json json_data = as::util::read_json_file(new_path);
	if (json_data.contains("type"))
	{
		std::string entity_type = json_data["type"].get<std::string>();
		out_entity->type = as::variable_string_to_enum(entity_type);

		if (out_entity->type == as::ent::entity_type::WORLD)
		{
			as::world* out_world = (as::world*)AS_MALLOC(sizeof(as::world));
			*out_world = as::world();
			if (json_data.contains("entities"))
			{
				std::vector<std::string> entities_file_paths = json_data["entities"].get<std::vector<std::string>>();
				out_world->entities_count = entities_file_paths.size();
				for (u16 i = 0 ; i < out_world->entities_count ; i++)
				{
					as::entity* current_entity = nullptr;
					if (parse_file(entities_file_paths[i], absolute_path, current_entity) && current_entity)
					{
						add_sub_entity(out_entity, current_entity);
					}
				}
				out_world->entities = out_entity->sub_entities;
			}
			if (json_data.contains("is_active"))
			{
				bool is_active = false;
				if (get_bool(json_data, "is_active", is_active))
				{
					out_world->is_active = is_active;
				}
			}
			delete_entity_data(out_entity->data_ptr);
			out_entity->data_ptr = out_world;
		}
		else if (out_entity->type == as::ent::entity_type::MODEL)
		{
			as::model* out_model = nullptr;

			if (json_data.contains("path"))
			{
				std::string model_path = as::util::get_current_path() + "/../" + json_data["path"].get<std::string>();
				out_model = (as::model*)AS_MALLOC(sizeof(as::model));
				*out_model = as::model();
				out_model->data = LoadModel(model_path.c_str());
				out_model->owner_entity = out_entity;
			}
			else
			{
				AS_LOG(LV_WARNING, "Model json file does not have path or vertices array");
				as::delete_entity_data(out_entity);
				return false;
			}
			if (json_data.contains("shaders"))
			{
				std::vector<std::string> shaders = json_data["shaders"].get<std::vector<std::string>>();
				for (u16 i = 0; i < shaders.size(); i++)
				{
					as::entity* shader_entity = nullptr;
					if (parse_file(shaders[i], absolute_path, shader_entity) && out_model->data.materialCount > i)
					{
						as::shader* shader_found = nullptr;
						if (as::get_shader_from_entity(shader_entity, shader_found))
						{
							out_model->data.materials[i].shader = shader_found->data;
						}
						as::add_sub_entity(out_entity, shader_entity);
					}
				}
				if (out_model->data.materialCount > shaders.size())
				{
					as::entity* defaullt_shader_entity = nullptr;
					if (parse_file(DEFAULT_SHADER_PATH, absolute_path, defaullt_shader_entity))
					{
						as::shader* shader_found = nullptr;
						if (as::get_shader_from_entity(defaullt_shader_entity, shader_found))
						{
							for (u16 i = 0; i < out_model->data.materialCount; i++)
							{
								out_model->data.materials[i].shader = shader_found->data;
							}
							as::add_sub_entity(out_entity, defaullt_shader_entity);
						}
					}
				}
			}
			else
			{
				AS_LOG(LV_WARNING, "Model json file does not have shaders");
				as::delete_entity_data(out_entity);
				return false;
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
			as::delete_entity_data(out_entity->data_ptr);
			out_entity->data_ptr = out_model;
		}
		else if (out_entity->type == as::ent::entity_type::MATERIAL)
		{
			as::material* out_material = (as::material*)AS_MALLOC(sizeof(as::material));
			*out_material = as::material();
			out_material->owner_entity = out_entity;
			if (json_data.contains("shader"))
			{
				as::entity* out_shader_entity = nullptr;
				if (parse_file(json_data["shader"], absolute_path, out_shader_entity))
				{
					as::shader* found_shader = nullptr;
					if (get_shader_from_entity(out_shader_entity, found_shader))
					{
						out_material->data.shader = found_shader->data;
					}
				}
				if (json_data.contains("textures"))
				{
					// TODO: Add uniform name for each texture
					std::vector<std::string> textures = json_data["textures"].get<std::vector<std::string>>();
					for (u16 i = 0; i < textures.size(); i++)
					{
						as::entity* texture_entity = nullptr;
						if (parse_file(textures[i], absolute_path, texture_entity))
						{
							as::texture* found_texture = nullptr;
							if (get_texture_from_entity(texture_entity, found_texture))
							{
								out_material->data.maps[i].texture = found_texture->data;
							}
							as::add_sub_entity(out_entity, texture_entity);
						}
					}
				}
			}
		}
		else if (out_entity->type == as::ent::entity_type::SHADER)
		{
			as::shader* out_shader = (as::shader*)AS_MALLOC(sizeof(as::shader));
			*out_shader = as::shader();
			std::string vertex_path;
			std::string fragment_path;
			if (json_data.contains("vertex_path"))
			{
				vertex_path = as::util::get_current_path() + "/../" + json_data["vertex_path"].get<std::string>();
			}
			else
			{
				AS_LOG(LV_WARNING, "Shader json file does not have vertex shader");
				as::delete_entity_data(out_entity);
				return false;
			}
			if (json_data.contains("fragment_path"))
			{
				fragment_path = as::util::get_current_path() + "/../" + json_data["fragment_path"].get<std::string>();
			}
			else
			{
				AS_LOG(LV_WARNING, "Shader json file does not have fragment shader");
				as::delete_entity_data(out_entity);
				return false;
			}
			out_shader->data = LoadShader(vertex_path.c_str(), fragment_path.c_str());
			if (out_shader->data.id > 0)
			{
				out_shader->owner_entity = out_entity;
				out_entity->data_ptr = out_shader;
			}
			else
			{
				AS_LOG(LV_WARNING, "Could not create the shader");
				AS_FREE(out_shader);
				out_shader = nullptr;
				as::delete_entity_data(out_entity);
				return false;
			}
		}
		else if (out_entity->type == as::ent::entity_type::TEXTURE)
		{
			if (json_data.contains("path"))
			{
				std::string texture_path = as::util::get_current_path() + "/../" + json_data["path"].get<std::string>();

				bool found_texture = false;
				// TODO: caching
				//for (size_t i = 0; i < allocations_count; i++)
				//{
				//	as::entity* found_entity_ptr = static_cast<as::entity*>(allocations[i]->ptr);
				//	if (found_entity_ptr && found_entity_ptr->type == as::ent::TEXTURE)
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
					as::texture* out_texture = (as::texture*)AS_MALLOC(sizeof(as::texture));
					*out_texture = as::texture();
					out_texture->data = LoadTexture(texture_path.c_str());
					out_texture->owner_entity = out_entity;
					if (out_texture->data.id > 0)
					{
						delete_entity_data(out_entity->data_ptr);
						out_entity->data_ptr = out_texture;
					}
					else
					{
						AS_LOG(LV_WARNING, "Could not load the texture");
						AS_FREE(out_texture);
						out_texture = nullptr;
						as::delete_entity_data(out_entity);
						return false;
					}
				}
			}
			else
			{
				AS_LOG(LV_WARNING, "Texture json file does not have path");
				as::delete_entity_data(out_entity);
				return false;
			}
		}
		else if (out_entity->type == as::ent::entity_type::CAMERA)
		{
			as::camera* out_camera = (as::camera*)AS_MALLOC(sizeof(as::camera));
			*out_camera = as::camera();
			out_camera->owner_entity = out_entity;
			get_vec3(json_data, "position", out_camera->data.position);
			get_vec3(json_data, "target", out_camera->data.target);
			if (json_data.contains("is_active"))
			{
				out_camera->is_active = json_data["is_active"].get<bool>();
			}
			delete_entity_data(out_entity->data_ptr);
			Camera camera = { 0 };
			out_camera->data.up = Vector3(0.0f, 1.0f, 0.0f );          // Camera up vector (rotation towards target)
			out_camera->data.fovy = 75.f;                                // Camera field-of-view Y
			out_camera->data.projection = CAMERA_PERSPECTIVE;                   // Camera mode type

			out_entity->data_ptr = out_camera;
		}
		else if (out_entity->type == as::ent::entity_type::LIGHT)
		{
			as::light* out_light = (as::light*)AS_MALLOC(sizeof(as::light));
			*out_light = as::light();

			get_vec3(json_data, "location", out_light->location);
			get_float(json_data, "intensity", out_light->intensity);
			get_float(json_data, "attenuation", out_light->intensity);
			get_vec3(json_data, "color", out_light->color);
			out_light->owner_entity = out_entity;
			delete_entity_data(out_entity->data_ptr);
			out_entity->data_ptr = out_light;
		}
	}
	return true;
}

void as::add_sub_entity(as::entity*& parent_entity, as::entity* sub_entity)
{

	parent_entity->sub_entities_count++;
	if (parent_entity->sub_entities)
	{
		parent_entity->sub_entities = (as::entity**)AS_REALLOC(parent_entity->sub_entities, sizeof(as::entity*) * parent_entity->sub_entities_count);
	}
	else
	{
		parent_entity->sub_entities = (as::entity**)AS_MALLOC(sizeof(as::entity*) * parent_entity->sub_entities_count);
	}
	parent_entity->sub_entities[parent_entity->sub_entities_count - 1] = sub_entity;
}

void as::delete_entity_data(as::entity*& entity)
{
	if (entity && entity->type < as::ent::MAX)
	{
		//as::world* world = nullptr;
		//if (as::get_world_from_entity(entity, world))
		//{
		//	as::delete_world_data(world);
		//}

		for (u32 i = 0; i < entity->sub_entities_count; i++)
		{
			as::delete_entity_data(entity->sub_entities[i]);
		}
		AS_FREE(entity->sub_entities);
		entity->sub_entities = nullptr;
		entity->sub_entities_count = 0;

		if (entity->data_ptr)
		{
			AS_FREE(entity->data_ptr);
			entity->data_ptr = nullptr;
		}
		if (entity->fn_ptr)
		{
			AS_FREE(entity->fn_ptr);
			entity->fn_ptr = nullptr;
		}
		if (entity)
		{
			AS_FREE(entity);
			entity = nullptr;
		}
	}
}

void as::delete_entity_data(void*& data_ptr)
{
	as::entity* entity_ptr = (as::entity*)data_ptr;
	if (entity_ptr)
	{
		delete_entity_data(entity_ptr);
	}
	delete(data_ptr);
	data_ptr = nullptr;
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


as::camera* as::find_active_camera(const as::world* world)
{
	for (u32 i = 0 ; i < world->entities_count ; i++)
	{
		if (world->entities[i] && world->entities[i]->type == as::ent::CAMERA)
		{
			as::camera* found_camera = static_cast<as::camera*>(world->entities[i]->data_ptr);
			if (found_camera && found_camera->is_active)
			{
				return found_camera;
			}
		}
	}
	return nullptr;
}

u32 as::get_all_lights(const as::world* world, as::light**& out_lights)
{
	u32 light_count = 0;
	out_lights = (as::light**)AS_MALLOC(sizeof(as::light*) * MAX_LIGHTS_PER_WORLD);
	for (u32 i = 0; i < world->entities_count; i++)
	{
		if (world->entities[i] && world->entities[i]->type == as::ent::LIGHT)
		{
			as::light* found_light = static_cast<as::light*>(world->entities[i]->data_ptr);
			if (found_light)
			{
				out_lights[light_count] = found_light;
				light_count++;
			}
		}
	}
	if (light_count == 0)
	{
		AS_FREE(out_lights);
		out_lights = nullptr;
	}
	return light_count;
}


void as::update_lights_uniforms(const Shader& shader, light** lights, const u32& lights_count)
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
		std::vector<const as::model*> models_to_draw;
		as::camera* camera_to_use = nullptr;
		for (u16 i = 0; i < world->entities_count; i++)
		{	
			if (world->entities[i])
			{
				as::model* out_model = nullptr;
				as::camera* out_camera = nullptr;
				if (get_model_from_entity(world->entities[i], out_model))
				{
					models_to_draw.push_back(out_model);
				}
				else if (get_camera_from_entity(*world->entities[i], out_camera))
				{
					if (out_camera && out_camera->is_active)
					{
						camera_to_use = out_camera;
					}
				}
			}
		}

		AS_ASSERT(!models_to_draw.empty(), "Cannot draw 0 models, check the world content");
		AS_ASSERT(camera_to_use, "Cannot draw with no active camera, check the world content");

		// get all lights
		as::light** lights = nullptr;
		const u32 lights_count = get_all_lights(world, lights);

		BeginDrawing();
		clear_background();
		BeginMode3D(camera_to_use->data);
		for (const as::model* model : models_to_draw)
		{
			if (model)
			{
				for (u32 j = 0; j < model->data.materialCount; j++)
				{
					update_lights_uniforms(model->data.materials[j].shader, lights, lights_count);
				}
				Vector3 position = Vector3(model->data.transform.m12, model->data.transform.m13, model->data.transform.m14);
				DrawModel(model->data, position, 1.0, WHITE);
			}
		}
		DrawGrid(30, 10.f);
		EndMode3D();
		EndDrawing();

		AS_FREE(lights);
	}
	return false;
}

void as::delete_world_data(as::world*& world)
{
	if (world)
	{
		for (u16 i = 0; i < world->entities_count; i++)
		{
			delete_entity_data(world->entities[i]);
		}
		AS_FREE(world->entities);
		world->entities = nullptr;
		world->entities_count = 0;
	}
}

void as::clear_background()
{
	ClearBackground(Color(0.f, 0.f, 0.f));
}