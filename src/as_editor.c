// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_editor.h"

void as_rotate_object1(as_object* object, const f64 delta_time)
{
	as_object_set_translation(object, AS_VEC_PTR(as_vec3, sin(as_get_time()) * 3.5, cos(as_get_time()) * 5., 3.));
}
void as_rotate_object2(as_object* object, const f64 delta_time)
{
	as_object_set_translation(object, AS_VEC_PTR(as_vec3, cos(as_get_time()) * 1.5, sin(as_get_time()) * 10.));
}

void as_editor_set_default_scene()
{    
	as_camera* camera = as_camera_create(AS_VEC_PTR(as_vec3, -15.f, -15.f, 10.f), AS_VEC_PTR(as_vec3, 0.f, 0.f, 0.f));
	as_camera_set_view(camera, AS_CAMERA_FREE);

	as_texture* texture = as_texture_create(AS_PATH_DEFAULT_TEXTURE);
	as_shader* shader = as_shader_create(AS_PATH_DEFAULT_VERT_SHADER, AS_PATH_DEFAULT_FRAG_SHADER);
	
	as_assign_texture_to_shader(shader, texture);
	// as_shape shape_sphere = as_generate_sphere(.2f, 4, 4);
	// as_shape shape_quad = as_generate_quad();
	as_shape shape_cube = as_generate_cube();
	
	as_object* object1 = as_object_create_with_tick(&shape_cube, shader, &as_rotate_object1);
	as_object* object2 = as_object_create(&shape_cube, shader);
	as_object_set_translation(object2, AS_VEC_PTR(as_vec3, -40., -40., 1.));
	as_object_set_instance_count(object2, 10000);
}
