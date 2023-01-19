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

bool as::create_shader(const char* vert_shader_src, const char* frag_shader_src, as::shader& out_shader)
{
	bool vert_shader_result, frag_shader_result;
	// vertex shader
	out_shader.vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(out_shader.vertex_shader, 1, &vert_shader_src, NULL);
	glCompileShader(out_shader.vertex_shader);
	vert_shader_result = check_compilation_status(out_shader.vertex_shader);

	// fragment shader
	out_shader.fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(out_shader.fragment_shader, 1, &frag_shader_src, NULL);
	glCompileShader(out_shader.fragment_shader);
	frag_shader_result = check_compilation_status(out_shader.fragment_shader);

	return (vert_shader_result && frag_shader_result);
}

bool as::create_shader_from_files(const char* vert_shader_path, const char* frag_shader_path, as::shader& out_shader)
{
	if (vert_shader_path && frag_shader_path)
	{
		std::string vert_shader_path_string = util::get_current_path() + "/../" + std::string(vert_shader_path);
		std::string frag_shader_path_string = util::get_current_path() + "/../" + std::string(frag_shader_path);

		char* vert_shader = util::read_file(vert_shader_path_string.c_str());
		char* frag_shader = util::read_file(frag_shader_path_string.c_str());
		if (vert_shader && frag_shader)
		{
			return create_shader(vert_shader, frag_shader, out_shader);
		}
	}
	return false;
}

void as::create_shader_program(u32& out_shader_program)
{
	out_shader_program = glCreateProgram();
}

bool as::bind_shaders_to_program(const u32& shader_program, as::shader& shader_to_attach)
{
	glAttachShader(shader_program, shader_to_attach.vertex_shader);
	glAttachShader(shader_program, shader_to_attach.fragment_shader);
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
	shader_to_attach.shader_program = shader_program;
	return check_gl_error();
}

void as::delete_shader(const as::shader& shader)
{
	AS_LOG(LV_LOG, "Deleting shader");
	glDeleteShader(shader.vertex_shader);
	glDeleteShader(shader.fragment_shader);
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
	for (u16 i = 0 ; i < shader.textures.size() ; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		set_uniform_integer(shader.shader_program, shader.textures[i].uniform_name, i);
		glBindTexture(GL_TEXTURE_2D, shader.textures[i].id);
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

void as::add_textures_to_shader(const std::vector<as::texture>& textures, as::shader& shader)
{
	for (const as::texture& current_texture : textures)
	{
		shader.textures.push_back(current_texture);
	}
}

bool as::create_mesh(const std::vector<as::vertex>& vertices, const std::vector<u32>& indices, as::mesh& out_mesh)
{
	AS_LOG(LV_LOG, "Creating mesh with [" + std::to_string(vertices.size())+ "] vertices, and [" + std::to_string(indices.size())+ "] indices");

	if (vertices.empty() || indices.empty())
	{
		AS_LOG(LV_WARNING, "Cannot create mesh, both vertices and indices have to be > 0");
		return false;
	}

	i64 vertices_size = sizeof(as::vertex) * vertices.size();
	out_mesh.vertices = vertices;

	i64 indices_size = sizeof(u32) * indices.size();
	out_mesh.indices = indices;

	// create buffers/arrays
	glGenVertexArrays(1, &out_mesh.VAO);
	glGenBuffers(1, &out_mesh.VBO);
	glGenBuffers(1, &out_mesh.EBO);

	// bind and set buffer
	glBindVertexArray(out_mesh.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, out_mesh.VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices_size, &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out_mesh.EBO);
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

bool as::assign_shader(as::shader& shader, as::mesh& mesh)
{
	AS_LOG(LV_LOG, "Assigning shader to mesh");
	mesh.shader_ptr = &shader;
	return true;
}

bool as::draw(as::mesh& mesh)
{
	if (mesh.shader_ptr)
	{
		as::bind_uniforms(*mesh.shader_ptr);
		glBindVertexArray(mesh.VAO);
		glDrawElements(GL_TRIANGLES, static_cast<u32>(mesh.indices.size()), GL_UNSIGNED_INT, 0);

		// reset
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
	}
	else
	{
		AS_LOG(LV_WARNING, "Cannot render a mesh without a shader pointer");
		return false;
	}
	return check_gl_error();
}

bool as::delete_mesh_data(as::mesh& mesh)
{
	AS_LOG(LV_LOG, "Deleting object data");
	glDeleteBuffers(1, &mesh.VBO);
	glDeleteBuffers(1, &mesh.EBO);
	glDeleteVertexArrays(1, &mesh.VAO);
	return check_gl_error();
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

bool process_mesh(aiMesh* mesh, const aiScene* scene, as::mesh& out_mesh, std::vector<as::texture>& out_textures)
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
	
	// 1. diffuse maps
	load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse", out_textures);
	// 2. specular maps
	load_material_textures(material, aiTextureType_SPECULAR, "texture_specular", out_textures);
	// 3. normal maps
	load_material_textures(material, aiTextureType_HEIGHT, "texture_normal", out_textures);
	// 4. height maps
	load_material_textures(material, aiTextureType_AMBIENT, "texture_height", out_textures);

	return as::create_mesh(vertices, indices, out_mesh);
}

bool process_node(const aiScene* scene, aiNode* node, std::vector<as::mesh>& out_meshes, std::vector<as::texture>& out_textures)
{
	if (node)
	{
		// process each mesh located at the current node
		for (u32 i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			as::mesh processed_mesh;
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

void as::load_model(const char* path, as::model& out_model, std::vector<as::texture>& out_textures)
{
	std::string full_path = as::util::get_current_path() + "/../" + std::string(path);
	AS_LOG(LV_LOG, "Loading model [" + full_path + "]");
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(full_path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		AS_LOG(LV_WARNING, import.GetErrorString());
		return;
	}
	strcpy(out_model.path, full_path.c_str());
	process_node(scene, scene->mRootNode, out_model.meshes, out_textures);
}

bool as::draw(as::model& model)
{
	u32 current_shader_program = 1;
	for (as::mesh& mesh : model.meshes)
	{
		if (mesh.shader_ptr && mesh.shader_ptr->shader_program != current_shader_program)
		{
			current_shader_program = mesh.shader_ptr->shader_program; 
			glUseProgram(current_shader_program);
		}
		draw(mesh);
	}
	return check_gl_error();
}

bool as::delete_model_data(as::model& model)
{
	for (as::mesh& current_mesh : model.meshes)
	{
		delete_mesh_data(current_mesh);
	}
	return check_gl_error();
}

void as::clear_background()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool as::draw(const std::vector<as::object>& objects)
{
	for (const as::object& current_object : objects)
	{
		for (as::model* current_model : current_object.models)
		{
			if (current_model)
			{
				for (as::mesh& mesh : current_model->meshes)
				{
					draw(mesh);
				}
			}
		}
	}
	return check_gl_error();
}

glm::mat4 as::get_matrix_view(const as::camera& camera)
{
	return glm::lookAt(camera.position, camera.position + camera.front, camera.up);
}

void as::update_camera_vectors(as::camera& camera)
{
	// calculate the new Front vector
	glm::vec3 front;
	f32 pitch = camera.rotation.y;
	f32 yaw = camera.rotation.z;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	camera.front = glm::normalize(front);
	// also re-calculate the Right and Up vector
	camera.right = glm::normalize(glm::cross(camera.front, camera.world_up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	camera.up = glm::normalize(glm::cross(camera.right, camera.front));
}
