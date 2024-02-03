// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_editor.h"

void as_rotate_object(as_object* object, const f64 delta_time)
{
	as_object_set_translation(object, AS_VEC_PTR(as_vec3, sin(as_get_time()) * 3.5, cos(as_get_time()) * 5.));
}

void as_editor_set_default_scene()
{    
	as_camera* camera = as_camera_create(AS_VEC_PTR(as_vec3, -15.f, -15.f, 10.f), AS_VEC_PTR(as_vec3, 0.f, 0.f, 0.f));
	as_camera_set_view(camera, AS_CAMERA_FREE);

	as_texture* texture = as_texture_create(AS_PATH_DEFAULT_TEXTURE);
	as_shader* shader = as_shader_create(AS_PATH_DEFAULT_VERT_SHADER, AS_PATH_DEFAULT_FRAG_SHADER);
	
	as_assign_texture_to_shader(shader, texture);
	as_object* object1 = as_object_create_with_tick(shader, &as_rotate_object);
	as_object* object2 = as_object_create(shader);
	as_object_set_instance_count(object2, 10000);
}
