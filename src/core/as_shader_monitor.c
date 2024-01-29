// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_shader_monitor.h"
#include "as_memory.h"

void* as_shader_monitor_thread_run(as_shader_monitor_thread* thread_data)
{
	AS_ASSERT(thread_data, "cannot execute as_shader_monitor_thread_run, params nullptr");
	AS_ASSERT(thread_data->frame_count, "cannot execute as_shader_monitor_thread_run, frame count invalid");
	AS_ASSERT(thread_data->shader_update_func, "cannot execute as_shader_monitor_thread_run, shader update function invalid");
	while (thread_data->is_running)
	{
		const u64 frame_count = *thread_data->frame_count;
		as_shader* shader = thread_data->shader;
		if (shader && as_shader_is_unlocked(frame_count, shader))
		{
			if (as_shader_has_changed(shader->filename_fragment) || as_shader_has_changed(shader->filename_vertex))
			{
				as_shader_set_locked(frame_count, shader);
				thread_data->shader_update_func(shader);
				as_shader_set_unlocked(shader);
			}
		}
		sleep_seconds(.1f); // wait time 100ms
	}
	return NULL;
}

as_shader_monitor* as_shader_monitor_create(u64* frame_count, void shader_update_func(as_shader*))
{
	AS_ASSERT(frame_count, "Cannot create shader monitor, frame_count is null");
	AS_ASSERT(shader_update_func, "Cannot create shader monitor, shader_update_func is null");

	as_shader_monitor* monitor = AS_MALLOC_SINGLE(as_shader_monitor);
	monitor->is_running = true;
	monitor->frame_count = frame_count;
	monitor->shader_update_func = shader_update_func;
	as_mutex_init(&monitor->mutex);
	AS_SET_VALID(monitor);
	return monitor;
}

void as_shader_monitored_destroy(as_shader_monitor* monitor)
{
	if (!monitor || AS_IS_INVALID(monitor)) { return; }
	as_mutex_destroy(&monitor->mutex);
	for (sz i = 0; i < monitor->threads.size; i++)
	{
		monitor->threads.data[i].is_running = false;
		as_thread_join(monitor->threads.data[i].thread);
	}
	AS_CLEAR_ARRAY(monitor->threads);
	AS_SET_INVALID(monitor);
	AS_FREE(monitor);
}

void as_shader_monitor_add(u64* frame_counter, as_shader_monitor* monitor, as_shader* shader)
{
	AS_ASSERT(monitor, "cannot add shader to monitor, invalid monitor");
	AS_ASSERT(shader, "cannot add shader to monitor, invalid shader");
	AS_ASSERT(frame_counter, "cannot add shader to monitor, invalid frame_counter");

	as_shader_monitor_thread thread_data /*= AS_MALLOC_SINGLE(as_shader_monitor_thread)*/ = { 0 };
	AS_PUSH_BACK_ARRAY(monitor->threads, thread_data);
	as_shader_monitor_thread* thread = AS_ARRAY_GET(monitor->threads, monitor->threads.size - 1);
	AS_ASSERT(thread, "Could not add valid thread data to the monitor threads");
	thread->is_running = true;
	thread->frame_count = frame_counter;
	thread->shader_update_func = monitor->shader_update_func;
	thread->shader = shader;
	thread->thread = as_thread_create(as_shader_monitor_thread_run, thread);
}
