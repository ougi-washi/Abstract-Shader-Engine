// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_shader_monitor.h"
#include "as_memory.h"
#include <math.h>

void* as_shader_monitor_thread_run(as_shader_monitor_thread* thread_data)
{
	AS_ASSERT(thread_data, "cannot execute as_shader_monitor_thread_run, params nullptr");
	AS_ASSERT(thread_data->frame_count, "cannot execute as_shader_monitor_thread_run, frame count invalid");
	AS_ASSERT(thread_data->shader_update_func, "cannot execute as_shader_monitor_thread_run, shader update function invalid");
	AS_ASSERT(thread_data->file_pool, "cannot execute as_shader_monitor_thread_run, file pool invalid");
	while (thread_data->is_running)
	{
		if (AS_IS_INVALID(thread_data)) { continue; }
		AS_WAIT_AND_LOCK(thread_data);
		const u64 frame_count = *thread_data->frame_count;
		as_flagged_struct* shader = thread_data->shader;
		as_render_queue* render_queue = thread_data->render_queue;
		if (shader && AS_IS_UNLOCKED(shader))
		{
			if (as_shader_has_changed(thread_data->shader_binary_pool, thread_data->file_pool, thread_data->file_path))
			{
				AS_WAIT_AND_LOCK(shader);
				thread_data->shader_update_func(render_queue, shader);
				AS_UNLOCK(shader);
			}
			sleep_seconds(1/100.);
		}
		AS_UNLOCK(thread_data);
	}
	return NULL;
}

as_shader_monitor* as_shader_monitor_create(u64* frame_count, as_render_queue* render_queue)
{
	AS_ASSERT(frame_count, "Cannot create shader monitor, frame_count is null");

	as_shader_monitor* monitor = AS_MALLOC_SINGLE(as_shader_monitor);
	monitor->is_running = true;
	monitor->frame_count = frame_count;
	monitor->render_queue = render_queue;
	as_mutex_init(&monitor->mutex);
	AS_SET_VALID(monitor);
	return monitor;
}

void as_shader_monitored_destroy(as_shader_monitor* monitor)
{
	if (!monitor || AS_IS_INVALID(monitor)) { return; }
	as_mutex_destroy(&monitor->mutex);
	for (sz i = 0; i < AS_STATIC_ARRAY_SIZE(monitor->threads); i++)
	{
		as_shader_monitor_thread* monitor_thread = AS_STATIC_ARRAY_GET(monitor->threads, i);
		if (AS_STATIC_ARRAY_IS_VALID(monitor->threads, i))
		{
			AS_STATIC_ARRAY_REMOVE(monitor->threads, i);
			AS_SET_INVALID(monitor_thread);
			monitor_thread->is_running = false;
			as_thread_join(monitor_thread->thread);
			AS_FREE(monitor_thread->file_pool);
			AS_FREE(monitor_thread->shader_binary_pool);
		}
	}
	//AS_ARRAY_CLEAR(monitor->threads);
	AS_SET_INVALID(monitor);
	AS_FREE(monitor);
}
void as_shader_monitor_add(u64* frame_counter, as_shader_monitor* monitor, void* shader, const char* file_to_check, void shader_update_func(as_render_queue*, void*))
{
	AS_ASSERT(monitor, "cannot add shader to monitor, invalid monitor");
	AS_ASSERT(shader, "cannot add shader to monitor, invalid shader");
	AS_ASSERT(frame_counter, "cannot add shader to monitor, invalid frame_counter");
	AS_ASSERT(shader_update_func, "Cannot add shader to monitor, shader_update_func is null");

	sz thread_index = -1;
	AS_STATIC_ARRAY_ADD(monitor->threads, thread_index);
	as_shader_monitor_thread* thread = AS_STATIC_ARRAY_GET(monitor->threads, thread_index);
	AS_WARNING_RETURN_IF_FALSE(thread, "Could not make a new shader monitor thread, could be a size issue");

	thread->file_pool = AS_MALLOC_SINGLE(as_file_pool);
	thread->shader_binary_pool = AS_MALLOC_SINGLE(as_shader_binary_pool);
	thread->is_running = true;
	thread->frame_count = frame_counter;
	thread->shader_update_func = shader_update_func;
	thread->render_queue = monitor->render_queue;
	thread->shader = shader;
	strcpy(thread->file_path, file_to_check);
	thread->thread = as_thread_create(as_shader_monitor_thread_run, thread);
	AS_SET_VALID(thread);
	AS_FLOG(LV_LOG, "Created shader monitor thread %p", thread);
}
//
//as_shader_monitor_thread* as_shader_monitor_find_thread(as_shader_monitor* monitor, const char* filename_vertex, const char* filename_fragment)
//{
//	for (sz i = 0; i < AS_STATIC_ARRAY_SIZE(monitor->threads); i++)
//	{
//		if (!AS_STATIC_ARRAY_IS_VALID(monitor->threads))
//		{
//			continue;
//		}
//		as_shader_monitor_thread* thread_found = AS_STATIC_ARRAY_GET(monitor->threads, i);
//		if (!thread_found)
//		{
//			continue;
//		}
//		as_shader* shader_found = thread_found->shader;
//		if (!shader_found)
//		{
//			continue;
//		}
//		if (strcmp(shader_found->filename_fragment, filename_fragment) == 0 &&
//			strcmp(shader_found->filename_vertex, filename_vertex) == 0)
//		{
//			return thread_found;
//		}
//	}
//	return NULL;
//}

as_shader* as_shader_monitor_find_shader(as_shader_monitor* monitor, const char* filename_vertex, const char* filename_fragment)
{
	for (sz i = 0; i < AS_STATIC_ARRAY_SIZE(monitor->threads); i++)
	{
		if (!AS_STATIC_ARRAY_IS_VALID(monitor->threads, i))
		{
			continue;
		}
		as_shader_monitor_thread* thread_found = AS_STATIC_ARRAY_GET(monitor->threads, i);
		if (!thread_found)
		{
			continue;
		}
		as_shader* shader_found = thread_found->shader;
		if (!shader_found)
		{
			continue;
		}
		if (strcmp(shader_found->filename_fragment, filename_fragment) == 0 &&
			strcmp(shader_found->filename_vertex, filename_vertex) == 0)
		{
			return shader_found;
		}
	}
	return NULL;
}
