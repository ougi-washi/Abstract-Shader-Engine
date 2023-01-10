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

	// objects
	

	// textures
	bool load_texture(const char* path, as::texture& out_texture);

	// meshes
	bool create_mesh(const std::vector<as::vertex>& vertices, const std::vector<u32>& indices, as::mesh& out_mesh);
	bool assign_shader(as::shader& shader, as::mesh& out_meshes);
	bool draw(as::mesh& mesh);
	bool delete_mesh_data(as::mesh* mesh);

	// models
	void load_model(const char* path, as::model& out_model);
	bool draw(as::model& model);

	// render
	void clear_background();
	bool draw(const std::vector<as::object>& objects);

	// camera
	glm::mat4 get_matrix_view(const as::camera& camera);
	void update_camera_vectors(as::camera& camera);
};