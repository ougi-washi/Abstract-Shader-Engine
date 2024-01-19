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

	as_shader_uniforms_32* uniforms = as_uniforms_create();
	as_texture* texture = as_texture_create(render, "../resources/textures/default_texture.png");
	as_shader_add_uniform_texture(uniforms, texture);
	as_shader* shader = as_shader_create(render, uniforms, "../resources/shaders/default_vertex.glsl", "../resources/shaders/default_fragment.glsl");
	as_object* object = as_object_create(render, shader);
	as_object_set_translation(object, &(as_vec3) {0., 0., 2.5});
	as_objects_1024* objects = as_objects_create();
	as_object_add(object, objects);

	while (!as_display_context_should_close(display_context))
	{
		as_display_context_poll_event();
		as_render_draw_frame(render, display_context, objects);
	}
	as_display_context_destroy(display_context);
	as_display_context_terminate();
	as_objects_destroy(render, objects);
	as_render_destroy(render);
	AS_LOG_MEMORY();
	return 0;
}
