// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_engine.h"
#include "core/as_shader_monitor.h"
#include "core/as_render_queue.h"

typedef struct as_engine
{
	as_render* render;
	as_render_queue* render_queue;
	as_shader_monitor* monitor;
	void* display_context;
	as_objects_1024* objects;
} as_engine;

static as_engine engine = { 0 };
// static as_input* input = NULL; 

void key_callback(void* window, const i32 key, const i32 scancode, const i32 action, const i32 mods)
{
	if (key == AS_KEY_ESCAPE && action == AS_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void as_engine_init()
{
	AS_LOG(LV_LOG, "Initializing the engine");
	engine.display_context = as_display_context_create(AS_ENGINE_WINDOW_WIDTH, AS_ENGINE_WINDOW_HEIGHT, AS_ENGINE_WINDOW_NAME, &key_callback);
	engine.render = as_render_create(engine.display_context);
	engine.monitor = as_shader_monitor_create(&engine.render->frame_counter);
	engine.render_queue = as_rq_create();
	engine.objects = as_objects_create();
}

void as_engine_clear()
{
	AS_LOG(LV_LOG, "Clearing the engine");
	as_display_context_destroy(engine.display_context);
	as_display_context_terminate();
	as_shader_monitored_destroy(engine.monitor);
	as_objects_destroy(engine.render, engine.objects);
	as_render_destroy(engine.render);
	as_rq_destroy(engine.render_queue);
	AS_LOG_MEMORY();
}

bool as_engine_should_loop()
{
	bool should_loop = !as_display_context_should_close(engine.display_context);
	as_render_start_draw_loop(engine.render);
	as_display_context_poll_event();
	return should_loop;
}

void as_engine_draw()
{
	as_render_draw_frame(engine.render, engine.display_context, engine.objects);
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
	as_shader_monitor_add(&engine.render->frame_counter, engine.monitor, shader);
	return shader;
}


as_object* as_object_create(as_shader* shader)
{
	AS_ASSERT(shader, "Trying create object, but object is NULL");
	as_object* object = as_object_make(engine.render, shader);
	as_object_add(object, engine.objects);
	return object;
}

sz as_assign_texture_to_shader(as_shader* shader, as_texture* texture)
{
	sz index = as_shader_add_uniform_texture(&shader->uniforms, texture);
	as_shader_update(engine.render, shader);
	return index;
}
