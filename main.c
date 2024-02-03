// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_engine.h"

i32 main()
{
	as_engine_init();

	as_camera* camera = as_camera_create(AS_VEC_PTR(as_vec3, 10.f, 10.f, 10.f), AS_VEC_PTR(as_vec3, 0.f, 0.f, 0.f));
	as_camera_set_view(camera, AS_CAMERA_FREE);

	as_texture* texture = as_texture_create(AS_PATH_DEFAULT_TEXTURE);
	as_shader* shader = as_shader_create(AS_PATH_DEFAULT_VERT_SHADER, AS_PATH_DEFAULT_FRAG_SHADER);
	
	as_assign_texture_to_shader(shader, texture);
	as_object* object1 = as_object_create(shader);
	as_object* object2 = as_object_create(shader);
	as_object_set_instance_count(object2, 10000000);
	as_object_set_translation(object1, AS_VEC_PTR(as_vec3, 0., 0., 0.));
	as_object_set_translation(object2, AS_VEC_PTR(as_vec3, 0., 0., -2000.));

	while (as_engine_should_loop())
	{
		const f64 delta_time = as_get_delta_time();
		const f64 current_time = as_get_time();

		// rotate object 1
		as_object_rotate(object1, 3. * delta_time, AS_VEC3_X_AXIS_PTR);

		// move around object 2
		f32 x = cos(as_radians(current_time * 1.) * 10.f) * 2.f;
		f32 y = sin(as_radians(current_time * 1.) * 10.f) * 2.f;
		as_mat4_set_translation(&object2->transform, AS_VEC_PTR(as_vec3, x, y));

		as_engine_draw();
	}

	as_engine_clear();
}
