#include "engine_types.h"

namespace as
{
	// generic 
	void delete_vertex_array(const u32& VAO);

	// shaders
	bool create_shader(const char* vert_shader_src, const char* frag_shader_src, as::shader& out_shader);
	bool create_shader_from_files(const char* vert_shader_path, const char* frag_shader_path, as::shader& out_shader);
	void create_shader_program(u32& out_shader_program);
	bool bind_shaders_to_program(const u32& shader_program, const as::shader& shader_to_attach);
	void delete_shader(const as::shader& shader);
	void delete_shader_program(const u32& shader_program);
	
	// shader uniforms
	void add_uniform(const as::uniform& uniform_to_add, as::shader& shader);
	void set_uniform_bool(const u32& shader_program, const char* name, const bool& value);
	void set_uniform_integer(const u32& shader_program, const char* name, const i32& value);
	void set_uniform_f32(const u32& shader_program, const char* name, const f32& value);
	void bind_uniforms(const as::shader& shader);
	void bind_uniforms(const as::object& object);
	void bind_uniforms(const std::vector<as::object>& objects);

	// objects
	bool initialize_object(const f32* vertices, const u32& vertices_count, const i32* indices, const u32& indices_count, u32& VAO, as::object& out_object);
	bool assign_shader(as::object& object, u32& VAO, as::shader& shader);
	bool delete_object_data(as::object* object);

	// textures
	bool load_texture(const char* path, as::texture& out_texture);

	// render
	void clear_background();
	bool draw(const u32& shader_program, const u32& VAO, const std::vector<as::object>& objects);
};