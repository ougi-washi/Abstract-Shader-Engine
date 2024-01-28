// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_window.h"
#include "core/as_render.h"
#include "core/as_shader.h"
#include "as_memory.h"


void key_callback(void* window, const i32 key, const i32 scancode, const i32 action, const i32 mods)
{
	if (key == AS_KEY_ESCAPE && action == AS_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

i32 main() 
{
	void* display_context = as_display_context_create(1200, 720, "Abstract Shader Engine", &key_callback);
	AS_ASSERT(display_context, "invalid display context");

	as_render* render = as_render_create(display_context);
	as_shader_monitor* monitor = as_shader_monitor_create(&render->frame_counter);

	as_shader_uniforms_32* uniforms = as_uniforms_create();
	as_texture* texture = as_texture_create(render, "../resources/textures/default_texture.png");
	as_shader_add_uniform_texture(uniforms, texture);
	as_shader* shader = as_shader_create(render, uniforms, "../resources/shaders/default_vertex.glsl", "../resources/shaders/default_fragment.glsl");
	as_shader_monitor_add(render, monitor, shader);
	as_object* object1 = as_object_create(render, shader);
	as_object* object2 = as_object_create(render, shader);
	as_object_set_translation(object1, AS_VEC_PTR(as_vec3, 0., 0., 0.));
	as_object_set_translation(object2, AS_VEC_PTR(as_vec3, 0., 0., 2.));
	as_objects_1024* objects = as_objects_create();
	as_object_add(object1, objects);
	as_object_add(object2, objects);
	//as_mat4_translate(&object2->transform, AS_VEC_PTR(as_vec3, 0., -3., -3.));
	u32 instance_count = 1;
	while (!as_display_context_should_close(display_context))
	{
		as_render_start_draw_loop(render);
		as_display_context_poll_event();

		const f32 delta_time = as_render_get_delta_time(render);
		const f32 current_time = as_render_get_time(render);
		as_object_rotate(object1, 3. * delta_time, AS_VEC3_X_AXIS_PTR);
		//as_object_rotate_around_pivot(object2, 4.5 * delta_time, AS_VEC3_Z_AXIS_PTR, AS_VEC_PTR(as_vec3, 5., 5., 8.));
		//as_vec3* translate_value = AS_VEC_PTR(as_vec3, cos(current_time * 0.01), sin(current_time * 0.01), 0.f);
		as_vec3 translate_value = { 0 };
		translate_value.x = cos(as_radians(current_time * 5.) * 10.f) * 5.f;
		translate_value.y = sin(as_radians(current_time * 5.) * 10.f) * 5.f;
		translate_value.z = 0.;
		if (instance_count < 125)
		{
			as_object_set_instance_count(object2, ++instance_count);
		}
		as_mat4_set_translation(&object2->transform, &translate_value);
		as_vec3 translation = as_mat4_get_translation(&object2->transform);
		as_quat rotation = as_mat4_get_rotation(&object2->transform);
		as_vec3 scale = as_mat4_get_scale(&object2->transform);
		as_render_draw_frame(render, display_context, objects);
		as_render_end_draw_loop(render);
	}
	as_display_context_destroy(display_context);
	as_display_context_terminate();
	as_shader_monitored_destroy(monitor);
	as_objects_destroy(render, objects);
	as_render_destroy(render);
	AS_LOG_MEMORY();
	return 0;
}
