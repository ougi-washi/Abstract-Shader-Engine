// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_editor.h"
#include "core/as_console.h"

void as_rotate_object1(as_object* object, const f64 delta_time)
{
	as_object_set_translation(object, AS_VEC_PTR(as_vec3, 0., -cos(as_get_time() * .5) * 2., 0.));
}
void as_rotate_object2(as_object* object, const f64 delta_time)
{
	as_object_set_translation(object, AS_VEC_PTR(as_vec3, 0., sin(as_get_time() * .48) * 2.));
}

void as_editor_set_default_scene()
{    
	as_camera* camera = as_camera_create(AS_VEC_PTR(as_vec3, -15.f, -15.f, 10.f), AS_VEC_PTR(as_vec3, 0.f, 0.f, 0.f));
	as_camera_set_view(camera, AS_CAMERA_FREE);

	//as_texture* texture = as_texture_create(AS_PATH_DEFAULT_TEXTURE);
	as_shader* shader = as_shader_create(AS_PATH_DEFAULT_VERT_SHADER, AS_PATH_DEFAULT_FRAG_SHADER);
	
	//as_assign_texture_to_shader(shader, texture);
	as_shape* shape_sphere = as_generate_sphere(.6, 8, 8);

	as_object* object1 = as_object_create_with_tick(shape_sphere, shader, &as_rotate_object1);
	as_object_set_translation(object1, AS_VEC_PTR(as_vec3, 0.5, 0., 0.));

	as_object* object2 = as_object_create_with_tick(shape_sphere, shader, &as_rotate_object2);
	as_object_set_translation(object2, AS_VEC_PTR(as_vec3, 0., 0., 0.));
	as_object_set_instance_count(object2, 1);
	as_destroy_shape(shape_sphere);
}

bool as_editor_should_loop()
{
	return as_engine_should_loop();
}