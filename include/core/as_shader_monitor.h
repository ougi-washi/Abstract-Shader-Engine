// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#include "core/as_render.h"

typedef struct as_shader_monitor_thread
{
	bool is_running;
	as_shader* shader;
	u64* frame_count;
	as_thread thread;
} as_shader_monitor_thread;

AS_DECLARE_ARRAY(as_shader_monitor_threads_256, 256, as_shader_monitor_thread);

typedef struct as_shader_monitor
{
	as_shader_monitor_threads_256 threads;

	as_mutex mutex;
	bool is_running;
	u64* frame_count;
	ADD_FLAG;
} as_shader_monitor;

extern as_shader_monitor* as_shader_monitor_create(u64* frame_count);
extern void as_shader_monitored_destroy(as_shader_monitor* monitor);
extern void as_shader_monitor_add(u64* frame_counter, as_shader_monitor* monitor, as_shader* shader);
