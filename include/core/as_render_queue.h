// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"
#include "as_threads.h"
#include "core/as_render.h"

#define AS_RENDER_QUEUE_WAIT_TIME 1./100000. 
#define AS_RENDER_QUEUE_REST_TIME 1./1000000. // wait 1ms when there is nothing to process
#define AS_RENDER_QUEUE_SIZE 1024
#define AS_RENDER_QUEUE_MAX_ARG_SIZE 512

typedef struct as_render_command
{
	void (*func_ptr)(void*);
	void* arg[AS_RENDER_QUEUE_MAX_ARG_SIZE];
} as_render_command;
AS_DECLARE_ARRAY(as_render_commands_1024, AS_RENDER_QUEUE_SIZE, as_render_command);

typedef struct as_render_queue
{
	bool is_running;
	as_thread thread;
	as_render_commands_1024 commands;

	ADD_FLAG;
} as_render_queue;

extern as_render_queue* as_rq_create();
extern void as_rq_destroy(as_render_queue* render_queue);
extern sz as_rq_get_queue_size(as_render_queue* render_queue);
extern void as_rq_wait_queue(as_render_queue* render_queue);
extern void as_rq_submit(as_render_queue* render_queue, void func_ptr(void*), void* arg, const u64 arg_size);

extern void as_rq_render_start_draw_loop(as_render_queue* render_queue, as_render* render);
extern void as_rq_render_end_draw_loop(as_render_queue* render_queue, as_render* render);
extern void as_rq_render_draw_frame(as_render_queue* render_queue, as_render* render, void* display_context, as_camera* camera, as_objects_1024* objects);
extern void as_rq_render_destroy(as_render_queue* render_queue, as_render* render);

extern void as_rq_texture_update(as_render_queue* render_queue, as_texture* texture, as_render* render);
extern void as_rq_texture_destroy(as_render_queue* render_queue, as_render* render, as_texture* texture);

extern void as_rq_shader_set_uniforms(as_render_queue* render_queue, as_render* render, as_shader* shader, as_shader_uniforms_32* uniforms);
extern void as_rq_shader_update(as_render_queue* render_queue, as_render* render, as_shader* shader);
extern void as_rq_shader_recompile(as_render_queue* render_queue, as_shader* shader);
extern void as_rq_shader_destroy(as_render_queue* render_queue, as_render* render, as_shader* shader);

extern void as_rq_objects_destroy(as_render_queue* render_queue, as_render* render, as_objects_1024* objects);
