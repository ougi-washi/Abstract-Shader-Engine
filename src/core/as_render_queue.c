// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_render_queue.h"
#include "as_memory.h"

void* as_render_queue_thread_run(as_render_queue* queue)
{
	if (AS_IS_INVALID(queue)) { return NULL; }

	while (queue->is_running)
	{
		const sz queue_size = queue->commands.size;
		if (queue_size > 0)
		{
			as_render_command* command = &queue->commands.data[queue_size - 1];
			if (command)
			{
				command->func_ptr(command->arg);
			}
			AS_REMOVE_AT_ARRAY(queue->commands, queue_size - 1);
		}
		else
		{
			sleep_seconds(AS_RENDER_QUEUE_WAIT_TIME);
		}
	}
	return NULL;
}

as_render_queue* as_render_queue_create()
{
	as_render_queue* queue = AS_MALLOC_SINGLE(as_render_queue);
	AS_SET_VALID(queue);
	queue->is_running = true;
	queue->thread = as_thread_create(&as_render_queue_thread_run, queue);
	return queue;
}

void as_render_queue_destroy(as_render_queue* render_queue)
{
	if (AS_IS_INVALID(render_queue)) { return; }
	render_queue->is_running = false;
	as_thread_join(render_queue->thread);
	AS_FREE(render_queue);
}

void as_render_queue_submit(as_render_queue* render_queue, void* func_ptr(void*), void* arg)
{
	AS_ASSERT(render_queue, "Cannot submit to renderer queue, render_queue is null");
	AS_ASSERT(func_ptr, "Cannot submit to renderer queue, func_ptr is null");

	as_render_command command = { 0 };
	command.func_ptr = func_ptr;
	command.arg = arg;
	AS_INSERT_AT_ARRAY(render_queue->commands, 0, command);
}
