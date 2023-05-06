// external dependencies (libraries)
#include "glad/gl.h"
#include "glad/wgl.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#endif //STB_IMAGE_IMPLEMENTATION
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// internal dependencies (engine)
#include "engine_utility.h"
#include "engine_core.h"

bool check_compilation_status(const u32& shader_to_check)
{
	i32 success;
	char infoLog[512];
	glGetShaderiv(shader_to_check, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader_to_check, 512, NULL, infoLog);
		AS_LOG(LV_ERROR, std::string("Shader compilation failed: ") + infoLog);
		return false;
	}
	return true;
}

bool check_gl_error()
{
	u32 gl_error = glGetError();
	if (gl_error == GL_NO_ERROR)
	{
		return true;
	}
	AS_LOG(LV_ERROR, "GL error : " + std::to_string(gl_error));
	return false;
}

void as::configure()
{
	glEnable(GL_DEPTH_TEST);
	//wglSwapIntervalEXT(1); figure out a way to include this
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

bool get_vec3(const json& json_data, const char* json_field_name, glm::vec3& out_vector)
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

bool get_transform(const json& json_data, as::transform& out_transform)
{
	if (json_data.contains("transform"))
	{
		json transform_json_data = json_data["transform"];
		bool is_valid_location = get_vec3(transform_json_data, "location", out_transform.location);
		bool is_valid_rotation = get_vec3(transform_json_data, "rotation", out_transform.rotation);
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

std::string as::to_string(const as::entity* entity)
{
	if (entity)
	{
		{
			as::world* world = nullptr;
			if (as::get_world_from_entity(entity, world))
			{
				return as::to_string(world);
			}
		}

		{
			as::camera* camera = nullptr;
			if (as::get_camera_from_entity(entity, camera))
			{
				return as::to_string(camera);
			}
		}

		{
			as::model* model = nullptr;
			if (as::get_model_from_entity(entity, model))
			{
				return as::to_string(model);
			}
		}
	}
	return std::string();
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
		AS_LOG(LV_LOG, "_______________PRE-FREE_______________");
		AS_LOG_MEMORY();
		as::delete_entity_data(out_entity);
		AS_LOG(LV_LOG, "_______________POST-FREE_______________");
		AS_LOG_MEMORY();
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
				std::string model_path = json_data["path"].get<std::string>();
				std::vector<as::texture> out_textures;
				as::load_model(model_path.c_str(), out_model, out_textures);
			}
			else
			{
				AS_LOG(LV_WARNING, "Model json file does not have path");
				as::delete_entity_data(out_entity);
				return false;
			}
			if (json_data.contains("shaders"))
			{
				std::vector<std::string> shaders = json_data["shaders"].get<std::vector<std::string>>();
				for (u16 i = 0; i < shaders.size(); i++)
				{
					as::entity* shader_entity = nullptr;
					if (parse_file(shaders[i], absolute_path, shader_entity))
					{
						if (out_model->meshes && shader_entity->data_ptr)
						{
							if (out_model->mesh_count > i)
							{
								as::assign_shader(static_cast<as::shader*>(shader_entity->data_ptr), out_model->meshes[i]);
								as::add_sub_entity(out_entity, shader_entity);
							}
						}
					}
				}
				if (out_model->mesh_count > shaders.size())
				{
					as::entity* defaullt_shader_entity = nullptr;
					if (parse_file(DEFAULT_SHADER_PATH, absolute_path, defaullt_shader_entity))
					{
						for (u16 i = 0; i < out_model->mesh_count; i++)
						{
							if (out_model->meshes[i] && !out_model->meshes[i]->shader_ptr)
							{
								as::assign_shader(static_cast<as::shader*>(defaullt_shader_entity->data_ptr), out_model->meshes[i]);
							}
						}
						as::add_sub_entity(out_entity, defaullt_shader_entity);
					}
				}
			}
			else
			{
				AS_LOG(LV_WARNING, "Model json file does not have shaders");
				as::delete_entity_data(out_entity);
				return false;
			}
			as::transform out_transform;
			if (get_transform(json_data, out_transform))
			{
				as::apply_transform(out_transform, *out_model);
			};
			as::delete_entity_data(out_entity->data_ptr);
			out_entity->size = as::get_model_size(*out_model);
			out_entity->data_ptr = out_model;
		}
		else if (out_entity->type == as::ent::entity_type::SHADER)
		{
			as::shader* out_shader = (as::shader*)AS_MALLOC(sizeof(as::shader));
			// I believe normal initialization is not possible due to the uniforms being std::vector
			out_shader->vertex_shader = -1;
			out_shader->fragment_shader = -1;
			memset(out_shader->textures, 0, sizeof(as::texture*) * MAX_TEXTURE_COUNT_PER_SHADER);
			out_shader->texture_count = 0;
			out_shader->shader_program = 0;
			std::string vertex_path;
			std::string fragment_path;
			if (json_data.contains("vertex_path"))
			{
				vertex_path = json_data["vertex_path"].get<std::string>();
			}
			else
			{
				AS_LOG(LV_WARNING, "Shader json file does not have vertex shader");
				as::delete_entity_data(out_entity);
				return false;
			}
			if (json_data.contains("fragment_path"))
			{
				fragment_path = json_data["fragment_path"].get<std::string>();
			}
			else
			{
				AS_LOG(LV_WARNING, "Shader json file does not have fragment shader");
				as::delete_entity_data(out_entity);
				return false;
			}
			if (json_data.contains("textures"))
			{
				std::vector<std::string> textures = json_data["textures"].get<std::vector<std::string>>();
				for (u16 i = 0; i < textures.size(); i++)
				{
					as::entity* texture_entity = nullptr;
					if (parse_file(textures[i], absolute_path, texture_entity))
					{
						if (texture_entity->data_ptr)
						{
							as::add_texture_to_shader(static_cast<as::texture*>(texture_entity->data_ptr), out_shader);
						}
						as::add_sub_entity(out_entity, texture_entity);
					}
				}
			}
			if (as::create_shader_from_files(vertex_path.c_str(), fragment_path.c_str(), out_shader))
			{
				u32 shader_program;
				as::create_shader_program(shader_program);
				as::bind_shaders_to_program(shader_program, out_shader);
				delete_entity_data(out_entity->data_ptr);
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
				std::string texture_path = json_data["path"].get<std::string>();

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
					if (json_data.contains("uniform_name"))
					{
						std::string uniform_name = json_data["uniform_name"].get<std::string>();
						strcpy(out_texture->uniform_name, uniform_name.c_str());
					}
					if (as::load_texture(texture_path.c_str(), *out_texture))
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

			as::update_camera_vectors(out_camera);
			get_transform(json_data, out_camera->transform_mat);
			if (json_data.contains("is_active"))
			{
				out_camera->is_active = json_data["is_active"].get<bool>();
			}
			delete_entity_data(out_entity->data_ptr);
			out_entity->data_ptr = out_camera;
		}
		else if (out_entity->type == as::ent::entity_type::LIGHT)
		{
			as::light* out_light = (as::light*)AS_MALLOC(sizeof(as::light));
			*out_light = as::light();

			get_transform(json_data, out_light->transform_mat);
			if (json_data.contains("intensity"))
			{
				out_light->intensity = json_data["intensity"].get<f32>();
			}
			delete_entity_data(out_entity->data_ptr);
			out_entity->data_ptr = out_light;
		}
	}
	return true;
}

bool as::draw(const std::vector<as::entity*>& entities, const as::world* world)
{
	for (const as::entity* current_entity : entities)
	{
		as::model* current_model = nullptr;
		if (as::get_model_from_entity(current_entity, current_model))
		{
			as::draw(current_model, world);
		}
	}
	return true;
}

size_t as::get_entity_size(const as::entity* entity)
{
	size_t total_size = 0;

	for (u32 i = 0 ; i < entity->sub_entities_count ; i++)
	{
		total_size += get_entity_size(entity->sub_entities[i]);
	}

	switch (entity->type)
	{
	case ent::WORLD:
	{
		as::world* world = nullptr;
		if (as::get_world_from_entity(entity, world))
		{
			total_size += get_world_size(*world);
		}
	}
	case ent::MODEL:
	{
		as::model* model = nullptr;
		if (as::get_model_from_entity(entity, model))
		{
			total_size += as::get_model_size(*model);
		}
		break;
	}
	case ent::SHADER:
	{
		as::shader* shader = nullptr;
		if (as::get_shader_from_entity(entity, shader))
		{
			total_size += as::get_shader_size(*shader);
		}
		break;
	}
	case ent::TEXTURE:
	{
		as::texture* texture = nullptr;
		if (as::get_texture_from_entity(entity, texture))
		{
			total_size += get_texture_size(texture);
		}
		break;
	}
	case ent::CAMERA:
	{
		as::camera* camera = nullptr;
		if (as::get_camera_from_entity(entity, camera))
		{
			total_size += get_camera_size(*camera);
		}
		break;
	}
	default:
		break;
	}

	total_size += sizeof(entity);
	return total_size;
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
		as::shader* shader = nullptr;
		if (as::get_shader_from_entity(entity, shader))
		{
			as::delete_shader_data(shader);
		}

		as::model* model = nullptr;
		if (as::get_model_from_entity(entity, model))
		{
			as::delete_model_data(model);
		}

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

void as::apply_location(const glm::vec3& location, glm::mat4& transform_matrix)
{
	transform_matrix = glm::translate(transform_matrix, location);
}

void as::apply_rotation(const glm::vec3& rotation, glm::mat4& transform_matrix)
{
	transform_matrix = glm::rotate(transform_matrix, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));
	transform_matrix = glm::rotate(transform_matrix, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
	transform_matrix = glm::rotate(transform_matrix, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
}

void as::apply_scale(const glm::vec3& scale, glm::mat4& transform_matrix)
{
	transform_matrix = glm::scale(transform_matrix, scale);
}

void as::apply_transform(const transform& transform, glm::mat4& transform_matrix)
{
	apply_location(transform.location, transform_matrix);
	apply_rotation(transform.rotation, transform_matrix);
	apply_scale(transform.scale, transform_matrix);
}

bool as::create_shader(const char* vert_shader_src, const char* frag_shader_src, as::shader*& out_shader)
{
	bool vert_shader_result, frag_shader_result;
	// vertex shader
	out_shader->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(out_shader->vertex_shader, 1, &vert_shader_src, NULL);
	glCompileShader(out_shader->vertex_shader);
	vert_shader_result = check_compilation_status(out_shader->vertex_shader);

	// fragment shader
	out_shader->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(out_shader->fragment_shader, 1, &frag_shader_src, NULL);
	glCompileShader(out_shader->fragment_shader);
	frag_shader_result = check_compilation_status(out_shader->fragment_shader);

	return (vert_shader_result && frag_shader_result);
}

bool as::create_shader_from_files(const char* vert_shader_path, const char* frag_shader_path, as::shader*& out_shader)
{
	if (vert_shader_path && frag_shader_path)
	{
		std::string vert_shader_path_string = util::get_current_path() + "/../" + std::string(vert_shader_path);
		std::string frag_shader_path_string = util::get_current_path() + "/../" + std::string(frag_shader_path);

		std::string vert_shader = util::read_file(vert_shader_path_string.c_str());
		std::string frag_shader = util::read_file(frag_shader_path_string.c_str());
		if (!vert_shader.empty() && !frag_shader.empty())
		{
			return create_shader(vert_shader.c_str(), frag_shader.c_str(), out_shader);
		}
	}
	return false;
}

void as::create_shader_program(u32& out_shader_program)
{
	out_shader_program = glCreateProgram();
}

bool as::bind_shaders_to_program(const u32& shader_program, as::shader*& shader_to_attach)
{
	glAttachShader(shader_program, shader_to_attach->vertex_shader);
	glAttachShader(shader_program, shader_to_attach->fragment_shader);
	glLinkProgram(shader_program);

	i32 success;
	char infoLog[512];
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) 
	{
		glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
		AS_LOG(LV_ERROR, std::string("Binding shaders failed: ") + infoLog);
		return false;
	}
	shader_to_attach->shader_program = shader_program;
	return check_gl_error();
}

bool as::deep_copy_shader(const as::shader* source, as::shader*& destination)
{
	if (source && destination)
	{
		memcpy(destination, source, sizeof(as::shader));
		memcpy(destination->textures, source->textures, sizeof(as::texture*) * MAX_TEXTURE_COUNT_PER_SHADER);
		destination->uniforms = source->uniforms;
		return true;
	}
	AS_LOG(LV_ERROR, "Cannot deep copy shader, nullptr");
	return false;
}

bool as::deep_copy_shader(const as::shader* source, void*& destination)
{
	as::shader* destination_shader = static_cast<as::shader*>(destination);
	return as::deep_copy_shader(source, destination_shader);
}

size_t as::get_shader_size(const as::shader& shader)
{
	size_t total_size = 0;

	for (const as::uniform& uniform : shader.uniforms)
	{
		total_size += sizeof(as::uniform);
	}
	for (u16 i = 0 ; i < shader.texture_count ; i++)
	{
		if (shader.textures[i])
		{
			total_size += get_texture_size(shader.textures[i]);
		}
	}
	total_size += sizeof(shader);
	return total_size;
}

std::string as::to_string(const as::shader* shader)
{
	std::string final_string = "";
	if (shader)
	{
		final_string += "{\n	fragment_shader:" +
			std::to_string(shader->fragment_shader) +
			",\n	vertex_shader:" +
			std::to_string(shader->vertex_shader) +
			",\n	texture_count:" +
			std::to_string(shader->texture_count) +
			",\n	textures:[";
		for (u16 i = 0 ; i < shader->texture_count ; i++)
		{
			final_string += as::to_string(shader->textures[i]);
			if (shader->texture_count != i - 1)
			{
				final_string += ",\n	";
			}
		}
		final_string += "]}";
	}
	return final_string;
}

void as::delete_shader_data(as::shader*& shader)
{
	glDeleteShader(shader->vertex_shader);
	glDeleteShader(shader->fragment_shader);
	for (u16 i = 0 ; i < shader->texture_count ; i++ )
	{
		AS_FREE(shader->textures[i]);
		shader->textures[i] = nullptr;

	}
	shader->uniforms.clear();
}

void as::delete_shader_program(const u32& shader_program)
{
	AS_LOG(LV_LOG, "Deleting shader program [" + std::to_string(shader_program) + "]");
	glDeleteProgram(shader_program);
}

void as::add_uniform(const as::uniform& uniform_to_add, as::shader& shader)
{
	shader.uniforms.push_back(uniform_to_add);
}

void as::set_uniform_bool(const u32& shader_program, const char* name, const bool& value)
{
	glUseProgram(shader_program);
	glUniform1i(glGetUniformLocation(shader_program, name), (i32)value);
}

void as::set_uniform_integer(const u32& shader_program, const char* name, const i32& value)
{
	glUseProgram(shader_program);
	glUniform1i(glGetUniformLocation(shader_program, name), value);
}

void as::set_uniform_f32(const u32& shader_program, const char* name, const f32& value)
{
	glUseProgram(shader_program);
	glUniform1f(glGetUniformLocation(shader_program, name), value);
}

void as::set_uniform_vec2(const u32& shader_program, const char* name, const glm::vec2& value)
{
	glUseProgram(shader_program);
	glUniform2fv(glGetUniformLocation(shader_program, name), 1, &value[0]);
}

void as::set_uniform_vec3(const u32& shader_program, const char* name, const glm::vec3& value)
{
	glUseProgram(shader_program);
	glUniform3fv(glGetUniformLocation(shader_program, name), 1, &value[0]);
}

void as::set_uniform_vec4(const u32& shader_program, const char* name, const glm::vec4& value)
{
	glUseProgram(shader_program);
	glUniform4fv(glGetUniformLocation(shader_program, name), 1, &value[0]);
}

void as::set_uniform_mat2(const u32& shader_program, const char* name, const glm::mat2& value)
{
	glUseProgram(shader_program);
	glUniformMatrix2fv(glGetUniformLocation(shader_program, name), 1, GL_FALSE, &value[0][0]);
}

void as::set_uniform_mat3(const u32& shader_program, const char* name, const glm::mat3& value)
{
	glUseProgram(shader_program);
	glUniformMatrix3fv(glGetUniformLocation(shader_program, name), 1, GL_FALSE, &value[0][0]);
}

void as::set_uniform_mat4(const u32& shader_program, const char* name, const glm::mat4& value)
{
	glUseProgram(shader_program);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, name), 1, GL_FALSE, &value[0][0]);
}

void as::bind_uniforms(const as::shader& shader)
{
	for (u16 i = 0 ; i < shader.texture_count ; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		set_uniform_integer(shader.shader_program, shader.textures[i]->uniform_name, i);
		glBindTexture(GL_TEXTURE_2D, shader.textures[i]->id);
		check_gl_error();
	}
}

bool as::load_texture(const char* path, as::texture& out_texture)
{
	if (path)
	{
		std::string path_string = util::get_current_path() + "/../" + std::string(path);
		AS_LOG(LV_LOG, "Loading texture [" + path_string + "]");
		strcpy(out_texture.path, path_string.c_str());
		glGenTextures(1, &out_texture.id);
		
		// load image, create texture and generate mipmaps
		//stbi_set_flip_vertically_on_load(true);
		u8* data = stbi_load(path_string.c_str(), &out_texture.width, &out_texture.height, &out_texture.number_of_channels, 0);
		if (data)
		{
			GLenum format = GL_RGB;
			if (out_texture.number_of_channels == 1)
			{
				format = GL_RED;
			}
			else if (out_texture.number_of_channels == 3)
			{
				format = GL_RGB;
			}
			else if (out_texture.number_of_channels == 4)
			{
				format = GL_RGBA;
			}

			glBindTexture(GL_TEXTURE_2D, out_texture.id);
			glTexImage2D(GL_TEXTURE_2D, 0, format, out_texture.width, out_texture.height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			AS_LOG(LV_WARNING, "Cannot load texture [" + path_string + "]");
			stbi_image_free(data);
			return false;
		}
		return true;
	}
	AS_LOG(LV_WARNING, "Cannot load texture, nullptr");
	return false;
}

void as::add_texture_to_shader(as::texture* texture, as::shader*& shader)
{
	if (texture && shader)
	{
		shader->texture_count++;
		if (shader->texture_count <= MAX_TEXTURE_COUNT_PER_SHADER)
		{
			shader->textures[shader->texture_count - 1] = texture;
		}
		else
		{
			AS_LOG(LV_WARNING, "Cannot add texture to shader, it exceeds the number of possible textures per shader");
		}
	}
	else
	{
		AS_LOG(LV_WARNING, "Cannot add texture to shader, nullptr ptr");
	}
	//shader->textures.push_back(textures);
}

bool as::deep_copy_texture(const as::texture* source, as::texture*& destination)
{
	if (source && destination)
	{
		memcpy(destination, source, sizeof(as::texture));
		return true;
	}
	AS_LOG(LV_ERROR, "Cannot deep copy texture, nullptr");
	return false;
}

bool as::deep_copy_texture(const as::texture* source, void*& destination)
{
	as::texture* destination_texture = static_cast<as::texture*>(destination);
	return deep_copy_texture(source, destination_texture);
}

size_t as::get_texture_size(const as::texture& texture)
{
	return sizeof(texture);
}

std::string as::to_string(const as::texture* texture)
{
	std::string final_string = "";
	if (texture)
	{
		final_string += "{\n	path:" +
			std::string(texture->path) +
			",\n	height:" +
			std::to_string(texture->height) +
			",\n	width:" +
			std::to_string(texture->width) +
			"\n}";
	}
	return final_string;
}

size_t as::get_texture_size(const as::texture* texture)
{
	if (texture)
	{
		return sizeof(*texture);
	}
	AS_LOG(LV_WARNING, "Texture pointer invalid, cannot get the size");
	return 0;
}

void as::delete_texture_data(as::texture*& texture)
{
	if (texture)
	{
		AS_FREE(texture);
	}
}

bool as::create_mesh(const std::vector<as::vertex>& vertices, const std::vector<u32>& indices, as::mesh*& out_mesh)
{
	AS_LOG(LV_LOG, "Creating mesh with [" + std::to_string(vertices.size())+ "] vertices, and [" + std::to_string(indices.size())+ "] indices");

	if (vertices.empty() || indices.empty())
	{
		AS_LOG(LV_WARNING, "Cannot create mesh, both vertices and indices have to be > 0");
		return false;
	}

	i64 vertices_size = sizeof(as::vertex) * vertices.size();
	i64 indices_size = sizeof(u32) * indices.size();

	if (out_mesh)
	{
		AS_FREE(out_mesh);
	}
	out_mesh = (as::mesh*)AS_MALLOC(sizeof(as::mesh));
	*out_mesh = as::mesh();
	out_mesh->vertex_count = vertices.size();
	out_mesh->index_count = indices.size();
	out_mesh->vertices = (as::vertex*)AS_MALLOC(vertices_size);
	out_mesh->indices = (u32*)AS_MALLOC(indices_size);
	memcpy(out_mesh->vertices, vertices.data(), vertices_size);
	memcpy(out_mesh->indices, indices.data(), indices_size);

	// create buffers/arrays
	glGenVertexArrays(1, &out_mesh->VAO);
	glGenBuffers(1, &out_mesh->VBO);
	glGenBuffers(1, &out_mesh->EBO);

	// bind and set buffer
	glBindVertexArray(out_mesh->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, out_mesh->VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices_size, &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out_mesh->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, &indices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(as::vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(as::vertex), (void*)offsetof(as::vertex, normal));
	// vertex texture coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(as::vertex), (void*)offsetof(as::vertex, tex_coords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(as::vertex), (void*)offsetof(as::vertex, tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(as::vertex), (void*)offsetof(as::vertex, bitangent));
	// ids
	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(as::vertex), (void*)offsetof(as::vertex, bone_ids));
	// weights
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(as::vertex), (void*)offsetof(as::vertex, weights));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return check_gl_error();
}

bool as::assign_shader(as::shader* shader, as::mesh*& out_mesh)
{
	if (out_mesh && shader)
	{
		out_mesh->shader_ptr = shader;
		return true;
	}
	return false;
}

bool as::draw(const as::mesh* mesh)
{
	if (mesh)
	{
		if (mesh->shader_ptr)
		{
			as::bind_uniforms(*mesh->shader_ptr);
			glBindVertexArray(mesh->VAO);
			glDrawElements(GL_TRIANGLES, static_cast<u32>(mesh->index_count), GL_UNSIGNED_INT, 0);

			// reset
			glBindVertexArray(0);
			glActiveTexture(GL_TEXTURE0);
		}
		else
		{
			AS_LOG(LV_WARNING, "Cannot render a mesh without a shader pointer");
			return false;
		}
	}
	else
	{
		AS_LOG(LV_WARNING, "Cannot render an invalid mesh");
		return false;
	}
	return check_gl_error();
}

bool as::deep_copy_mesh(const as::mesh* source, as::mesh*& destination)
{
	if (source && destination)
	{
		memcpy(destination, source, sizeof(as::mesh));
		destination->indices = source->indices;
		destination->vertices = source->vertices;
		return true;
	}
	AS_LOG(LV_ERROR, "Cannot deep copy mesh, nullptr");
	return false;
}

bool as::deep_copy_mesh(const as::mesh* source, void*& destination)
{
	as::mesh* destination_mesh = static_cast<as::mesh*>(destination);
	return deep_copy_mesh(source, destination_mesh);
}

std::string as::to_string(const as::mesh* mesh)
{
	std::string final_string = "";
	if (mesh)
	{
		final_string += "mesh\n{\n	indices:" +
			std::to_string(mesh->index_count) +
			", vertices:" +
			std::to_string(mesh->vertex_count) +
			",\n	shader:\n" +
			as::to_string(mesh->shader_ptr) +
			"\n}";
	}
	return final_string;
}

void as::delete_mesh_data(as::mesh*& mesh)
{
	if (mesh)
	{
		if (mesh->indices)
		{
			AS_FREE(mesh->indices);
			mesh->indices = nullptr;
		}
		if (mesh->vertices)
		{
			AS_FREE(mesh->vertices);
			mesh->vertices = nullptr;
		}
		AS_FREE(mesh);
		mesh = nullptr;
	}
}

bool load_material_textures(aiMaterial* material, aiTextureType type, const char* type_name, std::vector<as::texture>& out_textures)
{
	for (u32 i = 0; i < material->GetTextureCount(type); i++)
	{
		aiString str;
		material->GetTexture(type, i, &str);
		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		bool skip = false;
		for (u32 j = 0; j < out_textures.size(); j++)
		{
			if (std::strcmp(out_textures[j].path, str.C_Str()) == 0)
			{
				skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}
		if (!skip)
		{   // if texture hasn't been loaded already, load it
			as::texture texture;
			texture.id = i;
			memcpy(texture.uniform_name, type_name, sizeof(type_name));
			if (as::load_texture(str.C_Str(), texture))
			{
				out_textures.push_back(texture);
			}
		}
	}
	return true;
}

bool process_mesh(aiMesh* mesh, const aiScene* scene, as::mesh*& out_mesh, std::vector<as::texture>& out_textures)
{
	std::vector<as::vertex> vertices;
	std::vector<u32> indices;

	// walk through each of the mesh's vertices
	for (u32 i = 0; i < mesh->mNumVertices; i++)
	{
		as::vertex vertex;

		// positions
		vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

		// normals
		if (mesh->HasNormals())
		{
			vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		}
		// texture coordinates
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vertex.tex_coords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else
		{
			vertex.tex_coords = glm::vec2(0.0f, 0.0f);
		}
		// tangent
		if (mesh->mTangents)
		{
			vertex.tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		}
		else
		{
			vertex.tangent = glm::vec3(.0f);
		}
		// bitangent
		if (mesh->mBitangents)
		{
			vertex.bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}
		else
		{
			vertex.bitangent = glm::vec3(.0f);
		}

		vertices.push_back(vertex);
	}

	// now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (u32 i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (u32 j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	
	// automatic materials disabled for now
	// 1. diffuse maps
	//load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse", out_textures);
	// 2. specular maps
	//load_material_textures(material, aiTextureType_SPECULAR, "texture_specular", out_textures);
	// 3. normal maps
	//load_material_textures(material, aiTextureType_HEIGHT, "texture_normal", out_textures);
	// 4. height maps
	//load_material_textures(material, aiTextureType_AMBIENT, "texture_height", out_textures);

	return as::create_mesh(vertices, indices, out_mesh);
}

bool process_node(const aiScene* scene, aiNode* node, std::vector<as::mesh*>& out_meshes, std::vector<as::texture>& out_textures)
{
	if (node)
	{
		// process each mesh located at the current node
		for (u32 i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			as::mesh* processed_mesh = nullptr;
			if (process_mesh(mesh, scene, processed_mesh, out_textures))
			{
				out_meshes.push_back(processed_mesh);
			}
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (u32 i = 0; i < node->mNumChildren; i++)
		{
			process_node(scene, node->mChildren[i], out_meshes, out_textures);
		}
		return true;
	}
	return false;
};

void as::load_model(const char* path, as::model*& out_model, std::vector<as::texture>& out_textures)
{
	if (out_model == nullptr)
	{
		out_model = (as::model*)AS_MALLOC(sizeof(as::model));
		*out_model = as::model();
	}
	std::string full_path = as::util::get_current_path() + "/../" + std::string(path);
	AS_LOG(LV_LOG, "Loading model [" + full_path + "]");
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(full_path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		AS_LOG(LV_WARNING, import.GetErrorString());
		return;
	}
	strcpy(out_model->path, full_path.c_str());
	std::vector <as::mesh*> meshes;
	process_node(scene, scene->mRootNode, meshes, out_textures);
	out_model->mesh_count = meshes.size();
	if (meshes.data())
	{
		out_model->meshes = (as::mesh**)AS_MALLOC(sizeof(as::mesh*) * meshes.size());
		memcpy(out_model->meshes, meshes.data(), sizeof(as::mesh*) * meshes.size());
	}
}

void as::apply_location(const glm::vec3& location, as::model& model)
{
	apply_location(location, model.transform_matrix);
}

void as::apply_rotation(const glm::vec3& rotation, as::model& model)
{
	apply_rotation(rotation, model.transform_matrix);
}

void as::apply_scale(const glm::vec3& scale, as::model& model)
{
	apply_scale(scale, model.transform_matrix);
}

void as::apply_transform(const as::transform& transform, as::model& model)
{
	apply_transform(transform, model.transform_matrix);
}

bool as::draw(const as::model* model, const as::world* world)
{
	if (model && model->mesh_count > 0 && model->meshes[0]->shader_ptr)
	{
		u32 current_shader_program = model->meshes[0]->shader_ptr->shader_program;
		update_draw_uniforms(current_shader_program, world, model->transform_matrix);

		for (u16 i = 0 ; i < model->mesh_count ; i++)
		{
			if (model->meshes[i] && model->meshes[i]->shader_ptr && model->meshes[i]->shader_ptr->shader_program != current_shader_program)
			{
				current_shader_program = model->meshes[i]->shader_ptr->shader_program;
				glUseProgram(current_shader_program);
				update_draw_uniforms(current_shader_program, world, model->transform_matrix); // update again in case shader program changed
			}

			draw(model->meshes[i]);
		}
	}
	return check_gl_error();
}

bool as::deep_copy_model(const as::model* source, as::model* destination)
{
	if (source && destination)
	{
		memcpy(destination, source, sizeof(source));
		return true;
	}
	AS_LOG(LV_ERROR, "Cannot deep copy shader, nullptr");
	return false;
}

bool as::deep_copy_model(const as::model& source, void* destination)
{
	as::model* destination_model = (as::model*)destination;
	if (destination_model)
	{
		return as::deep_copy_model(&source, destination_model);
	}
	return false;
}

size_t as::get_model_size(const as::model& model)
{
	size_t total_size = 0;
	for (u16 i = 0 ; i < model.mesh_count ; i++)
	{
		if (model.meshes[i])
		{
			total_size += model.meshes[i]->vertex_count * sizeof(as::vertex);
			total_size += model.meshes[i]->index_count * sizeof(u32);
			total_size += sizeof(as::mesh);
		}
	}
	total_size += sizeof(model);
	return total_size;
}

std::string as::to_string(const as::model* model)
{
	std::string final_string = "";
	if (model)
	{
		final_string += "model:\n{\n	path:" +
			std::string(model->path) +
			",\n	meshes:";
		for (u16 i = 0; i < model->mesh_count; i++)
		{
			final_string += as::to_string(model->meshes[i]);
			final_string += ",\n	";
		}
		final_string += "\n}";
	}
	return final_string;
}

bool as::delete_model_data(as::model*& model)
{
	for (u16 i = 0; i < model->mesh_count; i++)
	{
		delete_mesh_data(model->meshes[i]);
	}
	AS_FREE(model->meshes);
	model->meshes = nullptr;
	return check_gl_error();
}

void as::clear_background()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool as::draw(const std::vector<as::model*>& models, const as::world* world)
{
	for (const as::model* current_model : models)
	{
		AS_ASSERT(current_model, "Invalid model pointer");
		draw(current_model, world);
	}
	return check_gl_error();
}

bool as::draw(const std::vector<const as::model*>& models, const as::world* world)
{
	for (const as::model* current_model : models)
	{
		AS_ASSERT(current_model, "Invalid model pointer");
		draw(current_model, world);
	}
	return check_gl_error();
}

void as::update_draw_uniforms(const u32& shader_program, const as::world* world, const glm::mat4& model_transform)
{
	// camera
	as::camera* camera = find_active_camera(world);
	if (camera)
	{
		as::set_uniform_mat4(shader_program, "projection", as::get_matrix_projection(*camera)); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		as::set_uniform_mat4(shader_program, "view", as::get_matrix_view(*camera));
	}
	else
	{
		AS_LOG(LV_WARNING, "No active camera");
	}

	as::set_uniform_mat4(shader_program, "model", model_transform);
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

glm::mat4 as::get_matrix_view(const as::camera& camera)
{
	return glm::lookAt(camera.transform_mat.location, camera.transform_mat.location + camera.front, camera.up);
}

glm::mat4 as::get_matrix_projection(const as::camera& camera)
{
	if (camera.aspect_ratio > 0.01)
	{
		return glm::perspective(camera.fov, camera.aspect_ratio, camera.near_plane, camera.far_plane);
	}
	return glm::mat4();
}

void as::update_camera_vectors(as::camera* camera)
{
	// calculate the new Front vector
	glm::vec3 front;
	f32 pitch = camera->transform_mat.rotation.y;
	f32 yaw = camera->transform_mat.rotation.z;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	camera->front = glm::normalize(front);
	// also re-calculate the Right and Up vector
	camera->right = glm::normalize(glm::cross(camera->front, camera->world_up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	camera->up = glm::normalize(glm::cross(camera->right, camera->front));
}

size_t as::get_camera_size(const as::camera& camera)
{
	return sizeof(camera);
}

std::string as::to_string(const as::camera* camera)
{
	std::string final_string = "";
	if (camera)
	{
		final_string += "camera:\n{\n	transform:" +
			util::transform_to_string(camera->transform_mat) +
			"\n}";
	}
	return final_string;
}

bool as::draw(const as::world* world, const f32& aspect_ratio)
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
						out_camera->aspect_ratio = aspect_ratio;
						camera_to_use = out_camera;
					}
				}
			}
		}

		if (models_to_draw.empty())
		{
			//AS_LOG(LV_WARNING, "Cannot draw 0 models, check the world content");
			return false;
		}
		else if (!camera_to_use)
		{
			AS_LOG(LV_WARNING, "Cannot draw with no active camera, check the world content");
			return false;
		}
		else
		{
			return draw(models_to_draw, world);
		}
	}
	return false;
}

size_t as::get_world_size(const as::world& world)
{
	size_t total_size = 0;
	for (u16 i = 0; i < world.entities_count; i++)
	{
		if (world.entities[i])
		{
			get_entity_size(world.entities[i]);
		}
	}
	return total_size;
}

std::string as::to_string(const as::world* world)
{
	std::string final_string = "";
	if (world)
	{
		final_string += "world:\n{\n	is_active:" +
			util::bool_to_string(world->is_active) +
			",\n	entities:\n[\n	";
		for (u16 i = 0 ; i < world->entities_count ; i++)
		{
			final_string += as::to_string(world->entities[i]);
			if (world->entities_count != i - 1)
			{
				final_string += ",\n	";
			}
		}
		final_string += "]\n}";
	}
	return final_string;
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

