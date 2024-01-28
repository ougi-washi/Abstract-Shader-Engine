// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"
#include "as_threads.h"

#define AS_RENDER_QUEUE_WAIT_TIME 0.0001 // wait 1ms when there is nothing to process

typedef struct as_render_command
{
	void* (*func_ptr)(void*);
	void* arg;
} as_render_command;
AS_DECLARE_ARRAY(as_render_commands_1024, 1024, as_render_command);

typedef struct as_render_queue
{
	bool is_running;
	as_thread thread;
	as_render_commands_1024 commands;

	ADD_FLAG;
} as_render_queue;

extern as_render_queue* as_render_queue_create();
extern void as_render_queue_destroy(as_render_queue* render_queue);
extern void as_render_queue_submit(as_render_queue* render_queue, void* func_ptr(void*), void* arg);
