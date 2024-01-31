// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_engine.h"
#include "core/as_shader_monitor.h"
#include "core/as_render_queue.h"

typedef struct as_engine
{
	as_render* render;
	as_render_queue* render_queue;
	as_shader_monitor* shader_monitor;
	void* display_context;
	as_camera* camera;
	as_objects_1024* objects;
} as_engine;

static as_engine engine = { 0 };
// static as_input* input = NULL; 

void key_callback(void* window, const i32 key, const i32 scancode, const i32 action, const i32 mods)
{
	// TODO: Move to input handle

	if (key == AS_KEY_ESCAPE && action == AS_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	// TODO: Move to camera updater
	if (engine.camera && engine.camera->type == AS_CAMERA_FREE)
	{
		const f32 deltaTime = as_get_delta_time(); 
		if (action == AS_PRESS || action == AS_REPEAT)
		{
			switch (key)
			{
			case AS_KEY_W:
			{
				as_vec3 movement_vector = { 0 };
				as_vec3_sub(&movement_vector, &engine.camera->target, &engine.camera->position);
				as_vec3_normalize(&movement_vector);
				as_vec3_mul_scalar(&movement_vector, &movement_vector, deltaTime);
				as_vec3_mul_scalar(&movement_vector, &movement_vector, engine.camera->movement_speed);
				as_vec3_add(&engine.camera->position, &engine.camera->position, &movement_vector);
				break;
			}
			case AS_KEY_S:
			{
				as_vec3 movement_vector = { 0 };
				as_vec3_sub(&movement_vector, &engine.camera->target, &engine.camera->position);
				as_vec3_normalize(&movement_vector);
				as_vec3_mul_scalar(&movement_vector, &movement_vector, deltaTime);
				as_vec3_mul_scalar(&movement_vector, &movement_vector, engine.camera->movement_speed);
				as_vec3_sub(&engine.camera->position, &engine.camera->position, &movement_vector);
				break;
			}
			case AS_KEY_A:
			{
				as_vec3 movement_vector = { 0 };
				as_vec3_sub(&movement_vector, &engine.camera->target, &engine.camera->position);
				as_vec3 forward_vec = as_vec3_cross(&engine.camera->up, &movement_vector);
				as_vec3_normalize(&forward_vec);
				as_vec3_mul_scalar(&forward_vec, &forward_vec, deltaTime);
				as_vec3_mul_scalar(&forward_vec, &forward_vec, engine.camera->movement_speed);
				as_vec3_sub(&engine.camera->position, &engine.camera->position, &forward_vec);
				break;
			}
			case AS_KEY_D:
			{
				as_vec3 movement_vector = { 0 };
				as_vec3_sub(&movement_vector, &engine.camera->target, &engine.camera->position);
				as_vec3 forward_vec = as_vec3_cross(&engine.camera->up, &movement_vector);
				as_vec3_normalize(&forward_vec);
				as_vec3_mul_scalar(&forward_vec, &forward_vec, deltaTime);
				as_vec3_mul_scalar(&forward_vec, &forward_vec, engine.camera->movement_speed);
				as_vec3_add(&engine.camera->position, &engine.camera->position, &forward_vec);
				break;
			}
			}
		}
	}
}

void as_engine_init()
{
	AS_LOG(LV_LOG, "Initializing the engine");
	engine.display_context = as_display_context_create(AS_ENGINE_WINDOW_WIDTH, AS_ENGINE_WINDOW_HEIGHT, AS_ENGINE_WINDOW_NAME, &key_callback);
	engine.render = as_render_create(engine.display_context);
	engine.shader_monitor = as_shader_monitor_create(&engine.render->frame_counter, &as_rq_shader_recompile, engine.render_queue);
	engine.objects = as_objects_create();
	engine.render_queue = as_rq_create();

	while (AS_IS_INVALID(engine.render)) {};
}

void as_engine_clear()
{
	AS_LOG(LV_LOG, "Clearing the engine");

	as_shader_monitored_destroy(engine.shader_monitor);
	as_rq_destroy(engine.render_queue);

	as_objects_destroy(engine.render, engine.objects);
	as_render_destroy(engine.render);

	as_display_context_destroy(engine.display_context);
	as_display_context_terminate();

	AS_LOG_MEMORY();
}

bool as_engine_should_loop()
{
	bool should_loop = !as_display_context_should_close(engine.display_context);
	as_display_context_poll_event();
	as_rq_render_start_draw_loop(engine.render_queue, engine.render);
	return should_loop;
}

void as_engine_draw()
{
	as_rq_render_draw_frame(engine.render_queue, engine.render, engine.display_context, engine.camera, engine.objects);
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
	as_object* object = as_object_make(engine.render, shader);
	as_object_add(object, engine.objects);
	return object;
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
