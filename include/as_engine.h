// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_window.h"
#include "core/as_render.h"
#include "core/as_render_queue.h"
#include "core/as_shader.h"
#include "as_memory.h"

typedef struct as_engine
{
	as_render* render;
	as_render_queue* render_queue;
	as_shader_monitor* monitor;
	void* display_context;
	as_objects_1024* objects;
} as_engine;

extern void as_engine_init();
extern void as_engine_clear();
extern bool as_engine_should_loop();
extern void as_engine_draw(); // generally at the end of the engine loop

extern f64 as_get_time();
extern f64 as_get_delta_time();

extern as_texture* as_texture_create(const char* texture_path);
extern as_shader* as_shader_create(const char* vertex_shader_path, const char* fragment_shader_path);
extern as_object* as_object_create(as_shader* shader);

extern sz as_assign_texture_to_shader(as_shader* shader, as_texture* texture);
