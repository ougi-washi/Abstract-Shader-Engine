// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_window.h"
#include "core/as_render.h"
#include "core/as_shader.h"

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

	as_render render = {0};
	as_render_create(&render, display_context);

	as_shader_uniforms_32 uniforms = {0};
	as_texture texture = {0};
	as_texture_create(&render, &texture, "../resources/textures/default_texture.png");
	as_shader_add_uniform_texture(&uniforms, &texture);
	as_shader* shader = as_shader_create(&render, &uniforms, "../resources/shaders/default_vertex.glsl", "../resources/shaders/default_fragment.glsl");
	as_object* object = as_object_create(&render, shader);
	as_objects_1024 objects = {0};
	as_object_add(object, &objects);

	while (!as_display_context_should_close(display_context))
	{
		as_display_context_poll_event();
		as_render_draw_frame(&render, display_context, &objects);
	}
	as_display_context_destroy(display_context);
	as_display_context_terminate();
	return 0;
}
