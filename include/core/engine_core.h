#include "engine_types.h"
#include "engine_memory.h"

namespace as
{
	// general
	void init_window(const u16& width, const u16& height, const char* title);
	void init_gl();

	// files / entities
	std::string variable_enum_to_string(const as::entity_type& in_type);
	as::entity_type variable_string_to_enum(const std::string& in_type_str);
	void set_path(const std::string& path, const bool& absolute_path, as::entity_data& out_entity_data);
	as::world* get_world_from_file(const std::string& path, const bool& absolute_path);
	bool get_model_from_file(const std::string& path, const bool& absolute_path, as::model& out_model);
	bool get_shader_from_file(const std::string& path, const bool& absolute_path, as::shader& out_shader);
	bool get_texture_from_file(const std::string& path, const bool& absolute_path, as::texture& out_texture);
	bool get_camera_from_file(const std::string& path, const bool& absolute_path, as::camera& out_camera);
	bool get_light_from_file(const std::string& path, const bool& absolute_path, as::light& out_light);

	// transform
	void apply_location(const Vector3& location, Matrix& transform_matrix);
	void apply_rotation(const Vector3& rotation, Matrix& transform_matrix);
	void apply_scale(const Vector3& scale, Matrix& transform_matrix);
	void apply_transform(const Transform& transform, Matrix& transform_matrix);

	// entity
	bool is_valid(const as::entity_data& entity_data);

	// camera
	as::camera* find_active_camera(const as::world& world);

	// light
	void update_lights_uniforms(const Shader& shader, as::light** lights, const u32& lights_count);

	// world
	bool draw(const as::world* world);

	// render
	void clear_background();

	// pool
	as::world& get_incremental_world_from_pool(); // This will increase world count
	as::model& get_incremental_model_from_pool(); // This will increase model count
	as::shader& get_incremental_shader_from_pool(); // This will increase shader count
	as::texture& get_incremental_texture_from_pool(); // This will increase texture count
	as::light& get_incremental_light_from_pool(); // This will increase light count
	as::camera& get_incremental_camera_from_pool(); // This will increase camera count

};