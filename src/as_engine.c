// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_engine.h"
#include "core/as_shader_monitor.h"
#include "core/as_render_queue.h"
#include "core/as_input.h"
#include "core/as_tick.h"

typedef struct as_engine
{
	as_render* render;
	as_render_queue* render_queue;
	as_shader_monitor* shader_monitor;
	void* display_context;
	as_camera* camera;
	as_scene* scene;
	as_input_buffer* input_buffer;
	as_tick_system* tick_system;
} as_engine;

static as_engine engine = {0};

void key_callback(void* window, const i32 key, const i32 scancode, const i32 action, const i32 mods)
{
	as_input_add(engine.input_buffer, key, action);
}

void as_engine_init()
{
	AS_LOG(LV_LOG, "Initializing the engine");
	engine.display_context = as_display_context_create(AS_ENGINE_WINDOW_WIDTH, AS_ENGINE_WINDOW_HEIGHT, AS_ENGINE_WINDOW_NAME, &key_callback);
	engine.render = as_render_create(engine.display_context);
	engine.shader_monitor = as_shader_monitor_create(&engine.render->frame_counter, &as_rq_shader_recompile, engine.render_queue);
	engine.scene = as_scene_create(AS_PATH_DEFAULT_SCENE);
	engine.render_queue = as_rq_create();
	engine.input_buffer = as_input_create();
	engine.tick_system = as_tick_system_create();
	while (AS_IS_INVALID(engine.render)) {};
}

void as_engine_clear()
{
	AS_LOG(LV_LOG, "Clearing the engine");

	as_input_destory(engine.input_buffer);

	as_shader_monitored_destroy(engine.shader_monitor);
	as_rq_destroy(engine.render_queue);

	as_tick_system_destroy(engine.tick_system);
	as_scene_destroy(engine.render, engine.scene);
	as_render_destroy(engine.render);

	as_display_context_destroy(engine.display_context);
	as_display_context_terminate();

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
	as_rq_render_draw_frame(engine.render_queue, engine.render, engine.display_context, engine.camera, engine.scene);
	as_rq_wait_queue(engine.render_queue);
	as_render_end_draw_loop(engine.render);
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
	as_texture* texture = as_texture_make(texture_path);
	as_texture_update(engine.render, texture);
	return texture;
}

as_shader* as_shader_create(const char* vertex_shader_path, const char* fragment_shader_path)
{
	as_shader* shader = as_shader_make(engine.render, vertex_shader_path, fragment_shader_path);
	as_shader_monitor_add(&engine.render->frame_counter, engine.shader_monitor, shader);
	return shader;
}

as_object* as_object_create(as_shader* shader)
{
	AS_ASSERT(shader, "Trying create object, but shader is NULL");
	as_object* object = as_object_make(engine.render, engine.scene, shader);
	return object;
}

as_object* as_object_create_with_tick(as_shader* shader, void tick_func_ptr(as_object*, const f64))
{
	AS_ASSERT(tick_func_ptr, "Cannot create ticking object, invalid function ptr");
	as_object* object = as_object_create(shader);
	as_tick_handle* handle = as_tick_handle_create(engine.tick_system);
	handle->func_ptr = &tick_func_ptr;
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

void as_input_loop_tick()
{
	if (as_is_pressed(AS_KEY_ESCAPE))
	{
		glfwSetWindowShouldClose(engine.display_context, true);
	}

	if (as_is_pressed(AS_KEY_LEFT_CONTROL) && as_is_pressed(AS_KEY_S))
	{
		AS_SERIALIZE(as_scene, engine.scene, AS_PATH_DEFAULT_SCENE);
		return;
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
			as_vec3_sub(&engine.camera->position, &engine.camera->position, &forward_vec);
		}
		else if (as_is_pressed(AS_KEY_D))
		{
			as_vec3 movement_vector = {0};
			as_vec3_sub(&movement_vector, &engine.camera->target, &engine.camera->position);
			as_vec3 forward_vec = as_vec3_cross(&engine.camera->up, &movement_vector);
			as_vec3_normalize(&forward_vec);
			as_vec3_mul_scalar(&forward_vec, &forward_vec, delta_time);
			as_vec3_mul_scalar(&forward_vec, &forward_vec, engine.camera->movement_speed);
			as_vec3_add(&engine.camera->position, &engine.camera->position, &forward_vec);
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
					as_vec3_add(&engine.camera->position, &engine.camera->position, &movement_vector);
				}
			}
			else if (as_is_pressed(AS_KEY_Q))
			{
				if (distance_to_target < (distance_to_target + threshold_distance))
				{
					as_vec3_sub(&engine.camera->position, &engine.camera->position, &movement_vector);
				}
			}
		}
	}
}