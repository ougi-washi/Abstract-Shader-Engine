// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#include "core/as_render.h"
#include "core/as_render_queue.h"

typedef struct as_shader_monitor_thread
{
	bool is_running;
	void* shader;
	u64* frame_count;
	as_thread thread;
	char file_path[AS_MAX_PATH_SIZE];
	void (*shader_update_func)(as_render_queue*, void*);
	as_file_pool* file_pool;
	as_render_queue* render_queue;
	AS_DECLARE_TYPE;
} as_shader_monitor_thread;

AS_ARRAY_DECLARE(as_shader_monitor_threads_256, 256, as_shader_monitor_thread);

typedef struct as_shader_monitor
{
	as_shader_monitor_threads_256 threads;

	as_mutex mutex;
	bool is_running;
	u64* frame_count;
	as_render_queue* render_queue; // optional
	AS_DECLARE_TYPE;
} as_shader_monitor;

extern as_shader_monitor* as_shader_monitor_create(u64* frame_count, as_render_queue* render_queue);
extern void as_shader_monitored_destroy(as_shader_monitor* monitor);
extern void as_shader_monitor_add(u64* frame_counter, as_shader_monitor* monitor, void* shader, const char* file_to_check, void shader_update_func(as_render_queue*, void*));
extern as_shader_monitor_thread* as_shader_monitor_find_thread(as_shader_monitor* monitor, const char* filename_vertex, const char* filename_fragment);
extern as_shader* as_shader_monitor_find_shader(as_shader_monitor* monitor, const char* filename_vertex, const char* filename_fragment);
