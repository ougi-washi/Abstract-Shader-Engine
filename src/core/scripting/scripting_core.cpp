#include "scripting/scripting_core.h"
#include "engine_utility.h"

std::string as::script::variable_enum_to_string(const variable_type& in_type) 
{ 
	return variable_type_strings[in_type]; 
}

as::script::variable_type as::script::variable_string_to_enum(const std::string& in_type_str) 
{ 
	for (u8 i = 0 ; i < variable_type_strings.size() ; i++)
	{
		if (variable_type_strings[i] == in_type_str)
		{
			return (variable_type)i;
		}
	}
	return variable_type::NONE;
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

bool as::script::parse_file(const std::string& path, const bool& absolute_path, as::script::entity& out_entity)
{
	std::string new_path;
	if (absolute_path)
	{
		new_path = path;
	}
	else
	{
		new_path = as::util::get_current_path() + "/../" + path;
	}

	json json_data = as::util::read_json_file(new_path);
	if (json_data.contains("type"))
	{
		std::string entity_type = json_data["type"].get<std::string>();
		out_entity.type = variable_string_to_enum(entity_type);

		if (out_entity.type == as::script::WORLD)
		{
			if (json_data.contains("entities"))
			{
				std::vector<std::string> entities = json_data["entities"].get<std::vector<std::string>>();
				
			}
		}

		if (out_entity.type == as::script::MODEL)
		{
			as::model out_model;

			if (json_data.contains("path"))
			{
				std::string model_path = json_data["path"].get<std::string>();
				std::vector<as::texture> out_textures;
				as::load_model(model_path.c_str(), out_model, out_textures);
			}
			else
			{
				AS_LOG(LV_WARNING, "Model json file does not have path");
				script::delete_entity_data(out_entity);
				return false;
			}
			if (json_data.contains("shaders"))
			{
				std::vector<std::string> shaders = json_data["shaders"].get<std::vector<std::string>>();
				for (u16 i = 0 ; i < shaders.size() ; i++)
				{
					as::script::entity shader_entity;
					if (parse_file(shaders[i], absolute_path, shader_entity))
					{
						if (out_model.meshes.size() > i && shader_entity.data_ptr)
						{
							as::assign_shader(*static_cast<as::shader*>(shader_entity.data_ptr), out_model.meshes[i]);
						}
						out_entity.sub_entities.push_back(shader_entity);
					}
				}
			}
			else
			{
				AS_LOG(LV_WARNING, "Model json file does not have shaders");
				script::delete_entity_data(out_entity);
				return false;
			}
			out_entity.data_ptr = new as::model(out_model);
		}
		if (out_entity.type == as::script::SHADER)
		{
			as::shader out_shader;
			std::string vertex_path;
			std::string fragment_path;
			if (json_data.contains("vertex_path"))
			{
				vertex_path = json_data["vertex_path"].get<std::string>();
			}
			else
			{
				AS_LOG(LV_WARNING, "Shader json file does not have vertex shader");
				script::delete_entity_data(out_entity);
				return false;
			}
			if (json_data.contains("fragment_path"))
			{
				fragment_path = json_data["fragment_path"].get<std::string>();
			}
			else
			{
				AS_LOG(LV_WARNING, "Shader json file does not have fragment shader");
				script::delete_entity_data(out_entity);
				return false;
			}
			if (json_data.contains("textures"))
			{
				std::vector<std::string> textures = json_data["textures"].get<std::vector<std::string>>();
				for (u16 i = 0; i < textures.size(); i++)
				{
					as::script::entity texture_entity;
					if (parse_file(textures[i], absolute_path, texture_entity))
					{
						if (texture_entity.data_ptr)
						{
							as::add_texture_to_shader(*static_cast<as::texture*>(texture_entity.data_ptr), out_shader);
						}
						out_entity.sub_entities.push_back(texture_entity);
					}
				}
			}
			if (as::create_shader_from_files(vertex_path.c_str(), fragment_path.c_str(), out_shader))
			{
				u32 shader_program;
				as::create_shader_program(shader_program);
				as::bind_shaders_to_program(shader_program, out_shader);
				out_entity.data_ptr = new as::shader(out_shader);
			}
			else
			{
				AS_LOG(LV_WARNING, "Could not create the shader");
				script::delete_entity_data(out_entity);
				return false;
			}
		}
		if (out_entity.type == as::script::TEXTURE)
		{
			if (json_data.contains("path"))
			{
				std::string texture_path = json_data["path"].get<std::string>();
				as::texture out_texture;
				if (json_data.contains("uniform_name"))
				{
					std::string uniform_name = json_data["uniform_name"].get<std::string>();
					strcpy(out_texture.uniform_name, uniform_name.c_str());
				}
				if (as::load_texture(texture_path.c_str(), out_texture))
				{
					//out_entity.data_ptr = &out_texture;
					//memcpy(out_entity.data_ptr, &out_texture, sizeof(as::texture));
					out_entity.data_ptr = new as::texture(out_texture);
					//deep_copy_texture(&out_texture, out_entity.data_ptr);
				}
				else
				{
					AS_LOG(LV_WARNING, "Could not load the texture");
					script::delete_entity_data(out_entity);
					return false;
				}
			}
			else
			{
				AS_LOG(LV_WARNING, "Texture json file does not have path");
				script::delete_entity_data(out_entity);
				return false;
			}
		}
	}
	return true;
}

void as::script::delete_entity_data(as::script::entity& entity)
{
	for (as::script::entity& sub_entity : entity.sub_entities)
	{
		delete_entity_data(sub_entity);
	}

	as::model model;
	if (as::script::get_model_from_entity(entity, model))
	{
		as::delete_model_data(model);
	}

	delete(entity.data_ptr);
	delete(entity.fn_ptr);
}

bool as::script::get_model_from_entity(const as::script::entity& entity, as::model& out_model)
{
	if (entity.type == as::script::MODEL)
	{
		if (entity.data_ptr)
		{
			out_model = *static_cast<as::model*>(entity.data_ptr);
			return true;
		}
		else
		{
			AS_LOG(LV_ERROR, "Cannot get model from entity, data is nullptr")
		}
	}
	return false;
}

bool as::script::draw(const std::vector<as::script::entity>& entities, const as::camera& camera)
{
	for (const as::script::entity& current_entity : entities)
	{
		as::model current_model;
		if (as::script::get_model_from_entity(current_entity, current_model))
		{
			as::draw(current_model, camera);
		}
	}
	return true;
}
