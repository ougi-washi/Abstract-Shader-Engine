#include "engine_types.h"
#include "engine_memory.h"

namespace as
{
	// general
	void init_window(const u16& width, const u16& height, const char* title);

	// entities
	std::string variable_enum_to_string(const as::ent::entity_type& in_type);
	as::ent::entity_type variable_string_to_enum(const std::string& in_type_str);
	bool get_model_from_entity(const as::entity& entity, as::model& out_model);
	bool get_model_from_entity(const as::entity* entity, as::model*& out_model);
	bool get_camera_from_entity(const as::entity& entity, as::camera& out_camera);
	bool get_camera_from_entity(const as::entity& entity, as::camera*& out_camera);
	bool get_camera_from_entity(const as::entity* entity, as::camera*& out_camera);
	bool get_world_from_entity(const as::entity& entity, as::world& out_world);
	bool get_world_from_entity(const as::entity* entity, as::world*& out_world);
	bool get_shader_from_entity(const as::entity& entity, as::shader& out_shader);
	bool get_shader_from_entity(const as::entity* entity, as::shader*& out_shader);
	bool get_material_from_entity(const as::entity* entity, as::material*& out_material);
	bool get_texture_from_entity(const as::entity& entity, as::texture& out_texture);
	bool get_texture_from_entity(const as::entity* entity, as::texture*& out_texture);
	std::string to_string(const as::entity* entity);
	bool parse_file(const std::string& path, const bool& absolute_path, as::entity*& out_entity);
	void add_sub_entity(as::entity*& parent_entity, as::entity* sub_entity);

	void delete_entity_data(as::entity*& entity);
	void delete_entity_data(void*& data_ptr);

	// transform
	void apply_location(const Vector3& location, Matrix& transform_matrix);
	void apply_rotation(const Vector3& rotation, Matrix& transform_matrix);
	void apply_scale(const Vector3& scale, Matrix& transform_matrix);
	void apply_transform(const Transform& transform, Matrix& transform_matrix);

	// camera
	as::camera* find_active_camera(const as::world* world);

	// light
	u32 get_all_lights(const as::world* world, as::light**& out_lights);
	void update_lights_uniforms(const Shader& shader, as::light** lights, const u32& lights_count);

	// world
	bool draw(const as::world* world);
	void delete_world_data(as::world*& world);

	// render
	void clear_background();
};