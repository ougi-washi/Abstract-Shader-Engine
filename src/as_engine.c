// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_engine.h"
#include "core/as_shader_monitor.h"
#include "core/as_render_queue.h"
#include "core/as_input.h"
#include "core/as_tick.h"
#include "core/as_console.h"
#include "core/as_serialization.h"

typedef struct as_engine
{
	as_render* render;
	as_render_queue* render_queue;
	as_shader_monitor* shader_monitor;
	void* display_context;
	as_camera* camera;
	as_scene* scene;
	as_textures_pool* textures_pool;
	as_screen_objects_group* ui_objects_group;
	as_input_buffer* input_buffer;
	as_tick_system* tick_system;
	as_content* content;
	as_console* console;
} as_engine;

static as_engine engine = {0};

void key_callback(void* window, const i32 key, const i32 scancode, const i32 action, const i32 mods)
{
	as_input_add(engine.input_buffer, key, action);
}

void as_command_create_texture(const char* texture_path, const char* extra_0, const char* extra_1)
{
	if (!texture_path)
	{
		AS_LOG(LV_WARNING, "Cannot create texture, invalid path");
		return;
	}

	as_texture* texture = as_texture_create(texture_path);
	if (!texture)
	{
		AS_LOG(LV_WARNING, "Could not create texture");
		return;
	}
	const i32 content_index = as_content_add_asset(engine.content, texture, AS_ASSET_TYPE_TEXTURE, false, as_texture_destroy);
	AS_FLOG(LV_LOG, "Created texture asset at %d", content_index);
}

void as_command_create_shader(const char* vertex_shader_path, const char* fragment_shader_path, const char* extra_0)
{
	if (!vertex_shader_path || !fragment_shader_path)
	{
		AS_LOG(LV_WARNING, "Cannot create shader, invalid paths");
		return;
	}

	as_shader* shader = as_shader_create(vertex_shader_path, fragment_shader_path);
	if (!shader)
	{
		AS_LOG(LV_WARNING, "Could not create shader");
		return;
	}
	const i32 content_index = as_content_add_asset(engine.content, shader, AS_ASSET_TYPE_SHADER, true, NULL);
	AS_FLOG(LV_LOG, "Created shader asset at %d", content_index);
}

void as_command_create_sphere(const char* size, const char* latitude_divisions, const char* longitude_divisions)
{
	AS_WARNING_RETURN_IF_FALSE(size, "Cannot create sphere, invalid size argument");
	AS_WARNING_RETURN_IF_FALSE(latitude_divisions, "Cannot create sphere, invalid latitude_divisions argument");
	AS_WARNING_RETURN_IF_FALSE(longitude_divisions, "Cannot create sphere, invalid longitude_divisions argument");

	const f32 size_float = atof(size);
	const i32 latitude_divisions_int = atoi(latitude_divisions);
	const i32 longitude_divisions_int = atoi(longitude_divisions);

	as_shape* generated_sphere = as_generate_sphere(size_float, latitude_divisions_int, longitude_divisions_int);

	const i32 content_index = as_content_add_asset(engine.content, generated_sphere, AS_ASSET_TYPE_SHAPE, true, NULL);
	AS_FLOG(LV_LOG, "Created object asset at %d", content_index);
}

void as_command_create_object(const char* shape_index, const char* shader_index, const char* extra_0)
{
	if (!shape_index || !shader_index)
	{
		AS_LOG(LV_WARNING, "Cannot create object, invalid asset indices");
		return;
	}

	const i32 shape_index_int = atoi(shape_index);
	const i32 shader_index_int = atoi(shader_index);

	as_asset* shader = as_content_get_asset(engine.content, shader_index_int);
	as_asset* shape = as_content_get_asset(engine.content, shape_index_int);

	if (!shader || shader->type != AS_ASSET_TYPE_SHADER)
	{
		AS_LOG(LV_WARNING, "Cannot create object, invalid shader");
	}
	if (!shape || shape->type != AS_ASSET_TYPE_SHAPE)
	{
		AS_LOG(LV_WARNING, "Cannot create object, invalid shape");
	}

	as_object* object = as_object_create(shape->ptr, shader->ptr);
	if (!object)
	{
		AS_LOG(LV_WARNING, "Could not create object");
		return;
	}
	const i32 content_index = as_content_add_asset(engine.content, shader, AS_ASSET_TYPE_OBJECT, true, NULL);
	AS_FLOG(LV_LOG, "Created object asset at %d", content_index);
}

// maybe this should be moved to console defines
void as_engine_init_console()
{
	engine.console = as_console_create();
	as_command_mapping_128* command_mappings = as_console_get_mappings(engine.console);

	AS_ARRAY_PUSH_BACK(*command_mappings, ((as_command_mapping){
		"create_texture", 
		"Loads a texture in the content. Usage example: create_texture /resources/textures/example.png", 
		as_command_create_texture, 1}));

	AS_ARRAY_PUSH_BACK(*command_mappings, ((as_command_mapping){
		"create_shader",
		"Loads a shader in the content. Usage example: create_shader /resources/shaders/example_vert.png /resources/shaders/example_frag.png",
		&as_command_create_shader, 2}));

	AS_ARRAY_PUSH_BACK(*command_mappings, ((as_command_mapping){
		"create_sphere",
		"Creates sphere shape from size, latitude_divisions and, longitude_divisions. Usage example: create_sphere 0.6 10 10",
		& as_command_create_sphere, 3}));

	AS_ARRAY_PUSH_BACK(*command_mappings, ((as_command_mapping){
		"create_object",
		"Loads a object in the content. Usage example, where 5 is the index for the shape and 7 is the index for the shader: create_object 5 7",
		&as_command_create_object, 2}));
}

void as_engine_init()
{
	AS_LOG(LV_LOG, "Initializing the engine");
	engine.display_context = as_display_context_create(AS_ENGINE_WINDOW_WIDTH, AS_ENGINE_WINDOW_HEIGHT, AS_ENGINE_WINDOW_NAME, &key_callback);
	engine.render = as_render_create(engine.display_context);
	engine.render_queue = as_rq_create();
	engine.shader_monitor = as_shader_monitor_create(&engine.render->frame_counter, engine.render_queue);
	engine.input_buffer = as_input_create();
	engine.tick_system = as_tick_system_create();
	engine.content = as_content_create();
	engine.ui_objects_group = as_screen_objects_group_create();
	engine.textures_pool = as_textures_pool_create();
	as_engine_init_console();

	while (AS_IS_INVALID(engine.render)) {};
}

void as_engine_clear()
{
	AS_LOG(LV_LOG, "Clearing the engine");

	as_input_destory(engine.input_buffer);
	as_screen_objects_group_destroy(engine.ui_objects_group);
	as_shader_monitored_destroy(engine.shader_monitor);
	as_textures_pool_destroy(engine.textures_pool);
	as_content_destroy(engine.content);
	as_rq_destroy(engine.render_queue);

	as_tick_system_destroy(engine.tick_system);
	as_scene_destroy(engine.render, engine.scene);
	as_render_destroy(engine.render);

	as_display_context_destroy(engine.display_context);
	as_display_context_terminate();

	as_console_destroy(engine.console);

	AS_LOG_MEMORY();
}

bool as_engine_should_loop()
{
	bool should_loop = !as_display_context_should_close(engine.display_context);
	as_input_loop_tick();
	as_display_context_poll_event();
	as_rq_render_start_draw_loop(engine.render_queue, engine.render);
	as_tick_system_execute(engine.tick_system, as_get_delta_time());
	return should_loop;
}

void as_engine_draw()
{
	if (!engine.scene)
	{
		engine.scene = as_scene_create(engine.render, AS_PATH_DEFAULT_SCENE);
	}
	if (engine.camera)
	{
		as_rq_render_draw_frame(engine.render_queue, engine.render, engine.display_context, engine.camera, engine.scene, engine.ui_objects_group);
	}
	as_rq_wait_queue(engine.render_queue);
	as_render_end_draw_loop(engine.render);
}

void as_engine_set_scene(as_scene* scene)
{
	// TODO: maybe it's better to defer to the next frame by storing current scene and next frame scene
	AS_ASSERT(scene, "Cannot set engine scene, invalid scene");
	AS_WAIT_AND_LOCK(engine.render);
	engine.scene = scene;
	AS_UNLOCK(engine.render);
}

as_render* as_engine_get_render()
{
	return engine.render;	
}

as_content* as_engine_get_content()
{
	return engine.content;
}

void as_engine_reset_scene()
{
	AS_WAIT_AND_LOCK(engine.render);
	as_scene_destroy(engine.render, engine.scene);
	engine.scene = as_scene_create(engine.render, AS_PATH_DEFAULT_SCENE);
	AS_UNLOCK(engine.render);
}

f64 as_get_time()
{
	return as_render_get_time(engine.render);
}

f64 as_get_delta_time()
{
	return as_render_get_delta_time(engine.render);
}

as_texture* as_texture_create(const char* texture_path)
{
	as_texture* texture = AS_ARRAY_INCREMENT(*engine.textures_pool);
	as_texture_init(texture, texture_path);

	as_rq_texture_update(engine.render_queue, texture, engine.render);
	return texture;
}

as_shader* as_shader_create(const char* vertex_shader_path, const char* fragment_shader_path)
{
	as_shader* found_shader = as_shader_monitor_find_shader(engine.shader_monitor, vertex_shader_path, fragment_shader_path);
	if (found_shader)
	{
		return found_shader;
	}
	as_shader* shader = as_shader_make(engine.render, vertex_shader_path, fragment_shader_path);
	//as_shader_monitor_add(&engine.render->frame_counter, engine.shader_monitor, shader, shader->filename_vertex, as_rq_shader_recompile);
	//as_shader_monitor_add(&engine.render->frame_counter, engine.shader_monitor, shader, shader->filename_fragment, as_rq_shader_recompile);
	return shader;
}

as_object* as_object_create(as_shape* shape, as_shader* shader)
{
	AS_ASSERT(shader, "Trying create object, but shader is NULL");
	as_object* object = as_object_consturct(engine.render, engine.scene);
	as_rq_object_update(engine.render_queue, engine.render, object, shape, shader);
	return object;
}

as_object* as_object_create_with_tick(as_shape* shape, as_shader* shader, void tick_func_ptr(as_object*, const f64))
{
	AS_ASSERT(tick_func_ptr, "Cannot create ticking object, invalid function ptr");
	as_object* object = as_object_create(shape, shader);
	as_tick_handle* handle = as_tick_handle_create(engine.tick_system);
 	handle->func_ptr = &tick_func_ptr;
	handle->arg = object;
	return object;
}

as_camera* as_camera_create(const as_vec3* position, const as_vec3* target)
{
	return as_camera_make(engine.scene, position, target);
}

void as_camera_set_view(as_camera* camera, const as_camera_type type)
{
	AS_ASSERT(camera, "Trying to set camera as view, but camera is NULL");
	engine.camera = camera;
	engine.camera->type = type;
	as_camera_set_main(engine.scene, camera);
}

as_asset* as_asset_register(void* ptr, const as_asset_type type)
{
	void (*destory_func_ptr)(void*);
	destory_func_ptr = NULL;
	bool free_on_destroy = true;

	if (type == AS_ASSET_TYPE_TEXTURE) 
	{
		destory_func_ptr = as_texture_destroy; 
		free_on_destroy = false;
	}
	//else if (type == AS_ASSET_TYPE_SHADER) { destory_func_ptr = as_engine_texture_destroy; }
	//else if (type == AS_ASSET_TYPE_OBJECT) { destory_func_ptr = as_engine_texture_destroy; }
	//else if (type == AS_ASSET_TYPE_TEXTURE) { destory_func_ptr = as_engine_texture_destroy; }
	//else if (type == AS_ASSET_TYPE_TEXTURE) { destory_func_ptr = as_engine_texture_destroy; }

	const sz index = as_content_add_asset(engine.content, ptr, type, free_on_destroy, destory_func_ptr);
	return as_content_get_asset(engine.content, index);
}

as_screen_object* as_screen_object_create(const char* fragment_shader_path)
{
	as_screen_object* screen_object = AS_ARRAY_INCREMENT(*engine.ui_objects_group);
	
	as_screen_object_init(engine.render, screen_object, fragment_shader_path);
	as_rq_screen_object_update(engine.render_queue, screen_object);
	as_shader_monitor_add(&engine.render->frame_counter, engine.shader_monitor, screen_object, screen_object->filename_fragment, as_rq_screen_object_update);
	return screen_object;
}

void as_screen_object_assign_texture(as_texture* texture)
{

}

sz as_assign_texture_to_shader(as_shader* shader, as_texture* texture)
{
	sz index = as_shader_add_uniform_texture(&shader->uniforms, texture);
	as_shader_update(engine.render, shader);
	return index;
}

bool as_is_pressed(const i32 key)
{
	return as_input_is_pressed(engine.input_buffer, key);
}

bool as_is_released(const i32 key)
{
	return as_input_is_released(engine.input_buffer, key);
}

// locking system
static bool scene_being_saved = false; 
static bool scene_being_loaded = false;
void as_input_loop_tick()
{
	if (as_is_pressed(AS_KEY_ESCAPE))
	{
		glfwSetWindowShouldClose(engine.display_context, true);
	}

	if (as_is_pressed(AS_KEY_LEFT_CONTROL) )
	{
		if (as_is_pressed(AS_KEY_S))
		{
			if (scene_being_saved)
			{
				return;
			}
			scene_being_saved = true;
			as_serialized_scene* serialized_scene = AS_MALLOC_SINGLE(as_serialized_scene);
			as_serialize_scene(engine.scene, serialized_scene);
			AS_SERIALIZE_TO_FILE(as_serialized_scene, serialized_scene, AS_PATH_DEFAULT_SCENE);
			// writing to the file is still being processed and we try to delete, this has to be delayed or something
			// thinking about creating a queue/command system for a lot of async stuff
			//AS_FREE(serialized_scene); 
			as_asset_register(serialized_scene, 0); // currently registering this asset to get deleted automatically on clean up
			return;
		}
		else if (as_is_pressed(AS_KEY_L))
		{
			if (scene_being_loaded)
			{
				return;
			}
			scene_being_loaded = true;
			as_serialized_scene* serialized_scene = AS_DESERIALIZE_FROM_FILE(as_serialized_scene, AS_PATH_DEFAULT_SCENE);
			as_deserialize_scene(engine.scene, serialized_scene, engine.render, engine.render_queue);
			//AS_FREE(serialized_scene); // same issue as above
			as_asset_register(serialized_scene, 0);
			return;
		}
	}

	if (as_is_released(AS_KEY_S))
	{
		scene_being_saved = false;
	}
	if (as_is_released(AS_KEY_L))
	{
		scene_being_loaded = false;
	}

	if (engine.camera && engine.camera->type == AS_CAMERA_FREE)
	{
		const f32 delta_time = as_get_delta_time();
		if (as_is_pressed(AS_KEY_W))
		{
			as_vec3 movement_vector;
			as_vec3_sub(&movement_vector, &engine.camera->target, &engine.camera->position);
			f32 distance_to_target = as_vec3_length(&movement_vector);
			as_vec3_normalize(&movement_vector);

			const f32 threshold_distance = 1.f;
			const f32 max_movement_distance = distance_to_target - threshold_distance;
			if (distance_to_target > threshold_distance)
			{
				as_vec3_mul_scalar(&movement_vector, &movement_vector, delta_time);
				as_vec3_mul_scalar(&movement_vector, &movement_vector, engine.camera->movement_speed);
				const f32 movement_length = as_vec3_length(&movement_vector);
				if (movement_length > max_movement_distance)
				{
					f32 scale = max_movement_distance / movement_length;
					as_vec3_mul_scalar(&movement_vector, &movement_vector, scale);
				}
				as_vec3_add(&engine.camera->position, &engine.camera->position, &movement_vector);
			}
		}
		else if (as_is_pressed(AS_KEY_S))
		{
			as_vec3 movement_vector = {0};
			as_vec3_sub(&movement_vector, &engine.camera->target, &engine.camera->position);
			as_vec3_normalize(&movement_vector);
			as_vec3_mul_scalar(&movement_vector, &movement_vector, delta_time);
			as_vec3_mul_scalar(&movement_vector, &movement_vector, engine.camera->movement_speed);
			as_vec3_sub(&engine.camera->position, &engine.camera->position, &movement_vector);
		}
		if (as_is_pressed(AS_KEY_A))
		{
			as_vec3 movement_vector = {0};
			as_vec3_sub(&movement_vector, &engine.camera->target, &engine.camera->position);
			as_vec3 forward_vec = as_vec3_cross(&engine.camera->up, &movement_vector);
			as_vec3_normalize(&forward_vec);
			as_vec3_mul_scalar(&forward_vec, &forward_vec, delta_time);
			as_vec3_mul_scalar(&forward_vec, &forward_vec, engine.camera->movement_speed);
			as_vec3 new_location = AS_VEC(as_vec3, 0.); 
			as_vec3_sub(&new_location, &engine.camera->position, &forward_vec);
			as_camera_set_position(engine.camera, &new_location);
		}
		else if (as_is_pressed(AS_KEY_D))
		{
			as_vec3 movement_vector = {0};
			as_vec3_sub(&movement_vector, &engine.camera->target, &engine.camera->position);
			as_vec3 forward_vec = as_vec3_cross(&engine.camera->up, &movement_vector);
			as_vec3_normalize(&forward_vec);
			as_vec3_mul_scalar(&forward_vec, &forward_vec, delta_time);
			as_vec3_mul_scalar(&forward_vec, &forward_vec, engine.camera->movement_speed);
			as_vec3 new_location = AS_VEC(as_vec3, 0.); 
			as_vec3_add(&new_location, &engine.camera->position, &forward_vec);
			as_camera_set_position(engine.camera, &new_location);
		}

		const bool pressed_e = as_is_pressed(AS_KEY_E);
		const bool pressed_q = as_is_pressed(AS_KEY_Q);
		if (pressed_e || pressed_q)
		{
			as_vec3 movement_vector = {0};
			as_vec3_mul_scalar(&movement_vector, AS_VEC_PTR(as_vec3, 0.f, 0.f, 1.f), delta_time);
			as_vec3_mul_scalar(&movement_vector, &movement_vector, engine.camera->movement_speed);

			as_vec3 target_vector;
			as_vec3_sub(&target_vector, &engine.camera->target, &engine.camera->position);
			const f32 distance_to_target = as_vec3_length(&target_vector);
			const f32 threshold_distance = 1.0f;

			if (as_is_pressed(AS_KEY_E))
			{
				if (distance_to_target > threshold_distance)
				{
					as_vec3 new_location = AS_VEC(as_vec3, 0.); 
					as_vec3_add(&new_location, &engine.camera->position, &movement_vector);
					as_camera_set_position(engine.camera, &new_location);
				}
			}
			else if (as_is_pressed(AS_KEY_Q))
			{
				if (distance_to_target < (distance_to_target + threshold_distance))
				{
					as_vec3 new_location = AS_VEC(as_vec3, 0.); 
					as_vec3_sub(&new_location, &engine.camera->position, &movement_vector);
					as_camera_set_position(engine.camera, &new_location);
				}
			}
		}
	}
}