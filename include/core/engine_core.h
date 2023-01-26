#include "engine_types.h"

namespace as
{
	// generic 
	void configure();

	// shaders
	bool create_shader(const char* vert_shader_src, const char* frag_shader_src, as::shader& out_shader);
	bool create_shader_from_files(const char* vert_shader_path, const char* frag_shader_path, as::shader& out_shader);
	void create_shader_program(u32& out_shader_program);
	bool bind_shaders_to_program(const u32& shader_program, as::shader& shader_to_attach);
	bool deep_copy_shader(const as::shader* source, as::shader*& destination);
	bool deep_copy_shader(const as::shader* source, void*& destination);
	void delete_shader(const as::shader& shader);
	void delete_shader_program(const u32& shader_program);
	
	// shader uniforms
	void add_uniform(const as::uniform& uniform_to_add, as::shader& shader);
	void set_uniform_bool(const u32& shader_program, const char* name, const bool& value);
	void set_uniform_integer(const u32& shader_program, const char* name, const i32& value);
	void set_uniform_f32(const u32& shader_program, const char* name, const f32& value);
	void set_uniform_vec2(const u32& shader_program, const char* name, const glm::vec2& value);
	void set_uniform_vec3(const u32& shader_program, const char* name, const glm::vec3& value);
	void set_uniform_vec4(const u32& shader_program, const char* name, const glm::vec4& value);
	void set_uniform_mat2(const u32& shader_program, const char* name, const glm::mat2& value);
	void set_uniform_mat3(const u32& shader_program, const char* name, const glm::mat3& value);
	void set_uniform_mat4(const u32& shader_program, const char* name, const glm::mat4& value);
	void bind_uniforms(const as::shader& shader);

	// textures
	bool load_texture(const char* path, as::texture& out_texture);
	void add_textures_to_shader(const std::vector<as::texture>& textures, as::shader& shader);
	void add_texture_to_shader(const as::texture& textures, as::shader& shader);
	bool deep_copy_texture(const as::texture* source, as::texture*& destination);
	bool deep_copy_texture(const as::texture* source, void*& destination);

	// meshes
	bool create_mesh(const std::vector<as::vertex>& vertices, const std::vector<u32>& indices, as::mesh& out_mesh);
	bool assign_shader(as::shader& shader, as::mesh& out_meshe);
	bool assign_shader(as::shader& shader, std::vector<as::mesh>& out_meshes);
	bool deep_copy_mesh(const as::mesh* source, as::mesh*& destination);
	bool deep_copy_mesh(const as::mesh* source, void*& destination);
	bool draw(const as::mesh& mesh);
	bool delete_mesh_data(as::mesh& mesh);

	// models
	void load_model(const char* path, as::model& out_model, std::vector<as::texture>& out_textures);
	bool draw(const as::model& model, const as::camera& camera);
	bool deep_copy_model(const as::model& source, as::model& destination);
	bool deep_copy_model(const as::model& source, void* destination);
	//bool deep_copy_model(void* source, void* destination);
	bool delete_model_data(as::model& model);

	// render
	void clear_background();
	bool draw(const std::vector<as::model>& models, const as::camera& camera);
	void update_draw_uniforms(const u32& shader_program, const as::camera& camera, const glm::mat4& model_transform);

	// camera
	glm::mat4 get_matrix_view(const as::camera& camera);
	glm::mat4 get_matrix_projection(const as::camera& camera);
	void update_camera_vectors(as::camera& camera);
};