#include "engine_types.h"

namespace as
{
	// generic 
	void configure();
	void delete_vertex_array(const u32& VAO);

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
	bool assign_shader(as::object& object, u32& VAO, as::shader& shader);
	bool delete_object_data(as::object* object);

	// textures
	bool load_texture(const char* path, as::texture& out_texture);

	// meshes
	bool create_mesh(const as::vertex* vertices, const u32& vertices_count, const u32* indices, const u32& indices_count, as::mesh& out_mesh);
	bool draw(const u32& shader_program, as::mesh& mesh);

	// render
	void clear_background();
	bool draw(const u32& shader_program, const u32& VAO, const std::vector<as::object>& objects);

	// camera
	glm::mat4 get_matrix_view(const as::camera& camera);
	void update_camera_vectors(as::camera& camera);
};